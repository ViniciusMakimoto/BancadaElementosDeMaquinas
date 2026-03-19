#include "WebServer.h"
#include <ArduinoJson.h>
#include <AsyncJson.h>

WebServer::WebServer() : server(80) {}

void WebServer::begin()
{
#ifdef WOKWI_EMU
    // Em simulação, conecta ao WiFi do Wokwi como Estação
    WiFi.mode(WIFI_STA);
    WiFi.begin("Wokwi-GUEST", "");
    Serial.print("[WIFI] Conectando à rede Wokwi-GUEST");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }
    Serial.println(" Conectado!");
    Serial.print("[WEB] IP: http://");
    Serial.println(WiFi.localIP());
    Serial.println("[WEB] No seu PC, acesse: http://localhost:8180");
#else
#ifdef CREATE_WIFI_AP
    // Modo Access Point (AP)
    WiFi.softAP(SSID_NAME, WIFI_PASS);
    WiFi.softAPConfig(LOCAL_IP, GATEWAY, SUBNET);
    Serial.print("[WIFI] Modo Access Point Ativado. SSID: ");
    Serial.println(SSID_NAME);
    Serial.print("[WEB] IP: http://");
    Serial.println(WiFi.softAPIP());
#else
    // Modo Station (STA) - Conecta-se a uma rede Wi-Fi existente
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID_NAME, WIFI_PASS);
    Serial.print("[WIFI] Conectando à rede ");
    Serial.print(SSID_NAME);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Conectado!");
    Serial.print("[WEB] IP: http://");
    Serial.println(WiFi.localIP());
#endif
#endif

    // 2. Configura Rotas Web
#ifdef WOKWI_EMU
#include "embedded_html.h" // Contém o site para simulação
    // Rota para Simulação (Serve bytes da memória)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        // Envia o GZIP a partir da PROGMEM
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html_gz, index_html_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response); });

    // Rota para o Chart.js na simulação
    server.on("/lib/chart.umd.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript", chart_umd_min_js_gz, chart_umd_min_js_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response); });
#else
    // Rota para Hardware Real (Serve arquivos do LittleFS)
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
#endif

    // 4. Inicia Servidor
    server.begin();
    Serial.println("[WEB] Servidor HTTP iniciado");
}

// A assinatura da callback de comando foi alterada para receber JsonVariant
void WebServer::configureApi(std::function<String()> getDataCallback, std::function<void(JsonVariant &)> postCommandCallback)
{
    // Rota GET: Front-end pede dados dos sensores
    server.on("/api/data", HTTP_GET, [getDataCallback](AsyncWebServerRequest *request)
              {
        String json = getDataCallback();
        request->send(200, "application/json", json); });

    // Rota POST: Front-end envia comandos (com validação de PIN)
    AsyncCallbackJsonWebHandler *commandHandler = new AsyncCallbackJsonWebHandler("/api/command", [this, postCommandCallback](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                  {
        // 1. Valida o PIN
        JsonObject jsonObj = json.as<JsonObject>();
        const char *receivedPin = jsonObj.containsKey("pin") ? jsonObj["pin"].as<const char *>() : "";

        Serial.println("[COMMAND] Comando recebido.");
        Serial.print("[COMMAND] PIN Recebido: ");
        Serial.println(receivedPin);
        Serial.print("[COMMAND] PIN Esperado: ");
        Serial.println(OPERATOR_PIN);

        if (strcmp(receivedPin, OPERATOR_PIN) != 0)
        {
            Serial.println("[COMMAND] Resultado: FALHA - PIN Inválido ou ausente");
            request->send(401, "application/json", "{\"error\":\"PIN inválido ou ausente\"}");
            return;
        }

        // 2. Se o PIN estiver correto, processa o comando
        Serial.println("[COMMAND] Resultado: SUCESSO - PIN Correto");
        postCommandCallback(json);

        request->send(200, "application/json", "{\"status\":\"Comando recebido\"}"); });
    server.addHandler(commandHandler);

    // Rota POST de Autenticação
    AsyncCallbackJsonWebHandler *authHandler = new AsyncCallbackJsonWebHandler("/api/auth", [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                               {
        // 1. Rate Limiting
        if (millis() - this->lastAuthAttempt < this->authAttemptCooldown)
        {
            request->send(429, "application/json", "{\"error\":\"Tente novamente mais tarde\"}");
            return;
        }
        this->lastAuthAttempt = millis();

        // 2. Valida o PIN
        JsonObject jsonObj = json.as<JsonObject>();
        const char *receivedPin = jsonObj.containsKey("pin") ? jsonObj["pin"].as<const char *>() : "";

        Serial.println("[AUTH] Tentativa de login recebida.");
        Serial.print("[AUTH] PIN Recebido: ");
        Serial.println(receivedPin);
        Serial.print("[AUTH] PIN Esperado: ");
        Serial.println(OPERATOR_PIN);

        if (strcmp(receivedPin, OPERATOR_PIN) == 0)
        {
            Serial.println("[AUTH] Resultado: SUCESSO");
            request->send(200, "application/json", "{\"status\":\"PIN correto\"}");
        }
        else
        {
            Serial.println("[AUTH] Resultado: FALHA");
            request->send(401, "application/json", "{\"error\":\"PIN inválido\"}");
        } });
    server.addHandler(authHandler);
}