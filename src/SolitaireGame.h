#pragma once

#include "Board.h"

#include <string>
#include <vector>

class SolitaireGame {
public:
    SolitaireGame();
    // Also useful for restoring/replaying a game or constructing a test board.
    explicit SolitaireGame(Board board);

    void newGame(BoardType type, int size);
    const Board& board() const noexcept { return board_; }
    bool isValidMove(Position from, Position to) const noexcept;
    bool makeMove(Position from, Position to);
    std::vector<Position> validDestinations(Position from) const;
    bool hasValidMoves() const noexcept;
    bool isGameOver() const noexcept;
    bool isWon() const noexcept;
    std::string getRating() const;

private:
    Board board_;
};
