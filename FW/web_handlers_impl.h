/* 
 * POOL CONNECT - WEB HANDLERS IMPLEMENTATION
 * Implémentations complètes des handlers HTTP
 */

#ifndef WEB_HANDLERS_IMPL_H
#define WEB_HANDLERS_IMPL_H

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "config.h"
#include "users.h"
#include "storage.h"
#include "mqtt_manager.h"
#include "weather.h"
#include "sensors.h"
#include "backup_restore.h"
#include "scenarios.h"

// ============================================================================
// FICHIERS STATIQUES
// ============================================================================

void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "index.html not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleCss() {
  File file = LittleFS.open("/style.css", "r");
  if (!file) {
    server.send(404, "text/plain", "CSS not found");
    return;
  }
  server.streamFile(file, "text/css");
  file.close();
}

void handleJs() {
  File file = LittleFS.open("/app.js", "r");
  if (!file) {
    server.send(404, "text/plain", "JS not found");
    return;
  }
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleLogo() {
  File file = LittleFS.open("/logo.png", "r");
  if (!file) {
    server.send(404, "text/plain", "logo not found");
    return;
  }
  server.streamFile(file, "image/png");
  file.close();
}

void handleFavicon() {
  File file = LittleFS.open("/favicon.png", "r");
  if (!file) {
    server.send(404, "text/plain", "Favicon not found");
    return;
  }
  server.streamFile(file, "image/png");
  file.close();
}


// ============================================================================
// API BASIQUE - TEMPS ET CAPTEURS
// ============================================================================

void handleApiTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[64];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &timeinfo);
    server.send(200, "text/plain", buf);
  } else {
    server.send(200, "text/plain", "-- / -- / ---- --:--:--");
  }
}

void handleApiTemp() {
  if (isnan(waterTemp) || waterTemp < -50 || waterTemp > 100) {
    server.send(200, "text/plain", "ERREUR");
    return;
  }
  server.send(200, "text/plain", String(waterTemp, 2));
}

void handleApiRelays() {
  DynamicJsonDocument doc(256);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < NUM_RELAYS; i++) {
    arr.add(digitalRead(relayPins[i]) == HIGH);
  }
  String out;
  serializeJson(arr, out);
  server.send(200, "application/json", out);
}

void handleApiRelay() {
  if (!server.hasArg("ch") || !server.hasArg("state")) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }
  
  int ch = server.arg("ch").toInt();
  int state = server.arg("state").toInt();
  
  if (ch < 0 || ch >= NUM_RELAYS) {
    server.send(400, "text/plain", "Invalid relay");
    return;
  }
  
  // Protection électrolyseur - nécessite pompe active
  if (ch == 1 && state == 1) {
    bool pompeActive = (digitalRead(relayPins[0]) == HIGH);
    if (!pompeActive) {
      server.send(400, "text/plain", "Erreur: Pompe doit être active");
      return;
    }
  }
  
  digitalWrite(relayPins[ch], state ? HIGH : LOW);
  
  if (mqttClient.connected()) {
    mqttClient.publish((mqttTopic + "/relay/" + String(ch) + "/state").c_str(), 
                      state ? "1" : "0");
  }
  
  server.send(200, "text/plain", "OK");
}

void handleApiSensors() {
  DynamicJsonDocument doc(512);
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
    doc["waterTemp"] = waterTemp;
    doc["waterPressure"] = waterPressure;
    doc["waterLeak"] = waterLeak;
    doc["coverOpen"] = coverOpen;
    doc["extTemp"] = tempExterieure;
    xSemaphoreGive(dataMutex);
  }
  
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiBuzzerMute() {
  if (server.hasArg("state")) {
    buzzerMuted = (server.arg("state").toInt() == 1);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing state");
  }
}

// ============================================================================
// API CALIBRATION
// ============================================================================

void handleApiGetCalibration() {
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
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleApiSaveCalibration() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
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
  
  saveCalibrationConfig();
  server.send(200, "text/plain", "OK");
}

