#include "WegInverter.h"
#include "AppConfig.h"

WegInverter::WegInverter() : _status(STOPPED),
                             _currentFrequency(0.0),
                             _lastUpdateTime(0)
{
}

void WegInverter::begin(int rxPin, int txPin)
{
    if (appConfigMgr.config.inverterSimEnabled)
    {
        DEBUG_PRINTLN("[WEG] Inversor inicializado (Simulado)");
    }
    else
    {
        Serial2.begin(appConfigMgr.config.modbusBaudRate, SERIAL_8N1, rxPin, txPin);
        node.begin(appConfigMgr.config.modbusSlaveId, Serial2);

        DEBUG_PRINTLN("[WEG] Inversor inicializado");
    }
}

void WegInverter::update()
{
    if (appConfigMgr.config.inverterSimEnabled)
    {
        taskENTER_CRITICAL(&inverterMutex);
        // A lógica de simulação contínua (rampa) foi removida para simplificar.
        // A frequência agora é alterada diretamente por setFrequency.
        // Se o inversor estiver parado, a frequência é zerada.
        if (_status == STOPPED)
        {
            _currentFrequency = 0.0;
        }
        taskEXIT_CRITICAL(&inverterMutex);
    }
    else
    {
        // Debounce para não sobrecarregar a comunicação Modbus
        if (millis() - _lastUpdateTime < appConfigMgr.config.inverterUpdateRate)
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
            // O valor do inversor é de 0 a 600, que corresponde a 0-60Hz
            _currentFrequency = node.getResponseBuffer(0) / 10.0;

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
            DEBUG_PRINTLN("[WEG] Falha ao ler dados do inversor.");
        }
        taskEXIT_CRITICAL(&inverterMutex);
    }
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
    if (appConfigMgr.config.inverterSimEnabled)
    {
        taskENTER_CRITICAL(&inverterMutex);
        if (hz >= 0 && hz <= 60) // Limita a frequência
        {
            _currentFrequency = hz;
        }
        taskEXIT_CRITICAL(&inverterMutex);
        DEBUG_PRINTLN("[WEG SIM] Frequência definida para: " + String(hz, 2) + " Hz");
    }
    else
    {
        DEBUG_PRINTLN("[WEG] Definindo Frequência: " + String(hz, 2) + " Hz");
        // O valor para o inversor é de 0 a 8196, que corresponde a 0-60Hz
        uint16_t value = static_cast<uint16_t>(hz * 136.6f);
        node.writeSingleRegister(REG_WRITE_FREQUENCY, value);
    }
}

void WegInverter::start()
{
    if (appConfigMgr.config.inverterSimEnabled)
    {
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
            DEBUG_PRINTLN("[WEG SIM] Comando START recebido");
        }
        else
        {
            DEBUG_PRINTLN("[WEG SIM] Não é possível iniciar, frequência é 0.");
        }
    }
    else
    {
        DEBUG_PRINTLN("[WEG] Comando START enviado");

        node.writeSingleRegister(REG_WRITE_COMMAND, 7);
    }
}

void WegInverter::stop()
{
    if (appConfigMgr.config.inverterSimEnabled)
    {
        taskENTER_CRITICAL(&inverterMutex);
        _status = STOPPED;
        taskEXIT_CRITICAL(&inverterMutex);
        DEBUG_PRINTLN("[WEG SIM] Comando STOP recebido");
    }
    else
    {
        DEBUG_PRINTLN("[WEG] Comando STOP enviado");

        node.writeSingleRegister(REG_WRITE_COMMAND, 0);
    }
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