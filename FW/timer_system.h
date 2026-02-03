/* 
 * POOL CONNECT - SYSTÈME DE TIMERS FLEXIBLE
 * Classe de gestion des timers (TimerManager)
 * timer_system.h   V0.2
 * 
 * NOTE: Les structures (FlexibleTimer, Action, Condition, etc.) sont définies dans types.h
 *       Ce fichier ne contient que la classe TimerManager pour éviter les doublons.
 */

#ifndef TIMER_SYSTEM_H
#define TIMER_SYSTEM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "types.h"
#include "config.h"
#include "logging.h"

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
    LOG_I(LOG_TIMER, "TimerManager cree - Capacite: %d timers", maxTimers);
    LOG_V(LOG_TIMER, "Taille memoire allouee: %d bytes", maxTimers * sizeof(FlexibleTimer));
  }
  
  ~TimerManager() {
    LOG_D(LOG_TIMER, "Destruction du TimerManager - Suppression de %d timers", timerCount);
    delete[] timers;
  }
  
  void init(float* wTemp, float* wPress, float* eTemp, 
            bool* cover, bool* leak, int* pins, int pCount) {
    LOG_D(LOG_TIMER, "Initialisation du TimerManager...");
    
    waterTemp = wTemp;
    waterPressure = wPress;
    extTemp = eTemp;
    coverOpen = cover;
    waterLeak = leak;
    relayPins = pins;
    relayCount = pCount;
    
    LOG_I(LOG_TIMER, "TimerManager initialise avec succes");
    LOG_V(LOG_TIMER, "Pointeurs configures: temp, pression, temp ext, volet, fuite");
    LOG_V(LOG_TIMER, "Nombre de relais disponibles: %d", relayCount);
  }
  
  bool addTimer(FlexibleTimer& timer) {
    LOG_D(LOG_TIMER, "Tentative d'ajout d'un timer...");
    
    if (timerCount >= maxTimers) {
      LOG_E(LOG_TIMER, "Impossible d'ajouter le timer: limite atteinte (%d/%d)", 
            timerCount, maxTimers);
      return false;
    }
    
    timer.id = millis() & 0x7FFFFFFF;
    timers[timerCount++] = timer;
    
    LOG_I(LOG_TIMER, "Timer ajoute avec succes - ID: %d, Nom: '%s'", timer.id, timer.name.c_str());
    LOG_I(LOG_TIMER, "Total de timers: %d/%d", timerCount, maxTimers);
    LOG_V(LOG_TIMER, "Timer: enabled=%d, conditions=%d, actions=%d", 
          timer.enabled, timer.conditionCount, timer.actionCount);
    
    return true;
  }
  
  bool deleteTimer(int id) {
    LOG_D(LOG_TIMER, "Tentative de suppression du timer ID %d...", id);
    
    for (int i = 0; i < timerCount; i++) {
      if (timers[i].id == id) {
        LOG_I(LOG_TIMER, "Timer trouve: '%s' (index %d)", timers[i].name.c_str(), i);
        
        // Décalage des timers
        for (int j = i; j < timerCount - 1; j++) {
          timers[j] = timers[j + 1];
        }
        timerCount--;
        
        LOG_I(LOG_TIMER, "Timer ID %d supprime avec succes", id);
        LOG_I(LOG_TIMER, "Total de timers: %d/%d", timerCount, maxTimers);
        return true;
      }
    }
    
    LOG_W(LOG_TIMER, "Timer ID %d non trouve - Suppression impossible", id);
    return false;
  }
  
  FlexibleTimer* getTimer(int id) {
    LOG_V(LOG_TIMER, "Recherche du timer ID %d...", id);
    
    for (int i = 0; i < timerCount; i++) {
      if (timers[i].id == id) {
        LOG_V(LOG_TIMER, "Timer ID %d trouve: '%s'", id, timers[i].name.c_str());
        return &timers[i];
      }
    }
    
    LOG_V(LOG_TIMER, "Timer ID %d non trouve", id);
    return nullptr;
  }
  
  FlexibleTimer* getTimers() { 
    LOG_V(LOG_TIMER, "Acces a la liste des timers (%d timers)", timerCount);
    return timers; 
  }
  
  int getTimerCount() { 
    LOG_V(LOG_TIMER, "Nombre de timers: %d", timerCount);
    return timerCount; 
  }
  
  bool checkConditions(FlexibleTimer& timer) {
    LOG_V(LOG_TIMER, "Verification des conditions pour timer '%s'", timer.name.c_str());
    LOG_V(LOG_TIMER, "Nombre de conditions: %d", timer.conditionCount);
    
    if (timer.conditionCount == 0) {
      LOG_V(LOG_TIMER, "Aucune condition - Timer autorise");
      return true;
    }
    
    for (int i = 0; i < timer.conditionCount; i++) {
      Condition& c = timer.conditions[i];
      bool met = false;
      
      switch(c.type) {
        case CONDITION_COVER_OPEN:
          met = *coverOpen;
          LOG_V(LOG_TIMER, "Condition %d: COVER_OPEN - %s (required=%d)", 
                i, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_COVER_CLOSED:
          met = !(*coverOpen);
          LOG_V(LOG_TIMER, "Condition %d: COVER_CLOSED - %s (required=%d)", 
                i, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_TEMP_MIN:
          met = (*waterTemp >= c.value);
          LOG_V(LOG_TIMER, "Condition %d: TEMP_MIN %.2f >= %.2f - %s (required=%d)", 
                i, *waterTemp, c.value, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_TEMP_MAX:
          met = (*waterTemp <= c.value);
          LOG_V(LOG_TIMER, "Condition %d: TEMP_MAX %.2f <= %.2f - %s (required=%d)", 
                i, *waterTemp, c.value, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_TEMP_EXT_MIN:
          met = (*extTemp >= c.value);
          LOG_V(LOG_TIMER, "Condition %d: TEMP_EXT_MIN %.2f >= %.2f - %s (required=%d)", 
                i, *extTemp, c.value, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_TEMP_EXT_MAX:
          met = (*extTemp <= c.value);
          LOG_V(LOG_TIMER, "Condition %d: TEMP_EXT_MAX %.2f <= %.2f - %s (required=%d)", 
                i, *extTemp, c.value, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_PRESSURE_MIN:
          met = (*waterPressure >= c.value);
          LOG_V(LOG_TIMER, "Condition %d: PRESSURE_MIN %.2f >= %.2f - %s (required=%d)", 
                i, *waterPressure, c.value, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_PRESSURE_MAX:
          met = (*waterPressure <= c.value);
          LOG_V(LOG_TIMER, "Condition %d: PRESSURE_MAX %.2f <= %.2f - %s (required=%d)", 
                i, *waterPressure, c.value, met ? "OUI" : "NON", c.required);
          break;
        case CONDITION_NO_LEAK:
          met = !(*waterLeak);
          LOG_V(LOG_TIMER, "Condition %d: NO_LEAK - %s (required=%d)", 
                i, met ? "OUI" : "NON", c.required);
          break;
        default:
          LOG_W(LOG_TIMER, "Condition %d: Type inconnu (%d)", i, (int)c.type);
          break;
      }
      
      if (c.required && !met) {
        LOG_W(LOG_TIMER, "Condition requise %d non satisfaite - Timer '%s' bloque", 
              i, timer.name.c_str());
        return false;
      }
    }
    
    LOG_I(LOG_TIMER, "Toutes les conditions satisfaites pour timer '%s'", timer.name.c_str());
    return true;
  }
  
  void processTimers(struct tm* timeinfo);
  bool saveToFile(const char* filename);
  bool loadFromFile(const char* filename);
};

#endif // TIMER_SYSTEM_H