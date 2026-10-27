#include "SolitaireGame.h"
#include "BoardFactory.h"
#include <gtest/gtest.h>
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
