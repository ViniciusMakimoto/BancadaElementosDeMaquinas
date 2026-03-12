/* ================================
   CONFIGURAÇÕES GERAIS
================================ */
const REQUEST_RATE = 1000; // ms: Taxa de atualização de dados do ESP32
const MOCK_SENSORS = false;
const MOCK_LOGIN = false;
let rpmChart;
let operatorPin = null;

window.onload = () => {
    initChart();
    if (MOCK_SENSORS) {
        mockFetchData();
    }
    else {
        fetchData(); // Inicia a busca de dados do ESP32
    }

    // Adiciona o manipulador de eventos para a navegação SPA
    document.querySelectorAll('.nav-link').forEach(link => {
        link.addEventListener('click', navigateToPage);
    });

    // Listener para ATUALIZAR O TEXTO do slider de frequência
    document.getElementById('freqSlider').addEventListener('input', (e) => {
        document.getElementById('freqVal').innerText = e.target.value;
    });

    // Listener para ENVIAR O COMANDO quando o usuário soltar o slider
    document.getElementById('freqSlider').addEventListener('change', (e) => {
        sendInverterCommand(); // Envia só a frequência
    });
};


/* ================================
   MOCK DA INTERFACE
================================ */

// --- Variáveis de estado da simulação ---
let simState = 'IDLE'; // Estado inicial é 'IDLE'
let stateTime = 0;
let baseRpm = 0;
let targetRpm = 0;
let previousRpm = 0;

const RAMP_UP_TIME = 15 * 1000;   // 5 segundos para acelerar
const STABLE_TIME = 25 * 1000;  // 15 segundos em velocidade estável
const RAMP_DOWN_TIME = 15 * 1000; // 5 segundos para desacelerar
const IDLE_TIME = 20 * 1000;      // 3 segundos em marcha lenta

const MAX_RUN_RPM = 1800; // RPM máximo em funcionamento
const MIN_RUN_RPM = 1600; // RPM mínimo em funcionamento
const IDLE_RPM = 400;     // RPM em marcha lenta

function mockFetchData() {
    const statusIndicator = document.getElementById('status-indicator');
    const statusText = document.getElementById('status-text');

    // Inicia o RPM base no valor de marcha lenta
    baseRpm = IDLE_RPM;

    setInterval(() => {
        // --- Lógica de simulação do motor (executa independentemente) ---
        stateTime += REQUEST_RATE;

        switch (simState) {
            case 'IDLE':
                baseRpm = IDLE_RPM + (Math.random() - 0.5) * 20;
                if (stateTime >= IDLE_TIME) {
                    simState = 'STARTING';
                    stateTime = 0;
                    targetRpm = Math.random() * (MAX_RUN_RPM - MIN_RUN_RPM) + MIN_RUN_RPM;
                    previousRpm = baseRpm;
                }
                break;
            case 'STARTING':
                baseRpm = previousRpm + (targetRpm - previousRpm) * (stateTime / RAMP_UP_TIME);
                if (stateTime >= RAMP_UP_TIME) {
                    simState = 'RUNNING';
                    stateTime = 0;
                    baseRpm = targetRpm;
                }
                break;
            case 'RUNNING':
                baseRpm = targetRpm + (Math.random() - 0.5) * 50;
                if (stateTime >= STABLE_TIME) {
                    simState = 'STOPPING';
                    stateTime = 0;
                    previousRpm = baseRpm;
                }
                break;
            case 'STOPPING':
                baseRpm = previousRpm + (IDLE_RPM - previousRpm) * (stateTime / RAMP_DOWN_TIME);
                if (stateTime >= RAMP_DOWN_TIME) {
                    simState = 'IDLE';
                    stateTime = 0;
                    baseRpm = IDLE_RPM;
                }
                break;
        }
        baseRpm = Math.max(0, baseRpm);

        // --- Geração de dados Mock ---
        statusIndicator.className = 'status-ok';
        statusText.textContent = 'Modo Simulado';

        const inverterFrequency = parseFloat(document.getElementById('freqSlider').value);
        let inverterStatus;

        // Lógica de Status: Falha > Parado (freq 0) > Girando
        if (Math.random() < 0.2) {
            inverterStatus = 'Falha';
            // RPMs permanecem 0
        } else if (inverterFrequency === 0) {
            inverterStatus = 'Parado';
            // RPMs permanecem 0
        } else {
            // Se a frequência for > 0 e não houver falha, o inversor está 'Girando'
            inverterStatus = 'Girando';
            // E os RPMs exibidos são baseados na simulação contínua
        }

        const mockData = {
            rpm1: Math.round(Math.max(0, baseRpm + (Math.random() - 0.5) * 15)),
            rpm2: Math.round(Math.max(0, baseRpm - 20 + (Math.random() - 0.5) * 20)),
            rpm3: Math.round(Math.max(0, baseRpm - 40 + (Math.random() - 0.5) * 25)),
            rpm4: Math.round(Math.max(0, baseRpm - 60 + (Math.random() - 0.5) * 30)),
            inverterFrequency: inverterFrequency,
            inverterStatus: inverterStatus
        };

        // Atualiza a interface com os dados simulados
        updateInterface(mockData);
    }, REQUEST_RATE);
}

