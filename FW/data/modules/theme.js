// ============================================================================
// THEME.JS - Gestion du thème (Dark/Light)
// ============================================================================

/**
 * Initialise le thème au chargement de la page
 * Applique le thème sauvegardé ou le thème par défaut
 */
function initTheme() {
  const savedTheme = localStorage.getItem('theme') || 'light';
  document.documentElement.setAttribute('data-theme', savedTheme);
  updateThemeButton();
    if (typeof initLanguage === 'function') {
    initLanguage();
  }
}

/**
 * Bascule entre le thème clair et sombre
 */
function toggleTheme() {
  const current = document.documentElement.getAttribute('data-theme') || 'light';
  const newTheme = current === 'light' ? 'dark' : 'light';
  document.documentElement.setAttribute('data-theme', newTheme);
  localStorage.setItem('theme', newTheme);
  savePreferences();
  updateThemeButton();
}

/**
 * Définit le thème
 * @param {string} theme - 'light' ou 'dark'
 * @param {boolean} save - Sauvegarder la préférence (default: true)
 */
function setTheme(theme, save = true) {
  // Valider le thème
  if (theme !== 'light' && theme !== 'dark') {
    console.warn('[THEME] Thème invalide:', theme, '- Utilisation de "light"');
    theme = 'light';
  }
  
  // Appliquer le thème
  document.documentElement.setAttribute('data-theme', theme);
  document.body.className = theme;
  
  // Sauvegarder si demandé
  if (save) {
    localStorage.setItem('theme', theme);
    
    // Sauvegarder sur l'ESP32
    if (typeof savePreferences === 'function') {
      savePreferences();
    }
  }
  
  // Mettre à jour le bouton
  updateThemeButton();
  
  console.log('[THEME] Thème appliqué:', theme);
}

/**
 * Retourne le thème actuel
 * @returns {string} 'light' ou 'dark'
 */
function getCurrentTheme() {
  return document.documentElement.getAttribute('data-theme') || 'light';
}

/**
 * Met à jour l'icône du bouton de basculement du thème
 */
function updateThemeButton() {
  const btn = document.getElementById('themeToggle');
  if (btn) {
    const theme = document.documentElement.getAttribute('data-theme');
    btn.textContent = theme === 'dark' ? '☀️' : '🌙';
  }
}

/**
 * Vérifie si le thème sombre est actif
 * @returns {boolean} True si mode sombre
 */
function isDarkMode() {
  return getCurrentTheme() === 'dark';
}

/**
 * Vérifie si le thème clair est actif
 * @returns {boolean} True si mode clair
 */
function isLightMode() {
  return getCurrentTheme() === 'light';
}

/**
 * Active le mode sombre
 */
function enableDarkMode() {
  setTheme('dark', true);
}

/**
 * Active le mode clair
 */
function enableLightMode() {
  setTheme('light', true);
}

/**
 * Détecte la préférence système de l'utilisateur
 * @returns {string} 'dark' ou 'light'
 */
function detectSystemTheme() {
  if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
    return 'dark';
  }
  return 'light';
}

/**
 * Applique le thème système de l'utilisateur
 */
function useSystemTheme() {
  const systemTheme = detectSystemTheme();
  setTheme(systemTheme, true);
}

/**
 * Écoute les changements de préférence système
 * Active automatiquement si l'option est activée
 */
function watchSystemTheme() {
  if (window.matchMedia) {
    const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)');
    
    mediaQuery.addEventListener('change', (e) => {
      const newTheme = e.matches ? 'dark' : 'light';
      console.log('[THEME] Changement détecté dans les préférences système:', newTheme);
      
      // Ne changer que si l'option "suivre le système" est activée
      if (localStorage.getItem('followSystemTheme') === 'true') {
        setTheme(newTheme, true);
      }
    });
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolTheme = {
  // Initialisation
  detectSystemTheme,
  initTheme,
  
  // Actions principales
  toggleTheme,
  setTheme,
  getCurrentTheme,
  
  // Vérifications
  isDarkMode,
  isLightMode,
  
  // Actions spécifiques
  enableDarkMode,
  enableLightMode,
  
  // Thème système
  detectSystemTheme,
  useSystemTheme,
  watchSystemTheme,
  
  // Interface
  updateThemeButton
};
