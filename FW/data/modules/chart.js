// ============================================================================
// CHART.JS - Gestion du graphique journalier
// ============================================================================

// Variables globales du graphique
let dailyChart = null;
let chartUpdateInterval = parseInt(localStorage.getItem('chartInterval')) || 300000; // Charger depuis localStorage ou 5 min par défaut
let chartIntervalTimer = null;
let chartData = {
  labels: [],
  waterTemp: [],
  pressure: [],
  pump: [],           
  electro: [],        
  light: [],          
  coverOpen: [],
  electrovalve: [],
  heatPump: [],
  activeTimers: []
};

// VARIABLES GLOBALES POUR L'HISTORIQUE
let currentDisplayDate = null;  // Date actuellement affichée (YYYY-MM-DD)
let availableDatesCache = [];   // Cache des dates disponibles

// ============================================================================
// CHARGEMENT DES DONNÉES DEPUIS L'ESP32
// ============================================================================
function updateChartDisplay() {
  console.log('🔄 Mise à jour de l\'affichage du graphique...');
  
  // Si le graphique existe, le détruire avant de le recréer
  if (dailyChart) {
    console.log('🗑️ Destruction de l\'ancien graphique');
    dailyChart.destroy();
    dailyChart = null;
  }
  
  // Recréer le graphique avec les nouvelles données
  initChart();
  
  console.log('✅ Graphique mis à jour avec succès');
}

async function loadChartDataFromESP32(dateStr) {
  console.log(`📊 Chargement des données pour: ${dateStr}`);
  
  try {
    const response = await fetch(`/api/chart/data?date=${dateStr}`);
    
    if (!response.ok) {
      if (response.status === 404) {
        console.warn(`⚠️ Aucune donnée pour ${dateStr}`);
        alert(t('no_data_for_date') || `Aucune donnée pour ${dateStr}`);
        return null;
      }
      throw new Error(`HTTP ${response.status}`);
    }
    
    const data = await response.json();
    
    if (data.error) {
      console.error('❌ Erreur serveur:', data.error);
      return null;
    }
    
    console.log(`✅ Données chargées: ${data.count} points, intervalle ${data.interval}ms`);
    
    // Convertir le format ESP32 vers le format chart
    return convertESP32DataToChart(data);
    
  } catch (error) {
    console.error('❌ Erreur chargement données:', error);
    alert('Erreur lors du chargement des données');
    return null;
  }
}

// ============================================================================
// CONVERSION DES DONNÉES ESP32 → CHART.JS
// ============================================================================

function convertESP32DataToChart(esp32Data) {
  const converted = {
    labels: [],
    waterTemp: [],
    pressure: [],
    pump: [],           // ✅ AJOUTÉ
    electro: [],        // ✅ AJOUTÉ
    light: [],          // ✅ AJOUTÉ
    coverOpen: [],
    electrovalve: [],
    heatPump: [],
    activeTimers: []
  };
  
  if (!esp32Data.points || esp32Data.points.length === 0) {
    return converted;
  }
  
  for (const point of esp32Data.points) {
    // Convertir timestamp en label HH:MM
    const date = new Date(point.t * 1000);
    const hours = String(date.getHours()).padStart(2, '0');
    const minutes = String(date.getMinutes()).padStart(2, '0');
    converted.labels.push(`${hours}:${minutes}`);
    
    // Données
    converted.waterTemp.push(point.wt || null);
    // ✅ PRESSION : Multiplier par 10 pour affichage graphique (légende "bar x10")
    // Les valeurs ESP32 sont déjà calibrées en BAR, on multiplie juste pour l'affichage
    converted.pressure.push((point.pr || 0) * 10);
    
    // ✅ NOUVEAUX : États des relais
    converted.pump.push(point.rp ? 1 : 0);            // Pompe
    converted.electro.push(point.re ? 1 : 0);         // Électrolyseur
    converted.light.push(point.rl ? 1 : 0);           // Éclairage
    
    // Anciens (déjà présents)
    converted.coverOpen.push(point.co ? 1 : 0);
    converted.electrovalve.push(point.rv ? 1 : 0);
    converted.heatPump.push(point.rh ? 1 : 0);
    converted.activeTimers.push(point.at || 0);
  }
  
  console.log(`🔄 Conversion: ${converted.labels.length} points`);
  
  return converted;
}

// ============================================================================
// AFFICHER UNE DATE SPÉCIFIQUE
// ============================================================================

