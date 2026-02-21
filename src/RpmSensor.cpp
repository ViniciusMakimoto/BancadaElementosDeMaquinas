#include "RpmSensor.h"

// Nota: Para interrupções em classes C++, geralmente precisamos de métodos estáticos ou wrappers globais.
// Esta é uma implementação simplificada para o esqueleto.

RpmSensor::RpmSensor(uint8_t pin)
{
    _pin = pin;
    _pulseCount = 0;
    _lastCalculationTime = 0;
}

void RpmSensor::begin()
{
    pinMode(_pin, INPUT_PULLUP);
    // attachInterrupt aqui exigirá um wrapper global no main ou uma função estática
}

void IRAM_ATTR RpmSensor::handleInterrupt()
{
    _pulseCount++;
}

int RpmSensor::getRPM()
{
    unsigned long currentTime = millis();
    if (currentTime - _lastCalculationTime >= 1000)
    {
        // Exemplo simples de cálculo (pulsos por segundo * 60)
        int rpm = _pulseCount * 60;
        _pulseCount = 0;
        _lastCalculationTime = currentTime;
        return rpm;
    }
    return -1; // Indica que não houve atualização
}