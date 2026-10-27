#include "BoardFactory.h"
#include <gtest/gtest.h>
#include <stdexcept>

TEST(BoardFactoryTests, DefaultEnglishBoardHas32PegsAndEmptyCenter) {
    const Board board = BoardFactory::create(BoardType::English, 7);
    EXPECT_EQ(board.size(), 7);
    EXPECT_EQ(board.playableCount(), 33);
    EXPECT_EQ(board.pegCount(), 32);
    EXPECT_EQ(board.at({3, 3}), Cell::Empty);
    EXPECT_EQ(board.at({0, 0}), Cell::Invalid);
}

TEST(BoardFactoryTests, EnglishSize9HasDistinctPlayableLayout) {
    const Board board = BoardFactory::create(BoardType::English, 9);
    EXPECT_EQ(board.playableCount(), 45);
    EXPECT_EQ(board.pegCount(), 44);
    EXPECT_EQ(board.at({4, 4}), Cell::Empty);
}

TEST(BoardFactoryTests, AllBoardTypesInitializeWithEmptyCenter) {
    for (BoardType type : {BoardType::English, BoardType::Hexagon, BoardType::Diamond}) {
        for (int size : {7, 9}) {
            const Board board = BoardFactory::create(type, size);
            EXPECT_EQ(board.at({size / 2, size / 2}), Cell::Empty);
            EXPECT_EQ(board.pegCount(), board.playableCount() - 1);
        }
    }
}

TEST(BoardFactoryTests, DifferentBoardTypesProduceDifferentShapes) {
    EXPECT_EQ(BoardFactory::create(BoardType::English, 7).playableCount(), 33);
    EXPECT_EQ(BoardFactory::create(BoardType::Hexagon, 7).playableCount(), 37);
    EXPECT_EQ(BoardFactory::create(BoardType::Diamond, 7).playableCount(), 25);
}

TEST(BoardFactoryTests, UnsupportedSizeIsRejected) {
    EXPECT_THROW(BoardFactory::create(BoardType::English, 8), std::invalid_argument);
    EXPECT_THROW(BoardFactory::create(BoardType::English, 5), std::invalid_argument);
}

TEST(BoardTests, InvalidPositionsCannotHoldPegs) {
    Board board = BoardFactory::create(BoardType::English, 7);
    EXPECT_THROW(board.setPeg({0, 0}, true), std::invalid_argument);
    EXPECT_THROW(board.at({-1, 3}), std::out_of_range);
}