/* ================================
   NAVEGAÇÃO SPA
================================ */
function navigateToPage(event) {
    event.preventDefault(); // Impede o comportamento padrão do link

    const targetId = event.currentTarget.getAttribute('href'); // #dashboard ou #controle

    // Esconde todas as páginas
    document.querySelectorAll('.page').forEach(page => {
        page.style.display = 'none';
    });

    // Mostra a página alvo
    document.querySelector(targetId).style.display = 'block';

    // Atualiza o estado 'active' nos links de navegação
    document.querySelectorAll('.nav-link').forEach(link => {
        link.classList.remove('active');
    });
    event.currentTarget.classList.add('active');
}


/* ================================
   COMUNICAÇÃO COM O ESP32
================================ */

// Busca dados da API (/api/data) em loop
function fetchData() {
    const statusIndicator = document.getElementById('status-indicator');
    const statusText = document.getElementById('status-text');

    setInterval(async () => {
        try {
            const response = await fetch('/api/data');
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            const data = await response.json();

            // Atualiza UI e status de conexão OK
            updateInterface(data);
            statusIndicator.className = 'status-ok';
            statusText.textContent = 'Conectado';

        } catch (error) {
            console.error("Falha ao buscar dados do ESP32:", error);

            // Atualiza status de conexão para ERRO
            statusIndicator.className = 'status-error';
            statusText.textContent = 'Erro de Conexão';
        }
    }, REQUEST_RATE);
}

// Envia comandos para a API (/api/command)
async function sendInverterCommand(state) {
    if (!operatorPin) {
        return;
    }

    const freqValue = parseFloat(document.getElementById('freqSlider').value);

    // Monta o objeto de comando dinamicamente
    const command = {
        frequency: freqValue,
        pin: operatorPin
    };
    if (state) { // Adiciona o estado do motor apenas se ele for fornecido (RUNNING/STOPPED)
        command.motorState = state;
    }

    try {
        const response = await fetch('/api/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(command)
        });

        if (!response.ok) {
            const errorData = await response.json();
            if (response.status === 401) { // PIN incorreto ou ausente
                alert(`Erro de Autenticação: ${errorData.error}`);
                // Desloga o operador se o PIN for inválido
                logoutOperator();
            } else {
                throw new Error(errorData.error || `HTTP error! status: ${response.status}`);
            }
        }
    } catch (error) {
        console.error("Falha ao enviar comando para o ESP32:", error);
        alert(`Erro ao comunicar com o dispositivo: ${error.message}`);
    }
}

/* ================================
   INTERFACE DO USUÁRIO (UI)
================================ */

