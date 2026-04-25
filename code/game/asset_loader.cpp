#include "asset_loader.h"

#include "game.h"
#include "texture.h"
#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "framebuffer.h"
#include "debug.h"
#include "noise.h"
#include "frustum.h"

#include "string_utils.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <ThreadPool.h>

#include <AL/al.h>
#include <AL/alext.h>

#include <stb_vorbis.c>

#define TEXTURE_ATLAS_ELEMENT 0
#define SPRITE_ELEMENT 1

void XMLCALL ParseParticleSettingsStartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
    Atlas *atlas = (Atlas *)userData;

    int element = -1;
    if(strcmp(name, "TextureAtlas") == 0)
    {
        element = TEXTURE_ATLAS_ELEMENT;
    }
    else if(strcmp(name, "sprite") == 0)
    {
        element = SPRITE_ELEMENT;
    }

    switch(element)
    {
        case TEXTURE_ATLAS_ELEMENT:
        {
            atlas->size.x = StrToFloat(atts[3]);
            atlas->size.y = StrToFloat(atts[5]);
            //SDL_Log("x: %f; y:%f", atlas->size.x, atlas->size.y);
        } break;

        case SPRITE_ELEMENT:
        {
            Sprite sprite = {};
            float pixelX = StrToFloat(atts[3]);
            float pixelY = StrToFloat(atts[5]);
            float pixelW = StrToFloat(atts[7]);
            float pixelH = StrToFloat(atts[9]);

            sprite.pos.x = pixelX / atlas->size.x;
            sprite.pos.y = (atlas->size.y - pixelY - pixelH) / atlas->size.y;

            sprite.size.x = pixelW / atlas->size.x;
            sprite.size.y = pixelH / atlas->size.y;

            atlas->sprites.push_back(sprite);

            //SDL_Log("%d. (%f, %f), (%f, %f)", (int)atlas->sprites.size(), sprite.pos.x, sprite.pos.y, sprite.size.x, sprite.size.y);
        } break;
    }
}

void XMLCALL ParseParticleSettingsEndElement(void *userData, const XML_Char *name) { }

void LoadParticleSystem(Game *game)
{
    size_t fileSize;
    void *fileMemory = SDL_LoadFile("../data/imgs/animated_smoke/1.xml", &fileSize);
    if(!fileMemory)
    {
        SDL_Log("Failed to load 1.xml. Error: %s", SDL_GetError());
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    if(!parser)
    {
        SDL_Log("Failed to create an XML parser");
    }

    XML_SetUserData(parser, (void *)&game->atlas);
    XML_SetElementHandler(parser, ParseParticleSettingsStartElement, ParseParticleSettingsEndElement);

    XML_Status parsingResult = XML_Parse(parser, (char *)fileMemory, (int)fileSize, XML_TRUE);
    if(!parsingResult)
    {
        SDL_Log("Failed to parse 1.xml");
    }
    SDL_free(fileMemory);

    game->particleTextures[0] = CreateTexture("../data/imgs/smoke.png");
    game->particleTextures[1] = CreateTexture("../data/imgs/smoke2.png");
    game->particleTextures[2] = CreateTexture("../data/imgs/smoke3.png");
    game->particleTextures[3] = CreateTexture("../data/imgs/smoke4.png");
    game->particleTextures[4] = CreateTexture("../data/imgs/smoke5.png");
    game->particleTextures[5] = CreateTexture("../data/imgs/animated_smoke/1.png");
    game->particleTextures[6] = CreateTexture("../data/imgs/fire.png");
    game->particleTextures[7] = CreateTexture("../data/imgs/fire2.png");
    game->particleTextures[8] = CreateTexture("../data/imgs/extra/alpha/circle_05_a.png");
    game->particleTextures[9] = CreateTexture("../data/imgs/extra/alpha/twirl_04_a.png");
    game->particleTextures[10] = CreateTexture("../data/imgs/extra/alpha/star_05_a.png");
    game->particleTextures[11] = CreateTexture("../data/imgs/extra/alpha/effect_02_a.png");
    game->particleTextures[12] = CreateTexture("../data/imgs/extra/alpha/trace_01_a.png");

    for(int i = 0; i < ArrayCount(game->particleSystems); i++)
    {
        game->particleSystems[i] = InitParticleSystem(game, &game->smokeSettings);
    }

    game->particleSystems[0].pos = glm::vec3(0.0f);

    int maxNumOfParticles = game->smokeSettings.maxNumOfParticles;
    game->particleData = (ParticleData *)calloc(maxNumOfParticles * ArrayCount(game->particleSystems), sizeof(ParticleData));
    game->textureID = game->particleTextures[game->currentTexture].id;

    game->atlas.path = "../data/imgs/animated_smoke/1.png";
    game->smokeSettings.atlas = &game->atlas;

    game->particlesQuad = CreateUnitQuadStripes();
    glBindVertexArray(game->particlesQuad.vao);

    glGenBuffers(1, &game->vboInstances);
    glBindBuffer(GL_ARRAY_BUFFER, game->vboInstances);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * maxNumOfParticles * ArrayCount(game->particleSystems),
                 game->particleData, GL_STREAM_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, scale));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, angle));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, uvOffset));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, uvScale));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, offset));
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, color));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
    {
        if(game->smokeSettings.prewarm)
        {
            game->particleSystems[i].prewarmTimer = StartTimer(game->smokeSettings.prewarmSeconds);
        }
    }
}

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

