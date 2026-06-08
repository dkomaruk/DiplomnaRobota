#ifndef SCENE_H

#include <string>

struct Game;

void SaveScene(Game *game, const std::string &filepath);
void LoadScene(Game *game, const std::string &filepath);

#define SCENE_H
#endif