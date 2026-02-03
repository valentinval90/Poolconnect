/* 
 * POOL CONNECT - USERS MANAGEMENT
 * Gestion des utilisateurs et authentification
 * users.h  V0.2
 */

#ifndef USERS_H
#define USERS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "globals.h"
#include "config.h"
#include "logging.h"

// ============================================================================
// HASH PASSWORD
// ============================================================================

String hashPassword(String password) {
  LOG_V(LOG_SYSTEM, "Generation du hash pour un mot de passe");
  
  unsigned long hash = 5381;
  for (int i = 0; i < password.length(); i++) {
    hash = ((hash << 5) + hash) + password[i];
  }
  
  String hashStr = String(hash, HEX);
  LOG_V(LOG_SYSTEM, "Hash genere: %s... (tronque pour securite)", hashStr.substring(0, 4).c_str());
  
  return hashStr;
}

// ============================================================================
// GESTION UTILISATEURS
// ============================================================================

void createDefaultAdmin() {
  LOG_I(LOG_SYSTEM, "Creation de l'utilisateur admin par defaut...");
  
  users[0].username = "admin";
  users[0].passwordHash = hashPassword("admin123");
  users[0].role = "admin";
  users[0].enabled = true;
  userCount = 1;
  
  LOG_W(LOG_SYSTEM, "Admin cree avec mot de passe par defaut: admin/admin123");
  LOG_W(LOG_SYSTEM, "IMPORTANT: Changez le mot de passe par defaut pour la securite!");
  LOG_I(LOG_SYSTEM, "Utilisateur: admin, Role: admin, Enabled: true");
}

void loadUsers() {
  LOG_D(LOG_SYSTEM, "Chargement des utilisateurs...");
  
  if (!LittleFS.exists("/users.json")) {
    LOG_W(LOG_SYSTEM, "Fichier /users.json non trouve - Creation admin par defaut");
    createDefaultAdmin();
    return;
  }
  
  File f = LittleFS.open("/users.json", FILE_READ);
  if (!f) {
    LOG_E(LOG_SYSTEM, "Erreur ouverture /users.json en lecture");
    LOG_W(LOG_SYSTEM, "Creation admin par defaut suite a l'erreur");
    LOG_STORAGE_OP("READ", "/users.json", false);
    createDefaultAdmin();
    return;
  }
  
  DynamicJsonDocument doc(4096);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    LOG_E(LOG_SYSTEM, "Erreur parsing JSON utilisateurs: %s", err.c_str());
    LOG_W(LOG_SYSTEM, "Creation admin par defaut suite a l'erreur");
    LOG_STORAGE_OP("READ", "/users.json", false);
    createDefaultAdmin();
    return;
  }
  
  JsonArray arr = doc.as<JsonArray>();
  userCount = 0;
  
  for (JsonObject o : arr) {
    if (userCount >= MAX_USERS) {
      LOG_W(LOG_SYSTEM, "Limite MAX_USERS atteinte (%d), utilisateurs restants ignores", MAX_USERS);
      break;
    }
    
    users[userCount].username = o["username"].as<String>();
    users[userCount].passwordHash = o["passwordHash"].as<String>();
    users[userCount].role = o["role"].as<String>();
    users[userCount].enabled = o["enabled"] | true;
    
    LOG_V(LOG_SYSTEM, "Utilisateur %d charge: %s (role=%s, enabled=%d)", 
          userCount, users[userCount].username.c_str(), 
          users[userCount].role.c_str(), users[userCount].enabled);
    
    userCount++;
  }
  
  LOG_I(LOG_SYSTEM, "Utilisateurs charges: %d utilisateurs", userCount);
  LOG_STORAGE_OP("READ", "/users.json", true);
  
  // Compter les admins et utilisateurs actifs
  int adminCount = 0;
  int activeCount = 0;
  for (int i = 0; i < userCount; i++) {
    if (users[i].role == "admin") adminCount++;
    if (users[i].enabled) activeCount++;
  }
  LOG_I(LOG_SYSTEM, "Statistiques: %d admins, %d utilisateurs actifs", adminCount, activeCount);
}

void saveUsers() {
  LOG_D(LOG_SYSTEM, "Sauvegarde des utilisateurs...");
  
  File f = LittleFS.open("/users.json", FILE_WRITE);
  if (!f) {
    LOG_E(LOG_SYSTEM, "Erreur ouverture /users.json en ecriture");
    LOG_STORAGE_OP("WRITE", "/users.json", false);
    return;
  }
  
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.to<JsonArray>();
  
  for (int i = 0; i < userCount; i++) {
    JsonObject o = arr.createNestedObject();
    o["username"] = users[i].username;
    o["passwordHash"] = users[i].passwordHash;
    o["role"] = users[i].role;
    o["enabled"] = users[i].enabled;
    
    LOG_V(LOG_SYSTEM, "Utilisateur %d sauvegarde: %s (role=%s)", 
          i, users[i].username.c_str(), users[i].role.c_str());
  }
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_SYSTEM, "Utilisateurs sauvegardes: %d utilisateurs (%d bytes)", 
        userCount, bytesWritten);
  LOG_STORAGE_OP("WRITE", "/users.json", true);
}

bool authenticateUser(String username, String password) {
  LOG_D(LOG_SYSTEM, "Tentative d'authentification pour: %s", username.c_str());
  
  String passHash = hashPassword(password);
  
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      LOG_V(LOG_SYSTEM, "Utilisateur trouve: %s", username.c_str());
      
      if (!users[i].enabled) {
        LOG_W(LOG_SYSTEM, "AUTHENTIFICATION REFUSEE: Utilisateur %s desactive", username.c_str());
        return false;
      }
      
      if (users[i].passwordHash == passHash) {
        LOG_I(LOG_SYSTEM, "AUTHENTIFICATION REUSSIE: %s (role=%s)", 
              username.c_str(), users[i].role.c_str());
        return true;
      } else {
        LOG_W(LOG_SYSTEM, "AUTHENTIFICATION ECHOUEE: Mot de passe incorrect pour %s", 
              username.c_str());
        return false;
      }
    }
  }
  
  LOG_W(LOG_SYSTEM, "AUTHENTIFICATION ECHOUEE: Utilisateur %s non trouve", username.c_str());
  return false;
}

String getUserRole(String username) {
  LOG_V(LOG_SYSTEM, "Recuperation du role pour: %s", username.c_str());
  
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      LOG_V(LOG_SYSTEM, "Role trouve pour %s: %s", username.c_str(), users[i].role.c_str());
      return users[i].role;
    }
  }
  
  LOG_W(LOG_SYSTEM, "Utilisateur %s non trouve - Role vide retourne", username.c_str());
  return "";
}

#endif // USERS_H