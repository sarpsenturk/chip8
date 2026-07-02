#pragma once

#include <array>
#include <cstdint>

class Chip8
{
public:
    using OpFunc = void (Chip8::*)();

    static constexpr auto max_stack_depth = 16;

    static constexpr auto display_width = 64;
    static constexpr auto display_height = 32;

    static constexpr auto fontset_start_addr = 0x50;

    Chip8();

    std::uint16_t fetch();     // Fetch the next instruction into the instruction register
    OpFunc decode();           // Decode the instruction and return the op function pointer
    void execute(OpFunc func); // Execute the given instruction function
    void cycle();              // Execute 1 CPU cycle

private:
    void op_00E0();    // Clear the display
    void op_00EE();    // Return from a subroutine
    void op_1nnn();    // Jump to location nnn
    void op_2nnn();    // Call subroutine at nnn
    void op_3xkk();    // Skip next instruction if Vx = kk
    void op_4xkk();    // Skip next instruction if Vx != kk
    void op_5xy0();    // Skip next instruction if Vx = Vy
    void op_6xkk();    // Set Vx = kk
    void op_7xkk();    // Set Vx = Vx + kk
    void op_8xy0();    // Set Vx = Vy
    void op_8xy1();    // Set Vx = Vx OR Vy
    void op_8xy2();    // Set Vx = Vx AND Vy
    void op_8xy3();    // Set Vx = Vx XOR Vy
    void op_8xy4();    // Set Vx = Vx + Vy, set VF = carry
    void op_8xy5();    // Set Vx = Vx - Vy, set VF = NOT borrow
    void op_8xy6();    // Set Vx = Vx SHR 1
    void op_8xy7();    // Set Vx = Vy - Vx, set VF = NOT borrow
    void op_8xyE();    // Set Vx = Vx SHL 1
    void op_9xy0();    // Skip next instruction if Vx != Vy
    void op_Annn();    // Set I = nnn
    void op_Bnnn();    // Jump to location nnn + V0
    void op_Cxkk();    // Set Vx = random byte AND kk
    void op_Dxyn();    // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
    void op_Ex9E();    // Skip next instruction if key with the value of Vx is pressed
    void op_ExA1();    // Skip next instruction if key with the value of Vx is not pressed
    void op_Fx07();    // Set Vx = delay timer value
    void op_Fx0A();    // Wait for a key press, store the value of the key in Vx
    void op_Fx15();    // Set delay timer = Vx
    void op_Fx18();    // Set sound timer = Vx
    void op_Fx1E();    // Set I = I + Vx
    void op_Fx29();    // Set I = location of sprite for digit Vx
    void op_Fx33();    // Store BCD representation of Vx in memory locations I, I+1, and I+2
    void op_Fx55();    // Store registers V0 through Vx in memory starting at location I
    void op_Fx65();    // Read registers V0 through Vx from memory starting at location I
    void op_INVALID(); // Invalid instruction error

    // A 12-bit value, the lowest 12 bits of the instruction
    [[nodiscard]] std::uint16_t op_var_nnn() const noexcept;
    // A 4-bit value, the lowest 4 bits of the instruction
    [[nodiscard]] std::uint8_t op_var_n() const noexcept;
    // A 4-bit value, the lower 4 bits of the high byte of the instruction
    [[nodiscard]] std::uint8_t op_var_x() const noexcept;
    // A 4-bit value, the upper 4 bits of the low byte of the instruction
    [[nodiscard]] std::uint8_t op_var_y() const noexcept;
    // An 8-bit value, the lowest 8 bits of the instruction
    [[nodiscard]] std::uint8_t op_var_kk() const noexcept;

    // Pops and returns an address from the stack
    std::uint16_t stack_pop();
    // Pushes an address onto the stack
    void stack_push(std::uint16_t value);

    std::array<std::uint8_t, 4096> memory_;                             // Random access memory 4kb
    std::array<std::uint8_t, 16> V_;                                    // General purpose registers V0-VF
    std::uint16_t I_;                                                   // Index register
    std::uint8_t dt_;                                                   // Delay timer
    std::uint8_t st_;                                                   // Sound timer
    std::uint16_t pc_;                                                  // Program counter
    std::uint16_t instruction_;                                         // Current instruction
    std::uint8_t sp_;                                                   // Stack pointer
    std::array<std::uint16_t, max_stack_depth> stack_;                  // Subroutine stack
    std::array<std::uint8_t, 16> keypad_;                               // 16-key hexadecimal keypad
    std::array<std::uint32_t, display_width * display_height> display_; // Monochrome display
};
