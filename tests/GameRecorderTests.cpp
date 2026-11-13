#include "BoardFactory.h"
#include "GameRecorder.h"
#include "SolitaireGame.h"
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>

namespace {
std::string temporaryRecord(const char* name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

void expectSameBoard(const Board& a, const Board& b) {
    ASSERT_EQ(a.type(), b.type());
    ASSERT_EQ(a.size(), b.size());
    for (int r = 0; r < a.size(); ++r) {
        for (int c = 0; c < a.size(); ++c) {
            EXPECT_EQ(a.at({r, c}), b.at({r, c})) << "at (" << r << ',' << c << ')';
        }
    }
}
} // namespace

TEST(GameRecorderTests, SavesAndLoadsExactMoveHistory) {
    const std::string path = temporaryRecord("cs449_s3_recording_449_a.txt");
    SolitaireGame game;
    GameRecorder recording;
    recording.start(game.board());
    ASSERT_TRUE(game.makeMove({3, 1}, {3, 3}));
    recording.capture(game.board(), "MANUAL_MOVE");
    recording.saveToFile(path);

    GameRecorder loaded = GameRecorder::loadFromFile(path);
    ASSERT_EQ(loaded.frameCount(), 2u);
    EXPECT_EQ(loaded.frame(0).action, "START");
    EXPECT_EQ(loaded.frame(1).action, "MANUAL_MOVE");
    EXPECT_EQ(loaded.frame(0).board.pegCount(), 32);
    EXPECT_EQ(loaded.frame(1).board.pegCount(), 31);
    expectSameBoard(game.board(), loaded.frame(1).board);
    std::remove(path.c_str());
}

TEST(GameRecorderTests, RandomizedStateRoundTripsWithoutLosingPegs) {
    const std::string path = temporaryRecord("cs449_s3_recording_449_b.txt");
    SolitaireGame game;
    game.newGame(BoardType::Hexagon, 7);
    GameRecorder recording;
    recording.start(game.board());
    std::mt19937 rng(449);
    ASSERT_TRUE(game.randomizeBoard(rng));
    recording.capture(game.board(), "RANDOMIZE");
    recording.saveToFile(path);
    const auto loaded = GameRecorder::loadFromFile(path);
    ASSERT_EQ(loaded.frameCount(), 2u);
    EXPECT_EQ(loaded.frame(1).action, "RANDOMIZE");
    EXPECT_EQ(loaded.frame(0).board.pegCount(), loaded.frame(1).board.pegCount());
    expectSameBoard(game.board(), loaded.frame(1).board);
    std::remove(path.c_str());
}

TEST(GameRecorderTests, StartResetsPreviousCapture) {
    SolitaireGame game;
    GameRecorder recording;
    recording.start(game.board());
    ASSERT_TRUE(game.makeMove({3, 1}, {3, 3}));
    recording.capture(game.board(), "MANUAL_MOVE");
    recording.start(game.board());
    ASSERT_EQ(recording.frameCount(), 1u);
    EXPECT_EQ(recording.frame(0).board.pegCount(), 31);
}

TEST(GameRecorderTests, RejectsMissingFileAndMalformedHeader) {
    EXPECT_THROW(GameRecorder::loadFromFile(temporaryRecord("cs449_s3_no_file_449.txt")),
                 std::runtime_error);
    const std::string path = temporaryRecord("cs449_s3_bad_file_449.txt");
    { std::ofstream out(path); out << "NOT_SOLITAIRE 1\n"; }
    EXPECT_THROW(GameRecorder::loadFromFile(path), std::runtime_error);
    std::remove(path.c_str());
}

TEST(GameRecorderTests, RequiresRecordingStartAndConsistentBoardType) {
    GameRecorder recording;
    const Board english = BoardFactory::create(BoardType::English, 7);
    const Board hexagon = BoardFactory::create(BoardType::Hexagon, 7);
    EXPECT_THROW(recording.capture(english, "MOVE"), std::logic_error);
    recording.start(english);
    EXPECT_THROW(recording.capture(hexagon, "MOVE"), std::invalid_argument);
}
