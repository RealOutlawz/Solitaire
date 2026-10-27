#pragma once

#include "SolitaireGame.h"

#include <QWidget>
#include <QString>
#include <optional>

class QComboBox;
class QLabel;
class QVBoxLayout;

class SolitaireWindow : public QWidget {
public:
    explicit SolitaireWindow(QWidget* parent = nullptr);

private:
    void onNewGame();
    void onHoleClicked(Position position);
    void renderBoard();
    void updateStatus(const QString& message = {});

    SolitaireGame game_;
    std::optional<Position> selected_;
    QComboBox* boardTypeCombo_ = nullptr;
    QComboBox* boardSizeCombo_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QVBoxLayout* boardAreaLayout_ = nullptr;
    QWidget* boardWidget_ = nullptr;
};
