#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"

#include <ModbusMaster.h>

enum InverterStatus
{
    STOPPED,
    RUNNING,
    FAULT
};

class WegInverter
{
private:
    ModbusMaster node;
    portMUX_TYPE inverterMutex = portMUX_INITIALIZER_UNLOCKED;
    uint8_t _slaveId;
    InverterStatus _status;
    float _currentFrequency;
    unsigned long _lastUpdateTime = 0;

    const char *getStatusString();

public:
    WegInverter(uint8_t slaveId);
    void begin(int rxPin, int txPin);
    void update();
    void getInverterDataJson(JsonDocument &doc);

    void setFrequency(float hz);
    void start();
    void stop();

    InverterStatus getStatus();
    float getCurrentFrequency();
};