#include "SolitaireWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <exception>
#include <utility>

SolitaireWindow::SolitaireWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("CS 449 - Peg Solitaire");
    resize(850, 710);

    auto* outer = new QVBoxLayout(this);
    outer->setSpacing(12);
    auto* heading = new QLabel("Peg Solitaire");
    heading->setStyleSheet("font-size: 24px; font-weight: 700;");
    outer->addWidget(heading);

    auto* setup = new QHBoxLayout();
    setup->addWidget(new QLabel("Board type:"));
    boardTypeCombo_ = new QComboBox();
    boardTypeCombo_->addItem("English", static_cast<int>(BoardType::English));
    boardTypeCombo_->addItem("Hexagon", static_cast<int>(BoardType::Hexagon));
    boardTypeCombo_->addItem("Diamond", static_cast<int>(BoardType::Diamond));
    setup->addWidget(boardTypeCombo_);
    setup->addWidget(new QLabel("Board size:"));
    boardSizeCombo_ = new QComboBox();
    boardSizeCombo_->addItem("7", 7);
    boardSizeCombo_->addItem("9", 9);
    setup->addWidget(boardSizeCombo_);
    setup->addWidget(new QLabel("Mode:"));
    modeCombo_ = new QComboBox();
    modeCombo_->addItem("Manual");
    modeCombo_->addItem("Automated");
    setup->addWidget(modeCombo_);
    auto* newGameButton = new QPushButton("New Game");
    setup->addWidget(newGameButton);
    setup->addStretch();
    outer->addLayout(setup);

    auto* actions = new QHBoxLayout();
    auto* autoMoveButton = new QPushButton("Auto Move");
    actions->addWidget(autoMoveButton);
    autoplayButton_ = new QPushButton("Autoplay");
    actions->addWidget(autoplayButton_);
    auto* randomizeButton = new QPushButton("Randomize");
    actions->addWidget(randomizeButton);
    recordCheck_ = new QCheckBox("Record game");
    actions->addWidget(recordCheck_);
    auto* saveButton = new QPushButton("Save Recording");
    actions->addWidget(saveButton);
    auto* replayButton = new QPushButton("Replay File");
    actions->addWidget(replayButton);
    actions->addStretch();
    outer->addLayout(actions);

    statusLabel_ = new QLabel();
    statusLabel_->setWordWrap(true);
    outer->addWidget(statusLabel_);
    boardAreaLayout_ = new QVBoxLayout();
    boardAreaLayout_->setAlignment(Qt::AlignCenter);
    outer->addLayout(boardAreaLayout_, 1);

    auto* help = new QLabel("Manual mode: select a peg, then a highlighted empty hole. "
                            "Autoplay makes randomized legal jumps; Randomize shuffles peg locations.");
    help->setWordWrap(true);
    outer->addWidget(help);

    autoplayTimer_ = new QTimer(this);
    autoplayTimer_->setInterval(260);
    replayTimer_ = new QTimer(this);
    replayTimer_->setInterval(420);

    connect(newGameButton, &QPushButton::clicked, this,
            [this]() { onNewGame(); });
    connect(autoMoveButton, &QPushButton::clicked, this,
            [this]() { onAutomatedMove(); });
    connect(autoplayButton_, &QPushButton::clicked, this,
            [this]() { onAutoplay(); });
    connect(autoplayTimer_, &QTimer::timeout, this,
            [this]() { onAutoplayTick(); });
    connect(randomizeButton, &QPushButton::clicked, this,
            [this]() { onRandomize(); });
    connect(recordCheck_, &QCheckBox::toggled, this,
            [this](bool enabled) { onRecordToggled(enabled); });
    connect(saveButton, &QPushButton::clicked, this,
            [this]() { onSaveRecording(); });
    connect(replayButton, &QPushButton::clicked, this,
            [this]() { onReplay(); });
    connect(replayTimer_, &QTimer::timeout, this,
            [this]() { onReplayTick(); });
    connect(modeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) {
                stopAutoplay();
                stopReplay();
                selected_.reset();
                renderBoard();
                updateStatus("Game mode changed.");
            });
    renderBoard();
}

const GameMode& SolitaireWindow::activeMode() const {
    return modeCombo_->currentIndex() == 0
        ? static_cast<const GameMode&>(manualMode_)
        : static_cast<const GameMode&>(automatedMode_);
}

void SolitaireWindow::stopAutoplay() {
    autoplayTimer_->stop();
    autoplayButton_->setText("Autoplay");
}

