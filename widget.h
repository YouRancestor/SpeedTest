#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_Listen_clicked();

    void on_pushButton_Connect_clicked();

    void serverListened();

    void connectionAppended();
    void connectionDisconnected();
    void connectionReadyRead();

    void clientConnected();
    void clientDisconnected();
    void clientReadyRead();


    void on_pushButton_Send_clicked();

    void sendData(qint64 size);

private:
    Ui::Widget *ui;

    QTcpServer server;
    QTcpSocket client;

    QList<QTcpSocket*> conns;

    struct Sample
    {
        QTime ts;
        int size;
    };
    QList<Sample> samples;

    QByteArray data;

};
#endif // WIDGET_H
