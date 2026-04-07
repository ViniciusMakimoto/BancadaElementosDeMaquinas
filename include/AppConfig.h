#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "Config.h"

// ============================================================
//  AppConfig — Struct com todas as configurações de runtime
//  Os valores aqui são carregados do NVS pelo ConfigManager.
// ============================================================
struct AppConfig
{
    // --- Segurança ---
    char operatorPin[8];
    char adminPin[8];

    // --- Sensores ---
    uint16_t pulsesPerRevolution;  // Pulsos por rotação
    uint16_t sensorUpdateRate;     // ms entre cálculos de RPM
    uint16_t sensorDebounceBase;   // ms de debounce (base, antes de /PPR)
    bool     sensorSimEnabled;

    // --- API / JSON ---
    uint16_t jsonUpdateRate;       // ms entre atualizações do buffer JSON

    // --- Inversor ---
    uint16_t inverterUpdateRate;   // ms entre polls Modbus
    bool     inverterSimEnabled;
    uint32_t modbusBaudRate;
    uint8_t  modbusSlaveId;

    // --- Rede ---
    bool  useAP;
    char  ssid[64];
    char  wifiPass[128];
    char  localIp[24];
    char  gateway[24];
    char  subnet[24];
};

// ============================================================
//  ConfigManager — Carrega e salva AppConfig via Preferences
// ============================================================
class ConfigManager
{
public:
    AppConfig config;

    // Carrega do NVS. Se não houver dados, aplica defaults de Config.h.
    void load();

    // Persiste o estado atual de `config` no NVS.
    void save();

    // Apaga o namespace do NVS e recarrega com defaults.
    void reset();

    // Serializa `config` como JSON no documento fornecido.
    // Por segurança, os PINs NÃO são incluídos por padrão.
    void toJson(JsonDocument &doc, bool includePins = false) const;

    // Preenche `config` a partir de um JSON (apenas os campos presentes).
    // Retorna true se algum campo de REDE foi alterado (exige restart).
    bool fromJson(JsonVariant &json);

private:
    static constexpr const char *NVS_NAMESPACE = "bancada";
};

// Instância global acessível por todos os módulos
extern ConfigManager appConfigMgr;
