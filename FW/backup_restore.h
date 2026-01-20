/* 
 * POOL CONNECT - BACKUP/RESTORE SYSTEM
 * Sauvegarde et restauration complète de la configuration
 */

#ifndef BACKUP_RESTORE_H
#define BACKUP_RESTORE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "config.h"

// ============================================================================
// GÉNÉRATION BACKUP JSON
// ============================================================================

String generateBackupJSON() {
  DynamicJsonDocument doc(32768); // 32KB pour backup complet
  
  // Métadonnées
  doc["version"] = FIRMWARE_VERSION;
  doc["timestamp"] = millis() / 1000;
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    doc["date"] = timeStr;
  }
  
  // Configuration système
  JsonObject sys = doc.createNestedObject("system");
  sys["pressureThreshold"] = sysConfig.pressureThreshold;
  sys["buzzerEnabled"] = sysConfig.buzzerEnabled;
  
  // Calibration
  JsonObject calib = doc.createNestedObject("calibration");
  
  JsonObject temp = calib.createNestedObject("temperature");
  temp["useCalibration"] = calibConfig.tempUseCalibration;
  temp["useTwoPoint"] = calibConfig.tempUseTwoPoint;
  temp["offset"] = calibConfig.tempOffset;
  temp["point1Raw"] = calibConfig.tempPoint1Raw;
  temp["point1Real"] = calibConfig.tempPoint1Real;
  temp["point2Raw"] = calibConfig.tempPoint2Raw;
  temp["point2Real"] = calibConfig.tempPoint2Real;
  
  JsonObject press = calib.createNestedObject("pressure");
  press["useCalibration"] = calibConfig.pressureUseCalibration;
  press["useTwoPoint"] = calibConfig.pressureUseTwoPoint;
  press["offset"] = calibConfig.pressureOffset;
  press["point1Raw"] = calibConfig.pressurePoint1Raw;
  press["point1Real"] = calibConfig.pressurePoint1Real;
  press["point2Raw"] = calibConfig.pressurePoint2Raw;
  press["point2Real"] = calibConfig.pressurePoint2Real;
  
  // Utilisateurs (sans les mots de passe pour sécurité)
  JsonArray usersArr = doc.createNestedArray("users");
  for (int i = 0; i < userCount; i++) {
    JsonObject u = usersArr.createNestedObject();
    u["username"] = users[i].username;
    u["passwordHash"] = users[i].passwordHash;
    u["role"] = users[i].role;
    u["enabled"] = users[i].enabled;
  }
  
  // Timers flexibles
  JsonArray timersArr = doc.createNestedArray("timers");
  for (int i = 0; i < flexTimerCount; i++) {
    JsonObject t = timersArr.createNestedObject();
    FlexibleTimer* timer = &flexTimers[i];
    
    t["id"] = timer->id;
    t["name"] = timer->name;
    t["enabled"] = timer->enabled;
    
    JsonArray days = t.createNestedArray("days");
    for (int d = 0; d < 7; d++) days.add(timer->days[d]);
    
    JsonObject start = t.createNestedObject("startTime");
    start["type"] = (int)timer->startTime.type;
    start["hour"] = timer->startTime.hour;
    start["minute"] = timer->startTime.minute;
    start["sunriseOffset"] = timer->startTime.sunriseOffset;
    
    JsonArray conds = t.createNestedArray("conditions");
    for (int c = 0; c < timer->conditionCount; c++) {
      JsonObject cond = conds.createNestedObject();
      cond["type"] = (int)timer->conditions[c].type;
      cond["value"] = timer->conditions[c].value;
      cond["required"] = timer->conditions[c].required;
    }
    
    JsonArray acts = t.createNestedArray("actions");
    for (int a = 0; a < timer->actionCount; a++) {
      JsonObject act = acts.createNestedObject();
      act["type"] = (int)timer->actions[a].type;
      act["relay"] = timer->actions[a].relay;
      act["state"] = timer->actions[a].state;
      act["delayMinutes"] = timer->actions[a].delayMinutes;
      act["conditionValue"] = timer->actions[a].conditionValue;
      act["maxWaitMinutes"] = timer->actions[a].maxWaitMinutes;
      act["description"] = timer->actions[a].description;
      act["buzzerCount"] = timer->actions[a].buzzerCount;
      act["ledColor"] = timer->actions[a].ledColor;
      act["ledMode"] = timer->actions[a].ledMode;
      act["ledDuration"] = timer->actions[a].ledDuration;
      
      if (timer->actions[a].type == ACTION_AUTO_DURATION) {
        JsonObject eq = act.createNestedObject("customEquation");
        eq["useCustom"] = timer->actions[a].customEquation.useCustom;
        eq["expression"] = timer->actions[a].customEquation.expression;
      }
    }
  }
  
  // MQTT
  JsonObject mqtt = doc.createNestedObject("mqtt");
  mqtt["server"] = mqttServer;
  mqtt["port"] = mqttPort;
  mqtt["user"] = mqttUser;
  mqtt["password"] = mqttPassword;
  mqtt["topic"] = mqttTopic;
  
  // Météo
  JsonObject weather = doc.createNestedObject("weather");
  weather["apiKey"] = weatherApiKey;
  weather["latitude"] = latitude;
  weather["longitude"] = longitude;
  
  // Historique (limité aux 50 dernières entrées)
  JsonArray histArr = doc.createNestedArray("history");
  int startIdx = (historyCount > 50) ? (historyCount - 50) : 0;
  for (int i = startIdx; i < historyCount; i++) {
    JsonObject h = histArr.createNestedObject();
    h["date"] = history[i].date;
    h["startTime"] = history[i].startTime;
    h["duration"] = history[i].duration;
    h["avgTemp"] = history[i].avgTemp;
  }
  
  String output;
  serializeJson(doc, output);
  return output;
}

