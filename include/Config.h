#pragma once
#include <Arduino.h>
#include <IPAddress.h>

// --- Settings ---
// Estes valores são os PADRÕES DE FÁBRICA usados pelo ConfigManager
// quando o NVS está vazio ou foi resetado.
#define DEFAULT_PULSES_PER_REVOLUTION 1
#define DEFAULT_SENSOR_UPDATE_RATE    1000  // ms
#define DEFAULT_SENSOR_DEBOUNCE_BASE  30    // ms (debounce base, antes de dividir por PPR)
#define DEFAULT_JSON_UPDATE_RATE      500   // ms
#define DEFAULT_INVERTER_UPDATE_RATE  1000  // ms

// Aliases para compatibilidade com código existente (serão substituídos
// progressivamente pelos valores dinâmicos do AppConfig)
static const uint16_t SENSOR_UPDATE_RATE   = DEFAULT_SENSOR_UPDATE_RATE;
static const uint16_t SENSOR_DEBOUNCE      = DEFAULT_SENSOR_DEBOUNCE_BASE;
static const uint16_t JSON_UPDATE_RATE     = DEFAULT_JSON_UPDATE_RATE;
static const uint16_t INVERTER_UPDATE_RATE = DEFAULT_INVERTER_UPDATE_RATE;
#define PULSES_PER_REVOLUTION DEFAULT_PULSES_PER_REVOLUTION

// --- Segurança ---
#define OPERATOR_PIN "0000"
#define ADMIN_PIN    "9999"

// --- Debug ---
#define DEBUG_MODE true

#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

// --- Modo de Simulação ---
#define DEFAULT_SENSOR_SIMULATION_ENABLED  false
#define DEFAULT_INVERTER_SIMULATION_ENABLED false
#define SENSOR_SIMULATION_ENABLED  DEFAULT_SENSOR_SIMULATION_ENABLED
#define WEG_INVERTER_SIMULATION_ENABLED DEFAULT_INVERTER_SIMULATION_ENABLED

// --- Wi-Fi & Rede ---
#define DEFAULT_USE_AP     true
#define DEFAULT_SSID_NAME  "Bancada_EM"
#define DEFAULT_WIFI_PASS  "12345678"
#define DEFAULT_LOCAL_IP   "192.168.4.1"
#define DEFAULT_GATEWAY    "192.168.4.1"
#define DEFAULT_SUBNET     "255.255.255.0"

// Descomente a linha abaixo para usar o modo Access Point (AP) como padrão de compilação
#define CREATE_WIFI_AP

static const char *SSID_NAME = DEFAULT_SSID_NAME;
static const char *WIFI_PASS = DEFAULT_WIFI_PASS;

// IP Fixo (padrão de compilação — runtime usa AppConfig)
static const IPAddress LOCAL_IP(192, 168, 4, 1);
static const IPAddress GATEWAY(192, 168, 4, 1);
static const IPAddress SUBNET(255, 255, 255, 0);

// --- Pinos do Hardware ---

// Sensores Indutivos
#define PIN_RPM_1 18
#define PIN_RPM_2 19
#define PIN_RPM_3 22
#define PIN_RPM_4 21

// Modbus RS485
#define PIN_RS485_RX 16
#define PIN_RS485_TX 17

// --- Configurações do Modbus ---
#define MODBUS_SLAVE_ID 1
#define MODBUS_BAUDRATE 9600

//  Registradores do Inversor
//  Leitura
static const uint16_t REG_READ_FREQUENCY = 2;
// #define REG_READ_CURRENT 0x0002
// #define REG_READ_MOTOR_STATUS 0x0003
// Escrita
static const uint16_t REG_WRITE_FREQUENCY = 683;
static const uint16_t REG_WRITE_COMMAND = 682;
