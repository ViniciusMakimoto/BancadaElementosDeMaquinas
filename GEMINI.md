# Contexto do Projeto: Bancada Didática de Elementos de Máquina

## 🤖 Perfil do Sistema
- **Hardware:** ESP32 (Dual Core)
- **Framework:** Arduino (via PlatformIO no VS Code)
- **Comunicação:** WebSockets (tempo real)
- **Sistema de Arquivos:** LittleFS (HTML, CSS e JS separados)

## 🛠️ Requisitos Técnicos
1. **Monitoramento:** - 4 Sensores Indutivos (RPM) - via interrupções.
   - 1 Acelerômetro MPU-6050 (Vibração) - via I2C.
2. **Atuador:**
   - 1 Inversor WEG controlado via Modbus RS485 (Serial2 do ESP32).
3. **Interface Web:**
   - Tema: Dark Mode Industrial.
   - Bibliotecas: Chart.js (Gráfico de vibração).
   - Segurança: Acesso Operador protegido por PIN (1234).

## 📂 Estrutura de Arquivos
- `/src/main.cpp`: Lógica C++ (Servidor Web, WebSocket e Sensores).
- `/data/index.html`: Estrutura da UI e Modal de Login.
- `/data/style.css`: Estilização Industrial.
- `/data/script.js`: Lógica de WebSocket e atualização do Chart.js.
- `platformio.ini`: Configurações de build e dependências.

## 📍 Status Atual
- Ambiente migrado da Arduino IDE para VS Code + PlatformIO.
- Bibliotecas configuradas: `ESPAsyncWebServer`, `AsyncTCP`, `ArduinoJson`.
- `platformio.ini` configurado para `board_build.filesystem = littlefs`.
- Interface Web emulada e funcional, pronta para upload via "Upload Filesystem Image".

## 🚀 Próximos Passos
1. Realizar o primeiro upload do Sistema de Arquivos (LittleFS).
2. Implementar a leitura das interrupções para os RPMs no `main.cpp`.
3. Integrar a biblioteca ModbusMaster para controle do Inversor WEG.