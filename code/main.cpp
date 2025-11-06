#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

#include "external/stb_image.cpp"

#include "infantry.cpp"
#include "entity.cpp"
#include "game.cpp"
#include "input.cpp"

#include "util/timer.cpp"

#include "graphics/light.cpp"
#include "graphics/camera.cpp"
#include "graphics/shader.cpp"
#include "graphics/texture.cpp"
#include "graphics/mesh.cpp"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdio.h>
#include <vector>

Model ImportModel(char *filepath, GLuint shader, GLuint diffuseTexture, GLuint specularTexture, uint32 flags = 0)
{
    Model result = {};

    const aiScene *scene = aiImportFile(filepath, flags);
    if(!scene)
    {
        SDL_Log("Failed to load %s. Error: %s", filepath, aiGetErrorString());
        return result;
    }

    result.meshes = (Mesh *)malloc(sizeof(Mesh) * scene->mNumMeshes);
    result.numOfMeshes = scene->mNumMeshes;

    for(uint32 i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[i];
        bool hasUVs = mesh->HasTextureCoords(0);

        std::vector<Vertex> vertices;
        std::vector<uint32> indices;
        for(uint32 j = 0; j < mesh->mNumVertices; j++)
        {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D norm = mesh->mNormals[j];

            Vertex vertex = {};
            vertex.position = vec3(pos.x, pos.y, pos.z);
            vertex.normal = vec3(norm.x, norm.y, norm.z);
            if(hasUVs)
            {
                aiVector3D uv = mesh->mTextureCoords[0][j];
                vertex.uv = vec2(uv.x, uv.y);
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

        result.meshes[i] = CreateMesh(vertices, indices, shader);
        result.meshes[i].material.diffuseTexture = diffuseTexture;
        result.meshes[i].material.specularTexture = specularTexture;
    }

    return result;
}

int main(int argc, char *argv[])
{
    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,     0.0f,  0.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,     0.0f,  0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    };

    GLuint shader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                        LoadShader("../data/shaders/fragment.frag"));

    GLuint lightSourceShader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                                   LoadShader("../data/shaders/fragment2.frag"));

    GLuint backpackDiffuseTexture = CreateTexture("../data/models/backpack/diffuse.jpg", 0);
    GLuint backpackSpecularTexture = CreateTexture("../data/models/backpack/specular.jpg", 1);


    Model test = ImportModel("../data/models/backpack/backpack.obj", shader,
                             backpackDiffuseTexture, backpackSpecularTexture,
                             aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);
    Entity testEntity = CreateEntity(&test);
    testEntity.position = vec3(0.0f, 0.0f, 3.0f);
    testEntity.rotation = vec3(0.0f, 180.0f, 0.0f);
    testEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(&testEntity);

    Model cube2 = ImportModel("../data/models/cube.obj", shader, CreateTexture("../data/models/cube_diffuse.png", 0), 0, aiProcess_Triangulate);
    Entity cubeEntity = CreateEntity(&cube2);
    cubeEntity.position = vec3(1.0f, 0.0f, 3.0f);
    cubeEntity.rotation = vec3(0.0f, 180.0f, 0.0f);
    cubeEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(&cubeEntity);

    Model sphere = ImportModel("../data/models/sphere.obj", shader, CreateTexture("../data/models/sphere_diffuse.png", 0), 0, aiProcess_Triangulate);
    Entity sphereEntity = CreateEntity(&sphere);
    sphereEntity.position = vec3(-1.0f, 0.0f, 3.0f);
    sphereEntity.rotation = vec3(0.0f, -90.0f, 0.0f);
    sphereEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(&sphereEntity);

    Model sphere2 = ImportModel("../data/models/sphere2.obj", shader, CreateTexture("../data/models/sphere2_diffuse.png", 0), 0, aiProcess_Triangulate);
    Entity sphereEntity2 = CreateEntity(&sphere2);
    sphereEntity2.position = vec3(0.0f, 1.0f, 3.0f);
    sphereEntity2.rotation = vec3(0.0f, -90.0f, 0.0f);
    sphereEntity2.scale = vec3(0.2f);
    game->sceneEntities.push_back(&sphereEntity2);

    MaterialPhong containerMaterial = {};
    containerMaterial.diffuseTexture = CreateTexture("../data/imgs/container2.png", 0);
    containerMaterial.specularTexture = CreateTexture("../data/imgs/container2_specular.png", 1);
    //containerMaterial.specularTexture = CreateTexture("../data/imgs/lighting_maps_specular_color.png", 1);
    containerMaterial.emissionTexture = CreateTexture("../data/imgs/matrix.jpg", 2);
    containerMaterial.shininess = 256.0f;

    Mesh cubeMesh = CreateMesh(vertices, sizeof(vertices), shader);
    cubeMesh.material = containerMaterial;
    //cubeMesh.texture = CreateTexture("../data/imgs/container.jpg", 0);

    Entity cube = CreateEntity(&cubeMesh);
    cube.position.x -= 3.0f;
    cube.position.z += 3.0f;
    cube.position.y -= 0.5f;
    game->sceneEntities.push_back(&cube);

    ShaderSetVec2(shader, "u_viewport", WINDOW_WIDTH, WINDOW_HEIGHT);

    vec3 dirDiffuse = vec3(0.9f);
    vec3 dirAmbient = vec3(0.05f);
    vec3 dirSpecular = vec3(1.0f);
    //vec3 dirSpecular = vec3(0.8f, 0.7f, 0.0f);
    DirectionalLight dirLight = CreateDirLight(vec3(1.5f, -1.0f, -0.8f), dirDiffuse, dirAmbient, dirSpecular);
    ShaderSetDirLight(shader, dirLight);
    //ShaderSetInt(shader, "u_dirLightCount", 0);

    Mesh lightMesh = CreateMesh(vertices, sizeof(vertices), lightSourceShader);
    PointLight pointLights[4] = {};
    Entity pointLightsSources[4];
    int width = 10;
    int maxPointLights = 4;
    for(int i = 0; i < maxPointLights; i++)
    {
        pointLights[i].position.x = ((float)SDL_rand(width) - width / 2.0f) * 2;
        pointLights[i].position.y = ((float)SDL_rand(width) - width / 2.0f) * 2;
        pointLights[i].position.z = ((float)SDL_rand(width) - width / 2.0f) * 2;

        pointLights[i].diffuse = vec3(0.5f);
        pointLights[i].ambient = vec3(0.05f);
        pointLights[i].specular = vec3(0.1f);

        pointLights[i].constant = 1.0f;
        pointLights[i].linear = 0.027f;
        pointLights[i].quadratic = 0.0028f;

        ShaderSetPointLight(shader, pointLights[i], i);

        pointLightsSources[i] = CreateEntity(&lightMesh);
        pointLightsSources[i].scale = vec3(0.2f);
        pointLightsSources[i].position = pointLights[i].position;

        game->sceneEntities.push_back(&pointLightsSources[i]);
    }

    ShaderSetInt(shader, "u_pointLightCount", maxPointLights);
    ShaderSetVec3(lightSourceShader, "u_lightColor", vec3(1.0f));

    Mesh soldierMeshes[2];
    soldierMeshes[0] = CreateMesh(vertices, sizeof(vertices), shader);
    soldierMeshes[0].material = containerMaterial;

    soldierMeshes[1] = CreateMesh(vertices, sizeof(vertices), shader);
    soldierMeshes[1].material = containerMaterial;

    for(int i = 0; i < 2; i++)
    {
        InfantrySquad *squad = (InfantrySquad *)malloc(sizeof(InfantrySquad));
        *squad = CreateInfantrySquad(&soldierMeshes[i], 1, 10);
        squad->position.z = -10.0f / 2.0f;
        squad->position.x = -10.0f / 2.0f;
        squad->position.y -= i;

        game->sceneEntities.push_back(squad);
    }

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        testEntity.rotation.y = (float)SDL_GetTicks() / 25.0f;
        UpdateGame(game);

        //Rendering
        glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ShaderSetVec3(shader, "u_viewPos", game->camera.position);
        ShaderSetVec3(shader, "u_viewDir", game->camera.direction);
        ShaderSetFloat(shader, "u_time", SDL_GetTicks() / 1000.0f);

        for(int i = 0; i < game->sceneEntities.size(); i++)
        {
            Entity *e = game->sceneEntities[i];
            RenderEntityFunc *Render = e->Render;
            Render(e, game);
            //e->Render(e, game);
        }

        SDL_GL_SwapWindow(game->window);

        //Lock FPS and deltaTime calculation
        Uint64 thisFrame = SDL_GetPerformanceCounter();
        if(game->lockFPS)
        {
            int targetFrames = 60;
            float targetTime = 1.0f / targetFrames;
            float elapsedWhileWaiting = 0.0f;
            while((elapsedWhileWaiting = (SDL_GetPerformanceCounter() - thisFrame) / (float)game->perfFreq) < targetTime)
            {
                SDL_Delay((uint32)((targetTime - elapsedWhileWaiting) * 1000));
            }

            thisFrame = SDL_GetPerformanceCounter();
        }

        game->deltaTime = (thisFrame - game->lastFrame) / (float)game->perfFreq;
        game->lastFrame = thisFrame;
    }

    return 0;
}

