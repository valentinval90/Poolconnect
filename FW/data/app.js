// Pool Connect Pro v4.0 - JavaScript Complet et Corrigé
// Gestion de l'interface web moderne avec onglets

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

let currentUser = null;
let currentRole = null;
let updateInterval = null;
let timers = [];
let lastActiveTimerCount = 0;
let chartUpdateInterval = parseInt(localStorage.getItem('chartInterval')) || 300000; // 5 min par défaut
let chartIntervalTimer = null;

// ============================================================================
// THÈME 
// ============================================================================

function initTheme() {
  const savedTheme = localStorage.getItem('theme') || 'light';
  document.documentElement.setAttribute('data-theme', savedTheme);
  updateThemeButton();
}

function toggleTheme() {
  const current = document.documentElement.getAttribute('data-theme') || 'light';
  const newTheme = current === 'light' ? 'dark' : 'light';
  document.documentElement.setAttribute('data-theme', newTheme);
  localStorage.setItem('theme', newTheme);
  updateThemeButton();
}

function updateThemeButton() {
  const btn = document.getElementById('themeToggle');
  if (btn) {
    const theme = document.documentElement.getAttribute('data-theme');
    btn.textContent = theme === 'dark' ? '☀️' : '🌙';
  }
}

// ============================================================================
// BACKUP/RESTORE
// ============================================================================

async function downloadBackup() {
  try {
    const response = await fetch('/api/backup/download');
    const json = await response.text();
    
    // Télécharger
    const blob = new Blob([json], { type: 'application/json' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    
    const date = new Date().toISOString().split('T')[0];
    a.download = `pool_backup_${date}.json`;
    a.click();
    window.URL.revokeObjectURL(url);
    
    alert('✅ Backup téléchargé');
  } catch (error) {
    console.error('Backup error:', error);
    alert('❌ Erreur téléchargement backup');
  }
}

async function uploadBackup() {
  const input = document.createElement('input');
  input.type = 'file';
  input.accept = '.json';
  
  input.onchange = async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    
    const reader = new FileReader();
    reader.onload = async (event) => {
      const json = event.target.result;
      
      if (!confirm('⚠️ ATTENTION\n\nRestaurer ce backup remplacera toute la configuration actuelle.\n\nContinuer ?')) {
        return;
      }
      
      try {
        const response = await fetch('/api/backup/upload', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: json
        });
        
        if (response.ok) {
          alert('✅ Backup restauré !\n\nLe système va redémarrer dans 5 secondes...');
          setTimeout(() => {
            window.location.reload();
          }, 5000);
        } else {
          const error = await response.text();
          alert('❌ Erreur: ' + error);
        }
      } catch (error) {
        console.error('Restore error:', error);
        alert('❌ Erreur restauration');
      }
    };
    
    reader.readAsText(file);
  };
  
  input.click();
}

async function saveBackupToESP() {
  if (!confirm('Créer un backup sur l\'ESP32 ?')) return;
  
  try {
    const response = await fetch('/api/backup/save', { method: 'POST' });
    const filename = await response.text();
    
    if (response.ok) {
      alert(`✅ Backup sauvegardé: ${filename}`);
    } else {
      alert('❌ Erreur sauvegarde');
    }
  } catch (error) {
    console.error('Save error:', error);
    alert('❌ Erreur');
  }
}

// ============================================================================
// SCÉNARIOS 
// ============================================================================

async function showScenariosModal() {
  try {
    const response = await fetch('/api/scenarios');
    const scenarios = await response.json();
    
    let html = '<div class="modal active" id="scenariosModal">';
    html += '<div class="modal-content">';
    html += '<div class="modal-header">';
    html += '<h2>🎯 Scénarios Pré-configurés</h2>';
    html += '<button class="close-btn" onclick="closeScenariosModal()">✕</button>';
    html += '</div>';
    html += '<div class="modal-body">';
    html += '<p class="info-text">Appliquez un scénario type pour démarrer rapidement</p>';
    html += '<div class="action-types-grid">';
    
    scenarios.forEach(scenario => {
      html += '<div class="action-type-card" onclick="applyScenario(' + scenario.id + ')">';
      html += '<div class="action-type-name">' + scenario.name + '</div>';
      html += '<div class="action-type-desc">' + scenario.description + '</div>';
      html += '</div>';
    });
    
    html += '</div></div></div></div>';
    
    document.body.insertAdjacentHTML('beforeend', html);
    
  } catch (error) {
    console.error('Scenarios error:', error);
    alert('❌ Erreur chargement scénarios');
  }
}

function closeScenariosModal() {
  const modal = document.getElementById('scenariosModal');
  if (modal) modal.remove();
}

async function applyScenario(scenarioId) {
  if (!confirm('Créer un timer depuis ce scénario ?')) return;
  
  try {
    const response = await fetch('/api/scenarios/apply', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ scenarioId })
    });
    
    if (response.ok) {
      alert('✅ Scénario appliqué !\n\nConsultez l\'onglet Timers.');
      closeScenariosModal();
      showTab('timers');
    } else {
      const error = await response.text();
      alert('❌ ' + error);
    }
  } catch (error) {
    console.error('Apply scenario error:', error);
    alert('❌ Erreur application scénario');
  }
}

// ============================================================================
// AUTHENTIFICATION
// ============================================================================

async function login(username, password) {
  try {
    const response = await fetch('/api/auth', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({username, password})
    });
    
    if (response.ok) {
      const data = await response.json();
      currentUser = data.username;
      currentRole = data.role;
      sessionStorage.setItem('poolUser', currentUser);
      sessionStorage.setItem('poolRole', currentRole);
      showApp();
    } else {
      showLoginError();
    }
  } catch (error) {
    console.error('Login error:', error);
    showLoginError();
  }
}

function showLoginError() {
  const errorDiv = document.getElementById('loginError');
  errorDiv.classList.add('show');
  setTimeout(() => errorDiv.classList.remove('show'), 3000);
}

function logout() {
  if (confirm('Êtes-vous sûr de vouloir vous déconnecter ?')) {
    currentUser = null;
    currentRole = null;
    sessionStorage.removeItem('poolUser');
    sessionStorage.removeItem('poolRole');
    if (updateInterval) clearInterval(updateInterval);
    showLogin();
  }
}

function checkAuth() {
  const user = sessionStorage.getItem('poolUser');
  const role = sessionStorage.getItem('poolRole');
  
  if (user && role) {
    currentUser = user;
    currentRole = role;
    showApp();
  } else {
    showLogin();
  }
}

function showLogin() {
  document.getElementById('loginContainer').classList.remove('hidden');
  document.getElementById('appContainer').classList.remove('active');
  document.getElementById('username').value = '';
  document.getElementById('loginPassword').value = '';
}

function showApp() {
  document.getElementById('loginContainer').classList.add('hidden');
  document.getElementById('appContainer').classList.add('active');
  
  const usernameDisplay = document.querySelector('.logo small');
  if (usernameDisplay) {
    usernameDisplay.textContent = `Connecté: ${currentUser} (${currentRole})`;
  }
  
  if (currentRole === 'admin') {
    document.getElementById('usersTab').style.display = 'block';
  } else {
    document.getElementById('usersTab').style.display = 'none';
  }
  
  showTab('dashboard');
  
  // Actualiser la sidebar immédiatement
  updateSidebar();
  
  loadChartData();
  setTimeout(() => {
    initChart();
    addChartDataPoint();
  }, 500);
  
  startChartInterval();
  
  if (updateInterval) clearInterval(updateInterval);
  updateInterval = setInterval(() => {
	updateSidebar(); // Actualiser sidebar toutes les 2 secondes
    const activeTab = document.querySelector('.tab-content.active').id;
    if (activeTab === 'tab-dashboard') updateDashboard();
    if (activeTab === 'tab-control') updateControl();
  }, 2000);
}

// ============================================================================
// GESTION INTERVALLE GRAPHIQUE
// ============================================================================

function startChartInterval() {
  // Arrêter l'ancien intervalle s'il existe
  if (chartIntervalTimer) clearInterval(chartIntervalTimer);
  
  // Créer le nouvel intervalle
  chartIntervalTimer = setInterval(() => {
    addChartDataPoint();
  }, chartUpdateInterval);
  
  console.log(`📊 Graphique configuré: mise à jour toutes les ${chartUpdateInterval / 60000} minutes`);
}

function updateChartInterval(newInterval) {
  chartUpdateInterval = newInterval;
  localStorage.setItem('chartInterval', newInterval);
  
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
  
  alert(`✅ Intervalle graphique changé à ${newInterval / 60000} minute(s)`);
}

function updateChartIntervalDisplay() {
  const minutes = chartUpdateInterval / 60000;
  const display = document.getElementById('current-chart-interval');
  if (display) {
    display.textContent = `${minutes} minute${minutes > 1 ? 's' : ''}`;
  }
}

// ============================================================================
// SIDEBAR UPDATE
// ============================================================================

async function updateSidebar() {
  try {
    // Mettre à jour l'heure/date
    const time = await fetch('/api/time').then(r => r.text());
    document.getElementById('datetime').textContent = '🕒 ' + time;
    
    // Mettre à jour la température de l'eau
    const temp = await fetch('/api/temp').then(r => r.text());
    if (temp !== 'ERREUR') {
      document.getElementById('temperature').textContent = temp + '°C';
      
      // Calculer temps de filtration recommandé (Temp / 2)
      const tempValue = parseFloat(temp);
      if (!isNaN(tempValue)) {
        const heuresRecommandees = Math.round(tempValue / 2);
        document.getElementById('tempsFiltrationRecommande').textContent = heuresRecommandees + 'h';
      }
    } else {
      document.getElementById('temperature').textContent = 'ERR';
    }
    
    // Calculer temps de filtration actuel (pompe ON)
    const relays = await fetch('/api/relays').then(r => r.json());
    const pompeActive = relays[0]; // Relais 0 = pompe
    
    if (pompeActive) {
      // TODO: Tracker le temps réel - pour l'instant on affiche juste "EN COURS"
      document.getElementById('tempsFiltrationActuel').textContent = 'En cours';
    } else {
      document.getElementById('tempsFiltrationActuel').textContent = 'Arrêté';
    }
    
  } catch (error) {
    console.error('Sidebar update error:', error);
  }
}

// ============================================================================
// NAVIGATION
// ============================================================================

function showTab(tabName) {
  document.querySelectorAll('.tab-content').forEach(tab => {
    tab.classList.remove('active');
  });
  
  document.querySelectorAll('.nav-tab').forEach(btn => {
    btn.classList.remove('active');
  });
  
  document.getElementById('tab-' + tabName).classList.add('active');
  
  document.querySelectorAll('.nav-tab').forEach(btn => {
    if (btn.onclick && btn.onclick.toString().includes(tabName)) {
      btn.classList.add('active');
    }
  });
  
  if (tabName === 'dashboard') updateDashboard();
  if (tabName === 'control') updateControl();
  if (tabName === 'timers') loadFlexibleTimers();
  if (tabName === 'calibration') loadCalibrationTab();
  if (tabName === 'settings') loadSettings();
  if (tabName === 'users' && currentRole === 'admin') loadUsers();
}

