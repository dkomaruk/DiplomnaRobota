#ifndef TEXTURE_H

#include "defines.h"

#include <GL/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

enum TextureFlags
{
    TextureFlag_Filter_Min_Nearest         = 0x00'00'00'01,
    TextureFlag_Filter_Min_Linear          = 0x00'00'00'02,
    TextureFlag_Filter_Min_LinLin          = 0x00'00'00'04,
    TextureFlag_Filter_Min_NearNear        = 0x00'00'00'08,
    TextureFlag_Filter_Min_NearLin         = 0x00'00'00'10,
    TextureFlag_Filter_Min_LinNear         = 0x00'00'00'20,

    TextureFlag_Filter_Mag_Nearest         = 0x00'00'00'40,
    TextureFlag_Filter_Mag_Linear          = 0x00'00'00'80,

    TextureFlag_RGBA                       = 0x00'00'01'00,
    TextureFlag_RGB                        = 0x00'00'02'00,
    TextureFlag_Depth                      = 0x00'00'04'00, //Size is implementation dependent
    TextureFlag_Depth32F                   = 0x00'00'08'00,

    TextureFlag_Repeat                     = 0x00'00'10'00,
    TextureFlag_ClampToEdge                = 0x00'00'20'00,
    TextureFlag_Mipmaps                    = 0x00'00'40'00,
    TextureFlag_FlipY                      = 0x00'00'80'00,
};

#define TexturePreset_Common (TextureFlag_Filter_Min_LinLin | \
                              TextureFlag_Filter_Mag_Linear | \
                              TextureFlag_Repeat            | \
                              TextureFlag_Mipmaps           | \
                              TextureFlag_FlipY             | \
                              TextureFlag_RGBA)

struct Texture
{
    GLuint id;
    union
    {
        struct {int x; int y;};
        glm::ivec2 size;
    };
};

struct Sprite
{
    char name[128];
    union
    {
        glm::vec4 rect;
        struct
        {
            glm::vec2 pos;
            glm::vec2 size;
        };
    };
};

struct Atlas
{
    char *path;
    glm::vec2 size;
    std::vector<Sprite> sprites;
};

Texture CreateGLTexture(u8 *image, int width, int height, int flags = TexturePreset_Common);
Texture CreateTexture(char *imagePath, int flags = TexturePreset_Common);

void SetTexture(Texture *texture, GLuint textureSlot);
void SetTexture(GLuint textureID, GLuint textureSlot);

#define TEXTURE_H
#endif