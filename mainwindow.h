#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_OpenFileButton_clicked();

    void on_SaveFileButton_clicked();

    void on_buildRunButton_clicked();

    void on_pushButton_clicked();

    void on_inputButton_clicked();

private:
    Ui::MainWindow *ui;
    void displayResult();
    void displayDebug();
};

#endif // MAINWINDOW_H