async function displayDateData(dateStr) {
  console.log(`📅 Affichage de la date: ${dateStr}`);
  
  // Afficher un indicateur de chargement
  const infoDiv = document.getElementById('chart-date-info');
  if (infoDiv) {
    infoDiv.innerHTML = `⏳ ${t('loading_data') || 'Chargement...'}`;
  }
  
  // Charger les données
  const data = await loadChartDataFromESP32(dateStr);
  
  if (!data || data.labels.length === 0) {
    if (infoDiv) {
      infoDiv.innerHTML = `❌ ${t('no_data_for_date') || 'Aucune donnée'}`;
    }
    
    // Réinitialiser le graphique
    chartData = {
      labels: [],
      waterTemp: [],
      pressure: [],
      pump: [],
      electro: [],
      light: [],
      coverOpen: [],
      electrovalve: [],
      heatPump: [],
      activeTimers: []
    };
    
    // Mettre à jour le badge à 0
    const pointsEl = document.getElementById('chart-points');
    if (pointsEl) {
      pointsEl.textContent = '0 points';
    }
    
    updateChartDisplay();
    return;
  }
  
  // Mettre à jour chartData global
  chartData = data;
  
  // Sauvegarder la date actuelle
  currentDisplayDate = dateStr;
  
  // Mettre à jour le sélecteur de date
  const dateSelector = document.getElementById('chart-date-selector');
  if (dateSelector) {
    dateSelector.value = dateStr;
  }
  
  // Mettre à jour les infos
  if (infoDiv) {
    const pointCount = data.labels.length;
    infoDiv.innerHTML = `
      📊 ${dateStr} - ${pointCount} ${t('points') || 'points'}
    `;
  }
  
  // Mettre à jour le badge de points
  const pointsEl = document.getElementById('chart-points');
  if (pointsEl) {
    pointsEl.textContent = data.labels.length + ' points';
  }
  
  // Rafraîchir l'affichage du graphique
  updateChartDisplay();
  
  console.log(`✅ Date ${dateStr} affichée avec succès`);
}

// ============================================================================
// NAVIGATION PAR JOUR
// ============================================================================

function navigateDayOffset(offset) {
  // Si pas de date courante, partir d'aujourd'hui
  if (!currentDisplayDate) {
    currentDisplayDate = new Date().toISOString().split('T')[0];
  }
  
  // Calculer la nouvelle date
  const currentDate = new Date(currentDisplayDate);
  currentDate.setDate(currentDate.getDate() + offset);
  
  const newDateStr = currentDate.toISOString().split('T')[0];
  
  console.log(`📅 Navigation: ${currentDisplayDate} → ${newDateStr} (${offset > 0 ? '+' : ''}${offset})`);
  
  displayDateData(newDateStr);
}

// ============================================================================
// CHARGER AUJOURD'HUI
// ============================================================================

function loadToday() {
  const today = new Date().toISOString().split('T')[0];
  console.log(`📆 Chargement d'aujourd'hui: ${today}`);
  displayDateData(today);
}

// ============================================================================
// CALLBACK DU SÉLECTEUR DE DATE
// ============================================================================

function onDateSelected() {
  const dateSelector = document.getElementById('chart-date-selector');
  if (!dateSelector) return;
  
  const selectedDate = dateSelector.value;
  
  if (!selectedDate) {
    console.warn('⚠️ Pas de date sélectionnée');
    return;
  }
  
  console.log(`🎯 Date sélectionnée manuellement: ${selectedDate}`);
  displayDateData(selectedDate);
}

// ============================================================================
// EXPORTER LA JOURNÉE EN CSV
// ============================================================================

async function exportCurrentDayCSV() {
  if (!currentDisplayDate) {
    alert('Aucune date sélectionnée');
    return;
  }
  
  console.log(`📥 Export CSV pour: ${currentDisplayDate}`);
  
  try {
    const response = await fetch(`/api/chart/export-csv?date=${currentDisplayDate}`);
    
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    
    const blob = await response.blob();
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `poolconnect_${currentDisplayDate}.csv`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    window.URL.revokeObjectURL(url);
    
    console.log('✅ Export CSV terminé');
    
  } catch (error) {
    console.error('❌ Erreur export CSV:', error);
    alert('Erreur lors de l\'export CSV');
  }
}

// ============================================================================
// AFFICHER LES DATES DISPONIBLES
// ============================================================================

