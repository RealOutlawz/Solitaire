#pragma once

#include "Board.h"

class BoardFactory {
public:
    static bool supportsSize(int size) noexcept;
    static Board create(BoardType type, int size);
};
