#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <stdio.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

struct Engine
{
    SDL_Window *window;
};

bool Init(Engine *engine);

char *LoadShader(char *filepath);
GLuint CompileShader(char *shaderCode, GLenum shaderType);
GLuint CreateShaderProgram(char *vertexShaderCode, char *fragmentShaderCode);

int main(int argc, char *argv[])
{
    Engine engine = {};
    if(!Init(&engine))
    {
        return -1;
    }

    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)(0 * sizeof(float)));

    GLuint shaderProgram = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                               LoadShader("../data/shaders/fragment.frag"));
    glUseProgram(shaderProgram);

    bool isRunning = true;
    while(isRunning)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_EVENT_QUIT:
                {
                    isRunning = false;
                } break;

                case SDL_EVENT_KEY_DOWN:
                {
                    if(event.key.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        isRunning = false;
                    }
                } break;
            }
        }

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(engine.window);
    }

    return 0;
}

bool Init(Engine *engine)
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL. Error: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    engine->window = SDL_CreateWindow("Diplom", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if(!engine->window)
    {
        SDL_Log("Failed to create a window. Error: %s", SDL_GetError());
        return false;
    }

    SDL_GLContext context = SDL_GL_CreateContext(engine->window);
    if(!context)
    {
        SDL_Log("Failed to create an OpenGL context. Error: %s", SDL_GetError());
        return false;
    }

    GLenum glewResult = glewInit();
    if(glewResult != GLEW_OK)
    {
        SDL_Log("Failed to initialize glew. Error: %s", glewGetErrorString(glewResult));
        return false;
    }

    return true;
}

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
