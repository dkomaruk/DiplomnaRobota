#include "editor_ui.h"

#include "particle_system.h"

#include <imgui.h>
//#include <imgui_curve.hpp>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace glm
{
    void to_json(json &j, const glm::vec2 &v)
    {
        j = json{{"x", v.x}, {"y", v.y}};
    }

    void from_json(const json &j, glm::vec2 &v)
    {
        v.x = j.at("x").get<float>();
        v.y = j.at("y").get<float>();
    }

    void to_json(json &j, const glm::vec3 &v)
    {
        j = json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
    }

    void from_json(const json &j, glm::vec3 &v)
    {
        v.x = j.at("x").get<float>();
        v.y = j.at("y").get<float>();
        v.z = j.at("z").get<float>();
    }

    void to_json(json &j, const glm::vec4 &v)
    {
        j = json{{"r", v.r}, {"g", v.g}, {"b", v.b}, {"a", v.a}};
    }

    void from_json(const json &j, glm::vec4 &v)
    {
        v.r = j.at("r").get<float>();
        v.g = j.at("g").get<float>();
        v.b = j.at("b").get<float>();
        v.a = j.at("a").get<float>();
    }
}

void SaveSettings(const ParticleSystemSettings& settings, const std::string& filepath) {
    json j;
    j["maxNumOfParticles"] = settings.maxNumOfParticles;
    j["prewarm"] = settings.prewarm;
    j["prewarmSeconds"] = settings.prewarmSeconds;

    j["spawnRate"] = settings.spawnRate;
    j["lifetime"] = settings.lifetime;
    j["limitedLife"] = settings.limitedLife;

    j["radius"] = settings.radius;

    j["minRotation"] = settings.minRotation;
    j["maxRotation"] = settings.maxRotation;
    j["minRotationSpeed"] = settings.minRotationSpeed;
    j["maxRotationSpeed"] = settings.maxRotationSpeed;

    j["minScale"] = settings.minScale;
    j["maxScale"] = settings.maxScale;
    j["minScaleVelocity"] = settings.minScaleVelocity;
    j["maxScaleVelocity"] = settings.maxScaleVelocity;

    j["minOffset"] = settings.minOffset;
    j["maxOffset"] = settings.maxOffset;
    j["minVelocity"] = settings.minVelocity;
    j["maxVelocity"] = settings.maxVelocity;

    j["minAccel"] = settings.minAccel;
    j["maxAccel"] = settings.maxAccel;

    j["minColor"] = settings.minColor;
    j["maxColor"] = settings.maxColor;
    j["minColorVelocity"] = settings.minColorVelocity;
    j["maxColorVelocity"] = settings.maxColorVelocity;

    j["isAnimated"] = settings.isAnimated;
    j["animationFPS"] = settings.animationFPS;
    if(settings.atlas)
    {
        j["atlasPath"] = settings.atlas->path;
    }


    // Handle Union/Array
    for(int i = 0; i < PARTICLES_MAX_CONTROL_POINTS; ++i)
    {
        j["velocityControlPoints"].push_back({settings.velocityControlPoints[i].x,
                                              settings.velocityControlPoints[i].y});
    }

    j["velocityOverLifetime"] = settings.velocityOverLifetime;

    std::ofstream file(filepath);
    file << j.dump(4);
}

