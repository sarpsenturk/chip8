#include "chip8.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <fstream>
#include <functional>
#include <random>
#include <stdexcept>

// Sprite data for the builtin fontset
static constexpr auto fontset = std::array{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Random number generator
std::uint8_t random_byte()
{
    static std::random_device rd;
    static std::default_random_engine re(rd());
    static std::uniform_int_distribution<std::uint8_t> dist(0u, 255u);
    return dist(re);
}

// Log a message to stdout
template<typename... Args>
void debug_log(std::format_string<Args...> fmt, Args&&... args)
{
    const auto msg = std::format(fmt, std::forward<Args>(args)...);
    std::printf("-- %s\n", msg.c_str());
}

Chip8::Chip8()
{
    debug_log("Chip8 interpreter");
    std::memcpy(memory_.data() + fontset_start_addr, fontset.data(), 80);
    debug_log("Fontset loaded into memory at {:#x}", fontset_start_addr);
    debug_log("PC = {:#x}", pc_);
}

bool Chip8::load_rom(const char* path)
{
    auto file = std::ifstream(path, std::ios::binary | std::ios::ate);
    if (file.good()) {
        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        if (file.read(reinterpret_cast<char*>(memory_.data() + prog_start_addr), size)) {
            debug_log("Loaded ROM {} with size {}", path, static_cast<int>(size));
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

std::uint16_t Chip8::fetch()
{
    const auto high = memory_[pc_];
    const auto low = memory_[pc_ + 1];
    return instruction_ = static_cast<std::uint16_t>((high << 8u) | low);
}

Chip8::OpFunc Chip8::decode()
{
    switch ((instruction_ & 0xf000) >> 12u) { // Switch on the high nibble
        case 0:
            if (instruction_ == 0x00E0) {
                return &Chip8::op_00E0;
            }
            if (instruction_ == 0x00EE) {
                return &Chip8::op_00EE;
            }
            debug_log("invalid instruction: {:#x}", instruction_);
            return &Chip8::op_INVALID;
        case 1:
            return &Chip8::op_1nnn;
        case 2:
            return &Chip8::op_2nnn;
        case 3:
            return &Chip8::op_3xkk;
        case 4:
            return &Chip8::op_4xkk;
        case 5:
            return &Chip8::op_5xy0;
        case 6:
            return &Chip8::op_6xkk;
        case 7:
            return &Chip8::op_7xkk;
        case 8:
            switch (instruction_ & 0xf) { // switch on the low nibble
                case 0:
                    return &Chip8::op_8xy0;
                case 1:
                    return &Chip8::op_8xy1;
                case 2:
                    return &Chip8::op_8xy2;
                case 3:
                    return &Chip8::op_8xy3;
                case 4:
                    return &Chip8::op_8xy4;
                case 5:
                    return &Chip8::op_8xy5;
                case 6:
                    return &Chip8::op_8xy6;
                case 7:
                    return &Chip8::op_8xy7;
                case 0xE:
                    return &Chip8::op_8xyE;
                default:
                    return &Chip8::op_INVALID;
            }
        case 9:
            return &Chip8::op_9xy0;
        case 0xA:
            return &Chip8::op_Annn;
        case 0xB:
            return &Chip8::op_Bnnn;
        case 0xC:
            return &Chip8::op_Cxkk;
        case 0xD:
            return &Chip8::op_Dxyn;
        case 0xE:
            switch (instruction_ & 0xff) { // switch on low byte
                case 0x9E:
                    return &Chip8::op_Ex9E;
                case 0xA1:
                    return &Chip8::op_ExA1;
                default:
                    return &Chip8::op_INVALID;
            }
        case 0xF:
            switch (instruction_ & 0xff) { // switch on low byte
                case 0x07:
                    return &Chip8::op_Fx07;
                case 0x0A:
                    return &Chip8::op_Fx0A;
                case 0x15:
                    return &Chip8::op_Fx15;
                case 0x18:
                    return &Chip8::op_Fx18;
                case 0x1E:
                    return &Chip8::op_Fx1E;
                case 0x29:
                    return &Chip8::op_Fx29;
                case 0x33:
                    return &Chip8::op_Fx33;
                case 0x55:
                    return &Chip8::op_Fx55;
                case 0x65:
                    return &Chip8::op_Fx65;
                default:
                    return &Chip8::op_INVALID;
            }
        default:
            return &Chip8::op_INVALID;
    }
}

void Chip8::execute(OpFunc func)
{
    std::invoke(func, this);
}

void Chip8::cycle()
{
    // Fetch current instruction
    const auto opcode = fetch();

    // Increment program counter
    pc_ += 2;

    // Decode opcode
    const auto op_func = decode();

    // Execute instruction
    execute(op_func);

    // Decrement timers if active
    if (dt_ > 0) {
        --dt_;
    }
    if (st_ > 0) {
        --st_;
    }
}

void Chip8::op_00E0()
{
    std::memset(display_.data(), 0, sizeof(display_));
}

void Chip8::op_00EE()
{
    pc_ = stack_pop();
}

void Chip8::op_1nnn()
{
    pc_ = op_var_nnn();
}

void Chip8::op_2nnn()
{
    stack_push(pc_);
    pc_ = op_var_nnn();
}

void Chip8::op_3xkk()
{
    const auto x = op_var_x();
    const auto kk = op_var_kk();
    if (V_[x] == kk) {
        pc_ += 2;
    }
}

void Chip8::op_4xkk()
{
    const auto x = op_var_x();
    const auto kk = op_var_kk();
    if (V_[x] != kk) {
        pc_ += 2;
    }
}

void Chip8::op_5xy0()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    if (V_[x] == V_[y]) {
        pc_ += 2;
    }
}

void Chip8::op_6xkk()
{
    const auto x = op_var_x();
    const auto kk = op_var_kk();
    V_[x] = kk;
}

void Chip8::op_7xkk()
{
    const auto x = op_var_x();
    const auto kk = op_var_kk();
    V_[x] += kk;
}

void Chip8::op_8xy0()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    V_[x] = V_[y];
}

void Chip8::op_8xy1()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    V_[x] |= V_[y];
    V_[0xf] = 0;
}

void Chip8::op_8xy2()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    V_[x] &= V_[y];
    V_[0xf] = 0;
}

