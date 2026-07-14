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

#include <cstring>
#include <string>

static constexpr auto cycles_per_frame = 10;

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
    // Parse command line arguments
    if (argc != 4) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Usage: %s [width] [height] [ROM]\n", argv[0]);
        return 1;
    }
    const auto width = std::stoi(argv[1]);
    const auto height = std::stoi(argv[2]);
    const char* rom = argv[3];

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create window
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Window* window = SDL_CreateWindow(
        "Chip8",
        width * main_scale,
        height * main_scale,
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
        return 1;
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
            if (ImGui::MenuItem("Exit")) {
                goto exit;
            }
            ImGui::EndMainMenuBar();
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
