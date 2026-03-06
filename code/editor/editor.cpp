#include "editor.h"

#include "particle_editor.h"
#include "terrain_editor.h"

#include "game.h"
#include "file.h"
#include "model.h"
#include "noise.h"
#include "shader.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_gradient/imgui_gradient.hpp>

void UpdateMenuBar(Game *game)
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
        if(ImGui::BeginMenu("Edit")) { ImGui::EndMenu(); }

        if(ImGui::BeginMenu("Windows"))
        {
            if(ImGui::MenuItem("Particle Editor", "1")) game->particleEditorWindow = true;
            if(ImGui::MenuItem("Terrain Generator", "2")) game->terrainGeneratorWindow = true;
            if(ImGui::MenuItem("Selected Entity", "3")) game->selectedEntityWindow = true;
            if(ImGui::MenuItem("Debug Settings", "4")) game->debugSettingsWindow = true;
            if(ImGui::MenuItem("Lighting Settings", "5")) game->lightingSettingsWindow = true;
            if(ImGui::MenuItem("Import Model", "6")) game->importModelWindow = true;
            if(ImGui::MenuItem("Value Noise", "7")) game->valueNoiseWindow = true;
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help")) { ImGui::EndMenu(); }

        ImGui::EndMainMenuBar();
    }
}

void UpdateDebugSettings(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Debug Settings", &game->debugSettingsWindow, flags);

    ImGui::Checkbox("Display Entity AABB", &game->renderAABB);
    ImGui::Checkbox("Display Picking Ray", &game->renderPickingRay);
    ImGui::Checkbox("Display Selection Frustum", &game->renderSelectionFrustum);
    ImGui::Checkbox("Display Terrain", &game->renderTerrain);
    ImGui::Checkbox("Display Counters", &game->renderCounters);

    if(ImGui::Checkbox("Display Particles", &game->renderParticles))
    {
        ShaderSetInt(game->postProcessShader, "u_showParticles", game->renderParticles);
    }

    ImGui::InputFloat("Camera Speed", &game->camera.speed, 0.05f);
    ImGui::InputFloat("Camera Sensitivity", &game->camera.sensitivity, 0.05f);

    ImGui::End();
}

void UpdateValueNoise(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Value Noise", &game->valueNoiseWindow, flags | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Image(game->valueNoise.id, ImVec2((float)game->valueNoise.x, (float)game->valueNoise.y));

    if(ImGui::Button("Generate"))
    {
        glDeleteTextures(1, &game->valueNoise.id);
        glm::vec2 size = glm::vec2(256.0f, 256.0f);
        uint8 *valueNoise = GenerateValueNoise(size);
        game->valueNoise = CreateGLTexture(valueNoise, (int)size.x, (int)size.y);
        free(valueNoise);
    }

    ImGui::End();
}

void UpdateSceneLight(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Lighting settings", &game->lightingSettingsWindow, flags);

    int lightingChanged = 0;
    lightingChanged += ImGui::DragFloat3("Ambient", &game->dirLight.ambient[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Diffuse", &game->dirLight.diffuse[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Specular", &game->dirLight.specular[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Direction", &game->dirLight.direction[0], 0.05f);

    if(lightingChanged)
    {
        ShaderSetDirLight(game->mainShader, game->dirLight);
        ShaderSetDirLight(game->animationShader, game->dirLight);
        ShaderSetDirLight(game->terrainShader, game->dirLight);
    }

    ImGui::End();
}

void UpdateSelectedEntity(Game *game, ImGuiWindowFlags flags)
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(200, 20), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Selected Entity", &game->selectedEntityWindow, flags);
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
                    UpdateAABBCorners(&selectedEntity->aabb);
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

void UpdateImportModel(Game *game, ImGuiWindowFlags flags)
{
    ImGui::Begin("Import Model", &game->importModelWindow, flags);

    static float importScale = 1.0f;
    ImGui::InputFloat("Import Scale", &importScale);

    if(ImGui::Button("Import"))
    {
        FileFilter filters[] = {{"FBX Files", "*.fbx"}, {"GLB Files", "*.glb"}, {"OBJ Files", "*.obj"}};
        std::string path = OpenFileDialog(filters, ArrayCount(filters));
        if(!path.empty())
        {
            Model *model = ImportModel((char *)path.c_str(), 0, aiProcess_Triangulate | aiProcess_GlobalScale,
                                        ModelType_DetermineOnLoad, importScale);

            if(model->type == ModelType_Static)
                model->material->shader = game->mainShader;
            else if(model->type == ModelType_Animated)
                model->material->shader = game->animationShader;
            else
                InvalidCodepath

            Entity *entity = (Entity *)calloc(1, sizeof(Entity));
            *entity = CreateEntity(model);

            entity->id = game->sceneEntities.back()->id + 1;
            game->sceneEntities.push_back(entity);

            game->lastFrame = SDL_GetPerformanceCounter();
        }
    }

    ImGui::End();
}

void UpdateEditorUI(Game *game)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    UpdateMenuBar(game);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             (game->input.isCursorHidden ? ImGuiWindowFlags_NoInputs : 0);

    if(game->valueNoiseWindow)
    {
        UpdateValueNoise(game, flags);
    }
    if(game->terrainGeneratorWindow)
    {
        UpdateTerrainEditorUI(game, &game->terrainGeneratorWindow, flags);
    }
    if(game->debugSettingsWindow)
    {
        UpdateDebugSettings(game, flags);
    }
    if(game->particleEditorWindow)
    {
        UpdateParticleEditorUI(game, &game->particleEditorWindow, flags);
    }
    if(game->lightingSettingsWindow)
    {
        UpdateSceneLight(game, flags);
    }
    if(game->selectedEntityWindow)
    {
        UpdateSelectedEntity(game, flags);
    }
    if(game->importModelWindow)
    {
        UpdateImportModel(game, flags);
    }
}