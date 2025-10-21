#ifndef SHADER_H

#include <GL/glew.h>

char *LoadShader(char *filepath);

GLuint CompileShader(char *shaderCode, GLenum shaderType);
GLuint CreateShaderProgram(char *vertexShaderCode, char *fragmentShaderCode);

#define SHADER_H
#endif