#pragma once
#include <Arduino.h>
// #include <ModbusMaster.h> // Descomente quando instalar a lib

class WegInverter
{
private:
    // ModbusMaster node;
    uint8_t _slaveId;

public:
    WegInverter(uint8_t slaveId);
    void begin(int rxPin, int txPin);
    void setFrequency(float hz);
    void start();
    void stop();
    float readCurrent(); // Exemplo de leitura
};