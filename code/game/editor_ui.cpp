#include "editor_ui.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

void UpdateEditorUI(Game *game)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("ImguiOK", 0, ImGuiWindowFlags_AlwaysAutoResize);

    ParticleSystem *smoke = &game->smokeParticles;

    ImGui::SliderFloat("Radius", &smoke->radius, 0.0f, 20.0f);
    ImGui::SliderFloat3("Position", &smoke->pos[0], -30.0f, 30.0f);
    ImGui::SliderFloat4("Min Color", &smoke->minColor[0], 0.0f, 2.0f);
    ImGui::SliderFloat4("Max Color", &smoke->maxColor[0], 0.0f, 2.0f);
    //ImGui::SliderFloat4("Color Velocity", &smoke->colorVelocity[0], -1.0f, 0.0f);
    ImGui::DragFloat4("Color Velocity", &smoke->colorVelocity[0], -1.0f, 0.0f);

    int oldNumOfParticles = smoke->numOfParticles;
    if(ImGui::InputInt("Num Of Particles", &smoke->numOfParticles))
    {
        smoke->particles = (Particle *)realloc(smoke->particles, sizeof(Particle) * smoke->numOfParticles);
        smoke->particleData = (ParticleData *)realloc(smoke->particleData, sizeof(ParticleData) * smoke->numOfParticles);

        int diff = smoke->numOfParticles - oldNumOfParticles;
        if(diff > 0)
        {
            memset(smoke->particles + oldNumOfParticles, 0, diff * sizeof(Particle));
            memset(smoke->particleData + oldNumOfParticles, 0, diff * sizeof(ParticleData));
        }
        else
        {
            smoke->nextParticle = 0;
        }
    }

    ImGui::SliderInt("Spawn Rate", &smoke->spawnRate, 0, 200);

    ImGui::End();
}