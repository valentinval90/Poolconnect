/* 
 * POOL CONNECT - WEATHER
 * Gestion des données météo
 */

#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "globals.h"

// ============================================================================
// MÉTÉO
// ============================================================================

void updateWeatherData() {
  if (weatherApiKey == "" || WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  
  // ========================================================================
  // API MÉTÉO ACTUELLE
  // ========================================================================
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + latitude +
               "&lon=" + longitude + "&appid=" + weatherApiKey + "&units=metric";
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    
    if (!deserializeJson(doc, payload)) {
      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        tempExterieure = doc["main"]["temp"].as<float>();
        
        // Récupérer les nuages pour estimer l'ensoleillement
        if (doc.containsKey("clouds")) {
          float cloudiness = doc["clouds"]["all"].as<float>(); // 0-100%
          weatherSunshine = 100.0 - cloudiness; // Inverser
        }
        
        xSemaphoreGive(dataMutex);
      }
      
      Serial.printf("☀️ Weather Now: %.2f°C, Sunshine: %.0f%%\n", 
                   tempExterieure, weatherSunshine);
    }
  } else {
    Serial.printf("⚠️ Weather API error: %d\n", httpCode);
  }
  http.end();
  
  // ========================================================================
  // API PRÉVISIONS (24h) pour MIN/MAX
  // ========================================================================
  url = "http://api.openweathermap.org/data/2.5/forecast?lat=" + latitude +
        "&lon=" + longitude + "&appid=" + weatherApiKey + "&units=metric&cnt=8";
  
  http.begin(url);
  httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(8192);
    
    if (!deserializeJson(doc, payload)) {
      JsonArray list = doc["list"];
      
      float minTemp = 999.0;
      float maxTemp = -999.0;
      
      for (JsonObject item : list) {
        float temp = item["main"]["temp"].as<float>();
        if (temp < minTemp) minTemp = temp;
        if (temp > maxTemp) maxTemp = temp;
      }
      
      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        weatherTempMin = minTemp;
        weatherTempMax = maxTemp;
        xSemaphoreGive(dataMutex);
      }
      
      Serial.printf("📊 Weather Forecast 24h: Min=%.2f°C, Max=%.2f°C\n", 
                   minTemp, maxTemp);
    }
  } else {
    Serial.printf("⚠️ Forecast API error: %d\n", httpCode);
  }
  http.end();
}

void saveWeatherConfig() {
  File f = LittleFS.open("/weather.json", FILE_WRITE);
  if (!f) return;
  StaticJsonDocument<512> doc;
  doc["apiKey"] = weatherApiKey;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;
  serializeJson(doc, f);
  f.close();
}

void loadWeatherConfig() {
  if (!LittleFS.exists("/weather.json")) return;
  File f = LittleFS.open("/weather.json", FILE_READ);
  if (!f) return;
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (!err) {
    weatherApiKey = doc["apiKey"].as<String>();
    latitude = doc["latitude"].as<String>();
    longitude = doc["longitude"].as<String>();
  }
}

#endif // WEATHER_H
