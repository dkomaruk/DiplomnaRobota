#ifndef TEXTURE_H

#include <GL/glew.h>

GLuint CreateGLTexture(uint8 *image, int width, int height, bool nearest = false, bool repeat = true);
GLuint CreateTexture(char *imagePath, bool flipY = true, bool neareest = false, bool repeat = true);

void SetTexture(GLuint texture, GLuint textureSlot);

#define TEXTURE_H
#endif