#pragma once

#include <array>

enum class Direction
{
    UpRight,
    Right,
    DownRight,
    DownLeft,
    Left,
    UpLeft,
};
static inline Direction operator+(const Direction& d, int n) { return static_cast<Direction>((static_cast<int>(d) + n % 6 + 6) % 6); }
static inline Direction operator-(const Direction& d, int n) { return static_cast<Direction>((static_cast<int>(d) - n % 6 + 6) % 6); }
static inline double toRotation(Direction d) {
    return static_cast<int>(d) * -60.0 + 60.0;
}
extern std::array<Direction, 6> allDirections;