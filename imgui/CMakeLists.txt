# ImGui library
add_library(imgui
	"imgui.h" "imgui.cpp"
	"imgui_internal.h"
	"imgui_draw.cpp"
	"imgui_tables.cpp" "imgui_widgets.cpp"
	"imstb_rectpack.h" "imstb_textedit.h" "imstb_truetype.h"
	"imgui_demo.cpp"
	"backends/imgui_impl_sdl3.h" "backends/imgui_impl_sdl3.cpp"
	"backends/imgui_impl_sdlrenderer3.h" "backends/imgui_impl_sdlrenderer3.cpp"
)
target_link_libraries(imgui PRIVATE SDL3::SDL3)
target_include_directories(imgui
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/backends
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h)

