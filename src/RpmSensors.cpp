#include "RpmSensors.h"
#include <math.h>

// --- Inicialização dos Membros Estáticos ---
SensorState RpmSensors::sensorStates[4];
const uint8_t RpmSensors::sensorPins[4] = {PIN_RPM_1, PIN_RPM_2, PIN_RPM_3, PIN_RPM_4};

// --- Implementação dos Métodos da Classe ---

void RpmSensors::begin()
{
#if not SENSOR_SIMULATION_ENABLED
    lastCalculationTime = 0;
    void (*isrFuncs[])() = {isr0, isr1, isr2, isr3};

    for (int i = 0; i < 4; i++)
    {
        pinMode(sensorPins[i], INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(sensorPins[i]), isrFuncs[i], RISING);
        sensorStates[i].pulseCount = 0;
        sensorStates[i].lastInterruptTime = 0;
        sensorStates[i].rpm = 0.0;
    }
#endif
}

void RpmSensors::update()
{
#if not SENSOR_SIMULATION_ENABLED
    unsigned long currentTime = millis();
    if (currentTime - lastCalculationTime >= SENSOR_UPDATE_RATE)
    {
        unsigned long dt = currentTime - lastCalculationTime;
        lastCalculationTime = currentTime;

        unsigned long pulsesArray[4];

        noInterrupts(); // Desativa interrupções para leitura segura do contador

        for (int i = 0; i < 4; i++)
        {
            pulsesArray[i] = sensorStates[i].pulseCount;
            sensorStates[i].pulseCount = 0; // Reseta para a próxima janela
        }

        interrupts();

        for (int i = 0; i < 4; i++)
        {
            unsigned long pulses = pulsesArray[i];
            // Cálculo da frequência e RPM
            float freqHz = (pulses * 1000.0) / dt;
            sensorStates[i].rpm = (freqHz / PULSES_PER_REVOLUTION) * 60.0;
        }
    }
#endif
}

void RpmSensors::getRpmsJson(JsonDocument &doc)
{
#if SENSOR_SIMULATION_ENABLED
    // Modo Simulação: Gera valores aleatórios
    doc["rpm1"] = random(800, 1200);
    doc["rpm2"] = random(700, 1100);
    doc["rpm3"] = random(600, 1000);
    doc["rpm4"] = random(500, 900);
#else
    // Modo Real: Arredonda para 1 casa decimal
    doc["rpm1"] = round(sensorStates[0].rpm * 10.0) / 10.0;
    doc["rpm2"] = round(sensorStates[1].rpm * 10.0) / 10.0;
    doc["rpm3"] = round(sensorStates[2].rpm * 10.0) / 10.0;
    doc["rpm4"] = round(sensorStates[3].rpm * 10.0) / 10.0;
#endif
}

// --- Implementação das Interrupções ---

void IRAM_ATTR RpmSensors::handleInterrupt(int sensorIndex)
{
    unsigned long now = millis();
    if (now - sensorStates[sensorIndex].lastInterruptTime >= SENSOR_DEBOUNCE)
    {
        sensorStates[sensorIndex].pulseCount++;
        sensorStates[sensorIndex].lastInterruptTime = now;
    }
}

void IRAM_ATTR RpmSensors::isr0() { handleInterrupt(0); }
void IRAM_ATTR RpmSensors::isr1() { handleInterrupt(1); }
void IRAM_ATTR RpmSensors::isr2() { handleInterrupt(2); }
void IRAM_ATTR RpmSensors::isr3() { handleInterrupt(3); }
