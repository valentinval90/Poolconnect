/* 
 * POOL CONNECT - SYSTÈME DE TIMERS FLEXIBLE
 * Classe de gestion des timers (TimerManager)
 * 
 * NOTE: Les structures (FlexibleTimer, Action, Condition, etc.) sont définies dans types.h
 *       Ce fichier ne contient que la classe TimerManager pour éviter les doublons.
 */

#ifndef TIMER_SYSTEM_H
#define TIMER_SYSTEM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "types.h"

// ============================================================================
// CLASSE DE GESTION DES TIMERS
// ============================================================================

class TimerManager {
private:
  FlexibleTimer* timers;
  int maxTimers;
  int timerCount;
  
  float* waterTemp;
  float* waterPressure;
  float* extTemp;
  bool* coverOpen;
  bool* waterLeak;
  int* relayPins;
  int relayCount;
  
public:
  TimerManager(int maxCount) : maxTimers(maxCount), timerCount(0) {
    timers = new FlexibleTimer[maxTimers];
  }
  
  ~TimerManager() {
    delete[] timers;
  }
  
  void init(float* wTemp, float* wPress, float* eTemp, 
            bool* cover, bool* leak, int* pins, int pCount) {
    waterTemp = wTemp;
    waterPressure = wPress;
    extTemp = eTemp;
    coverOpen = cover;
    waterLeak = leak;
    relayPins = pins;
    relayCount = pCount;
  }
  
  bool addTimer(FlexibleTimer& timer) {
    if (timerCount >= maxTimers) return false;
    timer.id = millis() & 0x7FFFFFFF;
    timers[timerCount++] = timer;
    return true;
  }
  
  bool deleteTimer(int id) {
    for (int i = 0; i < timerCount; i++) {
      if (timers[i].id == id) {
        for (int j = i; j < timerCount - 1; j++) {
          timers[j] = timers[j + 1];
        }
        timerCount--;
        return true;
      }
    }
    return false;
  }
  
  FlexibleTimer* getTimer(int id) {
    for (int i = 0; i < timerCount; i++) {
      if (timers[i].id == id) return &timers[i];
    }
    return nullptr;
  }
  
  FlexibleTimer* getTimers() { return timers; }
  int getTimerCount() { return timerCount; }
  
  bool checkConditions(FlexibleTimer& timer) {
    for (int i = 0; i < timer.conditionCount; i++) {
      Condition& c = timer.conditions[i];
      bool met = false;
      
      switch(c.type) {
        case CONDITION_COVER_OPEN:
          met = *coverOpen;
          break;
        case CONDITION_COVER_CLOSED:
          met = !(*coverOpen);
          break;
        case CONDITION_TEMP_MIN:
          met = (*waterTemp >= c.value);
          break;
        case CONDITION_TEMP_MAX:
          met = (*waterTemp <= c.value);
          break;
        case CONDITION_TEMP_EXT_MIN:
          met = (*extTemp >= c.value);
          break;
        case CONDITION_TEMP_EXT_MAX:
          met = (*extTemp <= c.value);
          break;
        case CONDITION_PRESSURE_MIN:
          met = (*waterPressure >= c.value);
          break;
        case CONDITION_PRESSURE_MAX:
          met = (*waterPressure <= c.value);
          break;
        case CONDITION_NO_LEAK:
          met = !(*waterLeak);
          break;
      }
      
      if (c.required && !met) {
        return false;
      }
    }
    return true;
  }
  
  void processTimers(struct tm* timeinfo);
  bool saveToFile(const char* filename);
  bool loadFromFile(const char* filename);
};

#endif // TIMER_SYSTEM_H
