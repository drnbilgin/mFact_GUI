#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <QObject>
#include "serialcom.h"
#include <QTimer>

class actuator : public QObject
{
    Q_OBJECT
public:
    explicit actuator(serialcom *serialcom, QObject *parent = 0);

    enum homeDir{
        homeX,
        homeY,
        homeZ,
        homeXYZ
    };
    enum movingMode{
        Absolute,
        Relative
    };

    struct position{
        float X;
        float Y;
        float Z;
        float E;
    };

    void disableSteppers();
    void enableSteppers();

    void setTool(int tool);
    void setMoving_mode(movingMode mode);
    void pushSpeed();
    void popSpeed();

    void move_x(float x,float speed);
    void move_y(float y,float speed);
    void move_z(float z,float speed);
    void move_e(float e, float speed);
    void move(float x,float y,float speed);
    void move(float x,float y, float z,float speed);
    void move(float x,float y, float z,float e,float speed);

    void home(homeDir dir);
    void updatePositions();

private:
    serialcom *com;
    position currPositions, mileStones;
    bool isMoving;

    void updateMilestone();
    void getPositions(position *pos,QString str);

signals:
    void sig_updatePositions(actuator::position pos);

public slots:
    void serialReceived(QString str);
};

#endif // ACTUATOR_H
