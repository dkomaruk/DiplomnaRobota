#include "editor_ui.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

void UpdateEditorUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("ImguiOK");
    ImGui::Text("AAAAAAAAAAAAAAAAAAAAAAAAAAA");
    ImGui::End();
}