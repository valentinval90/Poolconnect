/* 
 * POOL CONNECT - VARIABLES GLOBALES
 * Déclarations extern de toutes les variables globales
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <FastLED.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <INA226.h>
#include "config.h"
#include "types.h"

// ============================================================================
// OBJETS GLOBAUX
// ============================================================================

extern CRGB leds[NUM_LEDS];
extern OneWire oneWire;
extern DallasTemperature sensors;
extern INA226 ina226;

extern WebServer server;
extern WiFiClient espClient;
extern PubSubClient mqttClient;

// Dual Core
extern SemaphoreHandle_t dataMutex;
extern TaskHandle_t core1TaskHandle;

// ============================================================================
// PINOUT
// ============================================================================

extern const int relayPins[NUM_RELAYS];

// ============================================================================
// CONFIGURATION
// ============================================================================

extern SystemConfig sysConfig;
extern CalibrationConfig calibConfig;

// ============================================================================
// UTILISATEURS
// ============================================================================

extern User users[MAX_USERS];
extern int userCount;

// ============================================================================
// TIMERS
// ============================================================================

extern FlexibleTimer flexTimers[MAX_TIMERS];
extern int flexTimerCount;

// ============================================================================
// HISTORIQUE
// ============================================================================

extern HistoryEntry history[MAX_HISTORY];
extern int historyCount;

// ============================================================================
// CAPTEURS (protégés par mutex)
// ============================================================================

extern float waterTemp;
extern float waterPressure;
extern bool waterLeak;
extern bool coverOpen;
extern float pressureThreshold;
extern bool buzzerMuted;
extern float tempSum;
extern int tempSamples;

// ============================================================================
// MÉTÉO
// ============================================================================

extern String weatherApiKey;
extern String latitude;
extern String longitude;
extern float tempExterieure;
extern float weatherTempMax;
extern float weatherTempMin;
extern float weatherSunshine;
extern unsigned long lastWeatherUpdate;

// ============================================================================
// MQTT
// ============================================================================

extern String mqttServer;
extern int mqttPort;
extern String mqttUser;
extern String mqttPassword;
extern String mqttTopic;
extern unsigned long lastMqttAttempt;

#endif // GLOBALS_H
