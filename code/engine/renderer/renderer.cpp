#include "renderer.h"

#include "game.h"
#include "entity.h"
#include "debug.h"
#include "shader.h"
#include "line.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include <glm/vec4.hpp>
#include <GL/glew.h>

void RenderSceneEntities(Game *game)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i = 0; i < game->sceneEntities.size(); i++)
    {
        Entity *e = game->sceneEntities[i];

        bool isSelected = (game->selectedIDs.find(e->id) != game->selectedIDs.end());
        if((game->outlinePass && e->isSelectable && isSelected) || !game->outlinePass)
        {
            e->Render(e, game);
        }
    }
}

void RenderShadowPass(Game *game)
{
    game->shadowPass = true;
    glBindFramebuffer(GL_FRAMEBUFFER, game->shadowMapFbo.id);
    glViewport(0, 0, game->shadowMapFbo.depth.x, game->shadowMapFbo.depth.y);
    RenderSceneEntities(game);
    game->shadowPass = false;
}

void RenderOutlinePass(Game *game)
{
    game->outlinePass = true;
    glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);
    glViewport(0, 0, game->windowSize.x, game->windowSize.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineFbo.color.id, 0);
    RenderSceneEntities(game);
    game->outlinePass = false;
}

void RenderDebugGeometry(Game *game)
{
    if(game->renderPickingRay)
    {
        RenderLine(&game->pickingRay);
    }
    if(game->renderSelectionFrustum)
    {
        for(int i = 0; i < ArrayCount(game->frustumLines); i++)
        {
            RenderLine(&game->frustumLines[i]);
        }
        for(int i = 0; i < ArrayCount(game->frustumNormals); i++)
        {
            RenderLine(&game->frustumNormals[i]);
        }
    }

    if(game->renderShadowVolume)
    {
        for(int i = 0; i < 12; i++)
        {
            RenderLine(&game->shadowVolume[i]);
        }
    }
}

void RenderMainPass(Game *game)
{
    glPolygonMode(GL_FRONT_AND_BACK, game->polygonMode);

    glDepthMask(GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
    ShaderSetVec4(GetShader(game, "light_source"), "u_color", glm::vec4(1.0f));
    RenderSceneEntities(game);

    if(game->renderTerrain)
    {
        RenderTerrain(game);
    }

#if 0
    //Render grass
    UseShader(GetShader(game, "grass"));

    ShaderSetInt(GetShader(game, "grass"), "u_texture", 0);
    SetTexture(game->grass->material->diffuseTexture.id, 0);

    glBindVertexArray(game->grassQuad.vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, game->grassCount);
    glBindVertexArray(0);
#endif

    //Render skymap
    if(game->polygonMode == GL_FILL)
    {
        UseShader(GetShader(game, "skymap"));
        glDepthFunc(GL_LEQUAL);

        ShaderSetMatrix4(GetShader(game, "skymap"), "u_viewProjInverse", game->projViewInverse);

        SetTexture(game->skymapTexture.id, 0);
        ShaderSetInt(GetShader(game, "skymap"), "u_skyMap", 0);
        glBindVertexArray(game->fullscreenQuad.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDepthFunc(GL_LESS);
    }

    RenderDebugGeometry(game);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderParticlePass(Game *game)
{
    glBindFramebuffer(GL_FRAMEBUFFER, game->particlesFbo.id);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, game->particlesFbo.color.x, game->particlesFbo.color.y);

    SetTexture(&game->fullSceneDepthTexture, 2);
    ShaderSetInt(GetShader(game, "particle"), "u_sceneDepth", 2);
    ShaderSetVec2(GetShader(game, "particle"), "u_screenSize", game->particlesFbo.color.size);

    RenderParticles(game);

    glViewport(0, 0, game->windowSize.x, game->windowSize.y);
}

enum PostProcessTextureUnit
{
    PostProcTexUnit_Outline,
    PostProcTexUnit_Scene,
    PostProcTexUnit_Particles,
    PostProcTexUnit_SceneDepth,
    PostProcTexUnit_ParticlesDepth,
};

void RenderPostProcessing(Game *game)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SetTexture(&game->outlineFbo.color, PostProcTexUnit_Outline);
    ShaderSetInt(GetShader(game, "post_process"), "u_outline", PostProcTexUnit_Outline);

    SetTexture(&game->fullSceneTexture, PostProcTexUnit_Scene);
    ShaderSetInt(GetShader(game, "post_process"), "u_scene", PostProcTexUnit_Scene);

    SetTexture(&game->particlesFbo.color, PostProcTexUnit_Particles);
    ShaderSetInt(GetShader(game, "post_process"), "u_particles", PostProcTexUnit_Particles);

    SetTexture(&game->fullSceneDepthTexture, PostProcTexUnit_SceneDepth);
    ShaderSetInt(GetShader(game, "post_process"), "u_sceneDepth", PostProcTexUnit_SceneDepth);

    SetTexture(&game->particlesFbo.depth, PostProcTexUnit_ParticlesDepth);
    ShaderSetInt(GetShader(game, "post_process"), "u_smokeDepth", PostProcTexUnit_ParticlesDepth);

    ShaderSetVec2(GetShader(game, "post_process"), "u_lowResInvSize", 1.0f / (glm::vec2)game->particlesFbo.color.size);

    glBindVertexArray(game->fullscreenQuad.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderUI(Game *game)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(game->renderSelectionBox && game->input.mouseButtons[MOUSE_LEFT] && RECT_HAS_SIZE(game->selectionBox.size) &&
        !game->input.isMouseCapturedByImgui && !game->input.isCursorHidden && !game->editor.terrainGeneratorWindow)
    {
        RenderSelectionBox(game, &game->selectionBox);
    }

    if(game->renderCounters)
    {
        RenderText(&game->aliveParticlesText);
        RenderText(&game->deadParticlesText);
        RenderText(&game->fpsCounter);
        RenderText(&game->msPerFrame);
    }

    glDisable(GL_BLEND);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}