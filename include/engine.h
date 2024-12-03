#pragma once
#include "scene.h"

namespace gpr
{

class Engine
{
public:
    explicit Engine(Scene* scene);
    void Run();
private:
    void Begin();
    void End();
    Scene* scene_ = nullptr;
    SDL_Window* window_ = nullptr;
    SDL_GLContext glRenderContext_{};
};
    
} // namespace gpr
