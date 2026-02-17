#include "framebuffer.h"

#include "game.h"

void SetupFramebuffers(Game *game)
{
    //Framebuffer
    //https://www.reddit.com/r/GraphicsProgramming/comments/jwkpju/what_is_the_best_way_to_approach_a_multi_pass/

    //TODO: Multiple render targets, render picking and outline textures using one framebuffer and one render pass
    game->outlineFbo = CreateFramebuffer(glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT), FboTexturePreset_ColorLinearRGB, 0);
    game->fullSceneTexture = CreateGLTexture(NULL, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT,
                                             FboTexturePreset_ColorLinearRGB);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
    game->fullSceneDepthTexture = CreateGLTexture(NULL, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT,
                                                  FboTexturePreset_Depth32);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, game->fullSceneDepthTexture.id, 0);

    game->smokeFbo = CreateFramebuffer(glm::ivec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f),
                                       FboTexturePreset_ColorLinearRGBA, FboTexturePreset_Depth32);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer CreateFramebuffer(glm::ivec2 size, int colorFlags, int depthFlags)
{
    Framebuffer fbo = {};

    glGenFramebuffers(1, &fbo.id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);

    if(colorFlags)
    {
        fbo.color = CreateGLTexture(NULL, size.x, size.y, colorFlags);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.color.id, 0);
    }

    if(depthFlags)
    {
        fbo.depth = CreateGLTexture(NULL, size.x, size.y, depthFlags);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo.depth.id, 0);
    }

    if((colorFlags && depthFlags) && (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE))
    {
        SDL_Log("Framebuffer isn't complete");
    }

    return fbo;
}

void DeleteFramebuffer(Framebuffer *fbo)
{
    glDeleteFramebuffers(1, &fbo->id);
    glDeleteTextures(1, &fbo->color.id);
    glDeleteTextures(1, &fbo->depth.id);
}