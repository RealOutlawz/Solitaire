#include "GameMode.h"
#include <gtest/gtest.h>
#include <random>

TEST(GameModeTests, ManualModeRequiresARequestedMove) {
    SolitaireGame game;
    ManualGameMode manual;
    const GameMode& mode = manual; // Exercise virtual dispatch.
    std::mt19937 rng(449);
    EXPECT_FALSE(mode.takeTurn(game, rng));
    EXPECT_EQ(game.board().pegCount(), 32);
    EXPECT_TRUE(mode.takeTurn(game, rng, Move{{3, 1}, {3, 3}}));
    EXPECT_EQ(game.board().pegCount(), 31);
}

TEST(GameModeTests, ManualModeRejectsIllegalMoveWithoutMutatingBoard) {
    SolitaireGame game;
    ManualGameMode manual;
    std::mt19937 rng(449);
    EXPECT_FALSE(manual.takeTurn(game, rng, Move{{3, 0}, {3, 2}}));
    EXPECT_EQ(game.board().pegCount(), 32);
}

TEST(GameModeTests, AutomatedModeUsesRandomLegalMoveInsteadOfUserInput) {
    SolitaireGame game;
    AutomatedGameMode automated;
    std::mt19937 rng(449);
    EXPECT_FALSE(automated.takeTurn(game, rng, Move{{3, 1}, {3, 3}}));
    EXPECT_TRUE(automated.takeTurn(game, rng));
    EXPECT_EQ(game.board().pegCount(), 31);
}

TEST(GameModeTests, AutoplayModeStopsAtGameOver) {
    SolitaireGame game;
    AutomatedGameMode automated;
    std::mt19937 rng(44);
    int steps = 0;
    while (!game.isGameOver() && steps < 40) {
        ASSERT_TRUE(automated.takeTurn(game, rng));
        ++steps;
    }
    EXPECT_GT(steps, 0);
    EXPECT_TRUE(game.isGameOver());
    EXPECT_FALSE(automated.takeTurn(game, rng));
}
