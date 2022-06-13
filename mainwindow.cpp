#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QString>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QRandomGenerator>
#include <QTime>
#include <QCoreApplication>
#include <QThread>
#include <iostream>
#include "Windows.h"
#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>
#include <QGuiApplication>
#include <QDirIterator>
#include <iostream>
#include <string>
#include <vector>
#include <QRadioButton>
#include <QApplication>
#include <QByteArray>
#include <QTabWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QtWidgets>
#include <QButtonGroup>
#include <QTime>

using namespace std;


// 0: stopped
// 1: started
// 2: pause

// keybinds
HHOOK sHookKeyboard;
MainWindow *windowPointer;
void MainWindow::SetUpHook() {
    sHookKeyboard = ::SetWindowsHookExA(WH_KEYBOARD_LL,
                     [] (int i, WPARAM w, LPARAM l)
                     {
        KBDLLHOOKSTRUCT kbdStruct;
        kbdStruct = *((KBDLLHOOKSTRUCT*)l);
        int vk = kbdStruct.vkCode;
        qInfo() << windowPointer->status;
        // ignore up signals
        if (kbdStruct.flags != 128) {
        if ((vk == windowPointer->startKeybind) && (windowPointer->status == 0 || windowPointer->status == 2)) {
           windowPointer->on_startButton_clicked();
        } else if ((vk == windowPointer->pauseKeybind) && (windowPointer->status == 1)) {
           windowPointer->SetPaused();
        }
        }
        return ::CallNextHookEx(sHookKeyboard, i, w, l);
    },
                                        0, 0);
    this->Hook = sHookKeyboard;
}

// QThread::(m/u)sleep stops UI responding, semi hacky solution i found on stackoverflow to ensure that it still responds
inline void delay(int millisecondsWait)
{
    QEventLoop loop;
    QTimer t;
    t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(millisecondsWait);
    loop.exec();
}

std::string MainWindow::NumberToWord(int number, bool referred) {
    vector < std::string  > ones {
      "",
      "one",
      "two",
      "three",
      "four",
      "five",
      "six",
      "seven",
      "eight",
      "nine"
    };
    vector < std::string  > teens {
      "ten",
      "eleven",
      "twelve",
      "thirteen",
      "fourteen",
      "fifteen",
      "sixteen",
      "seventeen",
      "eighteen",
      "nineteen"
    };
    vector < std::string  > tens {
      "",
      "",
      "twenty",
      "thirty",
      "forty",
      "fifty",
      "sixty",
      "seventy",
      "eighty",
      "ninety"
    };
  if (!referred) {
      number++;
  }
  if (number < 10) {
    return ones[number];
  } else if (number < 20) {
    return teens[number - 10];
  } else if (number < 100) {
    return tens[number / 10] + ((number % 10 != 0) ? " " + this->NumberToWord(number % 10, true) : "");
  } else if (number < 1000) {
    return this->NumberToWord(number / 100, true) + " hundred" + ((number % 100 != 0) ? " " + this->NumberToWord(number % 100, true) : "");
  }
  return "error";
}

void MainWindow::GenerateTitles() {
  QFile file(":/titles.txt");
  file.open(QIODevice::ReadOnly);
  QStringList titles = QString(file.readAll()).split('\n');

  file.close();
  // sometimes there would be an extra (empty) line added to the titles string, this gets rid of it incase it pops up
  titles.removeAll(QString(""));
  int value = QRandomGenerator::global() -> bounded(titles.size());
  this -> setWindowTitle("AutoHelljacks: " + titles[value]);

}

void MainWindow::SetStart() {
    this->status = 1;
    this -> centralWidget() -> findChild < QPushButton * > ("startButton") -> setDisabled(true);
    this -> centralWidget() -> findChild < QPushButton * > ("stopButton") -> setDisabled(false);
    this -> centralWidget() -> findChild < QPushButton * > ("pauseButton") -> setDisabled(false);
    this -> centralWidget() -> findChild < QLabel * > ("statusLabel") -> setText("<html><head/><body><p>Current Status: <span style=\" font-weight:700; color:#1fa700;\">Active</span></p></body></html>");
}

void MainWindow::SetPaused() {
    this->status = 2;
    this -> centralWidget() -> findChild < QPushButton * > ("startButton") -> setDisabled(false);
    this -> centralWidget() -> findChild < QPushButton * > ("stopButton") -> setDisabled(false);
    this -> centralWidget() -> findChild < QPushButton * > ("pauseButton") -> setDisabled(true);
    this -> centralWidget() -> findChild < QLabel * > ("statusLabel") -> setText("<html><head/><body><p>Current Status: <span style=\" font-weight:700; color:#7c83ff;\">Paused</span></p></body></html>");
}

void MainWindow::SetStopped() {
    this->status = 0;
    this -> centralWidget() -> findChild < QPushButton * > ("startButton") -> setDisabled(false);
    this -> centralWidget() -> findChild < QPushButton * > ("stopButton") -> setDisabled(true);
    this -> centralWidget() -> findChild < QPushButton * > ("pauseButton") -> setDisabled(true);
    this -> centralWidget() -> findChild < QLabel * > ("statusLabel") -> setText("<html><head/><body><p>Current Status: <span style=\" font-weight:700; color:#ff0000;\">Stopped</span></p></body></html>");
}