void handleApiResetCalibration() {
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
  
  saveCalibrationConfig();
  server.send(200, "text/plain", "Calibration reset");
}

// ============================================================================
// API TIMERS FLEXIBLES
// ============================================================================

void handleApiFlexTimers() {
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
  }
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleApiAddFlexTimer() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  if (flexTimerCount >= MAX_TIMERS) {
    server.send(400, "text/plain", "Max timers reached");
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  FlexibleTimer* t = &flexTimers[flexTimerCount];
  JsonObject obj = doc.as<JsonObject>();
  
  t->id = obj["id"] | (int)(millis() & 0x7FFFFFFF);
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
  saveFlexTimers();
  
  server.send(200, "text/plain", "OK");
}

void handleApiUpdateFlexTimer() {
  String uri = server.uri();
  int flexPos = uri.indexOf("flex/");
  if (flexPos == -1) {
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  int idStart = flexPos + 5;
  String idStr = uri.substring(idStart);
  int id = idStr.toInt();
  
  if (id == 0) {
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
    server.send(404, "text/plain", "Timer not found");
    return;
  }
  
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  JsonObject obj = doc.as<JsonObject>();
  timer->name = obj["name"].as<String>();
  timer->enabled = obj["enabled"] | true;
  
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
  
  if (timer->context.state == TIMER_RUNNING) {
    for (int a = 0; a < timer->actionCount; a++) {
      if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
        digitalWrite(relayPins[timer->actions[a].relay], LOW);
      }
    }
  }
  
  timer->context.state = TIMER_IDLE;
  timer->context.currentActionIndex = 0;
  timer->context.tempMeasured = false;
  timer->context.lastError = "";
  timer->lastTriggeredDay = -1;
  
  saveFlexTimers();
  server.send(200, "text/plain", "OK");
}

