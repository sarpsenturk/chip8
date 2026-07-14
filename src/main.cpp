#include "chip8.hpp"
#include "keymap.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_internal.h>

#include <array>
#include <cstring>
#include <string>

static constexpr auto cycles_per_frame = 10;

// Default resolution is expressed as an integer scale factor applied to the
// Chip8's native display resolution (display_width x display_height).
static constexpr int default_scale = 15;

// Common scale presets shown in the "Display" menu.
static constexpr std::array<int, 8> scale_presets = {2, 4, 6, 8, 10, 12, 15, 20};

// Currently loaded ROM name/filepath
static char current_rom[512] = {};

// Filters for ROM selection dialog
static constexpr SDL_DialogFileFilter rom_filters[] = {
    {"CH8 ROM", "ch8"},
};

// Callback function on ROM load
void on_rom_loaded(const char* path, std::int32_t size)
{
    SDL_Log("Loaded ROM %s with size %d", path, size);
    const auto len = std::strlen(path);
    std::strncpy(current_rom, path, len);
    current_rom[len] = 0;
}

// SDL file dialog callback
void file_dialog_callback(void* userdata, const char* const* filelist, int filter)
{
    if (filelist == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "An error occured while opening a file: %s", SDL_GetError());
        return;
    } else if (*filelist == nullptr) {
        SDL_Log("File dialog was cancelled");
        return;
    }

    auto* chip8 = static_cast<Chip8*>(userdata);
    if (const auto size = chip8->load_rom(*filelist); size != -1) {
        on_rom_loaded(*filelist, size);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "An error occured while loading the ROM: %s", std::strerror(errno));
    }
}

int main(int argc, const char** argv)
{
    // Parse command line arguments. The window resolution is now chosen from
    // the UI, so the only optional argument left is a ROM to load at startup.
    if (argc != 2) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Usage: %s [ROM]\n", argv[0]);
        return 1;
    }
    const char* rom = argv[1];

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create window at a sensible default size (default_scale x the Chip8's
    // native display resolution), the user can change this later from the
    // "Display" menu.
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    const int initial_width = Chip8::display_width * default_scale;
    const int initial_height = Chip8::display_height * default_scale;
    SDL_Window* window = SDL_CreateWindow(
        "Chip8",
        initial_width * main_scale,
        initial_height * main_scale,
        SDL_WINDOW_RESIZABLE);

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);

    // Create chip8 framebuffer
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        Chip8::display_width,
        Chip8::display_height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    // Initialize interpreter
    Chip8 chip8;
    SDL_Log("Chip8 interpreter initialized");
    if (const auto size = chip8.load_rom(rom); size != -1) {
        on_rom_loaded(rom, size);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open ROM: %s\n", rom);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
    // io.ConfigDpiScaleFonts = true;        // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    // io.ConfigDpiScaleViewports = true;    // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // State for the "Custom..." resolution popup
    bool open_custom_resolution_popup = false;
    int custom_width = initial_width;
    int custom_height = initial_height;

    // Main loop
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    goto exit;
                case SDL_EVENT_KEY_DOWN: {
                    if (const auto key = keymap(event.key.key); key != -1) {
                        chip8.set_key_down(static_cast<std::uint8_t>(key));
                    }
                } break;
                case SDL_EVENT_KEY_UP: {
                    if (const auto key = keymap(event.key.key); key != -1) {
                        chip8.set_key_up(static_cast<std::uint8_t>(key));
                    }
                } break;
            }
        }

        for (int i = 0; i < cycles_per_frame; ++i) {
            chip8.cycle();
            if (chip8.draw_flag()) {
                break;
            }
        }
        chip8.tick_timers();

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Current window size, used to highlight the active resolution/preset
        int current_width = 0;
        int current_height = 0;
        SDL_GetWindowSize(window, &current_width, &current_height);

        // Submit menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load ROM")) {
                    SDL_ShowOpenFileDialog(
                        file_dialog_callback,
                        &chip8,
                        window,
                        &rom_filters[0],
                        1,
                        nullptr,
                        false);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Display")) {
                for (const auto scale : scale_presets) {
                    const int preset_width = Chip8::display_width * scale;
                    const int preset_height = Chip8::display_height * scale;
                    const auto label = std::to_string(preset_width) + " x " + std::to_string(preset_height) +
                                       " (" + std::to_string(scale) + "x)";
                    const bool selected = current_width == preset_width && current_height == preset_height;
                    if (ImGui::MenuItem(label.c_str(), nullptr, selected)) {
                        SDL_SetWindowSize(window, preset_width, preset_height);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Custom...")) {
                    custom_width = current_width;
                    custom_height = current_height;
                    open_custom_resolution_popup = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Exit")) {
                goto exit;
            }
            ImGui::EndMainMenuBar();
        }

        // Custom resolution popup
        if (open_custom_resolution_popup) {
            ImGui::OpenPopup("Custom Resolution");
            open_custom_resolution_popup = false;
        }
        if (ImGui::BeginPopupModal("Custom Resolution", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputInt("Width", &custom_width);
            ImGui::InputInt("Height", &custom_height);
            custom_width = custom_width < 1 ? 1 : custom_width;
            custom_height = custom_height < 1 ? 1 : custom_height;

            if (ImGui::Button("Apply")) {
                SDL_SetWindowSize(window, custom_width, custom_height);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Submit dockspace
        ImGuiID dockspace_id = ImGui::GetID("Dockspace");
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        // Create app layout
        if (ImGui::DockBuilderGetNode(dockspace_id)) {
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            ImGuiID dock_id_left = 0;
            ImGuiID dock_id_main = dockspace_id;
            ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.25f, &dock_id_left, &dock_id_main);
            ImGui::DockBuilderDockWindow("Display", dock_id_main);
            ImGui::DockBuilderDockWindow("State", dock_id_left);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        // Submit dockspace
        ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

        // Submit display window
        // Override docknode flags
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

        ImGui::SetNextWindowClass(&window_class);
        ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoDecoration);

        // Calculate size based on display aspect ratio
        const auto aspect = static_cast<float>(Chip8::display_width) / Chip8::display_height;
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x / size.y > aspect) {
            size.x = size.y * aspect;
        } else {
            size.y = size.x / aspect;
        }

        ImGui::Image(texture, size);
        ImGui::End();

        // Submit debug window
        ImGui::Begin("State");

        ImGui::Text("PROGRAM");
        ImGui::Text("%s", current_rom);
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

        // Sho demo window
        // ImGui::ShowDemoWindow();

        // Rendering
        SDL_RenderClear(renderer);

        SDL_UpdateTexture(texture, nullptr, chip8.pixels(), Chip8::display_pitch);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
    }

    // Destroy window & shutdown SDL
exit:
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
