// ============================================================================
// TIMERS.JS - Système de timers flexibles
// ============================================================================

// Variables globales des timers
let timers = [];
let currentEditTimer = null;
let currentEditAction = null;

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
    listEl.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">' + (t('no_timers') || 'Aucun timer configuré') + '</p>';
    countBadge.textContent = '0';
    return;
  }
  
  countBadge.textContent = timers.length;
  
  let html = '';
  timers.forEach(timer => {
    const stateClass = timer.context.state === 2 ? 'active' : 
                      !timer.enabled ? 'disabled' :
                      timer.context.state === 5 ? 'error' : '';
    
    const stateText = timer.context.state === 2 ? '▶️ ' + (t('timer_running') || 'En cours') :
                     !timer.enabled ? '⏸️ ' + (t('timer_disabled') || 'Désactivé') :
                     timer.context.state === 5 ? '❌ ' + (t('timer_error') || 'Erreur') :
                     '⏹️ ' + (t('timer_inactive') || 'Inactif');
    
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
            <button class="btn btn-sm" onclick="editTimer(${timer.id})">✏️ ${t('edit') || 'Modifier'}</button>
            <button class="btn btn-sm btn-danger" onclick="deleteFlexTimer(${timer.id})">🗑️</button>
          </div>
        </div>
        
        <div class="timer-flex-info">
          <div class="timer-info-item">
            <span class="timer-info-label">${t('state') || 'État'}</span>
            <span class="timer-info-value">${stateText}</span>
          </div>
          <div class="timer-info-item">
            <span class="timer-info-label">${t('start_label') || 'Démarrage'}</span>
            <span class="timer-info-value">${startTime}</span>
          </div>
          <div class="timer-info-item">
            <span class="timer-info-label">${t('days_label') || 'Jours'}</span>
            <span class="timer-info-value">${days}</span>
          </div>
          <div class="timer-info-item">
            <span class="timer-info-label">${t('actions') || 'Actions'}</span>
            <span class="timer-info-value">${timer.actionCount} ${t('steps') || 'étape(s)'}</span>
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
  const dayNames = [
    t('day_sunday') || 'Dim',
    t('day_monday') || 'Lun',
    t('day_tuesday') || 'Mar',
    t('day_wednesday') || 'Mer',
    t('day_thursday') || 'Jeu',
    t('day_friday') || 'Ven',
    t('day_saturday') || 'Sam'
  ];
  const activeDays = [];
  for (let i = 0; i < 7; i++) {
    if (days[i]) activeDays.push(dayNames[i]);
  }
  return activeDays.length === 7 ? (t('every_day') || 'Tous les jours') : activeDays.join(', ');
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
// ÉDITEUR DE TIMER
// ============================================================================

function showTimerEditor(timerId) {
  currentEditingTimer = timerId;
  timerActions = [];
  
  document.getElementById('timerEditorModal').classList.add('active');
  
  if (timerId) {
    document.getElementById('timerEditorTitle').textContent = '✏️ ' + (t('edit_timer') || 'Modifier Timer');
    loadTimerForEdit(timerId);
  } else {
    document.getElementById('timerEditorTitle').textContent = '➕ ' + (t('new_timer') || 'Nouveau Timer');
    resetTimerEditor();
  }
  
  showEditorTab('config');
  
  // Appliquer les traductions
  if (typeof PoolDashboard.updateDynamicTranslations === 'function') {
    setTimeout(PoolDashboard.updateDynamicTranslations, 100);
  }
}

function editTimer(timerId) {
  showTimerEditor(timerId);
}

async function loadTimerForEdit(timerId) {
  try {
    const response = await fetch('/api/timers/flex');
    const timers = await response.json();
    const timer = timers.find(t => t.id === timerId);
    
    if (!timer) {
      alert(t('timer_not_found') || 'Timer non trouvé');
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
            document.getElementById('cond-temp-min-val').value = temperatureToDisplay(cond.value).toFixed(1);
            break;
          case 3:
            document.getElementById('cond-temp-max').checked = true;
            document.getElementById('cond-temp-max-val').value = temperatureToDisplay(cond.value).toFixed(1);
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
    console.error((t('error') || 'Erreur') + ' chargement timer:', error);
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
// GESTION DES ACTIONS
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
    listEl.innerHTML = '<p style="text-align: center; color: #999; padding: 30px;">' + (t('no_actions') || 'Aucune action. Cliquez sur "Ajouter une action" pour commencer.') + '</p>';
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
    1: t('action_wait_title') || 'Attendre',
    4: t('action_measure_title') || 'Mesurer Température',
    5: t('action_auto_title') || 'Durée Automatique',
    7: t('action_buzzer_title') || 'Buzzer',
    8: t('action_led_title') || 'LED'
  };
  return names[action.type] || (t('actions') || 'Action');
}

function getActionDescription(action) {
  let desc = action.description || '';
  
  if (action.type === 5) {
    if (action.customEquation && action.customEquation.useCustom) {
      desc = `📐 ${action.customEquation.expression}`;
    } else {
      desc = '📐 ' + (t('temp_div_2_default') || 'Temp / 2 (défaut, 3h-24h)');
    }
  }
  
  // Détails spécifiques buzzer (type 7)
  if (action.type === 7) {
    if (action.buzzerCount === 0) {
      desc = t('continuous_alarm') || 'Alarme continue';
    } else {
      desc = `${action.buzzerCount} ${t('beeps') || 'bip(s)'}`;
    }
  }
  
  // Détails spécifiques LED (type 8)
  if (action.type === 8) {
    const colors = [
      t('led_off') || 'Off',
      t('led_blue') || 'Bleu',
      t('led_green') || 'Vert',
      t('led_cyan') || 'Cyan',
      t('led_red') || 'Rouge',
      t('led_magenta') || 'Magenta',
      t('led_yellow') || 'Jaune',
      t('led_white') || 'Blanc'
    ];
    const modes = [
      t('led_steady') || 'Fixe',
      t('led_blinking') || 'Clignotant',
      t('led_pulsing') || 'Pulsation'
    ];
    desc = `${colors[action.ledColor] || 'Off'} - ${modes[action.ledMode] || 'Fixe'}`;
    if (action.ledDuration > 0) {
      desc += ` (${action.ledDuration}s)`;
    }
  }
  
  if (action.delayMinutes > 0 && action.type !== 1) {
    desc += ` (${t('after') || 'après'} ${action.delayMinutes} min)`;
  }
  
  return desc;
}

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
    
    const colors = [
      t('led_off') || 'Off',
      t('led_blue') || 'Bleu',
      t('led_green') || 'Vert',
      t('led_cyan') || 'Cyan',
      t('led_red') || 'Rouge',
      t('led_magenta') || 'Magenta',
      t('led_yellow') || 'Jaune',
      t('led_white') || 'Blanc'
    ];
    const modes = [
      t('led_steady') || 'Fixe',
      t('led_blinking') || 'Clignotant',
      t('led_pulsing') || 'Pulsation'
    ];
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
  if (confirm(t('delete_action_confirm') || 'Supprimer cette action ?')) {
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
  
  const name = document.getElementById('edit-timer-name').value || (t('timer_unnamed') || 'Timer sans nom');
  const enabled = document.getElementById('edit-timer-enabled').checked;
  const startType = parseInt(document.getElementById('edit-start-type').value);
  
  let startText = '';
  if (startType === 0) {
    const h = document.getElementById('edit-start-hour').value;
    const m = document.getElementById('edit-start-minute').value;
    startText = `${h.padStart(2, '0')}:${m.padStart(2, '0')}`;
  } else if (startType === 1) {
    startText = `${t('sunrise') || 'Lever du soleil'} +${document.getElementById('edit-sun-offset').value}min`;
  } else {
    startText = `${t('sunset') || 'Coucher du soleil'} +${document.getElementById('edit-sun-offset').value}min`;
  }
  
  const dayNames = [
    t('day_sunday') || 'Dim',
    t('day_monday') || 'Lun',
    t('day_tuesday') || 'Mar',
    t('day_wednesday') || 'Mer',
    t('day_thursday') || 'Jeu',
    t('day_friday') || 'Ven',
    t('day_saturday') || 'Sam'
  ];
  
  const days = [];
  document.querySelectorAll('.day-btn').forEach((btn, i) => {
    if (btn.classList.contains('active')) {
      days.push(dayNames[i]);
    }
  });
  
  const conditions = [];
  if (document.getElementById('cond-cover-open').checked) {
    conditions.push('🎚️ ' + (t('cover_open') || 'Volet ouvert'));
  }
  if (document.getElementById('cond-temp-min').checked) {
    conditions.push(`🌡️ ${t('water_abbr') || 'Eau'} ≥ ${formatTemperature(parseFloat(document.getElementById('cond-temp-min-val').value), false)}${getTemperatureUnitLabel()}`);
  }
  if (document.getElementById('cond-temp-max').checked) {
    conditions.push(`🌡️ ${t('water_abbr') || 'Eau'} ≤ ${formatTemperature(parseFloat(document.getElementById('cond-temp-max-val').value), false)}${getTemperatureUnitLabel()}`);
  }
  
  let html = `
    <div style="background: white; padding: 20px; border-radius: 12px; margin-bottom: 20px;">
      <h3 style="margin-bottom: 15px;">${name} ${enabled ? '✅' : '⏸️'}</h3>
      <p><strong>${t('start_label') || 'Démarrage'}:</strong> ${startText}</p>
      <p><strong>${t('days_label') || 'Jours'}:</strong> ${days.join(', ') || (t('none') || 'Aucun')}</p>
      ${conditions.length > 0 ? `<p><strong>${t('conditions') || 'Conditions'}:</strong> ${conditions.join(', ')}</p>` : ''}
    </div>
    
    <h4 style="margin: 20px 0 15px;">${t('actions_timeline') || 'Timeline des actions'}:</h4>
    <div class="preview-timeline">
  `;
  
  if (timerActions.length === 0) {
    html += '<p style="color: #999;">' + (t('no_action_defined') || 'Aucune action définie') + '</p>';
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
      value: temperatureToStorage(document.getElementById('cond-temp-min-val').value),
      required: true
    });
  }
  if (document.getElementById('cond-temp-max').checked) {
    timer.conditions.push({
      type: 3,
      value: temperatureToStorage(document.getElementById('cond-temp-max-val').value),
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
      alert('❌ ' + (t('error') || 'Erreur') + ': ' + error);
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
  if (!confirm(t('delete_timer_confirm') || 'Supprimer ce timer ?')) return;
  
  try {
    await fetch(`/api/timers/flex/${id}`, { method: 'DELETE' });
    loadFlexibleTimers();
  } catch (error) {
    console.error('Erreur suppression:', error);
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
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolTimers = {
  // Variables
  get timers() { return typeof timers !== 'undefined' ? timers : []; },
  set timers(value) { if (typeof timers !== 'undefined') timers = value; },
  get currentEditTimer() { return currentEditTimer; },
  set currentEditTimer(value) { currentEditTimer = value; },
  get currentEditAction() { return currentEditAction; },
  set currentEditAction(value) { currentEditAction = value; },
  
	// SYSTÈME DE TIMERS FLEXIBLES
	loadFlexibleTimers,
	displayFlexibleTimers,
	getTimerProgress,
	getDaysText,
	getStartTimeText,

	// ÉDITEUR DE TIMER
	showTimerEditor,
	editTimer,
	loadTimerForEdit,
	closeTimerEditor,
	showEditorTab,
	resetTimerEditor,
	updateStartTimeInputs,
	toggleDay,

	//GESTION DES ACTIONS
	showActionSelector,
	closeActionSelector,
	addActionType,
	updateActionsList,
	getActionTypeName,
	getActionDescription,
	editActionForm,
	updateBuzzerForm,
	updateAutoEquationForm,
	saveActionEdit,
	closeActionEditor,
	deleteAction,
	dragStart,
	dragOver,
	drop,

	// APERÇU DU TIMER
	generateTimerPreview,

	// SAUVEGARDE
	saveTimer,
	toggleTimerEnabled,
	deleteFlexTimer,

	//Créer le modal d'édition d'action
	createActionEditorModal,
  
  // Utilitaires
  formatTimerDisplay: typeof formatTimerDisplay !== 'undefined' ? formatTimerDisplay : null,
  getActionName: typeof getActionName !== 'undefined' ? getActionName : null,
  
  // Conditions
  addCondition: typeof addCondition !== 'undefined' ? addCondition : null,
  removeCondition: typeof removeCondition !== 'undefined' ? removeCondition : null,
  
  // Calibration
  loadCalibrationTab: typeof loadCalibrationTab !== 'undefined' ? loadCalibrationTab : null
};