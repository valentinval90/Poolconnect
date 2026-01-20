/* 
 * POOL CONNECT - CONFIGURATION
 * Pinout, constantes et définitions globales
 */

#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "1.0.1"

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

#endif // CONFIG_H
