#include "shader.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>

char *LoadShader(char *filepath)
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

GLuint CreateShaderProgram(char *vertexShaderCode, char *fragmentShaderCode)
{
    GLuint vertexShader = CompileShader(vertexShaderCode, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(fragmentShaderCode, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

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

    free(vertexShaderCode);
    free(fragmentShaderCode);

    return shaderProgram;
}