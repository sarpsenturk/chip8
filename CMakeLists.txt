cmake_minimum_required(VERSION 3.25)

project(chip8 VERSION 0.1.0)

include(cmake/CPM.cmake)
CPMAddPackage("gh:libsdl-org/SDL#release-3.4.12")

add_subdirectory(imgui)

add_executable(chip8
	"src/main.cpp"
	"src/chip8.hpp" "src/chip8.cpp"
)
target_compile_features(chip8 PRIVATE cxx_std_20)
target_link_libraries(chip8 SDL3::SDL3 imgui)

