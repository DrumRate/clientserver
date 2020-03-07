#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
        connect(socket, SIGNAL(readyRead()), this, SLOT(sockReady()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(sockDisc()));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    socket->connectToHost("127.0.0.1",5555);
}

void MainWindow::  sockDisc()
    {
        socket->deleteLater();
    }

void MainWindow:: sockReady()
    {
        if (socket->waitForConnected(500))
        {
            socket->waitForReadyRead(500);
            if(!complexData)
                {
                    Data = socket->readAll();
                }
            else
                {
                    Data.append(socket->readAll());
                    complexData = false;
                }
            doc = QJsonDocument:: fromJson(Data, &docError);

            if(docError.errorString().toInt()==QJsonParseError::NoError)
                {
                    if((doc.object().value("type").toString() == "connect") && (doc.object().value("status").toString() == "yes"))
                        {
                            QMessageBox::information(this, "Информация", "Соединение установлено");
                        }
                    else if(doc.object().value("type").toString() == "resultSelect")
                        {
                            QStandardItemModel* model = new QStandardItemModel(nullptr);
                            model->setHorizontalHeaderLabels(QStringList()<<"name");

                            QJsonArray docAr = doc.object().value("result").toArray();
                            for (int i = 0; i<docAr.count();i++)
                                {
                                    QStandardItem* col = new QStandardItem(docAr[i].toObject().value("name").toString());
                                    model->appendRow(col);

                                }
                            ui->tableView->setModel(model);
                        }
                    else if ((doc.object().value("type").toString() == "size") && (doc.object().value("resp").toString() == "workers"))
                        {
                            requierSize = doc.object().value("lenght").toInt();
                            socket->write("{\"type\":\"recSize\",\"resp\":\"workers\"}");
                        }
                    else
                        {
                            complexData = true;
                            sockReady();
                        }
                }
            else
                {
                    QMessageBox::information(this, "Информация", "Ошибка с форматом передачи данных" + docError.errorString());
                }
        }
    }

void MainWindow::on_pushButton_2_clicked()
{
    if(socket->isOpen())
        {
            //socket->write("{\"type\":\"select\",\"params\":\"workers\"}");
            socket->write("{\"type\":\"recSize\",\"resp\":\"workers\"}");
            socket->waitForBytesWritten(500);
        }
    else
        {
            QMessageBox::information(this,"Информация", "Соединение не установлено");
        }
}
