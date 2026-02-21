#pragma once
#include <Arduino.h>

class RpmSensor
{
private:
    uint8_t _pin;
    volatile unsigned long _pulseCount;
    unsigned long _lastCalculationTime;

public:
    RpmSensor(uint8_t pin);
    void begin();
    void IRAM_ATTR handleInterrupt(); // Função chamada pela interrupção
    int getRPM();                     // Calcula e retorna o RPM atual
};