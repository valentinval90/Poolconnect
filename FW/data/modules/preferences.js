// ============================================================================
// PREFERENCES.JS - Gestion des préférences utilisateur
// ============================================================================

/**
 * Charge les préférences depuis l'ESP32 et les applique
 * Synchronise avec localStorage et les différents modules
 * @returns {Promise<boolean>} True si succès, false sinon
 */
async function loadPreferences() {
  console.log('[PREFS] Chargement des préférences depuis l\'ESP32...');
  
  try {
    const response = await fetch('/api/preferences');
    
    if (response.ok) {
      const prefs = await response.json();
      console.log('[PREFS] Préférences reçues:', prefs);
      
      let needsUpdate = false;
      
      // Appliquer la langue
      if (prefs.language && typeof setLanguage === 'function') {
        // Ne pas appeler setLanguage car cela déclencherait une sauvegarde
        // On met à jour directement
        if (typeof currentLanguage !== 'undefined') {
          currentLanguage = prefs.language;
          localStorage.setItem('language', prefs.language);
        }
        console.log('[PREFS] Langue appliquée:', prefs.language);
      }
      
      // Appliquer les unités
      if (prefs.temperatureUnit) {
        userUnits.temperature = prefs.temperatureUnit;
        needsUpdate = true;
      }
      if (prefs.pressureUnit) {
        userUnits.pressure = prefs.pressureUnit;
        needsUpdate = true;
      }
      if (needsUpdate) {
        saveUnitPreferences(); // Mettre à jour localStorage
        console.log('[PREFS] Unités appliquées:', userUnits);
      }
      
      // Appliquer le thème
      if (prefs.theme) {
        document.body.className = prefs.theme;
        localStorage.setItem('theme', prefs.theme);
        console.log('[PREFS] Thème appliqué:', prefs.theme);
      }
      
      // Appliquer l'intervalle du graphique
      if (prefs.chartUpdateInterval) {
        PoolChart.chartUpdateInterval = prefs.chartUpdateInterval;
        localStorage.setItem('chartInterval', PoolChart.chartUpdateInterval);
        console.log('[PREFS] Intervalle graphique appliqué:', PoolChart.chartUpdateInterval, 'ms');
      }
      
      // Appliquer les traductions si la fonction existe
      if (typeof applyTranslations === 'function') {
        applyTranslations();
      }
      
      console.log('[PREFS] ✓ Préférences chargées depuis l\'ESP32 avec succès');
      return true;
      
    } else {
      console.warn('[PREFS] Erreur HTTP:', response.status, '- Utilisation localStorage');
      return false;
    }
    
  } catch (error) {
    console.error('[PREFS] Erreur chargement:', error.message);
    console.log('[PREFS] Fallback sur localStorage');
    return false;
  }
}

/**
 * Sauvegarde les préférences sur l'ESP32
 * Collecte toutes les préférences depuis les différents modules
 * @returns {Promise<boolean>} True si succès, false sinon
 */
async function savePreferences() {
  const prefs = {
    language: typeof currentLanguage !== 'undefined' ? currentLanguage : 'fr',
    temperatureUnit: userUnits.temperature,
    pressureUnit: userUnits.pressure,
    theme: localStorage.getItem('theme') || 'light',
	chartUpdateInterval: PoolChart.chartUpdateInterval || 300000
  };
  
  console.log('[PREFS] Sauvegarde vers ESP32:', prefs);
  
  try {
    const response = await fetch('/api/preferences', {
      method: 'POST',
      headers: { 
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(prefs)
    });
    
    if (response.ok) {
      const result = await response.json();
      console.log('[PREFS] ✓ Sauvegardées sur ESP32:', result);
      return true;
    } else {
      const error = await response.text();
      console.error('[PREFS] ✗ Erreur HTTP:', response.status, error);
      return false;
    }
    
  } catch (error) {
    console.error('[PREFS] ✗ Erreur sauvegarde:', error.message);
    return false;
  }
}

/**
 * Réinitialise toutes les préférences aux valeurs par défaut
 * @returns {Promise<boolean>} True si succès
 */
async function resetPreferences() {
  const defaultPrefs = {
    language: 'fr',
    temperatureUnit: 'celsius',
    pressureUnit: 'bar',
    theme: 'light',
    chartUpdateInterval: 300000
  };
  
  console.log('[PREFS] Réinitialisation aux valeurs par défaut');
  
  // Mettre à jour les variables locales
  if (typeof currentLanguage !== 'undefined') {
    currentLanguage = defaultPrefs.language;
  }
  if (typeof userUnits !== 'undefined') {
    userUnits.temperature = defaultPrefs.temperatureUnit;
    userUnits.pressure = defaultPrefs.pressureUnit;
  }
  if (typeof chartUpdateInterval !== 'undefined') {
    chartUpdateInterval = defaultPrefs.chartUpdateInterval;
  }
  
  // Mettre à jour localStorage
  localStorage.setItem('language', defaultPrefs.language);
  localStorage.setItem('theme', defaultPrefs.theme);
  localStorage.setItem('chartInterval', defaultPrefs.chartUpdateInterval.toString());
  localStorage.setItem('userUnits', JSON.stringify({
    temperature: defaultPrefs.temperatureUnit,
    pressure: defaultPrefs.pressureUnit
  }));
  
  // Appliquer le thème
  document.body.className = defaultPrefs.theme;
  
  // Sauvegarder sur l'ESP32
  const success = await savePreferences();
  
  // Recharger la page pour appliquer tous les changements
  if (success) {
    setTimeout(() => {
      location.reload();
    }, 500);
  }
  
  return success;
}

/**
 * Exporte les préférences actuelles vers un objet
 * @returns {object} Objet contenant toutes les préférences
 */
function exportPreferences() {
  return {
    language: typeof currentLanguage !== 'undefined' ? currentLanguage : 'fr',
    temperatureUnit: typeof userUnits !== 'undefined' ? userUnits.temperature : 'celsius',
    pressureUnit: typeof userUnits !== 'undefined' ? userUnits.pressure : 'bar',
    theme: localStorage.getItem('theme') || 'light',
    chartUpdateInterval: typeof chartUpdateInterval !== 'undefined' ? chartUpdateInterval : 300000
  };
}

/**
 * Importe des préférences depuis un objet
 * @param {object} prefs - Objet contenant les préférences
 * @returns {Promise<boolean>} True si succès
 */
async function importPreferences(prefs) {
  console.log('[PREFS] Import des préférences:', prefs);
  
  // Valider les préférences
  if (!prefs || typeof prefs !== 'object') {
    console.error('[PREFS] Format de préférences invalide');
    return false;
  }
  
  // Appliquer chaque préférence si elle existe
  if (prefs.language && typeof setLanguage === 'function') {
    setLanguage(prefs.language);
  }
  
  if (prefs.temperatureUnit && typeof setTemperatureUnit === 'function') {
    setTemperatureUnit(prefs.temperatureUnit);
  }
  
  if (prefs.pressureUnit && typeof setPressureUnit === 'function') {
    setPressureUnit(prefs.pressureUnit);
  }
  
  if (prefs.theme && typeof setTheme === 'function') {
    PoolTheme.setTheme(prefs.theme);
  }
  
  if (prefs.chartUpdateInterval && typeof updateChartInterval === 'function') {
    updateChartInterval(prefs.chartUpdateInterval);
  }
  
  // Sauvegarder toutes les modifications
  return await savePreferences();
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolPreferences = {
  loadPreferences,
  savePreferences,
  resetPreferences,
  exportPreferences,
  importPreferences
};
