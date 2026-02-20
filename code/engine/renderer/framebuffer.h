#ifndef FRAMEBUFFER_H

#include "texture.h"

struct Game;

struct Framebuffer
{
    GLuint id;

    Texture color;
    Texture depth;
};

#define FboTexturePreset_ColorNearestRGB (TextureFlag_Filter_Min_Nearest | \
                                          TextureFlag_Filter_Mag_Nearest | \
                                          TextureFlag_RGB)
#define FboTexturePreset_ColorLinearRGB (TextureFlag_Filter_Min_Linear | \
                                         TextureFlag_Filter_Mag_Linear | \
                                         TextureFlag_RGB)
#define FboTexturePreset_ColorLinearRGBA (TextureFlag_Filter_Min_Linear | \
                                          TextureFlag_Filter_Mag_Linear | \
                                          TextureFlag_RGBA)
#define FboTexturePreset_Depth             (TextureFlag_Filter_Min_Nearest | \
                                           TextureFlag_Filter_Mag_Nearest | \
                                           TextureFlag_Depth)
#define FboTexturePreset_Depth32F          (TextureFlag_Filter_Min_Nearest | \
                                           TextureFlag_Filter_Mag_Nearest | \
                                           TextureFlag_Depth32F)

void SetupFramebuffers(Game *game);

Framebuffer CreateFramebuffer(glm::ivec2 size, int colorFlags, int depthFlags);
void DeleteFramebuffer(Framebuffer *fbo);

#define FRAMEBUFFER_H
#endif