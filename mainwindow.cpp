#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent, QString fileName) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings set;

    if(fileName.isEmpty()){

        inputFile.setFile(set.value("inputFile", "").toString());
        loadFile();
    }
    else{
        inputFile.setFile(fileName);
        loadFile();
    }

    ui->statusBar->addPermanentWidget(&status, 1);
    ui->actionDisable_Hirarchy_Warning->setChecked(set.value("disableHierarchyWarning").toBool());

    if(ui->actionDisable_Hirarchy_Warning->isChecked() == false){
        on_actionDisable_Hirarchy_Warning_triggered(false);
    }
}


MainWindow::~MainWindow()
{
    QSettings set;
    set.setValue("inputFile", inputFile.absoluteFilePath());
    set.setValue("disableHierarchyWarning", ui->actionDisable_Hirarchy_Warning->isChecked());
    delete ui;
}


void MainWindow::loadFile()
{
    ui->listWidget_original->clear();
    ui->listWidget_new->clear();
    inputLines.clear();
    inputLinesWithoutSheets.clear();
    sheetsLines.clear();
    status.clear();

    QFile f(inputFile.absoluteFilePath());
    if (f.open(QFile::ReadOnly)) {
        char buf[1024];
        qint64 lineLength;
        do{
            lineLength = f.readLine(buf, sizeof(buf));
            if (lineLength != -1) {
                QString line(buf);
                inputLines.append(line);
            }
        }while(lineLength != -1);

        status.setText(inputFile.fileName());
        status.setToolTip(inputFile.absoluteFilePath());
    }
    qDebug() << inputLines.size() << "lines read from file";

    // parse each line, and extract the sheets
    bool sheetOpen = false;
    int sheetCounter = 0;
    QVector<QString> sheetLines;
    for(int i=0; i<inputLines.size(); i++){
        QString line = inputLines[i];

        if(line.contains("$Sheet")){
            sheetOpen = true;
            sheetLines.clear();
            qDebug() << "new sheet";
        }

        // separate lines of input file
        if(sheetOpen){
            sheetLines.append(line);
        }else{
            inputLinesWithoutSheets.append(line);
        }

        if(line.contains("$EndSheet")){
            sheetOpen = false;
            sheetsLines.append(sheetLines);
            qDebug() << "sheetLines added to collection";
            sheetCounter++;
        }
    }
    qDebug() << "found" << sheetCounter << "sheets in the schematic";

    // Debug output:
//    for(int s=0; s<sheetsLines.size(); s++){
//        QString sheetName;
//        QString sheetFileName;
//        // for each line in a sheet
//        for(int l=0; l<sheetsLines[s].size(); l++){
//            QString line = sheetsLines[s][l];
//            qDebug() << s+1 << "::" << l <<" =" << line;
//        }
//    }


    // parse each sheet, and show them in the table
    for(int s=0; s<sheetsLines.size(); s++){
        QString sheetName = getSheetNameFromOriginalSheetIndex(s);

        ui->listWidget_original->addItem(QString::number(s+2) + " " + sheetName);
        QListWidgetItem* item = new QListWidgetItem(sheetName, 0, 1000+s); // set item type to sheet index (starting with 1000)
        ui->listWidget_new->addItem(item);
    }
}

QString MainWindow::extractNameFromSchLine(QString line)
{
    int i1=-1; // index of first "
    int i2=0; // index of last "

    for(int i=0; i<line.size() && i1==-1; i++){
        if(line[i] == '\"'){
            i1 = i;
            break;
        }
    }
    for(int i=line.size()-1; i>=0;  i--){
        if(line[i] == '\"'){
            i2 = i;
            break;
        }
    }


    if(line[i1] != '\"' || line[i2] != '\"' || i1 == line.size()-1 || i2<i1){
        QMessageBox::critical(this, "Error", "Error in extractNameFromSchLine()");
        return QString();
    }

    return line.mid(i1+1, i2-i1-1);
}

void MainWindow::on_actionOpen_triggered()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open KiCad Schematic file", inputFile.absolutePath(), "*.sch");

    if(fn.isEmpty()){
        return;
    }
    ui->listWidget_original->clear();
    ui->listWidget_new->clear();
    inputFile.setFile(fn);
    status.setText(fn);
    loadFile();
}


QString MainWindow::getSheetNameFromOriginalSheetIndex(int s)
{
    for(int l=0; l<sheetsLines[s].size(); l++){
        QString line = sheetsLines[s][l];
        if(line.startsWith("F0 ")){
            return extractNameFromSchLine(line);
        }
    }

    return QString();
}

void MainWindow::on_pushButton_renumber_pressed()
{
    QListWidgetItem* item=0;
    for(int i=0; i<ui->listWidget_new->count(); i++){
        item = ui->listWidget_new->item(i);
        int originalSheetIndex = item->type()-1000;
        QString sheetName = getSheetNameFromOriginalSheetIndex(originalSheetIndex);
        item->setText(QString::number(i+2) + " " + sheetName);
    }
}

void MainWindow::on_pushButton_renumber_released()
{
    QListWidgetItem* item=0;
    for(int i=0; i<ui->listWidget_new->count(); i++){
        item = ui->listWidget_new->item(i);
        int originalSheetIndex = item->type()-1000;
        QString sheetName = getSheetNameFromOriginalSheetIndex(originalSheetIndex);
        item->setText(sheetName);
    }
}

void MainWindow::on_actionSave_triggered()
{
    QFile original(inputFile.absoluteFilePath());
    QString bakFileName = inputFile.absoluteFilePath() + ".bak";
    QFile backup(bakFileName);
    backup.remove();
    original.copy(bakFileName);

    if(original.open(QIODevice::WriteOnly)){
        for(int l=0; l<inputLinesWithoutSheets.size(); l++){
            if(inputLinesWithoutSheets[l].startsWith("$EndSCHEMATC")){
                // insert all sheets before adding the $EndSCHEMATC-line

                for(int s=0; s<ui->listWidget_new->count(); s++){ // for all sheets
                    int osi = ui->listWidget_new->item(s)->type()-1000; // original sheet index
                    for(int sl=0; sl<sheetsLines[osi].size(); sl++){ // for all sheet lines
                        original.write(sheetsLines[osi][sl].toUtf8());
                    }
                }
            }
            original.write(inputLinesWithoutSheets[l].toUtf8());
        }
    }
    original.close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("KiCad Sheet Rearranger");
    msgBox.setTextFormat(Qt::RichText); //this is what makes the links clickable
    msgBox.setText("This tool is written by Karl Zeilhofer <br>"
                   "Please refer to <a href=\"http://www.zeilhofer.co.at/kicad-sheet-rearranger\">the project page</a>. <br>"
                   "It is publishe under the <a href=\"https://gnu.org/licenses/gpl.html\">GNU GPL License Version 3</a>.");
    msgBox.exec();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt (this, "KiCad Sheet Arranger");
}

void MainWindow::on_actionDisable_Hirarchy_Warning_triggered(bool checked)
{
    if(!checked){
        QMessageBox::information(this, "Flat Hierarchy Only!", "In this version of the KiCad Sheet Arranger only a flat hierarchy is supported. \n"
                                 "If you open and modify a schematic with sub-sub-sheets, then the shown sheet numbers are incorrect! \n"
                                 "Beside this, the program should do it's job anyway.");
    }
}
