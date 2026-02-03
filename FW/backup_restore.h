/* 
 * POOL CONNECT - BACKUP/RESTORE SYSTEM
 * Sauvegarde et restauration complète de la configuration
 * backup_restor.h   V0.3
 */

#ifndef BACKUP_RESTORE_H
#define BACKUP_RESTORE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "config.h"
#include "logging.h"

// ============================================================================
// GÉNÉRATION BACKUP JSON
// ============================================================================

String generateBackupJSON() {
  LOG_I(LOG_BACKUP, "Generation du backup JSON...");
  LOG_D(LOG_BACKUP, "Allocation du document JSON (32KB)");
  
  DynamicJsonDocument doc(32768); // 32KB pour backup complet
  
  // Métadonnées
  doc["version"] = FIRMWARE_VERSION;
  doc["timestamp"] = millis() / 1000;
  LOG_V(LOG_BACKUP, "Version: %s, Timestamp: %lu", FIRMWARE_VERSION, millis() / 1000);
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    doc["date"] = timeStr;
    LOG_V(LOG_BACKUP, "Date du backup: %s", timeStr);
  } else {
    LOG_W(LOG_BACKUP, "Date du backup non disponible (NTP non synchronise)");
  }
  
  // Configuration système
  LOG_D(LOG_BACKUP, "Sauvegarde de la configuration systeme...");
  JsonObject sys = doc.createNestedObject("system");
  sys["pressureThreshold"] = sysConfig.pressureThreshold;
  sys["buzzerEnabled"] = sysConfig.buzzerEnabled;
  LOG_V(LOG_BACKUP, "Config systeme: pressureThreshold=%.2f, buzzerEnabled=%d", 
        sysConfig.pressureThreshold, sysConfig.buzzerEnabled);
  
  // Calibration
  LOG_D(LOG_BACKUP, "Sauvegarde des calibrations...");
  JsonObject calib = doc.createNestedObject("calibration");
  
  JsonObject temp = calib.createNestedObject("temperature");
  temp["useCalibration"] = calibConfig.tempUseCalibration;
  temp["useTwoPoint"] = calibConfig.tempUseTwoPoint;
  temp["offset"] = calibConfig.tempOffset;
  temp["point1Raw"] = calibConfig.tempPoint1Raw;
  temp["point1Real"] = calibConfig.tempPoint1Real;
  temp["point2Raw"] = calibConfig.tempPoint2Raw;
  temp["point2Real"] = calibConfig.tempPoint2Real;
  LOG_V(LOG_BACKUP, "Calibration temperature: useCalib=%d, offset=%.2f", 
        calibConfig.tempUseCalibration, calibConfig.tempOffset);
  
  JsonObject press = calib.createNestedObject("pressure");
  press["useCalibration"] = calibConfig.pressureUseCalibration;
  press["useTwoPoint"] = calibConfig.pressureUseTwoPoint;
  press["offset"] = calibConfig.pressureOffset;
  press["point1Raw"] = calibConfig.pressurePoint1Raw;
  press["point1Real"] = calibConfig.pressurePoint1Real;
  press["point2Raw"] = calibConfig.pressurePoint2Raw;
  press["point2Real"] = calibConfig.pressurePoint2Real;
  LOG_V(LOG_BACKUP, "Calibration pression: useCalib=%d, offset=%.2f", 
        calibConfig.pressureUseCalibration, calibConfig.pressureOffset);
  
  LOG_D(LOG_BACKUP, "Sauvegarde des preferences utilisateur...");
  JsonObject prefs = doc.createNestedObject("userPreferences");
  prefs["language"] = userPrefs.language;
  prefs["temperatureUnit"] = userPrefs.temperatureUnit;
  prefs["pressureUnit"] = userPrefs.pressureUnit;
  prefs["theme"] = userPrefs.theme;
  prefs["chartUpdateInterval"] = userPrefs.chartUpdateInterval;
  LOG_V(LOG_BACKUP, "Preferences: lang=%s, tempUnit=%s, pressureUnit=%s, theme=%s, chartInterval=%d", 
        userPrefs.language.c_str(), 
        userPrefs.temperatureUnit.c_str(),
        userPrefs.pressureUnit.c_str(),
        userPrefs.theme.c_str(),
        userPrefs.chartUpdateInterval);
  LOG_I(LOG_BACKUP, "Preferences utilisateur sauvegardees");

  // Utilisateurs (sans les mots de passe pour sécurité)
  LOG_D(LOG_BACKUP, "Sauvegarde des utilisateurs...");
  JsonArray usersArr = doc.createNestedArray("users");
  for (int i = 0; i < userCount; i++) {
    JsonObject u = usersArr.createNestedObject();
    u["username"] = users[i].username;
    u["passwordHash"] = users[i].passwordHash;
    u["role"] = users[i].role;
    u["enabled"] = users[i].enabled;
    LOG_V(LOG_BACKUP, "Utilisateur %d: %s (role=%s, enabled=%d)", 
          i, users[i].username.c_str(), users[i].role.c_str(), users[i].enabled);
  }
  LOG_I(LOG_BACKUP, "%d utilisateurs sauvegardes", userCount);
  
  // Timers flexibles
  LOG_D(LOG_BACKUP, "Sauvegarde des timers flexibles...");
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
    
    LOG_V(LOG_BACKUP, "Timer %d: '%s' (enabled=%d, conditions=%d, actions=%d)", 
          i, timer->name.c_str(), timer->enabled, timer->conditionCount, timer->actionCount);
  }
  LOG_I(LOG_BACKUP, "%d timers flexibles sauvegardes", flexTimerCount);
  
  // MQTT
  LOG_D(LOG_BACKUP, "Sauvegarde de la configuration MQTT...");
  JsonObject mqtt = doc.createNestedObject("mqtt");
  mqtt["server"] = mqttServer;
  mqtt["port"] = mqttPort;
  mqtt["user"] = mqttUser;
  mqtt["password"] = mqttPassword;
  mqtt["topic"] = mqttTopic;
  LOG_V(LOG_BACKUP, "MQTT: serveur=%s:%d, user=%s", 
        mqttServer.c_str(), mqttPort, mqttUser.c_str());
  
  // Météo
  LOG_D(LOG_BACKUP, "Sauvegarde de la configuration meteo...");
  JsonObject weather = doc.createNestedObject("weather");
  weather["apiKey"] = weatherApiKey;
  weather["latitude"] = latitude;
  weather["longitude"] = longitude;
  LOG_V(LOG_BACKUP, "Meteo: lat=%s, lon=%s, apiKey=%s", 
        latitude.c_str(), longitude.c_str(), 
        weatherApiKey.length() > 0 ? "configure" : "non configure");
  
  // Historique (limité aux 50 dernières entrées)
  LOG_D(LOG_BACKUP, "Sauvegarde de l'historique...");
  JsonArray histArr = doc.createNestedArray("history");
  int startIdx = (historyCount > 50) ? (historyCount - 50) : 0;
  for (int i = startIdx; i < historyCount; i++) {
    JsonObject h = histArr.createNestedObject();
    h["date"] = history[i].date;
    h["startTime"] = history[i].startTime;
    h["duration"] = history[i].duration;
    h["avgTemp"] = history[i].avgTemp;
  }
  int savedHistoryCount = historyCount - startIdx;
  LOG_I(LOG_BACKUP, "%d entrees d'historique sauvegardees (limite 50)", savedHistoryCount);
  
  String output;
  serializeJson(doc, output);
  
  LOG_I(LOG_BACKUP, "Backup JSON genere avec succes - Taille: %d bytes", output.length());
  LOG_MEMORY();
  
  return output;
}

