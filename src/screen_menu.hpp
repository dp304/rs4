#ifndef SCREEN_MENU_HPP_INCLUDED
#define SCREEN_MENU_HPP_INCLUDED


template<class TPlatform>
class ScreenMenu : public rs4::IScreen
{
public:
    ScreenMenu(rs4::Game * game, TPlatform * platform);
private:
    void onStart(std::size_t i1) final {}
};


#endif // SCREEN_MENU_HPP_INCLUDED