Entity *AddNewEntityToScene(Game *game, Model *model, char *textId, glm::vec3 position = glm::vec3(0.0f),
                            glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f))
{
    Entity *newEntity = (Entity *)calloc(1, sizeof(Entity));
    *newEntity = CreateEntity(model);

    newEntity->position = position;
    newEntity->rotation = rotation;
    newEntity->scale = scale;

    strcpy(newEntity->textId, textId);
    newEntity->id = game->sceneEntities.size() ? (game->sceneEntities.back()->id + 1) : 1;

    game->sceneEntities.push_back(newEntity);

    return newEntity;
}

void LoadTestScene(Game *game)
{
    //AUDIO
    //LoadAudio(game);

    //TODO: Remove from asset loader
    SetupFramebuffers(game);

    //FONTS
    //int fontSizes[] = {4, 12, 18, 20, 24, 36, 48};
    int fontSizes[] = {48};
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
    GLuint mainShader = CreateShaderProgram(LoadShader("../data/shaders/main.vert"),
                                            LoadShader("../data/shaders/main.frag"));
    GLuint lightSourceShader = CreateShaderProgram(LoadShader("../data/shaders/main.vert"),
                                                   LoadShader("../data/shaders/colorFill.frag"));
    GLuint skinnedOutlineShader = CreateShaderProgram(LoadShader("../data/shaders/main_skinned.vert"),
                                                      LoadShader("../data/shaders/colorFill.frag"));
    GLuint uiTextShader = CreateShaderProgram(LoadShader("../data/shaders/uiText.vert"),
                                              LoadShader("../data/shaders/uiText.frag"));
    GLuint postProcessShader = CreateShaderProgram(LoadShader("../data/shaders/post_processing.vert"),
                                                   LoadShader("../data/shaders/post_processing.frag"));
    GLuint particleShader = CreateShaderProgram(LoadShader("../data/shaders/particle.vert"),
                                                LoadShader("../data/shaders/particle.frag"));
    GLuint terrainShader = CreateShaderProgram(LoadShader("../data/shaders/terrain.vert"),
                                               LoadShader("../data/shaders/terrain.frag"));
    GLuint animationShader = CreateShaderProgram(LoadShader("../data/shaders/main_skinned.vert"),
                                                 LoadShader("../data/shaders/main.frag"));
    GLuint lineShader = CreateShaderProgram(LoadShader("../data/shaders/line.vert"),
                                            LoadShader("../data/shaders/colorFill.frag"));
    GLuint selectionBoxShader = CreateShaderProgram(LoadShader("../data/shaders/uiText.vert"),
                                                    LoadShader("../data/shaders/colorFill.frag"));
    GLuint aabbShader = CreateShaderProgram(LoadShader("../data/shaders/uiText.vert"),
                                            LoadShader("../data/shaders/colorFill.frag"));
    GLuint skymapShader = CreateShaderProgram(LoadShader("../data/shaders/environment.vert"),
                                              LoadShader("../data/shaders/environment.frag"));
    GLuint shadowShader = CreateShaderProgram(LoadShader("../data/shaders/shadow.vert"),
                                              LoadShader("../data/shaders/shadow.frag"));
    GLuint skinnedShadowShader = CreateShaderProgram(LoadShader("../data/shaders/shadow_skinned.vert"),
                                                     LoadShader("../data/shaders/shadow.frag"));
    GLuint grassShader = CreateShaderProgram(LoadShader("../data/shaders/grass.vert"),
                                             LoadShader("../data/shaders/grass.frag"));

    ShaderSetVec2(mainShader, "u_viewport", game->windowSize);
    ShaderSetVec2(animationShader, "u_viewport", game->windowSize);

    ShaderSetVec4(lightSourceShader, "u_color", glm::vec4(1.0f));
    ShaderSetVec4(skinnedOutlineShader, "u_color", glm::vec4(1.0f));
    ShaderSetVec4(selectionBoxShader, "u_color", glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

    ShaderSetInt(postProcessShader, "u_outlineThickness", (int)game->outlineThickness);
    ShaderSetInt(postProcessShader, "u_inverted", 0);
    ShaderSetInt(postProcessShader, "u_grayscale", 0);
    ShaderSetInt(postProcessShader, "u_showOutline", 1);
    ShaderSetInt(postProcessShader, "u_showParticles", 1);

    game->shaders.push_back(mainShader);
    game->shaders.push_back(lightSourceShader);
    game->shaders.push_back(skinnedOutlineShader);
    game->shaders.push_back(postProcessShader);
    game->shaders.push_back(terrainShader);
    game->shaders.push_back(animationShader);
    game->shaders.push_back(lineShader);
    game->shaders.push_back(skymapShader);
    game->shaders.push_back(shadowShader);
    game->shaders.push_back(skinnedShadowShader);
    game->shaders.push_back(particleShader);
    game->shaders.push_back(grassShader);

    for(int i = 0; i < game->shaders.size(); i++)
    {
        ShaderSetMatrix4(game->shaders[i], "u_projection", game->perspectiveProjection);
    }

    game->shaders.push_back(uiTextShader);
    ShaderSetMatrix4(uiTextShader, "u_projection", game->orthoProjection);

    game->shaders.push_back(selectionBoxShader);
    ShaderSetMatrix4(selectionBoxShader, "u_projection", game->orthoProjection);

    game->mainShader = mainShader;
    game->postProcessShader = postProcessShader;
    game->outlineShader = lightSourceShader;
    game->lightSourceShader = lightSourceShader;
    game->skinnedOutlineShader = skinnedOutlineShader;
    game->uiTextShader = uiTextShader;
    game->particleShader = particleShader;
    game->terrainShader = terrainShader;
    game->animationShader = animationShader;
    game->lineShader = lineShader;
    game->selectionBoxShader = selectionBoxShader;
    game->skymapShader = skymapShader;
    game->shadowShader = shadowShader;
    game->skinnedShadowShader = skinnedShadowShader;
    game->grassShader = grassShader;

    //MESHES
#ifdef LOAD_ASSETS
    Model *abrams = ImportModel("../data/models/abrams/abrams.fbx", game->mainShader, aiProcess_Triangulate);
    Model *soldier = ImportModel("../data/models/soldier/Ginga Variation 3.fbx", game->animationShader,
                                 aiProcess_Triangulate | aiProcess_GlobalScale, ModelType_Animated, 0.01f);
    Model *soldierAnimated = ImportModel("../data/models/soldier/Rifle Run.fbx", game->animationShader,
                                         aiProcess_Triangulate | aiProcess_GlobalScale, ModelType_Animated, 0.01f);
    game->grass = ImportModel("../data/extra/grass2.fbx", game->mainShader, aiProcess_Triangulate |
                              aiProcess_GlobalScale, ModelType_Static, 0.001f);

    game->soldierEntity = AddNewEntityToScene(game, soldier, "soldier", glm::vec3(0.0f, 0.5f, 0.0f));
    game->tank = AddNewEntityToScene(game, abrams, "tank");
    game->soldierAnimated = AddNewEntityToScene(game, soldierAnimated, "animated_soldier");

    Entity *tank = game->tank;
    for(int i = 0; i < tank->model->numOfNodes; i++)
    {
        char *nodeName = tank->model->nodes[i].name;
        if(nodeName && strcmp(nodeName, "Tourelle_01") == 0)
        {
            tank->turret.nodeId = i;
            tank->turret.transform = tank->model->nodes[i].localTransform;
        }
        else if(nodeName && strcmp(nodeName, "Axe_Canon_01") == 0)
        {
            tank->gun.nodeId = i;
            tank->gun.transform = tank->model->nodes[i].localTransform;
        }
    }

    //Scene Light
    glm::vec3 dirDiffuse = glm::vec3(0.9f);
    glm::vec3 dirAmbient = glm::vec3(0.4f);
    glm::vec3 dirSpecular = glm::vec3(1.0f);
    game->dirLight = CreateDirLight(glm::vec3(1.3f, -2.3f, -0.0f), dirDiffuse, dirAmbient, dirSpecular);
    ShaderSetDirLight(game->mainShader, game->dirLight);
    ShaderSetDirLight(game->animationShader, game->dirLight);
    ShaderSetDirLight(game->terrainShader, game->dirLight);
    ShaderSetInt(game->mainShader, "u_dirLightCount", 1);
    ShaderSetInt(game->animationShader, "u_dirLightCount", 1);

    ShaderSetInt(game->mainShader, "u_pointLightCount", 0);
    ShaderSetInt(game->animationShader, "u_pointLightCount", 0);

    //Terrain
    game->terrain = CreateTerrainFromImage("../data/heightmap.png", 20.0f, 1.0f, 0.1f, 8, 22.0f);
    game->terrain.texture1 = CreateTexture("../data/wispy-grass-meadow_albedo.bmp");

    //Particles
    LoadParticleSystem(game);

    //Skymap
    //StartProfiling();
    //int flags = TexturePreset_Common;
    //flags = FLAG_TOGGLE(flags, TextureFlag_Filter_Min_LinLin | TextureFlag_Filter_Min_Nearest | TextureFlag_FlipY);
    //game->skymapTexture = CreateTexture("../data/imgs/extra/sky.jpg", flags);
    //EndProfiling("Skymap");

    //Debug lines
    game->pickingRay = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), game->lineShader, glm::vec3(1.0f, 0.0f, 0.0f));
    CreateFrustumLines(game->frustumLines, game->frustumNormals, game->lineShader);

