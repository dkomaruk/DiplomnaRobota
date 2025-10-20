#include "external/stb_image.cpp"

#include "timer.cpp"
#include "camera.cpp"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <vector>

using namespace glm;

#ifdef WINDOW_TRANSPARENT
#define WINDOW_WIDTH 1920.0f
#define WINDOW_HEIGHT 1080.0f
#else
#define WINDOW_WIDTH 1280.0f
#define WINDOW_HEIGHT 720.0f
#endif

struct Engine
{
    SDL_Window *window;
    bool lockFPS;
};

bool Init(Engine *engine);

char *LoadShader(char *filepath);
GLuint CompileShader(char *shaderCode, GLenum shaderType);
GLuint CreateShaderProgram(char *vertexShaderCode, char *fragmentShaderCode);

GLuint CreateTexture(char *imagePath, int textureUnit);

int main(int argc, char *argv[])
{
    Engine engine = {};
    if(!Init(&engine))
    {
        return -1;
    }

    GLuint containerTexture = CreateTexture("../data/imgs/container.jpg", 0);
    GLuint faceTexture = CreateTexture("../data/imgs/face.png", 1);
    GLuint wallTexture = CreateTexture("../data/imgs/wall.jpg", 2);

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    float vertices2[] = {
       -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
       -0.5f, -0.5f, 0.0f,  0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0 * sizeof(float)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    GLuint vao2;
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLuint vbo2;
    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(0 * sizeof(float)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(3 * sizeof(float)));

    GLuint shaderProgram = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                               LoadShader("../data/shaders/fragment.frag"));

    GLuint animatedShaderProgram = CreateShaderProgram(LoadShader("../data/shaders/vertex.vert"),
                                                       LoadShader("../data/shaders/fragment_animated.frag"));


    Camera camera = {};
    camera.position = vec3(0.0f, 0.0f, -5.0f);
    camera.direction = vec3(0.0f, 0.0f, -1.0f);
    camera.up = vec3(0.0f, 1.0f, 0.0f);
    camera.speed = 10.0f;
    camera.sensititivy = 30.0f;

    mat4 projection = perspective(radians(camera.fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 100.0f);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_projection"), 1, GL_FALSE, value_ptr(projection));

    glUseProgram(animatedShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(animatedShaderProgram, "u_projection"), 1, GL_FALSE, value_ptr(projection));

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    std::vector<vec3> clearColors = {
        vec3(144.0f, 213.0f, 255.0f), vec3(173.0f, 216.0f, 230.0f), vec3(152.0f, 255.0f, 152.0f),
        vec3(245.0f, 245.0f, 220.0f), vec3(204.0f, 153.0f, 153.0f), vec3(255.0f, 218.0f, 185.0f),
        vec3(159.0f, 226.0f, 191.0f), vec3(230.0f, 230.0f, 250.0f), vec3(211.0f, 211.0f, 211.0f),
        vec3(150.0f, 120.0f, 96.0f), vec3(200.0f, 162.0f, 200.0f),
    };

    int clearColorIndex = SDL_rand((int)clearColors.size());

    float colorTimerDuration = 5.0f * 60.0f;
    Timer colorTimer = StartTimer(colorTimerDuration);

    Uint64 perfFreq = SDL_GetPerformanceFrequency();
    Uint64 lastFrame = 0;
    float deltaTime = 0.0f;

    //engine.lockFPS = true;

    bool *keyboardState = (bool *)SDL_GetKeyboardState(0);

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

                case SDL_EVENT_MOUSE_MOTION:
                {
                    SDL_MouseMotionEvent mouse = event.motion;

                    camera.yaw += mouse.xrel * camera.sensititivy * deltaTime;
                    camera.pitch -= mouse.yrel * camera.sensititivy * deltaTime;
                    camera.pitch = SDL_clamp(camera.pitch, camera.maxPitch.x, camera.maxPitch.y);

                    camera.direction.x = cos(radians(camera.yaw)) * cos(radians(camera.pitch));
                    camera.direction.y = sin(radians(camera.pitch));
                    camera.direction.z = sin(radians(camera.yaw)) * cos(radians(camera.pitch));
                    camera.direction = normalize(camera.direction);
                } break;

                case SDL_EVENT_MOUSE_WHEEL:
                {
                    SDL_MouseWheelEvent wheel = event.wheel;

                    camera.fov -= wheel.y;

                    camera.fov = SDL_clamp(camera.fov, 1.0f, 45.0f);

                    projection = perspective(radians(camera.fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);

                    glUseProgram(shaderProgram);
                    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_projection"),
                                       1, GL_FALSE, value_ptr(projection));

                } break;
            }
        }

        float cameraSpeed = camera.speed * deltaTime;
        if(keyboardState[SDL_SCANCODE_W])
            camera.position += cameraSpeed * camera.direction;
        if(keyboardState[SDL_SCANCODE_S])
            camera.position -= cameraSpeed * camera.direction;
        if(keyboardState[SDL_SCANCODE_A])
            camera.position -= normalize(cross(camera.direction, camera.up)) * cameraSpeed;
        if(keyboardState[SDL_SCANCODE_D])
            camera.position += normalize(cross(camera.direction, camera.up)) * cameraSpeed;

        UpdateTimer(&colorTimer, deltaTime);
        if(colorTimer.isFinished)
        {
            int lastIndex = clearColorIndex;
            while(lastIndex == clearColorIndex)
            {
                clearColorIndex = SDL_rand((int)clearColors.size());
            }

            colorTimer = StartTimer(colorTimerDuration);
        }