async function showAvailableDates() {
  console.log('📋 Chargement des dates disponibles...');
  
  const modal = document.getElementById('dates-modal');
  const listDiv = document.getElementById('dates-list');
  
  if (!modal || !listDiv) {
    console.error('❌ Modal non trouvée');
    return;
  }
  
  modal.style.display = 'block';
  listDiv.innerHTML = `<p>⏳ ${t('loading_dates') || 'Chargement...'}</p>`;
  
  try {
    const response = await fetch('/api/chart/available-dates');
    
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    
    const dates = await response.json();
    
    if (!dates || dates.length === 0) {
      listDiv.innerHTML = '<p>Aucune date disponible</p>';
      return;
    }
    
    console.log(`✅ ${dates.length} dates trouvées`);
    
    // Trier par date décroissante (plus récent en premier)
    dates.sort((a, b) => new Date(b.date) - new Date(a.date));
    
    // Créer la liste
    let html = '';
    
    for (const dateInfo of dates) {
      const intervalMin = Math.round(dateInfo.interval / 60000);
      
      html += `
        <div class="date-item" onclick="PoolChart.displayDateData('${dateInfo.date}'); document.getElementById('dates-modal').style.display='none';">
          <div class="date-item-date">📅 ${dateInfo.date}</div>
          <div class="date-item-info">
            ${dateInfo.count} ${t('points') || 'points'} · 
            ${t('interval') || 'Intervalle'}: ${intervalMin} ${t('minutes') || 'min'}
          </div>
        </div>
      `;
    }
    
    listDiv.innerHTML = html;
    availableDatesCache = dates;
    
  } catch (error) {
    console.error('❌ Erreur chargement dates:', error);
    listDiv.innerHTML = '<p>❌ Erreur lors du chargement des dates</p>';
  }
}

// ============================================================================
// AFFICHER LES INFOS DE STOCKAGE
// ============================================================================

async function showStorageInfo() {
  console.log('💾 Chargement des infos de stockage...');
  
  const modal = document.getElementById('storage-modal');
  const contentDiv = document.getElementById('storage-info-content');
  
  if (!modal || !contentDiv) {
    console.error('❌ Modal non trouvée');
    return;
  }
  
  modal.style.display = 'block';
  contentDiv.innerHTML = `<p>⏳ ${t('loading_info') || 'Chargement...'}</p>`;
  
  try {
    const response = await fetch('/api/chart/storage-info');
    
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    
    const info = await response.json();
    
    console.log('✅ Infos de stockage chargées:', info);
    
    const totalMB = (info.totalBytes / 1024 / 1024).toFixed(2);
    const usedMB = (info.usedBytes / 1024 / 1024).toFixed(2);
    const freeMB = (info.freeBytes / 1024 / 1024).toFixed(2);
    const usedPercent = ((info.usedBytes / info.totalBytes) * 100).toFixed(1);
    const intervalMin = Math.round(info.intervalMs / 60000);
    
    contentDiv.innerHTML = `
      <div class="storage-stat">
        <span class="storage-stat-label">📊 Espace total:</span>
        <span class="storage-stat-value">${totalMB} MB</span>
      </div>
      
      <div class="storage-stat">
        <span class="storage-stat-label">📈 Espace utilisé:</span>
        <span class="storage-stat-value">${usedMB} MB (${usedPercent}%)</span>
      </div>
      
      <div class="storage-stat">
        <span class="storage-stat-label">💾 Espace libre:</span>
        <span class="storage-stat-value">${freeMB} MB</span>
      </div>
      
      <hr style="margin: 15px 0; border-color: var(--border-color);">
      
      <div class="storage-stat">
        <span class="storage-stat-label">📅 Jours archivés:</span>
        <span class="storage-stat-value">${info.currentDays} jours</span>
      </div>
      
      <div class="storage-stat">
        <span class="storage-stat-label">📊 Capacité maximum:</span>
        <span class="storage-stat-value">~${info.maxDays} jours</span>
      </div>
      
      <div class="storage-stat">
        <span class="storage-stat-label">⏱️ Intervalle de mesure:</span>
        <span class="storage-stat-value">${intervalMin} minutes</span>
      </div>
      
      <div class="storage-stat">
        <span class="storage-stat-label">📏 Points par jour:</span>
        <span class="storage-stat-value">${info.pointsPerDay} points</span>
      </div>
      
      <div class="storage-stat">
        <span class="storage-stat-label">📊 Points actuels:</span>
        <span class="storage-stat-value">${info.currentPoints} points</span>
      </div>
    `;
    
  } catch (error) {
    console.error('❌ Erreur chargement infos:', error);
    contentDiv.innerHTML = '<p>❌ Erreur lors du chargement des informations</p>';
  }
}

// ============================================================================
// GRAPHIQUE JOURNALIER
// ============================================================================

function exportChartDataCSV() {
  let csv = "Timestamp,WaterTemp,Pressure,CoverOpen,Electrovalve,HeatPump,ActiveTimers\n";
  
  for (let i = 0; i < chartData.labels.length; i++) {
    csv += `${chartData.labels[i]},`;
    csv += `${chartData.waterTemp[i]},`;
    csv += `${chartData.pressure[i] / 10},`; // Reconvertir en bar
    csv += `${chartData.coverOpen[i]},`;
    csv += `${chartData.electrovalve[i]},`;
    csv += `${chartData.heatPump[i]},`;
    csv += `${chartData.activeTimers[i]}\n`;
  }
  
  // Télécharger
  const blob = new Blob([csv], { type: 'text/csv' });
  const url = window.URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `pool_data_${new Date().toISOString().split('T')[0]}.csv`;
  a.click();
  window.URL.revokeObjectURL(url);
  
  alert('✅ ' + (t('csv_exported') || 'Données exportées en CSV'));
}

