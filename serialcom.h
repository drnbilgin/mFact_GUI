#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <QObject>
#include <QCoreApplication>
#include <QtSerialPort/QSerialPort>
#include <QQueue>
#include <QTimer>

class serialcom : public QObject
{
    Q_OBJECT
public:
    explicit serialcom(QString comPort, QObject *parent = 0);
    bool openPort();
    void closePort();
    bool send(QString data);
    void pause();
    void resume();
    void stop();
    void clear();
    bool isOpen();
    bool isQueueEmpty();
    bool isQueueFull();
    void setQueueSize(int size);

private:
    QSerialPort serial;
    QQueue<QString> sendQueue;
    QTimer tmr_loop,tmr_timeout;
    bool waitOK, timeoutOK,isGcode;
    int maxQueue_size;
    int timeout;


signals:
    void sig_queueEmpty();
    void sig_lineReceived(QString str);
    void sig_serialTimeout();

public slots:
    void slotSend(QString str);

private slots:
    void serialReceive_handler();
    void loop();
    void serialTimeout();

};

#endif // SERIALCOM_H
