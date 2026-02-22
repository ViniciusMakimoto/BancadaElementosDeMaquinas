#include "WebServer.h"

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
    // Em hardware real, cria um Ponto de Acesso (AP) com IP fixo
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(LOCAL_IP, GATEWAY, SUBNET);
    WiFi.softAP(SSID_NAME, WIFI_PASS);
    Serial.print("[WEB] AP Iniciado. IP: ");
    Serial.println(WiFi.softAPIP());
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

void WebServer::configureApi(std::function<String()> getDataCallback, std::function<void(String)> postCommandCallback)
{
    // Rota GET: Front-end pede dados dos sensores
    server.on("/api/data", HTTP_GET, [getDataCallback](AsyncWebServerRequest *request)
              {
        String json = getDataCallback();
        request->send(200, "application/json", json); });

    // Rota POST: Front-end envia comandos
    server.on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request)
              {
                  request->send(200, "text/plain", "OK"); // Responde rápido
              },
              NULL, [postCommandCallback](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
                  String body = "";
                  for (size_t i = 0; i < len; i++)
                      body += (char)data[i];
                  postCommandCallback(body); // Processa o comando
              });
}