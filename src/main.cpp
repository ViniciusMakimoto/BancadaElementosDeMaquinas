#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FileSystem.h"
#include "WebServer.h"
#include "RpmSensors.h"
#include "WegInverter.h"

// --- Instanciação dos Objetos ---
WebServer webServer;
RpmSensors rpmSensors;
WegInverter inversor(MODBUS_SLAVE_ID);

// --- Buffer Global para JSON ---
// Pré-aloca o buffer para o JSON. Evita alocação de memória e serialização a cada requisição.
char jsonBuffer[256];
unsigned long lastJsonUpdateTime = 0;

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
        // Callback para GET /api/data (Retorna JSON com dados)
        []() -> String
        {
            return jsonBuffer;
        },
        // Callback para POST /api/command (Recebe JSON)
        [](String body)
        {
            StaticJsonDocument<128> doc;
            DeserializationError error = deserializeJson(doc, body);

            if (error)
            {
                Serial.print(F("[API] Falha no parse do JSON de comando: "));
                Serial.println(error.c_str());
                return;
            }

            // A UI pode enviar a frequência e o estado no mesmo comando
            if (doc.containsKey("frequency"))
            {
                float freq = doc["frequency"];
                inversor.setFrequency(freq);
            }

            if (doc.containsKey("motorState"))
            {
                const char *state = doc["motorState"];
                if (strcmp(state, "RUNNING") == 0)
                {
                    inversor.start();
                }
                else if (strcmp(state, "STOPPED") == 0)
                {
                    inversor.stop();
                }
            }
        });

    // 2. Inicializa Web Server e Wi-Fi
    webServer.begin();

    // 3. Inicializa Inversor
    inversor.begin(PIN_RS485_RX, PIN_RS485_TX);

    // 4. Inicializa Sensores e Interrupções
    rpmSensors.begin();
}

void loop()
{
    // Atualiza o cálculo de RPM (só faz algo se não estiver em modo simulação)
    rpmSensors.update();
    inversor.update();

    // Atualiza o JSON para a API em uma frequência controlada (ex: 2x por segundo)
    if (millis() - lastJsonUpdateTime > JSON_UPDATE_RATE)
    {
        lastJsonUpdateTime = millis();

        StaticJsonDocument<256> doc;
        rpmSensors.getRpmsJson(doc);
        inversor.getInverterDataJson(doc);
        serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));
    }
}