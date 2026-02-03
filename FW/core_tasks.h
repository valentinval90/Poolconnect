/* 
 * POOL CONNECT - CORE 1 TASK
 * Gestion du Core 1 (capteurs, timers, MQTT, météo)
 * core_tasks.h   V0.2
 */

#ifndef CORE_TASKS_H
#define CORE_TASKS_H

#include <Arduino.h>
#include "globals.h"
#include "config.h"
#include "logging.h"
#include "sensors.h"
#include "mqtt_manager.h"
#include "weather.h"
#include "timer_processor.h"
#include "led_buzzer.h"

// ============================================================================
// CORE 1 TASK - Traitement des capteurs, timers et MQTT
// ============================================================================

void core1Task(void *parameter) {
  LOG_SEPARATOR();
  LOG_I(LOG_SYSTEM, "Core 1 Task demarre avec succes");
  LOG_I(LOG_SYSTEM, "Responsabilites: Capteurs, Timers, MQTT, Meteo, LED");
  LOG_V(LOG_SYSTEM, "Intervalle capteurs: 10s");
  LOG_V(LOG_SYSTEM, "Intervalle MQTT publish: 10s");
  LOG_V(LOG_SYSTEM, "Intervalle meteo: %lu ms", WEATHER_UPDATE_INTERVAL);
  LOG_SEPARATOR();
  
  unsigned long loopCount = 0;
  unsigned long lastLogTime = 0;
  
  while(true) {
    loopCount++;
    
    // Log périodique de l'activité du Core 1 (toutes les 60 secondes)
    if (millis() - lastLogTime > 60000) {
      LOG_I(LOG_SYSTEM, "Core 1 actif - Iterations: %lu", loopCount);
      LOG_MEMORY();
      lastLogTime = millis();
      loopCount = 0;
    }
    
    // ========================================================================
    // LECTURE CAPTEURS - Toutes les 10 secondes
    // ========================================================================
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead > 10000) {
      LOG_V(LOG_SENSOR, "Declenchement de la lecture periodique des capteurs");
      readSensors();
      // ============================================================================
      // AJOUT POINT AU GRAPHIQUE
      // ============================================================================
      
      // Collecter les états des relais
      bool relayStates[5] = {
        digitalRead(RELAY_POMPE),
        digitalRead(RELAY_ELECTROLYSEUR),
        digitalRead(RELAY_LAMPE),
        digitalRead(RELAY_ELECTROVALVE),
        digitalRead(RELAY_PAC)
      };
      
      // Compter les timers actifs
      uint8_t activeTimersCount = 0;
      for (int i = 0; i < flexTimerCount; i++) {
        if (flexTimers[i].enabled && flexTimers[i].context.state == TIMER_RUNNING) {
          activeTimersCount++;
        }
      }
      
      // Ajouter le point de données
      addChartPoint(waterTemp, waterPressure, relayStates, 
                    coverOpen, activeTimersCount);
      lastSensorRead = millis();
      LOG_V(LOG_SENSOR, "Prochaine lecture dans 10s");
    }
    
    // ========================================================================
    // TRAITEMENT TIMERS
    // ========================================================================
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      static int lastMinute = -1;
      int currentMinute = timeinfo.tm_min;
      
      // Log uniquement quand on change de minute pour éviter le spam
      if (currentMinute != lastMinute) {
        LOG_V(LOG_TIMER, "Traitement des timers - %02d:%02d", 
              timeinfo.tm_hour, timeinfo.tm_min);
        lastMinute = currentMinute;
      }
      
      processFlexTimers(&timeinfo);
    } else {
      static unsigned long lastNtpWarning = 0;
      if (millis() - lastNtpWarning > 300000) { // Warning toutes les 5 min
        LOG_W(LOG_SYSTEM, "NTP non synchronise - Les timers ne peuvent pas fonctionner");
        lastNtpWarning = millis();
      }
    }
    
    // ========================================================================
    // MQTT - Reconnexion et publication
    // ========================================================================
    mqttReconnect();
    
    if (mqttClient.connected()) {
      mqttClient.loop();
      
      static unsigned long lastMqttPublish = 0;
      if (millis() - lastMqttPublish > 10000) {
        LOG_V(LOG_MQTT, "Publication periodique des etats des capteurs");
        publishSensorStates();
        lastMqttPublish = millis();
        LOG_V(LOG_MQTT, "Prochaine publication dans 10s");
      }
    } else {
      static unsigned long lastMqttDisconnectLog = 0;
      if (millis() - lastMqttDisconnectLog > 60000) { // Log toutes les 60s si déconnecté
        LOG_W(LOG_MQTT, "Client MQTT deconnecte - Les donnees ne sont pas publiees");
        lastMqttDisconnectLog = millis();
      }
    }
    
    // ========================================================================
    // MÉTÉO - Mise à jour périodique
    // ========================================================================
    static unsigned long lastWeather = 0;
    if (millis() - lastWeather > WEATHER_UPDATE_INTERVAL) {
      LOG_D(LOG_WEATHER, "Declenchement de la mise a jour meteo");
      updateWeatherData();
      lastWeather = millis();
      LOG_V(LOG_WEATHER, "Prochaine mise a jour dans %lu ms", WEATHER_UPDATE_INTERVAL);
    }
    
    // ========================================================================
    // LED - Activité si pas d'alarme
    // ========================================================================
    static bool lastLeakState = false;
    static bool lastPressureAlarmState = false;
    bool currentPressureAlarm = (waterPressure > pressureThreshold);
    
    // Log les changements d'état d'alarme
    if (waterLeak != lastLeakState) {
      if (waterLeak) {
        LOG_E(LOG_SYSTEM, "ALARME: Fuite d'eau detectee!");
      } else {
        LOG_I(LOG_SYSTEM, "Alarme fuite desactivee");
      }
      lastLeakState = waterLeak;
    }
    
    if (currentPressureAlarm != lastPressureAlarmState) {
      if (currentPressureAlarm) {
        LOG_W(LOG_SYSTEM, "ALARME: Pression elevee detectee (%.2f BAR > %.2f BAR)", 
              waterPressure, pressureThreshold);
      } else {
        LOG_I(LOG_SYSTEM, "Alarme pression desactivee");
      }
      lastPressureAlarmState = currentPressureAlarm;
    }
    
    // Activité LED normale si pas d'alarme
    if (!waterLeak && !currentPressureAlarm) {
      ledActivity();
    }
    
    delay(10);
  }
}

#endif // CORE_TASKS_H