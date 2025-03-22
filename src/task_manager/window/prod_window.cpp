#include "prod_window.h"

ProdWindow::ProdWindow()
{
}

void ProdWindow::render_window()
{
    ImGui::Begin("Prod", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("task"))
    {
    }

    ImGui::End();
}