void handleApiDeleteFlexTimer() {
  String uri = server.uri();
  int flexPos = uri.indexOf("flex/");
  if (flexPos == -1) {
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  int idStart = flexPos + 5;
  String idStr = uri.substring(idStart);
  int id = idStr.toInt();
  
  if (id == 0) {
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
    server.send(404, "text/plain", "Timer not found");
    return;
  }
  
  for (int i = index; i < flexTimerCount - 1; i++) {
    flexTimers[i] = flexTimers[i + 1];
  }
  flexTimerCount--;
  
  saveFlexTimers();
  server.send(200, "text/plain", "OK");
}

void handleApiToggleFlexTimer() {
  String uri = server.uri();
  int flexPos = uri.indexOf("flex/");
  if (flexPos == -1) {
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  int idStart = flexPos + 5;
  int idEnd = uri.indexOf('/', idStart);
  
  if (idEnd == -1) {
    server.send(400, "text/plain", "Invalid URI");
    return;
  }
  
  String idStr = uri.substring(idStart, idEnd);
  int id = idStr.toInt();
  
  if (id == 0) {
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
    server.send(404, "text/plain", "Timer not found");
    return;
  }
  
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<128> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  bool newEnabled = doc["enabled"];
  
  if (!newEnabled && timer->context.state == TIMER_RUNNING) {
    for (int a = 0; a < timer->actionCount; a++) {
      if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
        digitalWrite(relayPins[timer->actions[a].relay], LOW);
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
  DynamicJsonDocument doc(512);
  doc["server"] = mqttServer;
  doc["port"] = mqttPort;
  doc["user"] = mqttUser;
  doc["password"] = "";
  doc["topic"] = mqttTopic;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiSaveMQTT() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  mqttServer = doc["server"].as<String>();
  mqttPort = doc["port"] | 1883;
  mqttUser = doc["user"].as<String>();
  mqttPassword = doc["password"].as<String>();
  mqttTopic = doc["topic"].as<String>();
  saveMQTTConfig();
  mqttClient.setServer(mqttServer.c_str(), mqttPort);
  mqttClient.setCallback(mqttCallback);
  server.send(200, "text/plain", "OK");
}

void handleApiMQTTStatus() {
  DynamicJsonDocument doc(128);
  doc["connected"] = mqttClient.connected();
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiMQTTRediscover() {
  if (mqttClient.connected()) {
    publishHomeAssistantDiscovery();
    publishSensorStates();
    server.send(200, "text/plain", "Discovery published");
  } else {
    server.send(503, "text/plain", "MQTT not connected");
  }
}

// ============================================================================
// API MÉTÉO
// ============================================================================

void handleApiWeatherConfig() {
  DynamicJsonDocument doc(512);
  doc["apiKey"] = weatherApiKey;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiWeatherSave() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  weatherApiKey = doc["apiKey"].as<String>();
  latitude = doc["latitude"].as<String>();
  longitude = doc["longitude"].as<String>();
  saveWeatherConfig();
  updateWeatherData();
  server.send(200, "text/plain", "OK");
}

// ============================================================================
// API SYSTÈME
// ============================================================================

void handleApiSystem() {
  DynamicJsonDocument doc(512);
  doc["version"] = FIRMWARE_VERSION;
  doc["ip"] = WiFi.localIP().toString();
  doc["uptime"] = millis() / 1000;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["chipId"] = (uint32_t)ESP.getEfuseMac();
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiGetSystemConfig() {
  StaticJsonDocument<256> doc;
  doc["pressureThreshold"] = sysConfig.pressureThreshold;
  doc["buzzerEnabled"] = sysConfig.buzzerEnabled;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiSaveSystemConfig() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  sysConfig.pressureThreshold = doc["pressureThreshold"] | 2.0;
  sysConfig.buzzerEnabled = doc["buzzerEnabled"] | true;
  pressureThreshold = sysConfig.pressureThreshold;
  buzzerMuted = !sysConfig.buzzerEnabled;
  saveSystemConfig();
  server.send(200, "text/plain", "OK");
}

void handleApiRestart() {
  server.send(200, "text/plain", "Restarting...");
  delay(1000);
  ESP.restart();
}

// ============================================================================
// API HISTORIQUE
// ============================================================================

void handleApiHistory() {
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = historyCount - 1; i >= 0; i--) {
    JsonObject o = arr.createNestedObject();
    o["date"] = history[i].date;
    o["startTime"] = history[i].startTime;
    o["duration"] = history[i].duration;
    o["avgTemp"] = history[i].avgTemp;
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// ============================================================================
// API AUTHENTIFICATION & UTILISATEURS
// ============================================================================

void handleApiAuth() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  String username = doc["username"].as<String>();
  String password = doc["password"].as<String>();
  if (authenticateUser(username, password)) {
    String role = getUserRole(username);
    StaticJsonDocument<128> response;
    response["success"] = true;
    response["role"] = role;
    response["username"] = username;
    String out;
    serializeJson(response, out);
    server.send(200, "application/json", out);
  } else {
    server.send(401, "text/plain", "Unauthorized");
  }
}

void handleApiGetUsers() {
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < userCount; i++) {
    JsonObject o = arr.createNestedObject();
    o["username"] = users[i].username;
    o["role"] = users[i].role;
    o["enabled"] = users[i].enabled;
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleApiAddUser() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  if (userCount >= MAX_USERS) {
    server.send(400, "text/plain", "Max users reached");
    return;
  }
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  String username = doc["username"].as<String>();
  String password = doc["password"].as<String>();
  String role = doc["role"].as<String>();
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      server.send(400, "text/plain", "User exists");
      return;
    }
  }
  users[userCount].username = username;
  users[userCount].passwordHash = hashPassword(password);
  users[userCount].role = role;
  users[userCount].enabled = true;
  userCount++;
  saveUsers();
  server.send(200, "text/plain", "OK");
}

void handleApiDeleteUser() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  String username = doc["username"].as<String>();
  int adminCount = 0;
  for (int i = 0; i < userCount; i++) {
    if (users[i].role == "admin") adminCount++;
  }
  bool found = false;
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      if (users[i].role == "admin" && adminCount <= 1) {
        server.send(400, "text/plain", "Cannot delete last admin");
        return;
      }
      for (int j = i; j < userCount - 1; j++) {
        users[j] = users[j + 1];
      }
      userCount--;
      found = true;
      break;
    }
  }
  if (found) {
    saveUsers();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(404, "text/plain", "User not found");
  }
}

void handleApiChangePassword() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  String username = doc["username"].as<String>();
  String oldPassword = doc["oldPassword"].as<String>();
  String newPassword = doc["newPassword"].as<String>();
  
  if (!authenticateUser(username, oldPassword)) {
    server.send(401, "text/plain", "Incorrect old password");
    return;
  }
  
  for (int i = 0; i < userCount; i++) {
    if (users[i].username == username) {
      users[i].passwordHash = hashPassword(newPassword);
      saveUsers();
      server.send(200, "text/plain", "Password changed");
      return;
    }
  }
  
  server.send(404, "text/plain", "User not found");
}

