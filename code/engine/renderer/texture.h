#ifndef TEXTURE_H

#include <GL/glew.h>

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

    TextureFlag_Repeat                     = 0x00'00'04'00,
    TextureFlag_FlipY                      = 0x00'00'08'00,

    TextureFlag_Common                     = TextureFlag_Filter_Min_LinLin | TextureFlag_Filter_Mag_Linear |
                                             TextureFlag_Repeat | TextureFlag_FlipY | TextureFlag_RGBA
};

GLuint CreateGLTexture(uint8 *image, int width, int height, TextureFlags flags = TextureFlag_Common);
GLuint CreateTexture(char *imagePath, TextureFlags flags = TextureFlag_Common);

void SetTexture(GLuint texture, GLuint textureSlot);

#define TEXTURE_H
#endif