// 2. COMPARAISON AVEC PÉRIODE PRÉCÉDENTE
let comparisonData = null;

function loadComparisonData(daysAgo) {
  // Charger données d'il y a X jours depuis localStorage
  const key = `poolChartData_${daysAgo}`;
  const saved = localStorage.getItem(key);
  
  if (saved) {
    comparisonData = JSON.parse(saved);
    
    // Ajouter dataset de comparaison au graphique
    if (dailyChart) {
      dailyChart.data.datasets.push({
        label: `Température (il y a ${daysAgo}j)`,
        data: comparisonData.waterTemp,
        borderColor: '#95a5a6',
        borderWidth: 2,
        borderDash: [10, 5],
        tension: 0.4,
        yAxisID: 'y-left',
        fill: false
      });
      dailyChart.update();
    }
    
    alert('✅ ' + (t('comparison_enabled') || 'Comparaison activée'));
  } else {
    alert('⚠️ Pas de données disponibles pour cette période');
  }
}

function clearComparison() {
  if (dailyChart && comparisonData) {
    // Retirer dernier dataset (comparaison)
    dailyChart.data.datasets.pop();
    dailyChart.update();
    comparisonData = null;
    alert('✅ ' + (t('comparison_disabled') || 'Comparaison désactivée'));
  }
}

// 3. ANNOTATIONS D'ÉVÉNEMENTS
const chartEvents = [];

function addChartEvent(label, color = '#e74c3c') {
  const now = new Date();
  const timeLabel = now.getHours().toString().padStart(2, '0') + ':' + 
                   now.getMinutes().toString().padStart(2, '0');
  
  chartEvents.push({
    time: timeLabel,
    label: label,
    color: color
  });
  
  // Ajouter annotation au graphique
  if (dailyChart) {
    const annotation = {
      type: 'line',
      mode: 'vertical',
      scaleID: 'x',
      value: timeLabel,
      borderColor: color,
      borderWidth: 2,
      label: {
        enabled: true,
        content: label,
        position: 'top'
      }
    };
    
    if (!dailyChart.options.plugins.annotation) {
      dailyChart.options.plugins.annotation = { annotations: [] };
    }
    
    dailyChart.options.plugins.annotation.annotations.push(annotation);
    dailyChart.update();
  }
  
  // Sauvegarder
  localStorage.setItem('chartEvents', JSON.stringify(chartEvents));
}

/*function loadChartEvents() {
  const saved = localStorage.getItem('chartEvents');
  if (saved) {
    const events = JSON.parse(saved);
    events.forEach(event => {
      // Réappliquer annotations
      if (dailyChart && chartData.labels.includes(event.time)) {
        addChartEvent(event.label, event.color);
      }
    });
  }
}*/

// 4. MOYENNES MOBILES
function addMovingAverage(periodMinutes = 30) {
  // Calculer moyenne mobile sur température
  const period = Math.floor(periodMinutes / chartUpdateInterval * 60000);
  const movingAvg = [];
  
  for (let i = 0; i < chartData.waterTemp.length; i++) {
    let sum = 0;
    let count = 0;
    
    for (let j = Math.max(0, i - period); j <= i; j++) {
      sum += chartData.waterTemp[j];
      count++;
    }
    
    movingAvg.push(sum / count);
  }
  
  // Ajouter au graphique
  if (dailyChart) {
    dailyChart.data.datasets.push({
      label: `Température (MA ${periodMinutes}min)`,
      data: movingAvg,
      borderColor: '#f39c12',
      backgroundColor: 'rgba(243, 156, 18, 0.1)',
      borderWidth: 3,
      tension: 0.4,
      yAxisID: 'y-left',
      fill: true
    });
    dailyChart.update();
  }
  
  alert('✅ ' + (t('moving_average_added') || 'Moyenne mobile ajoutée'));
}

