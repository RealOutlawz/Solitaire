#include "SolitaireWindow.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include <exception>

SolitaireWindow::SolitaireWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("CS 449 - Peg Solitaire");
    resize(720, 650);

    auto* outer = new QVBoxLayout(this);
    outer->setSpacing(14);

    auto* heading = new QLabel("Peg Solitaire");
    heading->setStyleSheet("font-size: 24px; font-weight: 700;");
    outer->addWidget(heading);

    auto* controls = new QHBoxLayout();
    controls->addWidget(new QLabel("Board type:"));
    boardTypeCombo_ = new QComboBox();
    boardTypeCombo_->addItem("English", static_cast<int>(BoardType::English));
    boardTypeCombo_->addItem("Hexagon", static_cast<int>(BoardType::Hexagon));
    boardTypeCombo_->addItem("Diamond", static_cast<int>(BoardType::Diamond));
    controls->addWidget(boardTypeCombo_);

    controls->addWidget(new QLabel("Board size:"));
    boardSizeCombo_ = new QComboBox();
    boardSizeCombo_->addItem("7", 7);
    boardSizeCombo_->addItem("9", 9);
    controls->addWidget(boardSizeCombo_);

    auto* newGameButton = new QPushButton("New Game");
    controls->addWidget(newGameButton);
    controls->addStretch();
    outer->addLayout(controls);

    statusLabel_ = new QLabel();
    statusLabel_->setWordWrap(true);
    outer->addWidget(statusLabel_);

    boardAreaLayout_ = new QVBoxLayout();
    boardAreaLayout_->setAlignment(Qt::AlignCenter);
    outer->addLayout(boardAreaLayout_, 1);

    auto* help = new QLabel("Select a peg, then a highlighted empty hole. "
                            "Orthogonal and diagonal jumps are allowed.");
    help->setWordWrap(true);
    outer->addWidget(help);

    connect(newGameButton, &QPushButton::clicked, this,
            [this]() { onNewGame(); });
    renderBoard();
}

void SolitaireWindow::onNewGame() {
    try {
        const auto type = static_cast<BoardType>(boardTypeCombo_->currentData().toInt());
        const int size = boardSizeCombo_->currentData().toInt();
        game_.newGame(type, size);
        selected_.reset();
        renderBoard();
        updateStatus("New game started.");
    } catch (const std::exception& error) {
        updateStatus(QString("Cannot start game: %1").arg(error.what()));
    }
}

void SolitaireWindow::onHoleClicked(Position p) {
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
        if (game_.makeMove(*selected_, p)) {
            selected_.reset();
            renderBoard();
            updateStatus("Move completed.");
            return;
        }
        // Allow selecting a different movable peg without having to deselect.
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
                // Keep the grid aligned without drawing off-board positions.
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
