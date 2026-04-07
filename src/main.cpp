#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "AppConfig.h"
#include "FileSystem.h"
#include "WebServer.h"
#include "RpmSensors.h"
#include "WegInverter.h"

// --- Instanciação dos Objetos ---
WebServer webServer;
RpmSensors rpmSensors;
WegInverter inversor;

// --- Buffer Global para JSON ---
// Pré-aloca o buffer para o JSON. Evita alocação de memória e serialização a cada requisição.
char jsonBuffer[256];
unsigned long lastJsonUpdateTime = 0;

void setup()
{
#if DEBUG_MODE
    Serial.begin(115200);
#endif

    // 1. Carrega configurações do NVS (deve ser o primeiro passo)
    appConfigMgr.load();

    // 2. Inicializa Sistema de Arquivos
    if (!FileSystem::begin())
    {
        DEBUG_PRINTLN("Falha crítica no FS. Sistema pode ficar instável.");
    }

    // Configura a API antes de iniciar o servidor
    webServer.configureApi(
        // Callback para GET /api/data (Retorna JSON com dados)
        []() -> String
        {
            return jsonBuffer;
        },
        // Callback para POST /api/command (Recebe JSON já validado e parseado)
        [](JsonVariant &doc)
        {
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

    // 3. Inicializa Web Server e Wi-Fi (usa AppConfig para rede)
    webServer.begin();

    // 4. Inicializa Inversor com baud rate do AppConfig
    inversor.begin(PIN_RS485_RX, PIN_RS485_TX);

    // 5. Inicializa Sensores e Interrupções
    rpmSensors.begin();
}

void loop()
{
    // Usa taxas de atualização dinâmicas do AppConfig (carregado do NVS)
    const AppConfig &cfg = appConfigMgr.config;

    // Atualiza o cálculo de RPM
    rpmSensors.update();
    inversor.update();

    // Atualiza o JSON para a API em frequência configurável
    if (millis() - lastJsonUpdateTime > cfg.jsonUpdateRate)
    {
        lastJsonUpdateTime = millis();

        StaticJsonDocument<256> doc;
        rpmSensors.getRpmsJson(doc);
        inversor.getInverterDataJson(doc);
        serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));
    }
}