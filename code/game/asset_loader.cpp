#include "asset_loader.h"

#include "game.h"
#include "texture.h"
#include "shader.h"
#include "infantry.h"

#include <SDL3_ttf/SDL_ttf.h>

#include "AL/al.h"
#include "AL/alext.h"

#include "stb_vorbis.c"

void LoadAudio(Game *game)
{
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

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, output, samplesLoaded * channels * sizeof(short), sampleRate);

    alGenSources(1, &game->source);
    alSourcei(game->source, AL_BUFFER, buffer);
    alSourcef(game->source, AL_GAIN, 0.5f);

    ALfloat srcPos[3] = {30.0f, 0.0f, 0.0f};
    alSourcefv(game->source, AL_POSITION, srcPos);
    alSourcef(game->source, AL_MAX_DISTANCE, 20.0f);

    //alSourcePlay(game->source);
}

//TODO: Move this somewhere else (doesn't belong in the asset loader)
void SetupFramebuffers(Game *game)
{
    //Framebuffer
    //https://www.reddit.com/r/GraphicsProgramming/comments/jwkpju/what_is_the_best_way_to_approach_a_multi_pass/

    glGenFramebuffers(1, &game->pickingFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo);

    glGenTextures(1, &game->pickingTexture.id);
    glBindTexture(GL_TEXTURE_2D, game->pickingTexture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->pickingTexture.id, 0);

    GLuint pickingRbo;
    glGenRenderbuffers(1, &pickingRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, pickingRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pickingRbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("Picking framebuffer is complete");
    }

    //TODO: Multiple render targets, render picking and outline textures using one framebuffer and one render pass
    glGenFramebuffers(1, &game->outlineFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo);

    glGenTextures(1, &game->outlineTexture.id);
    glBindTexture(GL_TEXTURE_2D, game->outlineTexture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineTexture.id, 0);

    glGenTextures(1, &game->fullSceneTexture.id);
    glBindTexture(GL_TEXTURE_2D, game->fullSceneTexture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("Outline/scene framebuffer is complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LoadAssets(Game *game)
{
    //AUDIO
    LoadAudio(game);

    //TODO: Remove from asset loader
    SetupFramebuffers(game);

    //FONTS
    int fontSizes[] = {4, 12, 18, 24, 36, 48};
    int numOfFonts = sizeof(fontSizes) / sizeof(int);

    for(int i = 0; i < numOfFonts; i++)
    {
        int fontSize = fontSizes[i];

        game->fonts[fontSize] = PrepareFont("../data/fonts/Roboto-Regular.ttf", fontSize);
        if(!game->fonts[fontSize].ttfFont)
        {
            SDL_Log("Failed to load Roboto-Regular.ttf font%d. Error: %s", fontSize, SDL_GetError());
        }
    }

    //SHADERS
    GLuint shader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                        LoadShader("../data/shaders/fragment.frag"));
    GLuint lightSourceShader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                                   LoadShader("../data/shaders/fragment2.frag"));
    GLuint uiTextShader = CreateShaderProgram(LoadShader("../data/shaders/uiText.vert"),
                                              LoadShader("../data/shaders/uiText.frag"));
    GLuint pickingShader = CreateShaderProgram(LoadShader("../data/shaders/picking.vert"),
                                               LoadShader("../data/shaders/picking.frag"));
    GLuint postProcessShader = CreateShaderProgram(LoadShader("../data/shaders/vertex3.vert"),
                                                   LoadShader("../data/shaders/fragment3.frag"));
    GLuint particleShader = CreateShaderProgram(LoadShader("../data/shaders/particle.vert"),
                                                   LoadShader("../data/shaders/particle.frag"));

    ShaderSetVec2(shader, "u_viewport", WINDOW_WIDTH, WINDOW_HEIGHT);

    ShaderSetVec3(lightSourceShader, "u_lightColor", glm::vec3(1.0f));

    ShaderSetInt(postProcessShader, "u_outlineThickness", (int)game->outlineThickness);
    ShaderSetInt(postProcessShader, "u_inverted", 0);
    ShaderSetInt(postProcessShader, "u_grayscale", 0);
    ShaderSetInt(postProcessShader, "u_showOutline", 1);

    game->shaders.push_back(shader);
    game->shaders.push_back(lightSourceShader);
    game->shaders.push_back(pickingShader);
    game->shaders.push_back(postProcessShader);

    for(int i = 0; i < game->shaders.size(); i++)
    {
        ShaderSetMatrix4(game->shaders[i], "u_projection", game->perspectiveProjection);
    }

    game->shaders.push_back(uiTextShader);
    ShaderSetMatrix4(uiTextShader, "u_projection", game->orthoProjection);

    game->shaders.push_back(particleShader);
    ShaderSetMatrix4(particleShader, "u_projection", game->perspectiveProjection);

    game->mainShader = shader;
    game->postProcessShader = postProcessShader;
    game->outlineShader = lightSourceShader;
    game->lightSourceShader = lightSourceShader;
    game->pickingShader = pickingShader;
    game->uiTextShader = uiTextShader;
    game->particleShader = particleShader;

    //MESHES
#ifdef LOAD_ASSETS
    Model *soldier = ImportModel("../data/models/soldier/soldier.obj", game->mainShader, aiProcess_Triangulate);
    //Model *soldier = ImportModel("../data/models/soldier/soldier.glb", game->mainShader, aiProcess_Triangulate);
    if(soldier->numOfMeshes != -1)
    {
        Entity *soldierEntity = (Entity *)malloc(sizeof(Entity));
        *soldierEntity = CreateEntity(soldier);
        strcpy(soldierEntity->textId, "soldier");
        soldierEntity->position.y += 0.5f;
        game->sceneEntities.push_back(soldierEntity);
    }

    Model *test = ImportModel("../data/models/backpack/backpack.obj", game->mainShader,
                             aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    Entity *testEntity = (Entity *)malloc(sizeof(Entity));
    *testEntity = CreateEntity(test);
    strcpy(testEntity->textId, "backpack");
    testEntity->position = glm::vec3(0.0f, 0.0f, 3.0f);
    testEntity->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    testEntity->scale = glm::vec3(0.2f);
    game->sceneEntities.push_back(testEntity);
    game->testEntity = testEntity;

    Model *sphere = ImportModel("../data/models/sphere.obj", game->mainShader, aiProcess_Triangulate);
    sphere->material->diffuseTexture = CreateTexture("../data/models/sphere_diffuse.png");

    Entity *sphereEntity = (Entity *)malloc(sizeof(Entity));
    *sphereEntity = CreateEntity(sphere);
    strcpy(sphereEntity->textId, "sphere");
    sphereEntity->position = glm::vec3(-1.0f, 0.0f, 3.0f);
    sphereEntity->rotation = glm::vec3(0.0f, -90.0f, 0.0f);
    sphereEntity->scale = glm::vec3(0.2f);
    game->sceneEntities.push_back(sphereEntity);

    Model *sphere2 = ImportModel("../data/models/sphere2.obj", game->mainShader, aiProcess_Triangulate);
    sphere2->material->diffuseTexture = CreateTexture("../data/models/sphere2_diffuse.png");

    Entity *sphereEntity2 = (Entity *)malloc(sizeof(Entity));
    *sphereEntity2 = CreateEntity(sphere2);
    strcpy(sphereEntity2->textId, "sphere2");
    sphereEntity2->position = glm::vec3(0.0f, 1.0f, 3.0f);
    sphereEntity2->rotation = glm::vec3(0.0f, -90.0f, 0.0f);
    sphereEntity2->scale = glm::vec3(0.2f);
    game->sceneEntities.push_back(sphereEntity2);

    Model *car = ImportModel("../data/models/car_scene.obj", game->mainShader, aiProcess_Triangulate);
    Texture carDiffuseTexture = CreateTexture("../data/models/car_diffuse.png");
    for(int i = 0; i < car->numOfMeshes; i++)
    {
        car->material[i].diffuseTexture = carDiffuseTexture;
    }

    Entity *carEntity = (Entity *)malloc(sizeof(Entity));
    *carEntity = CreateEntity(car);
    strcpy(carEntity->textId, "car");
    carEntity->position = glm::vec3(0.0f, 1.0f, -3.0f);
    carEntity->rotation = glm::vec3(0.0f, -90.0f, 0.0f);
    //carEntity.scale = glm::vec3(0.2f);
    game->sceneEntities.push_back(carEntity);

    MaterialPhong containerMaterial = {};
    containerMaterial.shader = game->mainShader;
    containerMaterial.diffuseTexture = CreateTexture("../data/imgs/container2.png");
    containerMaterial.specularTexture = CreateTexture("../data/imgs/container2_specular.png");
    //containerMaterial.specularTexture = CreateTexture("../data/imgs/lighting_maps_specular_color.png");
    containerMaterial.emissionTexture = CreateTexture("../data/imgs/matrix.jpg");
    containerMaterial.shininess = 256.0f;

    Model *cubeMesh = ImportModel("../data/models/cube.obj", game->mainShader, aiProcess_Triangulate);
    cubeMesh->material[0] = containerMaterial;

    Entity *cubeEntity = (Entity *)malloc(sizeof(Entity));
    *cubeEntity = CreateEntity(cubeMesh);
    strcpy(cubeEntity->textId, "cubeContainer");
    cubeEntity->position.x -= 3.0f;
    cubeEntity->position.z += 3.0f;
    cubeEntity->position.y -= 0.5f;
    cubeEntity->scale -= glm::vec3(0.5f);
    game->sceneEntities.push_back(cubeEntity);

    Entity *cubeEntity2 = (Entity *)malloc(sizeof(Entity));
    *cubeEntity2 = CreateEntity(cubeMesh);
    strcpy(cubeEntity2->textId, "cubeContainer2");
    cubeEntity2->position = glm::vec3(1.0f, 0.0f, 3.0f);
    cubeEntity2->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    cubeEntity2->scale = glm::vec3(0.2f);
    game->sceneEntities.push_back(cubeEntity2);

    Model *lightMesh = ImportModel("../data/models/cube.obj", lightSourceShader, aiProcess_Triangulate);

    glm::vec3 dirDiffuse = glm::vec3(0.9f);
    glm::vec3 dirAmbient = glm::vec3(0.05f);
    glm::vec3 dirSpecular = glm::vec3(1.0f);
    //glm::vec3 dirSpecular = glm::vec3(0.8f, 0.7f, 0.0f);
    DirectionalLight dirLight = CreateDirLight(glm::vec3(1.5f, -1.0f, -0.8f), dirDiffuse, dirAmbient, dirSpecular);
    ShaderSetDirLight(game->mainShader, dirLight);
    //ShaderSetInt(game->mainShader, "u_dirLightCount", 0);
    Entity *dirLightMesh = (Entity *)malloc(sizeof(Entity));
    *dirLightMesh = CreateEntity(lightMesh);
    strcpy(dirLightMesh->textId, "dirLightCube");
    dirLightMesh->scale = glm::vec3(50.0f);
    dirLightMesh->position = -dirLight.direction * 200.0f;

    game->sceneEntities.push_back(dirLightMesh);

    PointLight pointLights[4] = {};
    Entity *pointLightsSources = (Entity *)malloc(sizeof(Entity) * 4);
    int width = 10;
    int maxPointLights = 4;
    for(int i = 0; i < maxPointLights; i++)
    {
        pointLights[i].position.x = ((float)SDL_rand(width) - width / 2.0f) * 2;
        pointLights[i].position.y = ((float)SDL_rand(width) - width / 2.0f) * 2;
        pointLights[i].position.z = ((float)SDL_rand(width) - width / 2.0f) * 2;

        pointLights[i].diffuse = glm::vec3(0.5f);
        pointLights[i].ambient = glm::vec3(0.05f);
        pointLights[i].specular = glm::vec3(0.1f);

        pointLights[i].constant = 1.0f;
        pointLights[i].linear = 0.027f;
        pointLights[i].quadratic = 0.0028f;

        ShaderSetPointLight(game->mainShader, pointLights[i], i);

        pointLightsSources[i] = CreateEntity(lightMesh);
        pointLightsSources[i].scale = glm::vec3(0.15f);
        pointLightsSources[i].position = pointLights[i].position;

        game->sceneEntities.push_back(&pointLightsSources[i]);
    }

    ShaderSetInt(game->mainShader, "u_pointLightCount", maxPointLights);

    for(int i = 0; i < 2; i++)
    {
        InfantrySquad *squad = (InfantrySquad *)malloc(sizeof(InfantrySquad));
        *squad = CreateInfantrySquad(cubeMesh, 1, 10);
        //*squad = CreateInfantrySquad(soldier, 1, 10);
        squad->scale = glm::vec3(0.5f);
        squad->position.z = -10.0f / 2.0f;
        squad->position.x = -10.0f / 2.0f;
        squad->position.y -= i;

        game->sceneEntities.push_back(squad);
    }

    for(uint16 i = 0; i < game->sceneEntities.size(); i++)
    {
        game->sceneEntities[i]->id = i + 1;
    }
#endif

    game->fpsCounter = CreateText(&game->fonts[18], "0 FPS", glm::vec2(20.0f, 36.0f), game->uiTextShader);
    game->msPerFrame = CreateText(&game->fonts[18], "0 ms/f", glm::vec2(180.0f, 36.0f), game->uiTextShader);
    game->aliveParticlesText = CreateText(&game->fonts[18], "Alive Particles: 0", glm::vec2(20.0f, 72.0f), game->uiTextShader);

    game->fullscreenQuad = CreateQuadNDC(glm::vec2(0.0f), glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
}