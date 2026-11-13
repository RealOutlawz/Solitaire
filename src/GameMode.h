#pragma once

#include "SolitaireGame.h"

#include <optional>
#include <random>

// Different move sources share the same SolitaireGame rules and board state.
class GameMode {
public:
    virtual ~GameMode() = default;
    virtual bool takeTurn(SolitaireGame& game, std::mt19937& rng,
                          std::optional<Move> requested = std::nullopt) const = 0;
};

class ManualGameMode final : public GameMode {
public:
    bool takeTurn(SolitaireGame& game, std::mt19937& rng,
                  std::optional<Move> requested = std::nullopt) const override;
};

class AutomatedGameMode final : public GameMode {
public:
    bool takeTurn(SolitaireGame& game, std::mt19937& rng,
                  std::optional<Move> requested = std::nullopt) const override;
};
