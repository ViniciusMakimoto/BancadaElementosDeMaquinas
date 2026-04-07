#include "AppConfig.h"

// Instância global
ConfigManager appConfigMgr;

// ============================================================
//  Helpers internos
// ============================================================
static void parseIp(const char *str, IPAddress &out)
{
    out.fromString(str);
}

// ============================================================
//  load() — NVS → AppConfig (fallback para defaults de Config.h)
// ============================================================
void ConfigManager::load()
{
    Preferences prefs;
    bool ok = prefs.begin(NVS_NAMESPACE, /*readOnly=*/true);

    if (!ok)
    {
        DEBUG_PRINTLN("[CFG] NVS namespace não encontrado — aplicando defaults de fábrica.");
    }

    // --- Segurança ---
    String opPin  = ok ? prefs.getString("op_pin",  OPERATOR_PIN) : OPERATOR_PIN;
    String admPin = ok ? prefs.getString("adm_pin", ADMIN_PIN)    : ADMIN_PIN;
    strlcpy(config.operatorPin, opPin.c_str(),  sizeof(config.operatorPin));
    strlcpy(config.adminPin,    admPin.c_str(), sizeof(config.adminPin));

    // --- Sensores ---
    config.pulsesPerRevolution = ok ? prefs.getUShort("ppr",          DEFAULT_PULSES_PER_REVOLUTION)  : DEFAULT_PULSES_PER_REVOLUTION;
    config.sensorUpdateRate    = ok ? prefs.getUShort("sens_rate",    DEFAULT_SENSOR_UPDATE_RATE)     : DEFAULT_SENSOR_UPDATE_RATE;
    config.sensorDebounceBase  = ok ? prefs.getUShort("sens_deb",     DEFAULT_SENSOR_DEBOUNCE_BASE)   : DEFAULT_SENSOR_DEBOUNCE_BASE;
    config.sensorSimEnabled    = ok ? prefs.getBool("sim_sens",        DEFAULT_SENSOR_SIMULATION_ENABLED) : DEFAULT_SENSOR_SIMULATION_ENABLED;

    // --- API / JSON ---
    config.jsonUpdateRate      = ok ? prefs.getUShort("json_rate",    DEFAULT_JSON_UPDATE_RATE)       : DEFAULT_JSON_UPDATE_RATE;

    // --- Inversor ---
    config.inverterUpdateRate  = ok ? prefs.getUShort("inv_rate",     DEFAULT_INVERTER_UPDATE_RATE)   : DEFAULT_INVERTER_UPDATE_RATE;
    config.inverterSimEnabled  = ok ? prefs.getBool("sim_inv",         DEFAULT_INVERTER_SIMULATION_ENABLED) : DEFAULT_INVERTER_SIMULATION_ENABLED;
    config.modbusBaudRate      = ok ? prefs.getULong("mb_baud",       MODBUS_BAUDRATE)                : MODBUS_BAUDRATE;
    config.modbusSlaveId       = ok ? (uint8_t)prefs.getUChar("mb_slave", MODBUS_SLAVE_ID)            : MODBUS_SLAVE_ID;

    // --- Rede ---
    config.useAP = ok ? prefs.getBool("use_ap", DEFAULT_USE_AP) : DEFAULT_USE_AP;

    String ssid     = ok ? prefs.getString("ssid",      DEFAULT_SSID_NAME) : DEFAULT_SSID_NAME;
    String pass     = ok ? prefs.getString("wifi_pass", DEFAULT_WIFI_PASS) : DEFAULT_WIFI_PASS;
    String localIp  = ok ? prefs.getString("local_ip",  DEFAULT_LOCAL_IP)  : DEFAULT_LOCAL_IP;
    String gw       = ok ? prefs.getString("gateway",   DEFAULT_GATEWAY)   : DEFAULT_GATEWAY;
    String sn       = ok ? prefs.getString("subnet",    DEFAULT_SUBNET)    : DEFAULT_SUBNET;

    strlcpy(config.ssid,     ssid.c_str(),    sizeof(config.ssid));
    strlcpy(config.wifiPass, pass.c_str(),    sizeof(config.wifiPass));
    strlcpy(config.localIp,  localIp.c_str(), sizeof(config.localIp));
    strlcpy(config.gateway,  gw.c_str(),      sizeof(config.gateway));
    strlcpy(config.subnet,   sn.c_str(),      sizeof(config.subnet));

    if (ok) prefs.end();

    DEBUG_PRINTLN("[CFG] Configurações carregadas:");
    DEBUG_PRINTF("  Operador PIN: %s | Admin PIN: %s\n", config.operatorPin, config.adminPin);
    DEBUG_PRINTF("  PPR: %u | SensRate: %u ms | Debounce: %u ms\n",
                  config.pulsesPerRevolution, config.sensorUpdateRate, config.sensorDebounceBase);
    DEBUG_PRINTF("  JSON Rate: %u ms | Inv Rate: %u ms\n", config.jsonUpdateRate, config.inverterUpdateRate);
    DEBUG_PRINTF("  Rede: %s | SSID: %s | IP: %s\n",
                  config.useAP ? "AP" : "STA", config.ssid, config.localIp);
}

