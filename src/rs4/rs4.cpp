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


    stream->openr();
    stbi_uc * p = nullptr;
    try{
        stbi_set_flip_vertically_on_load(1);
        p = stbi_load_from_callbacks(&cbs, stream, &imp->width, &imp->height, &imp->pixel_size, 0);

        imp->pix.resize(imp->width*imp->height*imp->pixel_size);
        std::copy(p, p+imp->pix.size(), imp->pix.begin());

    }
    catch(...)
    {
        stream->close();
        if (p) stbi_image_free(p);
        throw;
    }
    stream->close();
    stbi_image_free(p);

    return imp;
}

} // namespace rs4
