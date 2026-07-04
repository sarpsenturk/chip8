#include "ui.hpp"
#include "chip8.hpp"

#include <imgui.h>
#include <imgui_internal.h>

void ImGui_Chip8_MainMenuBar(const MainMenuState& state)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM")) {
                SDL_ShowOpenFileDialog(
                    state.file_dialog_callback,
                    state.userdata,
                    state.window,
                    state.filters,
                    state.nfilters,
                    nullptr,
                    false);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void ImGui_Chip8_SetupLayout()
{
    ImGuiID dockspace_id = ImGui::GetID("Dockspace");
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Create app layout
    if (ImGui::DockBuilderGetNode(dockspace_id)) {
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
        ImGuiID dock_id_left = 0;
        ImGuiID dock_id_main = dockspace_id;
        ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
        ImGui::DockBuilderDockWindow("Display", dock_id_main);
        ImGui::DockBuilderDockWindow("State", dock_id_left);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    // Submit dockspace
    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
}

void ImGui_Chip8_DisplayWindow(const char* name, ImTextureID display, float aspect)
{
    // Override docknode flags
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    ImGui::SetNextWindowClass(&window_class);
    ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoDecoration);

    // Calculate size based on display aspect ratio
    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x / size.y > aspect) {
        size.x = size.y * aspect;
    } else {
        size.y = size.x / aspect;
    }

    ImGui::Image(display, size);
    ImGui::End();
}

void ImGui_Chip8_StateWindow(const char* name, const class Chip8& chip8)
{ // Override docknode flags
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    ImGui::Begin(name);

    ImGui::Text("PROGRAM");
    ImGui::Text("PC = %#04x", chip8.program_counter());

    ImGui::Separator();

    ImGui::Text("REGISTERS");
    ImGui::Text("Index = %u", chip8.index_register());
    for (std::uint8_t x = 0; x < 8; ++x) {
        ImGui::Text("V%x = %u", x, chip8.registers(x));
        ImGui::SameLine(150.f);
        ImGui::Text("V%x = %u", x + 8, chip8.registers(x + 8));
    }

    ImGui::Separator();

    ImGui::Text("TIMERS");
    ImGui::Text("DT = %u", chip8.delay_timer());
    ImGui::SameLine(150.f);
    ImGui::Text("ST = %u", chip8.sound_timer());

    ImGui::End();
}
