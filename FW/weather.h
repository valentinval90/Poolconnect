/* 
 * POOL CONNECT - WEATHER
 * Gestion des données météo 
 * weather.h  V0.2
 */

#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "globals.h"
#include "config.h"
#include "logging.h"

// ============================================================================
// MÉTÉO
// ============================================================================

void updateWeatherData() {
  LOG_D(LOG_WEATHER, "Demarrage mise a jour des donnees meteo...");
  
  if (weatherApiKey == "") {
    LOG_W(LOG_WEATHER, "Cle API meteo non configuree - Mise a jour annulee");
    return;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    LOG_W(LOG_WEATHER, "WiFi non connecte - Mise a jour meteo annulee");
    return;
  }
  
  LOG_I(LOG_WEATHER, "Cle API presente, WiFi connecte - Lancement des requetes API");
  LOG_V(LOG_WEATHER, "Coordonnees: lat=%s, lon=%s", latitude.c_str(), longitude.c_str());

  HTTPClient http;
  
  // ========================================================================
  // API MÉTÉO ACTUELLE
  // ========================================================================
  LOG_D(LOG_WEATHER, "Requete API meteo actuelle (OpenWeatherMap)...");
  
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + latitude +
               "&lon=" + longitude + "&appid=" + weatherApiKey + "&units=metric";
  
  LOG_V(LOG_WEATHER, "URL: http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=***&units=metric",
        latitude.c_str(), longitude.c_str());
  
  http.begin(url);
  int httpCode = http.GET();
  
  LOG_D(LOG_WEATHER, "Code HTTP recu: %d", httpCode);
  
  if (httpCode == 200) {
    LOG_I(LOG_WEATHER, "Requete API meteo actuelle reussie (HTTP 200)");
    
    String payload = http.getString();
    LOG_V(LOG_WEATHER, "Taille reponse JSON: %d bytes", payload.length());
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      LOG_E(LOG_WEATHER, "Erreur parsing JSON meteo actuelle: %s", error.c_str());
    } else {
      LOG_D(LOG_WEATHER, "JSON meteo actuelle parse avec succes");
      
      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        tempExterieure = doc["main"]["temp"].as<float>();
        LOG_V(LOG_WEATHER, "Temperature exterieure recuperee: %.2f C", tempExterieure);
        
        // Récupérer les nuages pour estimer l'ensoleillement
        if (doc.containsKey("clouds")) {
          float cloudiness = doc["clouds"]["all"].as<float>(); // 0-100%
          weatherSunshine = 100.0 - cloudiness; // Inverser
          LOG_V(LOG_WEATHER, "Couverture nuageuse: %.0f%%, Ensoleillement: %.0f%%", 
                cloudiness, weatherSunshine);
        } else {
          LOG_W(LOG_WEATHER, "Donnees de couverture nuageuse non disponibles");
        }
        
        xSemaphoreGive(dataMutex);
        
        LOG_I(LOG_WEATHER, "Meteo actuelle: %.2f C, Ensoleillement: %.0f%%", 
              tempExterieure, weatherSunshine);
      } else {
        LOG_E(LOG_WEATHER, "Impossible d'obtenir le mutex pour meteo actuelle");
      }
    }
  } else if (httpCode > 0) {
    LOG_E(LOG_WEATHER, "Erreur API meteo actuelle - Code HTTP: %d", httpCode);
    
    // Détails des codes d'erreur courants
    switch(httpCode) {
      case 401:
        LOG_E(LOG_WEATHER, "Erreur 401: Cle API invalide ou expiree");
        break;
      case 404:
        LOG_E(LOG_WEATHER, "Erreur 404: Coordonnees invalides");
        break;
      case 429:
        LOG_E(LOG_WEATHER, "Erreur 429: Limite d'appels API depassee");
        break;
      case 500:
      case 502:
      case 503:
        LOG_E(LOG_WEATHER, "Erreur serveur OpenWeatherMap");
        break;
    }
  } else {
    LOG_E(LOG_WEATHER, "Erreur de connexion API meteo actuelle: %s", http.errorToString(httpCode).c_str());
  }
  
  http.end();
  LOG_V(LOG_WEATHER, "Connexion HTTP meteo actuelle fermee");
  
  // ========================================================================
  // API PRÉVISIONS (24h) pour MIN/MAX
  // ========================================================================
  LOG_D(LOG_WEATHER, "Requete API previsions 24h (OpenWeatherMap)...");
  
  url = "http://api.openweathermap.org/data/2.5/forecast?lat=" + latitude +
        "&lon=" + longitude + "&appid=" + weatherApiKey + "&units=metric&cnt=8";
  
  LOG_V(LOG_WEATHER, "URL: http://api.openweathermap.org/data/2.5/forecast?lat=%s&lon=%s&appid=***&units=metric&cnt=8",
        latitude.c_str(), longitude.c_str());
  
  http.begin(url);
  httpCode = http.GET();
  
  LOG_D(LOG_WEATHER, "Code HTTP recu: %d", httpCode);
  
  if (httpCode == 200) {
    LOG_I(LOG_WEATHER, "Requete API previsions reussie (HTTP 200)");
    
    String payload = http.getString();
    LOG_V(LOG_WEATHER, "Taille reponse JSON: %d bytes", payload.length());
    
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      LOG_E(LOG_WEATHER, "Erreur parsing JSON previsions: %s", error.c_str());
    } else {
      LOG_D(LOG_WEATHER, "JSON previsions parse avec succes");
      
      JsonArray list = doc["list"];
      int forecastCount = list.size();
      LOG_V(LOG_WEATHER, "Nombre de previsions recues: %d", forecastCount);
      
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
        
        LOG_I(LOG_WEATHER, "Previsions 24h: Min=%.2f C, Max=%.2f C", minTemp, maxTemp);
        LOG_V(LOG_WEATHER, "Amplitude thermique: %.2f C", maxTemp - minTemp);
      } else {
        LOG_E(LOG_WEATHER, "Impossible d'obtenir le mutex pour previsions");
      }
    }
  } else if (httpCode > 0) {
    LOG_E(LOG_WEATHER, "Erreur API previsions - Code HTTP: %d", httpCode);
    
    // Détails des codes d'erreur courants
    switch(httpCode) {
      case 401:
        LOG_E(LOG_WEATHER, "Erreur 401: Cle API invalide ou expiree");
        break;
      case 404:
        LOG_E(LOG_WEATHER, "Erreur 404: Coordonnees invalides");
        break;
      case 429:
        LOG_E(LOG_WEATHER, "Erreur 429: Limite d'appels API depassee");
        break;
      case 500:
      case 502:
      case 503:
        LOG_E(LOG_WEATHER, "Erreur serveur OpenWeatherMap");
        break;
    }
  } else {
    LOG_E(LOG_WEATHER, "Erreur de connexion API previsions: %s", http.errorToString(httpCode).c_str());
  }
  
  http.end();
  LOG_V(LOG_WEATHER, "Connexion HTTP previsions fermee");
  
  LOG_SEPARATOR();
  LOG_I(LOG_WEATHER, "Mise a jour meteo terminee");
  LOG_I(LOG_WEATHER, "Resume: Temp ext=%.2f C, Sunshine=%.0f%%, Min/Max=%.2f/%.2f C",
        tempExterieure, weatherSunshine, weatherTempMin, weatherTempMax);
  LOG_SEPARATOR();
}

