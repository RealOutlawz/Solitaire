#pragma once

#include "Board.h"

#include <random>
#include <string>
#include <vector>

// A successful jump has one starting peg and one empty destination.
struct Move {
    Position from;
    Position to;
};

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
    // Enumerate legal jumps without changing the board.
    std::vector<Move> validMoves() const;
    // Choose one legal jump randomly (inject RNG for deterministic tests).
    bool makeAutomatedMove(std::mt19937& rng);
    // Shuffle peg locations without changing the count or board geometry.
    // Only accept an arrangement with a legal move remaining.
    bool randomizeBoard(std::mt19937& rng);
    // Restore a validated snapshot while replaying a recorded session.
    void restoreBoard(Board board);
    bool hasValidMoves() const noexcept;
    bool isGameOver() const noexcept;
    bool isWon() const noexcept;
    std::string getRating() const;

private:
    Board board_;
};
