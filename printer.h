#ifndef PRINTER_H
#define PRINTER_H

#include "serialcom.h"
#include "actuator.h"
#include <QTextStream>
#include <QFile>
#include <QTimer>
#include <QMessageBox>
#include <QPushButton>

class printer : public QObject
{
    Q_OBJECT
public:
    explicit printer(serialcom *serialcom, actuator *actp, QObject *parent = 0);

    enum states{
        idle,
        printing,
        stopped,
        paused,
        resuming,
        finished
    };

    bool start(QFile file);
    void stop();
    void pause();
    void resume();

    bool isPrinting();

private:



    serialcom *com;
    actuator *act;
    QTextStream gFile;
    QTimer tmr_sendLine;
    QMessageBox msgBox;

    int sendLine_period;
    int state;

signals:
    void sig_printState_changed(printer::states state);
public slots:
    void slot_tmr_sendLine();
    void slot_queueEmpty();
};

#endif // PRINTER_H
