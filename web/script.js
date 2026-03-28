/* ================================
   CONFIGURAÇÕES GERAIS
================================ */
const REQUEST_RATE = 1000; // ms: Taxa de atualização de dados do ESP32
const MOCK_SENSORS = true;
const MOCK_LOGIN = true;
let rpmChart;
let operatorPin = null;

window.onload = () => {
    initChart();
    CalcWizard.init();
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
    document.getElementById('rpm1').innerText = (data.rpm1 || 0) + " RPM";
    document.getElementById('rpm2').innerText = (data.rpm2 || 0) + " RPM";
    document.getElementById('rpm3').innerText = (data.rpm3 || 0) + " RPM";
    document.getElementById('rpm4').innerText = (data.rpm4 || 0) + " RPM";

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

/* ================================
   CALCULADORA - WIZARD DE ELEMENTOS DE MÁQUINA
================================ */

const CalcWizard = {
    currentStep: 0,
    totalSteps: 6,
    CV_TO_KW: 0.7355,

    BEARING_TYPES: [
        { name: 'Rolamento de Esferas', efficiency: 0.99 },
        { name: 'Rolamento de Rolos Cilíndricos', efficiency: 0.98 },
        { name: 'Rolamento de Rolos Cônicos', efficiency: 0.97 },
        { name: 'Rolamento de Agulhas', efficiency: 0.985 },
        { name: 'Bucha (Mancal de Deslizamento)', efficiency: 0.95 },
    ],

    // Output de cada estágio
    outputs: [
        { rpmIn: 0, rpmOut: 0, powerIn: 0, powerOut: 0, torque: 0, efficiency: 1, ratio: 1 },
        { rpmIn: 0, rpmOut: 0, powerIn: 0, powerOut: 0, torque: 0, efficiency: 1, ratio: 1 },
        { rpmIn: 0, rpmOut: 0, powerIn: 0, powerOut: 0, torque: 0, efficiency: 1, ratio: 1 },
        { rpmIn: 0, rpmOut: 0, powerIn: 0, powerOut: 0, torque: 0, efficiency: 1, ratio: 1 },
        { rpmIn: 0, rpmOut: 0, powerIn: 0, powerOut: 0, torque: 0, efficiency: 1, ratio: 1 },
    ],

    init() {
        this.populateBearingSelects();
        this.bindInputListeners();
        this.updateStepperUI();
        this.updateNavUI();
    },

    populateBearingSelects() {
        ['engr-rolamento', 'corr-rolamento'].forEach(id => {
            const sel = document.getElementById(id);
            this.BEARING_TYPES.forEach((bt, i) => {
                const opt = document.createElement('option');
                opt.value = i;
                opt.textContent = bt.name + ' (η=' + (bt.efficiency * 100).toFixed(1) + '%)';
                sel.appendChild(opt);
            });
        });
    },

    bindInputListeners() {
        const bind = (ids, step) => {
            ids.forEach(id => {
                const el = document.getElementById(id);
                el.addEventListener('input', () => this.recalculateFrom(step));
                if (el.tagName === 'SELECT') {
                    el.addEventListener('change', () => this.recalculateFrom(step));
                }
            });
        };
        bind(['motor-potencia', 'motor-frequencia', 'motor-polos', 'motor-slip', 'motor-rendimento'], 0);
        bind(['polia-d1', 'polia-d2', 'polia-rendimento'], 1);
        bind(['engr-z1', 'engr-z2', 'engr-modulo', 'engr-rendimento', 'engr-rolamento'], 2);
        bind(['corr-z1', 'corr-z2', 'corr-passo', 'corr-rendimento', 'corr-rolamento'], 3);
        bind(['red-relacao', 'red-rendimento'], 4);
    },

    recalculateFrom(step) {
        const calcFns = [this.calcMotor, this.calcPolias, this.calcEngrenagens, this.calcCorrentes, this.calcReducer];
        for (let i = step; i < 5; i++) {
            calcFns[i].call(this);
        }
        if (this.currentStep === 5 && !this._buildingSummary) this.buildSummary();
    },

    getVal(id) { return parseFloat(document.getElementById(id).value) || 0; },

    fmt(val, dec) {
        if (isNaN(val) || !isFinite(val)) return '—';
        return val.toFixed(dec === undefined ? 2 : dec);
    },

    calcTorque(powerKw, rpm) {
        return rpm > 0 ? (powerKw * 9549.297) / rpm : 0;
    },

    // --- ETAPA 0: MOTOR ---
    calcMotor() {
        const potCV = this.getVal('motor-potencia');
        const freq = this.getVal('motor-frequencia');
        const polos = this.getVal('motor-polos');
        const slip = this.getVal('motor-slip') / 100;
        const rend = this.getVal('motor-rendimento') / 100;

        const nSync = (polos > 0 && freq > 0) ? (120 * freq) / polos : 0;
        const nReal = nSync * (1 - slip);
        const potBruta = potCV * this.CV_TO_KW;
        const potKw = potBruta * rend;
        const torque = this.calcTorque(potKw, nReal);

        this.outputs[0] = { rpmIn: 0, rpmOut: nReal, powerIn: potBruta, powerOut: potKw, torque, efficiency: rend, ratio: 1 };

        document.getElementById('res-motor-nsync').textContent = this.fmt(nSync, 0) + ' RPM';
        document.getElementById('res-motor-nreal').textContent = this.fmt(nReal, 1) + ' RPM';
        document.getElementById('res-motor-potutil').textContent = this.fmt(potKw, 3) + ' kW';
        document.getElementById('res-motor-torque').textContent = this.fmt(torque, 2) + ' N·m';
    },

    // --- ETAPA 1: POLIAS ---
    calcPolias() {
        const prev = this.outputs[0];
        const d1 = this.getVal('polia-d1');
        const d2 = this.getVal('polia-d2');
        const rend = this.getVal('polia-rendimento') / 100;

        const ratio = d1 > 0 ? d2 / d1 : 0;
        const rpmOut = ratio > 0 ? prev.rpmOut / ratio : 0;
        const powerOut = prev.powerOut * rend;
        const torque = this.calcTorque(powerOut, rpmOut);

        this.outputs[1] = { rpmIn: prev.rpmOut, rpmOut, powerIn: prev.powerOut, powerOut, torque, efficiency: rend, ratio };

        document.getElementById('res-polia-i').textContent = this.fmt(ratio, 2);
        document.getElementById('res-polia-rpm').textContent = this.fmt(rpmOut, 1) + ' RPM';
        document.getElementById('res-polia-pot').textContent = this.fmt(powerOut, 3) + ' kW';
        document.getElementById('res-polia-torque').textContent = this.fmt(torque, 2) + ' N·m';
    },

    // --- ETAPA 2: ENGRENAGENS ---
    calcEngrenagens() {
        const prev = this.outputs[1];
        const z1 = this.getVal('engr-z1');
        const z2 = this.getVal('engr-z2');
        const mod = this.getVal('engr-modulo');
        const rend = this.getVal('engr-rendimento') / 100;
        const bIdx = parseInt(document.getElementById('engr-rolamento').value) || 0;
        const bEff = this.BEARING_TYPES[bIdx]?.efficiency || 1;

        const ratio = z1 > 0 ? z2 / z1 : 0;
        const rpmOut = ratio > 0 ? prev.rpmOut / ratio : 0;
        const d1 = z1 * mod;
        const d2 = z2 * mod;
        const totalEff = rend * bEff;
        const powerOut = prev.powerOut * totalEff;
        const torque = this.calcTorque(powerOut, rpmOut);

        this.outputs[2] = { rpmIn: prev.rpmOut, rpmOut, powerIn: prev.powerOut, powerOut, torque, efficiency: totalEff, ratio };

        document.getElementById('res-engr-i').textContent = this.fmt(ratio, 2);
        document.getElementById('res-engr-rpm').textContent = this.fmt(rpmOut, 1) + ' RPM';
        document.getElementById('res-engr-d1').textContent = this.fmt(d1, 1) + ' mm';
        document.getElementById('res-engr-d2').textContent = this.fmt(d2, 1) + ' mm';
        document.getElementById('res-engr-pot').textContent = this.fmt(powerOut, 3) + ' kW';
        document.getElementById('res-engr-torque').textContent = this.fmt(torque, 2) + ' N·m';
    },

    // --- ETAPA 3: CORRENTES ---
    calcCorrentes() {
        const prev = this.outputs[2];
        const z1 = this.getVal('corr-z1');
        const z2 = this.getVal('corr-z2');
        const passo = this.getVal('corr-passo');
        const rend = this.getVal('corr-rendimento') / 100;
        const bIdx = parseInt(document.getElementById('corr-rolamento').value) || 0;
        const bEff = this.BEARING_TYPES[bIdx]?.efficiency || 1;

        const ratio = z1 > 0 ? z2 / z1 : 0;
        const rpmOut = ratio > 0 ? prev.rpmOut / ratio : 0;
        const d1 = z1 > 0 ? passo / Math.sin(Math.PI / z1) : 0;
        const d2 = z2 > 0 ? passo / Math.sin(Math.PI / z2) : 0;
        const totalEff = rend * bEff;
        const powerOut = prev.powerOut * totalEff;
        const torque = this.calcTorque(powerOut, rpmOut);

        this.outputs[3] = { rpmIn: prev.rpmOut, rpmOut, powerIn: prev.powerOut, powerOut, torque, efficiency: totalEff, ratio };

        document.getElementById('res-corr-i').textContent = this.fmt(ratio, 2);
        document.getElementById('res-corr-rpm').textContent = this.fmt(rpmOut, 1) + ' RPM';
        document.getElementById('res-corr-d1').textContent = this.fmt(d1, 1) + ' mm';
        document.getElementById('res-corr-d2').textContent = this.fmt(d2, 1) + ' mm';
        document.getElementById('res-corr-pot').textContent = this.fmt(powerOut, 3) + ' kW';
        document.getElementById('res-corr-torque').textContent = this.fmt(torque, 2) + ' N·m';
    },

    // --- ETAPA 4: REDUTOR ---
    calcReducer() {
        const prev = this.outputs[3];
        const ratio = this.getVal('red-relacao');
        const rend = this.getVal('red-rendimento') / 100;

        const rpmOut = ratio > 0 ? prev.rpmOut / ratio : 0;
        const powerOut = prev.powerOut * rend;
        const torque = this.calcTorque(powerOut, rpmOut);

        this.outputs[4] = { rpmIn: prev.rpmOut, rpmOut, powerIn: prev.powerOut, powerOut, torque, efficiency: rend, ratio };

        document.getElementById('res-red-rpm').textContent = this.fmt(rpmOut, 1) + ' RPM';
        document.getElementById('res-red-pot').textContent = this.fmt(powerOut, 3) + ' kW';
        document.getElementById('res-red-torque').textContent = this.fmt(torque, 2) + ' N·m';
    },

    // --- ETAPA 5: RESUMO ---
    buildSummary() {
        this._buildingSummary = true;
        this.recalculateFrom(0);
        const c = document.getElementById('calc-summary-content');
        const names = ['Motor', 'Polias e Correia', 'Engrenagens', 'Correntes', 'Redutor'];

        let gEff = 1;
        this.outputs.forEach(o => { gEff *= o.efficiency; });
        const last = this.outputs[4];

        let h = '<div class="calc-summary-global">';
        h += '<div class="calc-summary-stat"><span class="stat-label">Rendimento Global</span><span class="stat-value">' + this.fmt(gEff * 100, 1) + '%</span></div>';
        h += '<div class="calc-summary-stat"><span class="stat-label">Potência Final</span><span class="stat-value">' + this.fmt(last.powerOut, 3) + ' kW</span></div>';
        h += '<div class="calc-summary-stat"><span class="stat-label">RPM Final</span><span class="stat-value">' + this.fmt(last.rpmOut, 1) + '</span></div>';
        h += '<div class="calc-summary-stat"><span class="stat-label">Torque Final</span><span class="stat-value">' + this.fmt(last.torque, 2) + ' N·m</span></div>';
        h += '</div>';

        h += '<div class="calc-summary-table-wrap"><table class="calc-summary-table">';
        h += '<thead><tr><th>Estágio</th><th>RPM Ent.</th><th>RPM Saída</th><th>Pot. Ent. (kW)</th><th>Pot. Saída (kW)</th><th>Torque (N·m)</th><th>η</th><th>Relação</th></tr></thead>';
        h += '<tbody>';
        this.outputs.forEach((o, i) => {
            h += '<tr>';
            h += '<td class="stage-name">' + names[i] + '</td>';
            h += '<td>' + (i === 0 ? '—' : this.fmt(o.rpmIn, 1)) + '</td>';
            h += '<td>' + this.fmt(o.rpmOut, 1) + '</td>';
            h += '<td>' + this.fmt(o.powerIn, 3) + '</td>';
            h += '<td>' + this.fmt(o.powerOut, 3) + '</td>';
            h += '<td>' + this.fmt(o.torque, 2) + '</td>';
            h += '<td>' + this.fmt(o.efficiency * 100, 1) + '%</td>';
            h += '<td>' + (i === 0 ? '—' : this.fmt(o.ratio, 2) + ':1') + '</td>';
            h += '</tr>';
        });
        h += '</tbody></table></div>';
        c.innerHTML = h;
        this._buildingSummary = false;
    },

    // --- RELATÓRIO (PRINT) ---
    printReport() {
        this._buildingSummary = true;
        this.recalculateFrom(0);
        this._buildingSummary = false;

        const names = ['Motor', 'Polias e Correia', 'Engrenagens', 'Correntes', 'Redutor'];
        const now = new Date();
        const dateStr = now.toLocaleDateString('pt-BR') + ' ' + now.toLocaleTimeString('pt-BR');

        let gEff = 1;
        this.outputs.forEach(o => { gEff *= o.efficiency; });
        const last = this.outputs[4];

        let r = '<div id="print-report">';
        r += '<h1>Relatório — Bancada Didática de Elementos de Máquina</h1>';
        r += '<p class="print-subtitle">Gerado em: ' + dateStr + '</p>';

        // Globals
        r += '<div class="print-globals">';
        r += '<div class="print-global-item"><span class="label">Rendimento Global</span><span class="value">' + this.fmt(gEff * 100, 1) + '%</span></div>';
        r += '<div class="print-global-item"><span class="label">Potência Final</span><span class="value">' + this.fmt(last.powerOut, 3) + ' kW</span></div>';
        r += '<div class="print-global-item"><span class="label">RPM Final</span><span class="value">' + this.fmt(last.rpmOut, 1) + '</span></div>';
        r += '<div class="print-global-item"><span class="label">Torque Final</span><span class="value">' + this.fmt(last.torque, 2) + ' N·m</span></div>';
        r += '</div>';

        // Input data per stage
        r += '<div class="print-section"><h2>Dados de Entrada</h2>';
        r += '<table class="print-input-table"><thead><tr><th>Estágio</th><th>Parâmetro</th><th>Valor</th></tr></thead><tbody>';

        const inputs = [
            { stage: 'Motor', params: [
                ['Potência Nominal', this.getVal('motor-potencia') + ' CV'],
                ['Frequência Inversor', this.getVal('motor-frequencia') + ' Hz'],
                ['Nº Polos', this.getVal('motor-polos')],
                ['Escorregamento', this.getVal('motor-slip') + '%'],
                ['Rendimento', this.getVal('motor-rendimento') + '%'],
            ]},
            { stage: 'Polias e Correia', params: [
                ['Diâm. Polia Motora (D₁)', this.getVal('polia-d1') + ' mm'],
                ['Diâm. Polia Movida (D₂)', this.getVal('polia-d2') + ' mm'],
                ['Rendimento', this.getVal('polia-rendimento') + '%'],
            ]},
            { stage: 'Engrenagens', params: [
                ['Nº Dentes Motora (Z₁)', this.getVal('engr-z1')],
                ['Nº Dentes Movida (Z₂)', this.getVal('engr-z2')],
                ['Módulo (m)', this.getVal('engr-modulo') + ' mm'],
                ['Rendimento Par', this.getVal('engr-rendimento') + '%'],
                ['Rolamento', document.getElementById('engr-rolamento').selectedOptions[0]?.text || '—'],
            ]},
            { stage: 'Correntes', params: [
                ['Nº Dentes Motora (Z₁)', this.getVal('corr-z1')],
                ['Nº Dentes Movida (Z₂)', this.getVal('corr-z2')],
                ['Passo (p)', this.getVal('corr-passo') + ' mm'],
                ['Rendimento', this.getVal('corr-rendimento') + '%'],
                ['Rolamento', document.getElementById('corr-rolamento').selectedOptions[0]?.text || '—'],
            ]},
            { stage: 'Redutor', params: [
                ['Relação (i)', this.getVal('red-relacao') + ':1'],
                ['Rendimento', this.getVal('red-rendimento') + '%'],
            ]},
        ];

        inputs.forEach(s => {
            s.params.forEach((p, i) => {
                r += '<tr>';
                if (i === 0) r += '<td rowspan="' + s.params.length + '">' + s.stage + '</td>';
                r += '<td>' + p[0] + '</td><td>' + p[1] + '</td></tr>';
            });
        });
        r += '</tbody></table></div>';

        // Results table
        r += '<div class="print-section"><h2>Resultados por Estágio</h2>';
        r += '<table><thead><tr><th>Estágio</th><th>RPM Ent.</th><th>RPM Saída</th><th>Pot. Ent. (kW)</th><th>Pot. Saída (kW)</th><th>Torque (N·m)</th><th>η</th><th>Relação</th></tr></thead><tbody>';
        this.outputs.forEach((o, i) => {
            r += '<tr>';
            r += '<td>' + names[i] + '</td>';
            r += '<td>' + (i === 0 ? '—' : this.fmt(o.rpmIn, 1)) + '</td>';
            r += '<td>' + this.fmt(o.rpmOut, 1) + '</td>';
            r += '<td>' + this.fmt(o.powerIn, 3) + '</td>';
            r += '<td>' + this.fmt(o.powerOut, 3) + '</td>';
            r += '<td>' + this.fmt(o.torque, 2) + '</td>';
            r += '<td>' + this.fmt(o.efficiency * 100, 1) + '%</td>';
            r += '<td>' + (i === 0 ? '—' : this.fmt(o.ratio, 2) + ':1') + '</td>';
            r += '</tr>';
        });
        r += '</tbody></table></div>';

        r += '<p class="print-footer">Bancada Didática de Elementos de Máquina — Relatório gerado automaticamente</p>';
        r += '</div>';

        // Inject, print, remove
        const div = document.createElement('div');
        div.innerHTML = r;
        document.body.appendChild(div);
        window.print();
        document.body.removeChild(div);
    },
    // --- NAVEGAÇÃO ---
    goToStep(step) {
        if (step < 0 || step >= this.totalSteps) return;
        document.querySelectorAll('.calc-step-panel').forEach((p, i) => {
            p.classList.toggle('active', i === step);
        });
        this.currentStep = step;
        this.updateStepperUI();
        this.updateNavUI();
        if (step === 5) this.buildSummary();
    },

    nextStep() { this.goToStep(this.currentStep + 1); },
    prevStep() { this.goToStep(this.currentStep - 1); },

    updateStepperUI() {
        document.querySelectorAll('.calc-stepper-item').forEach((item, i) => {
            item.classList.remove('active', 'completed');
            if (i < this.currentStep) item.classList.add('completed');
            else if (i === this.currentStep) item.classList.add('active');
        });
        document.querySelectorAll('.stepper-connector').forEach((conn, i) => {
            conn.classList.toggle('active', i < this.currentStep);
        });
        document.getElementById('calc-step-indicator').textContent = 'Etapa ' + (this.currentStep + 1) + ' de ' + this.totalSteps;
    },

    updateNavUI() {
        const prev = document.getElementById('calc-prev-btn');
        const next = document.getElementById('calc-next-btn');
        prev.style.visibility = this.currentStep === 0 ? 'hidden' : 'visible';
        if (this.currentStep === this.totalSteps - 1) {
            next.style.visibility = 'hidden';
        } else {
            next.style.visibility = 'visible';
            next.textContent = this.currentStep === this.totalSteps - 2 ? 'Ver Resumo →' : 'Próximo →';
        }
    },
};

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
