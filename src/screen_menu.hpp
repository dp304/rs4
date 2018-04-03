#ifndef SCREEN_MENU_HPP_INCLUDED
#define SCREEN_MENU_HPP_INCLUDED


#include "menu_control.hpp"
#include "menu_graphics.hpp"

template<class TPlatform, class TMachine>
class ScreenMenu : public rs4::IScreen
{
    typedef rs4::Caster<TMachine> Caster;

    typedef ControlMenu<typename TPlatform::Input, Caster> Control;
    typedef GraphicsMenu<typename TPlatform::Video> Graphics;

    Caster caster;

    rs4::IScreen * subscreen;

    Control control;
    Graphics graphics;

public:
    ScreenMenu(TPlatform * p, TMachine *m);
    void update(int dt) final
    {
        control.update();
        graphics.update(dt);
    }
    void render(float alpha) final
    {
        graphics.render(alpha);
    }
    void setSubscreen(rs4::IScreen * s) { subscreen=s; control.setSubscreen(s); graphics.setSubscreen(s); }
    rs4::IScreen * getSubscreen() const { return subscreen; }
private:
    void onStart() final {control.start();graphics.start();}
    void onStop() final {}
    void onPause() final {}
    void onUnpause() final {control.unpause();}
};

template<class TPlatform, class TMachine>
ScreenMenu<TPlatform,TMachine>::ScreenMenu(TPlatform * p, TMachine *m):
    caster(m),
    subscreen(nullptr),
    control(p->input, &caster),
    graphics(p->video)
{
}

#endif // SCREEN_MENU_HPP_INCLUDED
