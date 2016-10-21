#include "actuator.h"

actuator::actuator(serialcom *serialcom, QObject *parent) : QObject(parent)
{
    com = serialcom;

    connect(com,SIGNAL(sig_lineReceived(QString)),this,SLOT(serialReceived(QString)));
}

/*
 * Disables stepper motors
 */
void actuator::disableSteppers()
{
    com->send("M18");
}

/*
 * Enables stepper motors
 */
void actuator::enableSteppers()
{
    com->send("M17");
}

/*
 * Sets the active tool
 */
void actuator::setTool(int tool)
{
    if(tool < 0)
        return;

    com->send("T"+QString::number(tool));
}

/*
 * Sets moving mode
 * Absolute or Relative
 */
void actuator::setMoving_mode(actuator::movingMode mode)
{
    if(mode == actuator::Absolute)
        com->send("G90");
    if(mode == actuator::Relative)
        com->send("G91");
}

void actuator::pushSpeed()
{
    com->send("M120");
}

void actuator::popSpeed()
{
    com->send("M121");
}

/*
 * Moves X axis
 */
void actuator::move_x(float x, float speed)
{
    com->send("G1 X" + QString::number(x) + " F" + QString::number(speed));
    updateMilestone();
}

/*
 * Moves Y axis
 */
void actuator::move_y(float y, float speed)
{
    com->send("G1 Y" + QString::number(y) + " F" + QString::number(speed));
    updateMilestone();
}

/*
 * Moves Z axis
 */
void actuator::move_z(float z, float speed)
{
    com->send("G1 Z" + QString::number(z) + " F" + QString::number(speed));
    updateMilestone();
}

/*
 * Moves Extruder
 */
void actuator::move_e(float e, float speed)
{

    com->send("G1 E" + QString::number(e) + " F" + QString::number(speed));
    updateMilestone();
}

/*
 * Moves X and Y axis
 */
void actuator::move(float x, float y, float speed)
{
    com->send("G1 X" + QString::number(x) + " Y" + QString::number(y) + " F" + QString::number(speed));
    updateMilestone();
}

/*
 * Moves X, Y and Z axis
 */
void actuator::move(float x, float y, float z, float speed)
{
    com->send("G1 X" + QString::number(x) + " Y" + QString::number(y) + " Z" +QString::number(z) + " F" + QString::number(speed));
    updateMilestone();
}

/*
 * Moves X, Y and Z axis and Extruder
 */
void actuator::move(float x, float y, float z, float e, float speed)
{
    com->send("G1 X" + QString::number(x) + " Y" + QString::number(y) + " Z" +QString::number(z) + " E" +QString::number(e) + " F" + QString::number(speed));
    updateMilestone();
}

/*
 * Go home position
 */
void actuator::home(actuator::homeDir dir)
{
    QString str = "G28";

    switch(dir)
    {
    case homeX: str.append(" X0");
        break;
    case homeY: str.append(" Y0");
        break;
    case homeZ: str.append(" Z0");
        break;
    case homeXYZ:
        break;
    }

    com->send(str);
}

/*
 * Asks current positions from controller
 */
void actuator::updatePositions()
{
    com->send("M114.2");
}

/*
 * Asks current milestone from controller
 */
void actuator::updateMilestone()
{
    com->send("M114.4");
}

/*
 * Gets positions from received string and saves them a struct
 */
void actuator::getPositions(position *pos, QString str)
{
    pos->X = str.mid(str.indexOf("X:")+2,str.indexOf("Y:")-str.indexOf("X:")-2).toFloat();
    pos->Y = str.mid(str.indexOf("Y:")+2,str.indexOf("Z:")-str.indexOf("Y:")-2).toFloat();
    pos->Z = str.mid(str.indexOf("Z:")+2,str.length()-str.indexOf("Z:")-2).toFloat();
}


void actuator::serialReceived(QString str)
{
    // Received current positions
    if(str.contains("MPOS:"))
    {
        getPositions(&currPositions,str);
        emit sig_updatePositions(currPositions);
    }
    // Received current milestones
    if(str.contains("LMS:"))
    {
        getPositions(&mileStones,str);
    }
}
