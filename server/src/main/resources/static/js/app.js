let tempChart, humChart;
let isLiveMode = true;

function setDefaultDates() {
    const now = new Date();
    const tzOffset = now.getTimezoneOffset() * 60000;
    const localNow = new Date(now - tzOffset);

    const startOfDay = new Date(localNow);
    startOfDay.setUTCHours(0, 0, 0, 0);

    document.getElementById('endDate').value = localNow.toISOString().slice(0, 16);
    document.getElementById('startDate').value = startOfDay.toISOString().slice(0, 16);
}

function applyCustomDates() {
    isLiveMode = false;
    fetchAndDrawCharts();
}

function resetToLive() {
    isLiveMode = true;
    setDefaultDates();
    fetchAndDrawCharts();
}

async function fetchAndDrawCharts() {
    if (isLiveMode) {
        const now = new Date();
        const tzOffset = now.getTimezoneOffset() * 60000;
        const localNow = new Date(now - tzOffset);
        document.getElementById('endDate').value = localNow.toISOString().slice(0, 16);
    }

    const start = document.getElementById('startDate').value;
    const end = document.getElementById('endDate').value;

    try {
        const response = await fetch(`/api/sensor?start=${start}&end=${end}`);
        const data = await response.json();

        if (data.length === 0) {
            console.warn("За обраний період немає даних");
            return;
        }

        const timeLabels = [];
        const tempData = [];
        const humData = [];
        let sumTemp = 0, sumHum = 0, sumPres = 0;

        data.forEach(item => {
            const date = new Date(item.timestamp);
            timeLabels.push(date.toLocaleTimeString('uk-UA', {hour: '2-digit', minute: '2-digit'}));

            tempData.push(item.temperature);
            humData.push(item.humidity);

            sumTemp += item.temperature;
            sumHum += item.humidity;
            sumPres += item.pressure;
        });

        document.getElementById('avgTemp').innerText = (sumTemp / data.length).toFixed(1) + ' °C';
        document.getElementById('avgHum').innerText = (sumHum / data.length).toFixed(1) + ' %';
        document.getElementById('avgPres').innerText = (sumPres / data.length).toFixed(1) + ' hPa';

        if (tempChart) {
            tempChart.data.labels = timeLabels;
            tempChart.data.datasets[0].data = tempData;
            tempChart.update();

            humChart.data.labels = timeLabels;
            humChart.data.datasets[0].data = humData;
            humChart.update();
        } else {
            Chart.defaults.color = '#a0a0b5';
            Chart.defaults.borderColor = '#2a2a3c';

            tempChart = createChart('tempChart', 'Температура (°C)', timeLabels, tempData, '#00e676', 'rgba(0, 230, 118, 0.1)');
            humChart = createChart('humChart', 'Вологість (%)', timeLabels, humData, '#00bfff', 'rgba(0, 191, 255, 0.1)');
        }
    } catch (error) {
        console.error("Помилка при завантаженні даних:", error);
    }
}

function createChart(canvasId, label, labels, data, borderColor, bgColor) {
    const ctx = document.getElementById(canvasId).getContext('2d');
    return new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: label,
                data: data,
                borderColor: borderColor,
                backgroundColor: bgColor,
                borderWidth: 2,
                pointRadius: 0,
                pointHitRadius: 10,
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: {
                duration: 400
            }
        }
    });
}

async function loadTargetSettings() {
    try {
        const response = await fetch('/api/sensor/settings');
        const data = await response.json();
        document.getElementById('targetTempInput').value = data.temp;
        document.getElementById('targetHumInput').value = data.hum;
        document.getElementById('targetDevInput').value = data.dev;
    } catch (error) {
        console.error("Помилка завантаження налаштувань:", error);
    }
}

async function saveTargetSettings() {
    const t = parseFloat(document.getElementById('targetTempInput').value);
    const h = parseFloat(document.getElementById('targetHumInput').value);
    const d = parseFloat(document.getElementById('targetDevInput').value);

    try {
        await fetch('/api/sensor/settings', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({temp: t, hum: h, dev: d})
        });
        alert('Уставки збережено! Плата отримає їх протягом хвилини і змінить стан виконавчого механізму.');
    } catch (error) {
        alert('Помилка при збереженні!');
    }
}

loadTargetSettings();

setDefaultDates();
fetchAndDrawCharts();

setInterval(fetchAndDrawCharts, 60000);