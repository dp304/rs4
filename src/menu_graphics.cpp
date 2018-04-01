
#include "menu_graphics.hpp"


GraphicsMenu<rs4::VideoSDLGL>::GraphicsMenu(rs4::VideoSDLGL * video):
        video{video}
{

}

void GraphicsMenu<rs4::VideoSDLGL>::render(float alpha)
{


    if (subscreen != nullptr)
    {
        subscreen->render(alpha);

        /*glEnable(GL_BLEND);
        glClearColor(0.5f, 0.5f, 0.5f, 0.2f);

        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);

        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_BLEND);*/

        //RS4_GLERRORS
    }
    else
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

}



// TEST

void GraphicsMenu<rs4::VideoTest>::render(float alpha)
{
    //std::size_t idxBufOld = 1 - idxBuf;

}
