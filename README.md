# ⚙️ Bancada Didática de Elementos de Máquina

Este projeto implementa o sistema de controle e monitoramento para uma bancada didática de elementos de máquina, utilizando um ESP32 como cérebro da operação. A interação com o usuário é feita através de uma interface web moderna, servida diretamente pelo microcontrolador.

## 🎯 O que é o projeto?

O objetivo principal é criar uma plataforma IoT para o estudo prático de componentes mecânicos. A bancada permite o monitoramento em tempo real da rotação de quatro eixos distintos e o controle de velocidade de um motor de indução através de um inversor de frequência industrial, tudo isso acessível via Wi-Fi por qualquer navegador web.

## ✨ Features

- **Monitoramento Web em Tempo Real:** Interface web responsiva com tema industrial (dark mode).
- **Leitura de Rotação:** Medição de RPM de 4 sensores indutivos independentes.
- **Gráficos Dinâmicos:** Visualização da rotação em gráficos utilizando Chart.js.
- **Controle do Motor:** Comandos para iniciar, parar e ajustar a velocidade do motor (via Inversor WEG).
- **API RESTful:** Endpoints para consultar dados dos sensores (`GET`) e enviar comandos de controle (`POST`).
- **Arquitetura Otimizada:** A interface web é minificada, unificada e comprimida (Gzip), sendo servida em um único arquivo para máxima eficiência no ESP32.
- **Segurança:** Acesso à interface de operação protegido por PIN.

## 🛠️ Como o projeto funciona?

### Arquitetura Geral

A arquitetura é dividida em três camadas principais:

1.  **Hardware:** O ESP32 é o núcleo que se conecta aos sensores indutivos (para ler RPM) e ao inversor de frequência WEG (para controlar o motor) via comunicação serial RS485.
2.  **Backend (ESP32):** O firmware, desenvolvido em C++ com o framework Arduino e PlatformIO, gerencia o hardware, executa um servidor web assíncrono, processa os dados e responde a requisições de API.
3.  **Frontend (Interface Web):** Uma Single Page Application (SPA) desenvolvida com HTML, CSS e JavaScript que roda no navegador do cliente. Ela busca os dados do ESP32 periodicamente e exibe as informações, além de enviar os comandos do operador.

### Tecnologias e Lógicas

-   **Backend (ESP32):**
    -   **PlatformIO:** Utilizado como ambiente de desenvolvimento para gerenciar dependências e automação de build.
    -   **ESPAsyncWebServer:** Biblioteca para criar um servidor web leve e assíncrono, ideal para o ESP32.
    -   **ArduinoJson:** Para manipulação eficiente de objetos JSON nas requisições e respostas da API.
    -   **LittleFS:** Sistema de arquivos para armazenar a interface web comprimida.
    -   **Interrupções de Hardware:** Usadas para capturar os pulsos dos sensores de RPM de forma precisa, sem bloquear o loop principal.

-   **Frontend & Build:**
    -   **Vanilla JS/HTML/CSS:** A interface é construída com tecnologias web padrão para manter a leveza.
    -   **Chart.js:** Biblioteca para a criação dos gráficos de RPM.
    -   **Automação (Python):** Um script de pré-build (`scripts/build.py`) é executado automaticamente pelo PlatformIO para:
        1.  Ler os arquivos da pasta `/web`.
        2.  Minificar o CSS e o JavaScript.
        3.  Injetar o CSS, JS e o ícone diretamente no `index.html`.
        4.  Comprimir o HTML final com Gzip, gerando o arquivo `data/index.html.gz`.

## 🚀 Passo a passo de Instalação e Uso

### Pré-requisitos

1.  **Visual Studio Code** com a extensão **PlatformIO IDE** instalada.
2.  **Python 3.x** instalado e adicionado ao PATH do sistema.
3.  **Git** para clonar o repositório.

### Configuração do Ambiente

1.  **Clone o repositório:**
    ```bash
    git clone <URL_DO_REPOSITORIO>
    cd BancadaElementosDeMaquina
    ```

2.  **Crie o ambiente virtual e instale as dependências:**
    Para manter as dependências organizadas, crie e ative um ambiente virtual.

    ```bash
    # Cria o ambiente na pasta .venv
    python -m venv .venv
    ```

    **Ative o ambiente virtual:**
    - No **Windows** (usando PowerShell):
      ```powershell
      .venv\Scripts\Activate.ps1
      ```
    - No **macOS ou Linux**:
      ```bash
      source .venv/bin/activate
      ```
    
    Com o ambiente ativado, instale os pacotes necessários:
    ```bash
    pip install -r scripts/requirements.txt
    ```

3.  **Abra o projeto no VS Code:**
    - Abra o VS Code e, no menu do PlatformIO, clique em "Open Project" e selecione a pasta do projeto.
    - O PlatformIO irá instalar automaticamente as bibliotecas C++ (`ESPAsyncWebServer`, `ArduinoJson`, etc.) listadas em `platformio.ini`.

### Compilando e Enviando

1.  **Sistema de Arquivos (Interface Web):**
    - O processo de build da interface é automático. Ao compilar o projeto, o script de Python já será executado.
    - Para enviar a interface web para o ESP32, execute a tarefa **"Upload Filesystem Image"** no PlatformIO.

2.  **Firmware (Lógica Principal):**
    - Para compilar e enviar o código principal para o ESP32, execute a tarefa **"Upload"** no PlatformIO.

> **Nota:** Sempre que houver uma alteração na interface (na pasta `/web`), é necessário executar o "Upload Filesystem Image" novamente. Se a alteração for apenas no código C++ (pasta `/src`), basta usar a tarefa "Upload".

## 📂 Estrutura de Arquivos

-   `src/`: Contém todo o código fonte C++ que roda no ESP32.
-   `web/`: Contém os arquivos de desenvolvimento da interface web (HTML, CSS, JS). **É aqui que você deve editar a UI.**
-   `data/`: Contém os arquivos que serão enviados para o sistema de arquivos do ESP32. O `index.html.gz` é gerado automaticamente aqui.
-   `scripts/`: Scripts de automação em Python.
-   `include/`: Arquivos de cabeçalho (`.h`) do C++.
-   `platformio.ini`: Arquivo de configuração principal do PlatformIO, onde as dependências e configurações de build são definidas.

## 📈 Próximos Passos

A evolução deste projeto se concentra na implementação completa do controle do motor.

1.  **Integrar Inversor WEG (Modbus):**
    - Adicionar a biblioteca `ModbusMaster` às dependências do projeto.
    - Implementar a lógica de comunicação Modbus RS485 em `WegInverter.cpp`.
    - Ativar o controle do inversor no `main.cpp`.
2.  **Lógica de Controle:**
    - Implementar o *parsing* dos comandos da API para atuar no inversor (ex: `setFrequency`, `start`, `stop`).
3.  **Upload e Teste Físico:**
    - Realizar o upload e validar o sistema completo em hardware real.

## 📄 Licença

Este projeto está sob a licença MIT. Veja o arquivo `LICENSE` para mais detalhes.
