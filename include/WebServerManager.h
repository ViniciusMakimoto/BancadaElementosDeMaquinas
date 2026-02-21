#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"
#include "FileSystemWrapper.h"
#include <functional>

// Se estiver no Wokwi, precisamos incluir o header gerado (simulado)
#ifdef WOKWI_EMU
#include "embedded_html.h"
#endif

class WebServerManager
{
private:
    AsyncWebServer server;

public:
    WebServerManager();
    void begin();
    // Registra callbacks para fornecer dados (GET) e receber comandos (POST)
    void configureApi(std::function<String()> getDataCallback, std::function<void(String)> postCommandCallback);
};