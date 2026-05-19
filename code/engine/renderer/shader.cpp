#include "shader.h"

#include <SDL3/SDL.h>

#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>

GLuint currentShader = 0;

char *LoadShader(const char *filepath)
{
    FILE *file = fopen(filepath, "rb");

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(fileSize + 1);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if(bytesRead != fileSize)
    {
        SDL_Log("Bytes read: %d, bytes expected: %d", bytesRead, fileSize);
    }

    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}

GLuint CompileShader(char *shaderCode, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, NULL);

    glCompileShader(shader);

    GLint compilationResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilationResult);
    if(compilationResult != GL_TRUE)
    {
        char buffer[1024];
        glGetShaderInfoLog(shader, 1024, NULL, buffer);

        char *shaderTypeString = (shaderType == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        SDL_Log("Failed to compile %s shader. Error: %s\n", shaderTypeString, buffer);
    }

    return shader;
}

GLuint CreateShaderProgram(char *vertexShaderCode, char *fragmentShaderCode,
                           char *tessControlShaderCode, char *tessEvalShaderCode)
{
    GLuint vertexShader = CompileShader(vertexShaderCode, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(fragmentShaderCode, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    GLuint tescShader = 0;
    if(tessControlShaderCode)
    {
        tescShader = CompileShader(tessControlShaderCode, GL_TESS_CONTROL_SHADER);
        glAttachShader(shaderProgram, tescShader);
    }

    GLuint teseShader = 0;
    if(tessEvalShaderCode)
    {
        teseShader = CompileShader(tessEvalShaderCode, GL_TESS_EVALUATION_SHADER);
        glAttachShader(shaderProgram, teseShader);
    }

    glLinkProgram(shaderProgram);

    GLint linkResult;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkResult);
    if(linkResult != GL_TRUE)
    {
        char buffer[1024];
        glGetProgramInfoLog(shaderProgram, 1024, NULL, buffer);
        SDL_Log("Failed to link shader program. Error: %s\n", buffer);
    }

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if(tescShader)
    {
        glDetachShader(shaderProgram, tescShader);
        glDeleteShader(tescShader);
    }
    if(teseShader)
    {
        glDetachShader(shaderProgram, teseShader);
        glDeleteShader(teseShader);
    }

    free(vertexShaderCode);
    free(fragmentShaderCode);
    free(tessControlShaderCode);
    free(tessEvalShaderCode);

    return shaderProgram;
}

void UseShader(GLuint shader)
{
    glUseProgram(shader);
    currentShader = shader;
}

GLuint GetCurrentShader()
{
    return currentShader;
}

void ShaderSetInt(GLuint shader, char *uniform, int v)
{
    if(currentShader != shader) UseShader(shader);
    glUniform1i(glGetUniformLocation(shader, uniform), v);
}

void ShaderSetUInt(GLuint shader, char *uniform, u32 v)
{
    if(currentShader != shader) UseShader(shader);
    glUniform1ui(glGetUniformLocation(shader, uniform), v);
}

void ShaderSetFloat(GLuint shader, char *uniform, float v)
{
    if(currentShader != shader) UseShader(shader);
    glUniform1f(glGetUniformLocation(shader, uniform), v);
}

void ShaderSetVec2(GLuint shader, char *uniform, float v0, float v1)
{
    if(currentShader != shader) UseShader(shader);
    glUniform2f(glGetUniformLocation(shader, uniform), v0, v1);
}

void ShaderSetVec2(GLuint shader, char *uniform, glm::vec2 vector)
{
    if(currentShader != shader) UseShader(shader);
    glUniform2f(glGetUniformLocation(shader, uniform), vector.x, vector.y);
}

void ShaderSetVec3(GLuint shader, char *uniform, float v0, float v1, float v2)
{
    if(currentShader != shader) UseShader(shader);
    glUniform3f(glGetUniformLocation(shader, uniform), v0, v1, v2);
}

void ShaderSetVec3(GLuint shader, char *uniform, glm::vec3 vector)
{
    if(currentShader != shader) UseShader(shader);
    glUniform3f(glGetUniformLocation(shader, uniform), vector.x, vector.y, vector.z);
}

void ShaderSetVec4(GLuint shader, char *uniform, float v0, float v1, float v2, float v3)
{
    if(currentShader != shader) UseShader(shader);
    glUniform4f(glGetUniformLocation(shader, uniform), v0, v1, v2, v3);
}

void ShaderSetVec4(GLuint shader, char *uniform, glm::vec4 vector)
{
    if(currentShader != shader) UseShader(shader);
    glUniform4f(glGetUniformLocation(shader, uniform), vector.x, vector.y, vector.z, vector.w);
}

void ShaderSetMatrix4(GLuint shader, char *uniform, glm::mat4 matrix)
{
    if(currentShader != shader) UseShader(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, value_ptr(matrix));
}

void ShaderSetMatrix4Array(GLuint shader, char *uniform, float *matrix, int count)
{
    if(currentShader != shader) UseShader(shader);

    char buffer[256];
    sprintf(buffer, "%s[0]", uniform);

    glUniformMatrix4fv(glGetUniformLocation(shader, buffer), count, GL_FALSE, matrix);
}

void ShaderSetMaterial(GLuint shader, MaterialPhong *material)
{
    ShaderSetInt(shader, "u_material.diffuse", 0);
    ShaderSetInt(shader, "u_material.specular", 1);
    ShaderSetInt(shader, "u_material.emission", 2);
    ShaderSetFloat(shader, "u_material.shininess", material->shininess);
}

void ShaderSetDirLight(GLuint shader, DirectionalLight light)
{
    if(currentShader != shader) UseShader(shader);

    ShaderSetInt(shader, "u_dirLightCount", 1);
    ShaderSetVec3(shader, "u_dirLight.direction", light.direction);
    ShaderSetVec3(shader, "u_dirLight.diffuse", light.diffuse);
    ShaderSetVec3(shader, "u_dirLight.ambient", light.ambient);
    ShaderSetVec3(shader, "u_dirLight.specular", light.specular);
}

void ShaderSetPointLight(GLuint shader, PointLight light, int index)
{
    if(currentShader != shader) UseShader(shader);

    char uniformName[128];

    snprintf(uniformName, sizeof(uniformName), "u_pointLights[%d].position", index);
    ShaderSetVec3(shader, uniformName, light.position);

    snprintf(uniformName, sizeof(uniformName), "u_pointLights[%d].constant", index);
    ShaderSetFloat(shader, uniformName, light.constant);

    snprintf(uniformName, sizeof(uniformName), "u_pointLights[%d].linear", index);
    ShaderSetFloat(shader, uniformName, light.linear);

    snprintf(uniformName, sizeof(uniformName), "u_pointLights[%d].quadratic", index);
    ShaderSetFloat(shader, uniformName, light.quadratic);

    snprintf(uniformName, sizeof(uniformName), "u_pointLights[%d].diffuse", index);
    ShaderSetVec3(shader, uniformName, light.diffuse);

    snprintf(uniformName, sizeof(uniformName), "u_pointLights[%d].ambient", index);
    ShaderSetVec3(shader, uniformName, light.ambient);

    snprintf(uniformName, sizeof(uniformName), "u_pointLights[%d].specular", index);
    ShaderSetVec3(shader, uniformName, light.specular);
}

//void ShaderSetSpotLight(GLuint shader, SpotLight light)
//{
//    if(currentShader != shader) UseShader(shader);

//    ShaderSetVec3(shader, "u_dirLight.direction", light.position);
//    ShaderSetVec3(shader, "u_dirLight.diffuse", light.diffuse);
//    ShaderSetVec3(shader, "u_dirLight.ambient", light.ambient);
//    ShaderSetVec3(shader, "u_dirLight.specular", light.specular);

//}
