#ifndef TEXTURE_H

#include <GL/glew.h>

GLuint CreateTexture(char *imagePath, int textureUnit);

void SetTexture(GLuint texture, GLuint textureSlot);

#define TEXTURE_H
#endif