void Chip8::op_8xy3()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    V_[x] ^= V_[y];
    V_[0xf] = 0;
}

void Chip8::op_8xy4()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    const auto result = static_cast<std::uint16_t>(V_[x] + V_[y]);
    V_[x] = static_cast<std::uint8_t>(result);
    V_[0xf] = result > 0xff;
}

void Chip8::op_8xy5()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    const auto result = static_cast<std::uint8_t>(V_[x] - V_[y]);
    const auto flag = static_cast<std::uint8_t>(V_[x] >= V_[y]);
    V_[x] = result;
    V_[0xf] = flag;
}

void Chip8::op_8xy6()
{
    const auto x = op_var_x();
    const auto result = V_[x] >> 1u;
    const auto flag = V_[x] & 1u;
    V_[x] = result;
    V_[0xf] = flag;
}

void Chip8::op_8xy7()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    const auto result = static_cast<std::uint8_t>(V_[y] - V_[x]);
    const auto flag = static_cast<std::uint8_t>(V_[y] >= V_[x]);
    V_[x] = result;
    V_[0xf] = flag;
}

void Chip8::op_8xyE()
{
    const auto x = op_var_x();
    const auto result = V_[x] << 1u;
    const auto flag = (V_[x] & 0x80) >> 7u;
    V_[x] = result;
    V_[0xf] = flag;
}

void Chip8::op_9xy0()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    if (V_[x] != V_[y]) {
        pc_ += 2;
    }
}

void Chip8::op_Annn()
{
    I_ = op_var_nnn();
}

void Chip8::op_Bnnn()
{
    pc_ = static_cast<std::uint16_t>(op_var_nnn() + V_[0]);
}

