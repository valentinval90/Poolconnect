/* 
 * POOL CONNECT - CORE 1 TASK
 * Gestion du Core 1 (capteurs, timers, MQTT, météo)
 */

#ifndef CORE_TASKS_H
#define CORE_TASKS_H

#include <Arduino.h>
#include "globals.h"
#include "sensors.h"
#include "mqtt_manager.h"
#include "weather.h"
#include "timer_processor_complete.h"
#include "led_buzzer.h"

// ============================================================================
// CORE 1 TASK - Traitement des capteurs, timers et MQTT
// ============================================================================

void core1Task(void *parameter) {
  Serial.println("✓ Core 1 started");
  
  while(true) {
    // ========================================================================
    // LECTURE CAPTEURS - Toutes les 10 secondes
    // ========================================================================
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead > 10000) {
      readSensors();
      lastSensorRead = millis();
    }
    
    // ========================================================================
    // TRAITEMENT TIMERS
    // ========================================================================
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      processFlexTimers(&timeinfo);
    }
    
    // ========================================================================
    // MQTT - Reconnexion et publication
    // ========================================================================
    mqttReconnect();
    if (mqttClient.connected()) {
      mqttClient.loop();
      
      static unsigned long lastMqttPublish = 0;
      if (millis() - lastMqttPublish > 10000) {
        publishSensorStates();
        lastMqttPublish = millis();
      }
    }
    
    // ========================================================================
    // MÉTÉO - Mise à jour périodique
    // ========================================================================
    static unsigned long lastWeather = 0;
    if (millis() - lastWeather > WEATHER_UPDATE_INTERVAL) {
      updateWeatherData();
      lastWeather = millis();
    }
    
    // ========================================================================
    // LED - Activité si pas d'alarme
    // ========================================================================
    if (!waterLeak && waterPressure <= pressureThreshold) {
      ledActivity();
    }
    
    delay(10);
  }
}

#endif // CORE_TASKS_H
