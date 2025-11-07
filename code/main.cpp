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


    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    //alBufferData(buffer, AL_FORMAT_STEREO16, output, samplesLoaded * bytesPerStereoSample, sampleRate);
    alBufferData(buffer, AL_FORMAT_MONO16, output, samplesLoaded * channels * sizeof(short), sampleRate);

    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    //alSourcef(source, AL_GAIN, 0.01f);

    ALfloat srcPos[3] = {30.0f, 0.0f, 0.0f};    // 10 units away
    ALfloat srcVel[3] = {0.0f, 0.0f, 0.0f};
    alSourcefv(source, AL_POSITION, srcPos);
    alSourcefv(source, AL_VELOCITY, srcVel);

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    // set reference distance and max distance
    alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);   // no attenuation within 1 unit
    alSourcef(source, AL_MAX_DISTANCE, 50.0f);        // clamp at 50 units

    // optional rolloff factor for faster or slower fade
    alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);

    alSourcePlay(source);

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

    Model car = ImportModel("../data/models/car_scene.obj", shader, CreateTexture("../data/models/car_diffuse.png", 0), 0, aiProcess_Triangulate);
    Entity carEntity = CreateEntity(&car);
    carEntity.position = vec3(0.0f, 1.0f, -3.0f);
    carEntity.rotation = vec3(0.0f, -90.0f, 0.0f);
    //carEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(&carEntity);

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

    Mesh lightMesh = CreateMesh(vertices, sizeof(vertices), lightSourceShader);

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
        UpdateGame(game);

        Camera *camera = &game->camera;
        ALfloat listenerOri[6] = {
            camera->direction.x, camera->direction.y, camera->direction.z,
            camera->up.x, camera->up.y, camera->up.z
        };
        alListener3f(AL_POSITION, camera->position.x, camera->position.y, camera->position.z);
        alListenerfv(AL_ORIENTATION, listenerOri);

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
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