// ============================================================================
// DASHBOARD
// ============================================================================

async function updateDashboard() {
  try {
    const temp = await fetch('/api/temp').then(r => r.text());
    document.getElementById('dash-water-temp').textContent = temp === 'ERREUR' ? 'ERR' : temp + '°C';
    
    const sensors = await fetch('/api/sensors').then(r => r.json());
    document.getElementById('dash-pressure').textContent = sensors.waterPressure.toFixed(2) + ' bar';
    document.getElementById('dash-ext-temp').textContent = sensors.extTemp ? sensors.extTemp.toFixed(1) + '°C' : '--';
    
    const leakCard = document.getElementById('leak-card');
    const leakValue = document.getElementById('dash-leak');
    if (sensors.waterLeak) {
      leakValue.textContent = '⚠️ FUITE';
      leakCard.style.background = 'linear-gradient(135deg, #e74c3c, #c0392b)';
      leakCard.style.color = 'white';
    } else {
      leakValue.textContent = '✓ OK';
      leakCard.style.background = '';
      leakCard.style.color = '';
    }
    
    const relays = await fetch('/api/relays').then(r => r.json());
    const equipmentNames = ['pompe', 'electro', 'lampe', 'valve', 'pac'];
    const equipmentLabels = ['En marche', 'Actif', 'Allumée', 'Ouverte', 'En marche'];
    const equipmentLabelsOff = ['Arrêtée', 'Arrêté', 'Éteinte', 'Fermée', 'Arrêtée'];
    
    relays.forEach((state, i) => {
      const el = document.getElementById('eq-' + equipmentNames[i]);
      if (el) {
        el.textContent = state ? equipmentLabels[i] : equipmentLabelsOff[i];
        el.className = 'equipment-state' + (state ? ' active' : '');
      }
    });
    
    // FIX: Utiliser /api/timers/flex au lieu de /api/timers
    const timersData = await fetch('/api/timers/flex').then(r => r.json()).catch(() => []);
    const activeTimers = timersData.filter(t => t.context && t.context.state === 2); // TIMER_RUNNING = 2
    document.getElementById('active-timers-count').textContent = activeTimers.length;
    
    if (activeTimers.length !== lastActiveTimerCount) {
      lastActiveTimerCount = activeTimers.length;
      if (typeof addChartDataPoint === 'function') {
        addChartDataPoint();
      }
    }
    
    if (activeTimers.length > 0) {
      let html = '';
      activeTimers.forEach(timer => {
        html += `<div class="timer-item active">
          <div class="timer-info">
            <div class="timer-name">${timer.name}</div>
            <div class="timer-details">Action ${timer.context.currentAction + 1}/${timer.actionCount}</div>
          </div>
        </div>`;
      });
      document.getElementById('active-timers-list').innerHTML = html;
    } else {
      document.getElementById('active-timers-list').innerHTML = 
        '<p style="text-align: center; color: #999; padding: 20px;">Aucun timer actif</p>';
    }
    
    loadHistoryDashboard();
    
  } catch (error) {
    console.error('Dashboard update error:', error);
  }
}

async function loadHistoryDashboard() {
  try {
    const history = await fetch('/api/history').then(r => r.json());
    const historyEl = document.getElementById('dash-history');
    
    if (!history || history.length === 0) {
      historyEl.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">Aucun historique</p>';
      return;
    }
    
    let html = '';
    history.slice(0, 5).forEach(entry => {
      const durationH = Math.floor(entry.duration / 60);
      const durationM = entry.duration % 60;
      html += `<div class="history-item">
        📅 ${entry.date} - ⏰ ${entry.startTime} - 
        ⏱️ ${durationH}h${durationM.toString().padStart(2, '0')} - 
        🌡️ ${entry.avgTemp.toFixed(1)}°C
      </div>`;
    });
    
    historyEl.innerHTML = html;
  } catch (error) {
    console.error('History error:', error);
  }
}

// ============================================================================
// CONTRÔLE
// ============================================================================

async function updateControl() {
  try {
    const relays = await fetch('/api/relays').then(r => r.json());
    relays.forEach((state, i) => {
      const checkbox = document.getElementById('ctrl-relay' + i);
      if (checkbox) checkbox.checked = state;
    });
    
    const sensors = await fetch('/api/sensors').then(r => r.json());
    document.getElementById('ctrl-volet').textContent = sensors.coverOpen ? 'Ouvert' : 'Fermé';
    document.getElementById('ctrl-fuite').textContent = sensors.waterLeak ? '⚠️ FUITE' : '✓ OK';
    
  } catch (error) {
    console.error('Control update error:', error);
  }
}

async function toggleRelay(relay, state) {
  try {
    const response = await fetch(`/api/relay?ch=${relay}&state=${state ? 1 : 0}`);
    if (!response.ok) {
      const error = await response.text();
      if (error.includes('pompe')) {
        alert('⚠️ ' + error);
        const checkbox = document.getElementById('ctrl-relay' + relay);
        if (checkbox) checkbox.checked = false;
      }
    } else {
      addChartDataPoint();
    }
  } catch (error) {
    console.error('Relay toggle error:', error);
  }
}

