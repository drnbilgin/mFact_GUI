#ifndef TEMPERATURECONTROL_H
#define TEMPERATURECONTROL_H

#include <QObject>
#include "serialcom.h"
#include <QTimer>

// TODO: PID auto tune

class temperaturecontrol : public QObject
{
    Q_OBJECT
public:
    explicit temperaturecontrol(serialcom *serialcom, QObject *parent = 0);

    enum temperatureError {
        cantRead_nozzle,
        cantRead_platform,
        cantRead_environment,
        maxTemp_nozzleErr,
        maxTemp_platformErr,
        maxTemp_environmentErr
    };

    struct controllerStruct{
        float currentTemperature;
        float targetTemperature;
        int pwmValue;
    };

    struct tempControl{
        controllerStruct nozzle;
        controllerStruct platform;
        controllerStruct environment;
    };

    void setNozzle_temperature(float temp);
    void setNozzle_temperatureWait(float temp);
    void setPlatform_temperature(float temp);
    void setPlatform_temperatureWait(float temp);

    void getTemperatures_periodicallyStart(int period);
    void getTemperatures_periodicallyStop();

    void updateTemperatures();


private:
    serialcom *com;
    float maxNozzle_temperature,maxPlatform_temperature,maxEnvironment_temperature;
    QTimer tmr_temperatureUpdate;
    tempControl temperatures;

    void getTemperatures(QString str);
    void getSetandWait_temperatures(QString str, QString designator);
    float checkInf(QString str);
    void throwError(temperatureError err);

signals:
    void sig_updateTemperatures(temperaturecontrol::tempControl temps);
    void sig_print(QString str);
    void sig_temperatureError(temperaturecontrol::temperatureError errCode);

public slots:
    void serialReceived(QString str);
    void periodic_updateTemperatures();
};

#endif // TEMPERATURECONTROL_H
