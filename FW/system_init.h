/* 
 * POOL CONNECT - SYSTEM INITIALIZATION
 * Fonctions d'initialisation du système
 * system_init.h   V0.2
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
#include "logging.h"
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
  LOG_D(LOG_SYSTEM, "Initialisation des relais...");
  
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
    LOG_V(LOG_SYSTEM, "Relais %d (Pin %d) initialise a LOW", i, relayPins[i]);
  }
  
  pinMode(SENSOR_VOLET, INPUT);
  LOG_V(LOG_SYSTEM, "Capteur volet (Pin %d) configure en INPUT", SENSOR_VOLET);
  
  pinMode(SENSOR_FUITE, INPUT);
  LOG_V(LOG_SYSTEM, "Capteur fuite (Pin %d) configure en INPUT", SENSOR_FUITE);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  LOG_V(LOG_SYSTEM, "Buzzer (Pin %d) initialise a LOW", BUZZER_PIN);
  
  LOG_I(LOG_SYSTEM, "Relais initialises - %d relais configures", NUM_RELAYS);
}

// ============================================================================
// INITIALISATION LED
// ============================================================================

void initLED() {
  LOG_D(LOG_SYSTEM, "Initialisation de la LED RGB...");
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  LOG_V(LOG_SYSTEM, "FastLED configure - Pin: %d, Luminosite: 50", LED_PIN);
  
  setLEDStatus(LED_BOOTING);
  LOG_I(LOG_SYSTEM, "LED initialisee - Status: BOOTING");
}

// ============================================================================
// INITIALISATION FILESYSTEM
// ============================================================================

void initFilesystem() {
  LOG_D(LOG_SYSTEM, "Montage du filesystem LittleFS...");
  
  if (!LittleFS.begin(true)) {
    LOG_E(LOG_SYSTEM, "ERREUR CRITIQUE: Echec du montage LittleFS");
    setLEDStatus(LED_ERROR);
    while(1) {
      LOG_E(LOG_SYSTEM, "System halt - LittleFS mount failed");
      delay(5000);
    }
  }
  
  LOG_I(LOG_SYSTEM, "LittleFS monte avec succes");
  LOG_MEMORY();
  
  LOG_D(LOG_STORAGE, "Liste des fichiers:");
  listLittleFS();
}

// ============================================================================
// CHARGEMENT DES CONFIGURATIONS
// ============================================================================

void loadAllConfigs() {
  LOG_SEPARATOR();
  LOG_I(LOG_STORAGE, "Demarrage du chargement des configurations...");
  
  LOG_D(LOG_STORAGE, "Chargement des utilisateurs...");
  loadUsers();
  
  LOG_D(LOG_STORAGE, "Chargement de la configuration systeme...");
  loadSystemConfig();
  
  LOG_D(LOG_STORAGE, "Chargement de la calibration...");
  loadCalibrationConfig();
  
  LOG_D(LOG_STORAGE, "Chargement des timers flexibles...");
  loadFlexTimers();
  
  LOG_D(LOG_STORAGE, "Chargement de la configuration MQTT...");
  loadMQTTConfig();
  
  LOG_D(LOG_STORAGE, "Chargement de l'historique...");
  loadHistory();
  
  LOG_D(LOG_STORAGE, "Chargement de la configuration meteo...");
  loadWeatherConfig();
  
  LOG_I(LOG_STORAGE, "Toutes les configurations chargees avec succes");
  LOG_MEMORY();
  LOG_SEPARATOR();
}

// ============================================================================
// INITIALISATION CAPTEURS
// ============================================================================

void initSensors() {
  LOG_SEPARATOR();
  LOG_I(LOG_SENSOR, "Initialisation des capteurs...");
  
  // I2C pour INA226
  LOG_D(LOG_SENSOR, "Configuration du bus I2C...");
  Wire.begin(INA226_SDA, INA226_SCL);
  Wire.setClock(100000);
  LOG_V(LOG_SENSOR, "I2C configure - SDA: Pin %d, SCL: Pin %d, Freq: 100kHz", 
        INA226_SDA, INA226_SCL);
  
  // INA226 (Pression)
  LOG_D(LOG_SENSOR, "Initialisation du capteur INA226 (Pression)...");
  if (!ina226.begin()) {
    LOG_E(LOG_SENSOR, "INA226 non detecte a l'adresse 0x40");
    LOG_W(LOG_SENSOR, "La mesure de pression ne sera pas disponible");
  } else {
    ina226.setMaxCurrentShunt(0.02, 1.0);
    LOG_I(LOG_SENSOR, "INA226 initialise avec succes");
    LOG_V(LOG_SENSOR, "INA226 - Max Current: 0.02A, Shunt: 1.0 Ohm");
  }
  
  // DS18B20 (Température)
  LOG_D(LOG_SENSOR, "Initialisation du capteur DS18B20 (Temperature)...");
  sensors.begin();
  LOG_I(LOG_SENSOR, "DS18B20 initialise avec succes");
  
  LOG_I(LOG_SENSOR, "Tous les capteurs sont prets");
  LOG_SEPARATOR();
}

// ============================================================================
// INITIALISATION CORE 1 (Dual Core)
// ============================================================================

void initCore1() {
  LOG_SEPARATOR();
  LOG_I(LOG_SYSTEM, "Demarrage du systeme Dual Core...");
  
  // Créer le mutex pour la protection des données partagées
  LOG_D(LOG_SYSTEM, "Creation du mutex pour la protection des donnees...");
  dataMutex = xSemaphoreCreateMutex();
  
  if (dataMutex == NULL) {
    LOG_E(LOG_SYSTEM, "ERREUR CRITIQUE: Impossible de creer le mutex");
    LOG_E(LOG_SYSTEM, "Le systeme dual-core ne peut pas demarrer");
    return;
  }
  LOG_I(LOG_SYSTEM, "Mutex cree avec succes");
  
  // Démarrer la tâche sur le Core 1
  LOG_D(LOG_SYSTEM, "Creation de la tache Core1Task...");
  BaseType_t result = xTaskCreatePinnedToCore(
    core1Task,          // Fonction à exécuter
    "Core1Task",        // Nom de la tâche
    10000,              // Taille de la pile (bytes)
    NULL,               // Paramètre
    1,                  // Priorité
    &core1TaskHandle,   // Handle de la tâche
    1                   // Core 1
  );
  
  if (result == pdPASS) {
    LOG_I(LOG_SYSTEM, "Core 1 lance avec succes");
    LOG_V(LOG_SYSTEM, "Tache Core1Task - Stack: 10000 bytes, Priorite: 1");
  } else {
    LOG_E(LOG_SYSTEM, "Echec du lancement du Core 1");
  }
  
  LOG_MEMORY();
  LOG_SEPARATOR();
}

// ============================================================================
// INITIALISATION WIFI
// ============================================================================

void initWiFi() {
  LOG_SEPARATOR();
  LOG_I(LOG_NETWORK, "Demarrage de la connexion WiFi...");
  
  WiFiManager wm;
  setLEDStatus(LED_WIFI_CONNECTING);
  LOG_D(LOG_NETWORK, "WiFiManager en mode auto-connect...");
  LOG_I(LOG_NETWORK, "Point d'acces de secours: PoolConnect_AP");
  
  unsigned long startAttempt = millis();
  if (!wm.autoConnect("PoolConnect_AP")) {
    LOG_E(LOG_NETWORK, "Echec de la connexion WiFi apres timeout");
    LOG_W(LOG_NETWORK, "Redemarrage de l'ESP32...");
    delay(1000);
    ESP.restart();
  }
  
  unsigned long connectTime = millis() - startAttempt;
  LOG_I(LOG_NETWORK, "WiFi connecte avec succes en %lu ms", connectTime);
  LOG_I(LOG_NETWORK, "Adresse IP: %s", WiFi.localIP().toString().c_str());
  LOG_I(LOG_NETWORK, "Passerelle: %s", WiFi.gatewayIP().toString().c_str());
  LOG_I(LOG_NETWORK, "Masque reseau: %s", WiFi.subnetMask().toString().c_str());
  LOG_I(LOG_NETWORK, "SSID: %s", WiFi.SSID().c_str());
  LOG_I(LOG_NETWORK, "RSSI: %d dBm", WiFi.RSSI());
  LOG_V(LOG_NETWORK, "MAC Address: %s", WiFi.macAddress().c_str());
  
  setLEDStatus(LED_WIFI_CONNECTED);
  buzzerBeep(2); // Signal sonore de connexion réussie
  LOG_D(LOG_SYSTEM, "Signal sonore de connexion emis (2 bips)");
  
  LOG_SEPARATOR();
}

// ============================================================================
// INITIALISATION OTA (Over-The-Air Updates)
// ============================================================================

void initAruinoOTA() {
  LOG_D(LOG_NETWORK, "Configuration OTA (Over-The-Air Updates)...");
  
  ArduinoOTA.setHostname("PoolConnect");
  LOG_V(LOG_NETWORK, "OTA Hostname: PoolConnect");
  
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    LOG_I(LOG_NETWORK, "Demarrage de la mise a jour OTA: %s", type.c_str());
  });
  
  ArduinoOTA.onEnd([]() {
    LOG_I(LOG_NETWORK, "Mise a jour OTA terminee avec succes");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int lastPercent = 0;
    unsigned int percent = (progress / (total / 100));
    if (percent != lastPercent && percent % 10 == 0) {
      LOG_I(LOG_NETWORK, "Progression OTA: %u%%", percent);
      lastPercent = percent;
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    LOG_E(LOG_NETWORK, "Erreur OTA [%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      LOG_E(LOG_NETWORK, "Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      LOG_E(LOG_NETWORK, "Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      LOG_E(LOG_NETWORK, "Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      LOG_E(LOG_NETWORK, "Receive Failed");
    } else if (error == OTA_END_ERROR) {
      LOG_E(LOG_NETWORK, "End Failed");
    }
  });
  
  ArduinoOTA.begin();
  LOG_I(LOG_NETWORK, "OTA pret et en attente de mise a jour");
}

// ============================================================================
// INITIALISATION NTP (Network Time Protocol)
// ============================================================================

void initNTP() {
  LOG_SEPARATOR();
  LOG_I(LOG_NETWORK, "Configuration du protocole NTP...");
  LOG_V(LOG_NETWORK, "Serveur NTP: %s", NTP_SERVER);
  LOG_V(LOG_NETWORK, "GMT Offset: %d secondes", GMT_OFFSET_SEC);
  LOG_V(LOG_NETWORK, "Daylight Offset: %d secondes", DAYLIGHT_OFFSET_SEC);
  
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  LOG_I(LOG_NETWORK, "NTP configure - Attente de synchronisation...");
  
  // Attendre la synchronisation (max 10 secondes)
  struct tm timeinfo;
  int ntpRetry = 0;
  
  while (!getLocalTime(&timeinfo) && ntpRetry < 10) {
    LOG_V(LOG_NETWORK, "Tentative de synchronisation NTP %d/10...", ntpRetry + 1);
    delay(1000);
    ntpRetry++;
  }
  
  if (getLocalTime(&timeinfo)) {
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S", &timeinfo);
    LOG_I(LOG_NETWORK, "NTP synchronise avec succes");
    LOG_I(LOG_NETWORK, "Date et heure: %s", timeStr);
    LOG_V(LOG_NETWORK, "Jour de l'annee: %d", timeinfo.tm_yday);
    LOG_V(LOG_NETWORK, "Jour de la semaine: %d (0=Dimanche)", timeinfo.tm_wday);
  } else {
    LOG_W(LOG_NETWORK, "NTP non synchronise apres %d tentatives", ntpRetry);
    LOG_W(LOG_NETWORK, "La synchronisation continuera en arriere-plan");
  }
  
  LOG_SEPARATOR();
}

// ============================================================================
// INITIALISATION MQTT
// ============================================================================

void initMQTT() {
  LOG_SEPARATOR();
  
  if (mqttServer != "" && mqttServer.length() > 0) {
    LOG_I(LOG_MQTT, "Configuration du client MQTT...");
    LOG_I(LOG_MQTT, "Serveur: %s:%d", mqttServer.c_str(), mqttPort);
    
    mqttClient.setServer(mqttServer.c_str(), mqttPort);
    LOG_V(LOG_MQTT, "Serveur MQTT configure");
    
    mqttClient.setBufferSize(1024);
    LOG_V(LOG_MQTT, "Taille du buffer MQTT: 1024 bytes");
    
    mqttClient.setCallback(mqttCallback);
    LOG_V(LOG_MQTT, "Callback MQTT enregistre");
    
    LOG_I(LOG_MQTT, "MQTT configure avec succes");
    LOG_I(LOG_MQTT, "Username: %s", mqttUser.length() > 0 ? mqttUser.c_str() : "Non configure");
    LOG_I(LOG_MQTT, "Topic de base: %s", mqttTopic.c_str());
  } else {
    LOG_W(LOG_MQTT, "MQTT non configure");
    LOG_I(LOG_MQTT, "Vous pouvez configurer MQTT via l'interface web");
  }
  
  LOG_SEPARATOR();
}

#endif // SYSTEM_INIT_H