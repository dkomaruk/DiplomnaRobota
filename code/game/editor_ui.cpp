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

    updated += ImGui::DragFloat("Radius", &smoke->radius, 1.0f, 0.0f, 2000.0f);
    updated += ImGui::DragFloat("Min Scale", &smoke->minScale, 0.1f, 0.0f, 2.0f);
    updated += ImGui::DragFloat("Max Scale", &smoke->maxScale, 0.1f, 0.0f, 2.0f);
    updated += ImGui::DragFloat("Min Scale Velocity", &smoke->minScaleVelocity, 0.05f, -2.0f, 2.0f);
    updated += ImGui::DragFloat("Max Scale Velocity", &smoke->maxScaleVelocity, 0.05f, -2.0f, 2.0f);

    glm::vec3 minPosDiff = glm::vec3(0.0f);
    glm::vec3 oldMinPos = smoke->minPos;
    if(ImGui::DragFloat3("Min Position", &smoke->minPos[0], 0.1f, -3.0f, 3.0f))
    {
        minPosDiff = smoke->minPos - oldMinPos;
        ++updated;
    }

    glm::vec3 maxPosDiff = glm::vec3(0.0f);
    glm::vec3 oldMaxPos = smoke->maxPos;
    if(ImGui::DragFloat3("Max Position", &smoke->maxPos[0], 0.1f, -3.0f, 3.0f))
    {
        maxPosDiff = smoke->minPos - oldMaxPos;
        ++updated;
    }

    updated += ImGui::DragFloat3("Min Velocity", &smoke->minVelocity[0], 0.1f, -2.0f, 2.0f);
    updated += ImGui::DragFloat3("Max Velocity", &smoke->maxVelocity[0], 0.1f, -2.0f, 2.0f);

    updated += ImGui::DragFloat4("Min Color", &smoke->minColor[0], 0.01f, 0.0f, 2.0f);
    updated += ImGui::DragFloat4("Max Color", &smoke->maxColor[0], 0.01f, 0.0f, 2.0f);
    updated += ImGui::DragFloat4("Min Color Velocity", &smoke->minColorVelocity[0], 0.01f, -1.0f, 0.0f);
    updated += ImGui::DragFloat4("Max Color Velocity", &smoke->maxColorVelocity[0], 0.01f, -1.0f, 0.0f);

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
            p->minScale = smoke->minScale;
            p->maxScale = smoke->maxScale;
            p->minScaleVelocity = smoke->minScaleVelocity;
            p->maxScaleVelocity = smoke->maxScaleVelocity;
            p->minPos += minPosDiff;
            p->maxPos += maxPosDiff;
            p->minVelocity = smoke->minVelocity;
            p->maxVelocity = smoke->maxVelocity;
            p->minColor = smoke->minColor;
            p->maxColor = smoke->maxColor;
            p->minColorVelocity = smoke->minColorVelocity;
            p->maxColorVelocity = smoke->maxColorVelocity;
            p->spawnRate = smoke->spawnRate;
        }
    }

    ImGui::End();
}