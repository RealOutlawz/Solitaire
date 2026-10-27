#include "Board.h"

#include <stdexcept>
#include <utility>

Board::Board(int size, BoardType type, std::vector<std::vector<Cell>> cells)
    : size_(size), type_(type), cells_(std::move(cells)) {
    if (size <= 0 || static_cast<int>(cells_.size()) != size) {
        throw std::invalid_argument("Board must have size rows");
    }
    for (const auto& row : cells_) {
        if (static_cast<int>(row.size()) != size) {
            throw std::invalid_argument("Board rows must have size columns");
        }
    }
}

bool Board::inBounds(Position p) const noexcept {
    return p.row >= 0 && p.row < size_ && p.col >= 0 && p.col < size_;
}

bool Board::isPlayable(Position p) const noexcept {
    return inBounds(p) && cells_[p.row][p.col] != Cell::Invalid;
}

bool Board::hasPeg(Position p) const noexcept {
    return inBounds(p) && cells_[p.row][p.col] == Cell::Peg;
}

Cell Board::at(Position p) const {
    if (!inBounds(p)) {
        throw std::out_of_range("Board position is outside the grid");
    }
    return cells_[p.row][p.col];
}

int Board::pegCount() const noexcept {
    int count = 0;
    for (const auto& row : cells_) {
        for (Cell cell : row) {
            if (cell == Cell::Peg) ++count;
        }
    }
    return count;
}

int Board::playableCount() const noexcept {
    int count = 0;
    for (const auto& row : cells_) {
        for (Cell cell : row) {
            if (cell != Cell::Invalid) ++count;
        }
    }
    return count;
}

void Board::setPeg(Position p, bool occupied) {
    if (!isPlayable(p)) {
        throw std::invalid_argument("Cannot place a peg on an invalid position");
    }
    cells_[p.row][p.col] = occupied ? Cell::Peg : Cell::Empty;
}