// 5. ZOOM SUR INTERVALLE
function zoomToTimeRange(startHour, endHour) {
  if (!dailyChart) return;
  
  const filteredData = {
    labels: [],
    waterTemp: [],
    pressure: [],
    pump: [],           // ✅ AJOUTÉ
    electro: [],        // ✅ AJOUTÉ
    light: [],          // ✅ AJOUTÉ
    coverOpen: [],
    electrovalve: [],
    heatPump: [],
    activeTimers: []
  };
  
  chartData.labels.forEach((label, i) => {
    const hour = parseInt(label.split(':')[0]);
    
    if (hour >= startHour && hour <= endHour) {
      filteredData.labels.push(label);
      filteredData.waterTemp.push(chartData.waterTemp[i]);
      filteredData.pressure.push(chartData.pressure[i]);
      filteredData.pump.push(chartData.pump[i]);              // ✅ AJOUTÉ
      filteredData.electro.push(chartData.electro[i]);        // ✅ AJOUTÉ
      filteredData.light.push(chartData.light[i]);            // ✅ AJOUTÉ
      filteredData.coverOpen.push(chartData.coverOpen[i]);
      filteredData.electrovalve.push(chartData.electrovalve[i]);
      filteredData.heatPump.push(chartData.heatPump[i]);
      filteredData.activeTimers.push(chartData.activeTimers[i]);
    }
  });
  
  // Mettre à jour graphique
  dailyChart.data.labels = filteredData.labels;
  dailyChart.data.datasets[0].data = filteredData.waterTemp;
  dailyChart.data.datasets[1].data = filteredData.pressure;
  dailyChart.data.datasets[2].data = filteredData.coverOpen;
  dailyChart.data.datasets[3].data = filteredData.electrovalve;
  dailyChart.data.datasets[4].data = filteredData.heatPump;
  dailyChart.data.datasets[5].data = filteredData.activeTimers;
  dailyChart.data.datasets[6].data = filteredData.pump;        // ✅ AJOUTÉ
  dailyChart.data.datasets[7].data = filteredData.electro;     // ✅ AJOUTÉ
  dailyChart.data.datasets[8].data = filteredData.light;       // ✅ AJOUTÉ
  dailyChart.update();
  
  alert('✅ ' + (t('zoom_applied') || 'Zoom appliqué'));
}

function resetZoom() {
  if (!dailyChart) return;
  
  // Restaurer données complètes
  dailyChart.data.labels = chartData.labels;
  dailyChart.data.datasets[0].data = chartData.waterTemp;
  dailyChart.data.datasets[1].data = chartData.pressure;
  dailyChart.data.datasets[2].data = chartData.coverOpen;
  dailyChart.data.datasets[3].data = chartData.electrovalve;
  dailyChart.data.datasets[4].data = chartData.heatPump;
  dailyChart.data.datasets[5].data = chartData.activeTimers;
  dailyChart.data.datasets[6].data = chartData.pump;        // ✅ AJOUTÉ
  dailyChart.data.datasets[7].data = chartData.electro;     // ✅ AJOUTÉ
  dailyChart.data.datasets[8].data = chartData.light;       // ✅ AJOUTÉ
  dailyChart.update();
  
  alert('✅ ' + (t('full_view_restored') || 'Vue complète restaurée'));
}

// 6. STATISTIQUES INSTANTANÉES
function calculateChartStats() {
  if (chartData.waterTemp.length === 0) {
    alert('⚠️ Pas de données disponibles');
    return;
  }
  
  // Température
  const temps = chartData.waterTemp.filter(t => t > 0);
  const avgTemp = temps.reduce((a, b) => a + b, 0) / temps.length;
  const minTemp = Math.min(...temps);
  const maxTemp = Math.max(...temps);
  const tempRange = maxTemp - minTemp;
  
  // Pression
  const pressures = chartData.pressure.filter(p => p > 0).map(p => p / 10);
  const avgPress = pressures.reduce((a, b) => a + b, 0) / pressures.length;
  const minPress = Math.min(...pressures);
  const maxPress = Math.max(...pressures);
  
  // Équipements
  const coverOpenTime = chartData.coverOpen.filter(c => c === 1).length;
  const coverOpenPercent = (coverOpenTime / chartData.coverOpen.length * 100).toFixed(1);
  
  const heatPumpTime = chartData.heatPump.filter(h => h === 1).length;
  const heatPumpHours = (heatPumpTime * chartUpdateInterval / 3600000).toFixed(1);
  
  // Afficher
  alert(`📊 STATISTIQUES SUR 24H

🌡️ TEMPÉRATURE
  • Moyenne: ${avgTemp.toFixed(1)}°C
  • Min/Max: ${minTemp.toFixed(1)}°C / ${maxTemp.toFixed(1)}°C
  • Amplitude: ${tempRange.toFixed(1)}°C

💪 PRESSION
  • Moyenne: ${avgPress.toFixed(2)} bar
  • Min/Max: ${minPress.toFixed(2)} / ${maxPress.toFixed(2)} bar

⚙️ ÉQUIPEMENTS
  • Volet ouvert: ${coverOpenPercent}% du temps
  • PAC active: ${heatPumpHours}h
  • Points de données: ${chartData.labels.length}`);
}

