// Pool Connect Pro v4.0 - JavaScript Complet et Corrigé
// Gestion de l'interface web moderne avec onglets

document.addEventListener('DOMContentLoaded', () => {
	
  PoolUnits.loadUnitPreferences();
  PoolPreferences.loadPreferences();
  PoolTheme.initTheme();
  
  // Appliquer les traductions initiales après chargement
  setTimeout(() => {
    if (typeof applyTranslations === 'function') {
      applyTranslations();
    }
    if (typeof updateLanguageSelector === 'function') {
      updateLanguageSelector();
    }
  }, 100);
  // Créer le modal d'édition d'action
  PoolTimers.createActionEditorModal();
  
  document.getElementById('loginForm').addEventListener('submit', (e) => {
    e.preventDefault();
    const username = document.getElementById('username').value;
    const password = document.getElementById('loginPassword').value;
    PoolAuth.login(username, password);
  });
  
  document.getElementById('logoutBtn').addEventListener('click', PoolAuth.logout);
  
  // Event listener pour le changement d'intervalle graphique
	const chartIntervalSelect = document.getElementById('chart-interval-select');
	if (chartIntervalSelect) {
	  chartIntervalSelect.addEventListener('change', (e) => {
		const newInterval = parseInt(e.target.value);
		PoolChart.updateChartInterval(newInterval);
	  });
	  
	  // Initialiser l'affichage avec la valeur actuelle
	  chartIntervalSelect.value = PoolChart.chartUpdateInterval.toString();
	  PoolChart.updateChartIntervalDisplay();
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
      PoolSettings.loadSettings();
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
      PoolUsers.loadUsers();
    } catch (error) {
      console.error('Add user error:', error);
      alert('❌ Erreur ajout utilisateur');
    }
  });
  
  const monElement = document.getElementById('calibrationForm');
  if (monElement) {
	  monElement.addEventListener('submit', async (e) => {
	  
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
  }
  
  PoolAuth.checkAuth();
});