/* 
 * POOL CONNECT - CONFIGURATION
 * Pinout, constantes et définitions globales
 * config.h   V0.4
 */

#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "1.0.2"

// ============================================================================
// PINOUT ESP32-S3-WROOM-1-N16R8
// ============================================================================

// Relais (LOW=repos, HIGH=actif)
#define RELAY_POMPE 4
#define RELAY_ELECTROLYSEUR 5
#define RELAY_LAMPE 6
#define RELAY_ELECTROVALVE 7
#define RELAY_PAC 46

#define NUM_RELAYS 5

// Capteurs
#define SENSOR_FUITE 1
#define SENSOR_VOLET 2
#define DS18B20_PIN 14
#define INA226_SDA 18
#define INA226_SCL 17
#define INA226_ALERT 8

// Sorties
#define LED_PIN 38
#define BUZZER_PIN 21
#define NUM_LEDS 1
#define LED_TYPE SK6812
#define COLOR_ORDER GRB

// ============================================================================
// CONSTANTES
// ============================================================================

#define MAX_USERS 10
#define MAX_TIMERS 20
#define MAX_HISTORY 50

// NTP - Déclarations extern (définies dans globals_impl.cpp)
extern const char* NTP_SERVER;
extern const long GMT_OFFSET_SEC;
extern const int DAYLIGHT_OFFSET_SEC;

// MQTT
extern const unsigned long MQTT_RECONNECT_INTERVAL;

// Météo
extern const unsigned long WEATHER_UPDATE_INTERVAL;

// ============================================
// CONFIGURATION DES LOGS
// ============================================

// Activer/désactiver le système de logging
#define ENABLE_LOGGING true

// Niveau de log global (LOG_NONE=0, LOG_ERROR=1, LOG_WARNING=2, LOG_INFO=3, LOG_DEBUG=4, LOG_VERBOSE=5)
#define LOG_LEVEL 4

// Activer/désactiver les catégories spécifiques de logs
#define LOG_WEB_REQUESTS       true  // Logs des requêtes HTTP
#define LOG_WEB_DATA_EXCHANGE  true  // Logs des données échangées web <-> hardware
#define LOG_SENSOR_VALUES      true  // Logs des lectures de capteurs
#define LOG_TIMER_EVENTS       true  // Logs des événements de timers
#define LOG_MQTT_MESSAGES      true  // Logs des messages MQTT
#define LOG_STORAGE_OPS        true  // Logs des opérations de stockage
#define LOG_MEMORY_INFO        true  // Logs de l'état de la mémoire
#define LOG_OTA_OPERATIONS     true  // Logs des opérations OTA (Over-The-Air)
#define LOG_CHART_OPS          true  // Logs des opérations de graphique

// Vitesse du port série pour les logs
#define SERIAL_BAUD_RATE 115200

#endif // CONFIG_H
