#ifndef ENDSTOPS_H
#define ENDSTOPS_H

#include "serialcom.h"
#include "actuator.h"

class endstops : public QObject
{
    Q_OBJECT
public:
    explicit endstops(serialcom *serial, actuator *ac, QObject *parent = 0);

    struct limitStatus{
        int x_min;
        int x_max;
        int y_min;
        int y_max;
        int z_min;
        int z_max;
        int probe;
    };
    void getLimitSwitch_status();

private:
    enum limits{
        Xmin,
        Xmax,
        Ymin,
        Ymax,
        Zmin,
        Zmax,
    };

    serialcom *com;
    actuator *act;
    limitStatus switchStatus;

    void limitSwitch_triggered(endstops::limits switchName);
    void updateLimitSwitch_status(QString str);

signals:
    void sig_updateLimitStatus(endstops::limitStatus switches);
    void sig_limitSwitchTriggered(endstops::limits switchName);
public slots:
    void serialReceived(QString str);
};

#endif // ENDSTOPS_H