async function toggleBuzzer(enabled) {
  try {
    await fetch(`/api/buzzer/mute?state=${enabled ? 0 : 1}`);
  } catch (error) {
    console.error('Buzzer toggle error:', error);
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
  
  alert('✅ Données exportées en CSV');
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
    
    alert(`✅ Comparaison avec il y a ${daysAgo} jours activée`);
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
    alert('✅ Comparaison désactivée');
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

function loadChartEvents() {
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
}

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
  
  alert(`✅ Moyenne mobile ${periodMinutes}min ajoutée`);
}

// 5. ZOOM SUR INTERVALLE
function zoomToTimeRange(startHour, endHour) {
  if (!dailyChart) return;
  
  const filteredData = {
    labels: [],
    waterTemp: [],
    pressure: [],
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
  dailyChart.update();
  
  alert(`✅ Zoom sur ${startHour}h-${endHour}h`);
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
  dailyChart.update();
  
  alert('✅ Vue complète restaurée');
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

let dailyChart = null;
let chartData = {
  labels: [],
  waterTemp: [],
  pressure: [],
  coverOpen: [],
  electrovalve: [],
  heatPump: [],
  activeTimers: []
};

function initChart() {
  const ctx = document.getElementById('dailyChart');
  if (!ctx) return;
  
  dailyChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: chartData.labels,
      datasets: [
        {
          label: 'Température Eau (°C)',
          data: chartData.waterTemp,
          borderColor: '#3498db',
          backgroundColor: 'rgba(52, 152, 219, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          yAxisID: 'y-left',
          fill: false
        },
        {
          label: 'Pression (bar x10)',
          data: chartData.pressure,
          borderColor: '#27ae60',
          backgroundColor: 'rgba(39, 174, 96, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          yAxisID: 'y-left',
          fill: false
        },
        {
          label: 'Volet Ouvert',
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
          label: 'Électrovalve',
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
          label: 'PAC Active',
          data: chartData.heatPump,
          borderColor: '#e74c3c',
          backgroundColor: 'rgba(231, 76, 60, 0.2)',
          borderWidth: 2,
          stepped: true,
          yAxisID: 'y-left',
          fill: true
        },
        {
          label: 'Timers Actifs',
          data: chartData.activeTimers,
          borderColor: '#9b59b6',
          backgroundColor: 'rgba(155, 89, 182, 0.3)',
          borderWidth: 2,
          stepped: true,
          yAxisID: 'y-right',
          fill: true
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: {
        mode: 'index',
        intersect: false,
      },
      plugins: {
        legend: {
          display: false
        },
        tooltip: {
          callbacks: {
            label: function(context) {
              let label = context.dataset.label || '';
              if (label) {
                label += ': ';
              }
              if (context.parsed.y !== null) {
                if (label.includes('Pression')) {
                  label += (context.parsed.y / 10).toFixed(2) + ' bar';
                } else if (label.includes('Température')) {
                  label += context.parsed.y.toFixed(1) + '°C';
                } else {
                  label += context.parsed.y ? 'ON' : 'OFF';
                }
              }
              return label;
            }
          }
        }
      },
      scales: {
        x: {
          type: 'category',
          ticks: {
            maxRotation: 45,
            minRotation: 45,
            autoSkip: true,
            maxTicksLimit: 24
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
          max: 50,
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
            text: 'Temp (°C) / Pression x10'
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

async function addChartDataPoint() {
  try {
    const sensors = await fetch('/api/sensors').then(r => r.json());
    const relays = await fetch('/api/relays').then(r => r.json());
    const timersData = await fetch('/api/timers/flex').then(r => r.json()).catch(() => []);
    
    const now = new Date();
    const timeLabel = now.getHours().toString().padStart(2, '0') + ':' + 
                     now.getMinutes().toString().padStart(2, '0');
    
    chartData.labels.push(timeLabel);
    chartData.waterTemp.push(sensors.waterTemp || 0);
    chartData.pressure.push((sensors.waterPressure || 0) * 10);
    chartData.coverOpen.push(sensors.coverOpen ? 1 : 0);
    chartData.electrovalve.push(relays[3] ? 1 : 0);
    chartData.heatPump.push(relays[4] ? 1 : 0);
    
    const activeTimerCount = timersData.filter(t => t.context && t.context.state === 2).length;
    chartData.activeTimers.push(activeTimerCount > 0 ? 1 : 0);
    
    // ✨ NOUVEAU : Calcul dynamique du nombre de points max selon l'intervalle
    const maxPoints = Math.floor(86400000 / chartUpdateInterval); // 24h
    
    if (chartData.labels.length > maxPoints) {
      chartData.labels.shift();
      chartData.waterTemp.shift();
      chartData.pressure.shift();
      chartData.coverOpen.shift();
      chartData.electrovalve.shift();
      chartData.heatPump.shift();
      chartData.activeTimers.shift();
    }
    
    if (dailyChart) {
      dailyChart.update('none');
    }
    
    document.getElementById('chart-points').textContent = chartData.labels.length + ' points';
    
    saveChartData();
    
  } catch (error) {
    console.error('Chart data error:', error);
  }
}

function saveChartData() {
  try {
    // Sauvegarde actuelle
    localStorage.setItem('poolChartData', JSON.stringify(chartData));
    localStorage.setItem('poolChartDate', new Date().toDateString());
    
    // NOUVEAU: Sauvegarder aussi avec timestamp pour historique
    const today = new Date();
    const key = `poolChartData_${today.getFullYear()}${(today.getMonth()+1).toString().padStart(2,'0')}${today.getDate().toString().padStart(2,'0')}`;
    localStorage.setItem(key, JSON.stringify(chartData));
    
    // Nettoyer historique > 30 jours
    cleanOldChartData();
    
  } catch (error) {
    console.error('Save chart error:', error);
  }
}

function cleanOldChartData() {
  const today = new Date();
  const thirtyDaysAgo = new Date(today.getTime() - 30 * 24 * 60 * 60 * 1000);
  
  for (let i = 0; i < localStorage.length; i++) {
    const key = localStorage.key(i);
    
    if (key && key.startsWith('poolChartData_') && key.length === 22) {
      // Extraire date du key
      const dateStr = key.substring(14); // YYYYMMDD
      const year = parseInt(dateStr.substring(0, 4));
      const month = parseInt(dateStr.substring(4, 6)) - 1;
      const day = parseInt(dateStr.substring(6, 8));
      const keyDate = new Date(year, month, day);
      
      if (keyDate < thirtyDaysAgo) {
        localStorage.removeItem(key);
        console.log(`Nettoyage historique: ${key}`);
      }
    }
  }
}

function loadChartData() {
  try {
    const savedDate = localStorage.getItem('poolChartDate');
    const today = new Date().toDateString();
    
    if (savedDate !== today) {
      resetChartData();
      return;
    }
    
    const saved = localStorage.getItem('poolChartData');
    if (saved) {
      chartData = JSON.parse(saved);
      console.log('Chart data loaded:', chartData.labels.length, 'points');
    }
  } catch (error) {
    console.error('Load chart error:', error);
    resetChartData();
  }
}

function resetChartData() {
  chartData = {
    labels: [],
    waterTemp: [],
    pressure: [],
    coverOpen: [],
    electrovalve: [],
    heatPump: [],
    activeTimers: []
  };
  saveChartData();
}

function refreshChart() {
  addChartDataPoint();
  alert('✅ Graphique actualisé !');
}

// ============================================================================
// ONGLET ÉTALONNAGE DÉDIÉ
// ============================================================================

let calibrationAssistantStep = 0;
let calibrationMode = null; // 'temp' ou 'pressure'
let calibrationRawValues = [];
let calibrationRealValues = [];
let calibrationMonitor = null;

async function loadCalibrationTab() {
  try {
    const calib = await fetch('/api/calibration').then(r => r.json());
    const sensors = await fetch('/api/sensors').then(r => r.json());
    
    // Afficher valeurs actuelles
    document.getElementById('calib-current-temp-raw').textContent = sensors.waterTemp.toFixed(2);
    document.getElementById('calib-current-pressure-raw').textContent = sensors.waterPressure.toFixed(2);
    
    // État étalonnage température
    const tempStatus = document.getElementById('calib-temp-status');
    if (calib.temperature.useCalibration) {
      const mode = calib.temperature.useTwoPoint ? '2 points' : 'Offset';
      tempStatus.innerHTML = `✅ Actif (${mode})`;
      tempStatus.className = 'calib-status active';
    } else {
      tempStatus.innerHTML = '❌ Désactivé';
      tempStatus.className = 'calib-status inactive';
    }
    
    // État étalonnage pression
    const pressStatus = document.getElementById('calib-pressure-status');
    if (calib.pressure.useCalibration) {
      const mode = calib.pressure.useTwoPoint ? '2 points' : 'Offset';
      pressStatus.innerHTML = `✅ Actif (${mode})`;
      pressStatus.className = 'calib-status active';
    } else {
      pressStatus.innerHTML = '❌ Désactivé';
      pressStatus.className = 'calib-status inactive';
    }
    
    // Graphique avant/après si calibration active
    if (calib.temperature.useCalibration) {
      showCalibrationChart('temp', calib.temperature);
    }
    if (calib.pressure.useCalibration) {
      showCalibrationChart('pressure', calib.pressure);
    }
    
  } catch (error) {
    console.error('Calibration tab error:', error);
  }
}

function startCalibrationAssistant(type) {
  calibrationMode = type;
  calibrationAssistantStep = 1;
  calibrationRawValues = [];
  calibrationRealValues = [];
  
  document.getElementById('calibAssistantModal').classList.add('active');
  
  if (type === 'temp') {
    document.getElementById('calib-assistant-title').textContent = '🌡️ Assistant Étalonnage Température';
  } else {
    document.getElementById('calib-assistant-title').textContent = '💪 Assistant Étalonnage Pression';
  }
  
  showCalibrationStep(1);
}

function showCalibrationStep(step) {
  // Masquer tous les steps
  document.querySelectorAll('.calib-step').forEach(s => s.style.display = 'none');
  
  // Afficher le step actuel
  const currentStep = document.getElementById(`calib-step-${step}`);
  if (currentStep) {
    currentStep.style.display = 'block';
  }
  
  // Mise à jour progression
  document.getElementById('calib-progress-text').textContent = `Étape ${step}/5`;
  document.getElementById('calib-progress-bar').style.width = `${(step / 5) * 100}%`;
  
  // Actions spécifiques par étape
  switch(step) {
    case 1:
      // Choix du mode
      break;
      
    case 2:
      // Démarrer monitoring temps réel
      startRealTimeMonitoring();
      break;
      
    case 3:
      // Point 2 (si 2 points)
      if (calibrationRawValues.length > 0) {
        document.getElementById('calib-point1-summary').textContent = 
          `Point 1: Raw=${calibrationRawValues[0].toFixed(2)}, Réel=${calibrationRealValues[0].toFixed(2)}`;
      }
      startRealTimeMonitoring();
      break;
      
    case 4:
      // Validation
      stopRealTimeMonitoring();
      showCalibrationSummary();
      break;
      
    case 5:
      // Test
      testCalibration();
      break;
  }
}

function selectCalibrationMode(useTwoPoint) {
  if (useTwoPoint) {
    calibrationAssistantStep = 2;
    showCalibrationStep(2);
  } else {
    // Mode offset simple
    calibrationAssistantStep = 2;
    showCalibrationStep(2);
  }
}

function startRealTimeMonitoring() {
  stopRealTimeMonitoring();
  
  calibrationMonitor = setInterval(async () => {
    try {
      const sensors = await fetch('/api/sensors').then(r => r.json());
      
      const value = calibrationMode === 'temp' ? sensors.waterTemp : sensors.waterPressure;
      
      // Afficher valeur brute
      const displayElement = calibrationAssistantStep === 2 ? 
        document.getElementById('calib-realtime-value1') :
        document.getElementById('calib-realtime-value2');
      
      if (displayElement) {
        displayElement.textContent = value.toFixed(2);
        
        // Animation
        displayElement.style.transform = 'scale(1.1)';
        setTimeout(() => {
          displayElement.style.transform = 'scale(1)';
        }, 200);
      }
      
    } catch (error) {
      console.error('Monitoring error:', error);
    }
  }, 1000);
}

function stopRealTimeMonitoring() {
  if (calibrationMonitor) {
    clearInterval(calibrationMonitor);
    calibrationMonitor = null;
  }
}

async function captureCalibrationPoint(pointNumber) {
  try {
    const sensors = await fetch('/api/sensors').then(r => r.json());
    const rawValue = calibrationMode === 'temp' ? sensors.waterTemp : sensors.waterPressure;
    
    const realValueInput = pointNumber === 1 ? 
      document.getElementById('calib-real-value1') :
      document.getElementById('calib-real-value2');
    
    const realValue = parseFloat(realValueInput.value);
    
    if (isNaN(realValue)) {
      alert('⚠️ Veuillez entrer une valeur réelle valide');
      return;
    }
    
    calibrationRawValues.push(rawValue);
    calibrationRealValues.push(realValue);
    
    console.log(`Point ${pointNumber} capturé: Raw=${rawValue}, Réel=${realValue}`);
    
    // Passer à l'étape suivante
    if (pointNumber === 1 && calibrationRawValues.length === 1) {
      // Demander mode
      const mode = document.querySelector('input[name="calib-mode"]:checked').value;
      if (mode === 'twopoint') {
        calibrationAssistantStep = 3;
        showCalibrationStep(3);
      } else {
        calibrationAssistantStep = 4;
        showCalibrationStep(4);
      }
    } else if (pointNumber === 2) {
      calibrationAssistantStep = 4;
      showCalibrationStep(4);
    }
    
  } catch (error) {
    console.error('Capture error:', error);
    alert('❌ Erreur lors de la capture');
  }
}

function showCalibrationSummary() {
  const summaryDiv = document.getElementById('calib-summary');
  let html = '<div class="calib-summary-box">';
  
  html += `<h4>📊 Récapitulatif</h4>`;
  html += `<div class="calib-summary-line"><strong>Capteur:</strong> ${calibrationMode === 'temp' ? 'Température' : 'Pression'}</div>`;
  
  if (calibrationRawValues.length === 1) {
    // Mode offset
    const offset = calibrationRealValues[0] - calibrationRawValues[0];
    html += `<div class="calib-summary-line"><strong>Mode:</strong> Offset simple</div>`;
    html += `<div class="calib-summary-line"><strong>Offset calculé:</strong> ${offset > 0 ? '+' : ''}${offset.toFixed(3)}</div>`;
  } else {
    // Mode 2 points
    html += `<div class="calib-summary-line"><strong>Mode:</strong> Calibration 2 points</div>`;
    html += `<div class="calib-summary-line"><strong>Point 1:</strong> Raw=${calibrationRawValues[0].toFixed(2)} → Réel=${calibrationRealValues[0].toFixed(2)}</div>`;
    html += `<div class="calib-summary-line"><strong>Point 2:</strong> Raw=${calibrationRawValues[1].toFixed(2)} → Réel=${calibrationRealValues[1].toFixed(2)}</div>`;
  }
  
  html += '</div>';
  summaryDiv.innerHTML = html;
}

async function applyCalibration() {
  try {
    // Construire la config
    const config = {
      temperature: {},
      pressure: {}
    };
    
    // Charger config actuelle
    const currentCalib = await fetch('/api/calibration').then(r => r.json());
    
    if (calibrationMode === 'temp') {
      config.temperature.useCalibration = true;
      config.temperature.useTwoPoint = (calibrationRawValues.length === 2);
      
      if (calibrationRawValues.length === 1) {
        config.temperature.offset = calibrationRealValues[0] - calibrationRawValues[0];
        config.temperature.point1Raw = 10.0;
        config.temperature.point1Real = 10.0;
        config.temperature.point2Raw = 30.0;
        config.temperature.point2Real = 30.0;
      } else {
        config.temperature.offset = 0;
        config.temperature.point1Raw = calibrationRawValues[0];
        config.temperature.point1Real = calibrationRealValues[0];
        config.temperature.point2Raw = calibrationRawValues[1];
        config.temperature.point2Real = calibrationRealValues[1];
      }
      
      // Garder la config pression
      config.pressure = currentCalib.pressure;
      
    } else {
      config.pressure.useCalibration = true;
      config.pressure.useTwoPoint = (calibrationRawValues.length === 2);
      
      if (calibrationRawValues.length === 1) {
        config.pressure.offset = calibrationRealValues[0] - calibrationRawValues[0];
        config.pressure.point1Raw = 1.0;
        config.pressure.point1Real = 1.0;
        config.pressure.point2Raw = 3.0;
        config.pressure.point2Real = 3.0;
      } else {
        config.pressure.offset = 0;
        config.pressure.point1Raw = calibrationRawValues[0];
        config.pressure.point1Real = calibrationRealValues[0];
        config.pressure.point2Raw = calibrationRawValues[1];
        config.pressure.point2Real = calibrationRealValues[1];
      }
      
      // Garder la config température
      config.temperature = currentCalib.temperature;
    }
    
    // Sauvegarder
    const response = await fetch('/api/calibration', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify(config)
    });
    
    if (response.ok) {
      calibrationAssistantStep = 5;
      showCalibrationStep(5);
    } else {
      alert('❌ Erreur lors de l\'application');
    }
    
  } catch (error) {
    console.error('Apply calibration error:', error);
    alert('❌ Erreur lors de l\'application');
  }
}

async function testCalibration() {
  const testResults = document.getElementById('calib-test-results');
  testResults.innerHTML = '<p>🔄 Test en cours...</p>';
  
  try {
    // Faire plusieurs lectures
    let readings = [];
    for (let i = 0; i < 5; i++) {
      const sensors = await fetch('/api/sensors').then(r => r.json());
      const value = calibrationMode === 'temp' ? sensors.waterTemp : sensors.waterPressure;
      readings.push(value);
      await new Promise(resolve => setTimeout(resolve, 1000));
    }
    
    const avg = readings.reduce((a, b) => a + b, 0) / readings.length;
    const min = Math.min(...readings);
    const max = Math.max(...readings);
    const variance = Math.max(max - avg, avg - min);
    
    let html = '<div class="test-results-box">';
    html += '<h4>✅ Test terminé</h4>';
    html += `<div class="test-result-line"><strong>Moyenne:</strong> ${avg.toFixed(3)}</div>`;
    html += `<div class="test-result-line"><strong>Min:</strong> ${min.toFixed(3)}</div>`;
    html += `<div class="test-result-line"><strong>Max:</strong> ${max.toFixed(3)}</div>`;
    html += `<div class="test-result-line"><strong>Variance:</strong> ${variance.toFixed(3)}</div>`;
    
    if (variance < 0.5) {
      html += '<p style="color: var(--success); margin-top: 15px;">✅ Stabilité excellente</p>';
    } else if (variance < 1.0) {
      html += '<p style="color: var(--warning); margin-top: 15px;">⚠️ Stabilité acceptable</p>';
    } else {
      html += '<p style="color: var(--danger); margin-top: 15px;">❌ Forte variation détectée</p>';
    }
    
    html += '</div>';
    testResults.innerHTML = html;
    
  } catch (error) {
    console.error('Test error:', error);
    testResults.innerHTML = '<p style="color: var(--danger);">❌ Erreur lors du test</p>';
  }
}

function closeCalibrationAssistant() {
  stopRealTimeMonitoring();
  document.getElementById('calibAssistantModal').classList.remove('active');
  calibrationAssistantStep = 0;
  calibrationMode = null;
  calibrationRawValues = [];
  calibrationRealValues = [];
  
  // Recharger l'onglet
  loadCalibrationTab();
}

function showCalibrationChart(type, calibData) {
  // Créer un petit graphique montrant l'effet de la calibration
  // À implémenter si besoin
}

async function disableCalibration(type) {
  if (!confirm(`Désactiver l'étalonnage ${type === 'temp' ? 'température' : 'pression'} ?`)) {
    return;
  }
  
  try {
    const currentCalib = await fetch('/api/calibration').then(r => r.json());
    
    if (type === 'temp') {
      currentCalib.temperature.useCalibration = false;
    } else {
      currentCalib.pressure.useCalibration = false;
    }
    
    await fetch('/api/calibration', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify(currentCalib)
    });
    
    alert('✅ Étalonnage désactivé');
    loadCalibrationTab();
    
  } catch (error) {
    console.error('Disable error:', error);
    alert('❌ Erreur');
  }
}

// ============================================================================
// SYSTÈME DE TIMERS FLEXIBLES
// ============================================================================

let currentEditingTimer = null;
let timerActions = [];
let currentEditingActionIndex = null;

const RELAY_NAMES = ['Pompe', 'Électrolyseur', 'Lampe', 'Électrovalve', 'PAC'];

async function loadFlexibleTimers() {
  try {
    const response = await fetch('/api/timers/flex');
    if (!response.ok) {
      console.error('Erreur chargement timers flex');
      return;
    }
    
    const timers = await response.json();
    displayFlexibleTimers(timers);
  } catch (error) {
    console.error('Erreur:', error);
  }
}

function displayFlexibleTimers(timers) {
  const listEl = document.getElementById('timerFlexList');
  const countBadge = document.getElementById('timer-count-badge');
  
  if (!timers || timers.length === 0) {
    listEl.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">Aucun timer configuré</p>';
    countBadge.textContent = '0';
    return;
  }
  
  countBadge.textContent = timers.length;
  
  let html = '';
  timers.forEach(timer => {
    const stateClass = timer.context.state === 2 ? 'active' : 
                      !timer.enabled ? 'disabled' :
                      timer.context.state === 5 ? 'error' : '';
    
    const stateText = timer.context.state === 2 ? '▶️ En cours' :
                     !timer.enabled ? '⏸️ Désactivé' :
                     timer.context.state === 5 ? '❌ Erreur' :
                     '⏹️ Inactif';
    
    const days = getDaysText(timer.days);
    const startTime = getStartTimeText(timer.startTime);
    
    html += `
      <div class="timer-flex-item ${stateClass}">
        <div class="timer-flex-header">
          <div class="timer-flex-title">${timer.name}</div>
          <div class="timer-flex-controls">
            <label class="switch">
              <input type="checkbox" ${timer.enabled ? 'checked' : ''} 
                     onchange="toggleTimerEnabled(${timer.id}, this.checked)">
              <span class="slider"></span>
            </label>
            <button class="btn btn-sm" onclick="editTimer(${timer.id})">✏️ Modifier</button>
            <button class="btn btn-sm btn-danger" onclick="deleteFlexTimer(${timer.id})">🗑️</button>
          </div>
        </div>
        
        <div class="timer-flex-info">
          <div class="timer-info-item">
            <span class="timer-info-label">État</span>
            <span class="timer-info-value">${stateText}</span>
          </div>
          <div class="timer-info-item">
            <span class="timer-info-label">Démarrage</span>
            <span class="timer-info-value">${startTime}</span>
          </div>
          <div class="timer-info-item">
            <span class="timer-info-label">Jours</span>
            <span class="timer-info-value">${days}</span>
          </div>
          <div class="timer-info-item">
            <span class="timer-info-label">Actions</span>
            <span class="timer-info-value">${timer.actionCount} étape(s)</span>
          </div>
        </div>
        
        ${timer.context.state === 2 ? getTimerProgress(timer) : ''}
        
        ${timer.context.state === 5 ? `<div class="alert alert-danger">${timer.context.lastError || 'Erreur inconnue'}</div>` : ''}
      </div>
    `;
  });
  
  listEl.innerHTML = html;
}

function getTimerProgress(timer) {
  if (timer.actionCount === 0) return '';
  
  const progress = ((timer.context.currentAction + 1) / timer.actionCount) * 100;
  
  return `
    <div class="timer-progress">
      <div class="progress-bar-container">
        <div class="progress-bar" style="width: ${progress}%"></div>
      </div>
      <div class="progress-text">
        Action ${timer.context.currentAction + 1} / ${timer.actionCount}
      </div>
    </div>
  `;
}

function getDaysText(days) {
  const dayNames = ['Dim', 'Lun', 'Mar', 'Mer', 'Jeu', 'Ven', 'Sam'];
  const activeDays = [];
  for (let i = 0; i < 7; i++) {
    if (days[i]) activeDays.push(dayNames[i]);
  }
  return activeDays.length === 7 ? 'Tous les jours' : activeDays.join(', ');
}

function getStartTimeText(startTime) {
  switch(startTime.type) {
    case 0:
      return `${startTime.hour.toString().padStart(2, '0')}:${startTime.minute.toString().padStart(2, '0')}`;
    case 1:
      return `Lever du soleil ${startTime.sunriseOffset >= 0 ? '+' : ''}${startTime.sunriseOffset}min`;
    case 2:
      return `Coucher du soleil ${startTime.sunriseOffset >= 0 ? '+' : ''}${startTime.sunriseOffset}min`;
  }
}

// ============================================================================
// ÉDITEUR DE TIMER - FIX POUR CHARGER UN TIMER EXISTANT
// ============================================================================

function showTimerEditor(timerId) {
  currentEditingTimer = timerId;
  timerActions = [];
  
  document.getElementById('timerEditorModal').classList.add('active');
  
  if (timerId) {
    document.getElementById('timerEditorTitle').textContent = '✏️ Modifier Timer';
    loadTimerForEdit(timerId);
  } else {
    document.getElementById('timerEditorTitle').textContent = '➕ Nouveau Timer';
    resetTimerEditor();
  }
  
  showEditorTab('config');
}

// FIX: Fonction editTimer qui manquait
function editTimer(timerId) {
  showTimerEditor(timerId);
}

// FIX: Charger les données d'un timer existant
async function loadTimerForEdit(timerId) {
  try {
    const response = await fetch('/api/timers/flex');
    const timers = await response.json();
    const timer = timers.find(t => t.id === timerId);
    
    if (!timer) {
      alert('Timer non trouvé');
      closeTimerEditor();
      return;
    }
    
    console.log('📝 Chargement timer:', timer);
    
    document.getElementById('edit-timer-name').value = timer.name;
    document.getElementById('edit-timer-enabled').checked = timer.enabled;
    
    document.querySelectorAll('.day-btn').forEach((btn, i) => {
      if (timer.days[i]) {
        btn.classList.add('active');
      } else {
        btn.classList.remove('active');
      }
    });
    
    document.getElementById('edit-start-type').value = timer.startTime.type;
    document.getElementById('edit-start-hour').value = timer.startTime.hour;
    document.getElementById('edit-start-minute').value = timer.startTime.minute;
    document.getElementById('edit-sun-offset').value = timer.startTime.sunriseOffset || 0;
    updateStartTimeInputs();
    
    // Charger conditions
    document.querySelectorAll('#editor-conditions input[type="checkbox"]').forEach(cb => {
      cb.checked = false;
    });
    
    if (timer.conditions) {
      timer.conditions.forEach(cond => {
        switch(cond.type) {
          case 0:
            document.getElementById('cond-cover-open').checked = true;
            break;
          case 2:
            document.getElementById('cond-temp-min').checked = true;
            document.getElementById('cond-temp-min-val').value = cond.value;
            break;
          case 3:
            document.getElementById('cond-temp-max').checked = true;
            document.getElementById('cond-temp-max-val').value = cond.value;
            break;
        }
      });
    }
    
    // FIX: Charger actions - IMPORTANT: Copier complètement pour éviter les références
    timerActions = [];
    if (timer.actions && Array.isArray(timer.actions)) {
      console.log(`📋 Chargement de ${timer.actions.length} actions`);
      timer.actions.forEach((a, index) => {
        const actionCopy = {
          type: a.type,
          relay: a.relay || 0,
          state: a.state || false,
          delayMinutes: a.delayMinutes || 0,
          conditionValue: a.conditionValue || 0.0,
          maxWaitMinutes: a.maxWaitMinutes || 0,
          description: a.description || ''
        };
        timerActions.push(actionCopy);
        console.log(`  Action ${index + 1}:`, actionCopy);
      });
      updateActionsList();
      console.log('✅ Actions chargées:', timerActions.length);
    } else {
      console.warn('⚠️ Pas d\'actions dans le timer');
    }
    
  } catch (error) {
    console.error('Erreur chargement timer:', error);
    alert('Erreur lors du chargement du timer');
    closeTimerEditor();
  }
}

function closeTimerEditor() {
  document.getElementById('timerEditorModal').classList.remove('active');
  currentEditingTimer = null;
  timerActions = [];
}

function showEditorTab(tabName) {
  document.querySelectorAll('.editor-tab').forEach(tab => {
    tab.classList.remove('active');
  });
  document.querySelectorAll('.editor-tab-content').forEach(content => {
    content.classList.remove('active');
  });
  
  document.querySelector(`.editor-tab[onclick*="${tabName}"]`).classList.add('active');
  document.getElementById(`editor-${tabName}`).classList.add('active');
  
  if (tabName === 'preview') {
    generateTimerPreview();
  }
}

function resetTimerEditor() {
  document.getElementById('edit-timer-name').value = '';
  document.getElementById('edit-timer-enabled').checked = true;
  document.getElementById('edit-start-type').value = '0';
  document.getElementById('edit-start-hour').value = '9';
  document.getElementById('edit-start-minute').value = '0';
  document.getElementById('edit-sun-offset').value = '0';
  
  document.querySelectorAll('.day-btn').forEach((btn, i) => {
    if (i >= 1 && i <= 5) {
      btn.classList.add('active');
    } else {
      btn.classList.remove('active');
    }
  });
  
  document.querySelectorAll('#editor-conditions input[type="checkbox"]').forEach(cb => {
    if (cb.id !== 'cond-no-leak') cb.checked = false;
  });
  
  timerActions = [];
  updateActionsList();
  
  updateStartTimeInputs();
}

function updateStartTimeInputs() {
  const type = parseInt(document.getElementById('edit-start-type').value);
  const fixedInputs = document.getElementById('fixed-time-inputs');
  const sunInputs = document.getElementById('sun-offset-input');
  
  if (type === 0) {
    fixedInputs.style.display = 'flex';
    sunInputs.style.display = 'none';
  } else {
    fixedInputs.style.display = 'none';
    sunInputs.style.display = 'flex';
  }
}

function toggleDay(btn) {
  btn.classList.toggle('active');
}

// ============================================================================
// GESTION DES ACTIONS - FIX POUR FORMULAIRES AU LIEU DE PROMPTS
// ============================================================================

function showActionSelector() {
  document.getElementById('actionSelectorModal').classList.add('active');
}

function closeActionSelector() {
  document.getElementById('actionSelectorModal').classList.remove('active');
}

function addActionType(type) {
  closeActionSelector();
  
  let action = {};
  
  switch(type) {
    case 'relay':
      action = {
        type: 0,
        relay: 0,
        state: true,
        delayMinutes: 0,
        buzzerCount: 1,
        ledColor: 0,
        ledMode: 0,
        ledDuration: 0,
        description: 'Pompe ON'
      };
      break;
      
    case 'wait':
      action = {
        type: 1,
        delayMinutes: 10,
        buzzerCount: 1,
        ledColor: 0,
        ledMode: 0,
        ledDuration: 0,
        description: 'Attendre 10 minutes'
      };
      break;
      
    case 'measure':
      action = {
        type: 4,
        buzzerCount: 1,
        ledColor: 0,
        ledMode: 0,
        ledDuration: 0,
        description: 'Mesurer température (après 15min pompe)'
      };
      break;
      
    case 'auto':
      action = {
        type: 5,
        buzzerCount: 1,
        ledColor: 0,
        ledMode: 0,
        ledDuration: 0,
		customEquation: {
		  useCustom: false,
		  expression: 'waterTemp / 2'
		},
        description: 'Durée automatique = Température / 2'
      };
      break;
      
    case 'buzzer':
      action = {
        type: 7,  // ACTION_BUZZER = 7
        buzzerCount: 2,
        ledColor: 0,
        ledMode: 0,
        ledDuration: 0,
        description: 'Buzzer 2 bips'
      };
      break;
      
    case 'led':
      action = {
        type: 8,  // ACTION_LED = 8
        buzzerCount: 1,
        ledColor: 2,  // Vert par défaut
        ledMode: 0,
        ledDuration: 0,
        description: 'LED Verte'
      };
      break;
  }
  
  timerActions.push(action);
  updateActionsList();
}

function updateActionsList() {
  const listEl = document.getElementById('actionsList');
  
  if (timerActions.length === 0) {
    listEl.innerHTML = '<p style="text-align: center; color: #999; padding: 30px;">Aucune action. Cliquez sur "Ajouter une action" pour commencer.</p>';
    return;
  }
  
  let html = '';
  timerActions.forEach((action, index) => {
    html += `
      <div class="action-item" draggable="true" ondragstart="dragStart(event, ${index})" ondragover="dragOver(event)" ondrop="drop(event, ${index})">
        <div class="action-number">${index + 1}</div>
        <div class="action-content">
          <div class="action-type">${getActionTypeName(action)}</div>
          <div class="action-details">${getActionDescription(action)}</div>
        </div>
        <div class="action-controls">
          <button class="btn btn-sm btn-icon" onclick="editActionForm(${index})">✏️</button>
          <button class="btn btn-sm btn-icon btn-danger" onclick="deleteAction(${index})">🗑️</button>
        </div>
      </div>
    `;
  });
  
  listEl.innerHTML = html;
}

function getActionTypeName(action) {
  if (action.type === 0) {
    return `${RELAY_NAMES[action.relay]} ${action.state ? 'ON' : 'OFF'}`;
  }
  const names = {
    1: 'Attendre',
    4: 'Mesurer Température',
    5: 'Durée Automatique',
    7: 'Buzzer',
    8: 'LED'
  };
  return names[action.type] || 'Action';
}

function getActionDescription(action) {
  let desc = action.description || '';
  
  if (action.type === 5) {
    if (action.customEquation && action.customEquation.useCustom) {
      desc = `📐 ${action.customEquation.expression}`;
    } else {
      desc = '📐 Temp / 2 (défaut, 3h-24h)';
    }
  }
  
  // Détails spécifiques buzzer (type 7)
  if (action.type === 7) {
    if (action.buzzerCount === 0) {
      desc = 'Alarme continue';
    } else {
      desc = `${action.buzzerCount} bip(s)`;
    }
  }
  
  // Détails spécifiques LED (type 8)
  if (action.type === 8) {
    const colors = ['Off', 'Bleu', 'Vert', 'Cyan', 'Rouge', 'Magenta', 'Jaune', 'Blanc'];
    const modes = ['Fixe', 'Clignotant', 'Pulsation'];
    desc = `${colors[action.ledColor] || 'Off'} - ${modes[action.ledMode] || 'Fixe'}`;
    if (action.ledDuration > 0) {
      desc += ` (${action.ledDuration}s)`;
    }
  }
  
  if (action.delayMinutes > 0 && action.type !== 1) {
    desc += ` (après ${action.delayMinutes} min)`;
  }
  
  return desc;
}

// FIX: Formulaire au lieu de prompt()
function editActionForm(index) {
  currentEditingActionIndex = index;
  const action = timerActions[index];
  
  const modal = document.getElementById('actionEditorModal');
  if (!modal) {
    createActionEditorModal();
  }
  
  // Masquer tous les formulaires
  document.getElementById('action-relay-form').style.display = 'none';
  document.getElementById('action-wait-form').style.display = 'none';
  document.getElementById('action-buzzer-form').style.display = 'none';
  document.getElementById('action-led-form').style.display = 'none';
  
  // Remplir le formulaire selon le type
  if (action.type === 0) {
    // RELAIS
    document.getElementById('edit-action-relay').value = action.relay;
    document.getElementById('edit-action-state-on').checked = action.state;
    document.getElementById('edit-action-state-off').checked = !action.state;
    document.getElementById('edit-action-delay').value = action.delayMinutes;
    document.getElementById('action-relay-form').style.display = 'block';
    
  } else if (action.type === 1) {
    // ATTENTE
    document.getElementById('edit-action-wait-duration').value = action.delayMinutes;
    document.getElementById('action-wait-form').style.display = 'block';
	
  } else if (action.type === 5) {
	  // DURÉE AUTOMATIQUE
	  document.getElementById('action-auto-form').style.display = 'block';
	  
	  document.getElementById('edit-action-auto-use-custom').checked = 
		action.customEquation?.useCustom || false;
	  document.getElementById('edit-action-auto-equation').value = 
		action.customEquation?.expression || 'waterTemp / 2';
	  
	  updateAutoEquationForm();  

  } else if (action.type === 7) {
    // BUZZER (type 7)
    if (action.buzzerCount === 0) {
      document.getElementById('edit-action-buzzer-type').value = 'alarm';
      document.getElementById('buzzer-count-group').style.display = 'none';
    } else {
      document.getElementById('edit-action-buzzer-type').value = 'beep';
      document.getElementById('edit-action-buzzer-count').value = action.buzzerCount;
      document.getElementById('buzzer-count-group').style.display = 'block';
    }
    document.getElementById('action-buzzer-form').style.display = 'block';
    
  } else if (action.type === 8) {
    // LED (type 8)
    document.getElementById('edit-action-led-color').value = action.ledColor;
    document.getElementById('edit-action-led-mode').value = action.ledMode;
    document.getElementById('edit-action-led-duration').value = action.ledDuration;
    document.getElementById('action-led-form').style.display = 'block';
  }
  
  document.getElementById('actionEditorModal').classList.add('active');
}

// Fonction helper pour le formulaire buzzer
function updateBuzzerForm() {
  const type = document.getElementById('edit-action-buzzer-type').value;
  const countGroup = document.getElementById('buzzer-count-group');
  if (type === 'alarm') {
    countGroup.style.display = 'none';
  } else {
    countGroup.style.display = 'block';
  }
}

function updateAutoEquationForm() {
  const useCustom = document.getElementById('edit-action-auto-use-custom').checked;
  const equationGroup = document.getElementById('auto-equation-group');
  equationGroup.style.display = useCustom ? 'block' : 'none';
}

function saveActionEdit() {
  if (currentEditingActionIndex === null) return;
  
  const action = timerActions[currentEditingActionIndex];
  
  if (action.type === 0) {
    // RELAIS
    action.relay = parseInt(document.getElementById('edit-action-relay').value);
    action.state = document.getElementById('edit-action-state-on').checked;
    action.delayMinutes = parseInt(document.getElementById('edit-action-delay').value);
    action.description = `${RELAY_NAMES[action.relay]} ${action.state ? 'ON' : 'OFF'}`;
    
  } else if (action.type === 1) {
    // ATTENTE
    action.delayMinutes = parseInt(document.getElementById('edit-action-wait-duration').value);
    action.description = `Attendre ${action.delayMinutes} minutes`;
	
  } else if (action.type === 5) {
	  // DURÉE AUTOMATIQUE
	  action.customEquation = {
		useCustom: document.getElementById('edit-action-auto-use-custom').checked,
		expression: document.getElementById('edit-action-auto-equation').value
	  };
	  
	  if (action.customEquation.useCustom) {
		action.description = `Équation: ${action.customEquation.expression.substring(0, 40)}${action.customEquation.expression.length > 40 ? '...' : ''}`;
	  } else {
		action.description = 'Durée auto = Température / 2 (3h-24h)';
	  }
    
  } else if (action.type === 7) {
    // BUZZER (type 7)
    const buzzerType = document.getElementById('edit-action-buzzer-type').value;
    if (buzzerType === 'alarm') {
      action.buzzerCount = 0;
      action.description = 'Alarme continue';
    } else {
      action.buzzerCount = parseInt(document.getElementById('edit-action-buzzer-count').value);
      action.description = `Buzzer ${action.buzzerCount} bip(s)`;
    }
    
  } else if (action.type === 8) {
    // LED (type 8)
    action.ledColor = parseInt(document.getElementById('edit-action-led-color').value);
    action.ledMode = parseInt(document.getElementById('edit-action-led-mode').value);
    action.ledDuration = parseInt(document.getElementById('edit-action-led-duration').value);
    
    const colors = ['Off', 'Bleu', 'Vert', 'Cyan', 'Rouge', 'Magenta', 'Jaune', 'Blanc'];
    const modes = ['Fixe', 'Clignotant', 'Pulsation'];
    action.description = `LED ${colors[action.ledColor]} - ${modes[action.ledMode]}`;
    if (action.ledDuration > 0) {
      action.description += ` (${action.ledDuration}s)`;
    }
  }
  
  updateActionsList();
  closeActionEditor();
}

function closeActionEditor() {
  document.getElementById('actionEditorModal').classList.remove('active');
  currentEditingActionIndex = null;
}

function deleteAction(index) {
  if (confirm('Supprimer cette action ?')) {
    timerActions.splice(index, 1);
    updateActionsList();
  }
}

let draggedIndex = null;

function dragStart(event, index) {
  draggedIndex = index;
  event.dataTransfer.effectAllowed = 'move';
}

function dragOver(event) {
  event.preventDefault();
  event.dataTransfer.dropEffect = 'move';
}

function drop(event, dropIndex) {
  event.preventDefault();
  
  if (draggedIndex === null || draggedIndex === dropIndex) return;
  
  const draggedAction = timerActions[draggedIndex];
  timerActions.splice(draggedIndex, 1);
  timerActions.splice(dropIndex, 0, draggedAction);
  
  draggedIndex = null;
  updateActionsList();
}

// ============================================================================
// APERÇU DU TIMER
// ============================================================================

function generateTimerPreview() {
  const previewEl = document.getElementById('timerPreview');
  
  const name = document.getElementById('edit-timer-name').value || 'Timer sans nom';
  const enabled = document.getElementById('edit-timer-enabled').checked;
  const startType = parseInt(document.getElementById('edit-start-type').value);
  
  let startText = '';
  if (startType === 0) {
    const h = document.getElementById('edit-start-hour').value;
    const m = document.getElementById('edit-start-minute').value;
    startText = `${h.padStart(2, '0')}:${m.padStart(2, '0')}`;
  } else if (startType === 1) {
    startText = `Lever du soleil +${document.getElementById('edit-sun-offset').value}min`;
  } else {
    startText = `Coucher du soleil +${document.getElementById('edit-sun-offset').value}min`;
  }
  
  const days = [];
  document.querySelectorAll('.day-btn').forEach((btn, i) => {
    if (btn.classList.contains('active')) {
      days.push(['Dim', 'Lun', 'Mar', 'Mer', 'Jeu', 'Ven', 'Sam'][i]);
    }
  });
  
  const conditions = [];
  if (document.getElementById('cond-cover-open').checked) {
    conditions.push('🎚️ Volet ouvert');
  }
  if (document.getElementById('cond-temp-min').checked) {
    conditions.push(`🌡️ Eau ≥ ${document.getElementById('cond-temp-min-val').value}°C`);
  }
  if (document.getElementById('cond-temp-max').checked) {
    conditions.push(`🌡️ Eau ≤ ${document.getElementById('cond-temp-max-val').value}°C`);
  }
  
  let html = `
    <div style="background: white; padding: 20px; border-radius: 12px; margin-bottom: 20px;">
      <h3 style="margin-bottom: 15px;">${name} ${enabled ? '✅' : '⏸️'}</h3>
      <p><strong>Démarrage:</strong> ${startText}</p>
      <p><strong>Jours:</strong> ${days.join(', ') || 'Aucun'}</p>
      ${conditions.length > 0 ? `<p><strong>Conditions:</strong> ${conditions.join(', ')}</p>` : ''}
    </div>
    
    <h4 style="margin: 20px 0 15px;">Timeline des actions:</h4>
    <div class="preview-timeline">
  `;
  
  if (timerActions.length === 0) {
    html += '<p style="color: #999;">Aucune action définie</p>';
  } else {
    timerActions.forEach((action, i) => {
      html += `
        <div class="preview-timeline-item">
          <strong>${i + 1}. ${getActionTypeName(action)}</strong>
          <div style="color: #666; font-size: 0.9em;">${getActionDescription(action)}</div>
        </div>
      `;
    });
  }
  
  html += '</div>';
  
  previewEl.innerHTML = html;
}

// ============================================================================
// SAUVEGARDE
// ============================================================================

async function saveTimer() {
  const timer = {
    id: currentEditingTimer || Date.now(),
    name: document.getElementById('edit-timer-name').value,
    enabled: document.getElementById('edit-timer-enabled').checked,
    days: [],
    startTime: {
      type: parseInt(document.getElementById('edit-start-type').value),
      hour: parseInt(document.getElementById('edit-start-hour').value),
      minute: parseInt(document.getElementById('edit-start-minute').value),
      sunriseOffset: parseInt(document.getElementById('edit-sun-offset').value)
    },
    conditions: [],
    actions: timerActions
  };
  
  document.querySelectorAll('.day-btn').forEach((btn, i) => {
    timer.days.push(btn.classList.contains('active'));
  });
  
  if (document.getElementById('cond-cover-open').checked) {
    timer.conditions.push({ type: 0, value: 0, required: true });
  }
  if (document.getElementById('cond-temp-min').checked) {
    timer.conditions.push({
      type: 2,
      value: parseFloat(document.getElementById('cond-temp-min-val').value),
      required: true
    });
  }
  if (document.getElementById('cond-temp-max').checked) {
    timer.conditions.push({
      type: 3,
      value: parseFloat(document.getElementById('cond-temp-max-val').value),
      required: true
    });
  }
  
  if (!timer.name) {
    alert('⚠️ Veuillez donner un nom au timer');
    return;
  }
  
  if (timer.actions.length === 0) {
    alert('⚠️ Ajoutez au moins une action');
    return;
  }
  
  try {
    // FIX: Utiliser l'ID existant pour la mise à jour
    const url = currentEditingTimer ? `/api/timers/flex/${timer.id}` : '/api/timers/flex';
    const method = currentEditingTimer ? 'PUT' : 'POST';
    
    console.log(`${method} ${url}`, timer);
    
    const response = await fetch(url, {
      method: method,
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(timer)
    });
    
    if (response.ok) {
      alert('✅ Timer sauvegardé !');
      closeTimerEditor();
      loadFlexibleTimers();
    } else {
      const error = await response.text();
      console.error('Erreur serveur:', error);
      alert('❌ Erreur: ' + error);
    }
  } catch (error) {
    console.error('Erreur sauvegarde:', error);
    alert('❌ Erreur lors de la sauvegarde');
  }
}

async function toggleTimerEnabled(id, enabled) {
  try {
    await fetch(`/api/timers/flex/${id}/toggle`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ enabled })
    });
    loadFlexibleTimers();
  } catch (error) {
    console.error('Erreur toggle timer:', error);
  }
}

async function deleteFlexTimer(id) {
  if (!confirm('Supprimer ce timer ?')) return;
  
  try {
    await fetch(`/api/timers/flex/${id}`, { method: 'DELETE' });
    loadFlexibleTimers();
  } catch (error) {
    console.error('Erreur suppression:', error);
  }
}

// ============================================================================
// CALIBRATION
// ============================================================================

async function loadCalibration() {
  try {
    const calib = await fetch('/api/calibration').then(r => r.json());
    
    // Température - avec protection contre éléments manquants
    const tempEnable = document.getElementById('calib-temp-enable');
    if (tempEnable) tempEnable.checked = calib.temperature.useCalibration;
    
    const tempMode = document.getElementById('calib-temp-mode');
    if (tempMode) tempMode.value = calib.temperature.useTwoPoint ? 'twopoint' : 'offset';
    
    const tempOffset = document.getElementById('calib-temp-offset');
    if (tempOffset) tempOffset.value = calib.temperature.offset;
    
    const tempP1Raw = document.getElementById('calib-temp-p1-raw');
    if (tempP1Raw) tempP1Raw.value = calib.temperature.point1Raw;
    
    const tempP1Real = document.getElementById('calib-temp-p1-real');
    if (tempP1Real) tempP1Real.value = calib.temperature.point1Real;
    
    const tempP2Raw = document.getElementById('calib-temp-p2-raw');
    if (tempP2Raw) tempP2Raw.value = calib.temperature.point2Raw;
    
    const tempP2Real = document.getElementById('calib-temp-p2-real');
    if (tempP2Real) tempP2Real.value = calib.temperature.point2Real;
    
    // Pression - avec protection
    const pressEnable = document.getElementById('calib-pressure-enable');
    if (pressEnable) pressEnable.checked = calib.pressure.useCalibration;
    
    const pressMode = document.getElementById('calib-pressure-mode');
    if (pressMode) pressMode.value = calib.pressure.useTwoPoint ? 'twopoint' : 'offset';
    
    const pressOffset = document.getElementById('calib-pressure-offset');
    if (pressOffset) pressOffset.value = calib.pressure.offset;
    
    const pressP1Raw = document.getElementById('calib-pressure-p1-raw');
    if (pressP1Raw) pressP1Raw.value = calib.pressure.point1Raw;
    
    const pressP1Real = document.getElementById('calib-pressure-p1-real');
    if (pressP1Real) pressP1Real.value = calib.pressure.point1Real;
    
    const pressP2Raw = document.getElementById('calib-pressure-p2-raw');
    if (pressP2Raw) pressP2Raw.value = calib.pressure.point2Raw;
    
    const pressP2Real = document.getElementById('calib-pressure-p2-real');
    if (pressP2Real) pressP2Real.value = calib.pressure.point2Real;
    
    // Mettre à jour l'affichage seulement si les éléments existent
    if (tempMode) updateCalibTempFields();
    if (pressMode) updateCalibPressureFields();
    
  } catch (error) {
    console.error('Load calibration error:', error);
  }
}

function updateCalibTempFields() {
  const mode = document.getElementById('calib-temp-mode').value;
  const offsetFields = document.getElementById('temp-offset-fields');
  const twopointFields = document.getElementById('temp-twopoint-fields');
  
  if (mode === 'offset') {
    offsetFields.style.display = 'block';
    twopointFields.style.display = 'none';
  } else {
    offsetFields.style.display = 'none';
    twopointFields.style.display = 'block';
  }
}

function updateCalibPressureFields() {
  const mode = document.getElementById('calib-pressure-mode').value;
  const offsetFields = document.getElementById('pressure-offset-fields');
  const twopointFields = document.getElementById('pressure-twopoint-fields');
  
  if (mode === 'offset') {
    offsetFields.style.display = 'block';
    twopointFields.style.display = 'none';
  } else {
    offsetFields.style.display = 'none';
    twopointFields.style.display = 'block';
  }
}

async function resetCalibration() {
  if (!confirm('⚠️ Réinitialiser toute la calibration ?\n\nCela remettra les capteurs aux valeurs brutes.')) {
    return;
  }
  
  try {
    await fetch('/api/calibration/reset', { method: 'POST' });
    alert('✅ Calibration réinitialisée !');
    loadCalibration();
  } catch (error) {
    console.error('Reset calibration error:', error);
    alert('❌ Erreur lors de la réinitialisation');
  }
}

// ============================================================================
// PARAMÈTRES
// ============================================================================

async function loadSettings() {
  try {
    const mqttConfig = await fetch('/api/mqtt/config').then(r => r.json());
    document.getElementById('mqtt-server').value = mqttConfig.server || '';
    document.getElementById('mqtt-port').value = mqttConfig.port || 1883;
    document.getElementById('mqtt-user').value = mqttConfig.user || '';
    document.getElementById('mqtt-topic').value = mqttConfig.topic || 'pool/control';
    
    const mqttStatus = await fetch('/api/mqtt/status').then(r => r.json());
    const badge = document.getElementById('mqtt-status-badge');
    if (mqttStatus.connected) {
      badge.textContent = 'Connecté';
      badge.className = 'badge success';
    } else {
      badge.textContent = 'Déconnecté';
      badge.className = 'badge warning';
    }
  } catch (error) {
    console.error('MQTT config error:', error);
  }
  
  try {
    const weatherConfig = await fetch('/api/weather/config').then(r => r.json());
    document.getElementById('weather-api').value = weatherConfig.apiKey || '';
    document.getElementById('weather-lat').value = weatherConfig.latitude || '';
    document.getElementById('weather-lon').value = weatherConfig.longitude || '';
  } catch (error) {
    console.error('Weather config error:', error);
  }
  
  try {
    const sysConfig = await fetch('/api/system/config').then(r => r.json());
    document.getElementById('pressure-threshold').value = sysConfig.pressureThreshold || 2.0;
    document.getElementById('buzzer-enabled').checked = sysConfig.buzzerEnabled !== false;
  } catch (error) {
    console.error('System config error:', error);
  }
  
  // Charger infos système
  loadSystemInfo();
  // Charger calibration (ne pas bloquer si erreur)
  loadCalibration().catch(err => {
    console.error('Calibration load error:', err);
  });
}

async function loadSystemInfo() {
  try {
    const sysInfo = await fetch('/api/system').then(r => r.json());
    document.getElementById('sys-version').textContent = sysInfo.version || '--';
    document.getElementById('sys-ip').textContent = sysInfo.ip || '--';
    
    const uptimeSeconds = sysInfo.uptime || 0;
    const days = Math.floor(uptimeSeconds / 86400);
    const hours = Math.floor((uptimeSeconds % 86400) / 3600);
    const minutes = Math.floor((uptimeSeconds % 3600) / 60);
    document.getElementById('sys-uptime').textContent = `${days}j ${hours}h ${minutes}m`;
    
    const heapKB = Math.round(sysInfo.freeHeap / 1024);
    document.getElementById('sys-heap').textContent = heapKB + ' KB';
  } catch (error) {
    console.error('System info error:', error);
  }
}

async function testMQTT() {
  alert('Test de connexion MQTT...\nVérifiez les logs série pour le résultat.');
}

async function rediscoverHomeAssistant() {
  try {
    const response = await fetch('/api/mqtt/rediscover', {method: 'POST'});
    const result = await response.text();
    if (result.includes('published')) {
      alert('✅ Découverte Home Assistant republiée !');
    } else {
      alert('❌ Erreur: ' + result);
    }
  } catch (error) {
    alert('❌ Erreur lors de la republication');
  }
}

async function restartSystem() {
  if (!confirm('⚠️ Redémarrer le système ?\n\nCela prendra environ 30 secondes.')) return;
  
  try {
    await fetch('/api/system/restart', {method: 'POST'});
    alert('✅ Redémarrage en cours...\n\nReconnectez-vous dans 30 secondes.');
    logout();
  } catch (error) {
    console.error('Restart error:', error);
  }
}

// ============================================================================
// UTILISATEURS
// ============================================================================

async function loadUsers() {
  if (currentRole !== 'admin') return;
  
  try {
    const users = await fetch('/api/users').then(r => r.json());
    displayUsers(users);
  } catch (error) {
    console.error('Load users error:', error);
  }
}

function displayUsers(users) {
  const usersList = document.getElementById('usersList');
  const countBadge = document.getElementById('user-count-badge');
  
  if (!users || users.length === 0) {
    usersList.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">Aucun utilisateur</p>';
    countBadge.textContent = '0';
    return;
  }
  
  countBadge.textContent = users.length;
  
  let html = '';
  users.forEach(user => {
    html += `<div class="user-item">
      <div class="user-info">
        <div class="user-name">${user.username}</div>
        <div class="user-role">${user.role === 'admin' ? 'Administrateur' : 'Utilisateur'}</div>
      </div>
      <span class="user-badge ${user.role}">${user.role.toUpperCase()}</span>
      ${user.username !== 'admin' ? 
        `<button class="btn btn-danger" onclick="deleteUser('${user.username}')">Supprimer</button>` : 
        '<span style="color: #999; font-size: 0.9em;">Protégé</span>'}
    </div>`;
  });
  
  usersList.innerHTML = html;
}

async function deleteUser(username) {
  if (!confirm(`Supprimer l'utilisateur "${username}" ?`)) return;
  
  try {
    await fetch('/api/users/delete', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({username})
    });
    loadUsers();
  } catch (error) {
    console.error('Delete user error:', error);
    alert('❌ Erreur lors de la suppression');
  }
}

async function changePassword() {
  const oldPassword = prompt('Ancien mot de passe:');
  if (!oldPassword) return;
  
  const newPassword = prompt('Nouveau mot de passe (min 6 caractères):');
  if (!newPassword || newPassword.length < 6) {
    alert('❌ Le mot de passe doit faire au moins 6 caractères');
    return;
  }
  
  const confirmPassword = prompt('Confirmer le nouveau mot de passe:');
  if (newPassword !== confirmPassword) {
    alert('❌ Les mots de passe ne correspondent pas');
    return;
  }
  
  try {
    const response = await fetch('/api/users/change-password', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({
        username: currentUser,
        oldPassword: oldPassword,
        newPassword: newPassword
      })
    });
    
    if (response.ok) {
      alert('✅ Mot de passe changé avec succès !');
    } else {
      const error = await response.text();
      alert('❌ ' + error);
    }
  } catch (error) {
    console.error('Change password error:', error);
    alert('❌ Erreur lors du changement de mot de passe');
  }
}

// ============================================================================
// HELPER: Créer le modal d'édition d'action
// ============================================================================

function createActionEditorModal() {
  const modalHTML = `
  <div class="modal" id="actionEditorModal">
    <div class="modal-content">
      <div class="modal-header">
        <h2>✏️ Modifier l'action</h2>
        <button class="close-btn" onclick="closeActionEditor()">✕</button>
      </div>
      
      <div class="modal-body">
        <!-- RELAIS -->
        <div id="action-relay-form" style="display: none;">
          <div class="form-group">
            <label>Relais</label>
            <select id="edit-action-relay" style="width: 100%;">
              <option value="0">Pompe</option>
              <option value="1">Électrolyseur</option>
              <option value="2">Lampe</option>
              <option value="3">Électrovalve</option>
              <option value="4">PAC</option>
            </select>
          </div>
          
          <div class="form-group">
            <label>État</label>
            <div style="display: flex; gap: 10px;">
              <label style="display: flex; align-items: center; gap: 5px;">
                <input type="radio" name="action-state" id="edit-action-state-on" value="on" checked>
                <span>ON</span>
              </label>
              <label style="display: flex; align-items: center; gap: 5px;">
                <input type="radio" name="action-state" id="edit-action-state-off" value="off">
                <span>OFF</span>
              </label>
            </div>
          </div>
          
          <div class="form-group">
            <label>Délai avant exécution (minutes)</label>
            <input type="number" id="edit-action-delay" value="0" min="0" style="width: 100%;">
          </div>
        </div>
        
        <!-- ATTENTE -->
        <div id="action-wait-form" style="display: none;">
          <div class="form-group">
            <label>Durée d'attente (minutes)</label>
            <input type="number" id="edit-action-wait-duration" value="10" min="1" style="width: 100%;">
          </div>
        </div>
        
        <!-- BUZZER -->
        <div id="action-buzzer-form" style="display: none;">
          <div class="form-group">
            <label>Type de signal</label>
            <select id="edit-action-buzzer-type" onchange="updateBuzzerForm()" style="width: 100%;">
              <option value="beep">Bips</option>
              <option value="alarm">Alarme continue</option>
            </select>
          </div>
          
          <div class="form-group" id="buzzer-count-group">
            <label>Nombre de bips (1-10)</label>
            <input type="number" id="edit-action-buzzer-count" value="2" min="1" max="10" style="width: 100%;">
          </div>
        </div>
        
        <!-- LED -->
        <div id="action-led-form" style="display: none;">
          <div class="form-group">
            <label>Couleur</label>
            <select id="edit-action-led-color" style="width: 100%;">
              <option value="0">Off (Éteint)</option>
              <option value="1">🔵 Bleu</option>
              <option value="2" selected>🟢 Vert</option>
              <option value="3">🔵 Cyan</option>
              <option value="4">🔴 Rouge</option>
              <option value="5">🟣 Magenta</option>
              <option value="6">🟡 Jaune</option>
              <option value="7">⚪ Blanc</option>
            </select>
          </div>
          
          <div class="form-group">
            <label>Mode</label>
            <select id="edit-action-led-mode" style="width: 100%;">
              <option value="0" selected>Fixe</option>
              <option value="1">Clignotant</option>
              <option value="2">Pulsation</option>
            </select>
          </div>
          
          <div class="form-group">
            <label>Durée (secondes, 0=permanent)</label>
            <input type="number" id="edit-action-led-duration" value="0" min="0" style="width: 100%;">
            <small>0 = LED reste allumée jusqu'à prochaine action LED</small>
          </div>
        </div>
		
		<!-- DURÉE AUTOMATIQUE -->
		<div id="action-auto-form" style="display: none;">
		  <div class="form-group">
			<label style="display: flex; align-items: center; gap: 10px;">
			  <input type="checkbox" id="edit-action-auto-use-custom" onchange="updateAutoEquationForm()">
			  <span>🔧 Utiliser une équation personnalisée</span>
			</label>
			<small style="display: block; margin-top: 5px; color: #666;">
			  Par défaut : <code>waterTemp / 2</code> (limité entre 3h et 24h)
			</small>
		  </div>
		  
		  <div class="form-group" id="auto-equation-group" style="display: none;">
			<label>📐 Équation personnalisée</label>
			<textarea id="edit-action-auto-equation" rows="3" 
					  style="width: 100%; font-family: 'Courier New', monospace; 
							 padding: 10px; border-radius: 8px; border: 2px solid #e0e0e0;">waterTemp / 2</textarea>
			
			<details style="margin-top: 15px; border: 2px solid var(--primary); border-radius: 8px; padding: 10px;">
			  <summary style="cursor: pointer; color: var(--primary); font-weight: 600; font-size: 1.05em;">
				📊 Variables et exemples d'équations
			  </summary>
			  
			  <div style="background: #f8f9fa; padding: 15px; border-radius: 8px; margin-top: 10px;">
				<h4 style="color: var(--secondary); margin-bottom: 10px;">🔢 Variables disponibles</h4>
				<table style="width: 100%; font-size: 0.9em; border-collapse: collapse;">
				  <tr style="border-bottom: 1px solid #ddd;">
					<td style="padding: 8px; font-family: monospace; font-weight: 600;"><code>waterTemp</code></td>
					<td style="padding: 8px;">Température de l'eau mesurée (°C)</td>
				  </tr>
				  <tr style="border-bottom: 1px solid #ddd;">
					<td style="padding: 8px; font-family: monospace; font-weight: 600;"><code>extTemp</code></td>
					<td style="padding: 8px;">Température extérieure actuelle (°C)</td>
				  </tr>
				  <tr style="border-bottom: 1px solid #ddd;">
					<td style="padding: 8px; font-family: monospace; font-weight: 600;"><code>weatherMax</code></td>
					<td style="padding: 8px;">Température MAX prévue aujourd'hui (°C)</td>
				  </tr>
				  <tr style="border-bottom: 1px solid #ddd;">
					<td style="padding: 8px; font-family: monospace; font-weight: 600;"><code>weatherMin</code></td>
					<td style="padding: 8px;">Température MIN prévue aujourd'hui (°C)</td>
				  </tr>
				  <tr>
					<td style="padding: 8px; font-family: monospace; font-weight: 600;"><code>sunshine</code></td>
					<td style="padding: 8px;">Pourcentage d'ensoleillement (0-100)</td>
				  </tr>
				</table>
				
				<div style="margin-top: 20px; border-top: 2px solid #ddd; padding-top: 15px;">
				  <h4 style="color: var(--secondary); margin-bottom: 10px;">💡 Exemples d'équations</h4>
				  <ul style="margin-top: 10px; padding-left: 20px; line-height: 1.8;">
					<li>
					  <code style="background: #fff; padding: 4px 8px; border-radius: 4px; font-weight: 600;">waterTemp / 2</code>
					  <span style="color: #666;"> - Formule classique (défaut)</span>
					</li>
					<li>
					  <code style="background: #fff; padding: 4px 8px; border-radius: 4px; font-weight: 600;">(waterTemp + extTemp) / 3</code>
					  <span style="color: #666;"> - Moyenne entre eau et extérieur</span>
					</li>
					<li>
					  <code style="background: #fff; padding: 4px 8px; border-radius: 4px; font-weight: 600;">waterTemp / 2 + sunshine / 20</code>
					  <span style="color: #666;"> - Ajustement selon ensoleillement</span>
					</li>
					<li>
					  <code style="background: #fff; padding: 4px 8px; border-radius: 4px; font-weight: 600;">(waterTemp + weatherMax) / 2.5</code>
					  <span style="color: #666;"> - Basé sur température maximale</span>
					</li>
					<li>
					  <code style="background: #fff; padding: 4px 8px; border-radius: 4px; font-weight: 600;">(weatherMax - waterTemp) * 0.5 + 3</code>
					  <span style="color: #666;"> - Compensation écart météo</span>
					</li>
					<li>
					  <code style="background: #fff; padding: 4px 8px; border-radius: 4px; font-weight: 600;">waterTemp / 2 + (sunshine - 50) / 25</code>
					  <span style="color: #666;"> - Ajustement fin avec soleil</span>
					</li>
				  </ul>
				</div>
				
				<div style="margin-top: 15px; padding: 12px; background: #fff3cd; border-left: 4px solid #ffc107; border-radius: 4px;">
				  <strong>⚙️ Opérateurs supportés :</strong> 
				  <code>+ - * / ( )</code>
				  <br>
				  <strong>📏 Limites :</strong> 
				  Résultat automatiquement limité entre <strong>3h et 24h</strong>
				  <br>
				  <strong>🔄 Cycle :</strong> 
				  Si durée > 24h, le timer boucle sur le jour suivant
				</div>
			  </div>
			</details>
		  </div>
		</div>
      </div>
      
      <div class="modal-footer">
        <button class="btn" onclick="closeActionEditor()">Annuler</button>
        <button class="btn btn-success" onclick="saveActionEdit()">💾 Sauvegarder</button>
      </div>
    </div>
  </div>
  `;
  
  document.body.insertAdjacentHTML('beforeend', modalHTML);
}

// ============================================================================
// EVENT LISTENERS
// ============================================================================

document.addEventListener('DOMContentLoaded', () => {
	
  initTheme();
  // Créer le modal d'édition d'action
  createActionEditorModal();
  
  document.getElementById('loginForm').addEventListener('submit', (e) => {
    e.preventDefault();
    const username = document.getElementById('username').value;
    const password = document.getElementById('loginPassword').value;
    login(username, password);
  });
  
  document.getElementById('logoutBtn').addEventListener('click', logout);
  
  // Event listener pour le changement d'intervalle graphique
	const chartIntervalSelect = document.getElementById('chart-interval-select');
	if (chartIntervalSelect) {
	  chartIntervalSelect.addEventListener('change', (e) => {
		const newInterval = parseInt(e.target.value);
		updateChartInterval(newInterval);
	  });
	  
	  // Initialiser l'affichage avec la valeur actuelle
	  chartIntervalSelect.value = chartUpdateInterval.toString();
	  updateChartIntervalDisplay();
	}
  
  document.getElementById('mqttForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const config = {
      server: document.getElementById('mqtt-server').value,
      port: parseInt(document.getElementById('mqtt-port').value),
      user: document.getElementById('mqtt-user').value,
      password: document.getElementById('mqtt-password').value,
      topic: document.getElementById('mqtt-topic').value
    };
    
    try {
      await fetch('/api/saveMQTT', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(config)
      });
      alert('✅ Configuration MQTT sauvegardée !');
      loadSettings();
    } catch (error) {
      console.error('MQTT save error:', error);
      alert('❌ Erreur sauvegarde MQTT');
    }
  });
  
  document.getElementById('weatherForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const config = {
      apiKey: document.getElementById('weather-api').value,
      latitude: document.getElementById('weather-lat').value,
      longitude: document.getElementById('weather-lon').value
    };
    
    try {
      await fetch('/api/weather/save', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(config)
      });
      alert('✅ Configuration météo sauvegardée !');
    } catch (error) {
      console.error('Weather save error:', error);
      alert('❌ Erreur sauvegarde météo');
    }
  });
  
  document.getElementById('systemForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const config = {
      pressureThreshold: parseFloat(document.getElementById('pressure-threshold').value),
      buzzerEnabled: document.getElementById('buzzer-enabled').checked
    };
    
    try {
      await fetch('/api/system/config', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(config)
      });
      alert('✅ Configuration système sauvegardée !');
    } catch (error) {
      console.error('System save error:', error);
      alert('❌ Erreur sauvegarde système');
    }
  });
  
  document.getElementById('userForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const user = {
      username: document.getElementById('user-username').value,
      password: document.getElementById('user-password').value,
      role: document.getElementById('user-role').value
    };
    
    try {
      await fetch('/api/users/add', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(user)
      });
      
      document.getElementById('userForm').reset();
      alert('✅ Utilisateur ajouté !');
      loadUsers();
    } catch (error) {
      console.error('Add user error:', error);
      alert('❌ Erreur ajout utilisateur');
    }
  });
  
  document.getElementById('calibrationForm').addEventListener('submit', async (e) => {
  e.preventDefault();
  
  const config = {
    temperature: {
      useCalibration: document.getElementById('calib-temp-enable').checked,
      useTwoPoint: document.getElementById('calib-temp-mode').value === 'twopoint',
      offset: parseFloat(document.getElementById('calib-temp-offset').value),
      point1Raw: parseFloat(document.getElementById('calib-temp-p1-raw').value),
      point1Real: parseFloat(document.getElementById('calib-temp-p1-real').value),
      point2Raw: parseFloat(document.getElementById('calib-temp-p2-raw').value),
      point2Real: parseFloat(document.getElementById('calib-temp-p2-real').value)
    },
    pressure: {
      useCalibration: document.getElementById('calib-pressure-enable').checked,
      useTwoPoint: document.getElementById('calib-pressure-mode').value === 'twopoint',
      offset: parseFloat(document.getElementById('calib-pressure-offset').value),
      point1Raw: parseFloat(document.getElementById('calib-pressure-p1-raw').value),
      point1Real: parseFloat(document.getElementById('calib-pressure-p1-real').value),
      point2Raw: parseFloat(document.getElementById('calib-pressure-p2-raw').value),
      point2Real: parseFloat(document.getElementById('calib-pressure-p2-real').value)
    }
  };
  
  try {
    await fetch('/api/calibration', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify(config)
    });
    alert('✅ Calibration sauvegardée !');
  } catch (error) {
    console.error('Calibration save error:', error);
    alert('❌ Erreur sauvegarde calibration');
  }
});

  checkAuth();
});