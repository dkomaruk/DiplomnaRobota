#ifndef SHADER_H

#include "mesh.h"
#include "light.h"

#include "defines.h"

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

void ShaderSetInt(GLuint shader, char *uniform, int v);
void ShaderSetUInt(GLuint shader, char *uniform, u32 v);
void ShaderSetFloat(GLuint shader, char *uniform, float v);

void ShaderSetVec2(GLuint shader, char *uniform, float v0, float v1);
void ShaderSetVec2(GLuint shader, char *uniform, glm::vec2 vector);

void ShaderSetVec3(GLuint shader, char *uniform, float v0, float v1, float v2);
void ShaderSetVec3(GLuint shader, char *uniform, glm::vec3 vector);

void ShaderSetVec4(GLuint shader, char *uniform, float v0, float v1, float v2, float v3);
void ShaderSetVec4(GLuint shader, char *uniform, glm::vec4 vector);

void ShaderSetMatrix4(GLuint shader, char *uniform, glm::mat4 matrix);
void ShaderSetMatrix4Array(GLuint shader, char *uniform, float *matrix, int count = 1);
void ShaderSetMaterial(GLuint shader, MaterialPhong *material);

void ShaderSetDirLight(GLuint shader, DirectionalLight light);
void ShaderSetPointLight(GLuint shader, PointLight light, int index);
void ShaderSetSpotLight(GLuint shader, SpotLight light);

#define SHADER_H
#endif