function updateInterface(data) {
    // --- RPMs ---
    document.getElementById('rpm1').innerText = data.rpm1 || 0;
    document.getElementById('rpm2').innerText = data.rpm2 || 0;
    document.getElementById('rpm3').innerText = data.rpm3 || 0;
    document.getElementById('rpm4').innerText = data.rpm4 || 0;

    // --- Inversor ---
    const invStatusEl = document.getElementById('invStatus');
    const invFreqEl = document.getElementById('invFreq');
    const currentStatus = data.inverterStatus || "Parado"; // Default to "Parado"

    invFreqEl.innerText = `${(data.inverterFrequency || 0).toFixed(1)} Hz`;

    // Limpa classes de status anteriores
    invStatusEl.classList.remove('status-text-on', 'status-text-off', 'status-text-error');

    // Lógica de 3 estados para o status do inversor
    switch (currentStatus) {
        case 'Girando':
            invStatusEl.innerText = 'LIGADO';
            invStatusEl.classList.add('status-text-on');
            break;
        case 'Falha':
            invStatusEl.innerText = 'EM FALHA';
            invStatusEl.classList.add('status-text-error');
            break;
        case 'Parado':
        default:
            invStatusEl.innerText = 'DESLIGADO';
            invStatusEl.classList.add('status-text-off');
            break;
    }

    // --- Gráfico ---
    updateChart(data);
}

function openModal() {
    document.getElementById('loginModal').style.display = 'flex';
}

function closeModal() {
    document.getElementById('loginModal').style.display = 'none';
    document.getElementById('pinInput').value = ""; // Limpa o PIN
}

async function checkAuth() {
    const pin = document.getElementById('pinInput').value;

    if (!pin || pin.length !== 4) {
        alert("PIN deve ter 4 dígitos.");
        return;
    }

    if (MOCK_LOGIN) {
        loginOperator();
        closeModal();
        return;
    }


    try {
        const response = await fetch('/api/auth', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ pin: pin })
        });

        if (!response.ok) {
            const errorData = await response.json();
            throw new Error(errorData.error || `PIN inválido`);
        }

        // PIN Válido!
        operatorPin = pin;
        loginOperator();
        closeModal();
        console.log("Autenticação bem-sucedida!");

    } catch (error) {
        console.error("Falha na autenticação:", error);
        alert(`Erro de Autenticação: ${error.message}`);
        // Limpa o campo do PIN e mantém o modal aberto para nova tentativa
        document.getElementById('pinInput').value = "";
    }
}

function loginOperator() {
    document.getElementById('operatorControl').classList.remove('hidden');
    document.getElementById('loginBtn').innerText = "Logout";
    document.getElementById('loginBtn').setAttribute('onclick', 'logoutOperator()');
    document.querySelector('a[href="#controle"]').style.display = 'inline';
}

function logoutOperator() {
    operatorPin = null; // Limpa o PIN
    document.getElementById('operatorControl').classList.add('hidden');
    document.getElementById('loginBtn').innerText = "Login Operador";
    document.getElementById('loginBtn').setAttribute('onclick', 'openModal()');
    // Esconde o link de navegação para a página de controle
    document.querySelector('a[href="#controle"]').style.display = 'none';
    // Opcional: Redireciona para o dashboard se estiver na página de controle
    if (document.querySelector('#controle').style.display === 'block') {
        document.querySelector('a[href="#dashboard"]').click();
    }
}

function calculate() {
    const input1 = parseFloat(document.getElementById('calc-input1').value);
    const input2 = parseFloat(document.getElementById('calc-input2').value);
    const operator = document.getElementById('calc-operator').value;
    const resultEl = document.getElementById('calc-result');
    let result;

    if (isNaN(input1) || isNaN(input2)) {
        resultEl.innerText = "Entradas inválidas";
        return;
    }

    switch (operator) {
        case 'add':
            result = input1 + input2;
            break;
        case 'subtract':
            result = input1 - input2;
            break;
        case 'multiply':
            result = input1 * input2;
            break;
        case 'divide':
            if (input2 === 0) {
                resultEl.innerText = "Divisão por zero";
                return;
            }
            result = input1 / input2;
            break;
        default:
            resultEl.innerText = "Operador inválido";
            return;
    }
    resultEl.innerText = result.toFixed(2);
}

