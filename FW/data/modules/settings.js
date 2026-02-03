// ============================================================================
// SETTINGS.JS - Gestion des paramètres système (MQTT, Weather, System)
// ============================================================================

/**
 * Charge tous les paramètres du système
 * - Configuration MQTT
 * - Configuration météo
 * - Configuration système
 * - Informations système
 */
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
      badge.innerHTML = '<span data-i18n="connected">' + (t('connected') || 'Connecté') + '</span>';
      badge.className = 'badge success';
    } else {
      badge.innerHTML = '<span data-i18n="disconnected">' + (t('disconnected') || 'Déconnecté') + '</span>';
      badge.className = 'badge warning';
    }
    // Appliquer les traductions après modification du DOM
    if (typeof PoolDashboard.updateDynamicTranslations === 'function') {
      PoolDashboard.updateDynamicTranslations();
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
  PoolCalibration.loadCalibration().catch(err => {
    console.error('Calibration load error:', err);
  });
}

/**
 * Charge les informations système
 * - Version du firmware
 * - Adresse IP
 * - Temps de fonctionnement (uptime)
 * - Mémoire disponible
 */
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

/**
 * Teste la connexion MQTT
 * Affiche un message et vérifie les logs série
 */
async function testMQTT() {
  alert('Test de connexion MQTT...\nVérifiez les logs série pour le résultat.');
}

/**
 * Republie la découverte Home Assistant via MQTT
 * Permet de forcer la détection des entités par Home Assistant
 */
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

/**
 * Redémarre le système ESP32
 * Affiche une confirmation avant redémarrage
 */
async function restartSystem() {
  if (!confirm('⚠️ ' + (t('restart_system_confirm') || 'Redémarrer le système ?\n\nCela prendra environ 30 secondes.'))) return;
  
  try {
    await fetch('/api/system/restart', {method: 'POST'});
    alert('✅ Redémarrage en cours...\n\nReconnectez-vous dans 30 secondes.');
    PoolAuth.logout();
  } catch (error) {
    console.error('Restart error:', error);
  }
}

/**
 * Sauvegarde la configuration MQTT
 * @param {Event} e - Événement du formulaire
 */
async function saveMQTTConfig(e) {
  if (e) e.preventDefault();
  
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
    
    alert('✅ ' + (t('mqtt_saved') || 'Configuration MQTT sauvegardée !'));
    loadSettings();
  } catch (error) {
    console.error('[SETTINGS] Erreur sauvegarde MQTT:', error);
    alert('❌ ' + (t('mqtt_save_error') || 'Erreur sauvegarde MQTT'));
  }
}

/**
 * Sauvegarde la configuration météo
 * @param {Event} e - Événement du formulaire
 */
async function saveWeatherConfig(e) {
  if (e) e.preventDefault();
  
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
    
    alert('✅ ' + (t('weather_saved') || 'Configuration météo sauvegardée !'));
  } catch (error) {
    console.error('[SETTINGS] Erreur sauvegarde météo:', error);
    alert('❌ ' + (t('weather_save_error') || 'Erreur sauvegarde météo'));
  }
}

/**
 * Sauvegarde la configuration système
 * @param {Event} e - Événement du formulaire
 */
async function saveSystemConfig(e) {
  if (e) e.preventDefault();
  
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
    
    alert('✅ ' + (t('system_saved') || 'Configuration système sauvegardée !'));
  } catch (error) {
    console.error('[SETTINGS] Erreur sauvegarde système:', error);
    alert('❌ ' + (t('system_save_error') || 'Erreur sauvegarde système'));
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolSettings = {
  // Chargement
  loadSettings,
  loadSystemInfo,
  
  // Actions
  testMQTT,
  rediscoverHomeAssistant,
  restartSystem,
  
  // Sauvegarde
  saveMQTTConfig,
  saveWeatherConfig,
  saveSystemConfig,
};
