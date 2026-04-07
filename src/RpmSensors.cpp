#include "RpmSensors.h"
#include <math.h>
#include "AppConfig.h"

// --- Inicialização dos Membros Estáticos ---
SensorState RpmSensors::sensorStates[4];
const uint8_t RpmSensors::sensorPins[4] = {PIN_RPM_1, PIN_RPM_2, PIN_RPM_3, PIN_RPM_4};

// --- Implementação dos Métodos da Classe ---

void RpmSensors::begin()
{
    if (!appConfigMgr.config.sensorSimEnabled)
    {
        lastCalculationTime = 0;
        void (*isrFuncs[])() = {isr0, isr1, isr2, isr3};

        for (int i = 0; i < 4; i++)
        {
            pinMode(sensorPins[i], INPUT_PULLUP);
            attachInterrupt(digitalPinToInterrupt(sensorPins[i]), isrFuncs[i], RISING);
            sensorStates[i].periodInMicros = 0;
            sensorStates[i].lastInterruptTime = 0;
            sensorStates[i].rpm = 0.0;
        }
    }
}

void RpmSensors::update()
{
    if (!appConfigMgr.config.sensorSimEnabled)
    {
        unsigned long currentTime = millis();
        if (currentTime - lastCalculationTime >= appConfigMgr.config.sensorUpdateRate)
        {
            lastCalculationTime = currentTime;

            unsigned long periodArray[4];
            unsigned long lastIntArray[4];

            noInterrupts(); // Desativa interrupções para leitura segura

            for (int i = 0; i < 4; i++)
            {
                periodArray[i] = sensorStates[i].periodInMicros;
                lastIntArray[i] = sensorStates[i].lastInterruptTime;
            }

            interrupts();

            unsigned long currentMicros = micros();

            for (int i = 0; i < 4; i++)
            {
                // Se passou mais de 4 segundos desde a última interrupção, assumimos que o motor parou (0 RPM)
                if (currentMicros - lastIntArray[i] > 4000000UL)
                {
                    sensorStates[i].rpm = 0.0;
                }
                else if (periodArray[i] > 0)
                {
                    // Cálculo da frequência (Hz) = 1.000.000 / período (us)
                    float freqHz = 1000000.0 / periodArray[i];
                    sensorStates[i].rpm = (freqHz / appConfigMgr.config.pulsesPerRevolution) * 60.0;
                }
                else
                {
                    sensorStates[i].rpm = 0.0;
                }
            }
        }
    }
}

void RpmSensors::getRpmsJson(JsonDocument &doc)
{
    if (appConfigMgr.config.sensorSimEnabled)
    {
        // Modo Simulação: Gera valores aleatórios
        doc["rpm1"] = random(800, 1200);
        doc["rpm2"] = random(700, 1100);
        doc["rpm3"] = random(600, 1000);
        doc["rpm4"] = random(500, 900);
    }
    else
    {
        // Modo Real: Arredonda para 1 casa decimal
        doc["rpm1"] = round(sensorStates[0].rpm * 10.0) / 10.0;
        doc["rpm2"] = round(sensorStates[1].rpm * 10.0) / 10.0;
        doc["rpm3"] = round(sensorStates[2].rpm * 10.0) / 10.0;
        doc["rpm4"] = round(sensorStates[3].rpm * 10.0) / 10.0;
    }
}

// --- Implementação das Interrupções ---

void IRAM_ATTR RpmSensors::handleInterrupt(int sensorIndex)
{
    unsigned long now = micros();
    // A configuração é uma variável acessada aqui. Para maior segurança em ISR e menor latência, isso é aceitável no ESP32.
    unsigned long debounceMicros = appConfigMgr.config.sensorDebounceBase * 1000UL;
    unsigned long dt = now - sensorStates[sensorIndex].lastInterruptTime;

    if (dt >= debounceMicros)
    {
        if (sensorStates[sensorIndex].lastInterruptTime != 0)
        {
            sensorStates[sensorIndex].periodInMicros = dt;
        }
        sensorStates[sensorIndex].lastInterruptTime = now;
    }
}

void IRAM_ATTR RpmSensors::isr0() { handleInterrupt(0); }
void IRAM_ATTR RpmSensors::isr1() { handleInterrupt(1); }
void IRAM_ATTR RpmSensors::isr2() { handleInterrupt(2); }
void IRAM_ATTR RpmSensors::isr3() { handleInterrupt(3); }
