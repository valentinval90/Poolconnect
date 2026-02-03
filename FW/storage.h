/* 
 * POOL CONNECT - STORAGE
 * Gestion du stockage et persistence
 * storage.h  V0.2
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "config.h"
#include "logging.h"

// ============================================================================
// LITTLEFS
// ============================================================================

void listLittleFS() {
  LOG_D(LOG_STORAGE, "Liste des fichiers LittleFS:");
  
  File root = LittleFS.open("/");
  if (!root) {
    LOG_E(LOG_STORAGE, "Impossible d'ouvrir le repertoire racine");
    return;
  }
  
  int fileCount = 0;
  size_t totalSize = 0;
  
  File file = root.openNextFile();
  while (file) {
    size_t size = file.size();
    LOG_I(LOG_STORAGE, "  %s (%d bytes)", file.name(), size);
    totalSize += size;
    fileCount++;
    file = root.openNextFile();
  }
  
  LOG_I(LOG_STORAGE, "Total: %d fichiers, %d bytes utilises", fileCount, totalSize);
  
  // Informations sur l'espace disque
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();
  LOG_I(LOG_STORAGE, "Espace total: %d bytes, Utilise: %d bytes (%.1f%%)", 
        totalBytes, usedBytes, (usedBytes * 100.0) / totalBytes);
}

// ============================================================================
// CONFIGURATION SYSTÈME
// ============================================================================

void loadSystemConfig() {
  LOG_D(LOG_STORAGE, "Chargement de la configuration systeme...");
  
  if (!LittleFS.exists("/system_config.json")) {
    LOG_W(LOG_STORAGE, "Fichier /system_config.json non trouve - Configuration par defaut");
    return;
  }
  
  File f = LittleFS.open("/system_config.json", FILE_READ);
  if (!f) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /system_config.json en lecture");
    LOG_STORAGE_OP("READ", "/system_config.json", false);
    return;
  }
  
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    LOG_E(LOG_STORAGE, "Erreur parsing JSON: %s", err.c_str());
    LOG_STORAGE_OP("READ", "/system_config.json", false);
    return;
  }
  
  sysConfig.pressureThreshold = doc["pressureThreshold"] | 2.0;
  sysConfig.buzzerEnabled = doc["buzzerEnabled"] | true;
  pressureThreshold = sysConfig.pressureThreshold;
  buzzerMuted = !sysConfig.buzzerEnabled;
  
  LOG_I(LOG_STORAGE, "Configuration systeme chargee avec succes");
  LOG_I(LOG_STORAGE, "Seuil de pression: %.2f BAR", sysConfig.pressureThreshold);
  LOG_I(LOG_STORAGE, "Buzzer: %s", sysConfig.buzzerEnabled ? "Active" : "Desactive");
  LOG_STORAGE_OP("READ", "/system_config.json", true);
}

void saveSystemConfig() {
  LOG_D(LOG_STORAGE, "Sauvegarde de la configuration systeme...");
  
  File f = LittleFS.open("/system_config.json", FILE_WRITE);
  if (!f) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /system_config.json en ecriture");
    LOG_STORAGE_OP("WRITE", "/system_config.json", false);
    return;
  }
  
  StaticJsonDocument<256> doc;
  doc["pressureThreshold"] = sysConfig.pressureThreshold;
  doc["buzzerEnabled"] = sysConfig.buzzerEnabled;
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_STORAGE, "Configuration systeme sauvegardee (%d bytes)", bytesWritten);
  LOG_V(LOG_STORAGE, "Seuil pression: %.2f BAR, Buzzer: %s", 
        sysConfig.pressureThreshold, sysConfig.buzzerEnabled ? "ON" : "OFF");
  LOG_STORAGE_OP("WRITE", "/system_config.json", true);
}

// ============================================================================
// HISTORIQUE
// ============================================================================

void saveHistory() {
  LOG_D(LOG_STORAGE, "Sauvegarde de l'historique...");
  
  File f = LittleFS.open("/history.json", FILE_WRITE);
  if (!f) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /history.json en ecriture");
    LOG_STORAGE_OP("WRITE", "/history.json", false);
    return;
  }
  
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.to<JsonArray>();
  
  for (int i = 0; i < historyCount; i++) {
    JsonObject o = arr.createNestedObject();
    o["date"] = history[i].date;
    o["startTime"] = history[i].startTime;
    o["duration"] = history[i].duration;
    o["avgTemp"] = history[i].avgTemp;
  }
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_STORAGE, "Historique sauvegarde: %d entrees (%d bytes)", historyCount, bytesWritten);
  LOG_STORAGE_OP("WRITE", "/history.json", true);
}

void loadHistory() {
  LOG_D(LOG_STORAGE, "Chargement de l'historique...");
  
  if (!LittleFS.exists("/history.json")) {
    LOG_W(LOG_STORAGE, "Fichier /history.json non trouve - Historique vide");
    historyCount = 0;
    return;
  }
  
  File f = LittleFS.open("/history.json", FILE_READ);
  if (!f) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /history.json en lecture");
    LOG_STORAGE_OP("READ", "/history.json", false);
    historyCount = 0;
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    LOG_E(LOG_STORAGE, "Erreur parsing JSON historique: %s", err.c_str());
    LOG_STORAGE_OP("READ", "/history.json", false);
    historyCount = 0;
    return;
  }
  
  JsonArray arr = doc.as<JsonArray>();
  historyCount = 0;
  
  for (JsonObject o : arr) {
    if (historyCount >= MAX_HISTORY) {
      LOG_W(LOG_STORAGE, "Limite MAX_HISTORY atteinte (%d), entrees restantes ignorees", MAX_HISTORY);
      break;
    }
    strcpy(history[historyCount].date, o["date"]);
    strcpy(history[historyCount].startTime, o["startTime"]);
    history[historyCount].duration = o["duration"];
    history[historyCount].avgTemp = o["avgTemp"];
    historyCount++;
  }
  
  LOG_I(LOG_STORAGE, "Historique charge: %d entrees", historyCount);
  if (historyCount > 0) {
    LOG_V(LOG_STORAGE, "Premiere entree: %s %s (%d min, %.1f C)", 
          history[0].date, history[0].startTime, history[0].duration, history[0].avgTemp);
    LOG_V(LOG_STORAGE, "Derniere entree: %s %s (%d min, %.1f C)", 
          history[historyCount-1].date, history[historyCount-1].startTime, 
          history[historyCount-1].duration, history[historyCount-1].avgTemp);
  }
  LOG_STORAGE_OP("READ", "/history.json", true);
}

void addHistoryEntry(int duration, float avgTemp) {
  LOG_D(LOG_STORAGE, "Ajout d'une entree dans l'historique...");
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    LOG_E(LOG_STORAGE, "NTP non synchronise - Impossible d'ajouter l'entree");
    return;
  }
  
  // Déplacement si limite atteinte
  if (historyCount >= MAX_HISTORY) {
    LOG_V(LOG_STORAGE, "Limite MAX_HISTORY atteinte, decalage des entrees");
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      history[i] = history[i + 1];
    }
    historyCount = MAX_HISTORY - 1;
  }
  
  HistoryEntry *entry = &history[historyCount];
  strftime(entry->date, sizeof(entry->date), "%d/%m/%Y", &timeinfo);
  strftime(entry->startTime, sizeof(entry->startTime), "%H:%M", &timeinfo);
  entry->duration = duration;
  entry->avgTemp = avgTemp;
  
  LOG_I(LOG_STORAGE, "Nouvelle entree historique ajoutee");
  LOG_I(LOG_STORAGE, "Date: %s %s, Duree: %d min, Temp moyenne: %.1f C", 
        entry->date, entry->startTime, duration, avgTemp);
  
  historyCount++;
  
  saveHistory();
}

// ============================================================================
// TIMERS PERSISTENCE
// ============================================================================

void saveFlexTimers() {
  LOG_D(LOG_STORAGE, "Sauvegarde des timers flexibles...");
  
  File file = LittleFS.open("/timers_flex.json", "w");
  if (!file) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /timers_flex.json en ecriture");
    LOG_STORAGE_OP("WRITE", "/timers_flex.json", false);
    return;
  }
  
  DynamicJsonDocument doc(32768);
  JsonArray arr = doc.to<JsonArray>();
  
  for (int i = 0; i < flexTimerCount; i++) {
    JsonObject obj = arr.createNestedObject();
    FlexibleTimer* t = &flexTimers[i];
    
    obj["id"] = t->id;
    obj["name"] = t->name;
    obj["enabled"] = t->enabled;
    
    LOG_V(LOG_STORAGE, "Timer %d: '%s' (%s)", i, t->name.c_str(), t->enabled ? "active" : "inactif");
    
    JsonArray days = obj.createNestedArray("days");
    for (int d = 0; d < 7; d++) days.add(t->days[d]);
    
    JsonObject startObj = obj.createNestedObject("startTime");
    startObj["type"] = (int)t->startTime.type;
    startObj["hour"] = t->startTime.hour;
    startObj["minute"] = t->startTime.minute;
    startObj["sunriseOffset"] = t->startTime.sunriseOffset;
    
    JsonArray condArr = obj.createNestedArray("conditions");
    for (int c = 0; c < t->conditionCount; c++) {
      JsonObject cond = condArr.createNestedObject();
      cond["type"] = (int)t->conditions[c].type;
      cond["value"] = t->conditions[c].value;
      cond["required"] = t->conditions[c].required;
    }
    
    JsonArray actArr = obj.createNestedArray("actions");
    for (int a = 0; a < t->actionCount; a++) {
      JsonObject act = actArr.createNestedObject();
      act["type"] = (int)t->actions[a].type;
      act["relay"] = t->actions[a].relay;
      act["state"] = t->actions[a].state;
      act["delayMinutes"] = t->actions[a].delayMinutes;
      act["conditionValue"] = t->actions[a].conditionValue;
      act["maxWaitMinutes"] = t->actions[a].maxWaitMinutes;
      act["description"] = t->actions[a].description;
      act["buzzerCount"] = t->actions[a].buzzerCount;
      act["ledColor"] = t->actions[a].ledColor;
      act["ledMode"] = t->actions[a].ledMode;
      act["ledDuration"] = t->actions[a].ledDuration;
      
      // Équation personnalisée
      if (t->actions[a].type == ACTION_AUTO_DURATION) {
        JsonObject eq = act.createNestedObject("customEquation");
        eq["useCustom"] = t->actions[a].customEquation.useCustom;
        eq["expression"] = t->actions[a].customEquation.expression;
      }
    }
    
    LOG_V(LOG_STORAGE, "  Conditions: %d, Actions: %d", t->conditionCount, t->actionCount);
  }
  
  size_t bytesWritten = serializeJson(doc, file);
  file.close();
  
  LOG_I(LOG_STORAGE, "Timers flexibles sauvegardes: %d timers (%d bytes)", 
        flexTimerCount, bytesWritten);
  LOG_STORAGE_OP("WRITE", "/timers_flex.json", true);
}

void loadFlexTimers() {
  LOG_D(LOG_STORAGE, "Chargement des timers flexibles...");
  
  if (!LittleFS.exists("/timers_flex.json")) {
    LOG_W(LOG_STORAGE, "Fichier /timers_flex.json non trouve - Aucun timer");
    flexTimerCount = 0;
    return;
  }
  
  File file = LittleFS.open("/timers_flex.json", "r");
  if (!file) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /timers_flex.json en lecture");
    LOG_STORAGE_OP("READ", "/timers_flex.json", false);
    flexTimerCount = 0;
    return;
  }
  
  DynamicJsonDocument doc(32768);
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  
  if (err) {
    LOG_E(LOG_STORAGE, "Erreur parsing JSON timers: %s", err.c_str());
    LOG_STORAGE_OP("READ", "/timers_flex.json", false);
    flexTimerCount = 0;
    return;
  }
  
  JsonArray arr = doc.as<JsonArray>();
  flexTimerCount = 0;
  
  for (JsonObject obj : arr) {
    if (flexTimerCount >= MAX_TIMERS) {
      LOG_W(LOG_STORAGE, "Limite MAX_TIMERS atteinte (%d), timers restants ignores", MAX_TIMERS);
      break;
    }
    
    FlexibleTimer* t = &flexTimers[flexTimerCount];
    
    t->id = obj["id"];
    t->name = obj["name"].as<String>();
    t->enabled = obj["enabled"] | true;
    
    LOG_V(LOG_STORAGE, "Chargement timer %d: '%s' (%s)", 
          flexTimerCount, t->name.c_str(), t->enabled ? "active" : "inactif");
    
    JsonArray days = obj["days"];
    for (int i = 0; i < 7; i++) t->days[i] = days[i];
    
    JsonObject startObj = obj["startTime"];
    t->startTime.type = (StartTimeType)(int)startObj["type"];
    t->startTime.hour = startObj["hour"];
    t->startTime.minute = startObj["minute"];
    t->startTime.sunriseOffset = startObj["sunriseOffset"] | 0;
    
    JsonArray condArr = obj["conditions"];
    t->conditionCount = condArr.size();
    LOG_V(LOG_STORAGE, "  %d conditions", t->conditionCount);
    
    for (int i = 0; i < t->conditionCount && i < 10; i++) {
      JsonObject c = condArr[i];
      t->conditions[i].type = (ConditionType)(int)c["type"];
      t->conditions[i].value = c["value"];
      t->conditions[i].required = c["required"] | true;
    }
    
    JsonArray actArr = obj["actions"];
    t->actionCount = actArr.size();
    LOG_V(LOG_STORAGE, "  %d actions", t->actionCount);
    
    for (int i = 0; i < t->actionCount && i < 50; i++) {
      JsonObject a = actArr[i];
      t->actions[i].type = (ActionType)(int)a["type"];
      t->actions[i].relay = a["relay"];
      t->actions[i].state = a["state"];
      t->actions[i].delayMinutes = a["delayMinutes"];
      t->actions[i].conditionValue = a["conditionValue"] | 0.0;
      t->actions[i].maxWaitMinutes = a["maxWaitMinutes"] | 0;
      t->actions[i].description = a["description"].as<String>();
      t->actions[i].buzzerCount = a["buzzerCount"] | 1;
      t->actions[i].ledColor = a["ledColor"] | 0;
      t->actions[i].ledMode = a["ledMode"] | 0;
      t->actions[i].ledDuration = a["ledDuration"] | 0;
      
      // Équation personnalisée
      if (a.containsKey("customEquation")) {
        JsonObject eq = a["customEquation"];
        t->actions[i].customEquation.useCustom = eq["useCustom"] | false;
        t->actions[i].customEquation.expression = eq["expression"].as<String>();
        
        if (t->actions[i].customEquation.useCustom) {
          LOG_V(LOG_STORAGE, "    Action %d: Equation personnalisee detectee", i);
        }
      }
    }
    
    t->context.state = TIMER_IDLE;
    t->context.currentActionIndex = 0;
    t->lastTriggeredDay = -1;
    
    flexTimerCount++;
  }
  
  LOG_I(LOG_STORAGE, "Timers flexibles charges: %d timers", flexTimerCount);
  LOG_STORAGE_OP("READ", "/timers_flex.json", true);
  
  // Résumé des timers actifs
  int activeCount = 0;
  for (int i = 0; i < flexTimerCount; i++) {
    if (flexTimers[i].enabled) activeCount++;
  }
  LOG_I(LOG_STORAGE, "Timers actifs: %d sur %d", activeCount, flexTimerCount);
}

#endif // STORAGE_H