/* 
 * POOL CONNECT - MQTT MANAGER
 * Gestion MQTT et Home Assistant
 * mqtt_manager.h   V0.2
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "globals.h"
#include "config.h"
#include "logging.h"

// ============================================================================
// MQTT CONFIG
// ============================================================================

void saveMQTTConfig() {
  LOG_D(LOG_MQTT, "Sauvegarde de la configuration MQTT...");
  
  File f = LittleFS.open("/mqtt.json", FILE_WRITE);
  if (!f) {
    LOG_E(LOG_MQTT, "Erreur ouverture fichier /mqtt.json en ecriture");
    LOG_STORAGE_OP("WRITE", "/mqtt.json", false);
    return;
  }
  
  StaticJsonDocument<512> doc;
  doc["server"] = mqttServer;
  doc["port"] = mqttPort;
  doc["user"] = mqttUser;
  doc["password"] = mqttPassword;
  doc["topic"] = mqttTopic;
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_MQTT, "Configuration MQTT sauvegardee (%d bytes)", bytesWritten);
  LOG_V(LOG_MQTT, "Serveur: %s:%d, Topic: %s", mqttServer.c_str(), mqttPort, mqttTopic.c_str());
  LOG_STORAGE_OP("WRITE", "/mqtt.json", true);
}

void loadMQTTConfig() {
  LOG_D(LOG_MQTT, "Chargement de la configuration MQTT...");
  
  if (!LittleFS.exists("/mqtt.json")) {
    LOG_W(LOG_MQTT, "Fichier /mqtt.json non trouve - Configuration par defaut utilisee");
    return;
  }
  
  File f = LittleFS.open("/mqtt.json", FILE_READ);
  if (!f) {
    LOG_E(LOG_MQTT, "Erreur ouverture fichier /mqtt.json en lecture");
    LOG_STORAGE_OP("READ", "/mqtt.json", false);
    return;
  }
  
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    LOG_E(LOG_MQTT, "Erreur parsing JSON: %s", err.c_str());
    LOG_STORAGE_OP("READ", "/mqtt.json", false);
    return;
  }
  
  mqttServer = doc["server"].as<String>();
  mqttPort = doc["port"] | 1883;
  mqttUser = doc["user"].as<String>();
  mqttPassword = doc["password"].as<String>();
  mqttTopic = doc["topic"].as<String>();
  
  LOG_I(LOG_MQTT, "Configuration MQTT chargee avec succes");
  LOG_I(LOG_MQTT, "Serveur: %s:%d", mqttServer.c_str(), mqttPort);
  LOG_I(LOG_MQTT, "Topic de base: %s", mqttTopic.c_str());
  LOG_V(LOG_MQTT, "Username: %s", mqttUser.length() > 0 ? mqttUser.c_str() : "Non configure");
  LOG_STORAGE_OP("READ", "/mqtt.json", true);
}

void initMQTTClient() {
  LOG_D(LOG_MQTT, "Initialisation du client MQTT...");
  mqttClient.setBufferSize(1024);
  LOG_V(LOG_MQTT, "Buffer MQTT configure a 1024 bytes");
}

// ============================================================================
// MQTT HOME ASSISTANT DISCOVERY - VERSION CORRIGÉE
// ============================================================================

void publishHASwitch(String id, String name, int relayIndex, String deviceConfig) {
  String topic = "homeassistant/switch/" + id + "/config";
  
  LOG_D(LOG_MQTT, "Publication HA Switch Discovery: %s", name.c_str());
  LOG_V(LOG_MQTT, "Topic: %s", topic.c_str());
  
  DynamicJsonDocument doc(2048);
  doc["name"] = name;
  doc["unique_id"] = "pool_" + id;
  doc["command_topic"] = mqttTopic + "/relay/" + String(relayIndex) + "/set";
  doc["state_topic"] = mqttTopic + "/relay/" + String(relayIndex) + "/state";
  doc["payload_on"] = "1";
  doc["payload_off"] = "0";
  doc["optimistic"] = false;
  doc["retain"] = true;
  
  DynamicJsonDocument deviceDoc(512);
  deserializeJson(deviceDoc, deviceConfig);
  doc["device"] = deviceDoc.as<JsonObject>();
  
  String payload;
  serializeJson(doc, payload);
  
  LOG_V(LOG_MQTT, "Payload size: %d bytes", payload.length());
  
  bool published = mqttClient.publish(topic.c_str(), payload.c_str(), true);
  
  if (published) {
    LOG_I(LOG_MQTT, "HA Switch '%s' publie avec succes", name.c_str());
    LOG_MQTT_PUB(topic.c_str(), "[HA Discovery]");
  } else {
    LOG_E(LOG_MQTT, "Echec publication HA Switch '%s'", name.c_str());
  }
  
  delay(100);
}

void publishHASensor(String id, String name, String deviceClass, String unit, String icon, String deviceConfig) {
  String topic = "homeassistant/sensor/" + id + "/config";
  
  LOG_D(LOG_MQTT, "Publication HA Sensor Discovery: %s", name.c_str());
  LOG_V(LOG_MQTT, "Topic: %s", topic.c_str());
  
  DynamicJsonDocument doc(2048);
  doc["name"] = name;
  doc["unique_id"] = "pool_" + id;
  doc["state_topic"] = mqttTopic + "/sensor/" + id;
  doc["device_class"] = deviceClass;
  doc["unit_of_measurement"] = unit;
  doc["icon"] = icon;
  
  DynamicJsonDocument deviceDoc(512);
  deserializeJson(deviceDoc, deviceConfig);
  doc["device"] = deviceDoc.as<JsonObject>();
  
  String payload;
  serializeJson(doc, payload);
  
  LOG_V(LOG_MQTT, "Device class: %s, Unit: %s", deviceClass.c_str(), unit.c_str());
  
  bool published = mqttClient.publish(topic.c_str(), payload.c_str(), true);
  
  if (published) {
    LOG_I(LOG_MQTT, "HA Sensor '%s' publie avec succes", name.c_str());
    LOG_MQTT_PUB(topic.c_str(), "[HA Discovery]");
  } else {
    LOG_E(LOG_MQTT, "Echec publication HA Sensor '%s'", name.c_str());
  }
  
  delay(100);
}

void publishHABinarySensor(String id, String name, String deviceClass, String icon, String deviceConfig) {
  String topic = "homeassistant/binary_sensor/" + id + "/config";
  
  LOG_D(LOG_MQTT, "Publication HA Binary Sensor Discovery: %s", name.c_str());
  LOG_V(LOG_MQTT, "Topic: %s", topic.c_str());
  
  DynamicJsonDocument doc(2048);
  doc["name"] = name;
  doc["unique_id"] = "pool_" + id;
  doc["state_topic"] = mqttTopic + "/sensor/" + id;
  doc["device_class"] = deviceClass;
  doc["payload_on"] = "ON";
  doc["payload_off"] = "OFF";
  doc["icon"] = icon;
  
  DynamicJsonDocument deviceDoc(512);
  deserializeJson(deviceDoc, deviceConfig);
  doc["device"] = deviceDoc.as<JsonObject>();
  
  String payload;
  serializeJson(doc, payload);
  
  LOG_V(LOG_MQTT, "Device class: %s", deviceClass.c_str());
  
  bool published = mqttClient.publish(topic.c_str(), payload.c_str(), true);
  
  if (published) {
    LOG_I(LOG_MQTT, "HA Binary Sensor '%s' publie avec succes", name.c_str());
    LOG_MQTT_PUB(topic.c_str(), "[HA Discovery]");
  } else {
    LOG_E(LOG_MQTT, "Echec publication HA Binary Sensor '%s'", name.c_str());
  }
  
  delay(100);
}

void publishHomeAssistantDiscovery() {
  if (!mqttClient.connected()) {
    LOG_E(LOG_MQTT, "MQTT non connecte - Home Assistant Discovery annulee");
    return;
  }
  
  LOG_SEPARATOR();
  LOG_I(LOG_MQTT, "Demarrage Home Assistant Discovery...");
  
  String deviceId = "pool_connect_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  LOG_I(LOG_MQTT, "Device ID: %s", deviceId.c_str());
  
  DynamicJsonDocument deviceDoc(512);
  JsonArray identifiers = deviceDoc.createNestedArray("identifiers");
  identifiers.add(deviceId);
  deviceDoc["name"] = "Pool Connect Pro";
  deviceDoc["model"] = "ESP32-S3 Controller";
  deviceDoc["manufacturer"] = "Custom";
  deviceDoc["sw_version"] = String(FIRMWARE_VERSION);
  
  String deviceConfig;
  serializeJson(deviceDoc, deviceConfig);
  
  LOG_D(LOG_MQTT, "Publication des switches (5 relais)...");
  publishHASwitch("pompe", "Pompe", 0, deviceConfig);
  publishHASwitch("electrolyseur", "Électrolyseur", 1, deviceConfig);
  publishHASwitch("lampe", "Lampe", 2, deviceConfig);
  publishHASwitch("electrovalve", "Électrovalve", 3, deviceConfig);
  publishHASwitch("pac", "Pompe à Chaleur", 4, deviceConfig);
  
  LOG_D(LOG_MQTT, "Publication des sensors (3 capteurs)...");
  publishHASensor("water_temp", "Température Eau", "temperature", "°C", "mdi:thermometer-water", deviceConfig);
  publishHASensor("water_pressure", "Pression Eau", "pressure", "bar", "mdi:gauge", deviceConfig);
  publishHASensor("ext_temp", "Température Extérieure", "temperature", "°C", "mdi:thermometer", deviceConfig);
  
  LOG_D(LOG_MQTT, "Publication des binary sensors (2 capteurs)...");
  publishHABinarySensor("water_leak", "Fuite d'Eau", "moisture", "mdi:water-alert", deviceConfig);
  publishHABinarySensor("cover", "Volet Piscine", "opening", "mdi:window-shutter", deviceConfig);
  
  LOG_I(LOG_MQTT, "Home Assistant Discovery terminee avec succes");
  LOG_I(LOG_MQTT, "Total: 5 switches, 3 sensors, 2 binary sensors");
  LOG_SEPARATOR();
}

// ============================================================================
// MQTT CALLBACK
// ============================================================================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  
  LOG_MQTT_SUB(topic, msg.c_str());
  
  // Commande relay globale (legacy)
  String setTopic = mqttTopic + String("/relay/set");
  if (String(topic) == setTopic) {
    LOG_D(LOG_MQTT, "Reception commande relay globale");
    DynamicJsonDocument doc(256);
    if (!deserializeJson(doc, msg)) {
      int relay = doc["relay"];
      int state = doc["state"];
      
      if (relay >= 0 && relay < NUM_RELAYS) {
        digitalWrite(relayPins[relay], state ? HIGH : LOW);
        LOG_I(LOG_MQTT, "Relais %d commande via MQTT: %s", relay, state ? "ON" : "OFF");
        
        String stateTopic = mqttTopic + "/relay/" + String(relay) + "/state";
        mqttClient.publish(stateTopic.c_str(), state ? "1" : "0", true);
        LOG_MQTT_PUB(stateTopic.c_str(), state ? "1" : "0");
      } else {
        LOG_E(LOG_MQTT, "Index relay invalide: %d", relay);
      }
    } else {
      LOG_E(LOG_MQTT, "Erreur parsing JSON de la commande relay");
    }
  }
  
  // Commandes relay individuelles (Home Assistant)
  for (int i = 0; i < NUM_RELAYS; i++) {
    String relaySetTopic = mqttTopic + "/relay/" + String(i) + "/set";
    if (String(topic) == relaySetTopic) {
      bool state = (msg == "1");
      digitalWrite(relayPins[i], state ? HIGH : LOW);
      LOG_I(LOG_MQTT, "Relais %d commande via MQTT: %s", i, state ? "ON" : "OFF");
      
      String stateTopic = mqttTopic + "/relay/" + String(i) + "/state";
      mqttClient.publish(stateTopic.c_str(), state ? "1" : "0", true);
      LOG_MQTT_PUB(stateTopic.c_str(), state ? "1" : "0");
      return;
    }
  }
  
  LOG_W(LOG_MQTT, "Topic non gere: %s", topic);
}

// ============================================================================
// MQTT PUBLISH - VERSION AVEC LOGS ET RETAIN
// ============================================================================

void publishSensorStates() {
  if (!mqttClient.connected()) {
    LOG_W(LOG_MQTT, "MQTT non connecte - Publication des etats annulee");
    return;
  }
  
  LOG_D(LOG_MQTT, "Publication des etats des capteurs et relais...");
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
    String topic;
    String payload;
    
    // Température eau
    topic = mqttTopic + "/sensor/water_temp";
    payload = String(waterTemp, 2);
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
    LOG_MQTT_PUB(topic.c_str(), payload.c_str());
    LOG_V(LOG_MQTT, "Temperature eau: %.2f C", waterTemp);
    
    // Pression
    topic = mqttTopic + "/sensor/water_pressure";
    payload = String(waterPressure, 2);
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
    LOG_MQTT_PUB(topic.c_str(), payload.c_str());
    LOG_V(LOG_MQTT, "Pression: %.2f BAR", waterPressure);
    
    // Température extérieure
    topic = mqttTopic + "/sensor/ext_temp";
    payload = String(tempExterieure, 2);
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
    LOG_MQTT_PUB(topic.c_str(), payload.c_str());
    LOG_V(LOG_MQTT, "Temperature exterieure: %.2f C", tempExterieure);
    
    // Fuite
    topic = mqttTopic + "/sensor/water_leak";
    payload = waterLeak ? "ON" : "OFF";
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
    LOG_MQTT_PUB(topic.c_str(), payload.c_str());
    if (waterLeak) {
      LOG_W(LOG_MQTT, "FUITE DETECTEE!");
    }
    
    // Volet
    topic = mqttTopic + "/sensor/cover";
    payload = coverOpen ? "ON" : "OFF";
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
    LOG_MQTT_PUB(topic.c_str(), payload.c_str());
    LOG_V(LOG_MQTT, "Volet: %s", coverOpen ? "OUVERT" : "FERME");
    
    xSemaphoreGive(dataMutex);
  } else {
    LOG_E(LOG_MQTT, "Impossible d'obtenir le mutex pour la lecture des capteurs");
  }
  
  // États des relais
  for (int i = 0; i < NUM_RELAYS; i++) {
    bool state = digitalRead(relayPins[i]) == HIGH;
    String topic = mqttTopic + "/relay/" + String(i) + "/state";
    String payload = state ? "1" : "0";
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
    LOG_MQTT_PUB(topic.c_str(), payload.c_str());
    LOG_V(LOG_MQTT, "Relais %d: %s", i, state ? "ON" : "OFF");
  }
  
  LOG_I(LOG_MQTT, "Publication des etats terminee (%d capteurs + %d relais)", 5, NUM_RELAYS);
}

// ============================================================================
// MQTT RECONNECT
// ============================================================================

void mqttReconnect() {
  if (millis() - lastMqttAttempt < MQTT_RECONNECT_INTERVAL) return;
  if (mqttServer == "" || mqttClient.connected()) return;
  
  lastMqttAttempt = millis();
  
  LOG_I(LOG_MQTT, "Tentative de connexion MQTT...");
  LOG_D(LOG_MQTT, "Serveur: %s:%d", mqttServer.c_str(), mqttPort);
  LOG_V(LOG_MQTT, "Client ID: ESP32PoolConnect");
  
  if (mqttClient.connect("ESP32PoolConnect", mqttUser.c_str(), mqttPassword.c_str())) {
    LOG_I(LOG_MQTT, "Connexion MQTT reussie!");
    
    mqttClient.setCallback(mqttCallback);
    LOG_V(LOG_MQTT, "Callback MQTT enregistre");
    
    // Souscriptions
    LOG_D(LOG_MQTT, "Souscription aux topics de commande...");
    
    String topic = mqttTopic + "/relay/set";
    mqttClient.subscribe(topic.c_str());
    LOG_V(LOG_MQTT, "Souscrit: %s", topic.c_str());
    
    for (int i = 0; i < NUM_RELAYS; i++) {
      topic = mqttTopic + "/relay/" + String(i) + "/set";
      mqttClient.subscribe(topic.c_str());
      LOG_V(LOG_MQTT, "Souscrit: %s", topic.c_str());
    }
    
    LOG_I(LOG_MQTT, "Souscriptions terminees (%d topics)", NUM_RELAYS + 1);
    
    // Publication immédiate après connexion
    LOG_D(LOG_MQTT, "Publication initiale apres connexion...");
    publishHomeAssistantDiscovery();
    delay(500);  // Laisser le temps à HA de traiter la découverte
    publishSensorStates();
    
  } else {
    int errorCode = mqttClient.state();
    LOG_E(LOG_MQTT, "Echec de connexion MQTT (code erreur: %d)", errorCode);
    
    // Détails des codes d'erreur
    switch(errorCode) {
      case -4:
        LOG_E(LOG_MQTT, "MQTT_CONNECTION_TIMEOUT - Timeout de connexion");
        break;
      case -3:
        LOG_E(LOG_MQTT, "MQTT_CONNECTION_LOST - Connexion perdue");
        break;
      case -2:
        LOG_E(LOG_MQTT, "MQTT_CONNECT_FAILED - Echec de connexion");
        break;
      case -1:
        LOG_E(LOG_MQTT, "MQTT_DISCONNECTED - Deconnecte");
        break;
      case 1:
        LOG_E(LOG_MQTT, "MQTT_CONNECT_BAD_PROTOCOL - Mauvais protocole");
        break;
      case 2:
        LOG_E(LOG_MQTT, "MQTT_CONNECT_BAD_CLIENT_ID - Mauvais client ID");
        break;
      case 3:
        LOG_E(LOG_MQTT, "MQTT_CONNECT_UNAVAILABLE - Serveur indisponible");
        break;
      case 4:
        LOG_E(LOG_MQTT, "MQTT_CONNECT_BAD_CREDENTIALS - Mauvais identifiants");
        break;
      case 5:
        LOG_E(LOG_MQTT, "MQTT_CONNECT_UNAUTHORIZED - Non autorise");
        break;
      default:
        LOG_E(LOG_MQTT, "Code erreur inconnu");
        break;
    }
    
    LOG_W(LOG_MQTT, "Prochaine tentative dans %lu ms", MQTT_RECONNECT_INTERVAL);
  }
}

#endif // MQTT_MANAGER_H