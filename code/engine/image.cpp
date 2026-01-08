#include "image.h"

#include <SDL3/SDL.h>

bool SaveImage(char *filepath, void *pixels, int width, int height, int bytesPerPixel, bool flipY)
{
    bool success = true;

    if(flipY)
    {
        stbi_flip_vertically_on_write(true);
    }

    if(!stbi_write_png(filepath, width, height, bytesPerPixel, pixels, width * bytesPerPixel))
    {
        SDL_Log("Failed to save image at %s\n", filepath);
        success = false;
    }
    stbi_flip_vertically_on_write(false);

    return success;
}
