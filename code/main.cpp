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
#include "audio.cpp"

#include "util/timer.cpp"

#include "graphics/light.cpp"
#include "graphics/camera.cpp"
#include "graphics/shader.cpp"
#include "graphics/texture.cpp"
#include "graphics/mesh.cpp"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include "AL/al.h"
#include "AL/alext.h"

#include "stb_vorbis.c"

#include <stdio.h>
#include <vector>

int main(int argc, char *argv[])
{
    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    int channels, sampleRate;
    int bytesPerStereoSample = 4;
    short *output, *output2;
    int samplesLoaded = stb_vorbis_decode_filename("../data/audio/test_sample.ogg", &channels, &sampleRate, &output);

    if (channels != 1)
    {
        int monoSamples = samplesLoaded;
        short* mono = (short*)malloc(monoSamples * sizeof(short));
        for (int i = 0; i < monoSamples; i++) {
            int left  = output[2*i];
            int right = output[2*i + 1];
            mono[i] = (short)((left + right) / 2);
        }
        free(output);
        output = mono;
        channels = 1;
        samplesLoaded = monoSamples;
    }

    SDL_Log("channels: %d, sampleRate: %d, samplesLoaded: %d\n", channels, sampleRate, samplesLoaded);

    //alDistanceModel(AL_INVERSE_DISTANCE);
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, output, samplesLoaded * channels * sizeof(short), sampleRate);

    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, 0.5f);

    ALfloat srcPos[3] = {30.0f, 0.0f, 0.0f};
    alSourcefv(source, AL_POSITION, srcPos);
    alSourcef(source, AL_MAX_DISTANCE, 20.0f);

    alSourcePlay(source);

    GLuint shader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                        LoadShader("../data/shaders/fragment.frag"));

    GLuint lightSourceShader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                                   LoadShader("../data/shaders/fragment2.frag"));

    game->shaders.push_back(shader);
    game->shaders.push_back(lightSourceShader);
    game->outlineShader = lightSourceShader;

    Model soldier = ImportModel("../data/models/soldier/soldier.obj", shader, aiProcess_Triangulate);
    //Model soldier = ImportModel("../data/models/soldier/soldier.glb", shader, aiProcess_Triangulate);
    if(soldier.numOfMeshes != -1)
    {
        Entity soldierEntity = CreateEntity(&soldier);
        soldierEntity.position.y += 0.5f;
        game->sceneEntities.push_back(&soldierEntity);
    }

    Model test = ImportModel("../data/models/backpack/backpack.obj", shader,
                             aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    Entity testEntity = CreateEntity(&test);
    testEntity.position = vec3(0.0f, 0.0f, 3.0f);
    testEntity.rotation = vec3(0.0f, 180.0f, 0.0f);
    testEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(&testEntity);

    Model sphere = ImportModel("../data/models/sphere.obj", shader, aiProcess_Triangulate);
    sphere.meshes->material.diffuseTexture = CreateTexture("../data/models/sphere_diffuse.png");

    Entity sphereEntity = CreateEntity(&sphere);
    sphereEntity.position = vec3(-1.0f, 0.0f, 3.0f);
    sphereEntity.rotation = vec3(0.0f, -90.0f, 0.0f);
    sphereEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(&sphereEntity);

    Model sphere2 = ImportModel("../data/models/sphere2.obj", shader, aiProcess_Triangulate);
    sphere2.meshes->material.diffuseTexture = CreateTexture("../data/models/sphere2_diffuse.png");

    Entity sphereEntity2 = CreateEntity(&sphere2);
    sphereEntity2.position = vec3(0.0f, 1.0f, 3.0f);
    sphereEntity2.rotation = vec3(0.0f, -90.0f, 0.0f);
    sphereEntity2.scale = vec3(0.2f);
    game->sceneEntities.push_back(&sphereEntity2);

    Model car = ImportModel("../data/models/car_scene.obj", shader, aiProcess_Triangulate);
    GLuint carDiffuseTexture = CreateTexture("../data/models/car_diffuse.png");
    for(int i = 0; i < car.numOfMeshes; i++)
    {
        car.meshes[i].material.diffuseTexture = carDiffuseTexture;
    }

    Entity carEntity = CreateEntity(&car);
    carEntity.position = vec3(0.0f, 1.0f, -3.0f);
    carEntity.rotation = vec3(0.0f, -90.0f, 0.0f);
    //carEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(&carEntity);

    MaterialPhong containerMaterial = {};
    containerMaterial.diffuseTexture = CreateTexture("../data/imgs/container2.png");
    containerMaterial.specularTexture = CreateTexture("../data/imgs/container2_specular.png");
    //containerMaterial.specularTexture = CreateTexture("../data/imgs/lighting_maps_specular_color.png");
    containerMaterial.emissionTexture = CreateTexture("../data/imgs/matrix.jpg");
    containerMaterial.shininess = 256.0f;

    Model cubeMesh = ImportModel("../data/models/cube.obj", shader, aiProcess_Triangulate);
    cubeMesh.meshes[0].material = containerMaterial;

    Entity cubeEntity = CreateEntity(&cubeMesh);
    cubeEntity.position.x -= 3.0f;
    cubeEntity.position.z += 3.0f;
    cubeEntity.position.y -= 0.5f;
    cubeEntity.scale -= vec3(0.5f);
    game->sceneEntities.push_back(&cubeEntity);

    Entity cubeEntity2 = CreateEntity(&cubeMesh);
    cubeEntity2.position = vec3(1.0f, 0.0f, 3.0f);
    cubeEntity2.rotation = vec3(0.0f, 180.0f, 0.0f);
    cubeEntity2.scale = vec3(0.2f);
    game->sceneEntities.push_back(&cubeEntity2);

    ShaderSetVec2(shader, "u_viewport", WINDOW_WIDTH, WINDOW_HEIGHT);

    Model lightMesh = ImportModel("../data/models/cube.obj", lightSourceShader, aiProcess_Triangulate);

    vec3 dirDiffuse = vec3(0.9f);
    vec3 dirAmbient = vec3(0.05f);
    vec3 dirSpecular = vec3(1.0f);
    //vec3 dirSpecular = vec3(0.8f, 0.7f, 0.0f);
    DirectionalLight dirLight = CreateDirLight(vec3(1.5f, -1.0f, -0.8f), dirDiffuse, dirAmbient, dirSpecular);
    ShaderSetDirLight(shader, dirLight);
    //ShaderSetInt(shader, "u_dirLightCount", 0);
    Entity dirLightMesh = CreateEntity(&lightMesh);
    dirLightMesh.scale = vec3(50.0f);
    dirLightMesh.position = -dirLight.direction * 200.0f;

    game->sceneEntities.push_back(&dirLightMesh);

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
        pointLightsSources[i].scale = vec3(0.15f);
        pointLightsSources[i].position = pointLights[i].position;

        game->sceneEntities.push_back(&pointLightsSources[i]);
    }

    ShaderSetInt(shader, "u_pointLightCount", maxPointLights);
    ShaderSetVec3(lightSourceShader, "u_lightColor", vec3(1.0f));

    for(int i = 0; i < 2; i++)
    {
        InfantrySquad *squad = (InfantrySquad *)malloc(sizeof(InfantrySquad));
        *squad = CreateInfantrySquad(&cubeMesh, 1, 10);
        squad->scale = vec3(0.5f);
        squad->position.z = -10.0f / 2.0f;
        squad->position.x = -10.0f / 2.0f;
        squad->position.y -= i;

        game->sceneEntities.push_back(squad);
    }

    //Framebuffer
    //https://www.reddit.com/r/GraphicsProgramming/comments/jwkpju/what_is_the_best_way_to_approach_a_multi_pass/
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint fboTexture;
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    GLuint fboTexture2;
    glGenTextures(1, &fboTexture2);
    glBindTexture(GL_TEXTURE_2D, fboTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture2, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("Framebuffer is complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(0 * sizeof(float)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    GLuint outlineShader = CreateShaderProgram(LoadShader("../data/shaders/vertex3.vert"),
                                               LoadShader("../data/shaders/fragment3.frag"));


    float outlineThickness = 2.0f;
    ShaderSetInt(outlineShader, "u_outlineThickness", (int)outlineThickness);

    ShaderSetInt(outlineShader, "u_inverted", 0);
    ShaderSetInt(outlineShader, "u_grayscale", 0);
    ShaderSetInt(outlineShader, "u_showOutline", 0);

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateGame(game);

        Camera *camera = &game->camera;
        vec3 forward = normalize(camera->direction);

        vec3 worldUp = vec3(0.0f, 1.0f, 0.0f);
        vec3 right = normalize(cross(forward, worldUp));
        vec3 up = normalize(cross(right, forward));

        ALfloat listenerOri[6] = {
            camera->direction.x, camera->direction.y, camera->direction.z,
            up.x, up.y, up.z
        };
        alListener3f(AL_POSITION, camera->position.x, camera->position.y, camera->position.z);
        alListenerfv(AL_ORIENTATION, listenerOri);

        if(game->keys[SDL_SCANCODE_DOWN])
        {
            outlineThickness -= 5.0f * game->deltaTime;
            ShaderSetInt(outlineShader, "u_outlineThickness", (int)outlineThickness);
        }
        if(game->keys[SDL_SCANCODE_UP])
        {
            outlineThickness += 5.0f * game->deltaTime;
            ShaderSetInt(outlineShader, "u_outlineThickness", (int)outlineThickness);
        }

        if(game->keys[SDL_SCANCODE_SPACE] && !game->prevKeys[SDL_SCANCODE_SPACE])
        {
            int sourceState;
            alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
            if(sourceState == AL_PLAYING)
            {
                alSourcePause(source);
            }
            else
            {
                alSourcePlay(source);
            }
        }

        testEntity.rotation.y = (float)SDL_GetTicks() / 25.0f;

        //Rendering
        game->outlinePass = true;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
        glEnable(GL_DEPTH_TEST);
        RenderScene(game);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture2, 0);
        game->outlinePass = false;
        RenderScene(game);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(outlineShader);
        ShaderSetFloat(outlineShader, "u_time", (float)SDL_GetTicks() / 1000.0f);

        SetTexture(fboTexture, 0);
        ShaderSetInt(outlineShader, "u_outline", 0);
        SetTexture(fboTexture2, 1);
        ShaderSetInt(outlineShader, "u_scene", 1);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(game->window);

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

