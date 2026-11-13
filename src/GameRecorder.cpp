#include "GameRecorder.h"
#include "BoardFactory.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr const char* kHeader = "CS449_SOLITAIRE_RECORDING_V1";

char symbol(Cell cell) {
    switch (cell) {
        case Cell::Invalid: return 'X';
        case Cell::Empty: return '.';
        case Cell::Peg: return 'O';
    }
    throw std::runtime_error("Unrecognized cell type");
}

Cell decode(char ch) {
    switch (ch) {
        case 'X': return Cell::Invalid;
        case '.': return Cell::Empty;
        case 'O': return Cell::Peg;
        default: throw std::runtime_error("Invalid cell character in recording");
    }
}
} // namespace

void GameRecorder::start(const Board& initial) {
    frames_.clear();
    frames_.push_back({initial, "START"});
}

void GameRecorder::capture(const Board& board, const std::string& action) {
    if (frames_.empty()) throw std::logic_error("Start recording first");
    if (action.empty()) throw std::invalid_argument("Action must be named");
    if (board.size() != frames_.front().board.size() ||
        board.type() != frames_.front().board.type()) {
        throw std::invalid_argument("Cannot change board configuration mid-recording");
    }
    frames_.push_back({board, action});
}

void GameRecorder::saveToFile(const std::string& filename) const {
    if (frames_.empty()) throw std::logic_error("No game recorded");
    std::ofstream out(filename, std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot create recording file");
    out << kHeader << '\n' << frames_.size() << '\n';
    for (const auto& item : frames_) {
        const Board& board = item.board;
        out << "FRAME " << static_cast<int>(board.type()) << ' ' << board.size()
            << ' ' << std::quoted(item.action) << '\n';
        for (int r = 0; r < board.size(); ++r) {
            for (int c = 0; c < board.size(); ++c)
                out << symbol(board.at({r, c}));
            out << '\n';
        }
    }
    out.flush();
    if (!out) throw std::runtime_error("Could not finish writing recording");
}

GameRecorder GameRecorder::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Cannot open recording file");
    std::string header;
    if (!(in >> header) || header != kHeader)
        throw std::runtime_error("Not a supported Solitaire recording");

    std::size_t count = 0;
    if (!(in >> count) || count == 0 || count > 10000)
        throw std::runtime_error("Invalid recording frame count");
    GameRecorder recording;
    BoardType baseType = BoardType::English;
    int baseSize = 0;
    for (std::size_t index = 0; index < count; ++index) {
        std::string token, action;
        int typeValue = -1, size = 0;
        if (!(in >> token >> typeValue >> size >> std::quoted(action)) ||
            token != "FRAME" || action.empty() ||
            typeValue < 0 || typeValue > 2 || !BoardFactory::supportsSize(size)) {
            throw std::runtime_error("Invalid recording frame header");
        }
        const auto type = static_cast<BoardType>(typeValue);
        if (index == 0) { baseType = type; baseSize = size; }
        if (type != baseType || size != baseSize)
            throw std::runtime_error("Recording changes board configuration");

        const Board layout = BoardFactory::create(type, size);
        std::vector<std::vector<Cell>> cells(size, std::vector<Cell>(size));
        for (int r = 0; r < size; ++r) {
            std::string line;
            if (!(in >> line) || static_cast<int>(line.size()) != size)
                throw std::runtime_error("Recording has an incomplete board row");
            for (int c = 0; c < size; ++c) {
                cells[r][c] = decode(line[c]);
                if ((cells[r][c] == Cell::Invalid) !=
                    (layout.at({r, c}) == Cell::Invalid)) {
                    throw std::runtime_error("Recording board shape is invalid");
                }
            }
        }
        Board snapshot(size, type, std::move(cells));
        if (index == 0) recording.start(snapshot);
        else recording.capture(snapshot, action);
    }
    std::string trailing;
    if (in >> trailing) throw std::runtime_error("Unexpected data after recording");
    return recording;
}
