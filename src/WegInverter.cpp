#include "WegInverter.h"

WegInverter::WegInverter(uint8_t slaveId) : _slaveId(slaveId),
                                            _status(STOPPED),
                                            _currentFrequency(0.0)
{
}

void WegInverter::begin(int rxPin, int txPin)
{
#if WEG_INVERTER_SIMULATION_ENABLED
    Serial.println("[WEG] Inversor inicializado (Simulado)");
#else
    // Serial2.begin(MODBUS_BAUDRATE, SERIAL_8N1, rxPin, txPin);
    // node.begin(_slaveId, Serial2);
    Serial.println("[WEG] Inversor inicializado");
#endif
}

void WegInverter::update()
{
#if WEG_INVERTER_SIMULATION_ENABLED
    taskENTER_CRITICAL(&inverterMutex);
    // A lógica de simulação contínua (rampa) foi removida para simplificar.
    // A frequência agora é alterada diretamente por setFrequency.
    // Se o inversor estiver parado, a frequência é zerada.
    if (_status == STOPPED)
    {
        _currentFrequency = 0.0;
    }
    taskEXIT_CRITICAL(&inverterMutex);
#else
    // A lógica de hardware leria os registros do inversor aqui periodicamente,
    // se necessário, para verificar status ou outros dados.
#endif
}

void WegInverter::getInverterDataJson(JsonDocument &doc)
{
    taskENTER_CRITICAL(&inverterMutex);
    doc["inverterStatus"] = getStatusString();
    doc["inverterFrequency"] = _currentFrequency;
    taskEXIT_CRITICAL(&inverterMutex);
}

void WegInverter::setFrequency(float hz)
{
#if WEG_INVERTER_SIMULATION_ENABLED
    taskENTER_CRITICAL(&inverterMutex);
    if (hz >= 0 && hz <= 60) // Limita a frequência
    {
        _currentFrequency = hz;
    }
    taskEXIT_CRITICAL(&inverterMutex);
    Serial.println("[WEG SIM] Frequência definida para: " + String(hz, 2) + " Hz");

#else
    // Lógica Modbus para escrever no registrador de frequência
    Serial.println("[WEG] Definindo Frequência: " + String(hz, 2) + " Hz");
    // node.writeSingleRegister(ADDRESS, hz); // Exemplo
#endif
}

void WegInverter::start()
{
#if WEG_INVERTER_SIMULATION_ENABLED
    bool started = false;
    taskENTER_CRITICAL(&inverterMutex);
    if (_currentFrequency > 0)
    {
        _status = RUNNING;
        started = true;
    }
    taskEXIT_CRITICAL(&inverterMutex);

    if (started)
    {
        Serial.println("[WEG SIM] Comando START recebido");
    }
    else
    {
        Serial.println("[WEG SIM] Não é possível iniciar, frequência é 0.");
    }
#else
    // Lógica Modbus para enviar comando de partida
    Serial.println("[WEG] Comando START enviado");
#endif
}

void WegInverter::stop()
{
#if WEG_INVERTER_SIMULATION_ENABLED
    taskENTER_CRITICAL(&inverterMutex);
    _status = STOPPED;
    taskEXIT_CRITICAL(&inverterMutex);
    Serial.println("[WEG SIM] Comando STOP recebido");
#else
    // Lógica Modbus para enviar comando de parada
    Serial.println("[WEG] Comando STOP enviado");
#endif
}

InverterStatus WegInverter::getStatus()
{
    taskENTER_CRITICAL(&inverterMutex);
    InverterStatus status = _status;
    taskEXIT_CRITICAL(&inverterMutex);
    return status;
}

float WegInverter::getCurrentFrequency()
{
    taskENTER_CRITICAL(&inverterMutex);
    float freq = _currentFrequency;
    taskEXIT_CRITICAL(&inverterMutex);
    return freq;
}

const char *WegInverter::getStatusString()
{
    // Esta função é privada e chamada apenas por métodos já protegidos,
    // então não precisa de um semáforo próprio para evitar deadlocks.
    switch (_status)
    {
    case RUNNING:
        return "Girando";
    case FAULT:
        return "Falha";
    case STOPPED:
    default:
        return "Parado";
    }
}