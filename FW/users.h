/* 
 * POOL CONNECT - USERS MANAGEMENT
 * Gestion des utilisateurs et authentification
 */

#ifndef USERS_H
#define USERS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "globals.h"

// ============================================================================
// HASH PASSWORD
// ============================================================================

String hashPassword(String password) {
  unsigned long hash = 5381;
  for (int i = 0; i < password.length(); i++) {
    hash = ((hash << 5) + hash) + password[i];
  }
  return String(hash, HEX);
}

// ============================================================================
// GESTION UTILISATEURS
// ============================================================================

void createDefaultAdmin() {
  users[0].username = "admin";
  users[0].passwordHash = hashPassword("admin123");
  users[0].role = "admin";
  users[0].enabled = true;
  userCount = 1;
  // saveUsers() sera appelé par le code principal
  Serial.println("✓ Admin créé: admin/admin123");
}

void loadUsers() {
  if (!LittleFS.exists("/users.json")) {
    createDefaultAdmin();
    return;
  }
  
  File f = LittleFS.open("/users.json", FILE_READ);
  if (!f) {
    createDefaultAdmin();
    return;
  }
  
  DynamicJsonDocument doc(4096);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    createDefaultAdmin();
    return;
  }
  
  JsonArray arr = doc.as<JsonArray>();
  userCount = 0;
  
  for (JsonObject o : arr) {
    if (userCount >= MAX_USERS) break;
    users[userCount].username = o["username"].as<String>();
    users[userCount].passwordHash = o["passwordHash"].as<String>();
    users[userCount].role = o["role"].as<String>();
    users[userCount].enabled = o["enabled"] | true;
    userCount++;
  }
  
  Serial.printf("✓ %d utilisateurs chargés\n", userCount);
}

void saveUsers() {
  File f = LittleFS.open("/users.json", FILE_WRITE);
  if (!f) return;
  
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.to<JsonArray>();
  
  for (int i = 0; i < userCount; i++) {
    JsonObject o = arr.createNestedObject();
    o["username"] = users[i].username;
    o["passwordHash"] = users[i].passwordHash;
    o["role"] = users[i].role;
    o["enabled"] = users[i].enabled;
  }
  
  serializeJson(doc, f);
  f.close();
}

bool authenticateUser(String username, String password) {
  String passHash = hashPassword(password);
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username && 
        users[i].passwordHash == passHash && 
        users[i].enabled) {
      return true;
    }
  }
  return false;
}

String getUserRole(String username) {
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      return users[i].role;
    }
  }
  return "";
}

#endif // USERS_H
