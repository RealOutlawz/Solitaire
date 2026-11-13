#include "GameMode.h"

bool ManualGameMode::takeTurn(SolitaireGame& game, std::mt19937&,
                              std::optional<Move> requested) const {
    return requested && game.makeMove(requested->from, requested->to);
}

bool AutomatedGameMode::takeTurn(SolitaireGame& game, std::mt19937& rng,
                                 std::optional<Move> requested) const {
    if (requested) return false; // No manual destination in automated mode.
    return game.makeAutomatedMove(rng);
}