void LoadSettings(ParticleSystemSettings& settings, const std::string &filepath, Atlas *atlas)
{
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    json j;
    file >> j;

    settings.maxNumOfParticles = j.at("maxNumOfParticles");
    settings.prewarm = j.at("prewarm");
    settings.prewarmSeconds = j.at("prewarmSeconds");

    settings.spawnRate = j.at("spawnRate");
    settings.lifetime = j.at("lifetime");
    settings.limitedLife = j.at("limitedLife");

    settings.radius = j.at("radius");

    settings.minRotation = j.at("minRotation");
    settings.maxRotation = j.at("maxRotation");
    settings.minRotationSpeed = j.at("minRotationSpeed");
    settings.maxRotationSpeed = j.at("maxRotationSpeed");

    settings.minScale = j.at("minScale");
    settings.maxScale = j.at("maxScale");
    settings.minScaleVelocity = j.at("minScaleVelocity");
    settings.maxScaleVelocity = j.at("maxScaleVelocity");

    settings.minOffset = j.at("minOffset").get<glm::vec3>();
    settings.maxOffset = j.at("maxOffset").get<glm::vec3>();
    settings.minVelocity = j.at("minVelocity").get<glm::vec3>();
    settings.maxVelocity = j.at("maxVelocity").get<glm::vec3>();

    settings.minAccel = j.at("minAccel").get<glm::vec3>();
    settings.maxAccel = j.at("maxAccel").get<glm::vec3>();

    settings.minColor = j.at("minColor").get<glm::vec4>();
    settings.maxColor = j.at("maxColor").get<glm::vec4>();
    settings.minColorVelocity = j.at("minColorVelocity").get<glm::vec4>();
    settings.maxColorVelocity = j.at("maxColorVelocity").get<glm::vec4>();

    settings.atlas = atlas;
    settings.isAnimated = j.at("isAnimated");
    settings.animationFPS = j.at("animationFPS");

    if(j.contains("velocityControlPoints") && j["velocityControlPoints"].is_array())
    {
        const auto &pointsJson = j["velocityControlPoints"];

        size_t count = std::min(pointsJson.size(), (size_t)PARTICLES_MAX_CONTROL_POINTS);
        for(size_t i = 0; i < count; ++i)
        {
            settings.velocityControlPoints[i].x = pointsJson[i][0].get<float>();
            settings.velocityControlPoints[i].y = pointsJson[i][1].get<float>();
        }
    }

    settings.velocityOverLifetime = j.at("velocityOverLifetime");
}

void ReallocParticles(Game *game, ParticleSystemSettings *settings, int oldNumOfParticles)
{
    int particleNumDiff = settings->maxNumOfParticles - oldNumOfParticles;
    for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
    {
        ParticleSystem *p = &game->particleSystems[i];

        p->particles = (Particle *)realloc(p->particles, sizeof(Particle) * settings->maxNumOfParticles);
        if(particleNumDiff > 0)
        {
            memset(p->particles + oldNumOfParticles, 0, particleNumDiff * sizeof(Particle));
        }
        else
        {
            p->nextParticle = 0;
        }

        p->accumulatedSpawns = 0.0f;
    }

    int particleSystemsSize = sizeof(ParticleData) * settings->maxNumOfParticles * ArrayCount(game->particleSystems);
    game->particleData = (ParticleData *)realloc(game->particleData, particleSystemsSize);
    if(particleNumDiff > 0)
    {
        memset(game->particleData + oldNumOfParticles * ArrayCount(game->particleSystems), 0, particleNumDiff * sizeof(ParticleData) * ArrayCount(game->particleSystems));
    }
}


