#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QWidget window;
  window.setWindowTitle("Sprint 0 GUI Demo");

  auto *layout = new QVBoxLayout(&window);

  auto *title = new QLabel("Solitaire GUI Demo");
  layout->addWidget(title);

  auto *line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  layout->addWidget(line);

  auto *label = new QLabel("Board Type:");
  layout->addWidget(label);

  auto *english = new QRadioButton("English");
  auto *hexagon = new QRadioButton("Hexagon");
  auto *diamond = new QRadioButton("Diamond");

  english->setChecked(true);

  layout->addWidget(english);
  layout->addWidget(hexagon);
  layout->addWidget(diamond);

  auto *recordGame = new QCheckBox("Record Game");
  layout->addWidget(recordGame);

  window.resize(300, 220);
  window.show();

  return app.exec();
}