// 7. PRÉDICTION SIMPLE (TENDANCE LINÉAIRE)
function predictTemperature(hoursAhead = 2) {
  if (chartData.waterTemp.length < 10) {
    alert('⚠️ Pas assez de données pour prédiction');
    return;
  }
  
  // Régression linéaire simple sur derniers points
  const n = Math.min(20, chartData.waterTemp.length);
  const recentTemps = chartData.waterTemp.slice(-n);
  
  let sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
  
  for (let i = 0; i < n; i++) {
    sumX += i;
    sumY += recentTemps[i];
    sumXY += i * recentTemps[i];
    sumX2 += i * i;
  }
  
  const slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
  const intercept = (sumY - slope * sumX) / n;
  
  const pointsAhead = Math.floor(hoursAhead * 60 / (chartUpdateInterval / 60000));
  const prediction = slope * (n + pointsAhead) + intercept;
  
  const trend = slope > 0.1 ? '📈 Hausse' : slope < -0.1 ? '📉 Baisse' : '➡️ Stable';
  
  alert(`🔮 PRÉDICTION TEMPÉRATURE

Dans ${hoursAhead}h: ${prediction.toFixed(1)}°C
Tendance: ${trend}
Variation: ${(slope * pointsAhead).toFixed(2)}°C/h

⚠️ Prédiction basée sur tendance linéaire simple`);
}

