#include "chip8.hpp"

#include <cstdio>
#include <cstring>
#include <format>

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
}

void Chip8::fetch()
{
    const auto high = memory_[pc_];
    const auto low = memory_[pc_ + 1];
    instruction_ = static_cast<std::uint16_t>((high << 8u) | low);
    pc_ += 2;
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
    return instruction_ >> 8u & 0xf;
}

std::uint8_t Chip8::op_var_y() const noexcept
{
    return instruction_ >> 12u;
}

std::uint8_t Chip8::op_var_kk() const noexcept
{
    return static_cast<std::uint8_t>(instruction_ & 0xff);
}
