#include "render.h"


RenderSystem* CreateRenderSystem() {

    if (config == "OpenGL") {
        return OpenGLRenderSystem::AllocRenderSystem()
    }

    return nullptr;
}

std::unique_ptr<RenderSystem> RenderSystem::Load() {
    auto renderSystem = std::unique_ptr<RenderSystem>(CreateRenderSystem());
    return renderSystem;
}