// ============================================================================
// SAUVEGARDE BACKUP SUR FICHIER
// ============================================================================

bool saveBackupToFile(String filename = "/backup.json") {
  File file = LittleFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("❌ Erreur création backup");
    return false;
  }
  
  String json = generateBackupJSON();
  file.print(json);
  file.close();
  
  Serial.printf("✅ Backup sauvegardé: %s (%d bytes)\n", filename.c_str(), json.length());
  return true;
}

// ============================================================================
// RESTAURATION DEPUIS JSON
// ============================================================================

bool restoreFromJSON(String json) {
  DynamicJsonDocument doc(32768);
  DeserializationError err = deserializeJson(doc, json);
  
  if (err) {
    Serial.printf("❌ Erreur parse backup: %s\n", err.c_str());
    return false;
  }
  
  // Vérifier version
  String backupVersion = doc["version"].as<String>();
  Serial.printf("📦 Restauration backup version %s\n", backupVersion.c_str());
  
  // Système
  if (doc.containsKey("system")) {
    sysConfig.pressureThreshold = doc["system"]["pressureThreshold"] | 2.0;
    sysConfig.buzzerEnabled = doc["system"]["buzzerEnabled"] | true;
    pressureThreshold = sysConfig.pressureThreshold;
    buzzerMuted = !sysConfig.buzzerEnabled;
    saveSystemConfig();
  }
  
  // Calibration
  if (doc.containsKey("calibration")) {
    JsonObject calib = doc["calibration"];
    
    JsonObject temp = calib["temperature"];
    calibConfig.tempUseCalibration = temp["useCalibration"] | false;
    calibConfig.tempUseTwoPoint = temp["useTwoPoint"] | false;
    calibConfig.tempOffset = temp["offset"] | 0.0;
    calibConfig.tempPoint1Raw = temp["point1Raw"] | 10.0;
    calibConfig.tempPoint1Real = temp["point1Real"] | 10.0;
    calibConfig.tempPoint2Raw = temp["point2Raw"] | 30.0;
    calibConfig.tempPoint2Real = temp["point2Real"] | 30.0;
    
    JsonObject press = calib["pressure"];
    calibConfig.pressureUseCalibration = press["useCalibration"] | false;
    calibConfig.pressureUseTwoPoint = press["useTwoPoint"] | false;
    calibConfig.pressureOffset = press["offset"] | 0.0;
    calibConfig.pressurePoint1Raw = press["point1Raw"] | 1.0;
    calibConfig.pressurePoint1Real = press["point1Real"] | 1.0;
    calibConfig.pressurePoint2Raw = press["point2Raw"] | 3.0;
    calibConfig.pressurePoint2Real = press["point2Real"] | 3.0;
    
    saveCalibrationConfig();
  }
  
  // Utilisateurs
  if (doc.containsKey("users")) {
    JsonArray usersArr = doc["users"];
    userCount = 0;
    for (JsonObject u : usersArr) {
      if (userCount >= MAX_USERS) break;
      users[userCount].username = u["username"].as<String>();
      users[userCount].passwordHash = u["passwordHash"].as<String>();
      users[userCount].role = u["role"].as<String>();
      users[userCount].enabled = u["enabled"] | true;
      userCount++;
    }
    saveUsers();
  }
  
  // Timers
  if (doc.containsKey("timers")) {
    JsonArray timersArr = doc["timers"];
    flexTimerCount = 0;
    
    for (JsonObject t : timersArr) {
      if (flexTimerCount >= MAX_TIMERS) break;
      FlexibleTimer* timer = &flexTimers[flexTimerCount];
      
      timer->id = t["id"];
      timer->name = t["name"].as<String>();
      timer->enabled = t["enabled"] | true;
      
      JsonArray days = t["days"];
      for (int i = 0; i < 7; i++) timer->days[i] = days[i];
      
      JsonObject start = t["startTime"];
      timer->startTime.type = (StartTimeType)(int)start["type"];
      timer->startTime.hour = start["hour"];
      timer->startTime.minute = start["minute"];
      timer->startTime.sunriseOffset = start["sunriseOffset"] | 0;
      
      JsonArray conds = t["conditions"];
      timer->conditionCount = conds.size();
      for (int i = 0; i < timer->conditionCount && i < 10; i++) {
        JsonObject c = conds[i];
        timer->conditions[i].type = (ConditionType)(int)c["type"];
        timer->conditions[i].value = c["value"];
        timer->conditions[i].required = c["required"] | true;
      }
      
      JsonArray acts = t["actions"];
      timer->actionCount = acts.size();
      for (int i = 0; i < timer->actionCount && i < 50; i++) {
        JsonObject a = acts[i];
        timer->actions[i].type = (ActionType)(int)a["type"];
        timer->actions[i].relay = a["relay"];
        timer->actions[i].state = a["state"];
        timer->actions[i].delayMinutes = a["delayMinutes"];
        timer->actions[i].conditionValue = a["conditionValue"] | 0.0;
        timer->actions[i].maxWaitMinutes = a["maxWaitMinutes"] | 0;
        timer->actions[i].description = a["description"].as<String>();
        timer->actions[i].buzzerCount = a["buzzerCount"] | 1;
        timer->actions[i].ledColor = a["ledColor"] | 0;
        timer->actions[i].ledMode = a["ledMode"] | 0;
        timer->actions[i].ledDuration = a["ledDuration"] | 0;
        
        if (a.containsKey("customEquation")) {
          JsonObject eq = a["customEquation"];
          timer->actions[i].customEquation.useCustom = eq["useCustom"] | false;
          timer->actions[i].customEquation.expression = eq["expression"].as<String>();
        }
      }
      
      timer->context.state = TIMER_IDLE;
      timer->context.currentActionIndex = 0;
      timer->lastTriggeredDay = -1;
      
      flexTimerCount++;
    }
    saveFlexTimers();
  }
  
  // MQTT
  if (doc.containsKey("mqtt")) {
    mqttServer = doc["mqtt"]["server"].as<String>();
    mqttPort = doc["mqtt"]["port"] | 1883;
    mqttUser = doc["mqtt"]["user"].as<String>();
    mqttPassword = doc["mqtt"]["password"].as<String>();
    mqttTopic = doc["mqtt"]["topic"].as<String>();
    saveMQTTConfig();
  }
  
  // Météo
  if (doc.containsKey("weather")) {
    weatherApiKey = doc["weather"]["apiKey"].as<String>();
    latitude = doc["weather"]["latitude"].as<String>();
    longitude = doc["weather"]["longitude"].as<String>();
    saveWeatherConfig();
  }
  
  // Historique
  if (doc.containsKey("history")) {
    JsonArray histArr = doc["history"];
    historyCount = 0;
    for (JsonObject h : histArr) {
      if (historyCount >= MAX_HISTORY) break;
      strcpy(history[historyCount].date, h["date"]);
      strcpy(history[historyCount].startTime, h["startTime"]);
      history[historyCount].duration = h["duration"];
      history[historyCount].avgTemp = h["avgTemp"];
      historyCount++;
    }
    saveHistory();
  }
  
  Serial.println("✅ Restauration terminée");
  return true;
}

// ============================================================================
// BACKUP AUTOMATIQUE
// ============================================================================

unsigned long lastAutoBackup = 0;
const unsigned long AUTO_BACKUP_INTERVAL = 86400000UL; // 24h

void checkAutoBackup() {
  if (millis() - lastAutoBackup > AUTO_BACKUP_INTERVAL) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char filename[32];
      strftime(filename, sizeof(filename), "/backup_%Y%m%d.json", &timeinfo);
      saveBackupToFile(filename);
    } else {
      saveBackupToFile("/backup_auto.json");
    }
    lastAutoBackup = millis();
  }
}

#endif // BACKUP_RESTORE_H
