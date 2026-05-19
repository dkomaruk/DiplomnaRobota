#include "editor.h"

#include "particle_editor.h"

#include "game.h"
#include "asset_manager.h"
#include "noise.h"
#include "shader.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_gradient/imgui_gradient.hpp>

#include <mutex>
#include <queue>
#include <string>

void UpdateMenuBar(Game *game)
{
    Editor *editor = &game->editor;

    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
        if(ImGui::BeginMenu("Edit")) { ImGui::EndMenu(); }

        if(ImGui::BeginMenu("Windows"))
        {
            if(ImGui::MenuItem("Particle Editor", "1")) editor->particleEditorWindow = true;
            if(ImGui::MenuItem("Terrain Generator", "2")) editor->terrainGeneratorWindow = true;
            if(ImGui::MenuItem("Selected Entity", "3")) editor->selectedEntityWindow = true;
            if(ImGui::MenuItem("Debug Settings", "4")) editor->debugSettingsWindow = true;
            if(ImGui::MenuItem("Lighting Settings", "5")) editor->lightingSettingsWindow = true;
            if(ImGui::MenuItem("Import Model", "6")) editor->importModelWindow = true;
            if(ImGui::MenuItem("Value Noise", "7")) editor->valueNoiseWindow = true;
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help")) { ImGui::EndMenu(); }

        ImGui::EndMainMenuBar();
    }
}

void UpdateDebugSettings(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Debug Settings", &game->editor.debugSettingsWindow, flags);

    ImGui::Checkbox("Display Entity AABB", &game->renderAABB);
    ImGui::Checkbox("Display Picking Ray", &game->renderPickingRay);
    ImGui::Checkbox("Display Selection Frustum", &game->renderSelectionFrustum);
    ImGui::Checkbox("Display Terrain", &game->renderTerrain);
    ImGui::Checkbox("Display Counters", &game->renderCounters);

    if(ImGui::Checkbox("Display Particles", &game->renderParticles))
    {
        ShaderSetInt(GetShader(game, "post_process"), "u_showParticles", game->renderParticles);
    }

    ImGui::InputFloat("Camera Speed", &game->camera.speed, 0.05f);
    ImGui::InputFloat("Camera Sensitivity", &game->camera.sensitivity, 0.05f);

    ImGui::End();
}

void UpdateValueNoise(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Value Noise", &game->editor.valueNoiseWindow, flags | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Image(game->valueNoise.id, ImVec2((float)game->valueNoise.x, (float)game->valueNoise.y));

    if(ImGui::Button("Generate"))
    {
        glDeleteTextures(1, &game->valueNoise.id);
        glm::vec2 size = glm::vec2(256.0f, 256.0f);
        u8 *valueNoise = GenerateValueNoise(size);
        game->valueNoise = CreateGLTexture(valueNoise, (int)size.x, (int)size.y);
        free(valueNoise);
    }

    ImGui::End();
}

