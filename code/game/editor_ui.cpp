#include "editor_ui.h"

#include "particle_system.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_gradient/imgui_gradient.hpp>

#include <json.hpp>
#include <fstream>

#include <windows.h>
#include <commdlg.h>

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

    for(int i = 0; i < PARTICLES_MAX_CONTROL_POINTS; ++i)
    {
        j["velocityControlPoints"].push_back({settings.velocityControlPoints[i].x,
                                              settings.velocityControlPoints[i].y});
    }

    j["velocityOverLifetime"] = settings.velocityOverLifetime;
    j["colorOverLifetime"] = settings.colorOverLifetime;

    const ImGG::Gradient &gradient = settings.gradientWgt.gradient();

    std::list<ImGG::Mark> marks = gradient.get_marks();
    for(ImGG::Mark mark : marks)
    {
        ImGG::ColorRGBA c = mark.color;
        j["colorControlPoints"].push_back({mark.position.get(), c.x, c.y, c.z, c.w});
    }

    std::ofstream file(filepath);
    file << j.dump(4);
}



void LoadSettings(ParticleSystemSettings& settings, const std::string &filepath, Atlas *atlas)
{
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    json j;
    file >> j;

    ParticleSystemSettings *ds = GetDefaultSettings();

    settings.maxNumOfParticles = j.value("maxNumOfParticles", ds->maxNumOfParticles);
    settings.prewarm = j.value("prewarm", ds->prewarm);
    settings.prewarmSeconds = j.value("prewarmSeconds", ds->prewarmSeconds);

    settings.spawnRate = j.value("spawnRate", ds->spawnRate);
    settings.lifetime = j.value("lifetime", ds->lifetime);
    settings.limitedLife = j.value("limitedLife", ds->limitedLife);

    settings.radius = j.value("radius", ds->radius);

    settings.minRotation = j.value("minRotation", ds->minRotation);
    settings.maxRotation = j.value("maxRotation", ds->maxRotation);
    settings.minRotationSpeed = j.value("minRotationSpeed", ds->minRotationSpeed);
    settings.maxRotationSpeed = j.value("maxRotationSpeed", ds->maxRotationSpeed);

    settings.minScale = j.value("minScale", ds->minScale);
    settings.maxScale = j.value("maxScale", ds->maxScale);
    settings.minScaleVelocity = j.value("minScaleVelocity", ds->minScaleVelocity);
    settings.maxScaleVelocity = j.value("maxScaleVelocity", ds->maxScaleVelocity);

    settings.minOffset = ds->minOffset;
    settings.maxOffset = ds->maxOffset;
    settings.minVelocity = ds->minVelocity;
    settings.maxVelocity = ds->maxVelocity;
    settings.minAccel = ds->minAccel;
    settings.maxAccel = ds->maxAccel;
    settings.minColor = ds->minColor;
    settings.maxColor = ds->maxColor;
    settings.minColorVelocity = ds->minColorVelocity;
    settings.maxColorVelocity = ds->maxColorVelocity;

    if(j.contains("minOffset"))
        settings.minOffset = j.at("minOffset").get<glm::vec3>();
    if(j.contains("maxOffset"))
        settings.maxOffset = j.at("maxOffset").get<glm::vec3>();
    if(j.contains("minVelocity"))
        settings.minVelocity = j.at("minVelocity").get<glm::vec3>();
    if(j.contains("maxVelocity"))
        settings.maxVelocity = j.at("maxVelocity").get<glm::vec3>();
    if(j.contains("minAccel"))
        settings.minAccel = j.at("minAccel").get<glm::vec3>();
    if(j.contains("maxAccel"))
        settings.maxAccel = j.at("maxAccel").get<glm::vec3>();
    if(j.contains("minColor"))
        settings.minColor = j.at("minColor").get<glm::vec4>();
    if(j.contains("maxColor"))
        settings.maxColor = j.at("maxColor").get<glm::vec4>();
    if(j.contains("minColorVelocity"))
        settings.minColorVelocity = j.at("minColorVelocity").get<glm::vec4>();
    if(j.contains("maxColorVelocity"))
        settings.maxColorVelocity = j.at("maxColorVelocity").get<glm::vec4>();

    settings.atlas = atlas;
    settings.isAnimated = j.value("isAnimated", ds->isAnimated);
    settings.animationFPS = j.value("animationFPS", ds->animationFPS);

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

    settings.velocityOverLifetime = j.value("velocityOverLifetime", ds->velocityOverLifetime);
    if(j.count("colorOverLifetime"))
    {
        settings.colorOverLifetime = j.value("colorOverLifetime", ds->colorOverLifetime);
    }

    const ImGG::Gradient &gradient = settings.gradientWgt.gradient();

    std::list<ImGG::Mark> marks;
    if(j.contains("colorControlPoints") && j["colorControlPoints"].is_array())
    {
        const auto &pointsJson = j["colorControlPoints"];
        for(size_t i = 0; i < pointsJson.size(); ++i)
        {
            ImGG::Mark mark;
            mark.position.set(pointsJson[i][0].get<float>());
            mark.color.x = pointsJson[i][1].get<float>();
            mark.color.y = pointsJson[i][2].get<float>();
            mark.color.z = pointsJson[i][3].get<float>();
            mark.color.w = pointsJson[i][4].get<float>();
            marks.push_back(mark);
        }
    }

    settings.gradientWgt = ImGG::GradientWidget(marks);
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

std::string OpenFileDialog()
{
    char fileName[MAX_PATH] = "";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if(GetOpenFileNameA(&ofn))
        return std::string(fileName);
    return "";
}

std::string SaveFileDialog()
{
    char fileName[MAX_PATH] = "";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if(GetSaveFileNameA(&ofn))
        return std::string(fileName);
    return "";
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

    if(ImGui::CollapsingHeader("Color Over Lifetime"))
    {
        ImGui::Checkbox("Use Color Over Lifetime", &smoke->colorOverLifetime);

        ImGui::Spacing();
        ImGui::Spacing();

        smoke->gradientWgt.widget("Color");
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
        std::string path = SaveFileDialog();
        if(!path.empty())
            SaveSettings(*smoke, path);
    }
    if(ImGui::Button("Load"))
    {
        std::string path = OpenFileDialog();
        if(!path.empty())
        {
            LoadSettings(*smoke, path, smoke->atlas);
            ReallocParticles(game, smoke, oldNumOfParticles);
        }
    }

    ImGui::End();
}