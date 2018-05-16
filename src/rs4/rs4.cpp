#include "rs4.hpp"

#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
            s->skip(n);
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

    sp->samples.resize(len);

    if (stream->read(&sp->samples[0], sizeof(char), sp->samples.size()) != sp->samples.size())
        throw std::runtime_error("Unexpected end of stream reading WAV data");

    return sp;
}

} // namespace rs4
