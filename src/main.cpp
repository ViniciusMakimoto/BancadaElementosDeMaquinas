#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FileSystem.h"
#include "WebServer.h"
#include "RpmSensors.h"
// #include "WegInverter.h"

// --- Instanciação dos Objetos ---
WebServer webServer;
RpmSensors rpmSensors;
// WegInverter inversor(MODBUS_SLAVE_ID);

void setup()
{
    Serial.begin(115200);

    // 1. Inicializa Sistema de Arquivos
    if (!FileSystem::begin())
    {
        Serial.println("Falha crítica no FS. Sistema pode ficar instável.");
    }

    // Configura a API antes de iniciar o servidor
    webServer.configureApi(
        // Callback para GET /api/data (Retorna JSON com dados dos sensores)
        []() -> String
        {
            StaticJsonDocument<256> doc;
            rpmSensors.getRpmsJson(doc); // Preenche o JSON com os dados de RPM
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
    webServer.begin();

    // 3. Inicializa Inversor
    // inversor.begin(PIN_RS485_RX, PIN_RS485_TX);

    // 4. Inicializa Sensores e Interrupções
    rpmSensors.begin();
}

void loop()
{
    // Atualiza o cálculo de RPM (só faz algo se não estiver em modo simulação)
    rpmSensors.update();
}