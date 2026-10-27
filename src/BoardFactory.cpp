#include "BoardFactory.h"

#include <cmath>
#include <stdexcept>
#include <vector>

bool BoardFactory::supportsSize(int size) noexcept {
    return size == 7 || size == 9;
}

Board BoardFactory::create(BoardType type, int size) {
    if (!supportsSize(size)) {
        throw std::invalid_argument("Supported board sizes are 7 and 9");
    }
    if (type != BoardType::English && type != BoardType::Hexagon &&
        type != BoardType::Diamond) {
        throw std::invalid_argument("Unsupported board type");
    }

    const int center = size / 2;
    std::vector<std::vector<Cell>> cells(
        size, std::vector<Cell>(size, Cell::Invalid));

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            const int dr = row - center;
            const int dc = col - center;
            bool valid = false;
            switch (type) {
                case BoardType::English:
                    // A plus-shaped board with three-cell-wide arms.
                    valid = std::abs(dr) <= 1 || std::abs(dc) <= 1;
                    break;
                case BoardType::Hexagon:
                    // Axial-coordinate hexagon represented on a square grid.
                    valid = std::abs(dr + dc) <= center;
                    break;
                case BoardType::Diamond:
                    valid = std::abs(dr) + std::abs(dc) <= center;
                    break;
            }
            if (valid) cells[row][col] = Cell::Peg;
        }
    }
    cells[center][center] = Cell::Empty;
    return Board(size, type, std::move(cells));
}
