#include "SolitaireGame.h"
#include "BoardFactory.h"

#include <array>
#include <cmath>
#include <utility>

namespace {
constexpr std::array<Position, 8> kJumpDirections{{
    {-2, 0}, {2, 0}, {0, -2}, {0, 2},
    {-2, -2}, {-2, 2}, {2, -2}, {2, 2}
}};
} // namespace

SolitaireGame::SolitaireGame()
    : board_(BoardFactory::create(BoardType::English, 7)) {}

SolitaireGame::SolitaireGame(Board board) : board_(std::move(board)) {}

void SolitaireGame::newGame(BoardType type, int size) {
    // Create the new board before changing existing state; invalid options
    // therefore leave the current game untouched.
    Board replacement = BoardFactory::create(type, size);
    board_ = std::move(replacement);
}

bool SolitaireGame::isValidMove(Position from, Position to) const noexcept {
    if (!board_.hasPeg(from) || !board_.isPlayable(to) || board_.hasPeg(to)) {
        return false;
    }

    const int dr = to.row - from.row;
    const int dc = to.col - from.col;
    // Exactly two steps horizontally, vertically, or diagonally.
    const bool horizontal = dr == 0 && std::abs(dc) == 2;
    const bool vertical = dc == 0 && std::abs(dr) == 2;
    const bool diagonal = std::abs(dr) == 2 && std::abs(dc) == 2;
    if (!horizontal && !vertical && !diagonal) return false;

    const Position jumped{from.row + dr / 2, from.col + dc / 2};
    return board_.hasPeg(jumped);
}

bool SolitaireGame::makeMove(Position from, Position to) {
    if (isGameOver() || !isValidMove(from, to)) return false;

    const Position jumped{(from.row + to.row) / 2,
                          (from.col + to.col) / 2};
    board_.setPeg(from, false);
    board_.setPeg(jumped, false);
    board_.setPeg(to, true);
    return true;
}

std::vector<Position> SolitaireGame::validDestinations(Position from) const {
    std::vector<Position> destinations;
    for (const Position delta : kJumpDirections) {
        const Position to{from.row + delta.row, from.col + delta.col};
        if (isValidMove(from, to)) destinations.push_back(to);
    }
    return destinations;
}

bool SolitaireGame::hasValidMoves() const noexcept {
    for (int row = 0; row < board_.size(); ++row) {
        for (int col = 0; col < board_.size(); ++col) {
            const Position from{row, col};
            if (!board_.hasPeg(from)) continue;
            for (const Position delta : kJumpDirections) {
                if (isValidMove(from, {row + delta.row, col + delta.col})) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool SolitaireGame::isWon() const noexcept {
    return board_.pegCount() == 1;
}

bool SolitaireGame::isGameOver() const noexcept {
    return isWon() || !hasValidMoves();
}

std::string SolitaireGame::getRating() const {
    if (!isGameOver()) return "In progress";
    switch (board_.pegCount()) {
        case 1: return "Outstanding";
        case 2: return "Very Good";
        case 3: return "Good";
        default: return "Average";
    }
}
