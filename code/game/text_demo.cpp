#include "text_demo.h"

#include "game.h"

void InitTextDemo(Game *game)
{
    TextDemo *demo = &game->textDemo;

    demo->font = game->fonts[36];
    demo->font2 = game->fonts[18];

    char *text1 = "AAAaqApAPA aaapaIp I BBBb )(";
    char *text3 = "kerning This is just a bunch of text here";

    demo->textTest = CreateText(&demo->font, text1, glm::vec2(0.0f, 200.0f), game->uiTextShader, glm::vec3(1.0f, 0.0f, 0.0f));
    demo->textTest2 = CreateText(&demo->font, text3, glm::vec2(0.0f, 280.0f), game->uiTextShader, glm::vec3(1.0f));

    demo->helloWorldsCounterDisplay = CreateText(&demo->font2, "0 (hello worlds)", glm::vec2(300.0f, 36.0f), game->uiTextShader);

    demo->textInput = CreateText(&demo->font, (char *)demo->textInputBuffer.c_str(), glm::vec2(10.0f,  400.0f),
                                        game->uiTextShader, glm::vec3(1.0f));

    demo->textStatus = CreateText(&demo->font, "Text input: disabled", glm::vec2(WINDOW_WIDTH - 500.0f,  36.0f),
                                  game->uiTextShader, glm::vec3(1.0f));

    demo->textSample = "again place well so she change what out tell against know line stand it end like home hold develop while under tell such large move some it those mean many even school by can give keep seem out large such system have feel use keep here this know like";

    demo->inputTimer = StartTimer(15);
}

void UpdateTextDemo(Game *game)
{
    TextDemo *demo = &game->textDemo;
    Input *input = &game->input;

    if(!demo->isInitialized)
    {
        InitTextDemo(game);
        demo->isInitialized = true;
    }

    if(IsFirstPress(game, SDL_SCANCODE_F1))
    {
        demo->typingText = !demo->typingText;

        char *status;
        if(demo->typingText)
        {
            SDL_StartTextInput(game->window);
            status = "Text input: enabled";

            demo->textChanged = true;
        }
        else
        {
            SDL_StopTextInput(game->window);
            status = "Text input: disabled";
        }

        UpdateText(&demo->textStatus, status);
    }

    if(input->isBackspacePressed && game->textDemo.typingText && game->textDemo.textInputBuffer.length())
    {
        game->textDemo.textChanged = true;
        game->textDemo.textInputBuffer.pop_back();
    }

    if(input->typedText.length())
    {
        game->textDemo.textChanged = true;
        game->textDemo.textInputBuffer += input->typedText;
    }

    if(input->keys[SDL_SCANCODE_UP])
    {
        for(int i = 0; i < 100; i++)
        {
            demo->helloWorldsBuffer += "Hello, World! ";

            if(demo->helloWorldsCounter == 0)
            {
                demo->helloWorlds = CreateText(&game->fonts[36], &demo->helloWorldsBuffer[0],
                                               glm::vec2(0.0f, 400.0f), game->uiTextShader);
            }

            demo->helloWorldsCounter++;

        }

        UpdateText(&demo->helloWorlds, &demo->helloWorldsBuffer[0]);

        char buffer[20];
        sprintf(buffer, "%d (hello worlds)", demo->helloWorldsCounter);
        UpdateText(&demo->helloWorldsCounterDisplay, buffer);
    }
    if(input->keys[SDL_SCANCODE_DOWN])
    {
        for(int i = 0; i < 100; i++)
        {
            int textsCount = demo->helloWorldsCounter;
            if(textsCount)
            {
                demo->helloWorldsCounter--;

                std::string s = "Hello, World! ";
                demo->helloWorldsBuffer.erase(demo->helloWorldsBuffer.length() - s.length());
            }

        }

        UpdateText(&demo->helloWorlds, (char *)demo->helloWorldsBuffer.c_str());

        char buffer[20];
        sprintf(buffer, "%d (hello worlds)", demo->helloWorldsCounter);
        UpdateText(&demo->helloWorldsCounterDisplay, buffer);
    }

    if(!demo->typingText)
    {
        UpdateTimer(&demo->inputTimer, game->deltaTime);
        demo->textChanged = true;
    }

    if(demo->textChanged)
    {
        demo->textChanged = false;
        if(demo->typingText)
        {
            UpdateText(&demo->textInput, &demo->textInputBuffer[0]);
        }
        else
        {
            int lettersOut = (int)(demo->textSample.length() * (demo->inputTimer.elapsed / demo->inputTimer.duration));
            UpdateText(&demo->textInput, &demo->textSample[0], lettersOut);
        }
    }

    if(demo->inputTimer.isFinished)
    {
        if(demo->pauseTimer.isFinished)
            demo->pauseTimer = StartTimer(3);
        else
            UpdateTimer(&demo->pauseTimer, game->deltaTime);

        if(demo->pauseTimer.isFinished)
            demo->inputTimer = StartTimer(15);
    }
}

void RenderTextDemo(Game *game)
{
    TextDemo *demo = &game->textDemo;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    RenderText(&demo->textTest);
    RenderText(&demo->textTest2);

    RenderText(&demo->helloWorldsCounterDisplay);

    if(demo->helloWorldsCounter)
    {
        RenderText(&demo->helloWorlds);
    }

    RenderText(&demo->textStatus);
    RenderText(&demo->textInput);

    RenderText(&game->fpsCounter);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}