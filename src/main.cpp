#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Configurações do Access Point
const char* ssid = "Bancada_TCC_Vinicius";
const char* password = "12345678";

// Objetos do Servidor
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variáveis para simulação de dados
unsigned long lastUpdateTime = 0;
const long interval = 200; // Envia dados a cada 200ms

// Função que trata mensagens recebidas via WebSocket (Comandos da Web)
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);
        if (!error) {
            if (doc.containsKey("action")) {
                String action = doc["action"];
                Serial.println("Comando Inversor: " + action);
                // Aqui você inserirá a lógica Modbus posteriormente
            }
            if (doc.containsKey("freq")) {
                int freq = doc["freq"];
                Serial.printf("Nova Frequência: %d Hz\n", freq);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);

    // 1. Inicializa Sistema de Arquivos
    if (!LittleFS.begin()) {
        Serial.println("Erro ao montar LittleFS");
        return;
    }

    // 2. Configura Wi-Fi como Access Point
    WiFi.softAP(ssid, password);
    Serial.println("Wi-Fi Iniciado: " + String(ssid));
    Serial.print("IP para o navegador: ");
    Serial.println(WiFi.softAPIP());

    // 3. Configura WebSocket
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // 4. Serve os arquivos estáticos (HTML, CSS, JS)
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // 5. Inicia o servidor
    server.begin();
}

void loop() {
    // Limpeza de conexões mortas do WebSocket
    ws.cleanupClients();

    // Envia dados simulados para a Web periodicamente
    if (millis() - lastUpdateTime > interval) {
        StaticJsonDocument<200> sensorData;
        sensorData["rpm1"] = random(1150, 1250);
        sensorData["rpm2"] = random(800, 850);
        sensorData["rpm3"] = random(500, 550);
        sensorData["rpm4"] = random(300, 350);
        sensorData["vib"] = (float)random(5, 20) / 10.0;

        String jsonResponse;
        serializeJson(sensorData, jsonResponse);
        ws.textAll(jsonResponse); // Dispara para todos os navegadores conectados

        lastUpdateTime = millis();
    }
}