// ============================================================
//  save() — AppConfig → NVS
// ============================================================
void ConfigManager::save()
{
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, /*readOnly=*/false))
    {
        DEBUG_PRINTLN("[CFG] ERRO: Falha ao abrir NVS para escrita!");
        return;
    }

    prefs.putString("op_pin",    config.operatorPin);
    prefs.putString("adm_pin",   config.adminPin);
    prefs.putUShort("ppr",       config.pulsesPerRevolution);
    prefs.putUShort("sens_rate", config.sensorUpdateRate);
    prefs.putUShort("sens_deb",  config.sensorDebounceBase);
    prefs.putBool("sim_sens",    config.sensorSimEnabled);
    prefs.putUShort("json_rate", config.jsonUpdateRate);
    prefs.putUShort("inv_rate",  config.inverterUpdateRate);
    prefs.putBool("sim_inv",     config.inverterSimEnabled);
    prefs.putULong("mb_baud",    config.modbusBaudRate);
    prefs.putUChar("mb_slave",   config.modbusSlaveId);
    prefs.putBool("use_ap",      config.useAP);
    prefs.putString("ssid",      config.ssid);
    prefs.putString("wifi_pass", config.wifiPass);
    prefs.putString("local_ip",  config.localIp);
    prefs.putString("gateway",   config.gateway);
    prefs.putString("subnet",    config.subnet);

    prefs.end();
    DEBUG_PRINTLN("[CFG] Configurações salvas no NVS.");
}

// ============================================================
//  reset() — Apaga NVS e recarrega defaults
// ============================================================
void ConfigManager::reset()
{
    Preferences prefs;
    if (prefs.begin(NVS_NAMESPACE, false))
    {
        prefs.clear();
        prefs.end();
    }
    load(); // Recarrega com defaults
    DEBUG_PRINTLN("[CFG] Reset para defaults de fábrica concluído.");
}

// ============================================================
//  toJson() — Serializa config para ArduinoJson
// ============================================================
void ConfigManager::toJson(JsonDocument &doc, bool includePins) const
{
    if (includePins)
    {
        doc["operatorPin"] = config.operatorPin;
        doc["adminPin"]    = config.adminPin;
    }

    // Sensores
    doc["pulsesPerRevolution"] = config.pulsesPerRevolution;
    doc["sensorUpdateRate"]    = config.sensorUpdateRate;
    doc["sensorDebounceBase"]  = config.sensorDebounceBase;
    doc["sensorSimEnabled"]    = config.sensorSimEnabled;

    // API
    doc["jsonUpdateRate"]      = config.jsonUpdateRate;

    // Inversor
    doc["inverterUpdateRate"]  = config.inverterUpdateRate;
    doc["inverterSimEnabled"]  = config.inverterSimEnabled;
    doc["modbusBaudRate"]      = config.modbusBaudRate;
    doc["modbusSlaveId"]       = config.modbusSlaveId;

    // Rede
    doc["useAP"]    = config.useAP;
    doc["ssid"]     = config.ssid;
    // wifiPass: enviado como "***" por segurança — só atualiza se o usuário editar
    doc["wifiPass"] = "***";
    doc["localIp"]  = config.localIp;
    doc["gateway"]  = config.gateway;
    doc["subnet"]   = config.subnet;
}

