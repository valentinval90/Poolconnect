/* 
 * POOL CONNECT - TIMER PROCESSOR
 * Logique d'exécution des timers flexibles
 */

#ifndef TIMER_PROCESSOR_H
#define TIMER_PROCESSOR_H

#include <Arduino.h>
#include <time.h>
#include "globals.h"
#include "config.h"
#include "logging.h"
#include "timer_system.h"
#include "equation_parser.h"
#include "led_buzzer.h"

// ============================================================================
// UTILITAIRES
// ============================================================================

bool willTimerRestartImmediately(FlexibleTimer* timer, struct tm* timeinfo, 
                                 int currentDayOfYear, int currentMinutes,
                                 float wTemp, float wPress, float eTemp,
                                 bool cover, bool leak) {
  
  LOG_V(LOG_TIMER, "Verification redemarrage immediat pour timer '%s'", timer->name.c_str());
  
  // Vérifier si on est sur un nouveau jour
  if (timer->lastTriggeredDay == currentDayOfYear) {
    LOG_V(LOG_TIMER, "Timer deja execute aujourd'hui (jour %d)", currentDayOfYear);
    return false;
  }
  
  // Vérifier si le jour est activé
  if (!timer->days[timeinfo->tm_wday]) {
    LOG_V(LOG_TIMER, "Jour %d non actif pour ce timer", timeinfo->tm_wday);
    return false;
  }
  
  // Vérifier l'heure de démarrage
  int startMinutes = 0;
  bool timeOK = false;
  
  switch(timer->startTime.type) {
    case START_FIXED:
      startMinutes = timer->startTime.hour * 60 + timer->startTime.minute;
      timeOK = (currentMinutes >= startMinutes);
      LOG_V(LOG_TIMER, "START_FIXED: %d min >= %d min ? %s", currentMinutes, startMinutes, timeOK ? "OUI" : "NON");
      break;
    case START_SUNRISE:
      startMinutes = 7 * 60 + timer->startTime.sunriseOffset;
      timeOK = (currentMinutes >= startMinutes);
      LOG_V(LOG_TIMER, "START_SUNRISE: %d min >= %d min ? %s", currentMinutes, startMinutes, timeOK ? "OUI" : "NON");
      break;
    case START_SUNSET:
      startMinutes = 20 * 60 + timer->startTime.sunriseOffset;
      timeOK = (currentMinutes >= startMinutes);
      LOG_V(LOG_TIMER, "START_SUNSET: %d min >= %d min ? %s", currentMinutes, startMinutes, timeOK ? "OUI" : "NON");
      break;
  }
  
  if (!timeOK) {
    LOG_V(LOG_TIMER, "Heure de demarrage non atteinte");
    return false;
  }
  
  // Vérifier les conditions
  for (int i = 0; i < timer->conditionCount; i++) {
    Condition& c = timer->conditions[i];
    bool met = false;
    
    switch(c.type) {
      case CONDITION_COVER_OPEN: met = cover; break;
      case CONDITION_COVER_CLOSED: met = !cover; break;
      case CONDITION_TEMP_MIN: met = (wTemp >= c.value); break;
      case CONDITION_TEMP_MAX: met = (wTemp <= c.value); break;
      case CONDITION_TEMP_EXT_MIN: met = (eTemp >= c.value); break;
      case CONDITION_TEMP_EXT_MAX: met = (eTemp <= c.value); break;
      case CONDITION_PRESSURE_MIN: met = (wPress >= c.value); break;
      case CONDITION_PRESSURE_MAX: met = (wPress <= c.value); break;
      case CONDITION_NO_LEAK: met = !leak; break;
    }
    
    if (c.required && !met) {
      LOG_V(LOG_TIMER, "Condition requise #%d non satisfaite", i);
      return false;
    }
  }
  
  LOG_I(LOG_TIMER, "Timer '%s' pret a redemarrer immediatement", timer->name.c_str());
  return true;
}

// ============================================================================
// TRAITEMENT DES TIMERS
// ============================================================================