MainWindow::MainWindow(QWidget * parent): QMainWindow(parent), ui(new Ui::MainWindow) {
  this -> setFixedSize(QSize(570, 260));
  ui -> setupUi(this);
  this -> GenerateTitles();
  this -> status = 0;
  this -> SetUpHook();
  windowPointer = this;
  // set up manuals
  //⌅


  QDirIterator it(":/manuals", QDirIterator::Subdirectories);
  int count = 0;
  while (it.hasNext()) {
    QFile file(it.next());
    file.open(QIODevice::ReadOnly);
    QStringList lines = QString(file.readAll()).split('\n');
    QString name = lines[0].trimmed().mid(5); //name:
    QByteArray ba = name.toLocal8Bit();
    const char * charName = ba.data();
    QByteArray baFileName = file.fileName().toLocal8Bit();
    const char * fileCharName = baFileName.data();
    QRadioButton * button = new QRadioButton(QApplication::translate(fileCharName, charName));
    // see line 158
    button->setObjectName(fileCharName);
    // by default helljack(s) will be selected
    //QString comparer = QString("Helljack");
    if (name == QString("Helljack")) {
      button -> setChecked(true);
      this -> centralWidget() -> findChild < QGridLayout * > ("manualGridLayout") -> addWidget(button, 0, 0);
    } else {
      this -> centralWidget() -> findChild < QGridLayout * > ("manualGridLayout") -> addWidget(button, 1 + count, 0);
    }
    count++;
  }

}

MainWindow::~MainWindow() {
  ::UnhookWindowsHookEx(this -> Hook);

  delete ui;
}

void MainWindow::on_startButton_clicked() {
  HWND window = FindWindowW(NULL, L"Roblox");
  if (window != 0) {
    //this->centralWidget()->findChild<QGridLayout *>("manualGridLayout")
    SetForegroundWindow(window);
    this -> window = window;
    if (this -> status == 0) {
      QButtonGroup group;
      QList<QRadioButton *> allButtons = this -> centralWidget() -> findChild < QWidget * > ("scrollContents") -> findChildren<QRadioButton *>();
      for(int i = 0; i < allButtons.size(); ++i)
      {
         group.addButton(allButtons[i],i);
      }
      QFile file(group.checkedButton()->objectName());
      file.open(QIODevice::ReadOnly);
      QStringList readerLines = QString(file.readAll()).split('\n');
      // AKA: delete first line, since this will always just read name:<etc>
      readerLines.takeFirst();
      file.close();

      //counter
      int countTimes = this -> centralWidget() -> findChild < QSpinBox * > ("counter")->value();
      QProgressBar* bar = this -> centralWidget() -> findChild < QProgressBar * > ("progress");
      bar->setValue(0);
      bar->setMaximum(countTimes);
      this->SetStart();
      this->ReadManual(readerLines, countTimes);
    } else if (this -> status == 2) {
         this->SetStart();
    }
  } else {
    QMessageBox messageBox;
    messageBox.warning(0, "Error", "Roblox was not found. Please make sure you have the Roblox app open, and then try again.");
    messageBox.setFixedSize(500, 200);
    return;
  }
}

void MainWindow::on_stopButton_clicked()
{
    this -> SetStopped();
}

void MainWindow::ReadManual(QStringList readerLines, int countTimes) {
    for (int countI = 0; countI < countTimes; ++countI) {
        for (int lineI = 0; lineI < readerLines.size(); ++lineI) {
            if (this->status == 0) {
                break;
            }
          //  QString line = readerLines[lineI].trimmed();
            this->ProcessLine(lineI, readerLines, countI);
            QProgressBar* bar = this -> centralWidget() -> findChild < QProgressBar * > ("progress");
            bar->setValue(countI + 1);
        }
        if (this->status == 0) {
            break;
        }
    }
}

