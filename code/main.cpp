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
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    GLuint shaderProgram = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                               LoadShader("../data/shaders/fragment.frag"));

    Mesh cube = CreateMesh(vertices, sizeof(vertices), shaderProgram);
    cube.texture = CreateTexture("../data/imgs/container.jpg", 0);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_projection"), 1, GL_FALSE, value_ptr(engine.projection));

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    Entity cubes[10] = {};
    for(int i = 0; i < 10; i++)
    {
        cubes[i].position = cubePositions[i];
        cubes[i].mesh = cube;
    }

    std::vector<vec3> clearColors = {
        vec3(144.0f, 213.0f, 255.0f), vec3(173.0f, 216.0f, 230.0f), vec3(152.0f, 255.0f, 152.0f),
        vec3(245.0f, 245.0f, 220.0f), vec3(204.0f, 153.0f, 153.0f), vec3(255.0f, 218.0f, 185.0f),
        vec3(159.0f, 226.0f, 191.0f), vec3(230.0f, 230.0f, 250.0f), vec3(211.0f, 211.0f, 211.0f),
        vec3(150.0f, 120.0f, 96.0f), vec3(200.0f, 162.0f, 200.0f),
    };

    int clearColorIndex = SDL_rand((int)clearColors.size());

    float colorTimerDuration = 5.0f * 60.0f;
    Timer colorTimer = StartTimer(colorTimerDuration);

    while(engine.isRunning)
    {
        ProcessInput(&engine);
        UpdateEngine(&engine);

        UpdateTimer(&colorTimer, engine.deltaTime);
        if(colorTimer.isFinished)
        {
            int lastIndex = clearColorIndex;
            while(lastIndex == clearColorIndex)
            {
                clearColorIndex = SDL_rand((int)clearColors.size());
            }

            colorTimer = StartTimer(colorTimerDuration);
        }

#ifndef WINDOW_TRANSPARENT
        glClearColor(clearColors[clearColorIndex].r / 255.0f,
                     clearColors[clearColorIndex].g / 255.0f,
                     clearColors[clearColorIndex].b / 255.0f, 0.0f);
#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = SDL_GetTicks() / 1000.0f;
        for(int i = 0; i < 10; i++)
        {
            cubes[i].position = cubePositions[i] + vec3(0.0f, sin(time) * 0.1f * i * sign(sin(time) * i), (sin(time * 0.6f) * 2.0f - 2.0f));
            cubes[i].rotation = glm::vec3(20.0f * i, ((i != 0) ? (time * 15.0f) : 0.0f), 0.0f);

            RenderEntity(&engine, &cubes[i]);
        }

        SDL_GL_SwapWindow(engine.window);

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