void saveWeatherConfig() {
  LOG_D(LOG_WEATHER, "Sauvegarde de la configuration meteo...");
  
  File f = LittleFS.open("/weather.json", FILE_WRITE);
  if (!f) {
    LOG_E(LOG_WEATHER, "Erreur ouverture /weather.json en ecriture");
    LOG_STORAGE_OP("WRITE", "/weather.json", false);
    return;
  }
  
  StaticJsonDocument<512> doc;
  doc["apiKey"] = weatherApiKey;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_WEATHER, "Configuration meteo sauvegardee (%d bytes)", bytesWritten);
  LOG_V(LOG_WEATHER, "Coordonnees: lat=%s, lon=%s", latitude.c_str(), longitude.c_str());
  LOG_V(LOG_WEATHER, "Cle API: %s*** (tronquee pour securite)", 
        weatherApiKey.length() > 0 ? weatherApiKey.substring(0, 8).c_str() : "Non configuree");
  LOG_STORAGE_OP("WRITE", "/weather.json", true);
}

void loadWeatherConfig() {
  LOG_D(LOG_WEATHER, "Chargement de la configuration meteo...");
  
  if (!LittleFS.exists("/weather.json")) {
    LOG_W(LOG_WEATHER, "Fichier /weather.json non trouve - Configuration par defaut");
    return;
  }
  
  File f = LittleFS.open("/weather.json", FILE_READ);
  if (!f) {
    LOG_E(LOG_WEATHER, "Erreur ouverture /weather.json en lecture");
    LOG_STORAGE_OP("READ", "/weather.json", false);
    return;
  }
  
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  
  if (err) {
    LOG_E(LOG_WEATHER, "Erreur parsing JSON meteo config: %s", err.c_str());
    LOG_STORAGE_OP("READ", "/weather.json", false);
    return;
  }
  
  weatherApiKey = doc["apiKey"].as<String>();
  latitude = doc["latitude"].as<String>();
  longitude = doc["longitude"].as<String>();
  
  LOG_I(LOG_WEATHER, "Configuration meteo chargee avec succes");
  LOG_I(LOG_WEATHER, "Coordonnees: lat=%s, lon=%s", latitude.c_str(), longitude.c_str());
  
  if (weatherApiKey.length() > 0) {
    LOG_I(LOG_WEATHER, "Cle API presente (%d caracteres)", weatherApiKey.length());
    LOG_V(LOG_WEATHER, "Cle API: %s*** (tronquee pour securite)", weatherApiKey.substring(0, 8).c_str());
  } else {
    LOG_W(LOG_WEATHER, "Cle API non configuree");
  }
  
  LOG_STORAGE_OP("READ", "/weather.json", true);
}

#endif // WEATHER_H