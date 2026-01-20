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
  
  // Vérifier si on est sur un nouveau jour
  if (timer->lastTriggeredDay == currentDayOfYear) {
    return false;
  }
  
  // Vérifier si le jour est activé
  if (!timer->days[timeinfo->tm_wday]) {
    return false;
  }
  
  // Vérifier l'heure de démarrage
  int startMinutes = 0;
  bool timeOK = false;
  
  switch(timer->startTime.type) {
    case START_FIXED:
      startMinutes = timer->startTime.hour * 60 + timer->startTime.minute;
      timeOK = (currentMinutes >= startMinutes);
      break;
    case START_SUNRISE:
      startMinutes = 7 * 60 + timer->startTime.sunriseOffset;
      timeOK = (currentMinutes >= startMinutes);
      break;
    case START_SUNSET:
      startMinutes = 20 * 60 + timer->startTime.sunriseOffset;
      timeOK = (currentMinutes >= startMinutes);
      break;
  }
  
  if (!timeOK) return false;
  
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
    
    if (c.required && !met) return false;
  }
  
  return true;
}

// ============================================================================
// TRAITEMENT DES TIMERS
// ============================================================================

void processFlexTimers(struct tm* timeinfo) {
  if (!timeinfo) return;
  
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
      Serial.printf("🕐 %02d:%02d - %d timers actifs\n", 
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
      Serial.println("⚠️ Électrolyseur arrêté automatiquement (pompe arrêtée)");
      if (mqttClient.connected()) {
        mqttClient.publish((mqttTopic + "/relay/1/state").c_str(), "0");
      }
    }
  }
  lastPompeState = currentPompeState;
  
  for (int i = 0; i < flexTimerCount; i++) {
    FlexibleTimer* timer = &flexTimers[i];
    
    // Timer désactivé
    if (!timer->enabled) {
      if (timer->context.state != TIMER_IDLE) {
        // Arrêter tous les relais de ce timer
        for (int a = 0; a < timer->actionCount; a++) {
          if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
            digitalWrite(relayPins[timer->actions[a].relay], LOW);
          }
        }
        timer->context.state = TIMER_IDLE;
        Serial.printf("⏸️  Timer %d désactivé et arrêté\n", timer->id);
      }
      continue;
    }
    
    // Vérifier jour d'activation
    if (!timer->days[timeinfo->tm_wday]) {
      if (timer->context.state != TIMER_IDLE) {
        timer->context.state = TIMER_IDLE;
      }
      continue;
    }
    
    // Arrêt d'urgence si fuite
    if (waterLeak && timer->context.state == TIMER_RUNNING) {
      Serial.printf("🚨 Timer %d arrêté - FUITE DÉTECTÉE\n", timer->id);
      for (int a = 0; a < timer->actionCount; a++) {
        if (timer->actions[a].type == ACTION_RELAY && timer->actions[a].state) {
          digitalWrite(relayPins[timer->actions[a].relay], LOW);
        }
      }
      timer->context.state = TIMER_ERROR;
      timer->context.lastError = "Fuite détectée";
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
            break;
            
          case START_SUNRISE:
            startMinutes = 7 * 60 + timer->startTime.sunriseOffset;
            shouldStart = (currentMinutes >= startMinutes && 
                          timer->lastTriggeredDay != currentDayOfYear);
            break;
            
          case START_SUNSET:
            startMinutes = 20 * 60 + timer->startTime.sunriseOffset;
            shouldStart = (currentMinutes >= startMinutes && 
                          timer->lastTriggeredDay != currentDayOfYear);
            break;
        }
        
        if (shouldStart) {
          // Vérifier conditions
          bool conditionsOK = true;
          for (int c = 0; c < timer->conditionCount; c++) {
            bool met = false;
            Condition* cond = &timer->conditions[c];
            
            switch(cond->type) {
              case CONDITION_COVER_OPEN:
                met = coverOpen;
                break;
              case CONDITION_COVER_CLOSED:
                met = !coverOpen;
                break;
              case CONDITION_TEMP_MIN:
                met = (waterTemp >= cond->value);
                break;
              case CONDITION_TEMP_MAX:
                met = (waterTemp <= cond->value);
                break;
              case CONDITION_TEMP_EXT_MIN:
                met = (tempExterieure >= cond->value);
                break;
              case CONDITION_TEMP_EXT_MAX:
                met = (tempExterieure <= cond->value);
                break;
              case CONDITION_PRESSURE_MIN:
                met = (waterPressure >= cond->value);
                break;
              case CONDITION_PRESSURE_MAX:
                met = (waterPressure <= cond->value);
                break;
              case CONDITION_NO_LEAK:
                met = !waterLeak;
                break;
            }
            
            if (cond->required && !met) {
              conditionsOK = false;
              break;
            }
          }
          
          if (conditionsOK) {
            Serial.printf("▶️  Timer %d '%s' démarré\n", timer->id, timer->name.c_str());
            timer->context.state = TIMER_RUNNING;
            timer->context.timerStartMillis = nowMillis;
            timer->context.currentActionIndex = 0;
            timer->context.actionStartMillis = nowMillis;
            timer->context.tempMeasured = false;
            timer->lastTriggeredDay = currentDayOfYear;
          } else {
            Serial.printf("⏸️  Timer %d en attente - conditions non remplies\n", timer->id);
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
            // Vérifier si le timer va redémarrer immédiatement
            bool willRestart = willTimerRestartImmediately(
              timer, timeinfo, currentDayOfYear, currentMinutes,
              waterTemp, waterPressure, tempExterieure, coverOpen, waterLeak
            );
            
            if (willRestart) {
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
                
                Serial.printf("🔄 Timer %d: Cycle 24h détecté (%.1fh), redémarrage prévu demain\n",
                            timer->id, timer->context.calculatedDurationHours);
                Serial.printf("⭐️  Timer %d: Action %d (%s OFF) sautée pour continuité\n",
                            timer->id, timer->context.currentActionIndex + 1, 
                            relayNames[currentAction->relay]);
                
                // Sauter l'action
                timer->context.currentActionIndex++;
                timer->context.actionStartMillis = nowMillis;
                
                if (timer->context.currentActionIndex >= timer->actionCount) {
                  // Toutes actions terminées
                  Serial.printf("✅ Timer %d terminé\n", timer->id);
                  Serial.printf("🔄 Relais maintenus actifs pour cycle continu\n");
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
          
          Serial.printf("✅ Timer %d terminé\n", timer->id);
          
          if (willRestartImmediately) {
            Serial.printf("🔄 Timer %d va redémarrer immédiatement\n", timer->id);
            timer->context.state = TIMER_COMPLETED;
            timer->context.currentActionIndex = 0;
            
          } else {
            Serial.printf("⏸️  Timer %d arrêté - relais désactivés\n", timer->id);
            timer->context.state = TIMER_COMPLETED;
            
            // Arrêter tous les relais
            for (int a = 0; a < timer->actionCount; a++) {
              if (timer->actions[a].type == ACTION_RELAY) {
                digitalWrite(relayPins[timer->actions[a].relay], LOW);
                Serial.printf("  ↳ Relais %d OFF\n", timer->actions[a].relay);
              }
            }
            
            timer->context.currentActionIndex = 0;
          }
          
          break;
        }
        
        // ┌────────────────────────────────────────────────────────────────┐
        // │ Exécuter l'action courante                                     │
        // └────────────────────────────────────────────────────────────────┘
        Action* action = &timer->actions[timer->context.currentActionIndex];
        unsigned long actionElapsed = nowMillis - timer->context.actionStartMillis;
        
        // Gérer le délai
        if (action->delayMinutes > 0) {
          unsigned long delayMillis = action->delayMinutes * 60000UL;
          if (actionElapsed < delayMillis) {
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
                timer->context.lastError = "Pompe doit être active";
                timer->context.state = TIMER_ERROR;
                break;
              }
            }
            digitalWrite(relayPins[action->relay], action->state ? HIGH : LOW);
            Serial.printf("✓ Timer %d: Relais %d → %s\n", 
                         timer->id, action->relay, action->state ? "ON" : "OFF");
            actionComplete = true;
            break;
            
          case ACTION_WAIT_DURATION:
            if (actionElapsed >= (unsigned long)action->delayMinutes * 60000UL) {
              Serial.printf("✓ Timer %d: Attente terminée\n", timer->id);
              actionComplete = true;
            }
            break;
            
          case ACTION_MEASURE_TEMP:
          {
            // Vérifier pompe active
            if (digitalRead(relayPins[0]) != HIGH) {
              Serial.printf("⚠️ Timer %d: Démarrage pompe pour mesure\n", timer->id);
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
                Serial.printf("📊 Timer %d: Mesure 1/3 (5 min) = %.2f°C\n", 
                            timer->id, timer->context.measuredTemp1);
              }
              break;
            }
            
            // Mesure 2 à 10 minutes
            if (timer->context.tempMeasureCount == 1) {
              if (pumpTime >= 600000UL) {
                timer->context.measuredTemp2 = waterTemp;
                timer->context.tempMeasureCount = 2;
                Serial.printf("📊 Timer %d: Mesure 2/3 (10 min) = %.2f°C\n", 
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
                
                Serial.printf("📊 Timer %d: ┌──────────────────────────────┐\n", timer->id);
                Serial.printf("📊 Timer %d: Mesure 1 (5 min)  = %.2f°C\n", timer->id, timer->context.measuredTemp1);
                Serial.printf("📊 Timer %d: Mesure 2 (10 min) = %.2f°C\n", timer->id, timer->context.measuredTemp2);
                Serial.printf("📊 Timer %d: Mesure 3 (15 min) = %.2f°C\n", timer->id, timer->context.measuredTemp3);
                Serial.printf("📊 Timer %d: MOYENNE = %.2f°C ✅\n", timer->id, timer->context.measuredTempAvg);
                Serial.printf("📊 Timer %d: └──────────────────────────────┘\n", timer->id);
                
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
                    Serial.printf("❌ Timer %d: Erreur équation '%s'\n",
                                timer->id, action->customEquation.expression.c_str());
                    timer->context.lastError = "Erreur dans l'équation personnalisée";
                    timer->context.state = TIMER_ERROR;
                    break;
                  }
                  
                  Serial.printf("✅ Timer %d: Équation '%s' = %.2fh\n",
                              timer->id, action->customEquation.expression.c_str(), durationHours);
                  Serial.printf("   Variables: waterTemp=%.2f, extTemp=%.2f, max=%.2f, min=%.2f, sun=%.0f%%\n",
                              wTemp, eTemp, wMax, wMin, sun);
                  
                } else {
                  // Formule par défaut
                  durationHours = timer->context.measuredTempAvg / 2.0;
                  Serial.printf("✅ Timer %d: Durée auto (défaut) = %.1fh\n",
                              timer->id, durationHours);
                }
                
                // Limiter entre 3h et 24h
                if (durationHours < 3.0) {
                  Serial.printf("⚠️ Timer %d: Durée %.2fh < 3h, ajusté à 3h\n", 
                              timer->id, durationHours);
                  durationHours = 3.0;
                }
                
                if (durationHours > 24.0) {
                  Serial.printf("⚠️ Timer %d: Durée %.2fh > 24h, ajusté à 24h\n",
                              timer->id, durationHours);
                  durationHours = 24.0;
                }
                
                action->maxWaitMinutes = (int)(durationHours * 60);
                timer->context.actionStartMillis = nowMillis;
                
                Serial.printf("⏱️ Timer %d: Durée finale = %.1fh (%d minutes)\n",
                            timer->id, durationHours, action->maxWaitMinutes);

                timer->context.calculatedDurationHours = durationHours;
              }
              
              // Vérifier si la durée est écoulée
              unsigned long targetMillis = (unsigned long)action->maxWaitMinutes * 60000UL;
              
              if (actionElapsed >= targetMillis) {
                timer->context.totalElapsedMinutes += action->maxWaitMinutes;
                
                Serial.printf("✅ Timer %d: Action durée auto terminée (total: %lu min)\n",
                            timer->id, timer->context.totalElapsedMinutes);
                
                actionComplete = true;
              } else {
                // Affichage périodique de la progression
                static unsigned long lastProgressLog = 0;
                if (millis() - lastProgressLog > 300000) {
                  float progressPercent = (actionElapsed / (float)targetMillis) * 100.0;
                  unsigned long remainingMin = (targetMillis - actionElapsed) / 60000;
                  
                  Serial.printf("⏳ Timer %d: Progression %.1f%% - Reste %lu min\n",
                              timer->id, progressPercent, remainingMin);
                  lastProgressLog = millis();
                }
              }
            } else {
              static unsigned long lastTempWaitLog = 0;
              if (millis() - lastTempWaitLog > 60000) {
                Serial.printf("⏳ Timer %d: En attente de mesure température...\n", timer->id);
                lastTempWaitLog = millis();
              }
            }
            break;
          
          case ACTION_BUZZER:
            if (!buzzerMuted && sysConfig.buzzerEnabled) {
              if (action->buzzerCount == 0) {
                buzzerAlarm();
                Serial.printf("📊 Timer %d: Buzzer ALARME\n", timer->id);
              } else {
                buzzerBeep(action->buzzerCount);
                Serial.printf("📊 Timer %d: Buzzer %d bip(s)\n", 
                             timer->id, action->buzzerCount);
              }
            } else {
              Serial.printf("🔇 Timer %d: Buzzer désactivé\n", timer->id);
            }
            actionComplete = true;
            break;
            
          case ACTION_LED:
          {
            CRGB color;
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
            
            Serial.printf("💡 Timer %d: LED couleur=%d mode=%d durée=%ds\n",
                         timer->id, action->ledColor, action->ledMode, action->ledDuration);
            
            actionComplete = true;
            break;
          }
        }
        
        if (actionComplete) {
          timer->context.currentActionIndex++;
          timer->context.actionStartMillis = nowMillis;
          
          if (timer->context.currentActionIndex < timer->actionCount) {
            Serial.printf("➡️  Timer %d: Action %d/%d\n",
                         timer->id, timer->context.currentActionIndex + 1, 
                         timer->actionCount);
          }
        }
        
        break;
      }
      
      case TIMER_COMPLETED:
      {
        if (timer->lastTriggeredDay != currentDayOfYear) {
          timer->context.state = TIMER_IDLE;
        }
        break;
      }
      
      case TIMER_ERROR:
      {
        if (timer->lastTriggeredDay != currentDayOfYear) {
          Serial.printf("🔄 Timer %d: Reset erreur\n", timer->id);
          timer->context.state = TIMER_IDLE;
          timer->context.lastError = "";
        }
        break;
      }
    }
  }
}

#endif // TIMER_PROCESSOR_H