// ============================================================================
// SAUVEGARDE BACKUP SUR FICHIER
// ============================================================================

bool saveBackupToFile(String filename = "/backup.json") {
  LOG_I(LOG_BACKUP, "Sauvegarde du backup dans le fichier: %s", filename.c_str());
  
  File file = LittleFS.open(filename, FILE_WRITE);
  if (!file) {
    LOG_E(LOG_BACKUP, "Erreur lors de la creation du fichier backup: %s", filename.c_str());
    return false;
  }
  
  LOG_D(LOG_BACKUP, "Fichier ouvert en ecriture");
  
  String json = generateBackupJSON();
  size_t bytesWritten = file.print(json);
  file.close();
  
  if (bytesWritten == json.length()) {
    LOG_I(LOG_BACKUP, "Backup sauvegarde avec succes: %s (%d bytes)", 
          filename.c_str(), json.length());
    LOG_STORAGE_OP("BACKUP WRITE", filename.c_str(), true);
    return true;
  } else {
    LOG_E(LOG_BACKUP, "Erreur d'ecriture: %d bytes ecrits sur %d attendus", 
          bytesWritten, json.length());
    LOG_STORAGE_OP("BACKUP WRITE", filename.c_str(), false);
    return false;
  }
}

// ============================================================================
// RESTAURATION DEPUIS JSON
// ============================================================================