void UpdateSceneLight(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Lighting settings", &game->editor.lightingSettingsWindow, flags);

    int lightingChanged = 0;
    lightingChanged += ImGui::DragFloat3("Ambient", &game->dirLight.ambient[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Diffuse", &game->dirLight.diffuse[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Specular", &game->dirLight.specular[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Direction", &game->dirLight.direction[0], 0.05f);
    //lightingChanged += ImGui::DragFloat3("Direction", &game->dirLight.direction[0], 0.001f);

    if(lightingChanged)
    {
        ShaderSetDirLight(GetShader(game, "main"), game->dirLight);
        ShaderSetDirLight(GetShader(game, "animation"), game->dirLight);
        ShaderSetDirLight(GetShader(game, "terrain"), game->dirLight);
        ShaderSetDirLight(GetShader(game, "tessellated_terrain"), game->dirLight);
    }

    ImGui::End();
}

void UpdateSelectedEntity(Game *game, ImGuiWindowFlags flags)
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(200, 20), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Selected Entity", &game->editor.selectedEntityWindow, flags);
    if(game->lastSelectedId > 0)
    {
        int id = game->lastSelectedId;
        auto it = std::find_if(game->sceneEntities.begin(), game->sceneEntities.end(), [id](Entity *entity) {
            return entity->id == id;
        });

        if(it != game->sceneEntities.end())
        {
            Entity *selectedEntity = *it;
            ImGui::LabelText("Text ID", "%s", selectedEntity->textId);

            if(ImGui::CollapsingHeader("Transform"))
            {
                ImGui::DragFloat3("Position", &selectedEntity->position[0], 0.1f);
                if(ImGui::DragFloat3("Rotation", &selectedEntity->rotation[0], 0.1f))
                {
                    glm::mat4 modelMat = PrepareModelMatrix(glm::vec3(0.0f), selectedEntity->rotation, glm::vec3(1.0f));
                    selectedEntity->aabb = TransformAABB(&selectedEntity->model->aabb, modelMat);
                    UpdateAABBMesh(&selectedEntity->aabb, &selectedEntity->meshAABB, true);
                }
                ImGui::DragFloat3("Scale", &selectedEntity->scale[0], 0.1f);
            }

            if(ImGui::CollapsingHeader("Material"))
            {
                ImGui::LabelText("Shader ID", "%d", selectedEntity->model->material->shader);
                ImGui::LabelText("Diffuse Texture ID", "%d", selectedEntity->model->material->diffuseTexture.id);
                ImGui::LabelText("Specular Texture ID", "%d", selectedEntity->model->material->specularTexture.id);
            }
        }
    }

    ImGui::End();
}

struct ImportCallbackData
{
    std::mutex mutex;
    std::queue<std::string> queue;
};

void ImportModelCallback(void *userdata, const char * const *filelist, int filter)
{
    if(filelist && *filelist)
    {
        ImportCallbackData *importData = (ImportCallbackData *)userdata;

        importData->mutex.lock();
        importData->queue.push(filelist[0]);
        importData->mutex.unlock();
    }
}

void UpdateImportModel(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Import Model", &game->editor.importModelWindow, flags);

    ImGui::InputFloat("Import Scale", &game->editor.importScale);

    static ImportCallbackData importData = {};

    if(importData.mutex.try_lock())
    {
        while(importData.queue.size())
        {
            std::string& path = importData.queue.front();
            std::string modelName = RegisterAsset(&game->assets, path);
            Model *model = LoadModel(&game->assets, (char *)path.c_str(), modelName, 0,
                                     aiProcess_Triangulate | aiProcess_GlobalScale,
                                     ModelType_DetermineOnLoad, game->editor.importScale);

            if(model->type == ModelType_Static)
                model->material->shader = GetShader(game, "main");
            else if(model->type == ModelType_Animated)
                model->material->shader = GetShader(game, "animation");
            else
                InvalidCodepath

            Entity *entity = (Entity *)calloc(1, sizeof(Entity));
            *entity = CreateEntity(model);

            entity->id = game->sceneEntities.back()->id + 1;
            game->sceneEntities.push_back(entity);

            importData.queue.pop();
        }

        importData.mutex.unlock();
    }

    if(ImGui::Button("Import"))
    {
        static SDL_DialogFileFilter filters[] = {{"FBX Files", "fbx"}, {"GLB Files", "glb"}, {"OBJ Files", "obj"}};
        SDL_ShowOpenFileDialog(ImportModelCallback, (void *)&importData, game->window, filters, 3, 0, false);
    }

    ImGui::End();
}

void UpdateEditor(Game *game)
{
    Input *input = &game->input;
    Editor *editor = &game->editor;

    ImGuiIO &io = ImGui::GetIO();
    if(!io.WantCaptureKeyboard)
    {
        if(IsFirstPress(input, SDL_SCANCODE_1)) editor->particleEditorWindow = !editor->particleEditorWindow;
        if(IsFirstPress(input, SDL_SCANCODE_2)) editor->terrainGeneratorWindow = !editor->terrainGeneratorWindow;
        if(IsFirstPress(input, SDL_SCANCODE_3)) editor->selectedEntityWindow = !editor->selectedEntityWindow;
        if(IsFirstPress(input, SDL_SCANCODE_4)) editor->debugSettingsWindow = !editor->debugSettingsWindow;
        if(IsFirstPress(input, SDL_SCANCODE_5)) editor->lightingSettingsWindow = !editor->lightingSettingsWindow;
        if(IsFirstPress(input, SDL_SCANCODE_6)) editor->importModelWindow = !editor->importModelWindow;
        if(IsFirstPress(input, SDL_SCANCODE_7)) editor->valueNoiseWindow = !editor->valueNoiseWindow;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    UpdateMenuBar(game);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             (game->input.isCursorHidden ? ImGuiWindowFlags_NoInputs : 0);

    if(editor->valueNoiseWindow)
    {
        UpdateValueNoise(game, flags);
    }
    if(editor->terrainGeneratorWindow)
    {
        UpdateTerrainEditor(game, flags);
    }
    if(editor->debugSettingsWindow)
    {
        UpdateDebugSettings(game, flags);
    }
    if(editor->particleEditorWindow)
    {
        UpdateParticleEditorUI(game, &editor->particleEditorWindow, flags);
    }
    if(editor->lightingSettingsWindow)
    {
        UpdateSceneLight(game, flags);
    }
    if(editor->selectedEntityWindow)
    {
        UpdateSelectedEntity(game, flags);
    }
    if(editor->importModelWindow)
    {
        UpdateImportModel(game, flags);
    }
}