#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FileSystem.h"
#include "WebServer.h"
// #include "WegInverter.h"
// #include "RpmSensor.h"

// --- Instanciação dos Objetos ---
WebServer webManager;
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
    if (!FileSystem::begin())
    {
        Serial.println("Falha crítica no FS. Sistema pode ficar instável.");
    }

    // Configura a API antes de iniciar o servidor
    webManager.configureApi(
        // Callback para GET /api/data (Retorna JSON)
        []() -> String
        {
            // Cria um documento JSON para armazenar os dados
            StaticJsonDocument<256> doc;

            // Gera valores aleatórios para simular os 4 sensores de RPM
            doc["rpm1"] = random(800, 1200);
            doc["rpm2"] = random(700, 1100);
            doc["rpm3"] = random(600, 1000);
            doc["rpm4"] = random(500, 900);

            // Serializa o JSON para uma string
            String output;
            serializeJson(doc, output);
            return output;
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