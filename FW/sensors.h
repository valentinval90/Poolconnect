/* 
 * POOL CONNECT - SENSORS
 * Gestion des capteurs et calibration
 * sensor.h   V0.2
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Wire.h>
#include "globals.h"
#include "config.h"
#include "logging.h"
#include "led_buzzer.h"
#include "chart_storage.h"
#include "chart_event_points.h"

// ============================================================================
// CALIBRATION
// ============================================================================

float applyCalibratedTemp(float rawTemp) {
  if (!calibConfig.tempUseCalibration) {
    LOG_V(LOG_SENSOR, "Temperature: Calibration desactivee, valeur brute retournee");
    return rawTemp;
  }
  
  if (calibConfig.tempUseTwoPoint) {
    LOG_V(LOG_SENSOR, "Temperature: Application calibration 2 points");
    
    // Calibration 2 points : y = ax + b
    float denominator = calibConfig.tempPoint2Raw - calibConfig.tempPoint1Raw;
    
    // Protection : Division par zéro
    if (abs(denominator) < 0.01) {
      LOG_W(LOG_SENSOR, "Calibration Temp: points identiques (%.2f = %.2f), fallback offset", 
            calibConfig.tempPoint1Raw, calibConfig.tempPoint2Raw);
      float result = rawTemp + calibConfig.tempOffset;
      LOG_V(LOG_SENSOR, "Temp calibree (offset): %.2f C (brute: %.2f C, offset: %.2f)", 
            result, rawTemp, calibConfig.tempOffset);
      return result;
    }
    
    float slope = (calibConfig.tempPoint2Real - calibConfig.tempPoint1Real) / denominator;
    float intercept = calibConfig.tempPoint1Real - slope * calibConfig.tempPoint1Raw;
    float result = slope * rawTemp + intercept;
    
    LOG_V(LOG_SENSOR, "Temp calibree (2pts): %.2f C (brute: %.2f C, slope: %.4f, intercept: %.2f)", 
          result, rawTemp, slope, intercept);
    
    return result;
  } else {
    LOG_V(LOG_SENSOR, "Temperature: Application offset simple");
    float result = rawTemp + calibConfig.tempOffset;
    LOG_V(LOG_SENSOR, "Temp calibree (offset): %.2f C (brute: %.2f C, offset: %.2f)", 
          result, rawTemp, calibConfig.tempOffset);
    return result;
  }
}

float applyCalibratedPressure(float rawPressure) {
  if (!calibConfig.pressureUseCalibration) {
    LOG_V(LOG_SENSOR, "Pression: Calibration desactivee, valeur brute retournee");
    return rawPressure;
  }
  
  if (calibConfig.pressureUseTwoPoint) {
    LOG_V(LOG_SENSOR, "Pression: Application calibration 2 points");
    
    // Calibration 2 points
    float denominator = calibConfig.pressurePoint2Raw - calibConfig.pressurePoint1Raw;
    
    // Protection : Division par zéro
    if (abs(denominator) < 0.01) {
      LOG_W(LOG_SENSOR, "Calibration Pression: points identiques (%.2f = %.2f), fallback offset",
            calibConfig.pressurePoint1Raw, calibConfig.pressurePoint2Raw);
      float result = rawPressure + calibConfig.pressureOffset;
      LOG_V(LOG_SENSOR, "Pression calibree (offset): %.2f BAR (brute: %.2f BAR, offset: %.2f)", 
            result, rawPressure, calibConfig.pressureOffset);
      return result;
    }
    
    float slope = (calibConfig.pressurePoint2Real - calibConfig.pressurePoint1Real) / denominator;
    float intercept = calibConfig.pressurePoint1Real - slope * calibConfig.pressurePoint1Raw;
    float result = slope * rawPressure + intercept;
    
    LOG_V(LOG_SENSOR, "Pression calibree (2pts): %.2f BAR (brute: %.2f BAR, slope: %.4f, intercept: %.2f)", 
          result, rawPressure, slope, intercept);
    
    return result;
  } else {
    LOG_V(LOG_SENSOR, "Pression: Application offset simple");
    float result = rawPressure + calibConfig.pressureOffset;
    LOG_V(LOG_SENSOR, "Pression calibree (offset): %.2f BAR (brute: %.2f BAR, offset: %.2f)", 
          result, rawPressure, calibConfig.pressureOffset);
    return result;
  }
}

void saveCalibrationConfig() {
  LOG_D(LOG_STORAGE, "Sauvegarde de la configuration de calibration...");
  
  File f = LittleFS.open("/calibration.json", FILE_WRITE);
  if (!f) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /calibration.json en ecriture");
    LOG_STORAGE_OP("WRITE", "/calibration.json", false);
    return;
  }
  
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
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_STORAGE, "Calibration sauvegardee avec succes (%d bytes)", bytesWritten);
  LOG_V(LOG_STORAGE, "Temp: %s (%s), Pression: %s (%s)",
        calibConfig.tempUseCalibration ? "ON" : "OFF",
        calibConfig.tempUseTwoPoint ? "2pts" : "offset",
        calibConfig.pressureUseCalibration ? "ON" : "OFF",
        calibConfig.pressureUseTwoPoint ? "2pts" : "offset");
  LOG_STORAGE_OP("WRITE", "/calibration.json", true);
}

void loadCalibrationConfig() {
  LOG_D(LOG_STORAGE, "Chargement de la configuration de calibration...");
  
  if (!LittleFS.exists("/calibration.json")) {
    LOG_W(LOG_STORAGE, "Fichier /calibration.json non trouve - Valeurs par defaut utilisees");
    LOG_I(LOG_STORAGE, "Calibration: Temp=OFF, Pression=OFF");
    return;
  }
  
  File f = LittleFS.open("/calibration.json", FILE_READ);
  if (!f) {
    LOG_E(LOG_STORAGE, "Erreur ouverture /calibration.json en lecture");
    LOG_STORAGE_OP("READ", "/calibration.json", false);
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    LOG_E(LOG_STORAGE, "Erreur parsing JSON calibration: %s", err.c_str());
    LOG_STORAGE_OP("READ", "/calibration.json", false);
    return;
  }
  
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
  
  LOG_I(LOG_STORAGE, "Calibration chargee avec succes");
  LOG_I(LOG_STORAGE, "Temperature: %s (%s%s)",
        calibConfig.tempUseCalibration ? "Active" : "Inactive",
        calibConfig.tempUseTwoPoint ? "2 points" : "offset",
        calibConfig.tempUseCalibration ? "" : " - N/A");
  
  if (calibConfig.tempUseCalibration && calibConfig.tempUseTwoPoint) {
    LOG_V(LOG_STORAGE, "  Point 1: %.2f C (brut) -> %.2f C (reel)", 
          calibConfig.tempPoint1Raw, calibConfig.tempPoint1Real);
    LOG_V(LOG_STORAGE, "  Point 2: %.2f C (brut) -> %.2f C (reel)", 
          calibConfig.tempPoint2Raw, calibConfig.tempPoint2Real);
  } else if (calibConfig.tempUseCalibration) {
    LOG_V(LOG_STORAGE, "  Offset: %.2f C", calibConfig.tempOffset);
  }
  
  LOG_I(LOG_STORAGE, "Pression: %s (%s%s)",
        calibConfig.pressureUseCalibration ? "Active" : "Inactive",
        calibConfig.pressureUseTwoPoint ? "2 points" : "offset",
        calibConfig.pressureUseCalibration ? "" : " - N/A");
  
  if (calibConfig.pressureUseCalibration && calibConfig.pressureUseTwoPoint) {
    LOG_V(LOG_STORAGE, "  Point 1: %.2f BAR (brut) -> %.2f BAR (reel)", 
          calibConfig.pressurePoint1Raw, calibConfig.pressurePoint1Real);
    LOG_V(LOG_STORAGE, "  Point 2: %.2f BAR (brut) -> %.2f BAR (reel)", 
          calibConfig.pressurePoint2Raw, calibConfig.pressurePoint2Real);
  } else if (calibConfig.pressureUseCalibration) {
    LOG_V(LOG_STORAGE, "  Offset: %.2f BAR", calibConfig.pressureOffset);
  }
  
  LOG_STORAGE_OP("READ", "/calibration.json", true);
}

// ============================================================================
// LECTURE CAPTEURS
// ============================================================================

void readSensors() {
  LOG_V(LOG_SENSOR, "Debut de la lecture des capteurs...");
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
    // ========================================================================
    // TEMPÉRATURE DS18B20 AVEC CALIBRATION
    // ========================================================================
    LOG_V(LOG_SENSOR, "Lecture capteur DS18B20 (Temperature)...");
    sensors.requestTemperatures();
    float rawTemp = sensors.getTempCByIndex(0);
    
    if (rawTemp == DEVICE_DISCONNECTED_C) {
      LOG_E(LOG_SENSOR, "DS18B20 deconnecte ou erreur de lecture");
      waterTemp = 0.0;
    } else {
      waterTemp = applyCalibratedTemp(rawTemp);
      LOG_SENSOR_READ("Temperature eau", waterTemp, "C");
    }
    
    // ========================================================================
    // PRESSION INA226 AVEC CALIBRATION
    // ========================================================================
    static unsigned long lastPressureLog = 0;
    bool logPressure = (millis() - lastPressureLog > 10000);
    
    if (logPressure) {
      LOG_V(LOG_SENSOR, "Lecture capteur INA226 (Pression 4-20mA)...");
    }
    
    float current = ina226.getCurrent_mA();
    float rawPressure = 0.0;
    
    if (current >= 4.0 && current <= 20.0) {
      // Conversion 4-20mA vers 0-10 BAR
      rawPressure = ((current - 4.0) / 16.0) * 10.0;
      waterPressure = applyCalibratedPressure(rawPressure);
      
      if (logPressure) {
        LOG_SENSOR_READ("Pression eau", waterPressure, "BAR");
        LOG_V(LOG_SENSOR, "Pression brute: %.2f BAR, Courant: %.2f mA", rawPressure, current);
      }
    } else {
      if (logPressure) {
        if (current < 4.0) {
          LOG_W(LOG_SENSOR, "Courant pression hors plage: %.2f mA < 4.0 mA (Capteur deconnecte?)", current);
        } else {
          LOG_E(LOG_SENSOR, "Courant pression hors plage: %.2f mA > 20.0 mA (Surintensité!)", current);
        }
      }
      waterPressure = 0.0;
    }
    
    if (logPressure) {
      lastPressureLog = millis();
    }
    
    // ========================================================================
    // DÉTECTION DE FUITE
    // ========================================================================
    static unsigned long lastLeakLog = 0;
    static bool lastLeakState = false;
    static int leakChangeCount = 0;
    
    // Lecture digitale simple
    bool currentLeakState = (digitalRead(SENSOR_FUITE) == HIGH);
    
    // Log toutes les 30 secondes OU si changement d'état
    bool shouldLogLeak = (millis() - lastLeakLog > 30000) || (currentLeakState != lastLeakState);
    
    if (shouldLogLeak) {
      if (currentLeakState) {
        LOG_W(LOG_SENSOR, "Capteur de fuite: FUITE DETECTEE!");
      } else {
        LOG_I(LOG_SENSOR, "Capteur de fuite: OK (pas de fuite)");
      }
      
      // Si changement d'état
      if (currentLeakState != lastLeakState) {
        leakChangeCount++;
        
        if (currentLeakState) {
          LOG_SEPARATOR();
          LOG_E(LOG_SENSOR, "========================================");
          LOG_E(LOG_SENSOR, "   ALERTE FUITE D'EAU DETECTEE !");
          LOG_E(LOG_SENSOR, "========================================");
          LOG_E(LOG_SENSOR, "Changement d'etat #%d detecte", leakChangeCount);
          
          // Log état des relais
          LOG_I(LOG_SENSOR, "Etat actuel des relais:");
          const char* relayNames[] = {"Pompe", "Electrolyseur", "Lampe", "Electrovalve", "PAC"};
          for (int i = 0; i < NUM_RELAYS; i++) {
            bool state = digitalRead(relayPins[i]) == HIGH;
            LOG_I(LOG_SENSOR, "  %s: %s", relayNames[i], state ? "ON" : "OFF");
          }

          //Capturer sur le graphique
          captureCurrentStateToChart();

          LOG_SEPARATOR();
        } else {
          LOG_SEPARATOR();
          LOG_I(LOG_SENSOR, "========================================");
          LOG_I(LOG_SENSOR, "   FIN D'ALERTE - Plus de fuite");
          LOG_I(LOG_SENSOR, "========================================");
          LOG_SEPARATOR();
          
          //Capturer sur le graphique
          captureCurrentStateToChart();
        }
      }
      
      lastLeakLog = millis();
      lastLeakState = currentLeakState;
    }
    
    waterLeak = currentLeakState;
    
    // ========================================================================
    // DÉTECTION VOLET
    // ========================================================================
    static unsigned long lastCoverLog = 0;
    static bool lastCoverState = false;
    
    coverOpen = (digitalRead(SENSOR_VOLET) == LOW);
    
    // Log uniquement si changement d'état
    if (coverOpen != lastCoverState || (millis() - lastCoverLog > 60000)) {
      LOG_I(LOG_SENSOR, "Volet piscine: %s", coverOpen ? "OUVERT" : "FERME");
      lastCoverLog = millis();
      lastCoverState = coverOpen;

      //Capturer sur le graphique
      captureCurrentStateToChart();
    }
    
    xSemaphoreGive(dataMutex);
    
    LOG_V(LOG_SENSOR, "Lecture des capteurs terminee");
  } else {
    LOG_E(LOG_SENSOR, "Impossible d'obtenir le mutex pour la lecture des capteurs");
  }
  
  // ========================================================================
  // GESTION DES ALARMES
  // ========================================================================
  if (waterLeak) {
    LOG_W(LOG_SYSTEM, "ALARME: Fuite d'eau - Activation LED et buzzer");
    setLEDStatus(LED_ALARM);
    buzzerAlarm();
  }
  
  if (waterPressure > pressureThreshold) {
    LOG_W(LOG_SYSTEM, "ALARME: Pression elevee (%.2f BAR > %.2f BAR) - Activation LED et buzzer", 
          waterPressure, pressureThreshold);
    setLEDStatus(LED_ALARM);
    buzzerAlarm();
  }
}

#endif // SENSORS_H