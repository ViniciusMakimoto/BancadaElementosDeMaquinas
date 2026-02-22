#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"

// Estrutura para armazenar os dados de cada sensor
struct SensorState
{
    volatile unsigned long pulseCount;
    volatile unsigned long lastInterruptTime;
    float rpm;
};

class RpmSensors
{
public:
    void begin();
    void update(); // Calcula os RPMs
    void getRpmsJson(JsonDocument &doc);

private:
    static void IRAM_ATTR isr0();
    static void IRAM_ATTR isr1();
    static void IRAM_ATTR isr2();
    static void IRAM_ATTR isr3();

    static void IRAM_ATTR handleInterrupt(int sensorIndex);

    static SensorState sensorStates[4];
    static const uint8_t sensorPins[4];
    static const unsigned long DEBOUNCE_MS = 5;
    unsigned long lastCalculationTime;
};
