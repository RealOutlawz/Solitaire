#include "SolitaireGame.h"
#include "BoardFactory.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
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

std::vector<Move> SolitaireGame::validMoves() const {
    std::vector<Move> moves;
    for (int row = 0; row < board_.size(); ++row) {
        for (int col = 0; col < board_.size(); ++col) {
            const Position from{row, col};
            if (!board_.hasPeg(from)) continue;
            for (const Position to : validDestinations(from)) {
                moves.push_back({from, to});
            }
        }
    }
    return moves;
}

bool SolitaireGame::makeAutomatedMove(std::mt19937& rng) {
    const auto moves = validMoves();
    if (moves.empty()) return false;

    std::uniform_int_distribution<std::size_t> pick(0, moves.size() - 1);
    const Move chosen = moves[pick(rng)];
    return makeMove(chosen.from, chosen.to);
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

void SolitaireGame::restoreBoard(Board board) {
    board_ = std::move(board);
}

bool SolitaireGame::randomizeBoard(std::mt19937& rng) {
    if (isGameOver()) return false;

    std::vector<Position> positions;
    for (int row = 0; row < board_.size(); ++row) {
        for (int col = 0; col < board_.size(); ++col) {
            if (board_.isPlayable({row, col})) positions.push_back({row, col});
        }
    }
    const Board original = board_;
    const int pegCount = original.pegCount();
    if (pegCount == 0 || pegCount == static_cast<int>(positions.size())) return false;

    // Move *locations*, not the number of pegs; avoid an immediate dead end.
    for (int attempt = 0; attempt < 256; ++attempt) {
        std::shuffle(positions.begin(), positions.end(), rng);
        Board candidate = original;
        for (const Position p : positions) candidate.setPeg(p, false);
        for (int i = 0; i < pegCount; ++i) candidate.setPeg(positions[i], true);
        SolitaireGame trial(candidate);
        if (!trial.hasValidMoves()) continue;
        bool changed = false;
        for (int row = 0; row < candidate.size() && !changed; ++row) {
            for (int col = 0; col < candidate.size(); ++col) {
                const Position p{row, col};
                if (candidate.at(p) != original.at(p)) { changed = true; break; }
            }
        }
        if (changed) { board_ = std::move(candidate); return true; }
    }
    return false;
}
