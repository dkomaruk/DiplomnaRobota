#include "asset_loader.h"

#include "game.h"

#include "graphics/texture.h"
#include "graphics/shader.h"

void LoadAssets(Game *game)
{
    game->font = TTF_OpenFont("../data/fonts/arial.ttf", 18);
    if(!game->font)
    {
        SDL_Log("Failed to load arial.ttf font. Error: %s", SDL_GetError());
    }
    else
    {
        if(!TTF_SetFontSDF(game->font, true))
        {
            SDL_Log("Failed to enable SDF for arial.ttf font. Error: %s", SDL_GetError());
        }
    }

    GLuint shader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"), LoadShader("../data/shaders/fragment.frag"));
    GLuint lightSourceShader = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"), LoadShader("../data/shaders/fragment2.frag"));
    GLuint uiShader = CreateShaderProgram(LoadShader("../data/shaders/vertex3.vert"), LoadShader("../data/shaders/ui.frag"));
    GLuint pickingShader = CreateShaderProgram(LoadShader("../data/shaders/picking.vert"), LoadShader("../data/shaders/picking.frag"));
    GLuint postProcessShader = CreateShaderProgram(LoadShader("../data/shaders/vertex3.vert"), LoadShader("../data/shaders/fragment3.frag"));

    ShaderSetVec2(shader, "u_viewport", WINDOW_WIDTH, WINDOW_HEIGHT);

    ShaderSetVec3(lightSourceShader, "u_lightColor", vec3(1.0f));

    ShaderSetInt(postProcessShader, "u_outlineThickness", (int)game->outlineThickness);
    ShaderSetInt(postProcessShader, "u_inverted", 0);
    ShaderSetInt(postProcessShader, "u_grayscale", 0);
    ShaderSetInt(postProcessShader, "u_showOutline", 1);

    game->shaders.push_back(shader);
    game->shaders.push_back(lightSourceShader);
    game->shaders.push_back(pickingShader);
    game->shaders.push_back(postProcessShader);

    game->mainShader = shader;
    game->postProcessShader = postProcessShader;
    game->outlineShader = lightSourceShader;
    game->lightSourceShader = lightSourceShader;
    game->pickingShader = pickingShader;
    game->uiShader = uiShader;

    Model *soldier = ImportModel("../data/models/soldier/soldier.obj", game->mainShader, aiProcess_Triangulate);
    //Model *soldier = ImportModel("../data/models/soldier/soldier.glb", game->mainShader, aiProcess_Triangulate);
    if(soldier->numOfMeshes != -1)
    {
        Entity *soldierEntity = (Entity *)malloc(sizeof(Entity));
        *soldierEntity = CreateEntity(soldier);
        soldierEntity->position.y += 0.5f;
        game->sceneEntities.push_back(soldierEntity);
    }

    Model *test = ImportModel("../data/models/backpack/backpack.obj", game->mainShader,
                             aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    Entity *testEntity = (Entity *)malloc(sizeof(Entity));
    *testEntity = CreateEntity(test);
    testEntity->position = vec3(0.0f, 0.0f, 3.0f);
    testEntity->rotation = vec3(0.0f, 180.0f, 0.0f);
    testEntity->scale = vec3(0.2f);
    game->sceneEntities.push_back(testEntity);
    game->testEntity = testEntity;

    Model *sphere = ImportModel("../data/models/sphere.obj", game->mainShader, aiProcess_Triangulate);
    sphere->meshes->material.diffuseTexture = CreateTexture("../data/models/sphere_diffuse.png");

    Entity *sphereEntity = (Entity *)malloc(sizeof(Entity));
    *sphereEntity = CreateEntity(sphere);
    sphereEntity->position = vec3(-1.0f, 0.0f, 3.0f);
    sphereEntity->rotation = vec3(0.0f, -90.0f, 0.0f);
    sphereEntity->scale = vec3(0.2f);
    game->sceneEntities.push_back(sphereEntity);

    Model *sphere2 = ImportModel("../data/models/sphere2.obj", game->mainShader, aiProcess_Triangulate);
    sphere2->meshes->material.diffuseTexture = CreateTexture("../data/models/sphere2_diffuse.png");

    Entity *sphereEntity2 = (Entity *)malloc(sizeof(Entity));
    *sphereEntity2 = CreateEntity(sphere2);
    sphereEntity2->position = vec3(0.0f, 1.0f, 3.0f);
    sphereEntity2->rotation = vec3(0.0f, -90.0f, 0.0f);
    sphereEntity2->scale = vec3(0.2f);
    game->sceneEntities.push_back(sphereEntity2);

    Model *car = ImportModel("../data/models/car_scene.obj", game->mainShader, aiProcess_Triangulate);
    GLuint carDiffuseTexture = CreateTexture("../data/models/car_diffuse.png");
    for(int i = 0; i < car->numOfMeshes; i++)
    {
        car->meshes[i].material.diffuseTexture = carDiffuseTexture;
    }

    Entity *carEntity = (Entity *)malloc(sizeof(Entity));
    *carEntity = CreateEntity(car);
    carEntity->position = vec3(0.0f, 1.0f, -3.0f);
    carEntity->rotation = vec3(0.0f, -90.0f, 0.0f);
    //carEntity.scale = vec3(0.2f);
    game->sceneEntities.push_back(carEntity);

    MaterialPhong containerMaterial = {};
    containerMaterial.diffuseTexture = CreateTexture("../data/imgs/container2.png");
    containerMaterial.specularTexture = CreateTexture("../data/imgs/container2_specular.png");
    //containerMaterial.specularTexture = CreateTexture("../data/imgs/lighting_maps_specular_color.png");
    containerMaterial.emissionTexture = CreateTexture("../data/imgs/matrix.jpg");
    containerMaterial.shininess = 256.0f;

    Model *cubeMesh = ImportModel("../data/models/cube.obj", game->mainShader, aiProcess_Triangulate);
    cubeMesh->meshes[0].material = containerMaterial;

    Entity *cubeEntity = (Entity *)malloc(sizeof(Entity));
    *cubeEntity = CreateEntity(cubeMesh);
    cubeEntity->position.x -= 3.0f;
    cubeEntity->position.z += 3.0f;
    cubeEntity->position.y -= 0.5f;
    cubeEntity->scale -= vec3(0.5f);
    game->sceneEntities.push_back(cubeEntity);

    Entity *cubeEntity2 = (Entity *)malloc(sizeof(Entity));
    *cubeEntity2 = CreateEntity(cubeMesh);
    cubeEntity2->position = vec3(1.0f, 0.0f, 3.0f);
    cubeEntity2->rotation = vec3(0.0f, 180.0f, 0.0f);
    cubeEntity2->scale = vec3(0.2f);
    game->sceneEntities.push_back(cubeEntity2);

    Model *lightMesh = ImportModel("../data/models/cube.obj", lightSourceShader, aiProcess_Triangulate);

    vec3 dirDiffuse = vec3(0.9f);
    vec3 dirAmbient = vec3(0.05f);
    vec3 dirSpecular = vec3(1.0f);
    //vec3 dirSpecular = vec3(0.8f, 0.7f, 0.0f);
    DirectionalLight dirLight = CreateDirLight(vec3(1.5f, -1.0f, -0.8f), dirDiffuse, dirAmbient, dirSpecular);
    ShaderSetDirLight(game->mainShader, dirLight);
    //ShaderSetInt(game->mainShader, "u_dirLightCount", 0);
    Entity *dirLightMesh = (Entity *)malloc(sizeof(Entity));
    *dirLightMesh = CreateEntity(lightMesh);
    dirLightMesh->scale = vec3(50.0f);
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

        pointLights[i].diffuse = vec3(0.5f);
        pointLights[i].ambient = vec3(0.05f);
        pointLights[i].specular = vec3(0.1f);

        pointLights[i].constant = 1.0f;
        pointLights[i].linear = 0.027f;
        pointLights[i].quadratic = 0.0028f;

        ShaderSetPointLight(game->mainShader, pointLights[i], i);

        pointLightsSources[i] = CreateEntity(lightMesh);
        pointLightsSources[i].scale = vec3(0.15f);
        pointLightsSources[i].position = pointLights[i].position;

        game->sceneEntities.push_back(&pointLightsSources[i]);
    }

    ShaderSetInt(game->mainShader, "u_pointLightCount", maxPointLights);

    for(int i = 0; i < 2; i++)
    {
        InfantrySquad *squad = (InfantrySquad *)malloc(sizeof(InfantrySquad));
        *squad = CreateInfantrySquad(cubeMesh, 1, 10);
        squad->scale = vec3(0.5f);
        squad->position.z = -10.0f / 2.0f;
        squad->position.x = -10.0f / 2.0f;
        squad->position.y -= i;

        game->sceneEntities.push_back(squad);
    }

    for(uint16 i = 0; i < game->sceneEntities.size(); i++)
    {
        game->sceneEntities[i]->id = i + 1;
    }
}