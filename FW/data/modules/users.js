// ============================================================================
// USERS.JS - Gestion des utilisateurs
// ============================================================================

async function loadUsers() {
  if (PoolAuth.currentRole !== 'admin') return;
  
  try {
    const users = await fetch('/api/users').then(r => r.json());
    displayUsers(users);
  } catch (error) {
    console.error('Load users error:', error);
  }
}

/**
 * Affiche la liste des utilisateurs dans l'interface
 * @param {Array} users - Liste des utilisateurs
 */
function displayUsers(users) {
  const usersList = document.getElementById('usersList');
  const countBadge = document.getElementById('user-count-badge');
  
  if (!users || users.length === 0) {
    usersList.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">' + (t('no_users') || 'Aucun utilisateur') + '</p>';
    countBadge.textContent = '0';
    return;
  }
  
  countBadge.textContent = users.length;
  
  let html = '';
  users.forEach(user => {
    html += `<div class="user-item">
      <div class="user-info">
        <div class="user-name">${user.username}</div>
        <div class="user-role">${user.role === 'admin' ? 'Administrateur' : 'Utilisateur'}</div>
      </div>
      <span class="user-badge ${user.role}">${user.role.toUpperCase()}</span>
      ${user.username !== 'admin' ? 
        `<button class="btn btn-danger" onclick="deleteUser('${user.username}')">Supprimer</button>` : 
        '<span style="color: #999; font-size: 0.9em;">Protégé</span>'}
    </div>`;
  });
  
  usersList.innerHTML = html;
}

/**
 * Supprime un utilisateur
 * @param {string} username - Nom de l'utilisateur à supprimer
 */
async function deleteUser(username) {
  if (!confirm((t('delete_user_confirm') || 'Supprimer l\'utilisateur') + ' "' + username + '" ?')) return;
  
  try {
    await fetch('/api/users/delete', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({username})
    });
    loadUsers();
  } catch (error) {
    console.error('Delete user error:', error);
    alert('❌ Erreur lors de la suppression');
  }
}

/**
 * Change le mot de passe de l'utilisateur courant
 * Demande l'ancien mot de passe pour confirmation
 */
async function changePassword() {
  const oldPassword = prompt('Ancien mot de passe:');
  if (!oldPassword) return;
  
  const newPassword = prompt('Nouveau mot de passe (min 6 caractères):');
  if (!newPassword || newPassword.length < 6) {
    alert('❌ Le mot de passe doit faire au moins 6 caractères');
    return;
  }
  
  const confirmPassword = prompt('Confirmer le nouveau mot de passe:');
  if (newPassword !== confirmPassword) {
    alert('❌ Les mots de passe ne correspondent pas');
    return;
  }
  
  try {
    const response = await fetch('/api/users/change-password', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({
        username: PoolAuth.currentUser,
        oldPassword: oldPassword,
        newPassword: newPassword
      })
    });
    
    if (response.ok) {
      alert('✅ Mot de passe changé avec succès !');
    } else {
      const error = await response.text();
      alert('❌ ' + error);
    }
  } catch (error) {
    console.error('Change password error:', error);
    alert('❌ Erreur lors du changement de mot de passe');
  }
}

// ============================================================================
// EXPORT DES FONCTIONS
// ============================================================================

window.PoolUsers = {
  // Fonctions principales
  loadUsers,
  
  // Gestion des mots de passe
  changePassword
};