void processFlexTimers(struct tm* timeinfo) {
  if (!timeinfo) {
    LOG_E(LOG_TIMER, "timeinfo est NULL - Impossible de traiter les timers");
    return;
  }
  
  unsigned long nowMillis = millis();
  int currentDayOfYear = timeinfo->tm_yday;
  int currentMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;
  
  // Debug périodique
  static int lastMinute = -1;
  if (timeinfo->tm_min != lastMinute) {
    int activeCount = 0;
    for (int i = 0; i < flexTimerCount; i++) {
      if (flexTimers[i].context.state == TIMER_RUNNING) activeCount++;
    }
    if (activeCount > 0) {
      LOG_I(LOG_TIMER, "Heure: %02d:%02d - %d timers actifs en cours d'execution", 
            timeinfo->tm_hour, timeinfo->tm_min, activeCount);
    }
    lastMinute = timeinfo->tm_min;
  }
  
  // PROTECTION: Si la pompe s'arrête, arrêter aussi l'électrolyseur
  static bool lastPompeState = false;
  bool currentPompeState = (digitalRead(relayPins[0]) == HIGH);
  if (lastPompeState && !currentPompeState) {
    if (digitalRead(relayPins[1]) == HIGH) {
      digitalWrite(relayPins[1], LOW);
      LOG_W(LOG_TIMER, "PROTECTION: Electrolyseur arrete automatiquement (pompe arretee)");
      if (mqttClient.connected()) {
        mqttClient.publish((mqttTopic + "/relay/1/state").c_str(), "0");
        LOG_MQTT_PUB((mqttTopic + "/relay/1/state").c_str(), "0");
      }
    }
  }
  lastPompeState = currentPompeState;
  
  for (int i = 0; i < flexTimerCount; i++) {
    FlexibleTimer* timer = &flexTimers[i];
    
    // Timer désactivé
    if (!timer->enabled) {
      if (timer->context.state != TIMER_IDLE) {
        LOG_W(LOG_TIMER, "Timer %d '%s' desactive - Arret en cours", timer->id, timer->name.c_str());
        // Arrêter tous les relais de ce timer
        for (int a = 0; a < timer->actionCount; a++) {
          if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
            digitalWrite(relayPins[timer->actions[a].relay], LOW);
            LOG_D(LOG_TIMER, "Relais %d eteint", timer->actions[a].relay);
          }
        }
        timer->context.state = TIMER_IDLE;
        LOG_I(LOG_TIMER, "Timer %d desactive et arrete", timer->id);
      }
      continue;
    }
    
    // Vérifier jour d'activation
    if (!timer->days[timeinfo->tm_wday]) {
      if (timer->context.state != TIMER_IDLE) {
        LOG_V(LOG_TIMER, "Timer %d - Jour %d non actif, retour a IDLE", timer->id, timeinfo->tm_wday);
        timer->context.state = TIMER_IDLE;
      }
      continue;
    }
    
    // Arrêt d'urgence si fuite
    if (waterLeak && timer->context.state == TIMER_RUNNING) {
      LOG_SEPARATOR();
      LOG_E(LOG_TIMER, "========================================");
      LOG_E(LOG_TIMER, "URGENCE: Timer %d arrete - FUITE DETECTEE", timer->id);
      LOG_E(LOG_TIMER, "========================================");
      LOG_SEPARATOR();
      
      for (int a = 0; a < timer->actionCount; a++) {
        if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
          digitalWrite(relayPins[timer->actions[a].relay], LOW);
          LOG_W(LOG_TIMER, "Relais %d eteint (urgence)", timer->actions[a].relay);
        }
      }
      timer->context.state = TIMER_ERROR;
      timer->context.lastError = "Fuite détectée";
      LOG_E(LOG_TIMER, "Timer %d en etat ERROR: %s", timer->id, timer->context.lastError.c_str());
      continue;
    }
    
    // Logique d'état du timer
    switch(timer->context.state) {
      
      case TIMER_IDLE:
      {
        // Vérifier si c'est le moment de démarrer
        bool shouldStart = false;
        int startMinutes = 0;
        
        switch(timer->startTime.type) {
          case START_FIXED:
            startMinutes = timer->startTime.hour * 60 + timer->startTime.minute;
            shouldStart = (currentMinutes >= startMinutes && 
                          timer->lastTriggeredDay != currentDayOfYear);
            if (shouldStart) {
              LOG_V(LOG_TIMER, "Timer %d: Heure fixe atteinte (%02d:%02d)", 
                    timer->id, timer->startTime.hour, timer->startTime.minute);
            }
            break;
            
          case START_SUNRISE:
            startMinutes = 7 * 60 + timer->startTime.sunriseOffset;
            shouldStart = (currentMinutes >= startMinutes && 
                          timer->lastTriggeredDay != currentDayOfYear);
            if (shouldStart) {
              LOG_V(LOG_TIMER, "Timer %d: Lever du soleil + %d min atteint", 
                    timer->id, timer->startTime.sunriseOffset);
            }
            break;
            
          case START_SUNSET:
            startMinutes = 20 * 60 + timer->startTime.sunriseOffset;
            shouldStart = (currentMinutes >= startMinutes && 
                          timer->lastTriggeredDay != currentDayOfYear);
            if (shouldStart) {
              LOG_V(LOG_TIMER, "Timer %d: Coucher du soleil + %d min atteint", 
                    timer->id, timer->startTime.sunriseOffset);
            }
            break;
        }
        
        if (shouldStart) {
          LOG_D(LOG_TIMER, "Timer %d: Verification des conditions de demarrage...", timer->id);
          
          // Vérifier conditions
          bool conditionsOK = true;
          for (int c = 0; c < timer->conditionCount; c++) {
            bool met = false;
            Condition* cond = &timer->conditions[c];
            
            switch(cond->type) {
              case CONDITION_COVER_OPEN:
                met = coverOpen;
                LOG_V(LOG_TIMER, "Condition %d: Volet ouvert ? %s", c, met ? "OUI" : "NON");
                break;
              case CONDITION_COVER_CLOSED:
                met = !coverOpen;
                LOG_V(LOG_TIMER, "Condition %d: Volet ferme ? %s", c, met ? "OUI" : "NON");
                break;
              case CONDITION_TEMP_MIN:
                met = (waterTemp >= cond->value);
                LOG_V(LOG_TIMER, "Condition %d: Temp %.2f >= %.2f ? %s", c, waterTemp, cond->value, met ? "OUI" : "NON");
                break;
              case CONDITION_TEMP_MAX:
                met = (waterTemp <= cond->value);
                LOG_V(LOG_TIMER, "Condition %d: Temp %.2f <= %.2f ? %s", c, waterTemp, cond->value, met ? "OUI" : "NON");
                break;
              case CONDITION_TEMP_EXT_MIN:
                met = (tempExterieure >= cond->value);
                LOG_V(LOG_TIMER, "Condition %d: Temp ext %.2f >= %.2f ? %s", c, tempExterieure, cond->value, met ? "OUI" : "NON");
                break;
              case CONDITION_TEMP_EXT_MAX:
                met = (tempExterieure <= cond->value);
                LOG_V(LOG_TIMER, "Condition %d: Temp ext %.2f <= %.2f ? %s", c, tempExterieure, cond->value, met ? "OUI" : "NON");
                break;
              case CONDITION_PRESSURE_MIN:
                met = (waterPressure >= cond->value);
                LOG_V(LOG_TIMER, "Condition %d: Pression %.2f >= %.2f ? %s", c, waterPressure, cond->value, met ? "OUI" : "NON");
                break;
              case CONDITION_PRESSURE_MAX:
                met = (waterPressure <= cond->value);
                LOG_V(LOG_TIMER, "Condition %d: Pression %.2f <= %.2f ? %s", c, waterPressure, cond->value, met ? "OUI" : "NON");
                break;
              case CONDITION_NO_LEAK:
                met = !waterLeak;
                LOG_V(LOG_TIMER, "Condition %d: Pas de fuite ? %s", c, met ? "OUI" : "NON");
                break;
            }
            
            if (cond->required && !met) {
              conditionsOK = false;
              LOG_W(LOG_TIMER, "Condition requise %d non satisfaite - Timer %d annule", c, timer->id);
              break;
            }
          }
          
          if (conditionsOK) {
            LOG_SEPARATOR();
            LOG_I(LOG_TIMER, "========================================");
            LOG_I(LOG_TIMER, "DEMARRAGE TIMER %d: '%s'", timer->id, timer->name.c_str());
            LOG_I(LOG_TIMER, "========================================");
            LOG_I(LOG_TIMER, "Jour: %d, Heure: %02d:%02d", timeinfo->tm_wday, timeinfo->tm_hour, timeinfo->tm_min);
            LOG_I(LOG_TIMER, "Nombre d'actions: %d", timer->actionCount);
            LOG_SEPARATOR();
            
            timer->context.state = TIMER_RUNNING;
            timer->context.timerStartMillis = nowMillis;
            timer->context.currentActionIndex = 0;
            timer->context.actionStartMillis = nowMillis;
            timer->context.tempMeasured = false;
            timer->lastTriggeredDay = currentDayOfYear;
            
            LOG_TIMER_EVENT("START", timer->name.c_str());
          } else {
            LOG_W(LOG_TIMER, "Timer %d en attente - Conditions non remplies", timer->id);
            timer->lastTriggeredDay = currentDayOfYear;
          }
        }
        break;
      }
      
      case TIMER_RUNNING:
      {
        // ┌────────────────────────────────────────────────────────────────┐
        // │ Si on est sur les dernières actions ET cycle 24h              │
        // └────────────────────────────────────────────────────────────────┘
        if (timer->context.currentActionIndex >= timer->actionCount - 2) {
          
          // Vérifier si c'est un cycle de 24h (ou proche : entre 23.5h et 24h)
          bool isCycle24h = (timer->context.calculatedDurationHours >= 23.5 && 
                            timer->context.calculatedDurationHours <= 24.0);
          
          if (isCycle24h) {
            LOG_D(LOG_TIMER, "Timer %d: Detection cycle 24h (%.1fh)", 
                  timer->id, timer->context.calculatedDurationHours);
            
            // Vérifier si le timer va redémarrer immédiatement
            bool willRestart = willTimerRestartImmediately(
              timer, timeinfo, currentDayOfYear, currentMinutes,
              waterTemp, waterPressure, tempExterieure, coverOpen, waterLeak
            );
            
            if (willRestart) {
              LOG_I(LOG_TIMER, "Timer %d: Cycle 24h detecte, redemarrage prevu demain", timer->id);
              
              // Cycle 24h ET redémarrage prévu → identifier les relais à maintenir actifs
              bool relaysToKeepActive[NUM_RELAYS] = {false};
              
              // Regarder les premières actions pour savoir quels relais seront ON au redémarrage
              for (int a = 0; a < timer->actionCount && a < 5; a++) {
                if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
                  relaysToKeepActive[timer->actions[a].relay] = true;
                }
              }
              
              // Vérifier si l'action actuelle coupe un relais qui sera réactivé
              Action* currentAction = &timer->actions[timer->context.currentActionIndex];
              
              if (currentAction->type == ACTION_RELAY && 
                  !currentAction->state && 
                  relaysToKeepActive[currentAction->relay]) {
                
                // Cette action coupe un relais qui sera réactivé → LA SAUTER !
                const char* relayNames[] = {"Pompe", "Électrolyseur", "Lampe", "Électrovalve", "PAC"};
                
                LOG_I(LOG_TIMER, "Cycle 24h detecte (%.1fh), redemarrage prevu demain",
                      timer->context.calculatedDurationHours);
                LOG_I(LOG_TIMER, "Action %d (%s OFF) sautee pour continuite",
                      timer->context.currentActionIndex + 1, 
                      relayNames[currentAction->relay]);
                
                // Sauter l'action
                timer->context.currentActionIndex++;
                timer->context.actionStartMillis = nowMillis;
                
                if (timer->context.currentActionIndex >= timer->actionCount) {
                  // Toutes actions terminées
                  LOG_I(LOG_TIMER, "Timer %d termine", timer->id);
                  LOG_I(LOG_TIMER, "Relais maintenus actifs pour cycle continu");
                  timer->context.state = TIMER_COMPLETED;
                  timer->context.currentActionIndex = 0;
                }
                
                break; // Passer au prochain cycle
              }
            }
          }
        }
        
        // ┌────────────────────────────────────────────────────────────────┐
        // │ Vérifier si TOUTES les actions sont terminées                 │
        // └────────────────────────────────────────────────────────────────┘
        if (timer->context.currentActionIndex >= timer->actionCount) {
          
          bool willRestartImmediately = willTimerRestartImmediately(
            timer, timeinfo, currentDayOfYear, currentMinutes,
            waterTemp, waterPressure, tempExterieure, coverOpen, waterLeak
          );
          
          LOG_SEPARATOR();
          LOG_I(LOG_TIMER, "Timer %d termine", timer->id);
          
          if (willRestartImmediately) {
            LOG_I(LOG_TIMER, "Timer %d va redemarrer immediatement", timer->id);
            timer->context.state = TIMER_COMPLETED;
            timer->context.currentActionIndex = 0;
            
          } else {
            LOG_I(LOG_TIMER, "Timer %d arrete - Relais desactives", timer->id);
            timer->context.state = TIMER_COMPLETED;
            
            // Arrêter tous les relais
            for (int a = 0; a < timer->actionCount; a++) {
              if (timer->actions[a].type == ACTION_RELAY) {
                digitalWrite(relayPins[timer->actions[a].relay], LOW);
                LOG_V(LOG_TIMER, "  Relais %d OFF", timer->actions[a].relay);
              }
            }
            
            timer->context.currentActionIndex = 0;
          }
          
          LOG_SEPARATOR();
          LOG_TIMER_EVENT("COMPLETE", timer->name.c_str());
          
          break;
        }
        
        // ┌────────────────────────────────────────────────────────────────┐
        // │ Exécuter l'action courante                                     │
        // └────────────────────────────────────────────────────────────────┘
        Action* action = &timer->actions[timer->context.currentActionIndex];
        unsigned long actionElapsed = nowMillis - timer->context.actionStartMillis;
        
        LOG_V(LOG_TIMER, "Timer %d: Execution action %d/%d (type=%d)", 
              timer->id, timer->context.currentActionIndex + 1, timer->actionCount, (int)action->type);
        
        // Gérer le délai
        if (action->delayMinutes > 0) {
          unsigned long delayMillis = action->delayMinutes * 60000UL;
          if (actionElapsed < delayMillis) {
            static unsigned long lastDelayLog = 0;
            if (millis() - lastDelayLog > 60000) {
              unsigned long remainingMin = (delayMillis - actionElapsed) / 60000;
              LOG_V(LOG_TIMER, "Timer %d: Delai en cours - Reste %lu minutes", timer->id, remainingMin);
              lastDelayLog = millis();
            }
            break; // Attendre
          }
        }
        
        // Exécuter l'action
        bool actionComplete = false;
        
        switch(action->type) {
          case ACTION_RELAY:
            // Protection électrolyseur
            if (action->relay == 1 && action->state) {
              if (digitalRead(relayPins[0]) != HIGH) {
                LOG_E(LOG_TIMER, "Timer %d: ERREUR - Pompe doit etre active pour electrolyseur", timer->id);
                timer->context.lastError = "Pompe doit être active";
                timer->context.state = TIMER_ERROR;
                break;
              }
            }
            digitalWrite(relayPins[action->relay], action->state ? HIGH : LOW);
            LOG_I(LOG_TIMER, "Timer %d: Relais %d -> %s", 
                  timer->id, action->relay, action->state ? "ON" : "OFF");
            
            // Capturer le changement d'état sur le graphique
            captureCurrentStateToChart();
            
            if (mqttClient.connected()) {
              String topic = mqttTopic + "/relay/" + String(action->relay) + "/state";
              mqttClient.publish(topic.c_str(), action->state ? "1" : "0", true);
              LOG_MQTT_PUB(topic.c_str(), action->state ? "1" : "0");
            }
            
            actionComplete = true;
            break;
            
          case ACTION_WAIT_DURATION:
            if (actionElapsed >= (unsigned long)action->delayMinutes * 60000UL) {
              LOG_I(LOG_TIMER, "Timer %d: Attente de %d minutes terminee", timer->id, action->delayMinutes);
              actionComplete = true;
            }
            break;
            
          case ACTION_MEASURE_TEMP:
          {
            // Vérifier pompe active
            if (digitalRead(relayPins[0]) != HIGH) {
              LOG_W(LOG_TIMER, "Timer %d: Demarrage pompe pour mesure temperature", timer->id);
              digitalWrite(relayPins[0], HIGH);
              timer->context.timerStartMillis = nowMillis;
              timer->context.tempMeasureCount = 0;
              break;
            }
            
            unsigned long pumpTime = nowMillis - timer->context.timerStartMillis;
            
            // Mesure 1 à 5 minutes
            if (timer->context.tempMeasureCount == 0) {
              if (pumpTime >= 300000UL) {
                timer->context.measuredTemp1 = waterTemp;
                timer->context.tempMeasureCount = 1;
                LOG_I(LOG_TIMER, "Timer %d: Mesure 1/3 (5 min) = %.2f C", 
                      timer->id, timer->context.measuredTemp1);
              }
              break;
            }
            
            // Mesure 2 à 10 minutes
            if (timer->context.tempMeasureCount == 1) {
              if (pumpTime >= 600000UL) {
                timer->context.measuredTemp2 = waterTemp;
                timer->context.tempMeasureCount = 2;
                LOG_I(LOG_TIMER, "Timer %d: Mesure 2/3 (10 min) = %.2f C", 
                      timer->id, timer->context.measuredTemp2);
              }
              break;
            }
            
            // Mesure 3 à 15 minutes + calcul moyenne
            if (timer->context.tempMeasureCount == 2) {
              if (pumpTime >= 900000UL) {
                timer->context.measuredTemp3 = waterTemp;
                timer->context.measuredTempAvg = (timer->context.measuredTemp1 + 
                                                  timer->context.measuredTemp2 + 
                                                  timer->context.measuredTemp3) / 3.0;
                timer->context.tempMeasured = true;
                
                LOG_SEPARATOR();
                LOG_I(LOG_TIMER, "Timer %d: Mesures de temperature terminees", timer->id);
                LOG_I(LOG_TIMER, "  Mesure 1 (5 min)  = %.2f C", timer->context.measuredTemp1);
                LOG_I(LOG_TIMER, "  Mesure 2 (10 min) = %.2f C", timer->context.measuredTemp2);
                LOG_I(LOG_TIMER, "  Mesure 3 (15 min) = %.2f C", timer->context.measuredTemp3);
                LOG_I(LOG_TIMER, "  MOYENNE = %.2f C", timer->context.measuredTempAvg);
                LOG_SEPARATOR();
                
                actionComplete = true;
              }
              break;
            }
            break;
          }
          
          case ACTION_AUTO_DURATION:
            if (timer->context.tempMeasured) {
              if (action->maxWaitMinutes == 0) {
                float durationHours;
                
                if (action->customEquation.useCustom && action->customEquation.expression.length() > 0) {
                  // Équation personnalisée
                  LOG_I(LOG_TIMER, "Timer %d: Calcul avec equation personnalisee", timer->id);
                  LOG_D(LOG_TIMER, "Expression: %s", action->customEquation.expression.c_str());
                  
                  EquationParser parser;
                  bool error = false;
                  
                  float wTemp, eTemp, wMax, wMin, sun;
                  
                  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
                    wTemp = timer->context.measuredTempAvg;
                    eTemp = tempExterieure;
                    wMax = weatherTempMax;
                    wMin = weatherTempMin;
                    sun = weatherSunshine;
                    xSemaphoreGive(dataMutex);
                  }
                  
                  parser.setVariables(wTemp, eTemp, wMax, wMin, sun);
                  durationHours = parser.calculate(action->customEquation.expression, error);
                  
                  if (error || isnan(durationHours) || isinf(durationHours)) {
                    LOG_E(LOG_TIMER, "Timer %d: Erreur dans l'equation '%s'", 
                          timer->id, action->customEquation.expression.c_str());
                    timer->context.lastError = "Erreur dans l'équation personnalisée";
                    timer->context.state = TIMER_ERROR;
                    break;
                  }
                  
                  LOG_I(LOG_TIMER, "Timer %d: Resultat equation = %.2f heures", timer->id, durationHours);
                  LOG_V(LOG_TIMER, "Variables: waterTemp=%.2f, extTemp=%.2f, max=%.2f, min=%.2f, sun=%.0f%%",
                        wTemp, eTemp, wMax, wMin, sun);
                  
                } else {
                  // Formule par défaut
                  durationHours = timer->context.measuredTempAvg / 2.0;
                  LOG_I(LOG_TIMER, "Timer %d: Calcul avec formule par defaut (temp/2)", timer->id);
                  LOG_I(LOG_TIMER, "Duree calculee: %.2f heures", durationHours);
                }
                
                // Limiter entre 3h et 24h
                if (durationHours < 3.0) {
                  LOG_W(LOG_TIMER, "Timer %d: Duree %.2fh < 3h, ajuste a 3h", timer->id, durationHours);
                  durationHours = 3.0;
                }
                
                if (durationHours > 24.0) {
                  LOG_W(LOG_TIMER, "Timer %d: Duree %.2fh > 24h, ajuste a 24h", timer->id, durationHours);
                  durationHours = 24.0;
                }
                
                action->maxWaitMinutes = (int)(durationHours * 60);
                timer->context.actionStartMillis = nowMillis;
                
                LOG_I(LOG_TIMER, "Timer %d: Duree finale = %.1f heures (%d minutes)", 
                      timer->id, durationHours, action->maxWaitMinutes);

                timer->context.calculatedDurationHours = durationHours;
              }
              
              // Vérifier si la durée est écoulée
              unsigned long targetMillis = (unsigned long)action->maxWaitMinutes * 60000UL;
              
              if (actionElapsed >= targetMillis) {
                LOG_I(LOG_TIMER, "Timer %d: Duree automatique terminee (%d minutes)", 
                      timer->id, action->maxWaitMinutes);
                
                actionComplete = true;
              } else {
                // Affichage périodique de la progression
                static unsigned long lastProgressLog = 0;
                if (millis() - lastProgressLog > 300000) {
                  float progressPercent = (actionElapsed / (float)targetMillis) * 100.0;
                  unsigned long remainingMin = (targetMillis - actionElapsed) / 60000;
                  
                  LOG_I(LOG_TIMER, "Timer %d: Progression %.1f%% - Reste %lu minutes", 
                        timer->id, progressPercent, remainingMin);
                  lastProgressLog = millis();
                }
              }
            } else {
              static unsigned long lastTempWaitLog = 0;
              if (millis() - lastTempWaitLog > 60000) {
                LOG_V(LOG_TIMER, "Timer %d: En attente de mesure de temperature...", timer->id);
                lastTempWaitLog = millis();
              }
            }
            break;
          
          case ACTION_BUZZER:
            if (!buzzerMuted && sysConfig.buzzerEnabled) {
              if (action->buzzerCount == 0) {
                buzzerAlarm();
                LOG_I(LOG_TIMER, "Timer %d: Buzzer ALARME", timer->id);
              } else {
                buzzerBeep(action->buzzerCount);
                LOG_I(LOG_TIMER, "Timer %d: Buzzer %d bip(s)", timer->id, action->buzzerCount);
              }
            } else {
              LOG_V(LOG_TIMER, "Timer %d: Buzzer desactive", timer->id);
            }
            actionComplete = true;
            break;
            
          case ACTION_LED:
          {
            CRGB color;
            const char* colorNames[] = {"Noir", "Bleu", "Vert", "Cyan", "Rouge", "Magenta", "Jaune", "Blanc"};
            
            switch(action->ledColor) {
              case 0: color = CRGB::Black; break;
              case 1: color = CRGB::Blue; break;
              case 2: color = CRGB::Green; break;
              case 3: color = CRGB::Cyan; break;
              case 4: color = CRGB::Red; break;
              case 5: color = CRGB::Magenta; break;
              case 6: color = CRGB::Yellow; break;
              case 7: color = CRGB::White; break;
              default: color = CRGB::Black; break;
            }
            
            leds[0] = color;
            FastLED.show();
            
            LOG_I(LOG_TIMER, "Timer %d: LED %s, mode=%d, duree=%ds", 
                  timer->id, colorNames[action->ledColor], action->ledMode, action->ledDuration);
            
            actionComplete = true;
            break;
          }
        }
        
        if (actionComplete) {
          timer->context.currentActionIndex++;
          timer->context.actionStartMillis = nowMillis;
          
          if (timer->context.currentActionIndex < timer->actionCount) {
            LOG_I(LOG_TIMER, "Timer %d: Passage a l'action %d/%d", 
                  timer->id, timer->context.currentActionIndex + 1, timer->actionCount);
          }
        }
        
        break;
      }
      
      case TIMER_COMPLETED:
      {
        if (timer->lastTriggeredDay != currentDayOfYear) {
          LOG_D(LOG_TIMER, "Timer %d: Nouveau jour detecte, retour a IDLE", timer->id);
          timer->context.state = TIMER_IDLE;
        }
        break;
      }
      
      case TIMER_ERROR:
      {
        if (timer->lastTriggeredDay != currentDayOfYear) {
          LOG_I(LOG_TIMER, "Timer %d: Reset erreur (nouveau jour)", timer->id);
          LOG_V(LOG_TIMER, "Erreur precedente: %s", timer->context.lastError.c_str());
          timer->context.state = TIMER_IDLE;
          timer->context.lastError = "";
        }
        break;
      }
    }
  }
}

#endif // TIMER_PROCESSOR_H