 /* 
 * POOL CONNECT - MAIN
 * Poolconnect.ino   V0.4
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
// INCLUDES - MODULES DU PROJET
// ============================================================================

#include "config.h"
#include "logging.h"
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
#include "timer_processor.h" 
#include "core_tasks.h"                
#include "system_init.h"                
#include "web_handlers.h"                 
#include "backup_restore.h"
#include "scenarios.h"
#include "ota_manager.h"
#include "chart_storage.h"
#include "chart_archiver.h"
#include "chart_web_handlers.h"
#include "chart_event_points.h"

// ============================================================================
// SETUP - INITIALISATION DU SYSTÈME
// ============================================================================

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);
  
  // Initialisation du système de logging
  LOG_INIT();
  
  LOG_SEPARATOR();
  LOG_I(LOG_SYSTEM, "========================================");
  LOG_I(LOG_SYSTEM, "POOL CONNECT v%s", FIRMWARE_VERSION);
  LOG_I(LOG_SYSTEM, "ESP32-S3 - Demarrage du systeme");
  LOG_I(LOG_SYSTEM, "========================================");
  LOG_SEPARATOR();

  // Initialisation hardware
  LOG_I(LOG_SYSTEM, "Phase 1: Initialisation materielle...");
  initRelays();
  initLED();
  initFilesystem();
  LOG_I(LOG_SYSTEM, "Phase 1: Complete");
  
  // Chargement des configurations
  LOG_I(LOG_SYSTEM, "Phase 2: Chargement des configurations...");
  loadAllConfigs();
  LOG_I(LOG_SYSTEM, "Phase 2: Complete");
  
  // Initialisation capteurs et dual-core
  LOG_I(LOG_SYSTEM, "Phase 3: Initialisation capteurs et dual-core...");
  initSensors();
  initCore1();
  LOG_I(LOG_SYSTEM, "Phase 3: Complete");
  
  // Connexion réseau
  LOG_I(LOG_SYSTEM, "Phase 4: Connexion reseau...");
  initWiFi();
  initAruinoOTA();
  LOG_I(LOG_SYSTEM, "Phase 4: Complete");
  
  // ========================================================================
  // CONFIGURATION WEB SERVER - ROUTES
  // ========================================================================
  
  LOG_I(LOG_SYSTEM, "Phase 5: Configuration du serveur web...");
  
  // Fichiers statiques
  server.on("/", handleRoot);
  server.on("/style.css", handleCss);
  server.on("/app.js", handleJs);

  server.on("/modules/auth.js", handleJSauth);
  server.on("/modules/backup.js", handleJsbackup);
  server.on("/modules/calibration.js", handleJscalib);
  server.on("/modules/chart.js", handleJschart);
  server.on("/modules/control.js", handleJscontrol);
  server.on("/modules/dashboard.js", handleJsdashboard);
  server.on("/modules/preferences.js", handleJspref);
  server.on("/modules/scenarios.js", handleJsscenar);
  server.on("/modules/settings.js", handleJssettings);
  server.on("/modules/theme.js", handleJstheme);
  server.on("/modules/timers.js", handleJstimers);
  server.on("/modules/units.js", handleJsunits);
  server.on("/modules/users.js", handleJsusers);
  server.on("/modules/translations.js", handlelangageJs);
  server.on("/modules/ota.js", handleotaJs);

  server.on("/img/logo.png", handleLogo);
  server.on("/img/favicon.png", handleFavicon);
  
  // API Basique
  server.on("/api/time", handleApiTime);
  server.on("/api/temp", handleApiTemp);
  server.on("/api/relays", handleApiRelays);
  server.on("/api/relay", handleApiRelay);
  server.on("/api/sensors", handleApiSensors);
  server.on("/api/buzzer/mute", handleApiBuzzerMute);
  server.on("/api/pump/status", handleApiPumpStatus);
  
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

  // API Chart Data (nouveau système d'historique)
  server.on("/api/chart/data", HTTP_GET, handleApiChartData);
  server.on("/api/chart/available-dates", HTTP_GET, handleApiChartAvailableDates);
  server.on("/api/chart/storage-info", HTTP_GET, handleApiChartStorageInfo);
  server.on("/api/chart/force-archive", HTTP_POST, handleApiChartForceArchive);
  server.on("/api/chart/data", HTTP_DELETE, handleApiChartDeleteDay);
  server.on("/api/chart/export-csv", HTTP_GET, handleApiChartExportCSV);
  
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

  // API Préférence
  server.on("/api/preferences", HTTP_GET, handleApiGetPreferences);
  server.on("/api/preferences", HTTP_POST, handleApiSavePreferences);

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
      LOG_W(LOG_WEB, "Route non trouvee: %s", uri.c_str());
      server.send(404, "text/plain", "Not Found");
    }
  });

  LOG_I(LOG_SYSTEM, "Serveur web configure: 40+ endpoints enregistres");
  
  // ========================================================================
  // CONFIGURATION OTA UPDATE
  // ========================================================================
  
  LOG_D(LOG_SYSTEM, "Configuration OTA update via web...");
  
  server.on("/update", HTTP_GET, []() {
    LOG_WEB_REQUEST("GET", "/update");
    server.send(200, "text/html", 
      "<html><body><h1>Mise à jour OTA</h1>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'><input type='submit' value='Mettre à jour'>"
      "</form></body></html>");
  });

  server.on("/update", HTTP_POST, 
    []() { 
      bool success = !Update.hasError();
      LOG_I(LOG_SYSTEM, "Mise a jour OTA: %s", success ? "SUCCES" : "ECHEC");
      server.send(200, "text/plain", success ? "OK" : "FAIL"); 
      
      if (success) {
        LOG_I(LOG_SYSTEM, "Redemarrage dans 2 secondes...");
        delay(2000);
        ESP.restart();
      }
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        LOG_I(LOG_SYSTEM, "Debut mise a jour OTA: %s", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          LOG_E(LOG_SYSTEM, "Erreur demarrage mise a jour");
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          LOG_E(LOG_SYSTEM, "Erreur ecriture mise a jour");
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          LOG_I(LOG_SYSTEM, "Mise a jour terminee: %u bytes", upload.totalSize);
        } else {
          LOG_E(LOG_SYSTEM, "Erreur finalisation mise a jour");
          Update.printError(Serial);
        }
      }
    }
  );

  OTAManager::setupOTARoutes(&server);

  server.begin();
  LOG_I(LOG_SYSTEM, "Serveur web demarre sur le port 80");
  LOG_I(LOG_SYSTEM, "Phase 5: Complete");

  // Configuration finale
  LOG_I(LOG_SYSTEM, "Phase 6: Configuration finale...");
  initNTP();
  initMQTT();
  updateWeatherData();

    // Initialisation du système de graphique
  LOG_I(LOG_SYSTEM, "Initialisation du systeme de graphique...");
  initChartStorage();
  initChartArchiver();
  LOG_I(LOG_SYSTEM, "Systeme de graphique initialise");

  LOG_I(LOG_SYSTEM, "Phase 6: Complete");

  setLEDStatus(LED_RUNNING);

  checkAutoBackup();

  LOG_SEPARATOR();
  LOG_I(LOG_SYSTEM, "========================================");
  LOG_I(LOG_SYSTEM, "SETUP TERMINE AVEC SUCCES!");
  LOG_I(LOG_SYSTEM, "========================================");
  LOG_I(LOG_SYSTEM, "Systeme pret - Core 0: Web/OTA, Core 1: Capteurs/Timers");
  LOG_SEPARATOR();
  
  LOG_MEMORY();
}

// ============================================================================
// LOOP - BOUCLE PRINCIPALE (Core 0 - Web & OTA uniquement)
// ============================================================================

void loop() {
  server.handleClient();
  checkAutoBackup();
  checkMemoryPeriodic();
  delay(10);
}