bool restoreFromJSON(String json) {
  LOG_SEPARATOR();
  LOG_I(LOG_BACKUP, "Demarrage de la restauration depuis JSON...");
  LOG_I(LOG_BACKUP, "Taille du JSON a restaurer: %d bytes", json.length());
  
  DynamicJsonDocument doc(32768);
  DeserializationError err = deserializeJson(doc, json);
  
  if (err) {
    LOG_E(LOG_BACKUP, "Erreur de parsing du JSON: %s", err.c_str());
    return false;
  }
  
  LOG_I(LOG_BACKUP, "JSON parse avec succes");
  
  // Vérifier version
  String backupVersion = doc["version"].as<String>();
  LOG_I(LOG_BACKUP, "Version du backup: %s (version actuelle: %s)", 
        backupVersion.c_str(), FIRMWARE_VERSION);
  
  if (doc.containsKey("date")) {
    LOG_I(LOG_BACKUP, "Date du backup: %s", doc["date"].as<String>().c_str());
  }
  
  // Système
  if (doc.containsKey("system")) {
    LOG_D(LOG_BACKUP, "Restauration de la configuration systeme...");
    sysConfig.pressureThreshold = doc["system"]["pressureThreshold"] | 2.0;
    sysConfig.buzzerEnabled = doc["system"]["buzzerEnabled"] | true;
    pressureThreshold = sysConfig.pressureThreshold;
    buzzerMuted = !sysConfig.buzzerEnabled;
    saveSystemConfig();
    LOG_I(LOG_BACKUP, "Config systeme restauree: pressureThreshold=%.2f, buzzerEnabled=%d",
          sysConfig.pressureThreshold, sysConfig.buzzerEnabled);
  } else {
    LOG_W(LOG_BACKUP, "Pas de configuration systeme dans le backup");
  }
  
  // Calibration
  if (doc.containsKey("calibration")) {
    LOG_D(LOG_BACKUP, "Restauration des calibrations...");
    JsonObject calib = doc["calibration"];
    
    JsonObject temp = calib["temperature"];
    calibConfig.tempUseCalibration = temp["useCalibration"] | false;
    calibConfig.tempUseTwoPoint = temp["useTwoPoint"] | false;
    calibConfig.tempOffset = temp["offset"] | 0.0;
    calibConfig.tempPoint1Raw = temp["point1Raw"] | 10.0;
    calibConfig.tempPoint1Real = temp["point1Real"] | 10.0;
    calibConfig.tempPoint2Raw = temp["point2Raw"] | 30.0;
    calibConfig.tempPoint2Real = temp["point2Real"] | 30.0;
    LOG_V(LOG_BACKUP, "Calibration temperature: useCalib=%d, offset=%.2f",
          calibConfig.tempUseCalibration, calibConfig.tempOffset);
    
    JsonObject press = calib["pressure"];
    calibConfig.pressureUseCalibration = press["useCalibration"] | false;
    calibConfig.pressureUseTwoPoint = press["useTwoPoint"] | false;
    calibConfig.pressureOffset = press["offset"] | 0.0;
    calibConfig.pressurePoint1Raw = press["point1Raw"] | 1.0;
    calibConfig.pressurePoint1Real = press["point1Real"] | 1.0;
    calibConfig.pressurePoint2Raw = press["point2Raw"] | 3.0;
    calibConfig.pressurePoint2Real = press["point2Real"] | 3.0;
    LOG_V(LOG_BACKUP, "Calibration pression: useCalib=%d, offset=%.2f",
          calibConfig.pressureUseCalibration, calibConfig.pressureOffset);
    
    saveCalibrationConfig();
    LOG_I(LOG_BACKUP, "Calibrations restaurees avec succes");
  } else {
    LOG_W(LOG_BACKUP, "Pas de calibration dans le backup");
  }
  
    if (doc.containsKey("userPreferences")) {
    LOG_D(LOG_BACKUP, "Restauration des preferences utilisateur...");
    JsonObject prefs = doc["userPreferences"];
    
    if (prefs.containsKey("language")) {
      userPrefs.language = prefs["language"].as<String>();
      LOG_V(LOG_BACKUP, "Langue: %s", userPrefs.language.c_str());
    }
    
    if (prefs.containsKey("temperatureUnit")) {
      userPrefs.temperatureUnit = prefs["temperatureUnit"].as<String>();
      LOG_V(LOG_BACKUP, "Unite temperature: %s", userPrefs.temperatureUnit.c_str());
    }
    
    if (prefs.containsKey("pressureUnit")) {
      userPrefs.pressureUnit = prefs["pressureUnit"].as<String>();
      LOG_V(LOG_BACKUP, "Unite pression: %s", userPrefs.pressureUnit.c_str());
    }
    
    if (prefs.containsKey("theme")) {
      userPrefs.theme = prefs["theme"].as<String>();
      LOG_V(LOG_BACKUP, "Theme: %s", userPrefs.theme.c_str());
    }
    
    if (prefs.containsKey("chartUpdateInterval")) {
      userPrefs.chartUpdateInterval = prefs["chartUpdateInterval"];
      LOG_V(LOG_BACKUP, "Intervalle graphique: %d ms", userPrefs.chartUpdateInterval);
    }  

    if (doc.containsKey("userPreferences")) {
    LOG_D(LOG_BACKUP, "Restauration des preferences utilisateur...");
    JsonObject prefs = doc["userPreferences"];
    }
    
    if (prefs.containsKey("language")) {
      userPrefs.language = prefs["language"].as<String>();
      LOG_V(LOG_BACKUP, "Langue: %s", userPrefs.language.c_str());
    }
    
    if (prefs.containsKey("temperatureUnit")) {
      userPrefs.temperatureUnit = prefs["temperatureUnit"].as<String>();
      LOG_V(LOG_BACKUP, "Unite temperature: %s", userPrefs.temperatureUnit.c_str());
    }
    
    if (prefs.containsKey("pressureUnit")) {
      userPrefs.pressureUnit = prefs["pressureUnit"].as<String>();
      LOG_V(LOG_BACKUP, "Unite pression: %s", userPrefs.pressureUnit.c_str());
    }
    
    if (prefs.containsKey("theme")) {
      userPrefs.theme = prefs["theme"].as<String>();
      LOG_V(LOG_BACKUP, "Theme: %s", userPrefs.theme.c_str());
    }
    
    if (prefs.containsKey("chartUpdateInterval")) {
      userPrefs.chartUpdateInterval = prefs["chartUpdateInterval"];
      LOG_V(LOG_BACKUP, "Intervalle graphique: %d ms", userPrefs.chartUpdateInterval);
    }
    
    saveSystemConfig(); // Sauvegarder les préférences dans LittleFS
    LOG_I(LOG_BACKUP, "Preferences utilisateur restaurees avec succes");
  } else {
    LOG_W(LOG_BACKUP, "Aucune preference utilisateur dans le backup");
  }

  // Utilisateurs
  if (doc.containsKey("users")) {
    LOG_D(LOG_BACKUP, "Restauration des utilisateurs...");
    JsonArray usersArr = doc["users"];
    userCount = 0;
    for (JsonObject u : usersArr) {
      if (userCount >= MAX_USERS) {
        LOG_W(LOG_BACKUP, "Limite MAX_USERS atteinte (%d), utilisateurs restants ignores", MAX_USERS);
        break;
      }
      users[userCount].username = u["username"].as<String>();
      users[userCount].passwordHash = u["passwordHash"].as<String>();
      users[userCount].role = u["role"].as<String>();
      users[userCount].enabled = u["enabled"] | true;
      LOG_V(LOG_BACKUP, "Utilisateur %d restaure: %s (role=%s)", 
            userCount, users[userCount].username.c_str(), users[userCount].role.c_str());
      userCount++;
    }
    saveUsers();
    LOG_I(LOG_BACKUP, "%d utilisateurs restaures", userCount);
  } else {
    LOG_W(LOG_BACKUP, "Pas d'utilisateurs dans le backup");
  }
  
  // Timers
  if (doc.containsKey("timers")) {
    LOG_D(LOG_BACKUP, "Restauration des timers flexibles...");
    JsonArray timersArr = doc["timers"];
    flexTimerCount = 0;
    
    for (JsonObject t : timersArr) {
      if (flexTimerCount >= MAX_TIMERS) {
        LOG_W(LOG_BACKUP, "Limite MAX_TIMERS atteinte (%d), timers restants ignores", MAX_TIMERS);
        break;
      }
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
      LOG_V(LOG_BACKUP, "Timer '%s': %d conditions", timer->name.c_str(), timer->conditionCount);
      for (int i = 0; i < timer->conditionCount && i < 10; i++) {
        JsonObject c = conds[i];
        timer->conditions[i].type = (ConditionType)(int)c["type"];
        timer->conditions[i].value = c["value"];
        timer->conditions[i].required = c["required"] | true;
      }
      
      JsonArray acts = t["actions"];
      timer->actionCount = acts.size();
      LOG_V(LOG_BACKUP, "Timer '%s': %d actions", timer->name.c_str(), timer->actionCount);
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
      
      LOG_V(LOG_BACKUP, "Timer %d restaure: '%s' (enabled=%d)", 
            flexTimerCount, timer->name.c_str(), timer->enabled);
      flexTimerCount++;
    }
    saveFlexTimers();
    LOG_I(LOG_BACKUP, "%d timers flexibles restaures", flexTimerCount);
  } else {
    LOG_W(LOG_BACKUP, "Pas de timers dans le backup");
  }
  
  // MQTT
  if (doc.containsKey("mqtt")) {
    LOG_D(LOG_BACKUP, "Restauration de la configuration MQTT...");
    mqttServer = doc["mqtt"]["server"].as<String>();
    mqttPort = doc["mqtt"]["port"] | 1883;
    mqttUser = doc["mqtt"]["user"].as<String>();
    mqttPassword = doc["mqtt"]["password"].as<String>();
    mqttTopic = doc["mqtt"]["topic"].as<String>();
    saveMQTTConfig();
    LOG_I(LOG_BACKUP, "MQTT restaure: serveur=%s:%d, user=%s", 
          mqttServer.c_str(), mqttPort, mqttUser.c_str());
  } else {
    LOG_W(LOG_BACKUP, "Pas de configuration MQTT dans le backup");
  }
  
  // Météo
  if (doc.containsKey("weather")) {
    LOG_D(LOG_BACKUP, "Restauration de la configuration meteo...");
    weatherApiKey = doc["weather"]["apiKey"].as<String>();
    latitude = doc["weather"]["latitude"].as<String>();
    longitude = doc["weather"]["longitude"].as<String>();
    saveWeatherConfig();
    LOG_I(LOG_BACKUP, "Meteo restauree: lat=%s, lon=%s", 
          latitude.c_str(), longitude.c_str());
  } else {
    LOG_W(LOG_BACKUP, "Pas de configuration meteo dans le backup");
  }
  
  // Historique
  if (doc.containsKey("history")) {
    LOG_D(LOG_BACKUP, "Restauration de l'historique...");
    JsonArray histArr = doc["history"];
    historyCount = 0;
    for (JsonObject h : histArr) {
      if (historyCount >= MAX_HISTORY) {
        LOG_W(LOG_BACKUP, "Limite MAX_HISTORY atteinte (%d), entrees restantes ignorees", MAX_HISTORY);
        break;
      }
      strcpy(history[historyCount].date, h["date"]);
      strcpy(history[historyCount].startTime, h["startTime"]);
      history[historyCount].duration = h["duration"];
      history[historyCount].avgTemp = h["avgTemp"];
      historyCount++;
    }
    saveHistory();
    LOG_I(LOG_BACKUP, "%d entrees d'historique restaurees", historyCount);
  } else {
    LOG_W(LOG_BACKUP, "Pas d'historique dans le backup");
  }
  
  LOG_SEPARATOR();
  LOG_I(LOG_BACKUP, "Restauration terminee avec succes");
  LOG_MEMORY();
  LOG_SEPARATOR();
  
  return true;
}

