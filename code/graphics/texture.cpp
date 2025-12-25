#include "texture.h"

#include <stb_image.h>

GLuint CreateGLTexture(uint8 *image, int width, int height, TextureFlags flags)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLint minFilterFlags = flags & (TextureFlag_Filter_Min_Linear | TextureFlag_Filter_Min_Nearest |
                                    TextureFlag_Filter_Min_LinLin | TextureFlag_Filter_Min_NearNear |
                                    TextureFlag_Filter_Min_LinNear | TextureFlag_Filter_Min_NearLin);

    if(!minFilterFlags || (minFilterFlags & (minFilterFlags - 1)))
    {
        SDL_Log("Failed to create a texture. Multiple min filter flags are set");
        return 0;
    }
    if((flags & TextureFlag_Filter_Mag_Linear) && (flags & TextureFlag_Filter_Mag_Nearest))
    {
        SDL_Log("Failed to create a texture. Multiple mag filter flags are set");
        return 0;
    }

    GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
    if(flags & TextureFlag_Filter_Min_Linear)
        minFilter = GL_LINEAR;
    if(flags & TextureFlag_Filter_Min_Nearest)
        minFilter = GL_NEAREST;
    if(flags & TextureFlag_Filter_Min_NearNear)
        minFilter = GL_NEAREST_MIPMAP_NEAREST;
    if(flags & TextureFlag_Filter_Min_NearLin)
        minFilter = GL_NEAREST_MIPMAP_LINEAR;
    if(flags & TextureFlag_Filter_Min_LinNear)
        minFilter = GL_LINEAR_MIPMAP_NEAREST;

    GLint magFilter = (flags & TextureFlag_Filter_Mag_Linear) ? GL_LINEAR : GL_NEAREST;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    GLfloat maxAniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)maxAniso);

    GLint wrapping = (flags & TextureFlag_Repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);

    if((flags & TextureFlag_RGBA) && (flags & TextureFlag_RGB))
    {
        SDL_Log("Failed to create a texture. Multiple color channels flags are set");
        return 0;
    }

    GLint colorChannels = (flags & TextureFlag_RGB) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, colorChannels, GL_UNSIGNED_BYTE, (const void *)image);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

GLuint CreateTexture(char *imagePath, TextureFlags flags)
{
    int width, height, channels;
    int desiredChannels = 4;

    bool flipY = flags & TextureFlag_FlipY;
    stbi_set_flip_vertically_on_load(flipY);
    unsigned char *image = stbi_load(imagePath, &width, &height, &channels, desiredChannels);

    if(!image) return 0;

    GLuint texture = CreateGLTexture(image, width, height, flags);

    stbi_image_free(image);

    return texture;
}

void SetTexture(GLuint texture, GLuint textureSlot)
{
    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(GL_TEXTURE_2D, texture);
}