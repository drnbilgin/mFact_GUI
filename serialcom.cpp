#include "serialcom.h"

#define loopTime    5

serialcom::serialcom(QString comPort, QObject *parent) : QObject(parent)
{
    waitOK = false;
    timeoutOK = false;
    maxQueue_size = 128;
    timeout = 180000;

    serial.setPortName(comPort);
    serial.setBaudRate(QSerialPort::Baud57600);
    serial.setDataBits(QSerialPort::Data8);             // 8 bits.
    serial.setParity(QSerialPort::NoParity);            // No party bit.
    serial.setStopBits(QSerialPort::OneStop);           // One stop bit.
    serial.setFlowControl(QSerialPort::NoFlowControl);  // No flow control.

    // Connect signals to slots
    connect(&serial,SIGNAL(readyRead()),this,SLOT(serialReceive_handler()));
    connect(&tmr_loop,SIGNAL(timeout()),this,SLOT(loop()));
    connect(&tmr_timeout,SIGNAL(timeout()),this,SLOT(serialTimeout()));

}

bool serialcom::openPort()
{
    return(serial.open(QIODevice::ReadWrite));
}

void serialcom::closePort()
{
    serial.close();
}

/*
 *  Add new command to tail of the queue
 *  returns false if queue full
 */
bool serialcom::send(QString data)
{
    if(sendQueue.size() >= maxQueue_size) // if there is no room for new command return false
        return false;
    else
    {
        sendQueue.enqueue(data);

        this->resume();
        return true;
    }
}

// Pause sending commands
void serialcom::pause()
{
    if(tmr_loop.isActive())
        tmr_loop.stop();
}

// Resume sending commands
void serialcom::resume()
{
    if(!tmr_loop.isActive())
        tmr_loop.start(loopTime);
}

// Stop sending commands and flush the queue
void serialcom::stop()
{
    pause();
    clear();
}

void serialcom::clear()
{
    sendQueue.clear();
}

bool serialcom::isOpen()
{
    return serial.isOpen();
}

bool serialcom::isQueueEmpty()
{
    return sendQueue.isEmpty();
}

bool serialcom::isQueueFull()
{
    if(sendQueue.size() >= maxQueue_size)
        return true;
    else
        return false;
}

void serialcom::setQueueSize(int size)
{
    maxQueue_size = size;
}

void serialcom::slotSend(QString str)
{
    send(str);
}


void serialcom::loop()
{
    if(waitOK)  // If we waiting an answer dont send anything
        return;
    if(serial.isOpen()) // Check serial port is open
    {
        if(!timeoutOK)
        {
            if(!sendQueue.isEmpty())
            {
                QString str;

                str = sendQueue.dequeue() + "\n"; // Add line feed

                // If command is G M or T controller replies simply ok\n
                if(str.startsWith('G') || str.startsWith('M') || str.startsWith('T'))
                    isGcode = true;
                else
                    isGcode = false;

                if(serial.write(str.toStdString().c_str()) != -1)
                {
                    waitOK = true;
                    tmr_timeout.start(timeout);
                }
            }
            else
            {
                tmr_loop.stop();
                emit sig_queueEmpty();
            }
        }
        else
        {
            waitOK = false;
            emit sig_serialTimeout();
        }
    }
}

void serialcom::serialTimeout()
{
    timeoutOK = true;
    tmr_timeout.stop();
}

// Something received from smoothie
void serialcom::serialReceive_handler()
{
    // We cant read a line so do nothing and return
    if(!serial.canReadLine())
        return;

    tmr_timeout.stop();

    // Get string from buffer
    QString receivedString = QString::fromAscii(serial.readAll().data());

    if(receivedString == "ok\n")
    {
        waitOK = false;
        return;
    }
    if(receivedString.startsWith("ok") || !isGcode || receivedString.contains("ok") || receivedString.contains("!!"))
        waitOK = false;

    // Emit received string
    emit sig_lineReceived(receivedString.simplified());

}
