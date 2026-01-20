/* 
 * POOL CONNECT - MQTT MANAGER
 * Gestion MQTT et Home Assistant
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "globals.h"
#include "config.h"

// ============================================================================
// MQTT CONFIG
// ============================================================================

void saveMQTTConfig() {
  File f = LittleFS.open("/mqtt.json", FILE_WRITE);
  if (!f) return;
  StaticJsonDocument<512> doc;
  doc["server"] = mqttServer;
  doc["port"] = mqttPort;
  doc["user"] = mqttUser;
  doc["password"] = mqttPassword;
  doc["topic"] = mqttTopic;
  serializeJson(doc, f);
  f.close();
}

void loadMQTTConfig() {
  if (!LittleFS.exists("/mqtt.json")) return;
  File f = LittleFS.open("/mqtt.json", FILE_READ);
  if (!f) return;
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (!err) {
    mqttServer = doc["server"].as<String>();
    mqttPort = doc["port"] | 1883;
    mqttUser = doc["user"].as<String>();
    mqttPassword = doc["password"].as<String>();
    mqttTopic = doc["topic"].as<String>();
  }
}

void initMQTTClient() {
  mqttClient.setBufferSize(1024);  
}

// ============================================================================
// MQTT HOME ASSISTANT DISCOVERY - VERSION CORRIGÉE
// ============================================================================

void publishHASwitch(String id, String name, int relayIndex, String deviceConfig) {
  String topic = "homeassistant/switch/" + id + "/config";
  DynamicJsonDocument doc(2048);
  doc["name"] = name;
  doc["unique_id"] = "pool_" + id;
  doc["command_topic"] = mqttTopic + "/relay/" + String(relayIndex) + "/set";
  doc["state_topic"] = mqttTopic + "/relay/" + String(relayIndex) + "/state";
  doc["payload_on"] = "1";
  doc["payload_off"] = "0";
  doc["optimistic"] = false;
  doc["retain"] = true;
  
  // ✅ Parser le device JSON et l'ajouter correctement
  DynamicJsonDocument deviceDoc(512);
  deserializeJson(deviceDoc, deviceConfig);
  doc["device"] = deviceDoc.as<JsonObject>();
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.printf("📡 MQTT Discovery Switch: %s\n", topic.c_str());
  Serial.printf("   Payload: %s\n", payload.c_str());
  
  bool published = mqttClient.publish(topic.c_str(), payload.c_str(), true);
  Serial.printf("   Résultat: %s\n", published ? "✅ OK" : "❌ ÉCHEC");
  
  delay(100);
}

void publishHASensor(String id, String name, String deviceClass, String unit, String icon, String deviceConfig) {
  String topic = "homeassistant/sensor/" + id + "/config";
  DynamicJsonDocument doc(2048);
  doc["name"] = name;
  doc["unique_id"] = "pool_" + id;
  doc["state_topic"] = mqttTopic + "/sensor/" + id;
  doc["device_class"] = deviceClass;
  doc["unit_of_measurement"] = unit;
  doc["icon"] = icon;
  
  // ✅ Parser le device JSON et l'ajouter correctement
  DynamicJsonDocument deviceDoc(512);
  deserializeJson(deviceDoc, deviceConfig);
  doc["device"] = deviceDoc.as<JsonObject>();
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.printf("📡 MQTT Discovery Sensor: %s\n", topic.c_str());
  
  bool published = mqttClient.publish(topic.c_str(), payload.c_str(), true);
  Serial.printf("   Résultat: %s\n", published ? "✅ OK" : "❌ ÉCHEC");
  
  delay(100);
}

void publishHABinarySensor(String id, String name, String deviceClass, String icon, String deviceConfig) {
  String topic = "homeassistant/binary_sensor/" + id + "/config";
  DynamicJsonDocument doc(2048);
  doc["name"] = name;
  doc["unique_id"] = "pool_" + id;
  doc["state_topic"] = mqttTopic + "/sensor/" + id;
  doc["device_class"] = deviceClass;
  doc["payload_on"] = "ON";
  doc["payload_off"] = "OFF";
  doc["icon"] = icon;
  
  // ✅ Parser le device JSON et l'ajouter correctement
  DynamicJsonDocument deviceDoc(512);
  deserializeJson(deviceDoc, deviceConfig);
  doc["device"] = deviceDoc.as<JsonObject>();
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.printf("📡 MQTT Discovery Binary Sensor: %s\n", topic.c_str());
  
  bool published = mqttClient.publish(topic.c_str(), payload.c_str(), true);
  Serial.printf("   Résultat: %s\n", published ? "✅ OK" : "❌ ÉCHEC");
  
  delay(100);
}

void publishHomeAssistantDiscovery() {
  if (!mqttClient.connected()) {
    Serial.println("❌ MQTT non connecté - découverte annulée");
    return;
  }
  
  Serial.println("🏠 Publication Home Assistant Discovery...");
  
  String deviceId = "pool_connect_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  
  // ✅ Créer un JSON valide pour le device
  DynamicJsonDocument deviceDoc(512);
  JsonArray identifiers = deviceDoc.createNestedArray("identifiers");
  identifiers.add(deviceId);
  deviceDoc["name"] = "Pool Connect Pro";
  deviceDoc["model"] = "ESP32-S3 Controller";
  deviceDoc["manufacturer"] = "Custom";
  deviceDoc["sw_version"] = String(FIRMWARE_VERSION);
  
  String deviceConfig;
  serializeJson(deviceDoc, deviceConfig);
  
  Serial.printf("Device ID: %s\n", deviceId.c_str());

  publishHASwitch("pompe", "Pompe", 0, deviceConfig);
  publishHASwitch("electrolyseur", "Électrolyseur", 1, deviceConfig);
  publishHASwitch("lampe", "Lampe", 2, deviceConfig);
  publishHASwitch("electrovalve", "Électrovalve", 3, deviceConfig);
  publishHASwitch("pac", "Pompe à Chaleur", 4, deviceConfig);
  
  publishHASensor("water_temp", "Température Eau", "temperature", "°C", "mdi:thermometer-water", deviceConfig);
  publishHASensor("water_pressure", "Pression Eau", "pressure", "bar", "mdi:gauge", deviceConfig);
  publishHASensor("ext_temp", "Température Extérieure", "temperature", "°C", "mdi:thermometer", deviceConfig);
  
  publishHABinarySensor("water_leak", "Fuite d'Eau", "moisture", "mdi:water-alert", deviceConfig);
  publishHABinarySensor("cover", "Volet Piscine", "opening", "mdi:window-shutter", deviceConfig);
  
  Serial.println("✅ Home Assistant Discovery terminée");
}

// ============================================================================
// MQTT CALLBACK
// ============================================================================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("MQTT [%s]: %s\n", topic, msg.c_str());

  String setTopic = mqttTopic + String("/relay/set");
  if (String(topic) == setTopic) {
    DynamicJsonDocument doc(256);
    if (!deserializeJson(doc, msg)) {
      int relay = doc["relay"];
      int state = doc["state"];
      if (relay >= 0 && relay < NUM_RELAYS) {
        digitalWrite(relayPins[relay], state ? HIGH : LOW);
        mqttClient.publish((mqttTopic + "/relay/" + String(relay) + "/state").c_str(), state ? "1" : "0", true);
      }
    }
  }
  
  for (int i = 0; i < NUM_RELAYS; i++) {
    String relaySetTopic = mqttTopic + "/relay/" + String(i) + "/set";
    if (String(topic) == relaySetTopic) {
      bool state = (msg == "1");
      digitalWrite(relayPins[i], state ? HIGH : LOW);
      mqttClient.publish((mqttTopic + "/relay/" + String(i) + "/state").c_str(), state ? "1" : "0", true);
    }
  }
}

// ============================================================================
// MQTT PUBLISH - VERSION AVEC LOGS ET RETAIN
// ============================================================================

void publishSensorStates() {
  if (!mqttClient.connected()) {
    Serial.println("⚠️ MQTT non connecté - publication annulée");
    return;
  }
  
  Serial.println("📡 Publication états MQTT...");
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
    Serial.printf("  🌡️ Température: %.2f°C\n", waterTemp);
    mqttClient.publish((mqttTopic + "/sensor/water_temp").c_str(), 
                       String(waterTemp, 2).c_str(), true);
    
    Serial.printf("  💪 Pression: %.2f bar\n", waterPressure);
    mqttClient.publish((mqttTopic + "/sensor/water_pressure").c_str(), 
                       String(waterPressure, 2).c_str(), true);
    
    Serial.printf("  ☀️ Temp ext: %.2f°C\n", tempExterieure);
    mqttClient.publish((mqttTopic + "/sensor/ext_temp").c_str(), 
                       String(tempExterieure, 2).c_str(), true);
    
    Serial.printf("  💧 Fuite: %s\n", waterLeak ? "OUI" : "NON");
    mqttClient.publish((mqttTopic + "/sensor/water_leak").c_str(), 
                       waterLeak ? "ON" : "OFF", true);
    
    Serial.printf("  🎚️ Volet: %s\n", coverOpen ? "OUVERT" : "FERMÉ");
    mqttClient.publish((mqttTopic + "/sensor/cover").c_str(), 
                       coverOpen ? "ON" : "OFF", true);
    
    xSemaphoreGive(dataMutex);
  }
  
  for (int i = 0; i < NUM_RELAYS; i++) {
    bool state = digitalRead(relayPins[i]) == HIGH;
    Serial.printf("  ⚡ Relais %d: %s\n", i, state ? "ON" : "OFF");
    mqttClient.publish((mqttTopic + "/relay/" + String(i) + "/state").c_str(), 
                       state ? "1" : "0", true);
  }
  
  Serial.println("✅ Publication MQTT terminée");
}

// ============================================================================
// MQTT RECONNECT
// ============================================================================

void mqttReconnect() {
  if (millis() - lastMqttAttempt < MQTT_RECONNECT_INTERVAL) return;
  if (mqttServer == "" || mqttClient.connected()) return;
  
  lastMqttAttempt = millis();
  Serial.print("MQTT...");
  
  if (mqttClient.connect("ESP32PoolConnect", mqttUser.c_str(), mqttPassword.c_str())) {
    Serial.println("OK");
    mqttClient.setCallback(mqttCallback);
    mqttClient.subscribe((mqttTopic + "/relay/set").c_str());
    
    for (int i = 0; i < NUM_RELAYS; i++) {
      String topic = mqttTopic + "/relay/" + String(i) + "/set";
      mqttClient.subscribe(topic.c_str());
    }
    
    // ✅ Publier immédiatement après connexion
    publishHomeAssistantDiscovery();
    delay(500);  // Laisser le temps à HA de traiter la découverte
    publishSensorStates();
    
  } else {
    Serial.printf("FAIL(rc=%d)\n", mqttClient.state());
  }
}

#endif // MQTT_MANAGER_H