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

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <vector>

using namespace glm;

int main(int argc, char *argv[])
{
    Game game = {};
    if(!InitGame(&game))
    {
        return -1;
    }

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 1.0f,   1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 1.0f,   1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,   0.0f, 0.0f,   1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,   1.0f, 1.0f,   0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f
    };

    GLuint shader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                        LoadShader("../data/shaders/fragment.frag"));

    GLuint lightSourceShader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                                   LoadShader("../data/shaders/fragment2.frag"));

    MaterialPhong containerMaterial = {CreateTexture("../data/imgs/container2.png", 0),
                                       CreateTexture("../data/imgs/container2_specular.png", 1),
                                       CreateTexture("../data/imgs/matrix.jpg", 2), 32.0f};

    Mesh cubeMesh = CreateMesh(vertices, sizeof(vertices), shader);
    cubeMesh.material = containerMaterial;
    //cubeMesh.texture = CreateTexture("../data/imgs/container.jpg", 0);

    Entity cube = CreateEntity(&cubeMesh);
    cube.position.x -= 3.0f;
    cube.position.z += 3.0f;
    cube.position.y -= 0.5f;
    game.sceneEntities.push_back(&cube);

    ShaderSetVec2(shader, "u_viewport", WINDOW_WIDTH, WINDOW_HEIGHT);

    DirectionalLight dirLight = CreateDirLight(vec3(1.5f, -1.0f, -0.8f), vec3(0.5f), vec3(0.05f), vec3(0.5f, 0.5f, 0.5f));
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
        pointLights[i].specular = vec3(0.5f);

        //pointLights[i].constant = 1.0f;
        //pointLights[i].linear = 0.07f;
        //pointLights[i].quadratic = 0.017f;

        pointLights[i].constant = 1.0f;
        pointLights[i].linear = 0.027f;
        pointLights[i].quadratic = 0.0028f;

        ShaderSetPointLight(shader, pointLights[i], i);

        pointLightsSources[i] = CreateEntity(&lightMesh);
        pointLightsSources[i].scale = vec3(0.2f);
        pointLightsSources[i].position = pointLights[i].position;

        game.sceneEntities.push_back(&pointLightsSources[i]);
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

        game.sceneEntities.push_back(squad);
    }

    GLuint flashlightTexture = CreateTexture("../data/imgs/flashlightpattern.png", 3, false);
    ShaderSetInt(shader, "u_flashlightTexture", 3);
    SetTexture(flashlightTexture, 3);

    while(game.isRunning)
    {
        //Input
        ProcessInput(&game);

        //Update
        UpdateGame(&game);

        //Rendering
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ShaderSetVec3(cube.mesh->shader, "u_viewPos", game.camera.position);
        ShaderSetVec3(cube.mesh->shader, "u_viewDir", game.camera.direction);
        ShaderSetFloat(cube.mesh->shader, "u_time", SDL_GetTicks() / 1000.0f);


        for(int i = 0; i < game.sceneEntities.size(); i++)
        {
            Entity *e = game.sceneEntities[i];
            e->Render(e, &game);
        }

        SDL_GL_SwapWindow(game.window);

        //Lock FPS and deltaTime calculation
        Uint64 thisFrame = SDL_GetPerformanceCounter();
        if(game.lockFPS)
        {
            int targetFrames = 60;
            float targetTime = 1.0f / targetFrames;
            float elapsedWhileWaiting = 0.0f;
            while((elapsedWhileWaiting = (SDL_GetPerformanceCounter() - thisFrame) / (float)game.perfFreq) < targetTime)
            {
                SDL_Delay((uint32)((targetTime - elapsedWhileWaiting) * 1000));
            }

            thisFrame = SDL_GetPerformanceCounter();
        }

        game.deltaTime = (thisFrame - game.lastFrame) / (float)game.perfFreq;
        game.lastFrame = thisFrame;
    }

    return 0;
}

