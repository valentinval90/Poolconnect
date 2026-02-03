/* 
 * POOL CONNECT - TYPES & STRUCTURES
 * Définitions de toutes les structures de données
 * types.h   V0.3
 */

#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

// ============================================================================
// STRUCTURES
// ============================================================================

struct SystemConfig {
  float pressureThreshold;
  bool buzzerEnabled;
};

struct CalibrationConfig {
  // Température
  bool tempUseCalibration;
  bool tempUseTwoPoint;
  float tempOffset;
  float tempPoint1Raw;
  float tempPoint1Real;
  float tempPoint2Raw;
  float tempPoint2Real;
  
  // Pression
  bool pressureUseCalibration;
  bool pressureUseTwoPoint;
  float pressureOffset;
  float pressurePoint1Raw;
  float pressurePoint1Real;
  float pressurePoint2Raw;
  float pressurePoint2Real;
};

struct UserPreferences {
  String language;              // "fr" ou "en"
  String temperatureUnit;       // "celsius" ou "fahrenheit"
  String pressureUnit;          // "bar" ou "psi"
  String theme;                 // "light" ou "dark"
  int chartUpdateInterval;      // En millisecondes (300000 = 5min par défaut)
  
  UserPreferences() : language("fr"), 
                      temperatureUnit("celsius"),
                      pressureUnit("bar"),
                      theme("light"),
                      chartUpdateInterval(300000) {}
};

struct User {
  String username;
  String passwordHash;
  String role;
  bool enabled;
};

struct HistoryEntry {
  char date[12];
  char startTime[6];
  int duration;
  float avgTemp;
};

// ============================================================================
// ÉNUMÉRATIONS
// ============================================================================

enum LEDStatus {
  LED_BOOTING,
  LED_WIFI_CONNECTING,
  LED_WIFI_CONNECTED,
  LED_ERROR,
  LED_RUNNING,
  LED_ALARM
};

enum ActionType {
  ACTION_RELAY,           // Activer/désactiver un relais
  ACTION_WAIT_DURATION,   // Attendre X minutes
  ACTION_WAIT_TIME,       // Attendre jusqu'à HH:MM
  ACTION_WAIT_CONDITION,  // Attendre une condition
  ACTION_MEASURE_TEMP,    // Mesurer température (après 15min pompe)
  ACTION_AUTO_DURATION,   // Durée automatique = équation personnalisée
  ACTION_IF_CONDITION,    // Action conditionnelle
  ACTION_BUZZER,          // Buzzer (beep ou alarme)
  ACTION_LED              // LED (couleur et mode)
};

enum ConditionType {
  CONDITION_COVER_OPEN,      // Volet ouvert
  CONDITION_COVER_CLOSED,    // Volet fermé
  CONDITION_TEMP_MIN,        // Température eau >= X
  CONDITION_TEMP_MAX,        // Température eau <= X
  CONDITION_TEMP_EXT_MIN,    // Température ext >= X
  CONDITION_TEMP_EXT_MAX,    // Température ext <= X
  CONDITION_PRESSURE_MIN,    // Pression >= X
  CONDITION_PRESSURE_MAX,    // Pression <= X
  CONDITION_NO_LEAK          // Pas de fuite
};

enum StartTimeType {
  START_FIXED,    // Heure fixe HH:MM
  START_SUNRISE,  // Lever du soleil
  START_SUNSET    // Coucher du soleil
};

enum TimerState {
  TIMER_IDLE,           // En attente de démarrage
  TIMER_WAITING_START,  // Attente heure de début
  TIMER_RUNNING,        // En cours d'exécution
  TIMER_PAUSED,         // En pause (condition non remplie)
  TIMER_COMPLETED,      // Terminé
  TIMER_ERROR           // Erreur
};

// ============================================================================
// STRUCTURES DE TIMERS
// ============================================================================

struct CustomEquation {
  String expression;
  bool useCustom;
  
  CustomEquation() : expression("waterTemp / 2"), useCustom(false) {}
};

struct Condition {
  ConditionType type;
  float value;
  bool required;
  
  Condition() : type(CONDITION_NO_LEAK), value(0), required(true) {}
};

struct Action {
  ActionType type;
  int relay;
  bool state;
  int delayMinutes;
  float conditionValue;
  int maxWaitMinutes;
  String description;
  
  // Buzzer et LED
  int buzzerCount;
  int ledColor;
  int ledMode;
  int ledDuration;
  
  // Équation personnalisée pour ACTION_AUTO_DURATION
  CustomEquation customEquation;
  
  Action() : type(ACTION_RELAY), relay(0), state(false), 
             delayMinutes(0), conditionValue(0), maxWaitMinutes(0),
             buzzerCount(1), ledColor(0), ledMode(0), ledDuration(0) {}
};

struct StartTime {
  StartTimeType type;
  int hour;
  int minute;
  int sunriseOffset;
  
  StartTime() : type(START_FIXED), hour(9), minute(0), sunriseOffset(0) {}
};

struct TimerExecutionContext {
  int currentActionIndex;
  unsigned long actionStartMillis;
  unsigned long timerStartMillis;
  
  // Mesures de température (3 points + moyenne)
  float measuredTemp1;
  float measuredTemp2;
  float measuredTemp3;
  float measuredTempAvg;
  int tempMeasureCount;
  bool tempMeasured;
  
  // Durée calculée
  float calculatedDurationHours;
  
  bool pumpRunning15min;
  TimerState state;
  String lastError;
  unsigned long totalElapsedMinutes;
  
  TimerExecutionContext() : currentActionIndex(0), actionStartMillis(0),
                           timerStartMillis(0), 
                           measuredTemp1(0), measuredTemp2(0), measuredTemp3(0),
                           measuredTempAvg(0), tempMeasureCount(0),
                           tempMeasured(false), calculatedDurationHours(0),
                           pumpRunning15min(false), state(TIMER_IDLE), 
                           totalElapsedMinutes(0) {}
};

struct FlexibleTimer {
  int id;
  String name;
  bool enabled;
  bool days[7];
  StartTime startTime;
  Condition conditions[10];
  int conditionCount;
  Action actions[50];
  int actionCount;
  int lastTriggeredDay;
  TimerExecutionContext context;
  
  FlexibleTimer() : id(0), name(""), enabled(true), conditionCount(0),
                   actionCount(0), lastTriggeredDay(-1) {
    for (int i = 0; i < 7; i++) days[i] = false;
  }
};

#endif // TYPES_H