#endif

    glm::vec2 size = glm::vec2(256.0f, 256.0f);
    u8 *valueNoise = GenerateValueNoise(size);
    game->valueNoise = CreateGLTexture(valueNoise, (int)size.x, (int)size.y);
    free(valueNoise);

    float *perlinNoise = GeneratePerlinNoise(size, glm::ivec2(32), 4, 0.5f, 2.0f);

    u8 *perlinNoiseImage = NoiseToImage(perlinNoise, size);
    game->perlinNoise = CreateGLTexture(perlinNoiseImage, (int)size.x, (int)size.y);
    free(perlinNoiseImage);

    glm::vec3 textColor = glm::vec3(1.0f, 0.0f, 0.0f);
    game->fpsCounter = CreateText(&game->fonts[48], "0 FPS", glm::vec2(20.0f, 54.0f), game->uiTextShader, textColor);
    game->msPerFrame = CreateText(&game->fonts[48], "0 ms/f", glm::vec2(400.0f, 54.0f), game->uiTextShader, textColor);
    game->aliveParticlesText = CreateText(&game->fonts[48], "Alive Particles: 0", glm::vec2(20.0f, 54.0f + 60.0f), game->uiTextShader, textColor);
    game->deadParticlesText = CreateText(&game->fonts[48], "Dead Particles: 0", glm::vec2(20.0f, 54.0f + 60.0f + 60.0f), game->uiTextShader, textColor);

    game->fullscreenQuad = CreateQuadNDC(glm::vec2(0.0f), glm::vec2(game->windowSize), glm::vec2(game->windowSize));
}