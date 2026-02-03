// ============================================================================
// UNITS.JS - Système de gestion des unités (Température et Pression)
// ============================================================================

// Constantes d'unités
const UNITS = {
  TEMPERATURE: {
    CELSIUS: 'celsius',
    FAHRENHEIT: 'fahrenheit'
  },
  PRESSURE: {
    BAR: 'bar',
    PSI: 'psi'
  }
};

// État global des unités utilisateur
let userUnits = {
  temperature: UNITS.TEMPERATURE.CELSIUS,
  pressure: UNITS.PRESSURE.BAR
};

// ============================================================================
// CHARGEMENT ET SAUVEGARDE DES PRÉFÉRENCES D'UNITÉS
// ============================================================================

/**
 * Charge les préférences d'unités depuis localStorage et synchronise avec l'ESP32
 */
function loadUnitPreferences() {
  const savedUnits = localStorage.getItem('userUnits');
  if (savedUnits) {
    try {
      userUnits = JSON.parse(savedUnits);
    } catch (e) {
      console.error('Error loading unit preferences:', e);
    }
  }
  
  // CORRECTION: Vérifier l'existence avant d'appeler
  if (typeof window.PoolPreferences !== 'undefined' && 
      typeof window.PoolPreferences.loadPreferences === 'function') {
    window.PoolPreferences.loadPreferences();
  }
  
  updateUnitSelectors();
  updateUnitLabels();
}

/**
 * Sauvegarde les préférences d'unités dans localStorage et sur l'ESP32
 */
function saveUnitPreferences() {
  localStorage.setItem('userUnits', JSON.stringify(userUnits));
  
  // CORRECTION: Vérifier l'existence avant d'appeler
  if (typeof window.PoolPreferences !== 'undefined' && 
      typeof window.PoolPreferences.savePreferences === 'function') {
    window.PoolPreferences.savePreferences();
  }
}

/**
 * Définit l'unité de température et met à jour l'interface
 * @param {string} unit - 'celsius' ou 'fahrenheit'
 */
function setTemperatureUnit(unit) {
  userUnits.temperature = unit;
  saveUnitPreferences();
  updateUnitLabels();
  
  if (typeof window.PoolChart !== 'undefined' && 
      typeof window.PoolChart.updateChartUnits === 'function') {
    window.PoolChart.updateChartUnits();
  }
  
  if (typeof updateData === 'function') updateData();
}

/**
 * Définit l'unité de pression et met à jour l'interface
 * @param {string} unit - 'bar' ou 'psi'
 */
function setPressureUnit(unit) {
  userUnits.pressure = unit;
  saveUnitPreferences();
  updateUnitLabels();
  
  if (typeof window.PoolChart !== 'undefined' && 
      typeof window.PoolChart.updateChartUnits === 'function') {
    window.PoolChart.updateChartUnits();
  }
  
  if (typeof updateData === 'function') updateData();
}

// ============================================================================
// MISE À JOUR DE L'INTERFACE
// ============================================================================

/**
 * Met à jour les sélecteurs d'unités dans l'interface
 */
function updateUnitSelectors() {
  const tempSelector = document.getElementById('unit-temperature');
  const pressureSelector = document.getElementById('unit-pressure');
  
  if (tempSelector) tempSelector.value = userUnits.temperature;
  if (pressureSelector) pressureSelector.value = userUnits.pressure;
}

/**
 * Met à jour tous les labels d'unités dans l'interface
 */
function updateUnitLabels() {
  const tempUnit = getTemperatureUnitLabel();
  const pressureUnit = getPressureUnitLabel();
  
  // Labels dans les conditions du modal timer
  const condTempMinUnit = document.getElementById('cond-temp-min-unit');
  const condTempMaxUnit = document.getElementById('cond-temp-max-unit');
  const condExtTempUnit = document.getElementById('cond-ext-temp-unit');
  const condPressureUnit = document.getElementById('cond-pressure-unit');
  
  if (condTempMinUnit) condTempMinUnit.textContent = tempUnit;
  if (condTempMaxUnit) condTempMaxUnit.textContent = tempUnit;
  if (condExtTempUnit) condExtTempUnit.textContent = tempUnit;
  if (condPressureUnit) condPressureUnit.textContent = pressureUnit;
  
  // Labels dans l'étalonnage
  const calibTempUnit = document.getElementById('calib-temp-unit');
  const calibPressureUnit = document.getElementById('calib-pressure-unit');
  
  if (calibTempUnit) calibTempUnit.textContent = tempUnit;
  if (calibPressureUnit) calibPressureUnit.textContent = pressureUnit;
}

