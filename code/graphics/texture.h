#ifndef TEXTURE_H

#include <GL/glew.h>

GLuint CreateTexture(char *imagePath, int textureUnit, bool repeat = true);

void SetTexture(GLuint texture, GLuint textureSlot);

#define TEXTURE_H
#endif