void MainWindow::ProcessLine(int lineI, QStringList readerLines, int countTimes, QString nameFixedForLetters) {
    if (GetForegroundWindow() != this->window) {
        this->SetPaused();
    }
    while (this->status == 2)
    {
          delay(500);
          //if (GetForegroundWindow() == this->window) {
          //    this->SetStart();
          //}
      //  QThread::msleep(3000);
    }

    if (this->status == 0) {
        return;
    }
    QString line = readerLines[lineI].trimmed();
    qInfo() << "Reading line" << line;
    // as lines inside of a letterloop are lines on their own, the reader will go over them as if it was a normal line
    // this checks to see if the line is nested inside a letter loop
    if (line != QString("startLetterLoop") && readerLines.indexOf("endLetterLoop\r", lineI) != -1 && readerLines.indexOf("startLetterLoop\r", lineI) == -1 && nameFixedForLetters == QString("")) {
        return;
    }
    if (line == QString("startLetterLoop")) {
        int stopIndex = readerLines.indexOf("endLetterLoop\r", lineI);
        if (stopIndex == -1) {
            this->status = 0;
            QMessageBox messageBox;
            messageBox.warning(0, "Error", "An error occured while trying to run the manual.");
            messageBox.setFixedSize(500, 200);
           return;
        } else {
            QStringList letterLoop = readerLines.mid(lineI + 1, stopIndex - 1);
            QString nameNumber = QString::fromStdString(this->NumberToWord(countTimes)).trimmed();
            QString nameFixed = nameNumber.remove(" ");
                for (int letterI = 0; letterI < nameFixed.length(); letterI++) {
                     for (int countI = 0; countI < letterLoop.size(); ++countI) {

                    if (this->status == 0) {
                        break;
                    }

                  //  QString line = readerLines[lineI].trimmed();
                    this->ProcessLine(countI + (lineI + 1), readerLines, letterI, nameFixed);
                }
                if (this->status == 0) {
                    break;
                }
            }
        }
    } else if (line == QString("jump")) {
        double randomness = this -> centralWidget() -> findChild < QSpinBox * > ("randomness")->value() * 1.0;

        int preDelay = this -> centralWidget() -> findChild < QSpinBox * > ("preJump")->value() ;
        int randomPreDelay = preDelay * (1 + ((QRandomGenerator::global() -> bounded(randomness)) / 100));
        int postDelay = this -> centralWidget() -> findChild < QSpinBox * > ("postJump")->value() ;
        int randomPostDelay = postDelay * (1 + ((QRandomGenerator::global() -> bounded(randomness)) / 100));

        delay(randomPreDelay);
        this->PressKey(0x39, KEYEVENTF_SCANCODE);
        delay(30);
        this->PressKey(0x39, KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
        delay(randomPostDelay);
    } else if ( line.startsWith(QString("say:")) ) {
        QString fixedLine = line.sliced(4);
        if (nameFixedForLetters != QString("")) {
           QString letter = QString(nameFixedForLetters.at(countTimes));
           fixedLine.replace("%uppercaseLetter%", letter.toUpper());
           fixedLine.replace("%lowercaseLetter%", letter);
        } else {

        QString word = QString::fromStdString(this->NumberToWord(countTimes));
        fixedLine.replace("%uppercaseWord%", word.toUpper());
        fixedLine.replace("%lowercaseWord%", word);
        QString firstUpper = QString(word.at(0)).toUpper();
        fixedLine.replace("%grammarWord%", word.replace(0, 1, firstUpper));
        //letterDelay
        }

        double randomness = this -> centralWidget() -> findChild < QSpinBox * > ("randomness")->value() * 1.0;

        int letterDelay = this -> centralWidget() -> findChild < QSpinBox * > ("letterDelay")->value() * (fixedLine.length() * 1.0);
        int randomLetterDelay = letterDelay * (1 + ((QRandomGenerator::global() -> bounded(randomness)) / 100));
        int enterDelay = this -> centralWidget() -> findChild < QSpinBox * > ("preSend")->value() ;
        int randomEnterDelay = enterDelay * (1 + ((QRandomGenerator::global() -> bounded(randomness)) / 100));

        this->PressKey(0x35, KEYEVENTF_SCANCODE);//53
        delay(150);
        this->PressKey(0x35, KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);

        std::vector< INPUT > vec;
        delay(randomLetterDelay);
        for(auto chr : fixedLine)
        {

            INPUT key_input = { 0 };
            INPUT up_input = { 0 };

            key_input.type = INPUT_KEYBOARD;
            key_input.ki.dwFlags = KEYEVENTF_UNICODE;
            key_input.ki.wScan = chr.unicode();

            up_input.type = INPUT_KEYBOARD;
            up_input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
            up_input.ki.wScan = chr.unicode();
            vec.push_back(key_input);
            vec.push_back(up_input);
        }


        SendInput(vec.size(), vec.data(), sizeof(INPUT));
        // in case you tab out

        if (GetForegroundWindow() != this->window) {
            this->SetPaused();
        }

        while (this->status == 2)
        {
              delay(500);
              //if (GetForegroundWindow() == this->window) {
              //    this->SetStart();
              //}
          //  QThread::msleep(3000);
        }

        if (this->status == 0) {
            return;
        }


        delay(randomEnterDelay);
        this->PressKey(0x1c, KEYEVENTF_SCANCODE);//53
        delay(150);
        this->PressKey(0x1c, KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
    }
}

void MainWindow::PressKey(WORD key, DWORD flags) {
    INPUT keyPress[1] = {};
    ZeroMemory(keyPress, sizeof(keyPress));

    keyPress[0].type = INPUT_KEYBOARD;
    keyPress[0].ki.wVk = 0;
    keyPress[0].ki.wScan = key;
    keyPress[0].ki.dwFlags = flags;
    SendInput(ARRAYSIZE(keyPress), keyPress, sizeof(INPUT));
}


void MainWindow::on_pauseCombo_currentIndexChanged(int index)
{
    this->pauseKeybind = 112 + index;
}


void MainWindow::on_startCombo_currentIndexChanged(int index)
{
    this->startKeybind = 112 + index;
}



