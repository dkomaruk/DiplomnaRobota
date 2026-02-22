#include "editor_ui.h"

#include "particle_editor_ui.h"
#include "file.h"
#include "model.h"
#include "noise.h"
#include "shader.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_gradient/imgui_gradient.hpp>

void UpdateEditorUI(Game *game)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("Example1"))
        {
            if(ImGui::MenuItem("Example1 Option 1")) {}
            if(ImGui::MenuItem("Example1 Option 2", "Shortcut example")) {SDL_Log("Option2 selected");}
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Example2"))
        {
            if(ImGui::MenuItem("Example2 Option2")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             (game->input.isCursorHidden ? ImGuiWindowFlags_NoInputs : 0);

    ImGui::Begin("Value Noise", 0, flags | ImGuiWindowFlags_HorizontalScrollbar);
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

    ImGui::Begin("Perlin Noise", 0, flags | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Image(game->perlinNoise.id, ImVec2(256.0f, 256.0f));

    int changed = 0;

    static glm::ivec2 gridSize = glm::ivec2(32);
    changed += ImGui::InputInt2("Octaves", &gridSize[0]);

    static int octaves = 1;
    changed += ImGui::InputInt("Octaves", &octaves);

    static float persistence = 0.5f;
    changed += ImGui::InputFloat("Persistence", &persistence, 0.05f);

    static float lacunarity = 2.0f;
    changed += ImGui::InputFloat("Lacunarity", &lacunarity, 0.05f);

    if(ImGui::Button("Generate") || changed)
    {
        glDeleteTextures(1, &game->perlinNoise.id);
        glm::vec2 size = glm::vec2(1024.0f, 1024.0f);
        uint8 *perlinNoise = GeneratePerlinNoise(size, gridSize, octaves, persistence, lacunarity);
        game->perlinNoise = CreateGLTexture(perlinNoise, (int)size.x, (int)size.y);

        DeleteMesh(&game->terrain.mesh);
        free(game->terrain.heightmap);

        float yScale = 20.0f / 255.0f;
        float *heightmap = GetHeightmapData(perlinNoise, 4, size, size, yScale, 22.0f);
        Terrain terrain = CreateTerrain(heightmap, size, yScale, 1.0f, 0.1f, 4, 0.0f);

        terrain.splatMap = game->terrain.splatMap;
        terrain.texture0 = game->terrain.texture0;
        terrain.texture1 = game->terrain.texture1;
        terrain.texture2 = game->terrain.texture2;
        terrain.texture3 = game->terrain.texture3;
        game->terrain = terrain;

        free(perlinNoise);
    }
    ImGui::End();

    ImGui::Begin("Perlin Noise2", 0, flags | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Image(game->perlinNoise2.id, ImVec2((float)game->perlinNoise2.x, (float)game->perlinNoise2.y));

    if(ImGui::Button("Generate"))
    {
        glDeleteTextures(1, &game->perlinNoise2.id);
        glm::vec2 size = glm::vec2(256.0f, 256.0f);
        uint8 *perlinNoise = GeneratePerlinNoise(size, (uint8)5);
        game->perlinNoise2 = CreateGLTexture(perlinNoise, (int)size.x, (int)size.y);
        free(perlinNoise);
    }
    ImGui::End();

    ImGui::Begin("Debug Settings", 0, flags);

    ImGui::Checkbox("Display Entity AABB", &game->renderAABB);
    ImGui::Checkbox("Display Picking Ray", &game->renderPickingRay);
    ImGui::Checkbox("Display Selection Frustum", &game->renderSelectionFrustum);
    ImGui::Checkbox("Display Terrain", &game->renderTerrain);

    if(ImGui::Checkbox("Display Particles", &game->renderParticles))
    {
        ShaderSetInt(game->postProcessShader, "u_showParticles", game->renderParticles);
    }

    ImGui::End();

    //ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 420.0f, 0.0f), ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(420.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
    UpdateParticleEditorUI(game, flags);

    ImGui::Begin("Lighting settings", 0, flags);

    int lightingChanged = 0;
    lightingChanged += ImGui::DragFloat3("Ambient", &game->dirLight.ambient[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Diffuse", &game->dirLight.diffuse[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Specular", &game->dirLight.specular[0], 0.05f);
    lightingChanged += ImGui::DragFloat3("Direction", &game->dirLight.direction[0], 0.05f);

    if(lightingChanged)
    {
        ShaderSetDirLight(game->mainShader, game->dirLight);
        ShaderSetDirLight(game->animationShader, game->dirLight);
    }

    ImGui::End();

    ImGui::SetNextWindowSizeConstraints(ImVec2(200, 20), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Selected Entity", 0, flags);
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

    ImGui::Begin("Import Model", 0, flags);

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