function initChart() {
  const ctx = document.getElementById('dailyChart').getContext('2d');
  if (!ctx) return;
  
  // ✨ CONVERSION DES DONNÉES SELON LES UNITÉS PRÉFÉRÉES
  const convertedTempData = chartData.waterTemp.map(temp => {
    if (userUnits.temperature === UNITS.TEMPERATURE.FAHRENHEIT) {
      return celsiusToFahrenheit(temp);
    }
    return temp;
  });
  
  const convertedPressureData = chartData.pressure.map(press => {
    const barValue = press / 10; // Déjà multiplié par 10 lors du stockage
    if (userUnits.pressure === UNITS.PRESSURE.PSI) {
      return barToPsi(barValue);
    }
    return press; // Garder *10 pour BAR (pour lisibilité graphique)
  });
  
  dailyChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: chartData.labels,
      datasets: [
        {
          label: (t('chart_water_temp') || 'Température Eau') + ' (' + getTemperatureUnitLabel() + ')',
          data: convertedTempData,  // ✅ CONVERTI
          borderColor: '#3498db',
          backgroundColor: 'rgba(52, 152, 219, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          yAxisID: 'y-left',
          fill: false
        },
        {
          label: (t('chart_pressure') || 'Pression') + ' (' + getPressureUnitLabel() + (userUnits.pressure === UNITS.PRESSURE.BAR ? ' x10' : '') + ')',
          data: convertedPressureData,  // ✅ CONVERTI
          borderColor: '#27ae60',
          backgroundColor: 'rgba(39, 174, 96, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          yAxisID: 'y-left',
          fill: false
        },
        {
          label: t('chart_cover') || 'Volet Ouvert',
          data: chartData.coverOpen,
          borderColor: '#e67e22',
          backgroundColor: 'rgba(230, 126, 34, 0.1)',
          borderWidth: 2,
          borderDash: [5, 5],
          stepped: true,
          yAxisID: 'y-right',
          fill: false
        },
        {
          label: t('chart_valve') || 'Électrovalve',
          data: chartData.electrovalve,
          borderColor: '#00bcd4',
          backgroundColor: 'rgba(0, 188, 212, 0.1)',
          borderWidth: 2,
          borderDash: [5, 5],
          stepped: true,
          yAxisID: 'y-right',
          fill: false
        },
        {
          label: t('chart_heatpump') || 'PAC Active',
          data: chartData.heatPump,
          borderColor: '#e74c3c',
          backgroundColor: 'rgba(231, 76, 60, 0.2)',
          borderWidth: 2,
		  borderDash: [5, 5],
          stepped: true,
          yAxisID: 'y-right',
          fill: false
        },
        {
          label: t('chart_active_timers') || 'Timers Actifs',
          data: chartData.activeTimers,
          borderColor: '#9b59b6',
          backgroundColor: 'rgba(155, 89, 182, 0.3)',
          borderWidth: 2,
          stepped: true,
          yAxisID: 'y-right',
          fill: true
        },

        {
          label: t('chart_pump') || 'Pompe',
          data: chartData.pump,
          borderColor: '#3498db',
          backgroundColor: 'rgba(52, 152, 219, 0.3)',
          borderWidth: 2,
          borderDash: [3, 3],
          stepped: true,
          yAxisID: 'y-right',
          fill: false
        },
        {
          label: t('chart_electro') || 'Électrolyseur',
          data: chartData.electro,
          borderColor: '#f39c12',
          backgroundColor: 'rgba(243, 156, 18, 0.3)',
          borderWidth: 2,
          borderDash: [3, 3],
          stepped: true,
          yAxisID: 'y-right',
          fill: false
        },
        {
          label: t('chart_light') || 'Éclairage',
          data: chartData.light,
          borderColor: '#f1c40f',
          backgroundColor: 'rgba(241, 196, 15, 0.3)',
          borderWidth: 2,
          borderDash: [3, 3],
          stepped: true,
          yAxisID: 'y-right',
          fill: false
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: {
        mode: 'index',
        intersect: false
      },
      plugins: {
        legend: {
          display: true,
          position: 'bottom',
          labels: {
            usePointStyle: true,
            padding: 15,
            font: {
              size: 11
            }
          }
        },
        tooltip: {
          mode: 'index',
          intersect: false,
          backgroundColor: 'rgba(0, 0, 0, 0.8)',
          padding: 12,
          callbacks: {
            // ✨ TOOLTIPS AVEC CONVERSION CORRECTE
            label: function(context) {
              let label = context.dataset.label || '';
              
              if (label) {
                label += ': ';
              }
              
              // Température (dataset 0)
              if (context.datasetIndex === 0) {
                const rawTemp = chartData.waterTemp[context.dataIndex];
                if (userUnits.temperature === UNITS.TEMPERATURE.FAHRENHEIT) {
                  label += celsiusToFahrenheit(rawTemp).toFixed(1) + '°F';
                } else {
                  label += rawTemp.toFixed(1) + '°C';
                }
              }
              // Pression (dataset 1)
              else if (context.datasetIndex === 1) {
                const rawPress = chartData.pressure[context.dataIndex]; // En BAR
                if (userUnits.pressure === UNITS.PRESSURE.PSI) {
                  label += barToPsi(rawPress).toFixed(2) + ' PSI';
                } else {
                  label += rawPress.toFixed(2) + ' bar';
                }
              }
              // États binaires (datasets 2, 3, 4, 5)
              else {
                label += context.parsed.y === 1 ? 'ON' : 'OFF';
              }
              
              return label;
            }
          }
        }
      },
      scales: {
        x: {
          display: true,
          title: {
            display: true,
            text: t('time') || 'Heure'
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          }
        },
        'y-left': {
          type: 'linear',
          display: true,
          position: 'left',
          min: 0,
          max: (function() {
            // 🎯 CALCUL DYNAMIQUE DU MAX SELON L'UNITÉ
            if (userUnits.temperature === UNITS.TEMPERATURE.FAHRENHEIT) {
              // 50°C = 122°F
              return 122;
            }
            if (userUnits.pressure === UNITS.PRESSURE.PSI) {
              // 5 BAR = 72.5 PSI (arrondi à 73)
              return 73;
            }
            // Par défaut : 50 pour °C et BAR (×10)
            return 50;
          })(),
          ticks: {
            callback: function(value) {
              return value;
            }
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          },
          title: {
            display: true,
            text: 'Temp (' + getTemperatureUnitLabel() + ') / Pression (' + getPressureUnitLabel() + (userUnits.pressure === UNITS.PRESSURE.BAR ? ' x10' : '') + ')'
          }
        },
        'y-right': {
          type: 'linear',
          display: true,
          position: 'right',
          min: 0,
          max: 1,
          ticks: {
            stepSize: 1,
            callback: function(value) {
              return value === 1 ? 'ON' : 'OFF';
            }
          },
          grid: {
            drawOnChartArea: false
          },
          title: {
            display: true,
            text: 'États ON/OFF'
          }
        }
      }
    }
  });
}

function updateChartUnits() {
  if (dailyChart) {
    // Détruire le graphique existant
    dailyChart.destroy();
    // Recréer avec les nouvelles unités
    initChart();
  }
}

// ============================================================================
// FONCTION STUB - Compatibilité avec ancien code
// ============================================================================
// Cette fonction est appelée par dashboard.js et auth.js mais n'est plus utilisée
// Les données viennent maintenant de l'ESP32 via loadToday()

async function addChartDataPoint() {
  // ✅ STUB : Cette fonction ne fait plus rien
  // Les points sont ajoutés automatiquement par l'ESP32 (chart_storage.h)
  // Le graphique est mis à jour via loadToday() toutes les X minutes
  console.log('ℹ️ addChartDataPoint() appelé (fonction stub - données sur ESP32)');
}

// ============================================================================
// FONCTIONS STUBS - Compatibilité avec ancien code
// ============================================================================
// Ces fonctions sont appelées par l'ancien code mais ne sont plus utilisées

