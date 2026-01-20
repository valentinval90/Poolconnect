/* 
 * POOL CONNECT - STORAGE
 * Gestion du stockage et persistence
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "globals.h"

// ============================================================================
// LITTLEFS
// ============================================================================

void listLittleFS() {
  Serial.println("LittleFS files:");
  File root = LittleFS.open("/");
  if (!root) return;
  File file = root.openNextFile();
  while (file) {
    Serial.printf(" %s (%d bytes)\n", file.name(), file.size());
    file = root.openNextFile();
  }
}

// ============================================================================
// CONFIGURATION SYSTÈME
// ============================================================================

void loadSystemConfig() {
  if (!LittleFS.exists("/system_config.json")) {
    // saveSystemConfig() sera appelé par le code principal
    return;
  }
  
  File f = LittleFS.open("/system_config.json", FILE_READ);
  if (!f) return;
  
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (!err) {
    sysConfig.pressureThreshold = doc["pressureThreshold"] | 2.0;
    sysConfig.buzzerEnabled = doc["buzzerEnabled"] | true;
    pressureThreshold = sysConfig.pressureThreshold;
    buzzerMuted = !sysConfig.buzzerEnabled;
  }
}

void saveSystemConfig() {
  File f = LittleFS.open("/system_config.json", FILE_WRITE);
  if (!f) return;
  
  StaticJsonDocument<256> doc;
  doc["pressureThreshold"] = sysConfig.pressureThreshold;
  doc["buzzerEnabled"] = sysConfig.buzzerEnabled;
  
  serializeJson(doc, f);
  f.close();
}

// ============================================================================
// HISTORIQUE
// ============================================================================

void saveHistory() {
  File f = LittleFS.open("/history.json", FILE_WRITE);
  if (!f) return;
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < historyCount; i++) {
    JsonObject o = arr.createNestedObject();
    o["date"] = history[i].date;
    o["startTime"] = history[i].startTime;
    o["duration"] = history[i].duration;
    o["avgTemp"] = history[i].avgTemp;
  }
  serializeJson(doc, f);
  f.close();
}

void loadHistory() {
  if (!LittleFS.exists("/history.json")) {
    historyCount = 0;
    return;
  }
  File f = LittleFS.open("/history.json", FILE_READ);
  if (!f) {
    historyCount = 0;
    return;
  }
  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) {
    historyCount = 0;
    return;
  }
  JsonArray arr = doc.as<JsonArray>();
  historyCount = 0;
  for (JsonObject o : arr) {
    if (historyCount >= MAX_HISTORY) break;
    strcpy(history[historyCount].date, o["date"]);
    strcpy(history[historyCount].startTime, o["startTime"]);
    history[historyCount].duration = o["duration"];
    history[historyCount].avgTemp = o["avgTemp"];
    historyCount++;
  }
}

void addHistoryEntry(int duration, float avgTemp) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  if (historyCount >= MAX_HISTORY) {
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
  historyCount++;
  saveHistory();
}

// ============================================================================
// TIMERS PERSISTENCE
// ============================================================================

void saveFlexTimers() {
  File file = LittleFS.open("/timers_flex.json", "w");
  if (!file) {
    Serial.println("❌ Erreur écriture timers");
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
      
      // NOUVEAU : Équation personnalisée
      if (t->actions[a].type == ACTION_AUTO_DURATION) {
        JsonObject eq = act.createNestedObject("customEquation");
        eq["useCustom"] = t->actions[a].customEquation.useCustom;
        eq["expression"] = t->actions[a].customEquation.expression;
      }
    }
  }
  serializeJson(doc, file);
  file.close();
  Serial.println("✓ Timers sauvegardés");
}

void loadFlexTimers() {
  if (!LittleFS.exists("/timers_flex.json")) {
    Serial.println("ℹ️  Aucun timer flexible");
    flexTimerCount = 0;
    return;
  }
  
  File file = LittleFS.open("/timers_flex.json", "r");
  if (!file) {
    Serial.println("❌ Erreur lecture timers");
    flexTimerCount = 0;
    return;
  }
  
  DynamicJsonDocument doc(32768);
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  
  if (err) {
    Serial.printf("❌ Erreur parse JSON: %s\n", err.c_str());
    flexTimerCount = 0;
    return;
  }
  
  JsonArray arr = doc.as<JsonArray>();
  flexTimerCount = 0;
  
  for (JsonObject obj : arr) {
    if (flexTimerCount >= MAX_TIMERS) break;
    
    FlexibleTimer* t = &flexTimers[flexTimerCount];
    
    t->id = obj["id"];
    t->name = obj["name"].as<String>();
    t->enabled = obj["enabled"] | true;
    
    JsonArray days = obj["days"];
    for (int i = 0; i < 7; i++) t->days[i] = days[i];
    
    JsonObject startObj = obj["startTime"];
    t->startTime.type = (StartTimeType)(int)startObj["type"];
    t->startTime.hour = startObj["hour"];
    t->startTime.minute = startObj["minute"];
    t->startTime.sunriseOffset = startObj["sunriseOffset"] | 0;
    
    JsonArray condArr = obj["conditions"];
    t->conditionCount = condArr.size();
    for (int i = 0; i < t->conditionCount && i < 10; i++) {
      JsonObject c = condArr[i];
      t->conditions[i].type = (ConditionType)(int)c["type"];
      t->conditions[i].value = c["value"];
      t->conditions[i].required = c["required"] | true;
    }
    
    JsonArray actArr = obj["actions"];
    t->actionCount = actArr.size();
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
      // NOUVEAU : Équation personnalisée
      if (a.containsKey("customEquation")) {
        JsonObject eq = a["customEquation"];
        t->actions[i].customEquation.useCustom = eq["useCustom"] | false;
        t->actions[i].customEquation.expression = eq["expression"].as<String>();
      }
    }
    
    t->context.state = TIMER_IDLE;
    t->context.currentActionIndex = 0;
    t->lastTriggeredDay = -1;
    
    flexTimerCount++;
  }
  
  Serial.printf("✓ %d timers flexibles chargés\n", flexTimerCount);
}

#endif // STORAGE_H
