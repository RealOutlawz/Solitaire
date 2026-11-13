#include "SolitaireGame.h"
#include "BoardFactory.h"
#include <gtest/gtest.h>
#include <random>
#include <stdexcept>
#include <utility>

namespace {
Board emptyEnglishBoard() {
    Board board = BoardFactory::create(BoardType::English, 7);
    for (int r = 0; r < board.size(); ++r) {
        for (int c = 0; c < board.size(); ++c) {
            if (board.isPlayable({r, c})) board.setPeg({r, c}, false);
        }
    }
    return board;
}
} // namespace

// LLM-assisted test candidate #1. Review against AC 4.1 and capture prompt/answer.
TEST(SolitaireGameTests, ValidMoveRemovesJumpedPeg) {
    SolitaireGame game;
    const int before = game.board().pegCount();
    ASSERT_TRUE(game.makeMove({3, 1}, {3, 3}));
    EXPECT_FALSE(game.board().hasPeg({3, 1}));
    EXPECT_FALSE(game.board().hasPeg({3, 2}));
    EXPECT_TRUE(game.board().hasPeg({3, 3}));
    EXPECT_EQ(game.board().pegCount(), before - 1);
}

// LLM-assisted test candidate #2. Review against AC 4.3 and capture prompt/answer.
TEST(SolitaireGameTests, DiagonalMoveIsAllowed) {
    SolitaireGame game;
    game.newGame(BoardType::Hexagon, 7);
    const int before = game.board().pegCount();
    ASSERT_TRUE(game.makeMove({1, 5}, {3, 3}));
    EXPECT_FALSE(game.board().hasPeg({1, 5}));
    EXPECT_FALSE(game.board().hasPeg({2, 4}));
    EXPECT_TRUE(game.board().hasPeg({3, 3}));
    EXPECT_EQ(game.board().pegCount(), before - 1);
}

TEST(SolitaireGameTests, NewGameResetsMovesAndChangesSize) {
    SolitaireGame game;
    ASSERT_TRUE(game.makeMove({3, 1}, {3, 3}));
    game.newGame(BoardType::Diamond, 9);
    EXPECT_EQ(game.board().size(), 9);
    EXPECT_EQ(game.board().type(), BoardType::Diamond);
    EXPECT_EQ(game.board().pegCount(), 40);
    EXPECT_FALSE(game.board().hasPeg({4, 4}));
}

TEST(SolitaireGameTests, UnsupportedSizeKeepsExistingBoard) {
    SolitaireGame game;
    const int before = game.board().pegCount();
    EXPECT_THROW(game.newGame(BoardType::Hexagon, 8), std::invalid_argument);
    EXPECT_EQ(game.board().size(), 7);
    EXPECT_EQ(game.board().type(), BoardType::English);
    EXPECT_EQ(game.board().pegCount(), before);
}

TEST(SolitaireGameTests, OccupiedDestinationIsRejected) {
    SolitaireGame game;
    const int before = game.board().pegCount();
    EXPECT_FALSE(game.makeMove({3, 0}, {3, 2}));
    EXPECT_EQ(game.board().pegCount(), before);
    EXPECT_TRUE(game.board().hasPeg({3, 1}));
}

TEST(SolitaireGameTests, NoPegToJumpIsRejected) {
    Board board = BoardFactory::create(BoardType::English, 7);
    board.setPeg({3, 2}, false);
    SolitaireGame game(std::move(board));
    EXPECT_FALSE(game.makeMove({3, 1}, {3, 3}));
    EXPECT_TRUE(game.board().hasPeg({3, 1}));
    EXPECT_FALSE(game.board().hasPeg({3, 3}));
}

TEST(SolitaireGameTests, InvalidOrOffBoardMovesAreRejected) {
    SolitaireGame game;
    EXPECT_FALSE(game.makeMove({0, 0}, {0, 2}));
    EXPECT_FALSE(game.makeMove({3, 1}, {3, 6}));
    EXPECT_FALSE(game.makeMove({3, 3}, {3, 5}));
}

TEST(SolitaireGameTests, ValidDestinationsCanGuideUiHighlights) {
    SolitaireGame game;
    const auto targets = game.validDestinations({3, 1});
    ASSERT_EQ(targets.size(), 1u);
    EXPECT_EQ(targets[0].row, 3);
    EXPECT_EQ(targets[0].col, 3);
}

TEST(SolitaireGameTests, OnePegIsAWinAndEndsGame) {
    Board board = emptyEnglishBoard();
    board.setPeg({3, 3}, true);
    SolitaireGame game(std::move(board));
    EXPECT_TRUE(game.isWon());
    EXPECT_TRUE(game.isGameOver());
    EXPECT_FALSE(game.hasValidMoves());
    EXPECT_EQ(game.getRating(), "Outstanding");
    EXPECT_FALSE(game.makeMove({3, 3}, {3, 5}));
}

