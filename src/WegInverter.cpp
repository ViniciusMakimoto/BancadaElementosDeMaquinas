#include "WegInverter.h"

WegInverter::WegInverter(uint8_t slaveId) : _slaveId(slaveId),
                                            _status(STOPPED),
                                            _currentFrequency(0.0),
                                            _lastUpdateTime(0)
{
}

void WegInverter::begin(int rxPin, int txPin)
{
#if WEG_INVERTER_SIMULATION_ENABLED
    Serial.println("[WEG] Inversor inicializado (Simulado)");
#else
    Serial2.begin(MODBUS_BAUDRATE, SERIAL_8N1, rxPin, txPin);
    node.begin(_slaveId, Serial2);
    // Configura o pino de controle de direção do barramento RS485
    pinMode(PIN_RS485_DE, OUTPUT);
    digitalWrite(PIN_RS485_DE, LOW); // Modo de recepção por padrão

    // Adiciona callbacks para o controle do pino DE
    node.preTransmission([]()
                         { digitalWrite(PIN_RS485_DE, HIGH); });
    node.postTransmission([]()
                          { digitalWrite(PIN_RS485_DE, LOW); });

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
    // Debounce para não sobrecarregar a comunicação Modbus
    if (millis() - _lastUpdateTime < INVERTER_UPDATE_RATE)
    {
        return;
    }
    _lastUpdateTime = millis();

    uint8_t result;
    // Exemplo: Lendo a frequência do inversor (ajustar registradores)
    result = node.readHoldingRegisters(REG_READ_FREQUENCY, 1);

    taskENTER_CRITICAL(&inverterMutex);
    if (result == node.ku8MBSuccess)
    {

        // TODO: Validar se precisar de correção.
        // O valor para o inversor é de 0 a 8196, que corresponde a 0-60Hz
        _currentFrequency = node.getResponseBuffer(0) / 136.6f;

        // Por simplicidade, se a frequência > 0.1, está girando.
        if (_currentFrequency > 0.1)
        {
            _status = RUNNING;
        }
        else
        {
            _status = STOPPED;
        }
    }
    else
    {
        _status = FAULT; // Falha na comunicação
        _currentFrequency = 0;
        Serial.println("[WEG] Falha ao ler dados do inversor.");
    }
    taskEXIT_CRITICAL(&inverterMutex);
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
    Serial.println("[WEG] Definindo Frequência: " + String(hz, 2) + " Hz");
    // O valor para o inversor é de 0 a 8196, que corresponde a 0-60Hz
    uint16_t value = static_cast<uint16_t>(hz * 136.6f);
    node.writeSingleRegister(REG_WRITE_FREQUENCY, value);
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
    Serial.println("[WEG] Comando START enviado");

    node.writeSingleRegister(REG_WRITE_COMMAND, 7);
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
    Serial.println("[WEG] Comando STOP enviado");

    node.writeSingleRegister(REG_WRITE_COMMAND, 0);
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