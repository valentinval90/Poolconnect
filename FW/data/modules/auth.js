// ============================================================================
// AUTH.JS - Gestion de l'authentification
// ============================================================================

// Variables globales d'authentification
let currentUser = null;
let currentRole = null;
let updateInterval = null;

/**
 * Connexion de l'utilisateur
 * @param {string} username - Nom d'utilisateur
 * @param {string} password - Mot de passe
 */
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
      
      // Sauvegarder la session
      sessionStorage.setItem('poolUser', currentUser);
      sessionStorage.setItem('poolRole', currentRole);
      
      // Charger les préférences utilisateur
      PoolUnits.loadUnitPreferences();    
	  
      showApp();	  
    } else {
      showLoginError();
    }
  } catch (error) {
    console.error('Login error:', error);
    showLoginError();
  }
}

/**
 * Affiche un message d'erreur de connexion
 */
function showLoginError() {
  const errorDiv = document.getElementById('loginError');
  errorDiv.classList.add('show');
  setTimeout(() => errorDiv.classList.remove('show'), 3000);
}

/**
 * Déconnexion de l'utilisateur
 */
function logout() {
  if (confirm(t('logout_confirm') || 'Êtes-vous sûr de vouloir vous déconnecter ?')) {
    // Réinitialiser les variables
    currentUser = null;
    currentRole = null;
    
    // Nettoyer la session
    sessionStorage.removeItem('poolUser');
    sessionStorage.removeItem('poolRole');
    
    // Arrêter les mises à jour
    if (updateInterval) {
      clearInterval(updateInterval);
    }   
    
    showLogin();
  }
}

/**
 * Vérifie si l'utilisateur est authentifié
 */
function checkAuth() {
  const user = sessionStorage.getItem('poolUser');
  const role = sessionStorage.getItem('poolRole');
  
  if (user && role) {
    currentUser = user;
    currentRole = role;
    
    // Charger les préférences
    PoolUnits.loadUnitPreferences();
   
    showApp();
  } else {
    showLogin();
  }
}

/**
 * Affiche l'écran de connexion
 */
function showLogin() {
  document.getElementById('loginContainer').classList.remove('hidden');
  document.getElementById('appContainer').classList.remove('active');
  document.getElementById('username').value = '';
  document.getElementById('loginPassword').value = '';
}

/**
 * Affiche l'application après connexion réussie
 */
function showApp() {
  document.getElementById('loginContainer').classList.add('hidden');
  document.getElementById('appContainer').classList.add('active');
  
  const usernameDisplay = document.querySelector('.logo small');
  if (usernameDisplay) {
    usernameDisplay.textContent = (t('connected') || 'Connecté') + ': ' + currentUser + ' (' + currentRole + ')';
  }
  
  if (currentRole === 'admin') {
    document.getElementById('usersTab').style.display = 'block';
  } else {
    document.getElementById('usersTab').style.display = 'none';
  }
  
  PoolDashboard.showTab('dashboard');
  
  // Actualiser la sidebar immédiatement
  PoolDashboard.updateSidebar();
  
  PoolChart.loadChartData();
  setTimeout(() => {
    PoolChart.initChart();
    PoolChart.addChartDataPoint();
  }, 500);

  PoolChart.startChartInterval();
  
  if (updateInterval) clearInterval(updateInterval);
  updateInterval = setInterval(() => {
	PoolDashboard.updateSidebar(); // Actualiser sidebar toutes les 2 secondes
    const activeTab = document.querySelector('.tab-content.active').id;
    if (activeTab === 'tab-dashboard') PoolDashboard.updateDashboard();
    if (activeTab === 'tab-control') PoolControl.updateControl();
  }, 2000);
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolAuth = {
  // Fonctions principales
  login,
  logout,
  checkAuth,
  
  // Variables (accessibles en lecture/écriture)
  get currentUser() { return currentUser; },
  set currentUser(value) { currentUser = value; },
  get currentRole() { return currentRole; },
  set currentRole(value) { currentRole = value; },
};