void UpdateEditorUI(Game *game)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Particle System Editor", 0, ImGuiWindowFlags_AlwaysAutoResize);

    ParticleSystemSettings *smoke = &game->smokeSettings;

    ImGui::Checkbox("Limited Lifetime", &smoke->limitedLife);
    if(smoke->limitedLife)
    {
        ImGui::InputFloat("Existence Seconds", &smoke->lifetime);
    }

    ImGui::DragFloat("Radius", &smoke->radius, 1.0f, 0.0f, 2000.0f);

    if(ImGui::CollapsingHeader("Position"))
    {
        ImGui::DragFloat3("Min Position", &smoke->minOffset[0], 0.1f, -3.0f, 3.0f);
        ImGui::DragFloat3("Max Position", &smoke->maxOffset[0], 0.1f, -3.0f, 3.0f);
    }

    if(ImGui::CollapsingHeader("Rotation"))
    {
        ImGui::DragFloat("Min Rotation", &smoke->minRotation, 1.0f, 0.0f, 360.0f);
        ImGui::DragFloat("Max Rotation", &smoke->maxRotation, 1.0f, 0.0f, 360.0f);
        ImGui::DragFloat("Min Rotation Speed", &smoke->minRotationSpeed, 1.0f, -180.0f, 180.0f);
        ImGui::DragFloat("Max Rotation Speed", &smoke->maxRotationSpeed, 1.00f, -180.0f, 180.0f);
    }

    if(ImGui::CollapsingHeader("Scale"))
    {
        ImGui::DragFloat("Min Scale", &smoke->minScale, 0.1f, 0.0f, 2.0f);
        ImGui::DragFloat("Max Scale", &smoke->maxScale, 0.1f, 0.0f, 2.0f);
        ImGui::DragFloat("Min Scale Velocity", &smoke->minScaleVelocity, 0.05f, -2.0f, 2.0f);
        ImGui::DragFloat("Max Scale Velocity", &smoke->maxScaleVelocity, 0.05f, -2.0f, 2.0f);
    }

    if(ImGui::CollapsingHeader("Speed"))
    {
        ImGui::DragFloat3("Min Velocity", &smoke->minVelocity[0], 0.1f, -2.0f, 2.0f);
        ImGui::DragFloat3("Max Velocity", &smoke->maxVelocity[0], 0.1f, -2.0f, 2.0f);
        ImGui::DragFloat3("Min Acceleration", &smoke->minAccel[0], 0.1f, -2.0f, 2.0f);
        ImGui::DragFloat3("Max Acceleration", &smoke->maxAccel[0], 0.1f, -2.0f, 2.0f);
    }

    if(ImGui::CollapsingHeader("Color"))
    {
        ImGui::DragFloat4("Min Color", &smoke->minColor[0], 0.01f, 0.0f, 2.0f);
        ImGui::DragFloat4("Max Color", &smoke->maxColor[0], 0.01f, 0.0f, 2.0f);
        ImGui::DragFloat4("Min Color Velocity", &smoke->minColorVelocity[0], 0.01f, -1.0f, 0.0f);
        ImGui::DragFloat4("Max Color Velocity", &smoke->maxColorVelocity[0], 0.01f, -1.0f, 0.0f);
    }

    int oldNumOfParticles = smoke->maxNumOfParticles;
    if(ImGui::InputInt("Num Of Particles", &smoke->maxNumOfParticles))
    {
        ReallocParticles(game, smoke, oldNumOfParticles);
    }

    ImGui::DragInt("Spawn Rate", &smoke->spawnRate, 1, 0, 1000);

    if(ImGui::Combo("Texture", &game->currentTexture, "smoke\0smoke2\0smoke3\0smoke4\0smoke5\0smoke6\0\0"))
    {
        game->textureID = game->particleTextures[game->currentTexture].id;
    }

    if(ImGui::CollapsingHeader("Velocity Over Lifetime"))
    {
        ImGui::Checkbox("Use Velocity Over Lifetime", &smoke->velocityOverLifetime);

        static int selection = -1;
        ImGui::Curve("Test", ImVec2(300, 300), PARTICLES_MAX_CONTROL_POINTS,
                     smoke->imVelocityControlPoints, &selection, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

        static float x = 0.0f;
        ImGui::InputFloat("X ", &x);
        ImGui::SameLine();
        ImGui::LabelText("Y", "%f", ImGui::CurveValue(x, PARTICLES_MAX_CONTROL_POINTS,
                         smoke->imVelocityControlPoints));
    }

    if(ImGui::CollapsingHeader("Animation"))
    {
        ImGui::Checkbox("Is Animated", &smoke->isAnimated);
        ImGui::InputInt("Frames", &smoke->animationFPS);
    }

    if(ImGui::Button("Save"))
    {
        SaveSettings(*smoke, "PRESET1.json");
    }
    if(ImGui::Button("Load"))
    {
        LoadSettings(*smoke, "PRESET1.json", smoke->atlas);
        ReallocParticles(game, smoke, oldNumOfParticles);
    }

    ImGui::End();
}