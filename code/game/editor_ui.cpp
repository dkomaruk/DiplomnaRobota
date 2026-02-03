#include "editor_ui.h"

#include "particle_editor_ui.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <imgui_gradient/imgui_gradient.hpp>

void UpdateEditorUI(Game *game)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    //ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 420.0f, 0.0f), ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(420.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
    UpdateParticleEditorUI(game);

    ImGui::Begin("Selected Entity", 0, ImGuiWindowFlags_AlwaysAutoResize);
    if(game->lastSelectedId > 0 && (game->sceneEntities.size() >= game->lastSelectedId))
    {
        Entity *selectedEntity = game->sceneEntities[game->lastSelectedId - 1];

        ImGui::LabelText("Text ID", "%s", selectedEntity->textId);
        ImGui::DragFloat3("Position", &selectedEntity->position[0], 0.1f);
        ImGui::DragFloat3("Rotation", &selectedEntity->rotation[0], 0.1f);
        ImGui::DragFloat3("Scale", &selectedEntity->scale[0], 0.1f);

        Model *model = selectedEntity->models[0];
        if(model->type == ModelType_Animated)
        {
            ImGui::Text("Animation details");
        }
    }

    ImGui::End();
}