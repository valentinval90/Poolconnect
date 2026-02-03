/* 
 * POOL CONNECT - WEB HANDLERS IMPLEMENTATION
 * Implémentations complètes des handlers HTTP
 * web_handlers.h   V0.4
 */

#ifndef WEB_HANDLERS_IMPL_H
#define WEB_HANDLERS_IMPL_H

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "config.h"
#include "logging.h"
#include "users.h"
#include "storage.h"
#include "mqtt_manager.h"
#include "weather.h"
#include "sensors.h"
#include "backup_restore.h"
#include "scenarios.h"
#include "chart_event_points.h"

// ============================================================================
// FICHIERS STATIQUES
// ============================================================================

void handleRoot() {
  LOG_WEB_REQUEST("GET", "/");
  
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /index.html non trouve");
    server.send(404, "text/plain", "index.html not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /index.html (%d bytes)", file.size());
  server.streamFile(file, "text/html");
  file.close();
}

void handleCss() {
  LOG_WEB_REQUEST("GET", "/style.css");
  
  File file = LittleFS.open("/style.css", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /style.css non trouve");
    server.send(404, "text/plain", "CSS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /style.css (%d bytes)", file.size());
  server.streamFile(file, "text/css");
  file.close();
}

void handleJs() {
  LOG_WEB_REQUEST("GET", "/app.js");
  
  File file = LittleFS.open("/app.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /app.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /app.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJSauth() {
  LOG_WEB_REQUEST("GET", "/modules/auth.js");
  
  File file = LittleFS.open("/modules/auth.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/auth.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/auth.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJsbackup() {
  LOG_WEB_REQUEST("GET", "/modules/backup.js");
  
  File file = LittleFS.open("/modules/backup.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/backup.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/backup.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJscalib() {
  LOG_WEB_REQUEST("GET", "/modules/calibration.js");
  
  File file = LittleFS.open("/modules/calibration.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/calibration.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/calibration.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJschart() {
  LOG_WEB_REQUEST("GET", "/modules/chart.js");
  
  File file = LittleFS.open("/modules/chart.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/chart.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/chart.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJscontrol() {
  LOG_WEB_REQUEST("GET", "/modules/control.js");
  
  File file = LittleFS.open("/modules/control.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/control.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/control.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJsdashboard() {
  LOG_WEB_REQUEST("GET", "/modules/dashboard.js");
  
  File file = LittleFS.open("/modules/dashboard.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/dashboard.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/dashboard.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJspref() {
  LOG_WEB_REQUEST("GET", "/modules/preferences.js");
  
  File file = LittleFS.open("/modules/preferences.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/preferences.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/preferences.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJsscenar() {
  LOG_WEB_REQUEST("GET", "/modules/scenarios.js");
  
  File file = LittleFS.open("/modules/scenarios.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/scenarios.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/scenarios.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJssettings() {
  LOG_WEB_REQUEST("GET", "/modules/settings.js");
  
  File file = LittleFS.open("/modules/settings.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/settings.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/settings.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJstheme() {
  LOG_WEB_REQUEST("GET", "/modules/theme.js");
  
  File file = LittleFS.open("/modules/theme.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/theme.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/theme.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJstimers() {
  LOG_WEB_REQUEST("GET", "/modules/timers.js");
  
  File file = LittleFS.open("/modules/timers.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/timers.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/timers.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJsunits() {
  LOG_WEB_REQUEST("GET", "/modules/units.js");
  
  File file = LittleFS.open("/modules/units.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/units.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/units.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleJsusers() {
  LOG_WEB_REQUEST("GET", "/modules/users.js");
  
  File file = LittleFS.open("/modules/users.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/users.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/users.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handlelangageJs() {
  LOG_WEB_REQUEST("GET", "/modules/translations.js");
  
  File file = LittleFS.open("/modules/translations.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/translations.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/translations.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleotaJs() {
  LOG_WEB_REQUEST("GET", "/modules/ota.js");
  
  File file = LittleFS.open("/modules/ota.js", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /modules/ota.js non trouve");
    server.send(404, "text/plain", "JS not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /modules/ota.js (%d bytes)", file.size());
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleLogo() {
  LOG_WEB_REQUEST("GET", "/img/logo.png");
  
  File file = LittleFS.open("/img/logo.png", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /img/logo.png non trouve");
    server.send(404, "text/plain", "logo not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /img/logo.png (%d bytes)", file.size());
  server.streamFile(file, "image/png");
  file.close();
}

void handleFavicon() {
  LOG_WEB_REQUEST("GET", "/img/favicon.png");
  
  File file = LittleFS.open("/img/favicon.png", "r");
  if (!file) {
    LOG_E(LOG_WEB, "Fichier /img/favicon.png non trouve");
    server.send(404, "text/plain", "Favicon not found");
    return;
  }
  
  LOG_V(LOG_WEB, "Envoi de /img/favicon.png (%d bytes)", file.size());
  server.streamFile(file, "image/png");
  file.close();
}


// ============================================================================
// API BASIQUE - TEMPS ET CAPTEURS
// ============================================================================

void handleApiTime() {
  LOG_WEB_REQUEST("GET", "/api/time");
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[64];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &timeinfo);
    LOG_V(LOG_WEB, "Heure envoyee: %s", buf);
    server.send(200, "text/plain", buf);
  } else {
    LOG_W(LOG_WEB, "Heure non disponible (NTP non synchronise)");
    server.send(200, "text/plain", "-- / -- / ---- --:--:--");
  }
}

void handleApiTemp() {
  LOG_WEB_REQUEST("GET", "/api/temp");
  
  if (isnan(waterTemp) || waterTemp < -50 || waterTemp > 100) {
    LOG_W(LOG_WEB, "Temperature invalide: %.2f", waterTemp);
    server.send(200, "text/plain", "ERREUR");
    return;
  }
  
  LOG_V(LOG_WEB, "Temperature envoyee: %.2f C", waterTemp);
  server.send(200, "text/plain", String(waterTemp, 2));
}

void handleApiRelays() {
  LOG_WEB_REQUEST("GET", "/api/relays");
  
  DynamicJsonDocument doc(256);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < NUM_RELAYS; i++) {
    bool state = (digitalRead(relayPins[i]) == HIGH);
    arr.add(state);
    LOG_V(LOG_WEB, "Relais %d: %s", i, state ? "ON" : "OFF");
  }
  String out;
  serializeJson(arr, out);
  LOG_V(LOG_WEB, "Etats relais envoyes: %s", out.c_str());
  server.send(200, "application/json", out);
}

void handleApiRelay() {
  LOG_WEB_REQUEST("POST", "/api/relay");
  
  if (!server.hasArg("ch") || !server.hasArg("state")) {
    LOG_E(LOG_WEB, "Parametres manquants: ch ou state");
    server.send(400, "text/plain", "Bad Request");
    return;
  }
  
  int ch = server.arg("ch").toInt();
  int state = server.arg("state").toInt();
  
  LOG_D(LOG_WEB, "Commande relais: ch=%d, state=%d", ch, state);
  
  if (ch < 0 || ch >= NUM_RELAYS) {
    LOG_E(LOG_WEB, "Index relais invalide: %d", ch);
    server.send(400, "text/plain", "Invalid relay");
    return;
  }
  
  // Protection électrolyseur - nécessite pompe active
  if (ch == 1 && state == 1) {
    bool pompeActive = (digitalRead(relayPins[0]) == HIGH);
    if (!pompeActive) {
      LOG_W(LOG_WEB, "PROTECTION: Tentative d'activation electrolyseur sans pompe");
      server.send(400, "text/plain", "Erreur: Pompe doit être active");
      return;
    }
  }
  
  digitalWrite(relayPins[ch], state ? HIGH : LOW);
  LOG_I(LOG_WEB, "Relais %d -> %s", ch, state ? "ON" : "OFF");

  //Capturer le changement d'état sur le graphique
  captureCurrentStateToChart();

  if (mqttClient.connected()) {
    String topic = mqttTopic + "/relay/" + String(ch) + "/state";
    mqttClient.publish(topic.c_str(), state ? "1" : "0");
    LOG_MQTT_PUB(topic.c_str(), state ? "1" : "0");
  }
  
  server.send(200, "text/plain", "OK");
}

void handleApiSensors() {
  LOG_WEB_REQUEST("GET", "/api/sensors");
  
  DynamicJsonDocument doc(512);
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
    doc["waterTemp"] = waterTemp;
    doc["waterPressure"] = waterPressure;
    doc["waterLeak"] = waterLeak;
    doc["coverOpen"] = coverOpen;
    doc["extTemp"] = tempExterieure;
    xSemaphoreGive(dataMutex);
    
    LOG_V(LOG_WEB, "Capteurs: T=%.2f, P=%.2f, Fuite=%d, Volet=%d, Text=%.2f",
          waterTemp, waterPressure, waterLeak, coverOpen, tempExterieure);
  } else {
    LOG_E(LOG_WEB, "Impossible d'obtenir le mutex pour les capteurs");
  }
  
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiBuzzerMute() {
  LOG_WEB_REQUEST("POST", "/api/buzzer/mute");
  
  if (server.hasArg("state")) {
    buzzerMuted = (server.arg("state").toInt() == 1);
    LOG_I(LOG_WEB, "Buzzer: %s", buzzerMuted ? "MUTE" : "ACTIVE");
    server.send(200, "text/plain", "OK");
  } else {
    LOG_E(LOG_WEB, "Parametre 'state' manquant");
    server.send(400, "text/plain", "Missing state");
  }
}

// ============================================================================
// API CALIBRATION
// ============================================================================

void handleApiGetCalibration() {
  LOG_WEB_REQUEST("GET", "/api/calibration");
  
  DynamicJsonDocument doc(1024);
  
  JsonObject temp = doc.createNestedObject("temperature");
  temp["useCalibration"] = calibConfig.tempUseCalibration;
  temp["useTwoPoint"] = calibConfig.tempUseTwoPoint;
  temp["offset"] = calibConfig.tempOffset;
  temp["point1Raw"] = calibConfig.tempPoint1Raw;
  temp["point1Real"] = calibConfig.tempPoint1Real;
  temp["point2Raw"] = calibConfig.tempPoint2Raw;
  temp["point2Real"] = calibConfig.tempPoint2Real;
  
  JsonObject pressure = doc.createNestedObject("pressure");
  pressure["useCalibration"] = calibConfig.pressureUseCalibration;
  pressure["useTwoPoint"] = calibConfig.pressureUseTwoPoint;
  pressure["offset"] = calibConfig.pressureOffset;
  pressure["point1Raw"] = calibConfig.pressurePoint1Raw;
  pressure["point1Real"] = calibConfig.pressurePoint1Real;
  pressure["point2Raw"] = calibConfig.pressurePoint2Raw;
  pressure["point2Real"] = calibConfig.pressurePoint2Real;
  
  LOG_V(LOG_WEB, "Calibration envoyee: Temp(use=%d, 2pt=%d), Press(use=%d, 2pt=%d)",
        calibConfig.tempUseCalibration, calibConfig.tempUseTwoPoint,
        calibConfig.pressureUseCalibration, calibConfig.pressureUseTwoPoint);
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleApiSaveCalibration() {
  LOG_WEB_REQUEST("POST", "/api/calibration");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    LOG_E(LOG_WEB, "Erreur parsing JSON calibration: %s", err.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  JsonObject temp = doc["temperature"];
  calibConfig.tempUseCalibration = temp["useCalibration"] | false;
  calibConfig.tempUseTwoPoint = temp["useTwoPoint"] | false;
  calibConfig.tempOffset = temp["offset"] | 0.0;
  calibConfig.tempPoint1Raw = temp["point1Raw"] | 10.0;
  calibConfig.tempPoint1Real = temp["point1Real"] | 10.0;
  calibConfig.tempPoint2Raw = temp["point2Raw"] | 30.0;
  calibConfig.tempPoint2Real = temp["point2Real"] | 30.0;
  
  JsonObject pressure = doc["pressure"];
  calibConfig.pressureUseCalibration = pressure["useCalibration"] | false;
  calibConfig.pressureUseTwoPoint = pressure["useTwoPoint"] | false;
  calibConfig.pressureOffset = pressure["offset"] | 0.0;
  calibConfig.pressurePoint1Raw = pressure["point1Raw"] | 1.0;
  calibConfig.pressurePoint1Real = pressure["point1Real"] | 1.0;
  calibConfig.pressurePoint2Raw = pressure["point2Raw"] | 3.0;
  calibConfig.pressurePoint2Real = pressure["point2Real"] | 3.0;
  
  LOG_I(LOG_WEB, "Calibration sauvegardee: Temp(use=%d, offset=%.2f), Press(use=%d, offset=%.2f)",
        calibConfig.tempUseCalibration, calibConfig.tempOffset,
        calibConfig.pressureUseCalibration, calibConfig.pressureOffset);
  
  saveCalibrationConfig();
  server.send(200, "text/plain", "OK");
}

void handleApiResetCalibration() {
  LOG_WEB_REQUEST("POST", "/api/calibration/reset");
  
  calibConfig.tempUseCalibration = false;
  calibConfig.tempUseTwoPoint = false;
  calibConfig.tempOffset = 0.0;
  calibConfig.tempPoint1Raw = 10.0;
  calibConfig.tempPoint1Real = 10.0;
  calibConfig.tempPoint2Raw = 30.0;
  calibConfig.tempPoint2Real = 30.0;
  
  calibConfig.pressureUseCalibration = false;
  calibConfig.pressureUseTwoPoint = false;
  calibConfig.pressureOffset = 0.0;
  calibConfig.pressurePoint1Raw = 1.0;
  calibConfig.pressurePoint1Real = 1.0;
  calibConfig.pressurePoint2Raw = 3.0;
  calibConfig.pressurePoint2Real = 3.0;
  
  LOG_I(LOG_WEB, "Calibration remise a zero");
  
  saveCalibrationConfig();
  server.send(200, "text/plain", "Calibration reset");
}

// ============================================================================
// API TIMERS FLEXIBLES
// ============================================================================

void handleApiFlexTimers() {
  LOG_WEB_REQUEST("GET", "/api/timers/flex");
  
  DynamicJsonDocument doc(32768);
  JsonArray arr = doc.to<JsonArray>();
  
  LOG_D(LOG_WEB, "Generation JSON pour %d timers", flexTimerCount);
  
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
    
    obj["conditionCount"] = t->conditionCount;
    obj["actionCount"] = t->actionCount;
    
    JsonObject ctx = obj.createNestedObject("context");
    ctx["state"] = (int)t->context.state;
    ctx["currentAction"] = t->context.currentActionIndex;
    ctx["totalElapsedMinutes"] = t->context.totalElapsedMinutes;
    if (t->context.lastError.length() > 0) {
      ctx["lastError"] = t->context.lastError;
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
      
      if (t->actions[a].type == ACTION_AUTO_DURATION) {
        JsonObject eq = act.createNestedObject("customEquation");
        eq["useCustom"] = t->actions[a].customEquation.useCustom;
        eq["expression"] = t->actions[a].customEquation.expression;
      }
    }
    
    LOG_V(LOG_WEB, "Timer %d: '%s' (enabled=%d, actions=%d, state=%d)",
          i, t->name.c_str(), t->enabled, t->actionCount, (int)t->context.state);
  }
  
  String output;
  size_t jsonSize = serializeJson(doc, output);
  LOG_I(LOG_WEB, "JSON timers genere: %d timers, %d bytes", flexTimerCount, jsonSize);
  
  server.send(200, "application/json", output);
}

void handleApiAddFlexTimer() {
  LOG_WEB_REQUEST("POST", "/api/timers/flex");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  if (flexTimerCount >= MAX_TIMERS) {
    LOG_E(LOG_WEB, "Limite de timers atteinte: %d/%d", flexTimerCount, MAX_TIMERS);
    server.send(400, "text/plain", "Max timers reached");
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    LOG_E(LOG_WEB, "Erreur parsing JSON timer: %s", err.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  FlexibleTimer* t = &flexTimers[flexTimerCount];
  JsonObject obj = doc.as<JsonObject>();
  
  t->id = obj["id"] | (int)(millis() & 0x7FFFFFFF);
  t->name = obj["name"].as<String>();
  t->enabled = obj["enabled"] | true;
  
  LOG_D(LOG_WEB, "Ajout timer: ID=%d, Nom='%s', Enabled=%d", t->id, t->name.c_str(), t->enabled);
  
  JsonArray days = obj["days"];
  for (int i = 0; i < 7; i++) t->days[i] = days[i];
  
  JsonObject startObj = obj["startTime"];
  t->startTime.type = (StartTimeType)(int)startObj["type"];
  t->startTime.hour = startObj["hour"];
  t->startTime.minute = startObj["minute"];
  t->startTime.sunriseOffset = startObj["sunriseOffset"] | 0;
  
  JsonArray condArr = obj["conditions"];
  t->conditionCount = condArr.size();
  LOG_V(LOG_WEB, "Conditions: %d", t->conditionCount);
  
  for (int i = 0; i < t->conditionCount && i < 10; i++) {
    JsonObject c = condArr[i];
    t->conditions[i].type = (ConditionType)(int)c["type"];
    t->conditions[i].value = c["value"];
    t->conditions[i].required = c["required"] | true;
  }
  
  JsonArray actArr = obj["actions"];
  t->actionCount = actArr.size();
  LOG_V(LOG_WEB, "Actions: %d", t->actionCount);
  
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
  LOG_I(LOG_WEB, "Timer ajoute avec succes: '%s' (total: %d/%d)",
        t->name.c_str(), flexTimerCount, MAX_TIMERS);
  
  saveFlexTimers();
  
  server.send(200, "text/plain", "OK");
}

void handleApiUpdateFlexTimer() {
  LOG_WEB_REQUEST("PUT", "/api/timers/flex/[id]");
  
  String uri = server.uri();
  int flexPos = uri.indexOf("flex/");
  if (flexPos == -1) {
    LOG_E(LOG_WEB, "URI invalide: %s", uri.c_str());
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  int idStart = flexPos + 5;
  String idStr = uri.substring(idStart);
  int id = idStr.toInt();
  
  LOG_D(LOG_WEB, "Mise a jour timer ID: %d", id);
  
  if (id == 0) {
    LOG_E(LOG_WEB, "ID timer invalide: %d", id);
    server.send(400, "text/plain", "Invalid ID");
    return;
  }
  
  FlexibleTimer* timer = nullptr;
  for (int i = 0; i < flexTimerCount; i++) {
    if (flexTimers[i].id == id) {
      timer = &flexTimers[i];
      break;
    }
  }
  
  if (!timer) {
    LOG_W(LOG_WEB, "Timer ID %d non trouve", id);
    server.send(404, "text/plain", "Timer not found");
    return;
  }
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    LOG_E(LOG_WEB, "Erreur parsing JSON: %s", err.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  JsonObject obj = doc.as<JsonObject>();
  timer->name = obj["name"].as<String>();
  timer->enabled = obj["enabled"] | true;
  
  LOG_I(LOG_WEB, "Mise a jour timer ID %d: '%s', enabled=%d", id, timer->name.c_str(), timer->enabled);
  
  JsonArray days = obj["days"];
  for (int i = 0; i < 7; i++) timer->days[i] = days[i];
  
  JsonObject startObj = obj["startTime"];
  timer->startTime.type = (StartTimeType)(int)startObj["type"];
  timer->startTime.hour = startObj["hour"];
  timer->startTime.minute = startObj["minute"];
  timer->startTime.sunriseOffset = startObj["sunriseOffset"] | 0;
  
  JsonArray condArr = obj["conditions"];
  timer->conditionCount = condArr.size();
  for (int i = 0; i < timer->conditionCount && i < 10; i++) {
    JsonObject c = condArr[i];
    timer->conditions[i].type = (ConditionType)(int)c["type"];
    timer->conditions[i].value = c["value"];
    timer->conditions[i].required = c["required"] | true;
  }
  
  JsonArray actArr = obj["actions"];
  timer->actionCount = actArr.size();
  for (int i = 0; i < timer->actionCount && i < 50; i++) {
    JsonObject a = actArr[i];
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
  
  // Arrêter le timer s'il était en cours
  if (timer->context.state == TIMER_RUNNING) {
    LOG_W(LOG_WEB, "Timer ID %d en cours d'execution - Arret des relais", id);
    for (int a = 0; a < timer->actionCount; a++) {
      if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
        digitalWrite(relayPins[timer->actions[a].relay], LOW);
        LOG_V(LOG_WEB, "Relais %d eteint", timer->actions[a].relay);
      }
    }
  }
  
  timer->context.state = TIMER_IDLE;
  timer->context.currentActionIndex = 0;
  timer->context.tempMeasured = false;
  timer->context.lastError = "";
  timer->lastTriggeredDay = -1;
  
  LOG_I(LOG_WEB, "Timer ID %d mis a jour avec succes", id);
  
  saveFlexTimers();
  server.send(200, "text/plain", "OK");
}

void handleApiDeleteFlexTimer() {
  LOG_WEB_REQUEST("DELETE", "/api/timers/flex/[id]");
  
  String uri = server.uri();
  int flexPos = uri.indexOf("flex/");
  if (flexPos == -1) {
    LOG_E(LOG_WEB, "URI invalide: %s", uri.c_str());
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  int idStart = flexPos + 5;
  String idStr = uri.substring(idStart);
  int id = idStr.toInt();
  
  LOG_D(LOG_WEB, "Suppression timer ID: %d", id);
  
  if (id == 0) {
    LOG_E(LOG_WEB, "ID timer invalide: %d", id);
    server.send(400, "text/plain", "Invalid ID");
    return;
  }
  
  int index = -1;
  for (int i = 0; i < flexTimerCount; i++) {
    if (flexTimers[i].id == id) {
      index = i;
      break;
    }
  }
  
  if (index == -1) {
    LOG_W(LOG_WEB, "Timer ID %d non trouve", id);
    server.send(404, "text/plain", "Timer not found");
    return;
  }
  
  String timerName = flexTimers[index].name;
  
  // Décalage des timers
  for (int i = index; i < flexTimerCount - 1; i++) {
    flexTimers[i] = flexTimers[i + 1];
  }
  flexTimerCount--;
  
  LOG_I(LOG_WEB, "Timer '%s' (ID %d) supprime (total: %d/%d)",
        timerName.c_str(), id, flexTimerCount, MAX_TIMERS);
  
  saveFlexTimers();
  server.send(200, "text/plain", "OK");
}

void handleApiToggleFlexTimer() {
  LOG_WEB_REQUEST("POST", "/api/timers/flex/[id]/toggle");
  
  String uri = server.uri();
  int flexPos = uri.indexOf("flex/");
  if (flexPos == -1) {
    LOG_E(LOG_WEB, "URI invalide: %s", uri.c_str());
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  int idStart = flexPos + 5;
  int idEnd = uri.indexOf('/', idStart);
  
  if (idEnd == -1) {
    LOG_E(LOG_WEB, "URI invalide (pas de slash apres ID): %s", uri.c_str());
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  String idStr = uri.substring(idStart, idEnd);
  int id = idStr.toInt();
  
  LOG_D(LOG_WEB, "Toggle timer ID: %d", id);
  
  if (id == 0) {
    LOG_E(LOG_WEB, "ID timer invalide: %d", id);
    server.send(400, "text/plain", "Invalid ID");
    return;
  }
  
  FlexibleTimer* timer = nullptr;
  for (int i = 0; i < flexTimerCount; i++) {
    if (flexTimers[i].id == id) {
      timer = &flexTimers[i];
      break;
    }
  }
  
  if (!timer) {
    LOG_W(LOG_WEB, "Timer ID %d non trouve", id);
    server.send(404, "text/plain", "Timer not found");
    return;
  }
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<128> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    LOG_E(LOG_WEB, "Erreur parsing JSON: %s", err.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  bool newEnabled = doc["enabled"];
  
  LOG_I(LOG_WEB, "Toggle timer '%s' (ID %d): %s -> %s",
        timer->name.c_str(), id, timer->enabled ? "ON" : "OFF", newEnabled ? "ON" : "OFF");
  
  // Si on désactive un timer en cours, arrêter les relais
  if (!newEnabled && timer->context.state == TIMER_RUNNING) {
    LOG_W(LOG_WEB, "Timer ID %d desactive - Arret des relais actifs", id);
    for (int a = 0; a < timer->actionCount; a++) {
      if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
        digitalWrite(relayPins[timer->actions[a].relay], LOW);
        LOG_V(LOG_WEB, "Relais %d eteint", timer->actions[a].relay);
      }
    }
    timer->context.state = TIMER_IDLE;
    timer->context.currentActionIndex = 0;
  }
  
  timer->enabled = newEnabled;
  saveFlexTimers();
  server.send(200, "text/plain", "OK");
}

// ============================================================================
// API MQTT
// ============================================================================

void handleApiMQTTConfig() {
  LOG_WEB_REQUEST("GET", "/api/mqtt/config");
  
  DynamicJsonDocument doc(512);
  doc["server"] = mqttServer;
  doc["port"] = mqttPort;
  doc["user"] = mqttUser;
  doc["password"] = "";  // Ne pas renvoyer le mot de passe
  doc["topic"] = mqttTopic;
  
  LOG_V(LOG_WEB, "Config MQTT envoyee: server=%s, port=%d, user=%s, topic=%s",
        mqttServer.c_str(), mqttPort, mqttUser.c_str(), mqttTopic.c_str());
  
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiSaveMQTT() {
  LOG_WEB_REQUEST("POST", "/api/mqtt/config");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server.arg("plain"))) {
    LOG_E(LOG_WEB, "Erreur parsing JSON MQTT");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  mqttServer = doc["server"].as<String>();
  mqttPort = doc["port"] | 1883;
  mqttUser = doc["user"].as<String>();
  mqttPassword = doc["password"].as<String>();
  mqttTopic = doc["topic"].as<String>();
  
  LOG_I(LOG_WEB, "Config MQTT sauvegardee: server=%s:%d, user=%s, topic=%s",
        mqttServer.c_str(), mqttPort, mqttUser.c_str(), mqttTopic.c_str());
  
  saveMQTTConfig();
  mqttClient.setServer(mqttServer.c_str(), mqttPort);
  mqttClient.setCallback(mqttCallback);
  
  LOG_D(LOG_WEB, "Client MQTT reconfigure");
  
  server.send(200, "text/plain", "OK");
}

void handleApiMQTTStatus() {
  LOG_WEB_REQUEST("GET", "/api/mqtt/status");
  
  bool connected = mqttClient.connected();
  
  DynamicJsonDocument doc(128);
  doc["connected"] = connected;
  
  LOG_V(LOG_WEB, "Status MQTT: %s", connected ? "CONNECTE" : "DECONNECTE");
  
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiMQTTRediscover() {
  LOG_WEB_REQUEST("POST", "/api/mqtt/rediscover");
  
  if (mqttClient.connected()) {
    LOG_I(LOG_WEB, "Republication Home Assistant Discovery");
    publishHomeAssistantDiscovery();
    publishSensorStates();
    server.send(200, "text/plain", "Discovery published");
  } else {
    LOG_W(LOG_WEB, "MQTT non connecte - Impossible de publier discovery");
    server.send(503, "text/plain", "MQTT not connected");
  }
}

// ============================================================================
// API MÉTÉO
// ============================================================================

void handleApiWeatherConfig() {
  LOG_WEB_REQUEST("GET", "/api/weather/config");
  
  DynamicJsonDocument doc(512);
  doc["apiKey"] = weatherApiKey;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;
  
  LOG_V(LOG_WEB, "Config meteo envoyee: lat=%s, lon=%s, apiKey=%s",
        latitude.c_str(), longitude.c_str(), 
        weatherApiKey.length() > 0 ? "configuree" : "non configuree");
  
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiWeatherSave() {
  LOG_WEB_REQUEST("POST", "/api/weather/config");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server.arg("plain"))) {
    LOG_E(LOG_WEB, "Erreur parsing JSON meteo");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  weatherApiKey = doc["apiKey"].as<String>();
  latitude = doc["latitude"].as<String>();
  longitude = doc["longitude"].as<String>();
  
  LOG_I(LOG_WEB, "Config meteo sauvegardee: lat=%s, lon=%s",
        latitude.c_str(), longitude.c_str());
  
  saveWeatherConfig();
  
  LOG_I(LOG_WEB, "Declenchement mise a jour meteo");
  updateWeatherData();
  
  server.send(200, "text/plain", "OK");
}

// ============================================================================
// API SYSTÈME
// ============================================================================

void handleApiSystem() {
  LOG_WEB_REQUEST("GET", "/api/system");
  
  DynamicJsonDocument doc(512);
  doc["version"] = FIRMWARE_VERSION;
  doc["ip"] = WiFi.localIP().toString();
  doc["uptime"] = millis() / 1000;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["chipId"] = (uint32_t)ESP.getEfuseMac();
  
  LOG_V(LOG_WEB, "Info systeme: uptime=%lu s, heap=%lu bytes, IP=%s",
        millis()/1000, (unsigned long)ESP.getFreeHeap(), WiFi.localIP().toString().c_str());
  
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiGetSystemConfig() {
  LOG_WEB_REQUEST("GET", "/api/system/config");
  
  StaticJsonDocument<256> doc;
  doc["pressureThreshold"] = sysConfig.pressureThreshold;
  doc["buzzerEnabled"] = sysConfig.buzzerEnabled;
  
  LOG_V(LOG_WEB, "Config systeme: seuil pression=%.2f BAR, buzzer=%s",
        sysConfig.pressureThreshold, sysConfig.buzzerEnabled ? "ON" : "OFF");
  
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiSaveSystemConfig() {
  LOG_WEB_REQUEST("POST", "/api/system/config");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    LOG_E(LOG_WEB, "Erreur parsing JSON config systeme");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  sysConfig.pressureThreshold = doc["pressureThreshold"] | 2.0;
  sysConfig.buzzerEnabled = doc["buzzerEnabled"] | true;
  pressureThreshold = sysConfig.pressureThreshold;
  buzzerMuted = !sysConfig.buzzerEnabled;
  
  LOG_I(LOG_WEB, "Config systeme sauvegardee: seuil=%.2f BAR, buzzer=%s",
        sysConfig.pressureThreshold, sysConfig.buzzerEnabled ? "ON" : "OFF");
  
  saveSystemConfig();
  server.send(200, "text/plain", "OK");
}

void handleApiRestart() {
  LOG_WEB_REQUEST("POST", "/api/system/restart");
  
  LOG_W(LOG_WEB, "REDEMARRAGE SYSTEME DEMANDE");
  
  server.send(200, "text/plain", "Restarting...");
  delay(1000);
  
  LOG_I(LOG_WEB, "Redemarrage en cours...");
  ESP.restart();
}

void handleApiGetPreferences() {
  LOG_WEB_REQUEST("GET", "/api/preferences");
  
  // Construction de la réponse JSON
  StaticJsonDocument<512> doc;
  doc["language"] = userPrefs.language;
  doc["temperatureUnit"] = userPrefs.temperatureUnit;
  doc["pressureUnit"] = userPrefs.pressureUnit;
  doc["theme"] = userPrefs.theme;
  doc["chartUpdateInterval"] = userPrefs.chartUpdateInterval;
  
  String response;
  serializeJson(doc, response);
  
  LOG_V(LOG_WEB, "Preferences envoyees: lang=%s, tempUnit=%s, theme=%s", 
        userPrefs.language.c_str(), 
        userPrefs.temperatureUnit.c_str(),
        userPrefs.theme.c_str());
  LOG_WEB_JSON(response.c_str());
  
  server.send(200, "application/json", response);
  LOG_WEB_RESPONSE("/api/preferences", 200);
}

void handleApiSavePreferences() {
  LOG_WEB_REQUEST("POST", "/api/preferences");
  
  // Vérifier le body
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  // Parser le JSON
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    LOG_E(LOG_WEB, "Erreur parsing JSON preferences: %s", error.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  bool updated = false;
  
  // Mise à jour de la langue
  if (doc.containsKey("language")) {
    String newLang = doc["language"].as<String>();
    if (newLang == "fr" || newLang == "en") {
      userPrefs.language = newLang;
      updated = true;
      LOG_I(LOG_WEB, "Langue mise a jour: %s", userPrefs.language.c_str());
    } else {
      LOG_W(LOG_WEB, "Langue invalide: %s (doit etre 'fr' ou 'en')", newLang.c_str());
    }
  }
  
  // Mise à jour de l'unité de température
  if (doc.containsKey("temperatureUnit")) {
    String newUnit = doc["temperatureUnit"].as<String>();
    if (newUnit == "celsius" || newUnit == "fahrenheit") {
      userPrefs.temperatureUnit = newUnit;
      updated = true;
      LOG_I(LOG_WEB, "Unite temperature mise a jour: %s", userPrefs.temperatureUnit.c_str());
    } else {
      LOG_W(LOG_WEB, "Unite temperature invalide: %s", newUnit.c_str());
    }
  }
  
  // Mise à jour de l'unité de pression
  if (doc.containsKey("pressureUnit")) {
    String newUnit = doc["pressureUnit"].as<String>();
    if (newUnit == "bar" || newUnit == "psi") {
      userPrefs.pressureUnit = newUnit;
      updated = true;
      LOG_I(LOG_WEB, "Unite pression mise a jour: %s", userPrefs.pressureUnit.c_str());
    } else {
      LOG_W(LOG_WEB, "Unite pression invalide: %s", newUnit.c_str());
    }
  }
  
  // Mise à jour du thème
  if (doc.containsKey("theme")) {
    String newTheme = doc["theme"].as<String>();
    if (newTheme == "light" || newTheme == "dark") {
      userPrefs.theme = newTheme;
      updated = true;
      LOG_I(LOG_WEB, "Theme mis a jour: %s", userPrefs.theme.c_str());
    } else {
      LOG_W(LOG_WEB, "Theme invalide: %s", newTheme.c_str());
    }
  }
  
  // Mise à jour de l'intervalle du graphique
  if (doc.containsKey("chartUpdateInterval")) {
    int newInterval = doc["chartUpdateInterval"];
    if (newInterval >= 60000 && newInterval <= 3600000) { // Entre 1min et 1h
      userPrefs.chartUpdateInterval = newInterval;
      updated = true;
      LOG_I(LOG_WEB, "Intervalle graphique mis a jour: %d ms", userPrefs.chartUpdateInterval);
    } else {
      LOG_W(LOG_WEB, "Intervalle graphique invalide: %d ms (doit etre entre 60000 et 3600000)", newInterval);
    }
  }
  
  if (updated) {
    // Sauvegarder dans LittleFS
    saveSystemConfig();
    LOG_I(LOG_WEB, "Preferences utilisateur sauvegardees avec succes");
    
    // Construire la réponse avec les préférences mises à jour
    StaticJsonDocument<512> respDoc;
    respDoc["success"] = true;
    respDoc["message"] = "Preferences updated";
    
    JsonObject prefs = respDoc.createNestedObject("preferences");
    prefs["language"] = userPrefs.language;
    prefs["temperatureUnit"] = userPrefs.temperatureUnit;
    prefs["pressureUnit"] = userPrefs.pressureUnit;
    prefs["theme"] = userPrefs.theme;
    prefs["chartUpdateInterval"] = userPrefs.chartUpdateInterval;
    
    String response;
    serializeJson(respDoc, response);
    
    server.send(200, "application/json", response);
    LOG_WEB_RESPONSE("/api/preferences", 200);
  } else {
    LOG_W(LOG_WEB, "Aucune preference valide a mettre a jour");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"No valid preferences to update\"}");
    LOG_WEB_RESPONSE("/api/preferences", 400);
  }
}

// ============================================================================
// API HISTORIQUE
// ============================================================================

void handleApiHistory() {
  LOG_WEB_REQUEST("GET", "/api/history");
  
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.to<JsonArray>();
  
  LOG_D(LOG_WEB, "Generation JSON pour %d entrees historique", historyCount);
  
  for (int i = historyCount - 1; i >= 0; i--) {
    JsonObject o = arr.createNestedObject();
    o["date"] = history[i].date;
    o["startTime"] = history[i].startTime;
    o["duration"] = history[i].duration;
    o["avgTemp"] = history[i].avgTemp;
  }
  
  String out;
  size_t jsonSize = serializeJson(doc, out);
  LOG_I(LOG_WEB, "Historique envoye: %d entrees, %d bytes", historyCount, jsonSize);
  
  server.send(200, "application/json", out);
}

// ============================================================================
// API AUTHENTIFICATION & UTILISATEURS
// ============================================================================

void handleApiAuth() {
  LOG_WEB_REQUEST("POST", "/api/auth");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    LOG_E(LOG_WEB, "Erreur parsing JSON auth");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  String username = doc["username"].as<String>();
  String password = doc["password"].as<String>();
  
  LOG_D(LOG_WEB, "Tentative d'authentification: %s", username.c_str());
  
  if (authenticateUser(username, password)) {
    String role = getUserRole(username);
    
    LOG_I(LOG_WEB, "Authentification reussie: %s (role=%s)", username.c_str(), role.c_str());
    
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["role"] = role;
    response["username"] = username;
    String out;
    serializeJson(response, out);
    server.send(200, "application/json", out);
  } else {
    LOG_W(LOG_WEB, "Authentification echouee: %s", username.c_str());
    server.send(401, "text/plain", "Unauthorized");
  }
}

void handleApiGetUsers() {
  LOG_WEB_REQUEST("GET", "/api/users");
  
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.to<JsonArray>();
  
  LOG_D(LOG_WEB, "Generation liste des %d utilisateurs", userCount);
  
  for (int i = 0; i < userCount; i++) {
    JsonObject o = arr.createNestedObject();
    o["username"] = users[i].username;
    o["role"] = users[i].role;
    o["enabled"] = users[i].enabled;
  }
  
  String out;
  serializeJson(doc, out);
  LOG_V(LOG_WEB, "Liste utilisateurs envoyee: %d users", userCount);
  
  server.send(200, "application/json", out);
}

void handleApiAddUser() {
  LOG_WEB_REQUEST("POST", "/api/users");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  if (userCount >= MAX_USERS) {
    LOG_E(LOG_WEB, "Limite utilisateurs atteinte: %d/%d", userCount, MAX_USERS);
    server.send(400, "text/plain", "Max users reached");
    return;
  }
  
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    LOG_E(LOG_WEB, "Erreur parsing JSON user");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  String username = doc["username"].as<String>();
  String password = doc["password"].as<String>();
  String role = doc["role"].as<String>();
  
  LOG_D(LOG_WEB, "Ajout utilisateur: %s (role=%s)", username.c_str(), role.c_str());
  
  // Vérifier si l'utilisateur existe déjà
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      LOG_W(LOG_WEB, "Utilisateur %s existe deja", username.c_str());
      server.send(400, "text/plain", "User exists");
      return;
    }
  }
  
  users[userCount].username = username;
  users[userCount].passwordHash = hashPassword(password);
  users[userCount].role = role;
  users[userCount].enabled = true;
  userCount++;
  
  LOG_I(LOG_WEB, "Utilisateur %s ajoute avec succes (total: %d/%d)",
        username.c_str(), userCount, MAX_USERS);
  
  saveUsers();
  server.send(200, "text/plain", "OK");
}

void handleApiDeleteUser() {
  LOG_WEB_REQUEST("DELETE", "/api/users");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    LOG_E(LOG_WEB, "Erreur parsing JSON");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  String username = doc["username"].as<String>();
  
  LOG_D(LOG_WEB, "Suppression utilisateur: %s", username.c_str());
  
  // Compter les admins
  int adminCount = 0;
  for (int i = 0; i < userCount; i++) {
    if (users[i].role == "admin") adminCount++;
  }
  
  bool found = false;
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      // Protection: ne pas supprimer le dernier admin
      if (users[i].role == "admin" && adminCount <= 1) {
        LOG_W(LOG_WEB, "Impossible de supprimer le dernier admin");
        server.send(400, "text/plain", "Cannot delete last admin");
        return;
      }
      
      // Décalage des utilisateurs
      for (int j = i; j < userCount - 1; j++) {
        users[j] = users[j + 1];
      }
      userCount--;
      found = true;
      break;
    }
  }
  
  if (found) {
    LOG_I(LOG_WEB, "Utilisateur %s supprime (total: %d/%d)",
          username.c_str(), userCount, MAX_USERS);
    saveUsers();
    server.send(200, "text/plain", "OK");
  } else {
    LOG_W(LOG_WEB, "Utilisateur %s non trouve", username.c_str());
    server.send(404, "text/plain", "User not found");
  }
}

void handleApiChangePassword() {
  LOG_WEB_REQUEST("POST", "/api/users/password");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    LOG_E(LOG_WEB, "Erreur parsing JSON");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  String username = doc["username"].as<String>();
  String oldPassword = doc["oldPassword"].as<String>();
  String newPassword = doc["newPassword"].as<String>();
  
  LOG_D(LOG_WEB, "Changement mot de passe pour: %s", username.c_str());
  
  // Vérifier l'ancien mot de passe
  if (!authenticateUser(username, oldPassword)) {
    LOG_W(LOG_WEB, "Ancien mot de passe incorrect pour %s", username.c_str());
    server.send(401, "text/plain", "Incorrect old password");
    return;
  }
  
  // Changer le mot de passe
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      users[i].passwordHash = hashPassword(newPassword);
      saveUsers();
      
      LOG_I(LOG_WEB, "Mot de passe change avec succes pour %s", username.c_str());
      
      server.send(200, "text/plain", "Password changed");
      return;
    }
  }
  
  LOG_W(LOG_WEB, "Utilisateur %s non trouve", username.c_str());
  server.send(404, "text/plain", "User not found");
}

// ============================================================================
// API CONFIGURATION GRAPHIQUE
// ============================================================================

void handleApiChartConfig() {
  if (server.method() == HTTP_GET) {
    LOG_WEB_REQUEST("GET", "/api/chart/config");
    
    // Lire config
    int interval = 300000; // Défaut 5 min
    if (LittleFS.exists("/chart_config.json")) {
      File f = LittleFS.open("/chart_config.json", "r");
      if (f) {
        StaticJsonDocument<128> doc;
        if (!deserializeJson(doc, f)) {
          interval = doc["interval"] | 300000;
        }
        f.close();
      }
    }
    
    LOG_V(LOG_WEB, "Config graphique: interval=%d ms", interval);
    
    StaticJsonDocument<128> response;
    response["interval"] = interval;
    String output;
    serializeJson(response, output);
    server.send(200, "application/json", output);
    
  } else if (server.method() == HTTP_POST) {
    LOG_WEB_REQUEST("POST", "/api/chart/config");
    
    // Sauvegarder config
    if (!server.hasArg("plain")) {
      LOG_E(LOG_WEB, "Corps de requete manquant");
      server.send(400, "text/plain", "Missing body");
      return;
    }
    
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, server.arg("plain"))) {
      LOG_E(LOG_WEB, "Erreur parsing JSON");
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }
    
    int interval = doc["interval"] | 300000;
    LOG_I(LOG_WEB, "Config graphique sauvegardee: interval=%d ms", interval);
    
    File f = LittleFS.open("/chart_config.json", "w");
    if (f) {
      serializeJson(doc, f);
      f.close();
      LOG_V(LOG_WEB, "Fichier /chart_config.json ecrit");
      server.send(200, "text/plain", "OK");
    } else {
      LOG_E(LOG_WEB, "Erreur ouverture /chart_config.json");
      server.send(500, "text/plain", "File error");
    }
  }
}

//=============================================================================
// API BACKUP/RESTORE
//=============================================================================

void handleBackupDownload() {
  LOG_WEB_REQUEST("GET", "/api/backup/download");
  
  LOG_I(LOG_WEB, "Generation du JSON de backup");
  String json = generateBackupJSON();
  
  LOG_I(LOG_WEB, "Backup genere: %d bytes", json.length());
  server.send(200, "application/json", json);
}

void handleBackupUpload() {
  LOG_WEB_REQUEST("POST", "/api/backup/upload");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  LOG_I(LOG_WEB, "Restauration du backup (%d bytes)", server.arg("plain").length());
  
  bool success = restoreFromJSON(server.arg("plain"));
  
  if (success) {
    LOG_I(LOG_WEB, "Restauration reussie");
    server.send(200, "text/plain", "Restauration réussie");
  } else {
    LOG_E(LOG_WEB, "Erreur lors de la restauration");
    server.send(400, "text/plain", "Erreur restauration");
  }
}

void handleBackupList() {
  LOG_WEB_REQUEST("GET", "/api/backup/list");
  
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.to<JsonArray>();
  
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  
  int backupCount = 0;
  
  while (file) {
    String name = file.name();
    if (name.startsWith("/backup") && name.endsWith(".json")) {
      JsonObject obj = arr.createNestedObject();
      obj["name"] = name;
      obj["size"] = file.size();
      backupCount++;
      LOG_V(LOG_WEB, "Backup trouve: %s (%d bytes)", name.c_str(), file.size());
    }
    file = root.openNextFile();
  }
  
  LOG_I(LOG_WEB, "Liste backups: %d fichiers trouves", backupCount);
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleBackupSave() {
  LOG_WEB_REQUEST("POST", "/api/backup/save");
  
  struct tm timeinfo;
  char filename[32];
  
  if (getLocalTime(&timeinfo)) {
    strftime(filename, sizeof(filename), "/backup_%Y%m%d.json", &timeinfo);
    LOG_D(LOG_WEB, "Nom fichier backup: %s", filename);
  } else {
    strcpy(filename, "/backup_manual.json");
    LOG_W(LOG_WEB, "Heure non disponible - Utilisation nom par defaut: %s", filename);
  }
  
  LOG_I(LOG_WEB, "Sauvegarde du backup dans: %s", filename);
  
  bool success = saveBackupToFile(filename);
  
  if (success) {
    LOG_I(LOG_WEB, "Backup sauvegarde avec succes: %s", filename);
    server.send(200, "text/plain", filename);
  } else {
    LOG_E(LOG_WEB, "Erreur sauvegarde backup");
    server.send(500, "text/plain", "Erreur sauvegarde");
  }
}

//=============================================================================
// API SCÉNARIOS
//=============================================================================

void handleGetScenarios() {
  LOG_WEB_REQUEST("GET", "/api/scenarios");
  
  LOG_D(LOG_WEB, "Generation JSON scenarios");
  String json = getScenariosJSON();
  
  LOG_I(LOG_WEB, "JSON scenarios genere: %d bytes", json.length());
  server.send(200, "application/json", json);
}

void handleApplyScenario() {
  LOG_WEB_REQUEST("POST", "/api/scenarios/apply");
  
  if (!server.hasArg("plain")) {
    LOG_E(LOG_WEB, "Corps de requete manquant");
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<128> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    LOG_E(LOG_WEB, "Erreur parsing JSON: %s", err.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  int scenarioId = doc["scenarioId"];
  
  LOG_D(LOG_WEB, "Application scenario ID: %d", scenarioId);
  
  if (scenarioId < 0 || scenarioId >= SCENARIO_COUNT) {
    LOG_E(LOG_WEB, "ID scenario invalide: %d (max=%d)", scenarioId, SCENARIO_COUNT);
    server.send(400, "text/plain", "Invalid scenario ID");
    return;
  }
  
  // Créer timer depuis scénario
  LOG_I(LOG_WEB, "Creation timer depuis scenario %d", scenarioId);
  FlexibleTimer newTimer = createTimerFromScenario((ScenarioType)scenarioId);
  
  // Ajouter aux timers
  if (flexTimerCount >= MAX_TIMERS) {
    LOG_E(LOG_WEB, "Limite timers atteinte: %d/%d", flexTimerCount, MAX_TIMERS);
    server.send(400, "text/plain", "Max timers reached");
    return;
  }
  
  flexTimers[flexTimerCount] = newTimer;
  flexTimerCount++;
  
  LOG_I(LOG_WEB, "Timer cree depuis scenario: '%s' (total: %d/%d)",
        newTimer.name.c_str(), flexTimerCount, MAX_TIMERS);
  
  saveFlexTimers();
  
  server.send(200, "text/plain", "Scénario appliqué");
}

// ============================================================================
// STATUT POMPE ET TEMPS RESTANT
// ============================================================================

void handleApiPumpStatus() {
  LOG_WEB_REQUEST("GET", "/api/pump/status");
  
  DynamicJsonDocument doc(512);
  
  // État de la pompe (relais 0)
  bool pumpOn = (digitalRead(relayPins[0]) == HIGH);
  doc["pumpOn"] = pumpOn;
  
  // Initialiser les valeurs par défaut
  doc["controlledByTimer"] = false;
  doc["remainingMinutes"] = 0;
  doc["timerName"] = "";
  doc["currentAction"] = 0;
  doc["totalActions"] = 0;
  
  // Si la pompe est active, chercher le timer qui la contrôle
  if (pumpOn) {
    for (int i = 0; i < flexTimerCount; i++) {
      FlexibleTimer* timer = &flexTimers[i];
      
      // Vérifier si le timer est en cours d'exécution
      if (timer->enabled && timer->context.state == TIMER_RUNNING) {
        
        int currentIdx = timer->context.currentActionIndex;
        if (currentIdx >= 0 && currentIdx < timer->actionCount) {
          Action* action = &timer->actions[currentIdx];
          bool isPumpControlled = false;
          int actionDuration = 0;
          
          // CAS 1: L'action courante est ACTION_RELAY qui active la pompe
          if (action->type == ACTION_RELAY && action->relay == 0 && action->state) {
            isPumpControlled = true;
            
            // Regarder l'action suivante pour la durée
            if (currentIdx + 1 < timer->actionCount) {
              Action* nextAction = &timer->actions[currentIdx + 1];
              if (nextAction->type == ACTION_WAIT_DURATION) {
                actionDuration = nextAction->delayMinutes;
                LOG_D(LOG_WEB, "Pompe ON - durée fixe suivante: %d min", actionDuration);
              } else if (nextAction->type == ACTION_AUTO_DURATION) {
                // Durée calculée depuis l'équation
                actionDuration = (int)(timer->context.calculatedDurationHours * 60);
                LOG_D(LOG_WEB, "Pompe ON - durée auto suivante: %.2f h = %d min", 
                      timer->context.calculatedDurationHours, actionDuration);
              }
            }
          }
          
          // CAS 2: L'action courante est ACTION_WAIT_DURATION pendant que la pompe tourne
          else if (action->type == ACTION_WAIT_DURATION) {
            // Chercher l'action qui a activé la pompe en remontant
            for (int j = currentIdx - 1; j >= 0; j--) {
              Action* prevAction = &timer->actions[j];
              if (prevAction->type == ACTION_RELAY && prevAction->relay == 0) {
                if (prevAction->state) {
                  isPumpControlled = true;
                  actionDuration = action->delayMinutes;
                  LOG_D(LOG_WEB, "WAIT_DURATION - pompe activée par action %d, durée: %d min", j, actionDuration);
                }
                break; // On a trouvé l'action RELAY, on s'arrête
              }
            }
          }
          
          // CAS 3: L'action courante est ACTION_AUTO_DURATION pendant que la pompe tourne
          else if (action->type == ACTION_AUTO_DURATION) {
            // Chercher l'action qui a activé la pompe en remontant
            for (int j = currentIdx - 1; j >= 0; j--) {
              Action* prevAction = &timer->actions[j];
              if (prevAction->type == ACTION_RELAY && prevAction->relay == 0) {
                if (prevAction->state) {
                  isPumpControlled = true;
                  // Durée calculée depuis l'équation
                  actionDuration = (int)(timer->context.calculatedDurationHours * 60);
                  LOG_D(LOG_WEB, "AUTO_DURATION - pompe activée par action %d, durée calculée: %.2f h = %d min", 
                        j, timer->context.calculatedDurationHours, actionDuration);
                }
                break; // On a trouvé l'action RELAY, on s'arrête
              }
            }
          }
          
          // Si la pompe est contrôlée par ce timer, calculer le temps restant
          if (isPumpControlled && actionDuration > 0) {
            doc["controlledByTimer"] = true;
            doc["timerName"] = timer->name;
            doc["currentAction"] = currentIdx + 1;
            doc["totalActions"] = timer->actionCount;
            
            // Calculer le temps restant
            unsigned long elapsedMillis = millis() - timer->context.actionStartMillis;
            unsigned long elapsedMinutes = elapsedMillis / 60000;
            int remainingMinutes = actionDuration - elapsedMinutes;
            if (remainingMinutes < 0) remainingMinutes = 0;
            
            doc["remainingMinutes"] = remainingMinutes;
            doc["totalMinutes"] = actionDuration;
            
            LOG_I(LOG_WEB, "Pompe controlee par timer '%s' (action %d/%d, type %d): %d min restantes sur %d min totales",
                  timer->name.c_str(), currentIdx + 1, timer->actionCount, (int)action->type, 
                  remainingMinutes, actionDuration);
            
            break; // Timer trouvé, on sort de la boucle
          } else {
            LOG_V(LOG_WEB, "Timer '%s' en cours mais ne contrôle pas la pompe (isPumpControlled=%d, actionDuration=%d)", 
                  timer->name.c_str(), isPumpControlled, actionDuration);
          }
        }
      }
    }
  }
  
  String output;
  serializeJson(doc, output);
  LOG_V(LOG_WEB, "Statut pompe envoye: %s", output.c_str());
  
  server.send(200, "application/json", output);
}

#endif // WEB_HANDLERS_IMPL_H