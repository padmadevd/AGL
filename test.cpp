#include <AGL/agl.hpp>

#include <cstdlib>
#include <iostream>

int time = 0;

void OnTap(const Event *event, void *data){

    const EventGesture *g_event = (const EventGesture*)event;

    void **ptr = (void**)data;

    if(g_event->m_type == GESTURE_DRAG){
        
        // std::cout<<"drag bro\n";

        time += 1;
        if(time%30 != 0)
            return;

        Polygon2D &poly = *(Polygon2D*)(ptr[0]);
        Window &w = *(Window*)(ptr[1]);
        Camera2D &cam = *(Camera2D*)(ptr[2]);
        int mode = *(int*)(ptr[3]);

        if(mode != 0)
            return;

        glm::vec2 pos = g_event->m_end;
        pos.x *= w.GetWidth();
        pos.y *= w.GetHeight();
        pos = cam.ScreenToWorld(pos);

        poly.Add(pos);
    }
}

void genPoly(std::vector<glm::vec2> &points){

    float angle = 0;
    float radius = 10;

    while(angle < 360){

        radius *= -1;
        float r = 60+radius;

        glm::vec2 point;
        point.x = glm::cos(glm::radians(angle))*r;
        point.y = glm::sin(glm::radians(angle))*r;
        points.emplace_back(point);

        angle += 30;
    }
}

void genRandomColor(std::vector<glm::u8vec4> &colors, int n){

    for(int i = 0; i < n; i++){

        glm::u8vec4 color;
        color.r = rand()%256;
        color.g = rand()%256;
        color.b = rand()%256;
        color.a = 100;

        colors.emplace_back(color);
    }
}

#ifdef __cplusplus
extern "C" {
#endif

int WinMain(int argc, char *argv[]){

    AGLInit("AGL app", 100, 100, 800, 400);

    Window &w = GetWindowInstance();
    w.SetResizable(true);

    KeyInput &input = GetKeyInputInstance();
    BatchRenderer2D &renderer = GetRendererInstance();

    Camera2D cam;
    cam.m_pos = {0, 0};
    cam.m_width = 800;
    cam.m_height = 400;
    cam.m_rotation = 0;
    cam.m_zoom = 1;

    renderer.ClearColor({0, 0, 0, 1.f});

    Texture2D *flower = new Texture2D;
    flower->FromFile("flower.png", REPEAT, LINEAR, LINEAR);

    Image *circle = new Image;
    circle->GenGradientRadial(200, 200, 0, {255, 255, 255, 255}, {255, 255, 255, 0});
    circle->DrawLine({50, 50}, {150, 150}, {255, 255, 255, 255});

    Texture2D *texture = new Texture2D;
    texture->FromImage(circle, REPEAT, NEAREST, NEAREST);

    Texture2D *img = new Texture2D;
    img->FromFile("img.png", REPEAT, LINEAR, LINEAR);

    glm::vec2 origin = {0, 0};
    Particles *particles1 = CreateParticles(origin, 90, 90, 110, 40, 0, 2, .1, 0, {.3, .65, 1, .7}, {.3, .5, .7, 0}, {0, -1}, 500, 1000, texture);
    Particles *particles2 = CreateParticles(origin, 90, 70, 90, 30, 0, 2, .05, 0, {.7, .7, 1, .7}, {.7, .7, 1, 0}, {0, -1}, 500, 1000, texture);

    uint64_t lasttime = SDL_GetTicks64();
    uint64_t delta = 0;

    float rotation = 0;

    float x = 10;
    float y = 0;

    std::vector<glm::vec2> path ={{-200, 10}, {-180, 100}, {-140, -40}, {-100, 50}, {0, 150}, {50, 60}, {100, -20}, {150, -130}, {200, 100}};

    Polygon2D polygon;
    // genPoly(polygon.m_points);
    // polygon.Add({{-50, -50}, {50, -50}, {50, 50}, {-50, 50}});

    std::vector<Polygon2D> convex_polys;

    std::vector<glm::u8vec4> colors;

    int mode = 0;

    void *data[] = {&polygon, &w, &cam, &mode};
    EventDispatcher &eventDispatcher = GetEventDispatcherInstance();
    eventDispatcher.Register("AGL_EVENT_GESTURE", new EventHandler(&OnTap, data, "tap handler"));

    while(!w.ShouldClose()){

        delta = SDL_GetTicks64() - lasttime;
        lasttime = SDL_GetTicks64();

        PollEvents();

        if(input.isKeyPressed(KEY_SPACE)){
            mode = 0;
            polygon.m_points.clear();
        }
        if(input.isKeyPressed(KEY_RETURN)){
            mode = 1;
            convex_polys.clear();
            Decompose(polygon, convex_polys);
            colors.clear();
            genRandomColor(colors, convex_polys.size());
        }

        rotation += 45*(delta/1000.0);
        if(rotation > 360)
            rotation -= 360;

        particles1->origin = origin;
        particles2->origin = origin;

        // UpdateParticles(particles1, delta/1000.0);
        // UpdateParticles(particles2, delta/1000.0);

        renderer.Clear();
        renderer.BeginDraw(cam);
            // renderer.DrawRectangle({0, 0, 100, 100}, {0, 0}, 0, {255, 255, 255, 255});
            // renderer.DrawLineBezier({0, 0}, {100, 100}, 20, 4, {255, 0, 0, 255});
            // renderer.DrawPolyLines({0, 0}, 5, 30, 0, 4, {0, 0, 255, 255});
            // renderer.DrawSplineLinear(path.data(), path.size(), 10, {255, 255, 0, 100});
            // renderer.DrawSplineBasis(path.data(), path.size(), 10, {255, 0, 255, 100});
            // renderer.DrawSplineCatmullRom(path.data(), path.size(), 10, {0, 0, 255, 100});
            // renderer.DrawSplineBezierQuadratic(path.data(), path.size(), 10, {0, 255, 255, 100});
            // renderer.DrawPixel({10, 10}, {255, 255, 255, 255});
            // renderer.DrawCircleSector({x, y}, 50, 0, 360, 10, {150, 150, 250, 255});
            // renderer.DrawCircle({0, 0}, 50, {255, 255, 255, 255});
            // renderer.DrawRing({x, y}, 50, 70, 0, 360, 36, {255, 255, 255, 255});
            // renderer.DrawTexture2D(texture, {0, 0, texture->GetWidth(), texture->GetHeight()}, {x, y, texture->GetWidth()/2, texture->GetHeight()/2}, {texture->GetWidth()/4, texture->GetHeight()/4}, 0, {255,255, 255, 255});
            // // renderer.DrawTexture2D(flower, {0, 0, flower->GetWidth(), flower->GetHeight()}, {0, 0, 200, 200}, {100, 100}, rotation, {255, 255, 255, 255});

            if(mode == 1){
                for(int i = 0; i < convex_polys.size(); i++)
                    renderer.DrawConvexPolygon(convex_polys[i].m_points.data(), convex_polys[i].m_points.size(), {0, 0}, 0, colors[i]);    
            }

            for(int i = 0; i < polygon.size(); i++)
                    renderer.DrawCircle(polygon.m_points[i], 3, {255, 255, 255, 155});

        renderer.EndDraw();
        // RenderParticles(particles1, cam, false);
        // RenderParticles(particles2, cam, false);

        w.SwapBuffers();
    }
    delete texture;
    delete circle;
    FreeParticles(particles1);
    FreeParticles(particles2);
    AGLQuit();
    return 0;
}

#ifdef __cplusplus
}
#endif