/* 
 * POOL CONNECT - WEB HANDLERS
 * Déclarations des handlers HTTP
 */

#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <Arduino.h>

// Handlers de fichiers statiques
void handleRoot();
void handleCss();
void handleJs();
void handleLogo();
void handleFavicon();

// API Basique
void handleApiTime();
void handleApiTemp();
void handleApiRelays();
void handleApiRelay();
void handleApiSensors();
void handleApiBuzzerMute();

// API MQTT
void handleApiMQTTConfig();
void handleApiSaveMQTT();
void handleApiMQTTStatus();
void handleApiMQTTRediscover();

// API Météo
void handleApiWeatherConfig();
void handleApiWeatherSave();

// API Système
void handleApiSystem();
void handleApiGetSystemConfig();
void handleApiSaveSystemConfig();
void handleApiRestart();

// API Calibration
void handleApiGetCalibration();
void handleApiSaveCalibration();
void handleApiResetCalibration();

// API Historique
void handleApiHistory();

// API Authentification & Utilisateurs
void handleApiAuth();
void handleApiGetUsers();
void handleApiAddUser();
void handleApiDeleteUser();
void handleApiChangePassword();

// API Timers Flexibles
void handleApiFlexTimers();
void handleApiAddFlexTimer();
void handleApiUpdateFlexTimer();
void handleApiDeleteFlexTimer();
void handleApiToggleFlexTimer();

// Note: Les implémentations sont dans le fichier .ino principal
// pour éviter la duplication de code et simplifier la maintenance

#endif // WEB_HANDLERS_H
