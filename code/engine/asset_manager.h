#ifndef ASSET_MANAGER_H

#include "texture.h"
#include "model.h"

#include <GL/glew.h>

#include <string>
#include <unordered_map>

#include <filesystem>

struct Game;

struct ShaderDefinition
{
    std::string vertex;
    std::string fragment;
    std::string tess_control;
    std::string tess_eval;
};

struct AssetManager
{
    std::unordered_map<std::string, std::string> texturePaths;
    std::unordered_map<std::string, std::string> modelPaths;
    std::unordered_map<std::string, ShaderDefinition> shaderManifests;

    std::unordered_map<std::string, Texture> textures;
    std::unordered_map<std::string, Model *> models;
    std::unordered_map<std::string, GLuint> shaders;

    std::string baseDir, shadersDir;
};

Model *LoadModel(AssetManager *assets, const std::string &filepath, const std::string &modelName,
                 GLuint shader, u32 flags = 0, u16 type = ModelType_Static, float scale = 1.0f);

Model *GetModel(AssetManager *assets, const std::string &name, GLuint shader,
                u16 type = ModelType_DetermineOnLoad, u32 flags = 0, float scale = 1.0f);
Texture GetTexture(AssetManager *assets, const std::string &name, u32 flags = TexturePreset_Common);
Texture GetTextureByPath(AssetManager *assets, const std::string &path);
GLuint GetShader(Game *game, const std::string &name);
GLuint GetShader(AssetManager *assets, const std::string &name);

std::string RegisterAsset(AssetManager *assets, const std::filesystem::path &path);
Entity *AddNewEntityToScene(Game *game, Model *model, char *modelName, char *textId, glm::vec3 position = glm::vec3(0.0f),
                            glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

void LoadTestScene(Game *game);

#define ASSET_MANAGER_H
#endif