cmake_minimum_required(VERSION 3.25)

project(chip8 VERSION 0.1.0)

add_executable(chip8
	"src/main.cpp"
	"src/chip8.hpp" "src/chip8.cpp"
)
target_compile_features(chip8 PUBLIC cxx_std_20)
