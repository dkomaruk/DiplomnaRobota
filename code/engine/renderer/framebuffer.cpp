#include "framebuffer.h"

#include "game.h"

void SetupFramebuffers(Game *game)
{
    //Framebuffer
    //https://www.reddit.com/r/GraphicsProgramming/comments/jwkpju/what_is_the_best_way_to_approach_a_multi_pass/

    //TODO: Multiple render targets, render picking and outline textures using one framebuffer and one render pass
    //Full scene textures are created separately because I use the same framebuffer for outlines and full render and just swap out the textures
    game->outlineFbo = CreateFramebuffer(game->windowSize, FboTexturePreset_ColorLinearRGB, 0);
    game->fullSceneTexture = CreateGLTexture(NULL, game->windowSize.x, game->windowSize.y,
                                             FboTexturePreset_ColorLinearRGB);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
    game->fullSceneDepthTexture = CreateGLTexture(NULL, game->windowSize.x, game->windowSize.y,
                                                  FboTexturePreset_Depth32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, game->fullSceneDepthTexture.id, 0);

    game->particlesFbo = CreateFramebuffer(RECT_HALF(game->windowSize), FboTexturePreset_ColorLinearRGBA,
                                           FboTexturePreset_Depth32F);

    //game->shadowMapFbo = CreateFramebuffer(glm::ivec2(1024, 1024), 0,
    //                                       FboTexturePreset_Depth32F | TextureFlag_ClampToBorder);
    game->shadowMapFbo = CreateFramebuffer(glm::ivec2(8192, 8192), 0,
                                           FboTexturePreset_Depth32F | TextureFlag_ClampToBorder);

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
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if(depthFlags)
    {
        fbo.depth = CreateGLTexture(NULL, size.x, size.y, depthFlags);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo.depth.id, 0);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
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