TEST(SolitaireGameTests, NoPossibleMovesWithTwoPegsEndsGame) {
    Board board = emptyEnglishBoard();
    board.setPeg({0, 3}, true);
    board.setPeg({6, 3}, true);
    SolitaireGame game(std::move(board));
    EXPECT_FALSE(game.isWon());
    EXPECT_FALSE(game.hasValidMoves());
    EXPECT_TRUE(game.isGameOver());
    EXPECT_EQ(game.getRating(), "Very Good");
    EXPECT_FALSE(game.makeMove({0, 3}, {2, 3}));
}

TEST(SolitaireGameTests, ThreeAndFourPegsGetCorrectRatings) {
    Board board = emptyEnglishBoard();
    board.setPeg({0, 2}, true);
    board.setPeg({0, 4}, true);
    board.setPeg({6, 4}, true);
    SolitaireGame three(board);
    EXPECT_TRUE(three.isGameOver());
    EXPECT_EQ(three.getRating(), "Good");

    board.setPeg({6, 2}, true);
    SolitaireGame four(std::move(board));
    EXPECT_TRUE(four.isGameOver());
    EXPECT_EQ(four.getRating(), "Average");
}

TEST(SolitaireGameTests, GameStartsInProgress) {
    SolitaireGame game;
    EXPECT_FALSE(game.isGameOver());
    EXPECT_EQ(game.getRating(), "In progress");
}

// Sprint 3, milestone 1: automated move selection (not yet full autoplay).
TEST(SolitaireGameTests, EnumeratesFourLegalOpeningMovesOnEnglishBoard) {
    SolitaireGame game;
    const auto moves = game.validMoves();
    ASSERT_EQ(moves.size(), 4u);
    for (const Move& move : moves) {
        EXPECT_TRUE(game.isValidMove(move.from, move.to));
        EXPECT_EQ(move.to, (Position{3, 3}));
    }
}

TEST(SolitaireGameTests, AutomatedMoveChoosesLegalJumpAndRemovesOnePeg) {
    SolitaireGame game;
    const Board before = game.board();
    std::mt19937 rng(449);

    ASSERT_TRUE(game.makeAutomatedMove(rng));
    EXPECT_EQ(game.board().pegCount(), before.pegCount() - 1);

    // The resulting board must match one legal move from the original state.
    SolitaireGame baseline(before);
    bool matchesLegalMove = false;
    for (const Move& move : baseline.validMoves()) {
        SolitaireGame candidate(before);
        ASSERT_TRUE(candidate.makeMove(move.from, move.to));
        bool sameBoard = true;
        for (int row = 0; row < before.size(); ++row) {
            for (int col = 0; col < before.size(); ++col) {
                if (candidate.board().at({row, col}) != game.board().at({row, col})) {
                    sameBoard = false;
                }
            }
        }
        if (sameBoard) matchesLegalMove = true;
    }
    EXPECT_TRUE(matchesLegalMove);
}

TEST(SolitaireGameTests, AutomatedMovesEventuallyReachGameOver) {
    SolitaireGame game;
    std::mt19937 rng(449);
    const int initialPegs = game.board().pegCount();
    int movesMade = 0;
    while (!game.isGameOver() && movesMade < initialPegs) {
        ASSERT_TRUE(game.makeAutomatedMove(rng));
        ++movesMade;
    }
    EXPECT_GT(movesMade, 0);
    EXPECT_TRUE(game.isGameOver());
    EXPECT_LE(movesMade, initialPegs - 1);
    EXPECT_FALSE(game.makeAutomatedMove(rng));
}

TEST(SolitaireGameTests, RandomizeKeepsBoardShapeAndPegCount) {
    SolitaireGame game;
    std::mt19937 rng(449);
    const Board original = game.board();
    ASSERT_TRUE(game.randomizeBoard(rng));
    EXPECT_EQ(game.board().type(), original.type());
    EXPECT_EQ(game.board().size(), original.size());
    EXPECT_EQ(game.board().pegCount(), original.pegCount());
    EXPECT_TRUE(game.hasValidMoves());
    bool changed = false;
    for (int row = 0; row < original.size(); ++row) {
        for (int col = 0; col < original.size(); ++col) {
            const Position p{row, col};
            EXPECT_EQ(game.board().isPlayable(p), original.isPlayable(p));
            if (game.board().at(p) != original.at(p)) changed = true;
        }
    }
    EXPECT_TRUE(changed);
}

TEST(SolitaireGameTests, RandomizationRejectsGameOverAndLeavesBoardUnchanged) {
    Board board = emptyEnglishBoard();
    board.setPeg({3, 3}, true);
    SolitaireGame game(board);
    std::mt19937 rng(449);
    EXPECT_FALSE(game.randomizeBoard(rng));
    EXPECT_EQ(game.board().pegCount(), 1);
    EXPECT_TRUE(game.board().hasPeg({3, 3}));
}
