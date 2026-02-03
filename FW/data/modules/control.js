// ============================================================================
// CONTROL.JS - Gestion du contrôle manuel des équipements
// ============================================================================

/**
 * Met à jour l'affichage de l'onglet Contrôle
 * - États des relais (pompe, électrolyseur, etc.)
 * - État du volet
 * - Détection de fuite
 */
async function updateControl() {
  try {
    const relays = await fetch('/api/relays').then(r => r.json());
    relays.forEach((state, i) => {
      const checkbox = document.getElementById('ctrl-relay' + i);
      if (checkbox) checkbox.checked = state;
    });
    
    const sensors = await fetch('/api/sensors').then(r => r.json());
    document.getElementById('ctrl-volet').textContent = sensors.coverOpen ? (t('open') || 'Ouvert') : (t('closed') || 'Fermé');
    document.getElementById('ctrl-fuite').textContent = sensors.waterLeak ? '⚠️ ' + (t('leak_detected') || 'FUITE') : '✓ ' + (t('ok') || 'OK');
    
  } catch (error) {
    console.error('Control update error:', error);
  }
}

/**
 * Active/désactive un relais (équipement)
 * @param {number} relay - Numéro du relais (0-4)
 * @param {boolean} state - État souhaité (true = ON, false = OFF)
 */
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
      PoolChart.addChartDataPoint();
    }
  } catch (error) {
    console.error('Relay toggle error:', error);
  }
}

/**
 * Active/désactive le buzzer
 * @param {boolean} enabled - État du buzzer (true = activé, false = désactivé/muet)
 */
async function toggleBuzzer(enabled) {
  try {
    await fetch(`/api/buzzer/mute?state=${enabled ? 0 : 1}`);
  } catch (error) {
    console.error('Buzzer toggle error:', error);
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolControl = {
  // Fonctions principales
  updateControl,
  toggleRelay,
  toggleBuzzer,
};
