#ifndef TERRAIN_EDITOR_H

struct Game;

#include <imgui.h>

void UpdateTerrainEditorUI(Game *game, bool *windowState, ImGuiWindowFlags flags);

#define TERRAIN_EDITOR_H
#endif