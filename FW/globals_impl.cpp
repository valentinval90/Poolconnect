/* 
 * POOL CONNECT - GLOBALS IMPLEMENTATION
 * Définitions réelles des variables globales
 */

#include "globals.h"
#include "timer_system.h"

// ============================================================================
// OBJETS GLOBAUX
// ============================================================================

CRGB leds[NUM_LEDS];
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
INA226 ina226(0x40);

WebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Dual Core
SemaphoreHandle_t dataMutex = NULL;
TaskHandle_t core1TaskHandle = NULL;

// ============================================================================
// PINOUT
// ============================================================================

const int relayPins[] = {RELAY_POMPE, RELAY_ELECTROLYSEUR, RELAY_LAMPE, RELAY_ELECTROVALVE, RELAY_PAC};

// ============================================================================
// CONFIGURATION
// ============================================================================

SystemConfig sysConfig = {2.0, true};

CalibrationConfig calibConfig = {
  false, false, 0.0, 10.0, 10.0, 30.0, 30.0,  // Temp
  false, false, 0.0, 1.0, 1.0, 3.0, 3.0       // Pressure
};

// ============================================================================
// CONSTANTES (définies ici, déclarées dans config.h)
// ============================================================================

const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 3600;
const int DAYLIGHT_OFFSET_SEC = 3600;
const unsigned long MQTT_RECONNECT_INTERVAL = 5000;
const unsigned long WEATHER_UPDATE_INTERVAL = 600000UL;

// ============================================================================
// UTILISATEURS
// ============================================================================

User users[MAX_USERS];
int userCount = 0;

// ============================================================================
// TIMERS
// ============================================================================

FlexibleTimer flexTimers[MAX_TIMERS];
int flexTimerCount = 0;

// ============================================================================
// HISTORIQUE
// ============================================================================

HistoryEntry history[MAX_HISTORY];
int historyCount = 0;

// ============================================================================
// CAPTEURS
// ============================================================================

float waterTemp = 0.0;
float waterPressure = 0.0;
bool waterLeak = false;
bool coverOpen = true;
float pressureThreshold = 2.0;
bool buzzerMuted = false;
float tempSum = 0;
int tempSamples = 0;

// ============================================================================
// MÉTÉO
// ============================================================================

String weatherApiKey = "5cf2428f975acb80a72457db198dd2ab";
String latitude = "47.666672";
String longitude = "6.85";
float tempExterieure = 0.0;
float weatherTempMax = 0.0;
float weatherTempMin = 0.0;
float weatherSunshine = 0.0;
unsigned long lastWeatherUpdate = 0;

// ============================================================================
// MQTT
// ============================================================================

String mqttServer = "";
int mqttPort = 1883;
String mqttUser = "";
String mqttPassword = "";
String mqttTopic = "pool/control";
unsigned long lastMqttAttempt = 0;
