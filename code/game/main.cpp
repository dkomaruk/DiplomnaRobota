#define GLM_ENABLE_EXPERIMENTAL
#define IMGUI_DEFINE_MATH_OPERATORS

//TODO: Replace with SDL_ShowOpenFileDialog
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

//Has to be included at the start because of compilation errors otherwise
//TODO: Replace with a less heavy library
#include <json.hpp>

//GLM extensions have to be included before any other glm header file for some reason
#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>

#include "stb_image.cpp"
#include "stb_image_write.cpp"

#include "input.cpp"

#include "infantry.cpp"
#include "entity.cpp"
#include "game.cpp"

#include "audio.cpp"
#include "asset_loader.cpp"

#include "timer.cpp"
#include "text.cpp"
#include "text_demo.cpp"

#include "light.cpp"
#include "camera.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "image.cpp"
#include "mesh.cpp"
#include "particle_system.cpp"
#include "particle_editor_ui.cpp"
#include "terrain.cpp"
#include "framebuffer.cpp"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <time.h>

#include <imgui.h>

#include <expat.h>

aiNode *FindNode(aiNode *node, char *name)
{
    if(!node || (strcmp(name, node->mName.C_Str()) == 0))
    {
        return node;
    }

    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        aiNode *result = FindNode(node->mChildren[i], name);
        if(result)
        {
            return result;
        }
    }

    return NULL;
}

struct SceneImporter
{
    aiScene *scene;
    std::string dirPath;
    std::vector<std::string> loadedDiffusePaths, loadedSpecularPaths;
    std::vector<Texture> diffuseTextures, specularTextures;
    std::vector<Mesh> meshes;
    std::vector<MaterialPhong> materials;
    GLuint shader;
};

void ProcessNode(aiNode *node, SceneImporter *importData, aiMatrix4x4 modelMat)
{
    if(!node) return;
    SceneImporter *imp = importData;

    aiMatrix3x3 normalMat;
    aiMatrix3FromMatrix4(&normalMat, &modelMat);
    aiMatrix3Inverse(&normalMat);
    aiTransposeMatrix3(&normalMat);

    for(uint32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh *mesh = imp->scene->mMeshes[node->mMeshes[i]];
        bool hasUVs = mesh->HasTextureCoords(0);

        std::vector<Vertex> vertices;
        std::vector<uint32> indices;
        for(uint32 j = 0; j < mesh->mNumVertices; j++)
        {
            aiVector3D pos = modelMat * mesh->mVertices[j];

            //glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

            aiVector3D norm = normalMat * mesh->mNormals[j];
            norm.Normalize();

            Vertex vertex = {};
            vertex.position = glm::vec3(pos.x, pos.y, pos.z);
            vertex.normal = glm::vec3(norm.x, norm.y, norm.z);
            if(hasUVs)
            {
                aiVector3D uv = mesh->mTextureCoords[0][j];
                vertex.uv = glm::vec2(uv.x, uv.y);
            }

            vertices.push_back(vertex);
        }

        for(uint32 j = 0; j < mesh->mNumFaces; j++)
        {
            for(uint32 k = 0; k < mesh->mFaces[j].mNumIndices; k++)
            {
                indices.push_back(mesh->mFaces[j].mIndices[k]);
            }
        }

        aiMaterial *material = imp->scene->mMaterials[mesh->mMaterialIndex];

        Texture diffuseTexture = LoadTextures(imp->scene, material, imp->dirPath, &imp->loadedDiffusePaths, &imp->diffuseTextures, aiTextureType_DIFFUSE);
        Texture specularTexture = LoadTextures(imp->scene, material, imp->dirPath, &imp->loadedSpecularPaths, &imp->specularTextures, aiTextureType_SPECULAR);

        imp->meshes.push_back(CreateMesh(vertices, indices));
        imp->materials.push_back({imp->shader, diffuseTexture, specularTexture, {}, 32.0f});
    }

    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], imp, modelMat * node->mChildren[i]->mTransformation);
    }
}

Model *ImportModel2(char *filepath, GLuint shader)
{
    Model *result = (Model *)malloc(sizeof(Model));
    result->numOfMeshes = -1;

    const aiScene *scene = aiImportFile(filepath, 0);
    if(!scene)
    {
        SDL_Log("Failed to load %s. Error: %s", filepath, aiGetErrorString());
        return result;
    }

    std::string dirPath = filepath;
    size_t found = dirPath.find_last_of("\\/");
    dirPath = (found == std::string::npos) ? dirPath : dirPath.substr(0, found);

    SceneImporter imp = {};
    if(strcmp(filepath, "../data/models/abrams/abrams.fbx") == 0)
    {
        aiNode *turret = FindNode(scene->mRootNode, "Tourelle_01");
        if(turret)
        {
            imp.scene = (aiScene *)scene;
            imp.dirPath = dirPath;
            imp.shader = shader;

            aiMatrix4x4 modelMat = turret->mTransformation;
            aiMatrix4RotationX(&modelMat, glm::radians(-90.0f));
            ProcessNode(turret, &imp, modelMat);
        }
    }

    result->mesh = (Mesh *)malloc(sizeof(Mesh) * imp.meshes.size());
    result->material = (MaterialPhong *)malloc(sizeof(MaterialPhong) * imp.meshes.size());
    result->numOfMeshes = (int)imp.meshes.size();

    std::copy(imp.meshes.begin(), imp.meshes.end(), result->mesh);
    std::copy(imp.materials.begin(), imp.materials.end(), result->material);

    return result;
}

