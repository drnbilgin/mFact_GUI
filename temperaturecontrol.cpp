#include "temperaturecontrol.h"

temperaturecontrol::temperaturecontrol(serialcom *serialcom,QObject *parent) : QObject(parent)
{
    com = serialcom;

    maxNozzle_temperature = 260;
    maxPlatform_temperature = 120;
    maxEnvironment_temperature = 60;

    connect(com,SIGNAL(sig_lineReceived(QString)),this,SLOT(serialReceived(QString)));
    connect(&tmr_temperatureUpdate,SIGNAL(timeout()),this,SLOT(periodic_updateTemperatures()));
}

/*
 *  Sends command to set nozzle temperature
 */
void temperaturecontrol::setNozzle_temperature(float temp)
{
    com->send("M104 S" + QString::number(temp));
}

/*
 * Sends command to set nozzle temperature and wait
 * Controller will not process any command before reach target temperature
 * and it will update temperature value every second
 */
void temperaturecontrol::setNozzle_temperatureWait(float temp)
{
    com->send("M109 S" + QString::number(temp));
}

/*
 *  Sends command to set platform temperature
 */
void temperaturecontrol::setPlatform_temperature(float temp)
{
    com->send("M140 S" + QString::number(temp));
}

/*
 * Sends command to set platform temperature and wait
 * Controller will not process any command before reach target temperature
 * and it will update temperature value every second
 */
void temperaturecontrol::setPlatform_temperatureWait(float temp)
{
    com->send("M190 S" + QString::number(temp));
}

/*
 * Starts a timer to read temperatures every specified period ms
 */
void temperaturecontrol::getTemperatures_periodicallyStart(int period)
{
    tmr_temperatureUpdate.start(period);
}

/*
 * Stops timer
 */
void temperaturecontrol::getTemperatures_periodicallyStop()
{
    tmr_temperatureUpdate.stop();
}

/*
 * Asks temperature valus from controller
 */
void temperaturecontrol::updateTemperatures()
{
    com->send("M105");
}

/*
 * Gets current temperature, target temperature and pwm values from received string
 * and saves them to a struct as float
 * M105 command
 */
void temperaturecontrol::getTemperatures(QString str)
{
    temperatures.nozzle.currentTemperature = checkInf(str.mid(str.indexOf("T") + 2,str.indexOf("/") - str.indexOf("T") - 2));
    temperatures.nozzle.targetTemperature = str.mid(str.indexOf("/",str.indexOf("T")) + 1,str.indexOf("@",str.indexOf("T"))  - str.indexOf("/",str.indexOf("T")) -1).toFloat();
    temperatures.nozzle.pwmValue = str.mid(str.indexOf("@",str.indexOf("T")) + 1,str.indexOf("B") - str.indexOf("@",str.indexOf("T")) - 1).toFloat();

    temperatures.platform.currentTemperature = checkInf(str.mid(str.indexOf("B") + 2,str.indexOf("/",str.indexOf("B")) - str.indexOf("B") - 2));
    temperatures.platform.targetTemperature = str.mid(str.indexOf("/",str.indexOf("B")) + 1,str.indexOf("@",str.indexOf("B"))  - str.indexOf("/",str.indexOf("B")) -1).toFloat();
    temperatures.platform.pwmValue = str.mid(str.indexOf("@",str.indexOf("B")) + 1,str.indexOf("N") - str.indexOf("@",str.indexOf("B")) - 1).toFloat();

    temperatures.environment.currentTemperature = checkInf(str.mid(str.indexOf("N") + 2,str.indexOf("/",str.indexOf("N")) - str.indexOf("N") - 2));
    temperatures.environment.targetTemperature = str.mid(str.indexOf("/",str.indexOf("N")) + 1,str.indexOf("@",str.indexOf("N"))  - str.indexOf("/",str.indexOf("N")) -1).toFloat();
    temperatures.environment.pwmValue = str.mid(str.indexOf("@",str.indexOf("N")) + 1,str.length() - str.indexOf("@",str.indexOf("N")) - 1).toFloat();

    // Emit new temperature values
    emit sig_updateTemperatures(temperatures);

    // Check max temperatures
    if(temperatures.nozzle.currentTemperature >= maxNozzle_temperature)
        throwError(maxTemp_nozzleErr);
    if(temperatures.platform.currentTemperature >= maxPlatform_temperature)
        throwError(maxTemp_platformErr);
    if(temperatures.environment.currentTemperature >= maxEnvironment_temperature)
        throwError(maxTemp_environmentErr);
}

/*
 * Gets current temperature, target temperature and pwm values from received string
 * and saves them to a struct as float
 * Set and wait commands
 */
void temperaturecontrol::getSetandWait_temperatures(QString str, QString designator)
{
    float temp,target,pwm;

    temp = str.mid(str.indexOf(designator) + 2, str.indexOf("/") - str.indexOf(designator) -2).toFloat();
    target = str.mid(str.indexOf("/") + 1,str.indexOf("@") - str.indexOf("/") - 1).toFloat();
    pwm = str.mid(str.indexOf("@") + 1, str.length() - str.indexOf("@")).toFloat();

    if(designator == "T")
    {
        temperatures.nozzle.currentTemperature = temp;
        temperatures.nozzle.targetTemperature = target;
        temperatures.nozzle.pwmValue = pwm;
    }
    if(designator == "B")
    {
        temperatures.platform.currentTemperature = temp;
        temperatures.platform.targetTemperature = target;
        temperatures.platform.pwmValue  = pwm;
    }

    sig_updateTemperatures(temperatures);
}

/*
 * Check temperature value if it is infinite that means something wrong with thermistor
 * returns -999 if something wrong otherwise returns temperature value
 */
float temperaturecontrol::checkInf(QString str)
{
    if(str.contains("inf"))
        return -999;
    else
        return str.toFloat();
}

void temperaturecontrol::throwError(temperatureError err)
{
    emit sig_temperatureError(err);
}

void temperaturecontrol::serialReceived(QString str)
{
    // We received response for M105 command
    if(str.startsWith("ok T:"))
    {
        getTemperatures(str);
    }
    // Received response for set nozzle temperature and wait command
    if(str.startsWith("T:"))
    {
        getSetandWait_temperatures(str,"T");
    }
    // Received response for set platform temperature and wait command
    if(str.startsWith("B:"))
    {
        getSetandWait_temperatures(str,"B");
    }

    // Errors
    if(str.contains("Error: MINTEMP or MAXTEMP triggered") || str.contains("Temperature reading is unreliable"))
    {
        if(str.contains("on T"))
            throwError(cantRead_nozzle);
        if(str.contains("on B"))
           throwError(cantRead_platform);
        if(str.contains("on N"))
            throwError(cantRead_environment);
    }
}

void temperaturecontrol::periodic_updateTemperatures()
{
    updateTemperatures();
}
