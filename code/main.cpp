#include "external/stb_image.cpp"

#include "entity.cpp"
#include "engine.cpp"
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
    Engine engine = {};
    if(!InitEngine(&engine))
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

    Entity cube = CreateEntity(cubeMesh);

    Mesh lightMesh = CreateMesh(vertices, sizeof(vertices), lightSourceShader);
    Entity lightSource = CreateEntity(lightMesh);
    lightSource.scale = vec3(0.2f);
    lightSource.position = vec3(1.2f, 1.0f, 2.0f);

    ShaderSetVec3(cube.mesh.shader, "u_light.specular", vec3(1.0f));

    while(engine.isRunning)
    {
        ProcessInput(&engine);
        UpdateEngine(&engine);

        //float time = SDL_GetTicks() / 1000.0f;
        //lightSource.position.x = sin(time);
        //lightSource.position.z = cos(time);
        //lightSource.position.y = cos(time);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Rendering
        vec3 lightColor;
        lightColor.r = sin(SDL_GetTicks() / 1000.0f * 2.0f);
        lightColor.g = sin(SDL_GetTicks() / 1000.0f * 0.7f);
        lightColor.b = sin(SDL_GetTicks() / 1000.0f * 1.3f);

        vec3 lightDiffuse = lightColor * vec3(0.5f);

        ShaderSetVec3(cube.mesh.shader, "u_light.diffuse", lightDiffuse);
        ShaderSetVec3(cube.mesh.shader, "u_light.ambient", lightDiffuse * vec3(0.2f));

        //ShaderSetVec3(cube.mesh.shader, "u_light.diffuse", vec3(1.0f));
        //ShaderSetVec3(cube.mesh.shader, "u_light.ambient", vec3(ambientIntensity));
        //ShaderSetVec3(cube.mesh.shader, "u_light.ambient", vec3(1.0f));

        ShaderSetVec3(cube.mesh.shader, "u_viewPos", engine.camera.position);
        //ShaderSetVec3(cube.mesh.shader, "u_lightPos", vec3(engine.view * vec4(lightSource.position, 1.0f)));
        ShaderSetVec3(shader, "u_light.position", lightSource.position);

        ShaderSetVec3(lightSourceShader, "u_lightColor", lightColor);

        RenderEntity(&engine, &cube);
        RenderEntity(&engine, &lightSource);

        SDL_GL_SwapWindow(engine.window);

        //Lock FPS and deltaTime calculation
        Uint64 thisFrame = SDL_GetPerformanceCounter();
        if(engine.lockFPS)
        {
            int targetFrames = 60;
            float targetTime = 1.0f / targetFrames;
            float elapsedWhileWaiting = 0.0f;
            while((elapsedWhileWaiting = (SDL_GetPerformanceCounter() - thisFrame) / (float)engine.perfFreq) < targetTime)
            {
                SDL_Delay((uint32)((targetTime - elapsedWhileWaiting) * 1000));
            }

            thisFrame = SDL_GetPerformanceCounter();
        }

        engine.deltaTime = (thisFrame - engine.lastFrame) / (float)engine.perfFreq;
        engine.lastFrame = thisFrame;
    }

    return 0;
}

