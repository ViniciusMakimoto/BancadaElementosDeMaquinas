#include "WebServer.h"
#include <ArduinoJson.h>
#include <AsyncJson.h>

WebServer::WebServer() : server(80) {}

// ============================================================
//  Helpers de validação de PIN
// ============================================================

bool WebServer::validateAdminPin(const char *receivedPin)
{
    return (receivedPin != nullptr) &&
           (strcmp(receivedPin, appConfigMgr.config.adminPin) == 0);
}

bool WebServer::validateOperatorPin(const char *receivedPin)
{
    // Operador OU admin têm acesso a comandos
    return (receivedPin != nullptr) &&
           (strcmp(receivedPin, appConfigMgr.config.operatorPin) == 0 ||
            strcmp(receivedPin, appConfigMgr.config.adminPin) == 0);
}

// ============================================================
//  begin() — Inicializa Wi-Fi e servidor HTTP
// ============================================================
void WebServer::begin()
{
#ifdef WOKWI_EMU
    // Em simulação, conecta ao WiFi do Wokwi como Estação
    WiFi.mode(WIFI_STA);
    WiFi.begin("Wokwi-GUEST", "");
    DEBUG_PRINT("[WIFI] Conectando à rede Wokwi-GUEST");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        DEBUG_PRINT(".");
    }
    DEBUG_PRINTLN(" Conectado!");
    DEBUG_PRINT("[WEB] IP: http://");
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINTLN("[WEB] No seu PC, acesse: http://localhost:8180");
#else
    // Lê configurações de rede do AppConfig (runtime)
    const AppConfig &cfg = appConfigMgr.config;

    // Converte strings de IP armazenadas no AppConfig
    IPAddress localIp, gw, sn;
    localIp.fromString(cfg.localIp);
    gw.fromString(cfg.gateway);
    sn.fromString(cfg.subnet);

    if (cfg.useAP)
    {
        // Modo Access Point
        WiFi.softAP(cfg.ssid, cfg.wifiPass);
        WiFi.softAPConfig(localIp, gw, sn);
        DEBUG_PRINT("[WIFI] Modo Access Point. SSID: ");
        DEBUG_PRINTLN(cfg.ssid);
        DEBUG_PRINT("[WEB] IP: http://");
        DEBUG_PRINTLN(WiFi.softAPIP());
    }
    else
    {
        // Modo Station — conecta a uma rede existente
        WiFi.config(localIp, gw, sn);
        WiFi.mode(WIFI_STA);
        WiFi.begin(cfg.ssid, cfg.wifiPass);
        DEBUG_PRINT("[WIFI] Conectando à rede ");
        DEBUG_PRINT(cfg.ssid);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            DEBUG_PRINT(".");
        }
        DEBUG_PRINTLN(" Conectado!");
        DEBUG_PRINT("[WEB] IP: http://");
        DEBUG_PRINTLN(WiFi.localIP());
    }
#endif

    // Rotas estáticas (HTML/CSS/JS)
#ifdef WOKWI_EMU
#include "embedded_html.h"
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html_gz, index_html_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response); });

    server.on("/lib/chart.umd.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript", chart_umd_min_js_gz, chart_umd_min_js_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response); });
#else
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
#endif

    server.begin();
    DEBUG_PRINTLN("[WEB] Servidor HTTP iniciado");
}

