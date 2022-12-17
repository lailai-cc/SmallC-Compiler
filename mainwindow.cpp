#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAxWidget>
#include <QAxObject>
#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#include <QString>
#include <compiler.h>
#include <string.h>

int sumstep=0;
int nowstep=0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_OpenFileButton_clicked()
{
//    fileName = QFileDialog::getOpenFileName(this,
//        tr("Open file"), "./", tr("Files (*.txt *.c *.cpp)"));
//        //选择文件对话框/

        QFileDialog* f = new QFileDialog(this);
        f->setWindowTitle("Open file");
        f->setNameFilter("*.txt *.c *.cpp");
        f->setViewMode(QFileDialog::Detail);

        QString filePath;
        if(f->exec() == QDialog::Accepted){  /*选中文件*/

            filePath = f->selectedFiles()[0];

        ///文件内容//

        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            ui->errorText->appendPlainText("文件打开失败");
            return;
        }
        ui->inputTextEdit->clear();
        QTextStream readStream(&file);

        while(!readStream.atEnd())
        {
            ui->inputTextEdit->appendPlainText(readStream.readLine());
        }
        file.close();

        //清空其他框
        ui->resultText->clear();
        ui->tableText->clear();
        ui->pcodeText->clear();
        ui->stackText->clear();
        ui->outputText->clear();
        ui->debugText->clear();
        }


}

void MainWindow::on_SaveFileButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save File"),
            "",
            tr("Code Files (*.txt *.c *.cpp)"));

        if (!fileName.isNull())
        {
            QFile f(fileName);
            if(f.open(QFile::WriteOnly|QFile::Text)){

            QTextStream out(&f);
            out << ui->inputTextEdit->toPlainText();
            }
            f.close();

        }

}

void MainWindow::displayResult()
{
    QFile resultf("../fresult.txt");
    ui->resultText->clear();
    QTextStream readResultStream(&resultf);
    if(!resultf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("result文件打开失败");
        return;
    }
    while(!readResultStream.atEnd())
    {
        ui->resultText->appendPlainText(readResultStream.readLine());
    }
    resultf.close();

    QFile tablef("../ftable.txt");
    ui->tableText->clear();
    QTextStream readTableStream(&tablef);
    if(!tablef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("table文件打开失败");
        return;
    }
    while(!readTableStream.atEnd())
    {
        ui->tableText->appendPlainText(readTableStream.readLine());
    }
    tablef.close();

    QFile pcodef("../fcode.txt");
    QTextStream readPcodeStream(&pcodef);
    ui->pcodeText->clear();
    if(!pcodef.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("pcode文件打开失败");
        return;
    }
    while(!readPcodeStream.atEnd())
    {
        ui->pcodeText->appendPlainText(readPcodeStream.readLine());
    }
    pcodef.close();

    QFile outputf("../foutput.txt");
    QTextStream readOutputStream(&outputf);
    ui->outputText->clear();
    if(!outputf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("output文件打开失败");
        return;
    }
    while(!readOutputStream.atEnd())
    {
        ui->outputText->appendPlainText(readOutputStream.readLine());
    }
    outputf.close();
}


void MainWindow::displayDebug()
{
    QFile sresultf("./fsresult.txt");
    ui->resultText->clear();
    QTextStream readSresultStream(&sresultf);
    if(!sresultf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("result文件打开失败");
        return;
    }
    while(!readSresultStream.atEnd())
    {
        ui->resultText->appendPlainText(readSresultStream.readLine());
    }
    sresultf.close();

    QFile stackf("./fstack.txt");
    ui->stackText->clear();
    QTextStream readStackStream(&stackf);
    if(!stackf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("stack文件打开失败");
        return;
    }
    while(!readStackStream.atEnd())
    {
        ui->stackText->appendPlainText(readStackStream.readLine());
    }
    stackf.close();

    QFile debugf("./fscode.txt");
    QTextStream readDebugStream(&debugf);
    ui->debugText->clear();
    if(!debugf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("debug文件打开失败");
        return;
    }
    while(!readDebugStream.atEnd())
    {
        ui->debugText->appendPlainText(readDebugStream.readLine());
    }
    debugf.close();
}

void MainWindow::on_buildRunButton_clicked()
{
    sumstep=0;
    //将文本框数据取出并按行排列
    QFile myfile("input.txt");
    if (myfile.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&myfile);
        out << ui->inputTextEdit->toPlainText();
    }

    //编译运行
    int complete=compiler();

    if(complete==0){
        displayResult();
    }
    else{
        ui->errorText->appendPlainText("运行出错");
        //清空其他框
        ui->resultText->clear();
        ui->tableText->clear();
        ui->pcodeText->clear();
        ui->stackText->clear();
        ui->debugText->clear();
    }

    QFile file("../fcode.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->errorText->appendPlainText("文件打开失败");
        return;
    }

    QTextStream readStream(&file);
    while(!readStream.atEnd())
    {
        readStream.readLine();
        sumstep+=1;                 //统计总行数
    }
    file.close();

    nowstep=0;
}

void MainWindow::on_pushButton_clicked()
{
    //if(step<=stepsum)
    ++nowstep;
    printf("step%d\n",nowstep);
    bool finish=interpret(nowstep,sumstep);

    displayDebug();
    if(finish){
        nowstep=0;
    }

}

void MainWindow::on_inputButton_clicked()
{
    //将文本框数据取出并按行排列
    QFile myfile("sinput.txt");
    if (myfile.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&myfile);
        out << ui->inputText->toPlainText();
    }
}
