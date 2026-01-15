#include "editor_ui.h"

#include <imgui.h>
//#include <imgui_curve.hpp>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

void UpdateEditorUI(Game *game)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Particle System Editor", 0, ImGuiWindowFlags_AlwaysAutoResize);

    ParticleSystemSettings *smoke = &game->smokeSettings;

    static bool limitedLifetime = true;
    ImGui::Checkbox("Limited Lifetime", &limitedLifetime);
    if(limitedLifetime)
    {
        ImGui::InputFloat("Existence Seconds", &smoke->lifetime);
    }
    else
    {
        smoke->lifetime = 0.0f;
    }

    ImGui::DragFloat("Radius", &smoke->radius, 1.0f, 0.0f, 2000.0f);
    ImGui::DragFloat("Min Scale", &smoke->minScale, 0.1f, 0.0f, 2.0f);
    ImGui::DragFloat("Max Scale", &smoke->maxScale, 0.1f, 0.0f, 2.0f);
    ImGui::DragFloat("Min Scale Velocity", &smoke->minScaleVelocity, 0.05f, -2.0f, 2.0f);
    ImGui::DragFloat("Max Scale Velocity", &smoke->maxScaleVelocity, 0.05f, -2.0f, 2.0f);

    ImGui::DragFloat3("Min Position", &smoke->minOffset[0], 0.1f, -3.0f, 3.0f);
    ImGui::DragFloat3("Max Position", &smoke->maxOffset[0], 0.1f, -3.0f, 3.0f);

    ImGui::DragFloat3("Min Velocity", &smoke->minVelocity[0], 0.1f, -2.0f, 2.0f);
    ImGui::DragFloat3("Max Velocity", &smoke->maxVelocity[0], 0.1f, -2.0f, 2.0f);

    ImGui::DragFloat3("Min Acceleration", &smoke->minAccel[0], 0.1f, -2.0f, 2.0f);
    ImGui::DragFloat3("Max Acceleration", &smoke->maxAccel[0], 0.1f, -2.0f, 2.0f);

    ImGui::DragFloat4("Min Color", &smoke->minColor[0], 0.01f, 0.0f, 2.0f);
    ImGui::DragFloat4("Max Color", &smoke->maxColor[0], 0.01f, 0.0f, 2.0f);
    ImGui::DragFloat4("Min Color Velocity", &smoke->minColorVelocity[0], 0.01f, -1.0f, 0.0f);
    ImGui::DragFloat4("Max Color Velocity", &smoke->maxColorVelocity[0], 0.01f, -1.0f, 0.0f);

    int oldNumOfParticles = smoke->maxNumOfParticles;
    if(ImGui::InputInt("Num Of Particles", &smoke->maxNumOfParticles))
    {
        int particleNumDiff = smoke->maxNumOfParticles - oldNumOfParticles;
        for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
        {
            ParticleSystem *p = &game->particleSystems[i];

            p->particles = (Particle *)realloc(p->particles, sizeof(Particle) * smoke->maxNumOfParticles);
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

    ImGui::DragInt("Spawn Rate", &smoke->spawnRate, 1, 0, 1000);

    if(ImGui::Combo("Texture", &game->currentTexture, "smoke\0smoke2\0smoke3\0smoke4\0smoke5\0\0"))
    {
        game->textureID = game->particleTextures[game->currentTexture].id;
    }

    if(ImGui::CollapsingHeader("Color Over Lifetime"))
    {
        static int selection = -1;
        static ImVec2 points[10] = {ImVec2(ImGui::CurveTerminator, 0.0f)};
        ImGui::Curve("Test", ImVec2(300, 300), ArrayCount(points), points, &selection, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

        static float x = 0.0f;
        ImGui::InputFloat("X ", &x);
        ImGui::SameLine();
        ImGui::LabelText("Y", "%f", ImGui::CurveValue(x, ArrayCount(points), points));
    }

    ImGui::End();
}