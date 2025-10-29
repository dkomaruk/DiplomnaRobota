#include "external/stb_image.cpp"

#include "infantry.cpp"
#include "entity.cpp"
#include "game.cpp"
#include "input.cpp"

#include "util/timer.cpp"

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

    Mesh cubeMesh = CreateMesh(vertices, sizeof(vertices), shader);
    cubeMesh.texture = CreateTexture("../data/imgs/container.jpg", 0);
    cubeMesh.material.ambient = vec3(1.0f, 0.5f, 0.31f);
    cubeMesh.material.diffuse = vec3(1.0f, 0.5f, 0.31f);
    cubeMesh.material.specular = vec3(0.5f);
    cubeMesh.material.shininess = 32.0f;

    MaterialPhong whiteMaterial = {vec3(1.0f), vec3(1.0f), vec3(1.0f), 32.0f};

    MaterialPhong emeraldMaterial = {};
    emeraldMaterial.ambient = vec3(0.0215f, 0.1745f, 0.0215f);
    emeraldMaterial.diffuse = vec3(0.07568f, 0.61424f, 0.07568f);
    emeraldMaterial.specular = vec3(0.633f, 0.727811f, 0.633f);
    emeraldMaterial.shininess = (int)(0.6f * 128);

    MaterialPhong redPlasticMaterial = {};

    redPlasticMaterial.ambient =  vec3(0.1f, 0.0f, 0.0f);
    redPlasticMaterial.diffuse =  vec3(0.5f, 0.0f, 0.0f);
    redPlasticMaterial.specular = vec3(0.7f, 0.6f, 0.6f);
    redPlasticMaterial.shininess = (int)(0.25f * 128);

    //cubeMesh.material = emeraldMaterial;
    //cubeMesh.material = redPlasticMaterial;
    cubeMesh.material = whiteMaterial;

    //float ambientIntensity = (0.212671f * cubeMesh.material.ambient.r + 0.715160f * cubeMesh.material.ambient.g +
    //                          0.072169f * cubeMesh.material.ambient.b) / (0.212671f * cubeMesh.material.diffuse.r +
    //                          0.715160f * cubeMesh.material.diffuse.g + 0.072169f * cubeMesh.material.diffuse.b);

    Entity cube = CreateEntity(&cubeMesh);

    Mesh lightMesh = CreateMesh(vertices, sizeof(vertices), lightSourceShader);
    Entity lightSource = CreateEntity(&lightMesh);
    lightSource.scale = vec3(0.2f);
    lightSource.position = vec3(1.2f, 1.0f, 2.0f);

    ShaderSetVec3(cube.mesh->shader, "u_light.specular", vec3(1.0f));

    Mesh soldierMeshes[2];
    soldierMeshes[0] = CreateMesh(vertices, sizeof(vertices), shader);
    soldierMeshes[0].material = emeraldMaterial;

    soldierMeshes[1] = CreateMesh(vertices, sizeof(vertices), shader);
    soldierMeshes[1].material = redPlasticMaterial;

    for(int i = 0; i < 2; i++)
    {
        InfantrySquad *squad = (InfantrySquad *)malloc(sizeof(InfantrySquad));
        *squad = CreateInfantrySquad(&soldierMeshes[i], 1, 10);
        squad->position.z = 3.0f * i * ((i % 2 == 0) ? -1 : 1);

        game.sceneEntities.push_back(squad);
    }

    while(game.isRunning)
    {
        //Input
        ProcessInput(&game);

        //Update
        UpdateGame(&game);

        //float time = SDL_GetTicks() / 1000.0f;
        //lightSource.position.x = sin(time);
        //lightSource.position.z = cos(time);
        //lightSource.position.y = cos(time);

        //Rendering
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vec3 lightColor = vec3(1.0f);
        //lightColor.r = sin(SDL_GetTicks() / 1000.0f * 2.0f);
        //lightColor.g = sin(SDL_GetTicks() / 1000.0f * 0.7f);
        //lightColor.b = sin(SDL_GetTicks() / 1000.0f * 1.3f);

        //vec3 lightDiffuse = lightColor * vec3(0.5f);
        vec3 lightDiffuse = lightColor;

        ShaderSetVec3(cube.mesh->shader, "u_light.diffuse", lightDiffuse);
        ShaderSetVec3(cube.mesh->shader, "u_light.ambient", lightDiffuse * vec3(0.2f));

        //ShaderSetVec3(cube.mesh.shader, "u_light.diffuse", vec3(1.0f));
        //ShaderSetVec3(cube.mesh.shader, "u_light.ambient", vec3(ambientIntensity));
        //ShaderSetVec3(cube.mesh.shader, "u_light.ambient", vec3(1.0f));

        ShaderSetVec3(cube.mesh->shader, "u_viewPos", game.camera.position);
        //ShaderSetVec3(cube.mesh.shader, "u_lightPos", vec3(game.view * vec4(lightSource.position, 1.0f)));
        ShaderSetVec3(shader, "u_light.position", lightSource.position);

        ShaderSetVec3(lightSourceShader, "u_lightColor", lightColor);

        RenderEntity(&game, &cube);
        RenderEntity(&game, &lightSource);

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