// ============================================================
//  configureApi() — Registra todas as rotas da API
// ============================================================
void WebServer::configureApi(
    std::function<String()>             getDataCallback,
    std::function<void(JsonVariant &)>  postCommandCallback)
{
    // ----------------------------------------------------------
    // GET /api/data — Dados dos sensores (público)
    // ----------------------------------------------------------
    server.on("/api/data", HTTP_GET, [getDataCallback](AsyncWebServerRequest *request)
              {
        String json = getDataCallback();
        request->send(200, "application/json", json); });

    // ----------------------------------------------------------
    // POST /api/auth — Login unificado (retorna nível de acesso)
    // ----------------------------------------------------------
    AsyncCallbackJsonWebHandler *authHandler = new AsyncCallbackJsonWebHandler(
        "/api/auth",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
            // Rate Limiting
            if (millis() - this->lastAuthAttempt < this->authAttemptCooldown)
            {
                request->send(429, "application/json", "{\"error\":\"Tente novamente mais tarde\"}");
                return;
            }
            this->lastAuthAttempt = millis();

            JsonObject obj = json.as<JsonObject>();
            const char *pin = obj.containsKey("pin") ? obj["pin"].as<const char *>() : "";

            DEBUG_PRINTLN("[AUTH] Tentativa de login recebida.");

            // Testa PIN de Admin primeiro (nível mais alto)
            if (strcmp(pin, appConfigMgr.config.adminPin) == 0)
            {
                DEBUG_PRINTLN("[AUTH] PIN Admin correto → role=admin");
                request->send(200, "application/json", "{\"status\":\"ok\",\"role\":\"admin\"}");
                return;
            }
            // Testa PIN de Operador
            if (strcmp(pin, appConfigMgr.config.operatorPin) == 0)
            {
                DEBUG_PRINTLN("[AUTH] PIN Operador correto → role=operator");
                request->send(200, "application/json", "{\"status\":\"ok\",\"role\":\"operator\"}");
                return;
            }

            DEBUG_PRINTLN("[AUTH] PIN inválido.");
            request->send(401, "application/json", "{\"error\":\"PIN inválido\"}");
        });
    server.addHandler(authHandler);

    // ----------------------------------------------------------
    // POST /api/command — Comandos do inversor (operador ou admin)
    // ----------------------------------------------------------
    AsyncCallbackJsonWebHandler *commandHandler = new AsyncCallbackJsonWebHandler(
        "/api/command",
        [this, postCommandCallback](AsyncWebServerRequest *request, JsonVariant &json)
        {
            JsonObject obj = json.as<JsonObject>();
            const char *pin = obj.containsKey("pin") ? obj["pin"].as<const char *>() : "";

            DEBUG_PRINTLN("[CMD] Comando recebido.");

            if (!this->validateOperatorPin(pin))
            {
                DEBUG_PRINTLN("[CMD] PIN inválido ou insuficiente.");
                request->send(401, "application/json", "{\"error\":\"PIN inválido ou ausente\"}");
                return;
            }

            DEBUG_PRINTLN("[CMD] PIN válido — executando.");
            postCommandCallback(json);
            request->send(200, "application/json", "{\"status\":\"Comando recebido\"}");
        });
    server.addHandler(commandHandler);

    // ----------------------------------------------------------
    // GET /api/config — Lê configurações (somente admin)
    // ----------------------------------------------------------
    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        // Valida PIN admin via query param: /api/config?admin_pin=XXXX
        if (!request->hasParam("admin_pin"))
        {
            request->send(401, "application/json", "{\"error\":\"PIN admin obrigatório\"}");
            return;
        }
        const char *pin = request->getParam("admin_pin")->value().c_str();
        if (!this->validateAdminPin(pin))
        {
            DEBUG_PRINTLN("[CFG] GET /api/config — PIN admin inválido.");
            request->send(403, "application/json", "{\"error\":\"Acesso negado\"}");
            return;
        }

        // Serializa config (inclui PINs pois o admin está autenticado)
        StaticJsonDocument<768> doc;
        appConfigMgr.toJson(doc, /*includePins=*/true);

        String out;
        serializeJson(doc, out);
        DEBUG_PRINTLN("[CFG] GET /api/config — enviando configurações.");
        request->send(200, "application/json", out); });

    // ----------------------------------------------------------
    // POST /api/config — Salva configurações (somente admin)
    // ----------------------------------------------------------
    AsyncCallbackJsonWebHandler *configHandler = new AsyncCallbackJsonWebHandler(
        "/api/config",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
            JsonObject obj = json.as<JsonObject>();
            const char *pin = obj.containsKey("admin_pin") ? obj["admin_pin"].as<const char *>() : "";

            DEBUG_PRINTLN("[CFG] POST /api/config recebido.");

            if (!this->validateAdminPin(pin))
            {
                DEBUG_PRINTLN("[CFG] POST /api/config — PIN admin inválido.");
                request->send(403, "application/json", "{\"error\":\"Acesso negado\"}");
                return;
            }

            // Aplica os campos presentes no "config" do body
            JsonVariant configObj = obj["config"];
            if (configObj.isNull())
            {
                request->send(400, "application/json", "{\"error\":\"Campo 'config' ausente\"}");
                return;
            }

            bool networkChanged = appConfigMgr.fromJson(configObj);
            appConfigMgr.save();

            DEBUG_PRINTF("[CFG] Configurações salvas. Rede alterada: %s\n", networkChanged ? "SIM" : "NÃO");

            if (networkChanged)
            {
                request->send(200, "application/json", "{\"status\":\"ok\",\"restart_required\":true}");
            }
            else
            {
                request->send(200, "application/json", "{\"status\":\"ok\",\"restart_required\":false}");
            }
        });
    server.addHandler(configHandler);

    // ----------------------------------------------------------
    // POST /api/restart — Reinicia o ESP32 (somente admin)
    // ----------------------------------------------------------
    AsyncCallbackJsonWebHandler *restartHandler = new AsyncCallbackJsonWebHandler(
        "/api/restart",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
            JsonObject obj = json.as<JsonObject>();
            const char *pin = obj.containsKey("admin_pin") ? obj["admin_pin"].as<const char *>() : "";

            if (!this->validateAdminPin(pin))
            {
                request->send(403, "application/json", "{\"error\":\"Acesso negado\"}");
                return;
            }

            DEBUG_PRINTLN("[SYS] Reinicialização solicitada pelo admin via web.");
            request->send(200, "application/json", "{\"status\":\"Reiniciando...\"}");

            // Pequeno delay para garantir que a resposta seja enviada antes do restart
            delay(500);
            ESP.restart();
        });
    server.addHandler(restartHandler);
}