void Chip8::op_Cxkk()
{
    const auto x = op_var_x();
    const auto kk = op_var_kk();
    const auto random = random_byte();
    V_[x] = random & kk;
}

void Chip8::op_Dxyn()
{
    const auto x = op_var_x();
    const auto y = op_var_y();
    const auto n = op_var_n();

    // Wrap sprite positions
    const auto xpos = V_[x] % display_width;
    const auto ypos = V_[y] % display_height;

    // Reset Vf until a collision occurs
    V_[0xf] = 0;

    for (std::uint8_t row = 0; row < n; ++row) {
        const auto sprite = memory_[I_ + row];
        for (std::uint8_t col = 0; col < 8; ++col) {
            const auto src = sprite & (0x80 >> col);
            auto& dst = display_[(ypos + row) * display_width + (xpos + col)];
            if (src) {
                if (dst) {
                    V_[0xf] = 1;
                }
                dst ^= 0xffffffff;
            }
        }
    }
}

void Chip8::op_Ex9E()
{
    const auto x = op_var_x();
    if (keypad_[V_[x]]) {
        pc_ += 2;
    }
}

void Chip8::op_ExA1()
{
    const auto x = op_var_x();
    if (!keypad_[V_[x]]) {
        pc_ += 2;
    }
}

void Chip8::op_Fx07()
{
    const auto x = op_var_x();
    V_[x] = dt_;
}

void Chip8::op_Fx0A()
{
    const auto x = op_var_x();
    for (std::uint8_t key = 0; key < keypad_.size(); ++key) {
        if (keypad_[key]) {
            V_[x] = key;
            return;
        }
    }
    pc_ -= 2; // 'Wait' until a key press
}

void Chip8::op_Fx15()
{
    const auto x = op_var_x();
    dt_ = V_[x];
}

void Chip8::op_Fx18()
{
    const auto x = op_var_x();
    st_ = V_[x];
}

void Chip8::op_Fx1E()
{
    const auto x = op_var_x();
    I_ += V_[x];
}

void Chip8::op_Fx29()
{
    const auto x = op_var_x();
    I_ = static_cast<std::uint16_t>(fontset_start_addr + V_[x] * 5);
}

void Chip8::op_Fx33()
{
    const auto x = op_var_x();
    auto value = V_[x];
    memory_[I_ + 2] = value % 10;
    value /= 10;
    memory_[I_ + 1] = value % 10;
    value /= 10;
    memory_[I_] = value % 10;
}

void Chip8::op_Fx55()
{
    const auto x = op_var_x();
    for (std::uint8_t i = 0; i <= x; ++i) {
        memory_[I_ + i] = V_[i];
    }
}

void Chip8::op_Fx65()
{
    const auto x = op_var_x();
    for (std::uint8_t i = 0; i <= x; ++i) {
        V_[i] = memory_[I_ + i];
    }
}

void Chip8::op_INVALID()
{
    throw std::runtime_error("Invalid instruction");
}

std::uint16_t Chip8::op_var_nnn() const noexcept
{
    return instruction_ & 0xfff;
}

std::uint8_t Chip8::op_var_n() const noexcept
{
    return instruction_ & 0xf;
}

std::uint8_t Chip8::op_var_x() const noexcept
{
    return (instruction_ >> 8u) & 0xf;
}

std::uint8_t Chip8::op_var_y() const noexcept
{
    return (instruction_ >> 4u) & 0xf;
}

std::uint8_t Chip8::op_var_kk() const noexcept
{
    return static_cast<std::uint8_t>(instruction_ & 0xff);
}

std::uint16_t Chip8::stack_pop()
{
    assert(sp_ > 0 && "Cannot pop the stack when it's empty");
    --sp_;
    return stack_[sp_];
}

void Chip8::stack_push(std::uint16_t value)
{
    assert(sp_ < max_stack_depth && "Stack has reached max depth");
    stack_[sp_] = value;
    ++sp_;
}
