#include "texture.h"

#include <stb_image.h>

GLuint CreateGLTexture(uint8 *image, int width, int height, bool repeat)
{
    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLfloat maxAniso;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)maxAniso);

    GLint wrapping = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

GLuint CreateTexture(char *imagePath, bool flipY, bool repeat)
{
    int width, height, channels;
    int desiredChannels = 4;
    stbi_set_flip_vertically_on_load(flipY);
    unsigned char *image = stbi_load(imagePath, &width, &height, &channels, desiredChannels);

    if(!image) return 0;

    GLuint texture = CreateGLTexture(image, width, height, repeat);

    stbi_image_free(image);

    return texture;
}

void SetTexture(GLuint texture, GLuint textureSlot)
{
    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(GL_TEXTURE_2D, texture);
}