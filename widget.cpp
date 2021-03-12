#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , data(10000, '\1')
{
    ui->setupUi(this);
//    this->showFullScreen();
    connect(&server, SIGNAL(newConnection()), this, SLOT(connectionAppended()));
    connect(&client, SIGNAL(connected()), this, SLOT(clientConnected()));
    connect(&client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(&client, SIGNAL(readyRead()), this, SLOT(clientReadyRead()));

    on_pushButton_Listen_clicked();
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_Listen_clicked()
{
    if (!server.isListening())
    {
        int port = ui->spinBox_Port->value();
        bool ret = server.listen(QHostAddress::Any, port);
        if (ret)
        {
            ui->pushButton_Listen->setText("Unlisten");
            ui->textEdit->append(QString("Listened on port %1.").arg(port));
        }
        else
        {
            ui->textEdit->append(QString("Listen on port %1 failed.").arg(port));
        }
    }
    else
    {
        server.close();
        ui->textEdit->append("Unlistened");
        ui->pushButton_Listen->setText("Listen");
    }
}

void Widget::on_pushButton_Connect_clicked()
{
    if (!client.isOpen())
    {
        QString hostName = ui->lineEdit_Ip->text();
        int port = ui->spinBox_Port->value();
        client.connectToHost(hostName, port);
    }
    else
    {
        client.close();
        ui->pushButton_Connect->setText("Connect");
    }

}

void Widget::serverListened()
{

}

void Widget::connectionAppended()
{
    QTcpSocket* conn = server.nextPendingConnection();
    if (!conn) return;
    ui->textEdit->append(QString("Pending connection."));
    connect(conn, SIGNAL(disconnected()), this, SLOT(connectionDisconnected()));
    connect(conn, SIGNAL(readyRead()), this, SLOT(clientReadyRead()));
    conns.push_back(conn);

    connect(conn, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData(qint64)));
    conn->write(data);
}

void Widget::connectionDisconnected()
{
    QTcpSocket *conn = (QTcpSocket *)sender();
    conns.removeAll(conn);
    conn->deleteLater();
}

void Widget::connectionReadyRead()
{
    QTcpSocket *conn = (QTcpSocket *)sender();
    QByteArray data = conn->readAll();

}

void Widget::clientConnected()
{
    ui->pushButton_Connect->setText("Disconnect");

}

void Widget::clientDisconnected()
{
    ui->pushButton_Connect->setText("Connect");
    client.close();
}

void Widget::clientReadyRead()
{
    QByteArray data = client.readAll();
    int size = data.size();
    QTime ts = QTime::currentTime();
    Sample sample{ts, size};

    samples.push_back(sample);

    qint64 dt = samples.front().ts.elapsed();
    if (dt > 1000)
    {
        int sum_size = 0;
        for (auto it = samples.begin();  it != samples.end(); it++) {
            sum_size += it->size;
        }
        float bw = sum_size / dt;
        ui->textEdit->append(QString("bandwith: %1 KB/s.").arg(bw));
        samples.clear();
    }
}

void Widget::on_pushButton_Send_clicked()
{
    if (ui->pushButton_Send->text() == "Send")
    {
        for (auto it = conns.begin(); it != conns.end(); it++)
        {
            connect(*it, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData(qint64)));
            (*it)->write(data);
        }
        ui->pushButton_Send->setText("Stop");
    }
    else
    {
        for (auto it = conns.begin(); it!= conns.end(); it++)
        {
            disconnect(*it, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData(qint64)));
        }
        ui->pushButton_Send->setText("Send");
    }
}

void Widget::sendData(qint64 size)
{
    QTcpSocket *conn = (QTcpSocket *)sender();
    conn->write(data);
}
