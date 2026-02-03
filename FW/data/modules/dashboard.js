// ============================================================================
// DASHBOARD.JS - Gestion du tableau de bord principal
// ============================================================================

// Variable pour suivre le nombre de timers actifs (pour détecter les changements)
let lastActiveTimerCount = 0;

/**
 * Met à jour toutes les informations du dashboard
 * - Température de l'eau
 * - Pression et température extérieure
 * - Détection de fuite
 * - États des équipements (pompe, électrolyseur, etc.)
 * - Timers actifs
 * - Historique
 */
async function updateDashboard() {
  try {
    const temp = await fetch('/api/temp').then(r => r.text());
    document.getElementById('dash-water-temp').textContent = temp === 'ERREUR' ? 'ERR' : formatTemperature(parseFloat(temp));
    
    const sensors = await fetch('/api/sensors').then(r => r.json());
    document.getElementById('dash-pressure').textContent = formatPressure(sensors.waterPressure);
    document.getElementById('dash-ext-temp').textContent = (sensors.extTemp !== null && sensors.extTemp !== undefined) ? formatTemperature(sensors.extTemp) : '--' + getTemperatureUnitLabel();
    
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
    
    // Mettre à jour les états avec traductions
    updateEquipmentStateTranslations(relays);
    
    // Mettre à jour les classes CSS
    const equipmentNames = ['pompe', 'electro', 'lampe', 'valve', 'pac'];
    relays.forEach((state, i) => {
      const el = document.getElementById('eq-' + equipmentNames[i]);
      if (el) {
        el.className = 'equipment-state' + (state ? ' active' : '');
      }
    });
    
    // FIX: Utiliser /api/timers/flex au lieu de /api/timers
    const timersData = await fetch('/api/timers/flex').then(r => r.json()).catch(() => []);
    const activeTimers = timersData.filter(t => t.context && t.context.state === 2); // TIMER_RUNNING = 2
    document.getElementById('active-timers-count').textContent = activeTimers.length;
    
    if (activeTimers.length !== lastActiveTimerCount) {
      lastActiveTimerCount = activeTimers.length;
		if (typeof PoolChart.addChartDataPoint === 'function') {
		  PoolChart.addChartDataPoint();
		}
    }
    
    if (activeTimers.length > 0) {
      let html = '';
      activeTimers.forEach(timer => {
        html += `<div class="timer-item active">
          <div class="timer-info">
            <div class="timer-name">${timer.name}</div>
            <div class="timer-details">${t('timer_action') || 'Action'} ${timer.context.currentAction + 1}/${timer.actionCount}</div>
          </div>
        </div>`;
      });
      document.getElementById('active-timers-list').innerHTML = html;
    } else {
      document.getElementById('active-timers-list').innerHTML = 
        `<p style="text-align: center; color: #999; padding: 20px;">${t('no_active_timer')}</p>`;
    }
    
    loadHistoryDashboard();
    
  } catch (error) {
    console.error('Dashboard update error:', error);
  }
}

/**
 * Charge l'historique des dernières sessions de filtration
 * Affiche les 5 dernières entrées
 */
async function loadHistoryDashboard() {
  try {
    const history = await fetch('/api/history').then(r => r.json());
    const historyEl = document.getElementById('dash-history');
    
    if (!historyEl) return;
    
    if (!history || history.length === 0) {
      historyEl.innerHTML = `<p style="text-align: center; color: #999; padding: 20px;">${t('no_history') || 'Aucun historique'}</p>`;
      return;
    }
    
    let html = '';
    history.slice(0, 5).forEach(entry => {
      const durationH = Math.floor(entry.duration / 60);
      const durationM = entry.duration % 60;
      html += `<div class="history-item">
        📅 ${entry.date} - ⏰ ${entry.startTime} - 
        ⏱️ ${durationH}h${durationM.toString().padStart(2, '0')} - 
        🌡️ ${formatTemperature(entry.avgTemp)}
      </div>`;
    });
    
    historyEl.innerHTML = html;
    
  } catch (error) {
    console.error('[DASHBOARD] Erreur chargement historique:', error);
  }
}

/**
 * Met à jour les traductions des états des équipements
 * @param {Array} relays - États des relais
 */
function updateEquipmentStateTranslations(relays) {
  if (!relays || typeof t !== 'function') return;
  
  // Mettre à jour les états dans le dashboard
  const eqPompe = document.getElementById('eq-pompe');
  const eqElectro = document.getElementById('eq-electro');
  const eqLampe = document.getElementById('eq-lampe');
  const eqValve = document.getElementById('eq-valve');
  const eqPac = document.getElementById('eq-pac');
  
  if (eqPompe && relays.length > 0) {
    eqPompe.textContent = relays[0] ? t('eq_running') : t('eq_stopped');
  }
  if (eqElectro && relays.length > 1) {
    eqElectro.textContent = relays[1] ? t('eq_running') : t('eq_stopped');
  }
  if (eqLampe && relays.length > 2) {
    eqLampe.textContent = relays[2] ? t('eq_on') : t('eq_off');
  }
  if (eqValve && relays.length > 3) {
    eqValve.textContent = relays[3] ? t('eq_open') : t('eq_closed');
  }
  if (eqPac && relays.length > 4) {
    eqPac.textContent = relays[4] ? t('eq_running') : t('eq_stopped');
  }
}

