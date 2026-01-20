/* 
 * POOL CONNECT - SYSTEM INITIALIZATION
 * Fonctions d'initialisation du système
 */

#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Wire.h>
#include "globals.h"
#include "config.h"
#include "led_buzzer.h"
#include "storage.h"
#include "users.h"
#include "sensors.h"
#include "mqtt_manager.h"
#include "weather.h"
#include "core_tasks.h"

// ============================================================================
// INITIALISATION DES RELAIS
// ============================================================================

void initRelays() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }
  pinMode(SENSOR_VOLET, INPUT);
  pinMode(SENSOR_FUITE, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("✓ Relais initialisés");
}

// ============================================================================
// INITIALISATION LED
// ============================================================================

void initLED() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  setLEDStatus(LED_BOOTING);
  Serial.println("✓ LED initialisée");
}

// ============================================================================
// INITIALISATION FILESYSTEM
// ============================================================================

void initFilesystem() {
  if (!LittleFS.begin(true)) {
    Serial.println("❌ ERREUR: LittleFS Mount Failed");
    setLEDStatus(LED_ERROR);
    while(1) delay(1000); // Halt si échec critique
  }
  Serial.println("✓ LittleFS monté");
  listLittleFS();
}

// ============================================================================
// CHARGEMENT DES CONFIGURATIONS
// ============================================================================

void loadAllConfigs() {
  Serial.println("📂 Chargement des configurations...");
  loadUsers();
  loadSystemConfig();
  loadCalibrationConfig();
  loadFlexTimers();
  loadMQTTConfig();
  loadHistory();
  loadWeatherConfig();
  Serial.println("✓ Toutes les configurations chargées");
}

// ============================================================================
// INITIALISATION CAPTEURS
// ============================================================================

void initSensors() {
  Serial.println("🔧 Initialisation capteurs...");
  
  // I2C pour INA226
  Wire.begin(INA226_SDA, INA226_SCL);
  Wire.setClock(100000);
  
  // INA226 (Pression)
  if (!ina226.begin()) {
    Serial.println("  ❌ INA226 NOT FOUND at 0x40!");
  } else {
    Serial.println("  ✓ INA226 initialized");
    ina226.setMaxCurrentShunt(0.02, 1.0);
  }
  
  // DS18B20 (Température)
  sensors.begin();
  Serial.println("  ✓ DS18B20 initialisé");
  
  Serial.println("✓ Capteurs prêts");
}

// ============================================================================
// INITIALISATION CORE 1 (Dual Core)
// ============================================================================

void initCore1() {
  Serial.println("🚀 Démarrage du Core 1...");
  
  // Créer le mutex pour la protection des données partagées
  dataMutex = xSemaphoreCreateMutex();
  if (dataMutex == NULL) {
    Serial.println("❌ ERREUR: Impossible de créer le mutex!");
    return;
  }
  Serial.println("✓ Mutex créé");
  
  // Démarrer la tâche sur le Core 1
  xTaskCreatePinnedToCore(
    core1Task,          // Fonction à exécuter
    "Core1Task",        // Nom de la tâche
    10000,              // Taille de la pile (bytes)
    NULL,               // Paramètre
    1,                  // Priorité
    &core1TaskHandle,   // Handle de la tâche
    1                   // Core 1
  );
  
  Serial.println("✓ Core 1 lancé");
}

// ============================================================================
// INITIALISATION WIFI
// ============================================================================

void initWiFi() {
  Serial.println("📡 Connexion WiFi...");
  
  WiFiManager wm;
  setLEDStatus(LED_WIFI_CONNECTING);
  
  if (!wm.autoConnect("PoolConnect_AP")) {
    Serial.println("❌ WiFi failed, restart");
    ESP.restart();
  }
  
  Serial.print("✓ WiFi connecté - IP: ");
  Serial.println(WiFi.localIP());
  
  setLEDStatus(LED_WIFI_CONNECTED);
  buzzerBeep(2); // Signal sonore de connexion réussie
}

// ============================================================================
// INITIALISATION OTA (Over-The-Air Updates)
// ============================================================================

void initOTA() {
  ArduinoOTA.setHostname("PoolConnect");
  ArduinoOTA.begin();
  Serial.println("✓ OTA ready");
}

// ============================================================================
// INITIALISATION NTP (Network Time Protocol)
// ============================================================================

void initNTP() {
  Serial.println("⏰ Configuration NTP...");
  
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.println("✓ NTP configuré");
  
  // Attendre la synchronisation (max 10 secondes)
  Serial.print("Attente sync NTP");
  struct tm timeinfo;
  int ntpRetry = 0;
  
  while (!getLocalTime(&timeinfo) && ntpRetry < 10) {
    Serial.print(".");
    delay(1000);
    ntpRetry++;
  }
  
  if (getLocalTime(&timeinfo)) {
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S", &timeinfo);
    Serial.printf("\n✓ NTP synchronisé: %s (jour %d)\n", timeStr, timeinfo.tm_yday);
  } else {
    Serial.println("\n⚠️ NTP non synchronisé (continuera en arrière-plan)");
  }
}

// ============================================================================
// INITIALISATION MQTT
// ============================================================================

void initMQTT() {
  if (mqttServer != "" && mqttServer.length() > 0) {
    Serial.println("📡 Configuration MQTT...");
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    mqttClient.setBufferSize(1024);  // Forcer la taille du buffer
    mqttClient.setCallback(mqttCallback);
    Serial.printf("✓ MQTT configuré - Serveur: %s:%d\n", mqttServer.c_str(), mqttPort);
  } else {
    Serial.println("ℹ️  MQTT non configuré (peut être configuré via l'interface web)");
  }
}

#endif // SYSTEM_INIT_H