// ============================================================================
// FONCTIONS DE CONVERSION
// ============================================================================

/**
 * Convertit Celsius en Fahrenheit
 * @param {number} celsius - Température en °C
 * @returns {number} Température en °F
 */
function celsiusToFahrenheit(celsius) {
  return (celsius * 9/5) + 32;
}

/**
 * Convertit Bar en PSI
 * @param {number} bar - Pression en bar
 * @returns {number} Pression en PSI
 */
function barToPsi(bar) {
  return bar * 14.5038;
}

/**
 * Convertit une température selon l'unité cible
 * @param {number} celsius - Température en °C (unité de stockage)
 * @param {string} toUnit - Unité cible
 * @returns {number} Température convertie
 */
function convertTemperature(celsius, toUnit) {
  if (toUnit === UNITS.TEMPERATURE.FAHRENHEIT) {
    return celsiusToFahrenheit(celsius);
  }
  return celsius;
}

/**
 * Convertit une pression selon l'unité cible
 * @param {number} bar - Pression en bar (unité de stockage)
 * @param {string} toUnit - Unité cible
 * @returns {number} Pression convertie
 */
function convertPressure(bar, toUnit) {
  if (toUnit === UNITS.PRESSURE.PSI) {
    return barToPsi(bar);
  }
  return bar;
}

// ============================================================================
// FONCTIONS DE FORMATAGE POUR AFFICHAGE
// ============================================================================

/**
 * Formate une température pour l'affichage
 * @param {number} celsius - Température en °C (depuis stockage)
 * @param {boolean} includeUnit - Inclure le symbole d'unité
 * @returns {string} Température formatée
 */
function formatTemperature(celsius, includeUnit = true) {
  if (celsius === null || celsius === undefined || isNaN(celsius)) {
    return '--' + (includeUnit ? getTemperatureUnitLabel() : '');
  }
  
  const value = convertTemperature(celsius, userUnits.temperature);
  const rounded = Math.round(value * 10) / 10;
  
  if (includeUnit) {
    return rounded + getTemperatureUnitLabel();
  }
  return rounded;
}

/**
 * Formate une pression pour l'affichage
 * @param {number} bar - Pression en bar (depuis stockage)
 * @param {boolean} includeUnit - Inclure le symbole d'unité
 * @returns {string} Pression formatée
 */
function formatPressure(bar, includeUnit = true) {
  if (bar === null || bar === undefined || isNaN(bar)) {
    return '--' + (includeUnit ? ' ' + getPressureUnitLabel() : '');
  }
  
  const value = convertPressure(bar, userUnits.pressure);
  const rounded = Math.round(value * 100) / 100;
  
  if (includeUnit) {
    return rounded + ' ' + getPressureUnitLabel();
  }
  return rounded;
}

/**
 * Retourne le label de l'unité de température actuelle
 * @returns {string} '°C' ou '°F'
 */
function getTemperatureUnitLabel() {
  return userUnits.temperature === UNITS.TEMPERATURE.FAHRENHEIT ? '°F' : '°C';
}

/**
 * Retourne le label de l'unité de pression actuelle
 * @returns {string} 'bar' ou 'PSI'
 */
function getPressureUnitLabel() {
  return userUnits.pressure === UNITS.PRESSURE.PSI ? 'PSI' : 'bar';
}

// ============================================================================
// CONVERSION BIDIRECTIONNELLE (AFFICHAGE ↔ STOCKAGE)
// ============================================================================

/**
 * Convertit une valeur de température affichée vers le format de stockage (°C)
 * @param {number} displayValue - Valeur affichée dans l'unité utilisateur
 * @returns {number} Valeur en °C pour le stockage
 */
