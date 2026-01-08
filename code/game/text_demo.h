#ifndef TEXT_DEMO_H

#include "text.h"
#include "timer.h"

struct Game;

struct TextDemo
{
    bool isInitialized;

    Font font;
    Font font2;

    std::string textSample;

    bool typingText;
    Text textStatus;

    bool textChanged;
    std::string textInputBuffer = "Type here: ";
    Text textInput;

    Text helloWorldsCounterDisplay;

    int helloWorldsCounter = 0;
    std::string helloWorldsBuffer = "";
    Text helloWorlds;

    Text textTest;
    Text textTest2;

    Timer inputTimer;
    Timer pauseTimer;
};

void InitTextDemo(Game *game);

void UpdateTextDemo(Game *game);
void RenderTextDemo(Game *game);

#define TEXT_DEMO_H
#endif