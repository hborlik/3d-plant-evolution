#include <debug.h>
#include <renderer.h>
#include <resource.h>
#include <physics.h>

#include <imgui.h>

static bool enable_physics_timestep = true;

void show_material_editor_window() {
    ImGui::Begin("Material Editor");
    for (auto& mas : ev2::ResourceManager::get_singleton().get_materials()) {
        if (ImGui::CollapsingHeader(("Material " + mas.second->get_material()->name + " " + mas.first).c_str())) {
            
            if (ImGui::TreeNode("Color")) {
                ImGui::ColorPicker3("diffuse", glm::value_ptr(mas.second->get_material()->diffuse), ImGuiColorEditFlags_InputRGB);
                ImGui::TreePop();
            }
            ImGui::DragFloat("metallic",    &mas.second->get_material()->metallic, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("subsurface",  &mas.second->get_material()->subsurface, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("specular",    &mas.second->get_material()->specular, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("roughness",   &mas.second->get_material()->roughness, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("specularTint",&mas.second->get_material()->specularTint, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("clearcoat",   &mas.second->get_material()->clearcoat, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("clearcoatGloss", &mas.second->get_material()->clearcoatGloss, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("anisotropic", &mas.second->get_material()->anisotropic, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("sheen",       &mas.second->get_material()->sheen, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
            ImGui::DragFloat("sheenTint",   &mas.second->get_material()->sheenTint, 0.01f, 0.0f, 1.0f, "%.3f", 1.0f);
        }
    }
    ImGui::End();
}

void show_settings_editor_window() {
    ImGui::Begin("Settings");
    ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::DragFloat("ssao radius", &(ev2::renderer::Renderer::get_singleton().ssao_radius), 0.01f, 0.0f, 3.0f, "%.3f", 1.0f);
    ImGui::DragInt("ssao samples", (int32_t*)&(ev2::renderer::Renderer::get_singleton().ssao_kernel_samples), 1, 1, 64);
    ImGui::DragFloat("Exposure", &(ev2::renderer::Renderer::get_singleton().exposure), 0.01f, 0.05f, 1.0f, "%.3f", 1.0f);
    ImGui::DragFloat("Gamma", &(ev2::renderer::Renderer::get_singleton().gamma), 0.01f, 0.8f, 2.8f, "%.1f", 1.0f);
    ImGui::DragFloat("Shadow Bias World", &(ev2::renderer::Renderer::get_singleton().shadow_bias_world), 0.005f, 0.0001f, 1.0f, "%.5f", 1.0f);
    ImGui::Separator();
    ImGui::Text("World");
    if (ImGui::Checkbox("Enable Physics Timestep", &enable_physics_timestep)) {
        ev2::Physics::get_singleton().enable_simulation(enable_physics_timestep);
    }
    ImGui::End();
}

void show_game_debug_window(GameState* game) {
    ImGui::Begin("Game Settings");
    ImGui::Text("Time %.3f", game->time_day);
    ImGui::DragFloat("Time Speed", &(game->time_speed), 0.01f, 0.05f, 5.0f, "%.3f", 1.0f);
    ImGui::End();
}