void SolitaireWindow::stopReplay() {
    replayTimer_->stop();
    replaying_ = false;
}

void SolitaireWindow::captureAction(const QString& action) {
    if (recordCheck_->isChecked()) {
        recorder_.capture(game_.board(), action.toStdString());
    }
}

void SolitaireWindow::onNewGame() {
    stopAutoplay();
    stopReplay();
    try {
        const auto type = static_cast<BoardType>(boardTypeCombo_->currentData().toInt());
        const int size = boardSizeCombo_->currentData().toInt();
        game_.newGame(type, size);
        selected_.reset();
        if (recordCheck_->isChecked()) recorder_.start(game_.board());
        renderBoard();
        updateStatus("New game started.");
    } catch (const std::exception& error) {
        updateStatus(QString("Cannot start game: %1").arg(error.what()));
    }
}

void SolitaireWindow::onAutomatedMove() {
    stopAutoplay();
    stopReplay();
    modeCombo_->setCurrentIndex(1);
    selected_.reset();
    const bool moved = activeMode().takeTurn(game_, randomEngine_);
    if (moved) captureAction("AUTO_MOVE");
    renderBoard();
    updateStatus(moved ? "Automated move completed."
                       : "No automated move available (game over).");
}

void SolitaireWindow::onAutoplay() {
    if (autoplayTimer_->isActive()) {
        stopAutoplay();
        updateStatus("Autoplay paused.");
        return;
    }
    stopReplay();
    modeCombo_->setCurrentIndex(1);
    selected_.reset();
    if (game_.isGameOver()) {
        updateStatus("The game is already over. Start a new game.");
        return;
    }
    autoplayButton_->setText("Stop");
    autoplayTimer_->start();
    updateStatus("Autoplay running.");
}

void SolitaireWindow::onAutoplayTick() {
    const bool moved = activeMode().takeTurn(game_, randomEngine_);
    if (moved) captureAction("AUTO_MOVE");
    renderBoard();
    if (!moved || game_.isGameOver()) {
        stopAutoplay();
        updateStatus("Autoplay finished (game over).");
    } else {
        updateStatus("Autoplay running.");
    }
}

void SolitaireWindow::onRandomize() {
    stopAutoplay();
    stopReplay();
    modeCombo_->setCurrentIndex(0); // Randomize is a manual-game action.
    selected_.reset();
    const bool changed = game_.randomizeBoard(randomEngine_);
    if (changed) captureAction("RANDOMIZE");
    renderBoard();
    updateStatus(changed ? "Board randomized; peg count preserved."
                         : "Cannot randomize this game state.");
}

void SolitaireWindow::onRecordToggled(bool enabled) {
    if (enabled) {
        stopReplay();
        recorder_.start(game_.board());
        updateStatus("Recording started. Save Recording writes the file.");
    } else {
        updateStatus("Recording stopped. Use Save Recording to save it.");
    }
}

void SolitaireWindow::onSaveRecording() {
    if (recorder_.empty()) {
        updateStatus("Start recording before saving a file.");
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, "Save Solitaire Game", "solitaire-recording.txt", "Text files (*.txt)");
    if (path.isEmpty()) return;
    try {
        recorder_.saveToFile(path.toStdString());
        updateStatus(QString("Recording saved: %1 frames.").arg(static_cast<int>(recorder_.frameCount())));
    } catch (const std::exception& error) {
        updateStatus(QString("Recording error: %1").arg(error.what()));
    }
}

void SolitaireWindow::onReplay() {
    stopAutoplay();
    stopReplay();
    const QString path = QFileDialog::getOpenFileName(
        this, "Replay Solitaire Game", QString(), "Text files (*.txt)");
    if (path.isEmpty()) return;
    try {
        GameRecorder loaded = GameRecorder::loadFromFile(path.toStdString());
        if (recordCheck_->isChecked()) recordCheck_->setChecked(false);
        replayData_ = std::move(loaded);
        selected_.reset();
        game_.restoreBoard(replayData_.frame(0).board);
        boardTypeCombo_->setCurrentIndex(static_cast<int>(game_.board().type()));
        boardSizeCombo_->setCurrentText(QString::number(game_.board().size()));
        nextReplayFrame_ = 1;
        replaying_ = true;
        renderBoard();
        if (nextReplayFrame_ < replayData_.frameCount()) {
            replayTimer_->start();
            updateStatus("Replaying saved game...");
        } else {
            stopReplay();
            updateStatus("Replay complete (initial frame only).");
        }
    } catch (const std::exception& error) {
        stopReplay();
        updateStatus(QString("Replay error: %1").arg(error.what()));
    }
}

