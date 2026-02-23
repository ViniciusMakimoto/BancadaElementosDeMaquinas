/* ================================
   CONFIGURAÇÕES GERAIS
================================ */
const REQUEST_RATE = 1000; // ms: Taxa de atualização de dados do ESP32
let rpmChart;

window.onload = () => {
    initChart();
    fetchData(); // Inicia a busca de dados do ESP32

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
   COMUNICAÇÃO COM O ESP32
================================ */

// Busca dados da API (/api/data) em loop
function fetchData() {
    setInterval(async () => {
        try {
            const response = await fetch('/api/data');
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            const data = await response.json();
            updateInterface(data);
        } catch (error) {
            console.error("Falha ao buscar dados do ESP32:", error);
        }
    }, REQUEST_RATE);
}

// Envia comandos para a API (/api/command)
async function sendInverterCommand(state) {
    const freqValue = parseFloat(document.getElementById('freqSlider').value);
    
    // Monta o objeto de comando dinamicamente
    const command = { frequency: freqValue };
    if (state) { // Adiciona o estado do motor apenas se ele for fornecido (RUNNING/STOPPED)
        command.motorState = state;
    }

    try {
        await fetch('/api/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(command)
        });
    } catch (error) {
        console.error("Falha ao enviar comando para o ESP32:", error);
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
    const currentStatus = data.inverterStatus || "Parado";

    invFreqEl.innerText = `${(data.inverterFrequency || 0).toFixed(1)} Hz`;
    invStatusEl.innerText = currentStatus.toUpperCase();
    
    invStatusEl.className = (currentStatus === "Girando") ? "status-on" : "status-off";

    // --- Gráfico ---
    updateChart(data);
}

function openModal() {
    document.getElementById('loginModal').style.display = 'flex';
}

function checkAuth() {
    const pin = document.getElementById('pinInput').value;
    if (pin === "1234") {
        document.getElementById('operatorControl').classList.remove('hidden');
        document.getElementById('loginModal').style.display = 'none';
        document.getElementById('loginBtn').innerText = "Operador Logado";
    } else {
        alert("PIN Incorreto!");
    }
    document.getElementById('pinInput').value = "";
}

/* ================================
   GRÁFICO (Chart.js)
================================ */

function initChart() {
    const ctx = document.getElementById('vibrationChart').getContext('2d');
    rpmChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                { label: 'Motor RPM', data: [], borderColor: '#ff4d4d', tension: 0.3 },
                { label: 'Eixo 2 RPM', data: [], borderColor: '#4da6ff', tension: 0.3 },
                { label: 'Eixo 3 RPM', data: [], borderColor: '#00ff88', tension: 0.3 },
                { label: 'Eixo 4 RPM', data: [], borderColor: '#ffd633', tension: 0.3 }
            ]
        },
        options: {
            responsive: true,
            animation: false,
            scales: { y: { beginAtZero: true, max: 2000 } }
        }
    });
}

function updateChart(data) {
    rpmChart.data.labels.push(new Date().toLocaleTimeString());
    rpmChart.data.datasets.forEach((ds, i) => {
        ds.data.push(data[`rpm${i+1}`] || 0);
    });

    if (rpmChart.data.labels.length > 20) {
        rpmChart.data.labels.shift();
        rpmChart.data.datasets.forEach(ds => ds.data.shift());
    }
    rpmChart.update('none');
}