function temperatureToStorage(displayValue) {
  const value = parseFloat(displayValue);
  if (isNaN(value)) return 0;
  
  // Si l'unité affichée est °F, convertir en °C pour le stockage
  if (userUnits.temperature === UNITS.TEMPERATURE.FAHRENHEIT) {
    return (value - 32) * 5/9;
  }
  return value;
}

/**
 * Convertit une valeur de pression affichée vers le format de stockage (bar)
 * @param {number} displayValue - Valeur affichée dans l'unité utilisateur
 * @returns {number} Valeur en bar pour le stockage
 */
function pressureToStorage(displayValue) {
  const value = parseFloat(displayValue);
  if (isNaN(value)) return 0;
  
  // Si l'unité affichée est PSI, convertir en BAR pour le stockage
  if (userUnits.pressure === UNITS.PRESSURE.PSI) {
    return value / 14.5038;
  }
  return value;
}

/**
 * Convertit une valeur de température stockée (°C) vers l'unité d'affichage
 * @param {number} storageValue - Valeur stockée en °C
 * @returns {number} Valeur dans l'unité utilisateur
 */
function temperatureToDisplay(storageValue) {
  const value = parseFloat(storageValue);
  if (isNaN(value)) return 0;
  
  // Le stockage est toujours en °C, convertir en °F si nécessaire
  if (userUnits.temperature === UNITS.TEMPERATURE.FAHRENHEIT) {
    return celsiusToFahrenheit(value);
  }
  return value;
}

/**
 * Convertit une valeur de pression stockée (bar) vers l'unité d'affichage
 * @param {number} storageValue - Valeur stockée en bar
 * @returns {number} Valeur dans l'unité utilisateur
 */
function pressureToDisplay(storageValue) {
  const value = parseFloat(storageValue);
  if (isNaN(value)) return 0;
  
  // Le stockage est toujours en BAR, convertir en PSI si nécessaire
  if (userUnits.pressure === UNITS.PRESSURE.PSI) {
    return barToPsi(value);
  }
  return value;
}

// ============================================================================
// LABELS POUR LES GRAPHIQUES
// ============================================================================

/**
 * Retourne le label de température pour les graphiques avec traduction
 * @returns {string} Label traduit avec unité
 */
function getTemperatureChartLabel() {
  const unit = getTemperatureUnitLabel();
  const translateFunc = typeof t === 'function' ? t : (key) => key;
  return (translateFunc('chart_water_temp') || 'Température Eau') + ` (${unit})`;
}

/**
 * Retourne le label de pression pour les graphiques avec traduction
 * @returns {string} Label traduit avec unité
 */
function getPressureChartLabel() {
  const unit = getPressureUnitLabel();
  const translateFunc = typeof t === 'function' ? t : (key) => key;
  
  if (userUnits.pressure === UNITS.PRESSURE.PSI) {
    return (translateFunc('chart_pressure_psi') || 'Pression') + ` (${unit})`;
  }
  return (translateFunc('chart_pressure_x10') || 'Pression') + ' (bar x10)';
}

// ============================================================================
// EXPORT DES FONCTIONS ET VARIABLES
// ============================================================================

window.PoolUnits = {
  // Constantes
  UNITS,
  
  // État
  get userUnits() { return userUnits; },
  set userUnits(value) { userUnits = value; },
  
  // Gestion des préférences
  loadUnitPreferences,
  saveUnitPreferences,
  setTemperatureUnit,
  setPressureUnit,
  
  // Mise à jour interface
  updateUnitSelectors,
  updateUnitLabels,
  
  // Conversions de base
  celsiusToFahrenheit,
  barToPsi,
  convertTemperature,
  convertPressure,
  
  // Formatage
  formatTemperature,
  formatPressure,
  getTemperatureUnitLabel,
  getPressureUnitLabel,
  
  // Conversion bidirectionnelle
  temperatureToStorage,
  pressureToStorage,
  temperatureToDisplay,
  pressureToDisplay,
  
  // Graphiques
  getTemperatureChartLabel,
  getPressureChartLabel,
};

// Rendre les fonctions accessibles globalement pour rétrocompatibilité
Object.assign(window, window.PoolUnits);
