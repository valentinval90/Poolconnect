// ============================================================================
// BACKUP.JS - Gestion des sauvegardes et restaurations
// ============================================================================

/**
 * Télécharge un backup de la configuration complète
 * Crée un fichier JSON avec toutes les données du système
 */
async function downloadBackup() {
  try {
    const response = await fetch('/api/backup/download');
    const json = await response.text();
    
    // Télécharger
    const blob = new Blob([json], { type: 'application/json' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    
    const date = new Date().toISOString().split('T')[0];
    a.download = `pool_backup_${date}.json`;
    a.click();
    window.URL.revokeObjectURL(url);
    
    alert('✅ ' + (typeof t === 'function' ? t('backup_downloaded') : 'Backup téléchargé'));
  } catch (error) {
    console.error('Backup error:', error);
    alert('❌ ' + (typeof t === 'function' ? t('backup_error') : 'Erreur téléchargement backup'));
  }
}

/**
 * Restaure un backup depuis un fichier JSON
 * Ouvre un sélecteur de fichier et restaure la configuration
 */
async function uploadBackup() {
  const input = document.createElement('input');
  input.type = 'file';
  input.accept = '.json';
  
  input.onchange = async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    
    const reader = new FileReader();
    reader.onload = async (event) => {
      const json = event.target.result;
      
      if (!confirm(typeof t === 'function' ? t('backup_confirm') : '⚠️ ATTENTION\n\nRestaurer ce backup remplacera toute la configuration actuelle.\n\nContinuer ?')) {
        return;
      }
      
      try {
        const response = await fetch('/api/backup/upload', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: json
        });
        
        if (response.ok) {
          alert('✅ ' + (typeof t === 'function' ? t('backup_restored') : 'Backup restauré ! Le système va redémarrer dans 5 secondes...'));
          setTimeout(() => {
            window.location.reload();
          }, 5000);
        } else {
          const error = await response.text();
          alert('❌ ' + (t('error') || 'Erreur') + ': ' + error);
        }
      } catch (error) {
        console.error('Restore error:', error);
        alert('❌ ' + (t('backup_restore_error') || 'Erreur restauration'));
      }
    };
    
    reader.readAsText(file);
  };
  
  input.click();
}

/**
 * Sauvegarde un backup directement sur l'ESP32
 * Utile pour créer des points de restauration rapides
 */
async function saveBackupToESP() {
  if (!confirm(t('backup_esp32_confirm') || 'Créer un backup sur l\'ESP32 ?')) return;
  
  try {
    const response = await fetch('/api/backup/save', { method: 'POST' });
    const filename = await response.text();
    
    if (response.ok) {
      alert('✅ ' + (t('backup_saved') || 'Backup sauvegardé') + ': ' + filename);
    } else {
      alert('❌ ' + (t('backup_save_error') || 'Erreur sauvegarde'));
    }
  } catch (error) {
    console.error('Save error:', error);
    alert('❌ ' + (t('error') || 'Erreur'));
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolBackup = {
  // Actions principales
  downloadBackup,
  uploadBackup,
  
  // Backups ESP32
  saveBackupToESP,
};