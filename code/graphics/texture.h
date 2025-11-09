#ifndef TEXTURE_H

#include <GL/glew.h>

GLuint CreateGLTexture(uint8 *image, int width, int height, bool repeat = true);
GLuint CreateTexture(char *imagePath, bool flipY = true, bool repeat = true);

void SetTexture(GLuint texture, GLuint textureSlot);

#define TEXTURE_H
#endif