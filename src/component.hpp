#ifndef COMPONENT_HPP_INCLUDED
#define COMPONENT_HPP_INCLUDED

struct Position
{
    struct {float x, y;} buf[2];
	Position(float x0, float y0) :buf{ {x0,y0}, {x0,y0} } {}
};

struct Velocity
{
    float vx, vy;
};

struct Colour
{
    int r,g,b;
};

struct Health
{
    int hp;
    int maxhp;
};

struct Monster
{
    bool aggressive;
};

struct Player
{
    enum control_t {UP,DOWN,LEFT,RIGHT,FIRE1,FIRE2,n_CONTROL};
    bool control[n_CONTROL];
    int posX, posY;
    void clear() { *this = {{false},0,0}; }
};

struct Camera
{
    float distance;
    float x;
};

#endif // COMPONENT_HPP_INCLUDED