void SolitaireWindow::onReplayTick() {
    if (nextReplayFrame_ >= replayData_.frameCount()) {
        stopReplay();
        updateStatus("Replay complete.");
        return;
    }
    const auto& frame = replayData_.frame(nextReplayFrame_++);
    game_.restoreBoard(frame.board);
    renderBoard();
    if (nextReplayFrame_ >= replayData_.frameCount()) {
        stopReplay();
        updateStatus("Replay complete.");
    } else {
        updateStatus(QString("Replaying: %1").arg(QString::fromStdString(frame.action)));
    }
}

void SolitaireWindow::onHoleClicked(Position p) {
    if (replaying_) {
        updateStatus("Replay in progress. Start New Game to play manually.");
        return;
    }
    if (autoplayTimer_->isActive() || modeCombo_->currentIndex() != 0) {
        updateStatus("Select Manual mode to move a peg yourself.");
        return;
    }
    if (game_.isGameOver()) {
        updateStatus("The game is over. Select New Game to play again.");
        return;
    }
    const Board& board = game_.board();
    if (selected_.has_value() && *selected_ == p) {
        selected_.reset();
        renderBoard();
        return;
    }
    if (selected_.has_value()) {
        if (activeMode().takeTurn(game_, randomEngine_, Move{*selected_, p})) {
            captureAction("MANUAL_MOVE");
            selected_.reset();
            renderBoard();
            updateStatus("Move completed.");
            return;
        }
        if (board.hasPeg(p) && !game_.validDestinations(p).empty()) {
            selected_ = p;
            renderBoard();
            updateStatus("Different peg selected.");
            return;
        }
        renderBoard();
        updateStatus("Invalid move. Jump over one peg into an empty hole.");
        return;
    }
    if (board.hasPeg(p) && !game_.validDestinations(p).empty()) {
        selected_ = p;
        renderBoard();
        updateStatus("Peg selected. Choose a highlighted destination.");
    } else {
        updateStatus("Select a peg that has an available move.");
    }
}

void SolitaireWindow::renderBoard() {
    if (boardWidget_) {
        boardAreaLayout_->removeWidget(boardWidget_);
        boardWidget_->deleteLater();
    }
    boardWidget_ = new QWidget(this);
    auto* grid = new QGridLayout(boardWidget_);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(5);
    grid->setVerticalSpacing(5);

    const Board& board = game_.board();
    const auto destinations = selected_.has_value()
        ? game_.validDestinations(*selected_) : std::vector<Position>{};
    for (int row = 0; row < board.size(); ++row) {
        for (int col = 0; col < board.size(); ++col) {
            const Position p{row, col};
            if (!board.isPlayable(p)) {
                grid->addWidget(new QWidget(boardWidget_), row, col);
                continue;
            }
            auto* hole = new QPushButton(board.hasPeg(p) ? "●" : "○", boardWidget_);
            hole->setFixedSize(42, 42);
            hole->setCursor(Qt::PointingHandCursor);
            const bool selected = selected_.has_value() && *selected_ == p;
            bool destination = false;
            for (Position d : destinations) {
                if (d == p) { destination = true; break; }
            }
            QString background = "#e7eef7";
            if (selected) background = "#ffe08a";
            else if (destination) background = "#a9e5b5";
            hole->setStyleSheet(
                "QPushButton { border: 1px solid #64748b; border-radius: 8px; "
                "font-size: 21px; font-weight: bold; color: #233044; "
                "background-color: " + background + "; }");
            hole->setAccessibleName(QString("Row %1 column %2: %3")
                .arg(row).arg(col).arg(board.hasPeg(p) ? "peg" : "empty"));
            connect(hole, &QPushButton::clicked, this,
                    [this, p]() { onHoleClicked(p); });
            grid->addWidget(hole, row, col);
        }
    }
    boardAreaLayout_->addWidget(boardWidget_, 0, Qt::AlignCenter);
    updateStatus();
}

void SolitaireWindow::updateStatus(const QString& message) {
    QString summary = QString("Pegs left: %1").arg(game_.board().pegCount());
    if (game_.isGameOver()) {
        summary += QString(" | Game over - %1")
            .arg(QString::fromStdString(game_.getRating()));
    }
    if (!message.isEmpty()) summary += " | " + message;
    statusLabel_->setText(summary);
}