#ifndef WINDOW_TRANSPARENT
        glClearColor(clearColors[clearColorIndex].r / 255.0f,
                     clearColors[clearColorIndex].g / 255.0f,
                     clearColors[clearColorIndex].b / 255.0f, 0.0f);
#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_texture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_texture2"), 1);

        mat4 view = lookAt(camera.position, camera.position + camera.direction, vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_view"), 1, GL_FALSE, value_ptr(view));

        float time = SDL_GetTicks() / 1000.0f;

        glBindVertexArray(vao);
        for(int i = 0; i < 10; i++)
        {
            mat4 model = mat4(1.0f);
            model = translate(model, cubePositions[i]);
            model = translate(model, vec3(0.0f, sin(time) * 0.1f * i * sign(sin(time) * i),
                                                (sin(time * 0.6f) * 2.0f - 2.0f)));
            model = rotate(model, radians(time * 15.0f), (i != 0) ? normalize(cubePositions[i]) : vec3(0.0f, 1.0f, 0.0f));
            model = rotate(model, radians(20.0f * i), vec3(1.0f, 0.3f, 0.5f));

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_model"), 1, GL_FALSE, value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //float time = SDL_GetTicks() / 1000.0f;
        //mat4 model = mat4(1.0);
        ////model = translate(model, vec3(0.0f, 0.0f, (sin(time * 0.6f) * 2.0f - 5.0f)));
        //model = translate(model, vec3(0.0f, 0.0f, (sin(time * 0.6f) * 2.0f - 1.5f)));
        //model = rotate(model, radians(time * 15.0f), vec3(0.0f, 1.0f, 0.0f));
        //glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_model"), 1, GL_FALSE, value_ptr(model));
        //glUniform1f(glGetUniformLocation(shaderProgram, "u_mix"), 0.0f);
        //glBindVertexArray(vao);
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        //glBindVertexArray(vao2);
        //model = mat4(1.0f);
        //glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_model"), 1, GL_FALSE, value_ptr(model));
        //glUniform1f(glGetUniformLocation(shaderProgram, "u_mix"), 1.0f);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //model = mat4(1.0f);
        //model = translate(model, vec3(sin(time) / 2.0f, 0.0f, 0.0f));
        //model = rotate(model, SDL_GetTicks() / -1000.0f, vec3(0.0f, 0.0f, 1.0f));
        //model = scale(model, vec3(1.5f, 1.5f, 1.0f));
        //glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_model"), 1, GL_FALSE, value_ptr(model));
        //glUniform1f(glGetUniformLocation(shaderProgram, "u_mix"), 0.4f);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        SDL_GL_SwapWindow(engine.window);

        Uint64 thisFrame = SDL_GetPerformanceCounter();

        if(engine.lockFPS)
        {
            int targetFrames = 60;
            float targetTime = 1.0f / targetFrames;
            float elapsedWhileWaiting = 0.0f;
            while((elapsedWhileWaiting = (SDL_GetPerformanceCounter() - thisFrame) / (float)perfFreq) < targetTime)
            {
                SDL_Delay((uint32)((targetTime - elapsedWhileWaiting) * 1000));
            }

            thisFrame = SDL_GetPerformanceCounter();
        }

        deltaTime = (thisFrame - lastFrame) / (float)perfFreq;

        lastFrame = thisFrame;
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

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

#ifdef WINDOW_TRANSPARENT
    bool isBorderless = true;
    bool isTransparent = true;
#else
    bool isBorderless = false;
    bool isTransparent = false;
#endif

    Uint64 windowFlags = SDL_WINDOW_OPENGL |
                        (isBorderless ? SDL_WINDOW_TRANSPARENT : 0) |
                        (isTransparent ? SDL_WINDOW_TRANSPARENT : 0);

    engine->window = SDL_CreateWindow("Diplom", (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, windowFlags);
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

    SDL_Time ticks;
    SDL_GetCurrentTime(&ticks);
    SDL_srand(ticks);

    SDL_HideCursor();
    SDL_SetWindowRelativeMouseMode(engine->window, true);

    stbi_set_flip_vertically_on_load(true);

    //SDL_GL_SetSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

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


GLuint CreateTexture(char *imagePath, int textureUnit)
{
    int width, height, channels;
    int desiredChannels = 4;
    unsigned char *image = stbi_load(imagePath, &width, &height, &channels, desiredChannels);

    GLuint texture;
    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);

    return texture;
}