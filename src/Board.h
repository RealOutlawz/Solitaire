#pragma once

#include <vector>

enum class BoardType { English, Hexagon, Diamond };
enum class Cell { Invalid, Empty, Peg };

struct Position {
    int row = 0;
    int col = 0;
};

inline bool operator==(Position a, Position b) {
    return a.row == b.row && a.col == b.col;
}

class Board {
public:
    Board(int size, BoardType type, std::vector<std::vector<Cell>> cells);

    int size() const noexcept { return size_; }
    BoardType type() const noexcept { return type_; }
    bool inBounds(Position position) const noexcept;
    bool isPlayable(Position position) const noexcept;
    bool hasPeg(Position position) const noexcept;
    Cell at(Position position) const;
    int pegCount() const noexcept;
    int playableCount() const noexcept;
    void setPeg(Position position, bool occupied);

private:
    int size_;
    BoardType type_;
    std::vector<std::vector<Cell>> cells_;
};
