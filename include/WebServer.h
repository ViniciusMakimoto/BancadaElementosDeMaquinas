#pragma once
#include <functional>
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "FileSystem.h"
#include "AppConfig.h"

// Se estiver no Wokwi, precisamos incluir o header gerado (simulado)
#ifdef WOKWI_EMU
#include "embedded_html.h"
#endif

class WebServer
{
private:
    AsyncWebServer server;

    // Rate limiting para tentativas de autenticação
    long lastAuthAttempt = 0;
    const long authAttemptCooldown = 2000; // ms

    // Helper: valida PIN de admin vindo de um JsonObject
    bool validateAdminPin(const char *receivedPin);

    // Helper: valida PIN de operador OU admin (ambos têm acesso a comandos)
    bool validateOperatorPin(const char *receivedPin);

public:
    WebServer();
    void begin();

    // Registra callbacks para fornecer dados (GET) e receber comandos (POST)
    void configureApi(
        std::function<String()>             getDataCallback,
        std::function<void(JsonVariant &)>  postCommandCallback
    );
};