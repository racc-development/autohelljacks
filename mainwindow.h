    #ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include<Windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    HHOOK Hook;
    int count;
    int toDo;
    int status;
    int pauseKeybind = 117;
    int startKeybind = 116;

    ~MainWindow();
private slots:
  void on_startButton_clicked();

  void on_pauseCombo_currentIndexChanged(int index);

  void on_startCombo_currentIndexChanged(int index);

  void on_stopButton_clicked();

private:
    void GenerateTitles();
    void SetStart();
    void SetStopped();
    void SetPaused();
    void SetUpHook();

    void ReadManual(QStringList readerLines, int countTimes);
    void ProcessLine(int lineI, QStringList readerLines, int countTimes, QString nameFixedForLetters = "");
    void PressKey(WORD key, DWORD flags);


    HWND window;
    std::string NumberToWord(int number, bool referred = false);
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
