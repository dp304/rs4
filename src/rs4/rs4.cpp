#include "rs4.hpp"
#include "rs4_sdlglal.hpp"

#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace rs4
{

std::shared_ptr<LDImage> LDImage::load(IStream * stream, const char * path)
{
    std::shared_ptr<LDImage> imp = std::make_shared<LDImage>();

    stbi_io_callbacks cbs;
    cbs.read = [](void * user, char * data, int size) -> int
        {
            IStream * s = reinterpret_cast<IStream*>(user);
            return s->read(data, 1, size);
        };
    cbs.skip = [](void * user, int n) -> void
        {
            IStream * s = reinterpret_cast<IStream*>(user);
            if (! s->seek(n, SEEK_CUR))
                throw std::runtime_error("Failed to skip in image stream");
        };
    cbs.eof = [](void * user) -> int
        {
            IStream * s = reinterpret_cast<IStream*>(user);
            return s->eof();
        };


    stbi_uc * p = nullptr;
    try{
        stbi_set_flip_vertically_on_load(1);
        p = stbi_load_from_callbacks(&cbs, stream, &imp->width, &imp->height, &imp->pixel_size, 0);

        imp->pix.resize(imp->width*imp->height*imp->pixel_size);
        std::copy(p, p+imp->pix.size(), imp->pix.begin());

    }
    catch(...)
    {
        if (p) stbi_image_free(p);
        throw;
    }
    stbi_image_free(p);

    return imp;
}

static inline std::int32_t le32toi(const char * p)
{
    return ((p[0]&0xFF)<<0) | ((p[1]&0xFF)<<8) | ((p[2]&0xFF)<<16) | ((p[3]&0xFF)<<24);
}

static inline std::int16_t le16toi(const char * p)
{
    return ((p[0]&0xFF)<<0) | ((p[1]&0xFF)<<8);
}

std::shared_ptr<LDSound> LDSound::load(IStream * stream, const char * path)
{
    std::shared_ptr<LDSound> sp = std::make_shared<LDSound>();

    char wav_hdr[44];
    char * const wav_Format = wav_hdr + 8;
    char * const wav_Subchunk1ID = wav_hdr + 12;
    char * const wav_Subchunk1Size = wav_hdr + 16;
    char * const wav_AudioFormat = wav_hdr + 20;
    char * const wav_NumChannels = wav_hdr + 22;
    char * const wav_SampleRate = wav_hdr + 24;
    char * const wav_BitsPerSample = wav_hdr + 34;
    char * const wav_Subchunk2ID = wav_hdr + 36;
    char * const wav_Subchunk2Size = wav_hdr + 40;

    if (stream->read(wav_hdr, sizeof(char), 44) != 44)
        throw std::runtime_error("Unexpected end of stream reading WAV header");

    if (!std::equal(wav_hdr, wav_hdr+4, "RIFF") ||
        !std::equal(wav_Format, wav_Format+4, "WAVE") ||
        !std::equal(wav_Subchunk1ID, wav_Subchunk1ID+4, "fmt ")
        )
        throw std::runtime_error("Format error reading WAV");

    if (le32toi(wav_Subchunk1Size) != 16 ||
        le16toi(wav_AudioFormat) != 1
        )
        throw std::runtime_error("Only PCM WAV is supported");

    sp->channels = le16toi(wav_NumChannels);
    sp->rate = le32toi(wav_SampleRate);
    sp->sample_size = le16toi(wav_BitsPerSample);
    if (sp->sample_size % 8 != 0)
        throw std::runtime_error("Bits per sample in WAV must be multiple of 8");
    sp->sample_size /= 8;

    if (!std::equal(wav_Subchunk2ID, wav_Subchunk2ID+4, "data"))
        throw std::runtime_error("Format error reading WAV");

    long long len = le32toi(wav_Subchunk2Size);
    long long fsiz = stream->size();
    if (fsiz != -1 && fsiz != len + 44)
        throw std::runtime_error("Invalid size of WAV data");

    sp->samples.resize((std::size_t)len);

    if (stream->read(&sp->samples[0], sizeof(char), sp->samples.size()) != sp->samples.size())
        throw std::runtime_error("Unexpected end of stream reading WAV data");

    return sp;
}

class StreamMusicVorbis : public StreamMusic
{
    OggVorbis_File ovf;
    ov_callbacks cbs;

    void onOpenr() final;
    void onOpenw() final { throw std::runtime_error("StreamMusicVorbis cannot be opened for writing"); }
    void onClose() final;
    std::size_t onRead(void * buf, std::size_t siz, std::size_t num) final;
    std::size_t onWrite(const void * buf, std::size_t siz, std::size_t num) final { return 0; }
    //long long onTell() final;
    //bool onSeek(long long offset, int whence) final;
public:
    StreamMusicVorbis(std::unique_ptr<IStream> && src);
    ~StreamMusicVorbis();
};

StreamMusicVorbis::StreamMusicVorbis(std::unique_ptr<IStream> && src):
    StreamMusic{std::move(src)}
{
    cbs.read_func = [](void *ptr, size_t size, size_t nmemb, void *datasource) -> size_t
        {
            IStream * s = reinterpret_cast<IStream*>(datasource);
            return s->read(ptr, size, nmemb);
        };
    cbs.seek_func = [](void *datasource, ogg_int64_t offset, int whence) -> int
        {
            IStream * s = reinterpret_cast<IStream*>(datasource);
            if (s->seek(offset, whence))
                return 0;
            return -1;
        };
    cbs.tell_func = [](void *datasource) -> long
        {
            IStream * s = reinterpret_cast<IStream*>(datasource);
            return (long)(s->tell());
        };
    cbs.close_func = [](void *datasource) -> int
        {
            IStream * s = reinterpret_cast<IStream*>(datasource);
            s->close();
            return 0;
        };
}

void StreamMusicVorbis::onOpenr()
{
    source_stream->openr();
    if (ov_open_callbacks(source_stream.get(), &ovf, nullptr, 0, cbs))
        throw std::runtime_error("Failed to open Ogg Vorbis stream");
    vorbis_info * ovi = ov_info(&ovf, -1);
    channels = ovi->channels;
    rate = ovi->rate;
    // TODO setSize()
    sample_size = 2;
}

void StreamMusicVorbis::onClose()
{
    ov_clear(&ovf);
    channels = rate = sample_size = -1;
}

std::size_t StreamMusicVorbis::onRead(void * buf, std::size_t siz, std::size_t num)
{
    int bitstream;
    std::size_t n_start = 0;
    std::size_t n_remaining = num*siz;
    std::size_t n_got;
    do
    {
        n_got = ov_read(&ovf, (char*)buf+n_start, n_remaining, 0, 2, 1, &bitstream);

        if ( n_got==0 && (!loop || ov_raw_seek_lap(&ovf, 0)!=0) )
            return n_start / siz;

        n_start += n_got;
        n_remaining -= n_got;
    }
    while (n_remaining>0);
    return n_start / siz; //FAKE!

}

StreamMusicVorbis::~StreamMusicVorbis()
{
    if (isOpen()) close();
}

std::unique_ptr<StreamMusic> makeStreamMusicVorbis(std::unique_ptr<IStream> && stream)
{
    std::unique_ptr<StreamMusic> res = std::make_unique<StreamMusicVorbis>(std::move(stream));
    return std::move(res);
}



} // namespace rs4