function saveChartData() {
  // ✅ STUB : Les données sont maintenant sauvegardées sur l'ESP32
  console.log('ℹ️ saveChartData() appelé (fonction stub - sauvegarde ESP32)');
}

function cleanOldChartData() {
  // ✅ STUB : Le nettoyage est géré automatiquement par l'ESP32
  console.log('ℹ️ cleanOldChartData() appelé (fonction stub - nettoyage ESP32)');
}

// ============================================================================
// SYSTÈME ACTUEL - Chargement depuis ESP32
// ============================================================================

function loadChartData() {
console.log('📊 Initialisation du graphique avec données ESP32');
  
  // Initialiser le sélecteur de date avec aujourd'hui
  const today = new Date().toISOString().split('T')[0];
  const dateSelector = document.getElementById('chart-date-selector');
  
  if (dateSelector) {
    dateSelector.value = today;
    dateSelector.max = today;  // Ne pas permettre de sélectionner le futur
  }
  
  // Charger les données d'aujourd'hui
  loadToday();
  
  // ✅ Démarrer le rafraîchissement automatique
  startChartInterval();
}

function resetChartData() {
  console.log('🔄 Reset des données du graphique en mémoire');
  chartData = {
    labels: [],
    waterTemp: [],
    pressure: [],
    pump: [],
    electro: [],
    light: [],
    coverOpen: [],
    electrovalve: [],
    heatPump: [],
    activeTimers: []
  };
  // ✅ Les données sont maintenant sur l'ESP32, pas besoin de saveChartData()
}

function refreshChart() {
  console.log('🔄 Actualisation manuelle du graphique...');
  loadToday(); // ✅ Recharger depuis l'ESP32
  alert('✅ ' + (t('chart_refreshed') || 'Graphique actualisé depuis ESP32'));
}

// ============================================================================
// GESTION INTERVALLE GRAPHIQUE
// ============================================================================

function startChartInterval() {
  // Arrêter l'ancien intervalle s'il existe
  if (chartIntervalTimer) clearInterval(chartIntervalTimer);
  
  // ✅ NOUVEAU : Recharger depuis l'ESP32 au lieu de fetch /api/sensors
  chartIntervalTimer = setInterval(() => {
    console.log('🔄 Rechargement automatique depuis ESP32...');
    loadToday(); // Recharge les données du jour actuel depuis l'ESP32
  }, chartUpdateInterval);
  
  console.log(`📊 Graphique configuré: rechargement toutes les ${chartUpdateInterval / 60000} minutes depuis ESP32`);
}

function updateChartInterval(newInterval) {
  chartUpdateInterval = newInterval;
  localStorage.setItem('chartInterval', newInterval);
  savePreferences();
  
  // Sauvegarder aussi sur l'ESP32
  fetch('/api/chart/config', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({interval: newInterval})
  }).catch(err => console.error('Erreur sauvegarde intervalle:', err));
  
  // Redémarrer avec le nouvel intervalle
  startChartInterval();
  
  // Mettre à jour l'affichage
  updateChartIntervalDisplay();
  
  alert('✅ ' + (t('chart_interval_changed') || 'Intervalle changé à') + ' ' + (newInterval / 60000) + ' ' + (t('minutes') || 'minute(s)'));
}

function updateChartIntervalDisplay() {
  const minutes = chartUpdateInterval / 60000;
  const display = document.getElementById('current-chart-interval');
  if (display) {
    display.textContent = `${minutes} minute${minutes > 1 ? 's' : ''}`;
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolChart = {
  // Variables
  get dailyChart() { return dailyChart; },
  set dailyChart(value) { dailyChart = value; },
  get chartUpdateInterval() { return chartUpdateInterval; },
  set chartUpdateInterval(value) { chartUpdateInterval = value; },
  get chartIntervalTimer() { return chartIntervalTimer; },
  set chartIntervalTimer(value) { chartIntervalTimer = value; },
  get chartData() { return chartData; },
  set chartData(value) { chartData = value; },
  
  
  // Fonctions principales
  initChart,
  addChartDataPoint,
  exportChartDataCSV,
  loadComparisonData,
  addMovingAverage,
  clearComparison,
  addChartEvent,
  zoomToTimeRange,
  resetZoom,
  calculateChartStats,
  updateChartUnits,
  predictTemperature,
  loadChartData,
  saveChartData,
  resetChartData,
  refreshChart,
  
  // Nouvelles fonctions pour l'historique
  displayDateData,
  navigateDayOffset,
  loadToday,
  onDateSelected,
  exportCurrentDayCSV,
  showAvailableDates,
  showStorageInfo,
  loadChartDataFromESP32,
  convertESP32DataToChart,
  
  // Intervalles
  startChartInterval,
  updateChartInterval,
  updateChartIntervalDisplay,
  updateChartDisplay,
};