// ============================================================
//  fromJson() — Aplica chaves presentes no JSON ao config
//  Retorna true se algum campo de rede foi alterado (requer restart)
// ============================================================
bool ConfigManager::fromJson(JsonVariant &json)
{
    bool networkChanged = false;
    JsonObject obj = json.as<JsonObject>();

    // --- Segurança ---
    if (obj.containsKey("operatorPin"))
        strlcpy(config.operatorPin, obj["operatorPin"] | config.operatorPin, sizeof(config.operatorPin));
    if (obj.containsKey("adminPin"))
        strlcpy(config.adminPin, obj["adminPin"] | config.adminPin, sizeof(config.adminPin));

    // --- Sensores ---
    if (obj.containsKey("pulsesPerRevolution"))
        config.pulsesPerRevolution = obj["pulsesPerRevolution"] | config.pulsesPerRevolution;
    if (obj.containsKey("sensorUpdateRate"))
        config.sensorUpdateRate = obj["sensorUpdateRate"] | config.sensorUpdateRate;
    if (obj.containsKey("sensorDebounceBase"))
        config.sensorDebounceBase = obj["sensorDebounceBase"] | config.sensorDebounceBase;
    if (obj.containsKey("sensorSimEnabled"))
        config.sensorSimEnabled = obj["sensorSimEnabled"] | config.sensorSimEnabled;

    // --- API ---
    if (obj.containsKey("jsonUpdateRate"))
        config.jsonUpdateRate = obj["jsonUpdateRate"] | config.jsonUpdateRate;

    // --- Inversor ---
    if (obj.containsKey("inverterUpdateRate"))
        config.inverterUpdateRate = obj["inverterUpdateRate"] | config.inverterUpdateRate;
    if (obj.containsKey("inverterSimEnabled"))
        config.inverterSimEnabled = obj["inverterSimEnabled"] | config.inverterSimEnabled;
    if (obj.containsKey("modbusBaudRate"))
        config.modbusBaudRate = obj["modbusBaudRate"] | config.modbusBaudRate;
    if (obj.containsKey("modbusSlaveId"))
        config.modbusSlaveId = obj["modbusSlaveId"] | config.modbusSlaveId;

    // --- Rede (marca networkChanged) ---
    if (obj.containsKey("useAP"))
    {
        bool newUseAP = obj["useAP"] | config.useAP;
        if (newUseAP != config.useAP) { config.useAP = newUseAP; networkChanged = true; }
    }
    if (obj.containsKey("ssid"))
    {
        const char *newSsid = obj["ssid"] | config.ssid;
        if (strcmp(newSsid, config.ssid) != 0) { strlcpy(config.ssid, newSsid, sizeof(config.ssid)); networkChanged = true; }
    }
    // Senha: só atualiza se não for "***" (máscara)
    if (obj.containsKey("wifiPass"))
    {
        const char *newPass = obj["wifiPass"] | "";
        if (strlen(newPass) > 0 && strcmp(newPass, "***") != 0)
        {
            strlcpy(config.wifiPass, newPass, sizeof(config.wifiPass));
            networkChanged = true;
        }
    }
    if (obj.containsKey("localIp"))
    {
        const char *newIp = obj["localIp"] | config.localIp;
        if (strcmp(newIp, config.localIp) != 0) { strlcpy(config.localIp, newIp, sizeof(config.localIp)); networkChanged = true; }
    }
    if (obj.containsKey("gateway"))
    {
        const char *newGw = obj["gateway"] | config.gateway;
        if (strcmp(newGw, config.gateway) != 0) { strlcpy(config.gateway, newGw, sizeof(config.gateway)); networkChanged = true; }
    }
    if (obj.containsKey("subnet"))
    {
        const char *newSn = obj["subnet"] | config.subnet;
        if (strcmp(newSn, config.subnet) != 0) { strlcpy(config.subnet, newSn, sizeof(config.subnet)); networkChanged = true; }
    }

    return networkChanged;
}
