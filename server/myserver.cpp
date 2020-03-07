#include "myserver.h"

myserver::myserver()
    {

    }

myserver::~myserver()
    {

    }

void myserver:: startServer()
    {

    if(this->listen(QHostAddress::Any,5555))
        {
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("D:\\SQLiteDatabaseBrowserPortable\\Data\\workers.db");
            if (db.open())
                {
                    qDebug()<<"Listening and DB open";
                    qDebug()<<"Listening";
                }
            else
                {
                    qDebug()<<"DB not open";
                }

        }
    else
        {
            qDebug()<<"Not listening";
        }
    }

void myserver:: incomingConnection(int socketDescriptor)
    {
        socket = new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);

            connect(socket, SIGNAL(readyRead()), this, SLOT(sockReady()));
            connect(socket, SIGNAL(disconnected()),this, SLOT(sockDisc()));
        qDebug()<<socketDescriptor<<"Client connected";

        socket->write("{\"type\":\"connect\",\"status\":\"yes\"}");
        qDebug()<<"Send client status - YES";
    }

void myserver:: sockReady()
    {
        Data = socket->readAll();
        qDebug()<<"Data: "<<Data;

        doc = QJsonDocument::fromJson(Data, &docError);

        if(docError.errorString().toInt() == QJsonParseError::NoError)
            {
                if((doc.object().value("type").toString() == "recSize") && (doc.object().value("resp").toString() == "workers"))
                    {
                        itog = "{\"type\":\"resultSelect\",\"result\":[";
                        if(db.isOpen())
                            {
                                QSqlQuery* query = new QSqlQuery(db);
                                if(query->exec("SELECT name FROM listworkers"))
                                    {
                                        while(query->next())
                                            {
                                                itog.append("{\"name\":\"" + query->value(0).toString()+"\"},");
                                            }
                                        itog.remove(itog.length()-1, 1);

                                    }
                                else
                                    {
                                        qDebug()<<"Not succes";
                                    }
                                delete query;

                            }
                        itog.append("]}");

                        socket->write("{\"type\":\"size\",\"resp\":\"workers\",\"length\":" + QByteArray::number(itog.size())+"}");
                        socket->waitForBytesWritten(500);
                    }
                else if ((doc.object().value("type").toString() == "select") && (doc.object().value("params").toString() == "workers"))
                    {
                            socket->write(itog);
                            qDebug()<<"Size message: "<< socket->bytesToWrite();
                            socket->waitForBytesWritten(500);
                    }
            }
    }

void myserver:: sockDisc()
    {
        qDebug()<<"Disconnect";
        socket->deleteLater();

    }
