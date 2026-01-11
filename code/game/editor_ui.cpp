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

    int updated = 0;
    ParticleSystem *smoke = &game->particleSystems[0];

    updated += ImGui::DragFloat("Radius", &smoke->radius, 0.0f, 2000.0f);

    glm::vec3 diff = glm::vec3(0.0f);
    glm::vec3 oldPos = smoke->pos;
    if(ImGui::SliderFloat3("Position", &smoke->pos[0], -30.0f, 30.0f))
    {
        diff = smoke->pos - oldPos;
        ++updated;
    }

    updated += ImGui::SliderFloat4("Min Color", &smoke->minColor[0], 0.0f, 2.0f);
    updated += ImGui::SliderFloat4("Max Color", &smoke->maxColor[0], 0.0f, 2.0f);
    updated += ImGui::DragFloat4("Color Velocity", &smoke->colorVelocity[0], -1.0f, 0.0f);

    int oldNumOfParticles = smoke->maxNumOfParticles;
    if(ImGui::InputInt("Num Of Particles", &smoke->maxNumOfParticles))
    {
        int particleNumDiff = smoke->maxNumOfParticles - oldNumOfParticles;
        for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
        {
            ParticleSystem *p = &game->particleSystems[i];
            p->maxNumOfParticles = smoke->maxNumOfParticles;

            p->particles = (Particle *)realloc(p->particles, sizeof(Particle) * p->maxNumOfParticles);
            if(particleNumDiff > 0)
            {
                memset(p->particles + oldNumOfParticles, 0, particleNumDiff * sizeof(Particle));
            }
            else
            {
                p->nextParticle = 0;
            }
        }

        int particleSystemsSize = sizeof(ParticleData) * smoke->maxNumOfParticles * ArrayCount(game->particleSystems);
        game->particleData = (ParticleData *)realloc(game->particleData, particleSystemsSize);
        if(particleNumDiff > 0)
        {
            memset(game->particleData + oldNumOfParticles * ArrayCount(game->particleSystems), 0, particleNumDiff * sizeof(ParticleData) * ArrayCount(game->particleSystems));
        }
    }

    updated += ImGui::SliderInt("Spawn Rate", &smoke->spawnRate, 0, 200);

    if(ImGui::Combo("Texture", &game->currentTexture, "smoke\0smoke2\0smoke3\0smoke4\0smoke5\0\0"))
    {
        game->textureID = game->particleTextures[game->currentTexture].id;
        ++updated;
    }

    if(updated > 0 )
    {
        for(int i = 1; i < ArrayCount(game->particleSystems); i++)
        {
            ParticleSystem *p = &game->particleSystems[i];
            p->radius = smoke->radius;
            p->pos += diff;
            p->minColor = smoke->minColor;
            p->maxColor = smoke->maxColor;
            p->colorVelocity = smoke->colorVelocity;
            p->spawnRate = smoke->spawnRate;
        }
    }

    ImGui::End();
}