#pragma once
#include <functional>
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FileSystem.h"

// Se estiver no Wokwi, precisamos incluir o header gerado (simulado)
#ifdef WOKWI_EMU
#include "embedded_html.h"
#endif

class WebServer
{
private:
    AsyncWebServer server;
    long lastAuthAttempt = 0;
    const long authAttemptCooldown = 2000; // 2 segundos

public:
    WebServer();
    void begin();
    // Registra callbacks para fornecer dados (GET) e receber comandos (POST)
    void configureApi(std::function<String()> getDataCallback, std::function<void(JsonVariant &)> postCommandCallback);
};