// ============================================================================
// API CONFIGURATION GRAPHIQUE
// ============================================================================

void handleApiChartConfig() {
  if (server.method() == HTTP_GET) {
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
    
    StaticJsonDocument<128> response;
    response["interval"] = interval;
    String output;
    serializeJson(response, output);
    server.send(200, "application/json", output);
    
  } else if (server.method() == HTTP_POST) {
    // Sauvegarder config
    if (!server.hasArg("plain")) {
      server.send(400, "text/plain", "Missing body");
      return;
    }
    
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, server.arg("plain"))) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }
    
    File f = LittleFS.open("/chart_config.json", "w");
    if (f) {
      serializeJson(doc, f);
      f.close();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "File error");
    }
  }
}

//=============================================================================
// API BACKUP/RESTORE
//=============================================================================

void handleBackupDownload() {
  String json = generateBackupJSON();
  server.send(200, "application/json", json);
}

void handleBackupUpload() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  bool success = restoreFromJSON(server.arg("plain"));
  
  if (success) {
    server.send(200, "text/plain", "Restauration réussie");
  } else {
    server.send(400, "text/plain", "Erreur restauration");
  }
}

void handleBackupList() {
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.to<JsonArray>();
  
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    String name = file.name();
    if (name.startsWith("/backup") && name.endsWith(".json")) {
      JsonObject obj = arr.createNestedObject();
      obj["name"] = name;
      obj["size"] = file.size();
    }
    file = root.openNextFile();
  }
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleBackupSave() {
  struct tm timeinfo;
  char filename[32];
  
  if (getLocalTime(&timeinfo)) {
    strftime(filename, sizeof(filename), "/backup_%Y%m%d.json", &timeinfo);
  } else {
    strcpy(filename, "/backup_manual.json");
  }
  
  bool success = saveBackupToFile(filename);
  
  if (success) {
    server.send(200, "text/plain", filename);
  } else {
    server.send(500, "text/plain", "Erreur sauvegarde");
  }
}

//=============================================================================
// API SCÉNARIOS
//=============================================================================

void handleGetScenarios() {
  String json = getScenariosJSON();
  server.send(200, "application/json", json);
}

void handleApplyScenario() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing body");
    return;
  }
  
  StaticJsonDocument<128> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  
  if (err) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  int scenarioId = doc["scenarioId"];
  
  if (scenarioId < 0 || scenarioId >= SCENARIO_COUNT) {
    server.send(400, "text/plain", "Invalid scenario ID");
    return;
  }
  
  // Créer timer depuis scénario
  FlexibleTimer newTimer = createTimerFromScenario((ScenarioType)scenarioId);
  
  // Ajouter aux timers
  if (flexTimerCount >= MAX_TIMERS) {
    server.send(400, "text/plain", "Max timers reached");
    return;
  }
  
  flexTimers[flexTimerCount] = newTimer;
  flexTimerCount++;
  saveFlexTimers();
  
  server.send(200, "text/plain", "Scénario appliqué");
}

#endif // WEB_HANDLERS_IMPL_H