int main(int argc, char *argv[])
{
    srand((uint32)time(0));

    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    LoadAssets(game);

    std::vector<Vertex> lineVertices = {
        Vertex{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
    };

    Mesh line = CreateMesh(lineVertices);
    MaterialPhong lineMat = {};
    lineMat.shader = game->lightSourceShader;

    game->soldierEntity->position.x = 0.0f;
    game->soldierEntity->position.z = 0.0f;

    glm::vec2 target = {0.0f, 0.0f};
    glm::vec2 targetDirection = {0.0f, 0.0f};

    {
        //Need to do this before the game loop because calling glReadPixels for the first time
        //for some reason causes a huge freeze (174 ms unoptimized compilation). Subsequent calls are 10-12 ms
        //Maybe I should just do ray picking instead of using a framebuffer with IDs
        glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo.id);
        uint8 pixels[3];
        glReadPixels((int)0, (int)0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Model *abramsTurret = ImportModel2("../data/models//abrams.fbx", game->mainShader);
    //Model *abrams = ImportModel("../data/models/abrams/abrams.fbx", game->mainShader, aiProcess_PreTransformVertices);
    if(!abramsTurret)
    {
        SDL_Log("Failed to load abrams.fbx");
    }

    Entity tankTurret = CreateEntity(abramsTurret);
    tankTurret.position.y += 5.0f;
    tankTurret.id = game->sceneEntities.back()->id + 1;
    game->sceneEntities.push_back(&tankTurret);

    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 angularVelocity = glm::vec3(0.0f);

    game->lastFrame = SDL_GetPerformanceCounter();
    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateParticleEditorUI(game);
        UpdateGame(game);

        tankTurret.position += velocity * game->deltaTime + 0.5f * acceleration * Square(game->deltaTime);
        tankTurret.rotation += angularVelocity * game->deltaTime;

        if(IsFirstPress(game, SDL_SCANCODE_R))
        {
            velocity = glm::vec3(RandomBetween(-2.0f, 2.0f), 25.0f, RandomBetween(-2.0f, 2.0f));
            acceleration.y = -9.8f;
            angularVelocity = glm::vec3(RandomBetween(-180.0f, 180.0f),
                                        RandomBetween(-180.0f, 180.0f),
                                        RandomBetween(-180.0f, 180.0f));
        }

        angularVelocity -= angularVelocity * 0.3f * game->deltaTime;
        velocity += acceleration * game->deltaTime;

        float x = game->soldierEntity->position.x + targetDirection.x * 10.0f * game->deltaTime;
        float z = game->soldierEntity->position.z + targetDirection.y * 10.0f * game->deltaTime;
        float y = GetTerrainHeight(&game->terrain, x, z);

        game->soldierEntity->position = glm::vec3(x, y, z);

        if(glm::distance(glm::vec2(x, z), target) < 0.1f)
        {
            targetDirection = glm::vec2(0, 0);
        }

        //Rendering
        if(!game->textDemoEnabled)
        {
            glEnable(GL_DEPTH_TEST);

            game->pickingPass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo.id);
            RenderScene(game);
            game->pickingPass = false;

            if(IsFirstClick(game, MOUSE_LEFT))
            {
                float x, y;
                if(game->isCursorHidden)
                {
                    x = WINDOW_WIDTH / 2.0f;
                    y = WINDOW_HEIGHT / 2.0f;
                }
                else
                {
                    SDL_GetMouseState(&x, &y);
                    y = (int)WINDOW_HEIGHT - y;
                }

                uint64 start = SDL_GetPerformanceCounter();
                uint8 pixels[3];
                //This is very expensive, I think AABB ray intersection will be much more cheap
                glReadPixels((int)x, (int)y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
                uint32 pickedID = pixels[0];
                uint64 end = SDL_GetPerformanceCounter();
                SDL_Log("%f ms", ((end - start) / (float)game->perfFreq) * 1000.0f);

                if(!pickedID || !game->keys[SDL_SCANCODE_LSHIFT])
                {
                    game->selectedIDs.clear();
                }

                bool isAlreadyPicked = game->selectedIDs.count(pickedID);
                if(isAlreadyPicked && game->keys[SDL_SCANCODE_LSHIFT])
                {
                    game->selectedIDs.erase(pickedID);
                }
                else
                {
                    game->selectedIDs.insert(pickedID);
                }

                glm::vec3 windowPos = glm::vec3(x, y, 0.0f);
                glm::vec3 rayNear = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                                   glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));
                windowPos.z = 1.0f;
                glm::vec3 rayFar = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                                  glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));

                glm::vec3 rayDirection = glm::normalize(rayFar - rayNear);
                glm::vec3 rayOrigin = game->camera.position;

                lineVertices[0].position = glm::vec3(rayOrigin);
                lineVertices[1].position = glm::vec3(rayOrigin + rayDirection * 200.0f);
                UpdateMesh(&line, lineVertices);

                glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, rayOrigin, rayDirection, 200.0f);

                target = glm::vec2(intersectionPoint.x, intersectionPoint.z);
                targetDirection = glm::normalize(target - glm::vec2(game->soldierEntity->position.x, game->soldierEntity->position.z));
            }

            game->outlinePass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineFbo.color.id, 0);
            RenderScene(game);
            game->outlinePass = false;

            glDepthMask(GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
            RenderScene(game);

            RenderModel(game, abramsTurret, PrepareModelMatrix(tankTurret.position, tankTurret.rotation, tankTurret.scale));

            static GLenum terrainDisplayMode = GL_FILL;
            if(IsFirstPress(game, SDL_SCANCODE_SPACE))
            {
                terrainDisplayMode = (terrainDisplayMode == GL_LINE) ? GL_FILL : GL_LINE;
            }

            glPolygonMode(GL_FRONT_AND_BACK, terrainDisplayMode);
            RenderTerrain(game);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            ShaderSetVec3(game->lightSourceShader, "u_lightColor", 1.0f, 0.0f, 0.0f);
            RenderMesh(game, &line, &lineMat, glm::mat4(1.0f), GL_LINES);

            if(game->renderParticles)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, game->smokeFbo.id);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glViewport(0, 0, game->smokeFbo.color.x, game->smokeFbo.color.y);
                SetTexture(&game->fullSceneDepthTexture, 2);
                ShaderSetInt(game->particleShader, "u_sceneDepth", 2);
                ShaderSetVec2(game->particleShader, "u_screenSize", game->smokeFbo.color.size);
                RenderParticles(game);
                glViewport(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            SetTexture(&game->outlineFbo.color, 0);
            ShaderSetInt(game->postProcessShader, "u_outline", 0);
            SetTexture(&game->fullSceneTexture, 1);
            ShaderSetInt(game->postProcessShader, "u_scene", 1);
            SetTexture(&game->smokeFbo.color, 2);
            ShaderSetInt(game->postProcessShader, "u_smoke", 2);
            SetTexture(&game->fullSceneDepthTexture, 3);
            ShaderSetInt(game->postProcessShader, "u_sceneDepth", 3);
            SetTexture(&game->smokeFbo.depth, 4);
            ShaderSetInt(game->postProcessShader, "u_smokeDepth", 4);

            ShaderSetVec2(game->postProcessShader, "u_lowResInvSize", 1.0f / (glm::vec2)game->smokeFbo.color.size);

            glBindVertexArray(game->fullscreenQuad.vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            RenderText(&game->aliveParticlesText);
            RenderText(&game->deadParticlesText);
            RenderText(&game->fpsCounter);
            RenderText(&game->msPerFrame);

            glDisable(GL_BLEND);
        }
        else
        {
            RenderTextDemo(game);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(game->window);

        Uint64 thisFrame = SDL_GetPerformanceCounter();
        if(game->lockFPS)
        {
            int targetFrames = 20;
            float targetTime = 1.0f / targetFrames;
            float elapsedWhileWaiting = 0.0f;
            while((elapsedWhileWaiting = (SDL_GetPerformanceCounter() - thisFrame) / (float)game->perfFreq) < targetTime)
            {
                SDL_Delay((uint32)((targetTime - elapsedWhileWaiting) * 1000));
            }

            thisFrame = SDL_GetPerformanceCounter();
        }

        game->deltaTime = (thisFrame - game->lastFrame) / (float)game->perfFreq;

        float ms = game->deltaTime * 1000.0f;
        float fps = 1000.0f / ms;
        //SDL_Log("%f", fps);

        char buffer[20];
        sprintf(buffer, "%.5f FPS", fps);

        UpdateText(&game->fpsCounter, buffer);

        sprintf(buffer, "%.5f ms/f", ms);
        UpdateText(&game->msPerFrame, buffer);

        game->lastFrame = thisFrame;
    }

    SDL_Log("Exited the main loop\n");


    return 0;
}