// ============================================================================
// BACKUP AUTOMATIQUE
// ============================================================================

unsigned long lastAutoBackup = 0;
const unsigned long AUTO_BACKUP_INTERVAL = 86400000UL; // 24h

void checkAutoBackup() {
  if (millis() - lastAutoBackup > AUTO_BACKUP_INTERVAL) {
    LOG_I(LOG_BACKUP, "Declenchement du backup automatique (intervalle 24h atteint)");
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char filename[32];
      strftime(filename, sizeof(filename), "/backup_%Y%m%d.json", &timeinfo);
      LOG_I(LOG_BACKUP, "Fichier de backup date: %s", filename);
      
      if (saveBackupToFile(filename)) {
        LOG_I(LOG_BACKUP, "Backup automatique reussi");
      } else {
        LOG_E(LOG_BACKUP, "Echec du backup automatique");
      }
    } else {
      LOG_W(LOG_BACKUP, "NTP non synchronise, utilisation du nom par defaut");
      
      if (saveBackupToFile("/backup_auto.json")) {
        LOG_I(LOG_BACKUP, "Backup automatique reussi (fichier par defaut)");
      } else {
        LOG_E(LOG_BACKUP, "Echec du backup automatique");
      }
    }
    lastAutoBackup = millis();
  }
}

#endif // BACKUP_RESTORE_H