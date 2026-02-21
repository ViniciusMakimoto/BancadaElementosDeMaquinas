#include <Arduino.h>
#include "Config.h"
#include "FileSystemWrapper.h"
#include "WebServerManager.h"
// #include "WegInverter.h"
// #include "RpmSensor.h"

// --- Instanciação dos Objetos ---
WebServerManager webManager;
// WegInverter inversor(MODBUS_SLAVE_ID);

// Sensores (Exemplo para 1 sensor, replicar para os outros)
// RpmSensor sensor1(PIN_RPM_1);

// Wrapper de interrupção (necessário para attachInterrupt funcionar com classes)
// void IRAM_ATTR isrSensor1()
// {
//     sensor1.handleInterrupt();
// }

void setup()
{
    Serial.begin(115200);

    // 1. Inicializa Sistema de Arquivos (Wrapper)
    if (!FileSystemWrapper::begin())
    {
        Serial.println("Falha crítica no FS. Sistema pode ficar instável.");
    }

    // Configura a API antes de iniciar o servidor
    webManager.configureApi(
        // Callback para GET /api/data (Retorna JSON)
        []() -> String
        {
            // Aqui você montará o JSON real com ArduinoJson
            // Exemplo simplificado:
            return {}; //"{\"rpm1\": " + String(sensor1.getRPM()) + ", \"status\": \"OK\"}";
        },
        // Callback para POST /api/command (Recebe JSON)
        [](String body)
        {
            Serial.println("[API] Comando recebido: " + body);
            // Aqui você fará o parse do JSON e chamará inversor.setFrequency(), etc.
        });

    // 2. Inicializa Web Server e Wi-Fi
    webManager.begin();

    // 3. Inicializa Inversor
    // inversor.begin(PIN_RS485_RX, PIN_RS485_TX);

    // 4. Inicializa Sensores e Interrupções
    // sensor1.begin();
    // attachInterrupt(digitalPinToInterrupt(PIN_RPM_1), isrSensor1, RISING);
}

void loop()
{
    // O loop agora está livre de tarefas de rede (exceto o que roda em background no AsyncWebServer)
}