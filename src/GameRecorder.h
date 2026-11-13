#pragma once

#include "Board.h"

#include <cstddef>
#include <string>
#include <vector>

struct RecordedFrame {
    Board board;
    std::string action;
};

// Owns immutable copies of the initial board and every successful action.
// Persisting snapshots also makes randomized board changes replay exactly.
class GameRecorder {
public:
    void start(const Board& initial);
    void capture(const Board& board, const std::string& action);
    bool empty() const noexcept { return frames_.empty(); }
    std::size_t frameCount() const noexcept { return frames_.size(); }
    const RecordedFrame& frame(std::size_t index) const { return frames_.at(index); }

    void saveToFile(const std::string& filename) const;
    static GameRecorder loadFromFile(const std::string& filename);

private:
    std::vector<RecordedFrame> frames_;
};
