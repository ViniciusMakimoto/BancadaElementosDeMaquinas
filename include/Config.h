#pragma once
#include <Arduino.h>
#include <IPAddress.h>

// --- Settings ---
static const uint16_t SENSOR_UPDATE_RATE = 1000;
static const uint16_t SENSOR_DEBOUNCE = 5;
#define PULSES_PER_REVOLUTION 2

static const uint16_t JSON_UPDATE_RATE = 500;

static const uint16_t INVERTER_UPDATE_RATE = 1000;

// --- Modo de Simulação ---
#define SENSOR_SIMULATION_ENABLED false
#define WEG_INVERTER_SIMULATION_ENABLED false

// --- Wi-Fi & Rede ---
static const char *SSID_NAME = "Bancada_TCC_Vinicius";
static const char *WIFI_PASS = "12345678";

// IP Fixo
static const IPAddress LOCAL_IP(192, 168, 4, 1);
static const IPAddress GATEWAY(192, 168, 4, 1);
static const IPAddress SUBNET(255, 255, 255, 0);

// --- Pinos do Hardware ---

// Sensores Indutivos
#define PIN_RPM_1 18
#define PIN_RPM_2 19
#define PIN_RPM_3 21
#define PIN_RPM_4 22

// Modbus RS485
#define PIN_RS485_RX 16
#define PIN_RS485_TX 17
#define PIN_RS485_DE 4

// --- Configurações do Modbus ---
#define MODBUS_SLAVE_ID 1
#define MODBUS_BAUDRATE 9600

// TODO: Validar endereços
//  Registradores do Inversor (Exemplos, ajuste conforme manual)
//  Leitura
static const uint16_t REG_READ_FREQUENCY = 2;
// #define REG_READ_CURRENT 0x0002
// #define REG_READ_MOTOR_STATUS 0x0003
// Escrita
static const uint16_t REG_WRITE_FREQUENCY = 684;
static const uint16_t REG_WRITE_COMMAND = 683;