/* ================================
   GRÁFICO (Chart.js)
================================ */

function initChart() {
    const ctx = document.getElementById('mainChart').getContext('2d');
    rpmChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                { label: 'Motor RPM', data: [], borderColor: '#ff4d4d', tension: 0.3, yAxisID: 'y' },
                { label: 'Eixo 2 RPM', data: [], borderColor: '#4da6ff', tension: 0.3, yAxisID: 'y' },
                { label: 'Eixo 3 RPM', data: [], borderColor: '#00ff88', tension: 0.3, yAxisID: 'y' },
                { label: 'Eixo 4 RPM', data: [], borderColor: '#ffd633', tension: 0.3, yAxisID: 'y' },
                { label: 'Frequência (Hz)', data: [], borderColor: '#f92672', tension: 0.3, yAxisID: 'y1', borderDash: [5, 5] }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: false,
            scales: {
                y: {
                    type: 'linear',
                    display: true,
                    position: 'left',
                    title: {
                        display: true,
                        text: 'RPM'
                    }
                    // min/max serão definidos dinamicamente em updateChart()
                },
                y1: {
                    type: 'linear',
                    display: true,
                    position: 'right',
                    min: 0,
                    max: 70, // Range fixo para frequência (Hz)
                    title: {
                        display: true,
                        text: 'Hz'
                    },
                    // Não desenha a grade para o eixo Y secundário para não poluir o gráfico
                    grid: {
                        drawOnChartArea: false,
                    },
                }
            }
        }
    });
}
function updateChart(data) {
    // Adiciona o novo timestamp
    rpmChart.data.labels.push(new Date().toLocaleTimeString());

    // Adiciona os novos dados a cada dataset correspondente
    rpmChart.data.datasets.forEach(dataset => {
        switch (dataset.label) {
            case 'Motor RPM':
                dataset.data.push(data.rpm1 || 0);
                break;
            case 'Eixo 2 RPM':
                dataset.data.push(data.rpm2 || 0);
                break;
            case 'Eixo 3 RPM':
                dataset.data.push(data.rpm3 || 0);
                break;
            case 'Eixo 4 RPM':
                dataset.data.push(data.rpm4 || 0);
                break;
            case 'Frequência (Hz)':
                dataset.data.push(data.inverterFrequency || 0);
                break;
        }
    });

    // Mantém o histórico do gráfico em 20 pontos
    if (rpmChart.data.labels.length > 20) {
        rpmChart.data.labels.shift();
        rpmChart.data.datasets.forEach(ds => ds.data.shift());
    }

    // --- LÓGICA DE AUTO-SCALE DO EIXO Y (RPMs) ---
    // 1. Filtra apenas os datasets de RPM (que usam o eixo 'y')
    const rpmDatasets = rpmChart.data.datasets.filter(ds => ds.yAxisID === 'y');
    const allDataPoints = rpmDatasets.flatMap(dataset => dataset.data);

    if (allDataPoints.length > 0) {
        const minValue = Math.min(...allDataPoints);
        const maxValue = Math.max(...allDataPoints);

        // 2. Calcula um buffer (margem) para que os picos não fiquem na borda
        const range = maxValue - minValue;
        const buffer = range > 0 ? range * 0.2 : 100; // Buffer de 20% ou 100 se a linha for plana

        // 3. Define o novo range do eixo Y de RPM, garantindo que o mínimo não seja negativo
        rpmChart.options.scales.y.min = Math.max(0, Math.floor(minValue - buffer));
        rpmChart.options.scales.y.max = Math.ceil(maxValue + buffer);
    } else {
        // Fallback se não houver dados
        rpmChart.options.scales.y.min = 0;
        rpmChart.options.scales.y.max = 1000;
    }

    // Atualiza o gráfico sem animação para melhor performance
    rpmChart.update('none');
}