/**
 * Rafraîchit l'affichage
 */
function updateDynamicTranslations() {
  if (typeof applyTranslations === 'function') {
    applyTranslations();
  }
}

/**
 * Rafraîchit la sidebar (heure, température, temps recommandé)
 */
async function updateSidebar() {
  try {
    // Mettre à jour l'heure/date
    const time = await fetch('/api/time').then(r => r.text());
    document.getElementById('datetime').textContent = '🕒 ' + time;
    
    // Mettre à jour la température de l'eau
    const temp = await fetch('/api/temp').then(r => r.text());
    if (temp !== 'ERREUR') {
      document.getElementById('temperature').textContent = formatTemperature(temp);
      
      // Calculer temps de filtration recommandé (Temp / 2)
      const tempValue = parseFloat(temp);
      if (!isNaN(tempValue)) {
        const heuresRecommandees = Math.round(tempValue / 2);
        document.getElementById('tempsFiltrationRecommande').textContent = heuresRecommandees + 'h';
      }
    } else {
      document.getElementById('temperature').textContent = 'ERR';
    }
    
  } catch (error) {
    console.error('Sidebar update error:', error);
  }
}

/**
 * Met à jour uniquement le statut de la pompe (temps restant)
 * Appelé plus fréquemment que updateSidebar (toutes les 1-2 secondes)
 */
async function updatePumpStatus() {
  try {
    const tempsFiltrationActuelEl = document.getElementById('tempsFiltrationActuel');
    
    // Vérifier que l'élément existe
    if (!tempsFiltrationActuelEl) {
      return;
    }
    
    // Récupérer le statut de la pompe depuis l'ESP32
    const pumpStatus = await fetch('/api/pump/status').then(r => r.json());
    
    if (!pumpStatus.pumpOn) {
      // Pompe arrêtée
      tempsFiltrationActuelEl.textContent = t('stopped') || 'Arrêté';
    } else if (pumpStatus.controlledByTimer && pumpStatus.remainingMinutes > 0) {
      // Pompe contrôlée par timer - afficher le temps restant fourni par l'ESP32
      const remainingMinutes = pumpStatus.remainingMinutes;
      
      // Afficher le format approprié
      if (remainingMinutes >= 60) {
        // Plus d'une heure : format "3h45"
        const hours = Math.floor(remainingMinutes / 60);
        const minutes = remainingMinutes % 60;
        tempsFiltrationActuelEl.textContent = `${hours}h${minutes.toString().padStart(2, '0')}`;
      } else if (remainingMinutes >= 1) {
        // Entre 1 et 59 minutes : format "25min"
        tempsFiltrationActuelEl.textContent = `${remainingMinutes}min`;
      } else {
        // Moins d'une minute : afficher "< 1min"
        tempsFiltrationActuelEl.textContent = '< 1min';
      }
    } else {
      // Pompe active mais pas contrôlée par un timer
      tempsFiltrationActuelEl.textContent = t('running') || 'En cours';
    }
    
  } catch (error) {
    console.error('Pump status update error:', error);
  }
}

/**
 * Navigation
 */
function showTab(tabName) {
  // Appeler updateDynamicTranslations après changement d'onglet
  setTimeout(() => { if (typeof updateDynamicTranslations === 'function') updateDynamicTranslations(); }, 100);
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
  if (tabName === 'control') PoolControl.updateControl();
  if (tabName === 'timers') loadFlexibleTimers();
  if (tabName === 'calibration') PoolCalibration.loadCalibrationTab();
  if (tabName === 'settings') PoolSettings.loadSettings();
  if (tabName === 'users' && PoolAuth.currentRole === 'admin') PoolUsers.loadUsers();
  updateDynamicTranslations();
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolDashboard = {
  // Fonctions principales
  updateDashboard,
  loadHistoryDashboard,
  updateEquipmentStateTranslations,
  updateDynamicTranslations,
  updateSidebar,
  updatePumpStatus,
  showTab,
  
  // Variables
  get lastActiveTimerCount() { return lastActiveTimerCount; },
  set lastActiveTimerCount(value) { lastActiveTimerCount = value; }
};

// Mettre à jour le statut de la pompe toutes les 2 secondes (uniquement le temps restant)
setInterval(() => {
  if (typeof PoolDashboard.updatePumpStatus === 'function') {
    PoolDashboard.updatePumpStatus();
  }
}, 2000);
