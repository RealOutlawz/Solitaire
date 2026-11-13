#pragma once

#include "GameMode.h"
#include "GameRecorder.h"
#include "SolitaireGame.h"

#include <QWidget>
#include <QString>
#include <cstddef>
#include <optional>
#include <random>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QTimer;
class QVBoxLayout;

class SolitaireWindow : public QWidget {
public:
    explicit SolitaireWindow(QWidget* parent = nullptr);

private:
    const GameMode& activeMode() const;
    void onNewGame();
    void onAutomatedMove();
    void onAutoplay();
    void onAutoplayTick();
    void stopAutoplay();
    void onRandomize();
    void onRecordToggled(bool enabled);
    void onSaveRecording();
    void onReplay();
    void onReplayTick();
    void stopReplay();
    void captureAction(const QString& action);
    void onHoleClicked(Position position);
    void renderBoard();
    void updateStatus(const QString& message = {});

    SolitaireGame game_;
    ManualGameMode manualMode_;
    AutomatedGameMode automatedMode_;
    GameRecorder recorder_;
    GameRecorder replayData_;
    std::mt19937 randomEngine_{std::random_device{}()};
    std::optional<Position> selected_;
    std::size_t nextReplayFrame_ = 0;
    bool replaying_ = false;

    QComboBox* boardTypeCombo_ = nullptr;
    QComboBox* boardSizeCombo_ = nullptr;
    QComboBox* modeCombo_ = nullptr;
    QCheckBox* recordCheck_ = nullptr;
    QPushButton* autoplayButton_ = nullptr;
    QTimer* autoplayTimer_ = nullptr;
    QTimer* replayTimer_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QVBoxLayout* boardAreaLayout_ = nullptr;
    QWidget* boardWidget_ = nullptr;
};
