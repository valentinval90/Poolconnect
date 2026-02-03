// ============================================================================
// SCENARIOS.JS - Gestion des scénarios prédéfinis
// ============================================================================

/**
 * Affiche le modal des scénarios disponibles
 * Récupère la liste depuis l'ESP32 et l'affiche dans une interface modale
 */
async function showScenariosModal() {
  try {
    const response = await fetch('/api/scenarios');
    const scenarios = await response.json();
    
    let html = '<div class="modal active" id="scenariosModal">';
    html += '<div class="modal-content">';
    html += '<div class="modal-header">';
    html += `<h2>🎯 ${t('predefined_scenarios')}</h2>`;
    html += '<button class="close-btn" onclick="PoolScenarios.closeScenariosModal()">✕</button>';
    html += '</div>';
    html += '<div class="modal-body">';
    html += `<p class="info-text">${t('scenario_apply_info') || 'Appliquez un scénario type pour démarrer rapidement'}</p>`;
    html += '<div class="action-types-grid">';
    
    scenarios.forEach(scenario => {
      html += '<div class="action-type-card" onclick="PoolScenarios.applyScenario(' + scenario.id + ')">';
      // Utiliser data-i18n pour traduction dynamique
      const scenarioNameKey = 'scenario_' + scenario.id + '_name';
      const scenarioDescKey = 'scenario_' + scenario.id + '_desc';
      html += '<div class="action-type-name"><span data-i18n="' + scenarioNameKey + '">' + scenario.name + '</span></div>';
      html += '<div class="action-type-desc"><span data-i18n="' + scenarioDescKey + '">' + scenario.description + '</span></div>';
      html += '</div>';
    });
    
    html += '</div></div></div></div>';
    
    document.body.insertAdjacentHTML('beforeend', html);
    PoolDashboard.updateDynamicTranslations();
    
  } catch (error) {
    console.error('Scenarios error:', error);
    alert('❌ ' + (t('error_loading_scenarios') || 'Erreur chargement scénarios'));
  }
}

/**
 * Ferme le modal des scénarios
 */
function closeScenariosModal() {
  const modal = document.getElementById('scenariosModal');
  if (modal) modal.remove();
}

/**
 * Applique un scénario prédéfini
 * Crée un nouveau timer basé sur le scénario sélectionné
 * @param {number} scenarioId - ID du scénario à appliquer
 */
async function applyScenario(scenarioId) {
  if (!confirm(t('scenario_create_confirm') || 'Créer un timer depuis ce scénario ?')) return;
  
  try {
    const response = await fetch('/api/scenarios/apply', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ scenarioId })
    });
    
    if (response.ok) {
      alert('✅ ' + (t('scenario_applied') || 'Scénario appliqué !'));
      closeScenariosModal();
      PoolDashboard.showTab('timers');
    } else {
      const error = await response.text();
      alert('❌ ' + error);
    }
  } catch (error) {
    console.error('Apply scenario error:', error);
    alert('❌ ' + (t('error_applying_scenario') || 'Erreur application scénario'));
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolScenarios = {
  // Actions principales
  showScenariosModal,
  closeScenariosModal,
  applyScenario,
  
  // Consultation
  //getScenarioDetails,
  //listScenarios,
  //previewScenario,
  
  // Utilitaires
  // formatActionPreview
};
