#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFileInfo>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, QString fileName = "");
    ~MainWindow();
    void loadFile();
    QString extractNameFromSchLine(QString line);
    QString getSheetNameFromOriginalSheetIndex(int s);



private slots:
    void on_actionOpen_triggered();

    void on_pushButton_renumber_pressed();

    void on_pushButton_renumber_released();

    void on_actionSave_triggered();

    void on_actionAbout_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionDisable_Hirarchy_Warning_triggered(bool checked);

private:
    Ui::MainWindow *ui;

    QLabel status;

    QFileInfo inputFile;
    QVector<QString> inputLines;
    QVector<QString> inputLinesWithoutSheets;
    QVector<QVector<QString> > sheetsLines;

};

#endif // MAINWINDOW_H
