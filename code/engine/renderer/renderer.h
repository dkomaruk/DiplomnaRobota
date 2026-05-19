#ifndef RENDERER_H

struct Game;

void RenderSceneEntities(Game *game);
void RenderShadowPass(Game *game);
void RenderOutlinePass(Game *game);
void RenderMainPass(Game *game);
void RenderParticlePass(Game *game);
void RenderPostProcessing(Game *game);
void RenderUI(Game *game);

#define RENDERER_H
#endif