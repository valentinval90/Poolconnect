/* 
 * ESP32-S3 Pool Connect
 */

// ============================================================================
// INCLUDES - LIBRAIRIES EXTERNES
// ============================================================================

#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Arduino.h>
#include <FastLED.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <INA226.h>

// ============================================================================
// INCLUDES - MODULES DU PROJET (ordre important!)
// ============================================================================

#include "config.h"
#include "types.h"
#include "equation_parser.h"
#include "globals.h"
#include "led_buzzer.h"
#include "sensors.h"
#include "users.h"
#include "storage.h"
#include "weather.h"
#include "mqtt_manager.h"
#include "timer_system.h"
#include "timer_processor_complete.h" 
#include "core_tasks.h"                
#include "system_init.h"                
#include "web_handlers.h"               
#include "web_handlers_impl.h"     
#include "backup_restore.h"
#include "scenarios.h"

// ============================================================================
// SETUP - INITIALISATION DU SYSTÈME
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n========================");
  Serial.println("PoolConnect");
  Serial.println("========================\n");

  // Initialisation hardware
  initRelays();
  initLED();
  initFilesystem();
  
  // Chargement des configurations
  loadAllConfigs();
  
  // Initialisation capteurs et dual-core
  initSensors();
  initCore1();
  
  // Connexion réseau
  initWiFi();
  initOTA();
  
  // ========================================================================
  // CONFIGURATION WEB SERVER - ROUTES
  // ========================================================================
  
  // Fichiers statiques
  server.on("/", handleRoot);
  server.on("/style.css", handleCss);
  server.on("/app.js", handleJs);
  server.on("/logo.png", handleLogo);
  server.on("/favicon.png", handleFavicon);
  
  // API Basique
  server.on("/api/time", handleApiTime);
  server.on("/api/temp", handleApiTemp);
  server.on("/api/relays", handleApiRelays);
  server.on("/api/relay", handleApiRelay);
  server.on("/api/sensors", handleApiSensors);
  server.on("/api/buzzer/mute", handleApiBuzzerMute);
  
  // API MQTT
  server.on("/api/mqtt/config", HTTP_GET, handleApiMQTTConfig);
  server.on("/api/saveMQTT", HTTP_POST, handleApiSaveMQTT);
  server.on("/api/mqtt/status", HTTP_GET, handleApiMQTTStatus);
  server.on("/api/mqtt/rediscover", HTTP_POST, handleApiMQTTRediscover);
  
  // API Météo
  server.on("/api/weather/config", HTTP_GET, handleApiWeatherConfig);
  server.on("/api/weather/save", HTTP_POST, handleApiWeatherSave);
  
  // API Système
  server.on("/api/system", HTTP_GET, handleApiSystem);
  server.on("/api/system/config", HTTP_GET, handleApiGetSystemConfig);
  server.on("/api/system/config", HTTP_POST, handleApiSaveSystemConfig);
  server.on("/api/system/restart", HTTP_POST, handleApiRestart);
  
  // API Calibration
  server.on("/api/calibration", HTTP_GET, handleApiGetCalibration);
  server.on("/api/calibration", HTTP_POST, handleApiSaveCalibration);
  server.on("/api/calibration/reset", HTTP_POST, handleApiResetCalibration);
  
  // API Historique
  server.on("/api/history", HTTP_GET, handleApiHistory);

  // API Configuration Graphique
  server.on("/api/chart/config", handleApiChartConfig);
  
  // API Authentification & Utilisateurs
  server.on("/api/auth", HTTP_POST, handleApiAuth);
  server.on("/api/users", HTTP_GET, handleApiGetUsers);
  server.on("/api/users/add", HTTP_POST, handleApiAddUser);
  server.on("/api/users/delete", HTTP_POST, handleApiDeleteUser);
  server.on("/api/users/change-password", HTTP_POST, handleApiChangePassword);
  
  // API Timers Flexibles
  server.on("/api/timers/flex", HTTP_GET, handleApiFlexTimers);
  server.on("/api/timers/flex", HTTP_POST, handleApiAddFlexTimer);

  // API Backup/Restore
  server.on("/api/backup/download", HTTP_GET, handleBackupDownload);
  server.on("/api/backup/upload", HTTP_POST, handleBackupUpload);
  server.on("/api/backup/list", HTTP_GET, handleBackupList);
  server.on("/api/backup/save", HTTP_POST, handleBackupSave);
  
  // API Scénarios
  server.on("/api/scenarios", HTTP_GET, handleGetScenarios);
  server.on("/api/scenarios/apply", HTTP_POST, handleApplyScenario);
  
  // Routes dynamiques (PUT/DELETE sur /api/timers/flex/{id})
  server.onNotFound([]() {
    String uri = server.uri();
    
    if (uri.startsWith("/api/timers/flex/") && uri.endsWith("/toggle") && 
        server.method() == HTTP_POST) {
      handleApiToggleFlexTimer();
    } 
    else if (uri.startsWith("/api/timers/flex/") && server.method() == HTTP_PUT) {
      handleApiUpdateFlexTimer();
    } 
    else if (uri.startsWith("/api/timers/flex/") && server.method() == HTTP_DELETE) {
      handleApiDeleteFlexTimer();
    } 
    else {
      server.send(404, "text/plain", "Not Found");
    }
  });

  // ========================================================================
  // CONFIGURATION OTA UPDATE
  // ========================================================================
  
  server.on("/update", HTTP_GET, []() {
    server.send(200, "text/html", 
      "<html><body><h1>Mise à jour OTA</h1>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'><input type='submit' value='Mettre à jour'>"
      "</form></body></html>");
  });

  server.on("/update", HTTP_POST, 
    []() { 
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK"); 
      ESP.restart(); 
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    }
  );

  server.begin();
  Serial.println("✓ Web server démarré");

  // Configuration finale
  initNTP();
  initMQTT();
  updateWeatherData();

  setLEDStatus(LED_RUNNING);

  checkAutoBackup();

  Serial.println("\n========================================");
  Serial.println("✅ Setup terminé avec succès!");
  Serial.println("========================================\n");
}

// ============================================================================
// LOOP - BOUCLE PRINCIPALE (Core 0 - Web & OTA uniquement)
// ============================================================================

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  checkAutoBackup();
  delay(10);
}