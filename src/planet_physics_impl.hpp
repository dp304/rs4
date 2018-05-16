
#include <glm/glm.hpp>
#include "component.hpp"
#include "event.hpp"

template<class TCaster>
PhysicsPlanet<TCaster>::PhysicsPlanet(TCaster * c, rs4::Game * g, World * w)
        :caster(c),world(w)
{

    entt::DefaultRegistry::entity_type ent =
        world->registry.create(
            Position{{{0,0},{0,0}}},
            Velocity{0,0},
            Colour{255,255,255},
            Health{30,30}
            );
    world->registry.attach<Player>(ent);
    world->registry.attach<Camera>(ent, Camera{2.0f, 0.0f});
    //Camera & cam = world->registry.get<Camera>();
    //cam.distance = 2.0f;
    //cam.x = 0.0f;

    for (int i=0; i<20; i++)
    {
        float x, y;
        do
        {
            x = (float)rd()/rd.max()*1.6-0.8;
            y = (float)rd()/rd.max()*1.6-0.8;
        }
        while (glm::abs(x)<0.5f && glm::abs(y)<0.5f);

        float vx = (float)rd()/rd.max()*0.8-0.4, vy = (float)rd()/rd.max()*0.8-0.4;
        //float x = 25, y = 25+i*20;
        //float vx = (float)(i+1)*20, vy = 0.0f;
        int r, g = rd()%255, b = rd()%255;
        r = 127; //glm::min(g,b);
        //int r = 255, g = 255, b = 255;
        /*entt::DefaultRegistry::entity_type ent =*/
            world->registry.create(
                Position{{{x,y},{x,y}}},
                Velocity{vx,vy},
                Colour{r,g,b},
                Health{10,10},
                Monster{true}
                );

    }

}

template<class TCaster>
void PhysicsPlanet<TCaster>::update(int dt) {
    /*static bool ki=false;
    if(ki)game->exit();
    ki=true;*/

    world->swapI();
    const std::size_t i1 = world->i1;
    const std::size_t i0 = 1 - i1;



    auto pent = world->registry.attachee<Player>();
    Player &pcontrol = world->registry.get<Player>();
    Position &pposition = world->registry.get<Position>(pent);
    Velocity &pvelocity = world->registry.get<Velocity>(pent);
    Colour &pcolour = world->registry.get<Colour>(pent);
    Health &phealth = world->registry.get<Health>(pent);


    auto view = world->registry.persistent<Position, Velocity, Colour, Health>();

    for(auto entity: view)
    {
        Position &p = view.get<Position>(entity);
        Velocity &v = view.get<Velocity>(entity);

        p.buf[i1].x = p.buf[i0].x + v.vx * (0.001f*dt);
        p.buf[i1].y = p.buf[i0].y + v.vy * (0.001f*dt);

        bool coll = false;

        if (&p != &pposition
            && glm::abs(p.buf[i1].x - pposition.buf[i1].x) < 0.1
            && glm::abs(p.buf[i1].y - pposition.buf[i1].y) < 0.1
            )
        {
            // szorny utkozik hajoval
            coll = true;
            v.vx = -v.vx;
            v.vy = -v.vy;
            phealth.hp -= 1;
            pcolour = {pcolour.r, phealth.hp*pcolour.g/(phealth.hp+1), phealth.hp*pcolour.b/(phealth.hp+1)};
            if (phealth.hp == 0)
                world->registry.destroy(pent);

        }

        coll = collide(p.buf[i0].x,p.buf[i1].x,v.vx,-1.9)||coll;
        coll = collide(p.buf[i0].x,p.buf[i1].x,v.vx,1.9)||coll;
        coll = collide(p.buf[i0].y,p.buf[i1].y,v.vy,-0.9)||coll;
        coll = collide(p.buf[i0].y,p.buf[i1].y,v.vy,0.9)||coll;

        if (coll)
        {
            caster->signal(EventCollision{p.buf[i1].x, p.buf[i1].y});
            Colour & c = view.get<Colour>(entity);
            Health & h = view.get<Health>(entity);
            h.hp -= 1;
            c = {c.r, h.hp*c.g/(h.hp+1), h.hp*c.b/(h.hp+1)};
            if (h.hp == 0)
                world->registry.destroy(entity);
        }
        //assert(p.buf[i1].x>0 && p.buf[i1].x<800 && p.buf[i1].y>0 && p.buf[i1].y<600);
    }
    if (!world->registry.has<Player>() || world->registry.empty())
    {
        caster->signal(EventGameOver{});
        return;
    }

    //pposition.buf[i1].x = pposition.buf[i1].x + pcontrol.posX * 0.01f;
    //pposition.buf[i1].y = pposition.buf[i1].y + pcontrol.posY * 0.01f;
    //pposition.buf[i0].x = pposition.buf[i0].x + pcontrol.posX * 0.01f;
    //pposition.buf[i0].y = pposition.buf[i0].y + pcontrol.posY * 0.01f;
    Camera &cam = world->registry.get<Camera>();
    cam.distance -= pcontrol.posY * 0.02f;
    if (cam.distance < 0.0f)  cam.distance = 0.0f;
    if (cam.distance > 10.0f)  cam.distance = 10.0f;
    cam.x += pcontrol.posX * 0.02f;
    if (cam.x < -2.0f)  cam.x = -2.0f;
    if (cam.x > 2.0f)  cam.x = 2.0f;



    if (pcontrol.control[pcontrol.RIGHT]) pvelocity.vx = pvelocity.vx + 0.01f;
    if (pcontrol.control[pcontrol.LEFT]) pvelocity.vx = pvelocity.vx - 0.01f;
    if (pcontrol.control[pcontrol.UP]) pvelocity.vy = pvelocity.vy + 0.01f;
    if (pcontrol.control[pcontrol.DOWN]) pvelocity.vy = pvelocity.vy - 0.01f;



}

