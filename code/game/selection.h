#ifndef SELECTION_H

struct Game;
struct Ray;

struct SelectionBox
{
    glm::vec2 start = glm::vec2(0.0f);
    glm::vec2 size = glm::vec2(0.0f);
};

void UpdateSelection(Game *game);
void SelectSingleObject(Game *game, Ray *pickingRay);
void SelectMultipleObjects(Game *game);

void RenderSelectionBox(Game *game, SelectionBox *box);

#define SELECTION_H
#endif