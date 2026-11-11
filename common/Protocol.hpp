#pragma once
#include <cstdint>

enum class MessageType : uint8_t
{
    JOIN = 0,
    MATCH_FOUND,
    MOVE,
    BOARD_UPDATE,
    INVALID_MOVE,
    GAME_OVER,
    ERROR_MSG
};

struct MovePacket
{
    int32_t fromX;
    int32_t fromY;
    int32_t toX;
    int32_t toY;
};

struct MatchFoundPacket
{
    int32_t playerColor;
};

struct BoardUpdatePacket {
    uint8_t grid[8][8];
};