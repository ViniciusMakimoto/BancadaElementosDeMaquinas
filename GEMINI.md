# Contexto do Projeto: Bancada Didática de Elementos de Máquina

## 🤖 Perfil do Sistema

- **Hardware:** ESP32 (Dual Core)
- **Framework:** Arduino (via PlatformIO no VS Code)
- **Comunicação:** POSTG e GET (otimização para multiplos dispositivos)
- **Sistema de Arquivos:** LittleFS (HTML, CSS e JS juntos, minificados e compactados)
- **Simulação de Hardware:** Wokwi com separação de env e lógica customizada de littleFS

## 🛠️ Requisitos Técnicos

1. **Monitoramento:** - 4 Sensores Indutivos (RPM) - via interrupções.
2. **Atuador:**
   - 1 Inversor WEG controlado via Modbus RS485 (Serial2 do ESP32).
3. **Interface Web:**
   - Tema: Dark Mode Industrial.
   - Bibliotecas: Chart.js (Gráfico de vibração).
   - Segurança: Acesso Operador protegido por PIN (1234).

## 📂 Estrutura de Arquivos

- `/src/main.cpp`: Lógica C++ (Servidor Web, POST e GET e Sensores).
- `/data/index.html`: Estrutura da UI e Modal de Login.
- `/data/style.css`: Estilização Industrial.
- `/data/script.js`: Lógica de POST e GET e atualização do Chart.js.
- `platformio.ini`: Configurações de build e dependências.
- `/scripts`: Scripts em python para automação de build.

## 📍 Status Atual

- Ambiente migrado da Arduino IDE para VS Code + PlatformIO.
- Bibliotecas configuradas: `ESPAsyncWebServer`, `AsyncTCP`, `ArduinoJson`.
- `platformio.ini` configurado para `board_build.filesystem = littlefs`.
- Interface Web emulada e funcional, pronta para upload via "Upload Filesystem Image".
- Simulação via Wokwi funcional.

## 🚀 Próximos Passos

1. Realizar o primeiro upload do Sistema de Arquivos (LittleFS).
2. Implementar a leitura das interrupções para os RPMs no `main.cpp`.
3. Integrar a biblioteca ModbusMaster para controle do Inversor WEG.
