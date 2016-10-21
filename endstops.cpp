#include "endstops.h"

endstops::endstops(serialcom *serial, actuator *ac, QObject *parent) : QObject(parent)
{
    com = serial;
    act = ac;

    connect(com,SIGNAL(sig_lineReceived(QString)),this,SLOT(serialReceived(QString)));
}

/*
 * Asks limits status from controller
 */
void endstops::getLimitSwitch_status()
{
    com->send("M119");
}

/*
 * When a limit switch triggered while moving this function emit a signal
 * and moves 2 mm away from limit switch
 */
void endstops::limitSwitch_triggered(endstops::limits switchName)
{

    com->clear();
    com->send("M999");
    act->setMoving_mode(actuator::Relative);

    switch(switchName)
    {
    case Xmin:
        act->move_x(2,2000);
        break;
    case Xmax:
        act->move_x(-2,2000);
        break;
    case Ymin:
        act->move_y(2,2000);
        break;
    case Ymax:
        act->move_y(-2,2000);
        break;
    case Zmin:
        act->move_z(2,2000);
        break;
    case Zmax:
        act->move_z(-2,2000);
        break;
    }

    emit sig_limitSwitchTriggered(switchName);
}

/*
 * Gets limits status from string
 */
void endstops::updateLimitSwitch_status(QString str)
{
    switchStatus.x_min = str.mid(str.indexOf("min_x:") + 6,1).toInt();
    switchStatus.x_max = str.mid(str.indexOf("max_x:") + 6,1).toInt();
    switchStatus.y_min = str.mid(str.indexOf("min_y:") + 6,1).toInt();
    switchStatus.y_max = str.mid(str.indexOf("max_y:") + 6,1).toInt();
    switchStatus.z_min = str.mid(str.indexOf("min_z:") + 6,1).toInt();
    switchStatus.z_max = str.mid(str.indexOf("max_z:") + 6,1).toInt();
    switchStatus.probe = str.mid(str.indexOf("Probe:") + 6,2).toInt();

    emit sig_updateLimitStatus(switchStatus);
}

void endstops::serialReceived(QString str)
{
    if(str.startsWith("Limit switch"))
    {
        if(str.contains("min_x"))
            limitSwitch_triggered(Xmin);
        if(str.contains("max_x"))
            limitSwitch_triggered(Xmax);
        if(str.contains("min_y"))
            limitSwitch_triggered(Ymin);
        if(str.contains("max_y"))
            limitSwitch_triggered(Ymax);
        if(str.contains("min_z"))
            limitSwitch_triggered(Zmin);
        if(str.contains("max_z"))
            limitSwitch_triggered(Zmax);
    }
    // Received limits status
    if(str.startsWith("min_x:"))
        updateLimitSwitch_status(str);

}
