let rpmChart;
let simulationInterval;

let motorRPM = 0;
let targetRPM = 0;

let inverterOn = false;
let inverterFreq = 0;

let REQUEST_RATE = 1000;

window.onload = () => {
    initChart();
    // Inicia a busca de dados do ESP32
    fetchData(); 
};

/* ================================
   BUSCA DADOS DO ESP32
================================ */

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
            // Opcional: mostrar um indicador de erro na UI
        }
    }, REQUEST_RATE); // Busca dados a cada X segundos
}

function initChart() {
    const ctx = document.getElementById('vibrationChart').getContext('2d');

    rpmChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Motor RPM',
                    data: [],
                    borderColor: '#ff4d4d',
                    tension: 0.3
                },
                {
                    label: 'Eixo 2 RPM',
                    data: [],
                    borderColor: '#4da6ff',
                    tension: 0.3
                },
                {
                    label: 'Eixo 3 RPM',
                    data: [],
                    borderColor: '#00ff88',
                    tension: 0.3
                },
                {
                    label: 'Eixo 4 RPM',
                    data: [],
                    borderColor: '#ffd633',
                    tension: 0.3
                }
            ]
        },
        options: {
            responsive: true,
            animation: false,
            scales: {
                y: {
                    beginAtZero: true,
                    max: 2000
                }
            }
        }
    });
}

function sendControl(action) {

    if (action === "start") {
        inverterOn = true;
        document.getElementById("invStatus").innerText = "LIGADO";
        document.getElementById("invStatus").className = "status-on";
    }

    if (action === "stop") {
        inverterOn = false;
        inverterFreq = 0;
        targetRPM = 0;

        document.getElementById("invStatus").innerText = "DESLIGADO";
        document.getElementById("invStatus").className = "status-off";
    }
}

function updateFreq(val) {

    inverterFreq = parseFloat(val);

    document.getElementById('freqVal').innerText = val;
    document.getElementById('invFreq').innerText = val + " Hz";

    // Conversão simples Hz → RPM (motor 4 polos)
    if (inverterOn) {
        targetRPM = inverterFreq * 30; 
    }
}

function openModal() {
    document.getElementById('loginModal').style.display = 'flex';
}
 
function checkAuth() {
    const pin = document.getElementById('pinInput').value;
    // Validação simples no front (Idealmente, o ESP validaria via WS)
    if(pin === "1234") {
        document.getElementById('operatorControl').classList.remove('hidden');
        document.getElementById('loginModal').style.display = 'none';
        document.getElementById('loginBtn').innerText = "Operador Logado";
    } else {
        alert("PIN Incorreto!");
    }
}



/* ================================
   ATUALIZA INTERFACE + GRÁFICO
================================ */

function updateInterface(data) {

    document.getElementById('rpm1').innerText = data.rpm1;
    document.getElementById('rpm2').innerText = data.rpm2;
    document.getElementById('rpm3').innerText = data.rpm3;
    document.getElementById('rpm4').innerText = data.rpm4;

    rpmChart.data.labels.push(new Date().toLocaleTimeString());

    rpmChart.data.datasets[0].data.push(data.rpm1);
    rpmChart.data.datasets[1].data.push(data.rpm2);
    rpmChart.data.datasets[2].data.push(data.rpm3);
    rpmChart.data.datasets[3].data.push(data.rpm4);

    if (rpmChart.data.labels.length > 20) {
        rpmChart.data.labels.shift();
        rpmChart.data.datasets.forEach(ds => ds.data.shift());
    }

    rpmChart.update();
}
