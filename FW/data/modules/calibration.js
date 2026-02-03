// ============================================================================
// CALIBRATION.JS - Gestion de l'étalonnage des capteurs
// ============================================================================

// Variables globales d'authentification
let calibrationAssistantStep = 0;
let calibrationMode = null; // 'temp' ou 'pressure'
let calibrationRawValues = [];
let calibrationRealValues = [];
let calibrationMonitor = null;

/**
 * Charge les paramètres d'étalonnage depuis l'ESP32
 * Affiche les valeurs dans le formulaire d'étalonnage
 */
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

/**
 * Charge l'onglet calibration (appelé lors du changement d'onglet)
 */
async function loadCalibrationTab() {
  try {
    const calib = await fetch('/api/calibration').then(r => r.json());
    const sensors = await fetch('/api/sensors').then(r => r.json());
    
    // Afficher valeurs actuelles
    document.getElementById('calib-current-temp-raw').textContent = formatTemperature(sensors.waterTemp, false);
    document.getElementById('calib-current-pressure-raw').textContent = formatPressure(sensors.waterPressure, false);
    
    // État étalonnage température
    const tempStatus = document.getElementById('calib-temp-status');
    if (calib.temperature.useCalibration) {
      const mode = calib.temperature.useTwoPoint ? '2 points' : 'Offset';
      tempStatus.innerHTML = '✅ ' + (t('active') || 'Actif') + ' (' + mode + ')';
      tempStatus.className = 'calib-status active';
    } else {
      tempStatus.innerHTML = '❌ ' + (t('disabled') || 'Désactivé');
      tempStatus.className = 'calib-status inactive';
    }
    
    // État étalonnage pression
    const pressStatus = document.getElementById('calib-pressure-status');
    if (calib.pressure.useCalibration) {
      const mode = calib.pressure.useTwoPoint ? '2 points' : 'Offset';
      pressStatus.innerHTML = '✅ ' + (t('active') || 'Actif') + ' (' + mode + ')';
      pressStatus.className = 'calib-status active';
    } else {
      pressStatus.innerHTML = '❌ ' + (t('disabled') || 'Désactivé');
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

/**
 * Met à jour l'affichage des champs d'étalonnage de température
 * Affiche soit les champs offset, soit les champs deux points
 */
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

/**
 * Met à jour l'affichage des champs d'étalonnage de pression
 * Affiche soit les champs offset, soit les champs deux points
 */
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

/**
 * Réinitialise l'étalonnage aux valeurs par défaut
 */
async function resetCalibration() {
  if (!confirm('⚠️ ' + (t('reset_calibration_confirm') || 'Réinitialiser toute la calibration ?\n\nCela remettra les capteurs aux valeurs brutes.'))) {
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
// ONGLET ÉTALONNAGE DÉDIÉ
// ============================================================================

function startCalibrationAssistant(type) {
  calibrationMode = type;
  calibrationAssistantStep = 1;
  calibrationRawValues = [];
  calibrationRealValues = [];
  
  document.getElementById('calibAssistantModal').classList.add('active');
  
  if (type === 'temp') {
    document.getElementById('calib-assistant-title').textContent = '🌡️ ' + (t('calib_assistant_temp') || 'Assistant Étalonnage Température');
  } else {
    document.getElementById('calib-assistant-title').textContent = '💪 ' + (t('calib_assistant_pressure') || 'Assistant Étalonnage Pression');
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
  document.getElementById('calib-progress-text').textContent = (t('step') || 'Étape') + ' ' + step + '/5';
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
        const rawDisplay1 = calibrationMode === 'temp' ? 
          formatTemperature(calibrationRawValues[0], false) :
          formatPressure(calibrationRawValues[0], false);
        const realDisplay1 = calibrationMode === 'temp' ?
          formatTemperature(calibrationRealValues[0], false) :
          formatPressure(calibrationRealValues[0], false);
        document.getElementById('calib-point1-summary').textContent = 
          (t('point') || 'Point') + ' 1: Raw=' + rawDisplay1 + ', ' + (t('real') || 'Réel') + '=' + realDisplay1;
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
        // Convertir pour l'affichage selon l'unité sélectionnée
        const displayValue = calibrationMode === 'temp' ? 
          formatTemperature(value, false) : 
          formatPressure(value, false);
        displayElement.textContent = displayValue;
        
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
      alert('⚠️ ' + (t('enter_valid_value') || 'Veuillez entrer une valeur réelle valide'));
      return;
    }
    
    // Convertir la valeur réelle de l'affichage vers le stockage (toujours °C/BAR)
    const realValueForStorage = calibrationMode === 'temp' ?
      temperatureToStorage(realValue) :
      pressureToStorage(realValue);
    
    calibrationRawValues.push(rawValue);
    calibrationRealValues.push(realValueForStorage);
    
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
    alert('❌ ' + (t('capture_error') || 'Erreur lors de la capture'));
  }
}

function showCalibrationSummary() {
  const summaryDiv = document.getElementById('calib-summary');
  let html = '<div class="calib-summary-box">';
  
  html += '<h4>📊 ' + (t('summary') || 'Récapitulatif') + '</h4>';
  html += '<div class="calib-summary-line"><strong>' + (t('sensor') || 'Capteur') + ':</strong> ' + (calibrationMode === 'temp' ? (t('temperature') || 'Température') : (t('pressure') || 'Pression')) + '</div>';
  
  if (calibrationRawValues.length === 1) {
    // Mode offset
    const offset = calibrationRealValues[0] - calibrationRawValues[0];
    html += '<div class="calib-summary-line"><strong>' + (t('mode') || 'Mode') + ':</strong> ' + (t('simple_offset') || 'Offset simple') + '</div>';
    html += '<div class="calib-summary-line"><strong>' + (t('calculated_offset') || 'Offset calculé') + ':</strong> ' + (offset > 0 ? '+' : '') + offset.toFixed(3) + '</div>';
  } else {
    // Mode 2 points
    html += '<div class="calib-summary-line"><strong>' + (t('mode') || 'Mode') + ':</strong> ' + (t('two_point_calib') || 'Calibration 2 points') + '</div>';
    const rawDisplay1 = calibrationMode === 'temp' ? 
      formatTemperature(calibrationRawValues[0], false) :
      formatPressure(calibrationRawValues[0], false);
    const realDisplay1 = calibrationMode === 'temp' ?
      formatTemperature(calibrationRealValues[0], false) :
      formatPressure(calibrationRealValues[0], false);
    const rawDisplay2 = calibrationMode === 'temp' ? 
      formatTemperature(calibrationRawValues[1], false) :
      formatPressure(calibrationRawValues[1], false);
    const realDisplay2 = calibrationMode === 'temp' ?
      formatTemperature(calibrationRealValues[1], false) :
      formatPressure(calibrationRealValues[1], false);
    
    html += '<div class="calib-summary-line"><strong>' + (t('point') || 'Point') + ' 1:</strong> Raw=' + rawDisplay1 + ' → ' + (t('real') || 'Réel') + '=' + realDisplay1 + '</div>';
    html += '<div class="calib-summary-line"><strong>' + (t('point') || 'Point') + ' 2:</strong> Raw=' + rawDisplay2 + ' → ' + (t('real') || 'Réel') + '=' + realDisplay2 + '</div>';
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
      alert('❌ ' + (t('apply_error') || 'Erreur lors de l\'application'));
    }
    
  } catch (error) {
    console.error('Apply calibration error:', error);
    alert('❌ ' + (t('apply_error') || 'Erreur lors de l\'application'));
  }
}

async function testCalibration() {
  const testResults = document.getElementById('calib-test-results');
  testResults.innerHTML = '<p>🔄 ' + (t('testing') || 'Test en cours...') + '</p>';
  
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
    html += '<h4>✅ ' + (t('test_completed') || 'Test terminé') + '</h4>';
    html += `<div class="test-result-line"><strong>Moyenne:</strong> ${avg.toFixed(3)}</div>`;
    html += `<div class="test-result-line"><strong>Min:</strong> ${min.toFixed(3)}</div>`;
    html += `<div class="test-result-line"><strong>Max:</strong> ${max.toFixed(3)}</div>`;
    html += `<div class="test-result-line"><strong>Variance:</strong> ${variance.toFixed(3)}</div>`;
    
    if (variance < 0.5) {
      html += '<p style="color: var(--success); margin-top: 15px;">✅ ' + (t('excellent_stability') || 'Stabilité excellente') + '</p>';
    } else if (variance < 1.0) {
      html += '<p style="color: var(--warning); margin-top: 15px;">⚠️ Stabilité acceptable</p>';
    } else {
      html += '<p style="color: var(--danger); margin-top: 15px;">❌ Forte variation détectée</p>';
    }
    
    html += '</div>';
    testResults.innerHTML = html;
    
  } catch (error) {
    console.error('Test error:', error);
    testResults.innerHTML = '<p style="color: var(--danger);">❌ ' + (t('test_error') || 'Erreur lors du test') + '</p>';
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
  if (!confirm((t('disable_calibration_confirm') || 'Désactiver l\'étalonnage') + ' ' + (type === 'temp' ? (t('temperature') || 'température') : (t('pressure') || 'pression')) + ' ?')) {
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
    alert('❌ ' + (t('error') || 'Erreur'));
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolCalibration = {
  // Fonctions principales
  loadCalibration,
  loadCalibrationTab,
  startCalibrationAssistant,
  selectCalibrationMode,
  captureCalibrationPoint,
  applyCalibration,
  closeCalibrationAssistant,
  disableCalibration,
 
};