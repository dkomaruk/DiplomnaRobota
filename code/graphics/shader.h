#ifndef SHADER_H

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

char *LoadShader(char *filepath);

GLuint CompileShader(char *shaderCode, GLenum shaderType);
GLuint CreateShaderProgram(char *vertexShaderCode, char *fragmentShaderCode);

void UseShader(GLuint shader);
GLuint GetCurrentShader();

void ShaderSetVec2(GLuint shader, char *uniform, float v0, float v1);
void ShaderSetVec2(GLuint shader, char *uniform, glm::vec2 vector);

void ShaderSetVec3(GLuint shader, char *uniform, float v0, float v1, float v2);
void ShaderSetVec3(GLuint shader, char *uniform, glm::vec3 vector);

void ShaderSetVec4(GLuint shader, char *uniform, float v0, float v1, float v2, float v3);
void ShaderSetVec4(GLuint shader, char *uniform, glm::vec4 vector);

void ShaderSetMatrix4(GLuint shader, char *uniform, glm::mat4 matrix);

#define SHADER_H
#endif