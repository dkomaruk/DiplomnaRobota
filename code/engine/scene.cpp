#include "scene.h"

#include "game.h"
#include "entity.h"

#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

void SaveScene(Game *game, const std::string &filepath)
{
    json sceneJson;
    json entitiesArray = json::array();

    for(Entity *entity : game->sceneEntities)
    {
        json entityJson;
        entityJson["id"] = entity->id;
        entityJson["type"] = entity->type;
        entityJson["textId"] = entity->textId;
        entityJson["modelName"] = entity->modelName;

        entityJson["position"] = {entity->position.x, entity->position.y, entity->position.z};
        entityJson["rotation"] = {entity->rotation.x, entity->rotation.y, entity->rotation.z};
        entityJson["scale"] = {entity->scale.x, entity->scale.y, entity->scale.z};

        entityJson["snapToTerrain"] = entity->snapToTerrain;
        entityJson["isSelectable"] = entity->isSelectable;

        entitiesArray.push_back(entityJson);
    }

    sceneJson["entities"] = entitiesArray;

    std::ofstream file(filepath);
    if(file.is_open())
    {
        file << sceneJson.dump(4);
        file.close();
    }
    else
    {
        SDL_Log("Failed to open file for saving: %s", filepath.c_str());
    }
}

void LoadScene(Game *game, const std::string &filepath)
{
    std::ifstream file(filepath);
    if(!file.is_open())
    {
        SDL_Log("Failed to open scene file: %s", filepath.c_str());
        return;
    }

    json sceneJson;
    file >> sceneJson;

    if(sceneJson.contains("entities"))
    {
        for(const auto &entityJson : sceneJson["entities"])
        {
            std::string modelName = entityJson["modelName"];
            std::string textIdStr = entityJson["textId"];
            u16 type = entityJson["type"];

            GLuint shader = (type == EntityType_Static) ? GetShader(game, "main") : GetShader(game, "animation");

            Model *model = GetModel(&game->assets, modelName, shader, ModelType_DetermineOnLoad, aiProcess_GlobalScale, 1.0f);
            if(!model)
            {
                SDL_Log("Failed to load model %s for entity %s", modelName.c_str(), textIdStr.c_str());
                continue;
            }

            glm::vec3 pos(entityJson["position"][0], entityJson["position"][1], entityJson["position"][2]);
            glm::vec3 rotation(entityJson["rotation"][0], entityJson["rotation"][1], entityJson["rotation"][2]);
            glm::vec3 scale(entityJson["scale"][0], entityJson["scale"][1], entityJson["scale"][2]);

            char textId[25];
            strncpy(textId, textIdStr.c_str(), sizeof(textId) - 1);
            textId[sizeof(textId) - 1] = '\0';

            Entity *newEntity = AddNewEntityToScene(game, model, (char *)modelName.c_str(), textId, pos, rotation, scale);

            newEntity->type = type;
            newEntity->snapToTerrain = entityJson.value("snapToTerrain", false);
            newEntity->isSelectable = entityJson.value("isSelectable", true);

            UpdateEntity(game, newEntity);
        }
    }
}