#include "WegInverter.h"

WegInverter::WegInverter(uint8_t slaveId)
{
    _slaveId = slaveId;
}

void WegInverter::begin(int rxPin, int txPin)
{
    // Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
    // node.begin(_slaveId, Serial2);
    Serial.println("[WEG] Inversor inicializado (Simulado)");
}

void WegInverter::setFrequency(float hz)
{
    // Lógica Modbus para escrever no registrador de frequência
    Serial.printf("[WEG] Definindo Frequência: %.2f Hz\n", hz);
}

void WegInverter::start()
{
    Serial.println("[WEG] Comando START enviado");
}

void WegInverter::stop()
{
    Serial.println("[WEG] Comando STOP enviado");
}

float WegInverter::readCurrent()
{
    // Lógica Modbus para ler corrente
    return 0.0;
}