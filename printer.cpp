#include "printer.h"

printer::printer(serialcom *serialcom, actuator *actp, QObject *parent) : QObject(parent)
{
    com = serialcom;
    act = actp;

    state = idle;
    sendLine_period = 10;


    msgBox.setModal(true);
    msgBox.setWindowFlags(Qt::FramelessWindowHint);
    msgBox.setGeometry(250,150,150,150);
    msgBox.setCursor(Qt::BlankCursor);
    QPushButton *buttonYes = msgBox.addButton(tr("Evet"),QMessageBox::AcceptRole);
    buttonYes->setFixedSize(150,100);
    buttonYes->setFocusPolicy(Qt::NoFocus);
    QPushButton *buttonNo = msgBox.addButton(tr("Hayır"),QMessageBox::AcceptRole);
    buttonNo->setFixedSize(150,100);
    buttonNo->setFocusPolicy(Qt::NoFocus);

    //connect(com,SIGNAL(sig_lineReceived(QString)),this,SLOT(serialReceived(QString)));
    connect(com,SIGNAL(sig_queueEmpty()),this,SLOT(slot_queueEmpty()));
}

bool printer::start(QFile file)
{
    if(!file.isOpen())
        return false;

    gFile.setDevice(&file);

    state = printing;
    tmr_sendLine.start(sendLine_period);
    emit sig_printState_changed(printing);

    return true;
}

void printer::stop()
{
    msgBox.setText("Yazma işlemini durdurmak istediğinize emin misiniz?");
    int ret = msgBox.exec();
    if(ret == QMessageBox::Accepted)
    {
        tmr_sendLine.stop();
        state = stopped;
    }
}

void printer::pause()
{
    msgBox.setText("Yazma işlemini duraklatmak istediğinize emin misiniz?");
    int ret = msgBox.exec();
    if(ret == QMessageBox::Accepted)
    {
        tmr_sendLine.stop();
        state = paused;
    }
}

void printer::resume()
{
    act->move_z(-30,1500);
    act->move_e(10,1500);
    act->popSpeed();
    act->setMoving_mode(actuator::Absolute);

    tmr_sendLine.start(sendLine_period);
    emit sig_printState_changed(resuming);
    state = printing;
}

bool printer::isPrinting()
{
    return state == printing ? true : false;
}

void printer::slot_tmr_sendLine()
{
    QString line;


    if(com->isQueueFull())
        return;

    do
    {
        line = gFile.readLine();

        if(line.contains(";"))
            line.remove(line.indexOf(";"),line.length()-line.indexOf(";")); // Remove comments
    }while(line.isEmpty() && !line.isNull());

    if(gFile.atEnd())
    {
        tmr_sendLine.stop();
        state = finished;
        return;
    }

    com->send(line);

}

void printer::slot_queueEmpty()
{

    switch (state) {
    case stopped:
        emit sig_printState_changed(stopped);
        act->home(actuator::homeX);
        state = idle;
        break;
    case finished:
        emit sig_printState_changed(finished);
        state = idle;
        break;
    case paused:
        act->setMoving_mode(actuator::Relative);
        act->pushSpeed();
        act->move_e(-10,3000);
        act->move_z(30,1500);
        emit sig_printState_changed(paused);
        state = idle;
        break;
    default:
        break;
    }
}
