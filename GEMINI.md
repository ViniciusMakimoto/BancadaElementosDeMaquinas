# Contexto do Projeto: Bancada Didática de Elementos de Máquina

## 🤖 Perfil do Sistema

- **Hardware:** ESP32 (Dual Core)
- **Framework:** Arduino (via PlatformIO no VS Code)
- **Comunicação:** POST e GET (otimização para multiplos dispositivos)
- **Sistema de Arquivos:** LittleFS (HTML, CSS e JS unificados, minificados e comprimidos com Gzip)
- **Simulação de Hardware:** Wokwi com separação de env e lógica customizada de littleFS

## 🛠️ Requisitos Técnicos

1. **Monitoramento:** - 4 Sensores Indutivos (RPM) - via interrupções.
2. **Atuador:**
   - 1 Inversor WEG controlado via Modbus RS485 (Serial2 do ESP32).
3. **Interface Web:**
   - Tema: Dark Mode Industrial.
   - Bibliotecas: Chart.js (Gráficos).
   - Segurança: Acesso Operador protegido por PIN (1234).

## ⚙️ Processo de Build (Automação)

O projeto utiliza um script de pré-build (`scripts/build.py`) que automatiza a preparação da interface web antes do upload para o ESP32. O processo consiste em:

1.  **Leitura:** Lê os arquivos de desenvolvimento (`index.html`, `style.css`, `script.js`, `favicon.ico`) da pasta `/web`.
2.  **Minificação:** Reduz o tamanho do CSS e do JavaScript.
3.  **Injeção (Embedding):** O conteúdo do CSS, JS e do favicon (em formato Base64) é injetado diretamente no arquivo HTML. Isso reduz o número de requisições que o navegador precisa fazer.
4.  **Minificação do HTML:** O arquivo HTML resultante é minificado.
5.  **Compressão:** O HTML final é comprimido com **Gzip** para economizar espaço de armazenamento e acelerar o carregamento.
6.  **Gravação:** O resultado é salvo como `data/index.html.gz`.

Este processo garante que o arquivo enviado ao ESP32 seja o menor e mais eficiente possível.

## 📂 Estrutura de Arquivos

- `/src`: Lógica principal em C++ para o ESP32.
  - `main.cpp`: Ponto de entrada, inicialização de hardware e servidor.
  - `WebServer.cpp`: Configuração e rotas do servidor web assíncrono.
  - `RpmSensors.cpp`: Lógica para leitura dos sensores de RPM.
  - `WegInverter.cpp`: Controle do inversor via Modbus.
  - `FileSystem.cpp`: Gerenciamento do sistema de arquivos LittleFS.
- `/web`: Arquivos de desenvolvimento da interface do usuário (HTML, CSS, JS). **(Não vai para o ESP32)**
- `/data`: Arquivos que serão gravados no sistema de arquivos LittleFS do ESP32.
  - `index.html.gz`: Interface web unificada, minificada e comprimida.
  - `/lib`: Bibliotecas Javascript, como o `chart.umd.min.js.gz`.
- `/scripts`: Scripts em Python para automação de tarefas.
- `platformio.ini`: Configurações do projeto, bibliotecas e ambiente PlatformIO.

## 📍 Status Atual

- **Ambiente de Desenvolvimento:**
  - Projeto configurado para VS Code + PlatformIO.
  - Simulação no Wokwi está funcional (`wokwi.toml`).
- **Backend (ESP32):**
  - Servidor Web Assíncrono (`ESPAsyncWebServer`) implementado e servindo a interface.
  - API RESTful com endpoints `GET /api/data` e `POST /api/command` funcionais.
  - Leitura dos 4 sensores de RPM **implementada**, com cálculo de RPM via interrupções e modo de simulação.
- **Frontend (Interface Web):**
  - Processo de build automatizado com Python (`scripts/build.py`) que unifica, minifica e comprime a UI em `data/index.html.gz`.
  - Interface pronta para ser enviada ao ESP32 via "Upload Filesystem Image".
- **Sistema de Arquivos:**
  - `platformio.ini` configurado para usar `littlefs`.

## 🚀 Próximos Passos

1.  **Integrar Inversor WEG (Modbus):**
    - Adicionar a biblioteca `ModbusMaster` às dependências do projeto no `platformio.ini`.
    - Implementar a lógica de comunicação Modbus RS485 em `WegInverter.cpp`, substituindo a simulação atual.
    - Descomentar e integrar o objeto `inversor` no `main.cpp` para controlar o motor via API.
2.  **Lógica de Controle:**
    - Implementar o *parsing* dos comandos recebidos via `POST /api/command` para atuar no inversor (ex: `setFrequency`, `start`, `stop`).
3.  **Upload e Teste Físico:**
    - Realizar o upload do sistema de arquivos (`data`) e do firmware para o hardware físico.
    - Validar o funcionamento dos sensores de RPM e do controle do inversor.
4.  **Segurança:**
    - Implementar a verificação de PIN pelo ESP32
    - Validação das requisições de POST e GET para evitar usuários mal intencionados.
