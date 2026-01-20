/* 
 * POOL CONNECT - SENSORS
 * Gestion des capteurs et calibration
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Wire.h>
#include "globals.h"
#include "led_buzzer.h"

// ============================================================================
// CALIBRATION
// ============================================================================

float applyCalibratedTemp(float rawTemp) {
  if (!calibConfig.tempUseCalibration) return rawTemp;
  
  if (calibConfig.tempUseTwoPoint) {
    // Calibration 2 points : y = ax + b
    float denominator = calibConfig.tempPoint2Raw - calibConfig.tempPoint1Raw;
    
    // ✅ PROTECTION : Division par zéro
    if (abs(denominator) < 0.01) {
      Serial.println("⚠️ Calibration Temp: points identiques, fallback offset");
      return rawTemp + calibConfig.tempOffset;
    }
    
    float slope = (calibConfig.tempPoint2Real - calibConfig.tempPoint1Real) / denominator;
    float intercept = calibConfig.tempPoint1Real - slope * calibConfig.tempPoint1Raw;
    return slope * rawTemp + intercept;
  } else {
    // Offset simple
    return rawTemp + calibConfig.tempOffset;
  }
}

float applyCalibratedPressure(float rawPressure) {
  if (!calibConfig.pressureUseCalibration) return rawPressure;
  
  if (calibConfig.pressureUseTwoPoint) {
    // Calibration 2 points
    float denominator = calibConfig.pressurePoint2Raw - calibConfig.pressurePoint1Raw;
    
    // ✅ PROTECTION : Division par zéro
    if (abs(denominator) < 0.01) {
      Serial.println("⚠️ Calibration Pression: points identiques, fallback offset");
      return rawPressure + calibConfig.pressureOffset;
    }
    
    float slope = (calibConfig.pressurePoint2Real - calibConfig.pressurePoint1Real) / denominator;
    float intercept = calibConfig.pressurePoint1Real - slope * calibConfig.pressurePoint1Raw;
    return slope * rawPressure + intercept;
  } else {
    // Offset simple
    return rawPressure + calibConfig.pressureOffset;
  }
}

void saveCalibrationConfig() {
  File f = LittleFS.open("/calibration.json", FILE_WRITE);
  if (!f) return;
  
  DynamicJsonDocument doc(1024);
  
  JsonObject temp = doc.createNestedObject("temperature");
  temp["useCalibration"] = calibConfig.tempUseCalibration;
  temp["useTwoPoint"] = calibConfig.tempUseTwoPoint;
  temp["offset"] = calibConfig.tempOffset;
  temp["point1Raw"] = calibConfig.tempPoint1Raw;
  temp["point1Real"] = calibConfig.tempPoint1Real;
  temp["point2Raw"] = calibConfig.tempPoint2Raw;
  temp["point2Real"] = calibConfig.tempPoint2Real;
  
  JsonObject pressure = doc.createNestedObject("pressure");
  pressure["useCalibration"] = calibConfig.pressureUseCalibration;
  pressure["useTwoPoint"] = calibConfig.pressureUseTwoPoint;
  pressure["offset"] = calibConfig.pressureOffset;
  pressure["point1Raw"] = calibConfig.pressurePoint1Raw;
  pressure["point1Real"] = calibConfig.pressurePoint1Real;
  pressure["point2Raw"] = calibConfig.pressurePoint2Raw;
  pressure["point2Real"] = calibConfig.pressurePoint2Real;
  
  serializeJson(doc, f);
  f.close();
  
  Serial.println("✓ Calibration sauvegardée");
}

void loadCalibrationConfig() {
  if (!LittleFS.exists("/calibration.json")) {
    Serial.println("ℹ️  Pas de calibration - valeurs par défaut");
    return;
  }
  
  File f = LittleFS.open("/calibration.json", FILE_READ);
  if (!f) return;
  
  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (!err) {
    JsonObject temp = doc["temperature"];
    calibConfig.tempUseCalibration = temp["useCalibration"] | false;
    calibConfig.tempUseTwoPoint = temp["useTwoPoint"] | false;
    calibConfig.tempOffset = temp["offset"] | 0.0;
    calibConfig.tempPoint1Raw = temp["point1Raw"] | 10.0;
    calibConfig.tempPoint1Real = temp["point1Real"] | 10.0;
    calibConfig.tempPoint2Raw = temp["point2Raw"] | 30.0;
    calibConfig.tempPoint2Real = temp["point2Real"] | 30.0;
    
    JsonObject pressure = doc["pressure"];
    calibConfig.pressureUseCalibration = pressure["useCalibration"] | false;
    calibConfig.pressureUseTwoPoint = pressure["useTwoPoint"] | false;
    calibConfig.pressureOffset = pressure["offset"] | 0.0;
    calibConfig.pressurePoint1Raw = pressure["point1Raw"] | 1.0;
    calibConfig.pressurePoint1Real = pressure["point1Real"] | 1.0;
    calibConfig.pressurePoint2Raw = pressure["point2Raw"] | 3.0;
    calibConfig.pressurePoint2Real = pressure["point2Real"] | 3.0;
    
    Serial.println("✓ Calibration chargée");
    Serial.printf("  Temp: %s (%s)\n", 
                  calibConfig.tempUseCalibration ? "ON" : "OFF",
                  calibConfig.tempUseTwoPoint ? "2pts" : "offset");
    Serial.printf("  Pression: %s (%s)\n",
                  calibConfig.pressureUseCalibration ? "ON" : "OFF",
                  calibConfig.pressureUseTwoPoint ? "2pts" : "offset");
  }
}

// ============================================================================
// LECTURE CAPTEURS
// ============================================================================

void readSensors() {
  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
    // ✅ Température DS18B20 AVEC CALIBRATION
    sensors.requestTemperatures();
    float rawTemp = sensors.getTempCByIndex(0);
    waterTemp = applyCalibratedTemp(rawTemp);
    
    // ✅ Pression INA226 AVEC CALIBRATION
    static unsigned long lastPressureLog = 0;
    bool logPressure = (millis() - lastPressureLog > 10000);
    
    float current = ina226.getCurrent_mA();
    float rawPressure = 0.0;
    
    if (current >= 4.0 && current <= 20.0) {
      rawPressure = ((current - 4.0) / 16.0) * 10.0;
      waterPressure = applyCalibratedPressure(rawPressure);
      
      if (logPressure) {
        Serial.printf("💧 Pressure: %.2f bar (raw: %.2f, from %.2f mA)\n", 
                     waterPressure, rawPressure, current);
      }
    } else {
      if (logPressure) {
        if (current < 4.0) {
          Serial.printf("⚠️ Current %.2f mA < 4 mA\n", current);
        } else {
          Serial.printf("⚠️ Current %.2f mA > 20 mA\n", current);
        }
      }
      waterPressure = 0.0;
    }
    
    if (logPressure) {
      lastPressureLog = millis();
    }
    
    // ========================================================================
    // FUITE
    // ========================================================================
    static unsigned long lastLeakLog = 0;
    static bool lastLeakState = false;
    static int leakChangeCount = 0;
    
    // CORRECTION : Lecture digitale simple
    bool currentLeakState = (digitalRead(SENSOR_FUITE) == HIGH);
    
    // Log toutes les 30 secondes OU si changement d'état
    bool shouldLogLeak = (millis() - lastLeakLog > 30000) || (currentLeakState != lastLeakState);
    
    if (shouldLogLeak) {
      Serial.printf("💧 FUITE - État: %s\n",
                   currentLeakState ? "⚠️ FUITE DÉTECTÉE" : "✓ OK");
      
      // Si changement d'état
      if (currentLeakState != lastLeakState) {
        leakChangeCount++;
        
        if (currentLeakState) {
          Serial.println("🚨 ========================================");
          Serial.println("🚨 ALERTE FUITE D'EAU DÉTECTÉE !");
          Serial.println("🚨 ========================================");
          Serial.printf("🚨 Changement #%d détecté\n", leakChangeCount);
          
          // Log état des relais
          Serial.println("📊 État des relais:");
          const char* relayNames[] = {"Pompe", "Électrolyseur", "Lampe", "Électrovalve", "PAC"};
          for (int i = 0; i < NUM_RELAYS; i++) {
            bool state = digitalRead(relayPins[i]) == HIGH;
            Serial.printf("  %s: %s\n", relayNames[i], state ? "ON" : "OFF");
          }
        } else {
          Serial.println("✅ ========================================");
          Serial.println("✅ FIN D'ALERTE - Plus de fuite détectée");
          Serial.println("✅ ========================================");
        }
      }
      
      lastLeakLog = millis();
      lastLeakState = currentLeakState;
    }
    
    waterLeak = currentLeakState;
    
    // Volet
    coverOpen = (digitalRead(SENSOR_VOLET) == LOW);
    
    xSemaphoreGive(dataMutex);
  }
  
  // Alarmes
  if (waterLeak) {
    setLEDStatus(LED_ALARM);
    buzzerAlarm();
  }
  
  if (waterPressure > pressureThreshold) {
    setLEDStatus(LED_ALARM);
    buzzerAlarm();
  }
}

#endif // SENSORS_H
