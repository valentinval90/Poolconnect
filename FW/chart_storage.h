/* 
 * POOL CONNECT - CHART STORAGE
 * Système de stockage hiérarchique pour les données de graphique
 * chart_storage.h   V1.0
 */

#ifndef CHART_STORAGE_H
#define CHART_STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "logging.h"

// ============================================================================
// CONSTANTES
// ============================================================================

#define MAX_CHART_POINTS 1440        // Maximum de points par jour (1 minute = 1440 points)
#define CHART_DIR "/chart"           // Répertoire racine
#define CHART_CURRENT "/chart/current.json"  // Fichier du jour en cours

// ============================================================================
// STRUCTURES
// ============================================================================

struct ChartDataPoint {
  unsigned long timestamp;     // Unix timestamp
  float waterTemp;            // Température eau (°C)
  float pressure;             // Pression (BAR)
  bool relayPump;             // État pompe
  bool relayElectro;          // État électrolyseur
  bool relayLight;            // État lampe
  bool relayValve;            // État électrovalve
  bool relayPAC;              // État pompe à chaleur
  bool coverOpen;             // État volet
  uint8_t activeTimers;       // Nombre de timers actifs
};

struct ChartDayFile {
  int year;
  int month;
  int day;
  int pointCount;
  int intervalMs;             // Intervalle de mesure en ms
  
  ChartDayFile() : year(0), month(0), day(0), pointCount(0), intervalMs(300000) {}
};

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

ChartDataPoint chartBuffer[MAX_CHART_POINTS];
int chartBufferCount = 0;
int chartIntervalMs = 300000;  // 5 minutes par défaut
unsigned long lastChartSave = 0;
ChartDayFile currentDayFile;

// ============================================================================
// DÉCLARATIONS FORWARD
// ============================================================================

void saveCurrentDayFile();

// ============================================================================
// INITIALISATION
// ============================================================================

void initChartStorage() {
  LOG_I(LOG_CHART, "Initialisation du systeme de stockage graphique...");
  
  // Créer la structure de répertoires
  if (!LittleFS.exists(CHART_DIR)) {
    if (LittleFS.mkdir(CHART_DIR)) {
      LOG_I(LOG_CHART, "Repertoire %s cree", CHART_DIR);
    } else {
      LOG_E(LOG_CHART, "Erreur creation du repertoire %s", CHART_DIR);
      return;
    }
  }
  
  // Charger la configuration de l'intervalle
  if (LittleFS.exists("/chart_config.json")) {
    File f = LittleFS.open("/chart_config.json", "r");
    if (f) {
      StaticJsonDocument<128> doc;
      if (!deserializeJson(doc, f)) {
        chartIntervalMs = doc["interval"] | 300000;
        LOG_I(LOG_CHART, "Intervalle charge: %d ms (%d min)", 
              chartIntervalMs, chartIntervalMs / 60000);
      }
      f.close();
    }
  }
  
  // Charger le fichier du jour en cours s'il existe
  time_t now;
  time(&now);
  struct tm* timeinfo = localtime(&now);
  
  currentDayFile.year = timeinfo->tm_year + 1900;
  currentDayFile.month = timeinfo->tm_mon + 1;
  currentDayFile.day = timeinfo->tm_mday;
  currentDayFile.intervalMs = chartIntervalMs;
  
  if (LittleFS.exists(CHART_CURRENT)) {
    LOG_D(LOG_CHART, "Chargement du fichier du jour en cours...");
    
    File f = LittleFS.open(CHART_CURRENT, "r");
    if (f) {
      DynamicJsonDocument doc(400000);
      
      DeserializationError err = deserializeJson(doc, f);
      f.close();
      
      if (err) {
        LOG_E(LOG_CHART, "Erreur parsing JSON: %s", err.c_str());
        chartBufferCount = 0;
      } else {
        JsonArray points = doc["points"];
        chartBufferCount = 0;
        
        for (JsonObject p : points) {
          if (chartBufferCount >= MAX_CHART_POINTS) break;
          
          ChartDataPoint* point = &chartBuffer[chartBufferCount];
          point->timestamp = p["t"];
          point->waterTemp = p["wt"];
          point->pressure = p["pr"];
          point->relayPump = p["rp"];
          point->relayElectro = p["re"];
          point->relayLight = p["rl"];
          point->relayValve = p["rv"];
          point->relayPAC = p["rh"];
          point->coverOpen = p["co"];
          point->activeTimers = p["at"];
          
          chartBufferCount++;
        }
        
        LOG_I(LOG_CHART, "Fichier du jour charge: %d points", chartBufferCount);
      }
    }
  } else {
    LOG_I(LOG_CHART, "Pas de fichier du jour - nouveau jour demarre");
    chartBufferCount = 0;
  }
  
  LOG_I(LOG_CHART, "Initialisation terminee - Buffer: %d/%d points", 
        chartBufferCount, MAX_CHART_POINTS);
  LOG_MEMORY();
}

// ============================================================================
// HELPERS DE VALIDATION POUR SÉCURITÉ JSON
// ============================================================================

// Valider et normaliser un timestamp
inline time_t safeTimestamp(time_t t) {
  // Un timestamp valide doit être entre 2020 et 2100
  const time_t MIN_TIMESTAMP = 1577836800;  // 2020-01-01
  const time_t MAX_TIMESTAMP = 4102444800;  // 2100-01-01
  
  if (t < MIN_TIMESTAMP || t > MAX_TIMESTAMP) {
    LOG_W(LOG_CHART, "Timestamp invalide (%ld) -> utilisation de l'heure actuelle", t);
    time_t now;
    time(&now);
    return (now > MIN_TIMESTAMP && now < MAX_TIMESTAMP) ? now : MIN_TIMESTAMP;
  }
  return t;
}

// Valider et normaliser activeTimers
inline uint8_t safeActiveTimers(uint8_t count) {
  if (count > 20) {  // MAX_TIMERS = 20
    LOG_W(LOG_CHART, "ActiveTimers invalide (%d) -> 0", count);
    return 0;
  }
  return count;
}

// Normaliser un boolean (garantir 0 ou 1)
inline bool safeBool(bool b) {
  return b ? true : false;
}

// Valider et normaliser float
inline float safeFloat(float f) {
  if (isnan(f) || isinf(f)) {
    return 0.0;
  }
  return f;
}

// Valider year (année)
inline int safeYear(int y) {
  if (y < 2020 || y > 2100) {
    LOG_W(LOG_CHART, "Annee invalide (%d) -> 2025", y);
    return 2025;
  }
  return y;
}

// Valider month (mois)
inline int safeMonth(int m) {
  if (m < 1 || m > 12) {
    LOG_W(LOG_CHART, "Mois invalide (%d) -> 1", m);
    return 1;
  }
  return m;
}

// Valider day (jour)
inline int safeDay(int d) {
  if (d < 1 || d > 31) {
    LOG_W(LOG_CHART, "Jour invalide (%d) -> 1", d);
    return 1;
  }
  return d;
}

// Valider interval (intervalle en ms)
inline unsigned long safeInterval(unsigned long i) {
  // Intervalle valide entre 10 secondes et 1 heure
  const unsigned long MIN_INTERVAL = 10000;   // 10 secondes
  const unsigned long MAX_INTERVAL = 3600000; // 1 heure
  
  if (i < MIN_INTERVAL || i > MAX_INTERVAL) {
    LOG_W(LOG_CHART, "Intervalle invalide (%lu ms) -> 300000 ms", i);
    return 300000; // 5 minutes par défaut
  }
  return i;
}

// Valider count (nombre de points)
inline int safeCount(int c) {
  if (c < 0 || c > MAX_CHART_POINTS) {
    LOG_W(LOG_CHART, "Count invalide (%d) -> 0", c);
    return 0;
  }
  return c;
}


// ============================================================================
// AJOUT DE POINT
// ============================================================================

void addChartPoint(float waterTemp, float pressure, bool* relayStates, 
                   bool coverOpen, uint8_t activeTimers) {
  
  // Vérifier si c'est l'heure d'ajouter un point
  unsigned long now = millis();
  if (now - lastChartSave < chartIntervalMs && chartBufferCount > 0) {
    return;  // Pas encore l'heure
  }
  
  LOG_D(LOG_CHART, "Ajout d'un nouveau point de donnees...");
  
  // Buffer circulaire : si plein, décaler tous les points
  if (chartBufferCount >= MAX_CHART_POINTS) {
    LOG_V(LOG_CHART, "Buffer plein - Decalage des donnees (FIFO)");
    
    for (int i = 0; i < MAX_CHART_POINTS - 1; i++) {
      chartBuffer[i] = chartBuffer[i + 1];
    }
    chartBufferCount = MAX_CHART_POINTS - 1;
  }
  
  // Ajouter le nouveau point
  ChartDataPoint* point = &chartBuffer[chartBufferCount];
  
  // ✅ VALIDATION COMPLÈTE DE TOUTES LES DONNÉES
  time_t timestamp;
  time(&timestamp);
  point->timestamp = safeTimestamp(timestamp);
  
  point->waterTemp = safeFloat(waterTemp);
  point->pressure = safeFloat(pressure);
  point->relayPump = safeBool(relayStates[0]);
  point->relayElectro = safeBool(relayStates[1]);
  point->relayLight = safeBool(relayStates[2]);
  point->relayValve = safeBool(relayStates[3]);
  point->relayPAC = safeBool(relayStates[4]);
  point->coverOpen = safeBool(coverOpen);
  point->activeTimers = safeActiveTimers(activeTimers);

  
  chartBufferCount++;
  lastChartSave = now;
  
  LOG_V(LOG_CHART, "Point ajoute: T=%.1f C, P=%.2f BAR, Timers=%d", 
        waterTemp, pressure, activeTimers);
  LOG_I(LOG_CHART, "Buffer: %d/%d points", chartBufferCount, MAX_CHART_POINTS);
  
  // Sauvegarder toutes les 10 minutes ou si > 100 points
  if (chartBufferCount % 2 == 0 || chartBufferCount > 100) {
    saveCurrentDayFile();
  }
}

// ============================================================================
// SAUVEGARDE DU FICHIER DU JOUR
// ============================================================================

void saveCurrentDayFile() {
  LOG_D(LOG_CHART, "Sauvegarde du fichier du jour en cours...");
  
  File f = LittleFS.open(CHART_CURRENT, "w");
  if (!f) {
    LOG_E(LOG_CHART, "Erreur ouverture %s en ecriture", CHART_CURRENT);
    return;
  }
  
  DynamicJsonDocument doc(400000);  // 400 KB pour supporter intervalle 1 min
  
  // ✅ VALIDATION DES MÉTADONNÉES
  doc["date"] = String(safeYear(currentDayFile.year)) + "-" + 
                String(safeMonth(currentDayFile.month)) + "-" + 
                String(safeDay(currentDayFile.day));
  doc["interval"] = safeInterval(currentDayFile.intervalMs);
  doc["count"] = safeCount(chartBufferCount);
  
  JsonArray points = doc.createNestedArray("points");
  
  for (int i = 0; i < chartBufferCount; i++) {
    JsonObject p = points.createNestedObject();
    ChartDataPoint* point = &chartBuffer[i];
    
    // ✅ TOUTES LES DONNÉES VALIDÉES AVANT ÉCRITURE JSON
    p["t"] = safeTimestamp(point->timestamp);
    p["wt"] = safeFloat(point->waterTemp);
    p["pr"] = safeFloat(point->pressure);
    p["rp"] = safeBool(point->relayPump);
    p["re"] = safeBool(point->relayElectro);
    p["rl"] = safeBool(point->relayLight);
    p["rv"] = safeBool(point->relayValve);
    p["rh"] = safeBool(point->relayPAC);
    p["co"] = safeBool(point->coverOpen);
    p["at"] = safeActiveTimers(point->activeTimers);
  }
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_CHART, "Fichier du jour sauvegarde: %d points, %d bytes", 
        chartBufferCount, bytesWritten);
}

// ============================================================================
// ARCHIVAGE DU JOUR
// ============================================================================

bool archiveCurrentDay() {
  LOG_SEPARATOR();
  LOG_I(LOG_CHART, "Archivage du jour: %04d-%02d-%02d", 
        currentDayFile.year, currentDayFile.month, currentDayFile.day);
  
  if (chartBufferCount == 0) {
    LOG_W(LOG_CHART, "Aucun point a archiver - Archivage annule");
    return false;
  }
  
  // Créer le chemin: /chart/YYYY/MM/DD.json
  char dirPath[32];
  snprintf(dirPath, sizeof(dirPath), "/chart/%04d", currentDayFile.year);
  
  if (!LittleFS.exists(dirPath)) {
    if (!LittleFS.mkdir(dirPath)) {
      LOG_E(LOG_CHART, "Erreur creation repertoire %s", dirPath);
      return false;
    }
    LOG_D(LOG_CHART, "Repertoire cree: %s", dirPath);
  }
  
  snprintf(dirPath, sizeof(dirPath), "/chart/%04d/%02d", 
           currentDayFile.year, currentDayFile.month);
  
  if (!LittleFS.exists(dirPath)) {
    if (!LittleFS.mkdir(dirPath)) {
      LOG_E(LOG_CHART, "Erreur creation repertoire %s", dirPath);
      return false;
    }
    LOG_D(LOG_CHART, "Repertoire cree: %s", dirPath);
  }
  
  // Sauvegarder le fichier du jour
  char filePath[48];
  snprintf(filePath, sizeof(filePath), "/chart/%04d/%02d/%02d.json",
           currentDayFile.year, currentDayFile.month, currentDayFile.day);
  
  LOG_D(LOG_CHART, "Ecriture du fichier: %s", filePath);
  
  File f = LittleFS.open(filePath, "w");
  if (!f) {
    LOG_E(LOG_CHART, "Erreur ouverture %s en ecriture", filePath);
    return false;
  }
  
  // Même structure que current.json
  DynamicJsonDocument doc(400000);  // 400 KB pour supporter intervalle 1 min
  
  // ✅ VALIDATION DES MÉTADONNÉES
  doc["date"] = String(safeYear(currentDayFile.year)) + "-" + 
                String(safeMonth(currentDayFile.month)) + "-" + 
                String(safeDay(currentDayFile.day));
  doc["interval"] = safeInterval(currentDayFile.intervalMs);
  doc["count"] = safeCount(chartBufferCount);
  
  JsonArray points = doc.createNestedArray("points");
  
  for (int i = 0; i < chartBufferCount; i++) {
    JsonObject p = points.createNestedObject();
    ChartDataPoint* point = &chartBuffer[i];
    
    // ✅ TOUTES LES DONNÉES VALIDÉES AVANT ÉCRITURE JSON
    p["t"] = safeTimestamp(point->timestamp);
    p["wt"] = safeFloat(point->waterTemp);
    p["pr"] = safeFloat(point->pressure);
    p["rp"] = safeBool(point->relayPump);
    p["re"] = safeBool(point->relayElectro);
    p["rl"] = safeBool(point->relayLight);
    p["rv"] = safeBool(point->relayValve);
    p["rh"] = safeBool(point->relayPAC);
    p["co"] = safeBool(point->coverOpen);
    p["at"] = safeActiveTimers(point->activeTimers);
  }
  
  size_t bytesWritten = serializeJson(doc, f);
  f.close();
  
  LOG_I(LOG_CHART, "Jour archive avec succes: %d points, %d bytes", 
        chartBufferCount, bytesWritten);
  
  // Supprimer current.json
  if (LittleFS.exists(CHART_CURRENT)) {
    LittleFS.remove(CHART_CURRENT);
    LOG_D(LOG_CHART, "Fichier %s supprime", CHART_CURRENT);
  }
  
  // Réinitialiser le buffer pour le nouveau jour
  chartBufferCount = 0;
  
  time_t now;
  time(&now);
  struct tm* timeinfo = localtime(&now);
  
  currentDayFile.year = timeinfo->tm_year + 1900;
  currentDayFile.month = timeinfo->tm_mon + 1;
  currentDayFile.day = timeinfo->tm_mday;
  
  LOG_I(LOG_CHART, "Nouveau jour demarre: %04d-%02d-%02d", 
        currentDayFile.year, currentDayFile.month, currentDayFile.day);
  LOG_SEPARATOR();
  
  return true;
}

// ============================================================================
// RÉCUPÉRATION DES DONNÉES D'UN JOUR
// ============================================================================

String getChartDataForDate(int year, int month, int day) {
  LOG_D(LOG_CHART, "Recuperation des donnees pour: %04d-%02d-%02d", year, month, day);
  
  // Vérifier si c'est le jour en cours
  if (year == currentDayFile.year && month == currentDayFile.month && 
      day == currentDayFile.day) {
    LOG_D(LOG_CHART, "Jour en cours - Lecture du buffer RAM");
    
    DynamicJsonDocument doc(400000);  // 400 KB pour supporter intervalle 1 min
    
    // ✅ VALIDATION DES MÉTADONNÉES
    doc["date"] = String(safeYear(year)) + "-" + String(safeMonth(month)) + "-" + String(safeDay(day));
    doc["interval"] = safeInterval(chartIntervalMs);
    doc["count"] = safeCount(chartBufferCount);
    
    JsonArray points = doc.createNestedArray("points");
    
    for (int i = 0; i < chartBufferCount; i++) {
      JsonObject p = points.createNestedObject();
      ChartDataPoint* point = &chartBuffer[i];
      
      // ✅ TOUTES LES DONNÉES VALIDÉES AVANT ÉCRITURE JSON
      p["t"] = safeTimestamp(point->timestamp);
      p["wt"] = safeFloat(point->waterTemp);
      p["pr"] = safeFloat(point->pressure);
      p["rp"] = safeBool(point->relayPump);
      p["re"] = safeBool(point->relayElectro);
      p["rl"] = safeBool(point->relayLight);
      p["rv"] = safeBool(point->relayValve);
      p["rh"] = safeBool(point->relayPAC);
      p["co"] = safeBool(point->coverOpen);
      p["at"] = safeActiveTimers(point->activeTimers);
    }
    
    String output;
    serializeJson(doc, output);
    
    LOG_I(LOG_CHART, "Donnees retournees: %d points (%d bytes)", 
          chartBufferCount, output.length());
    
    return output;
  }
  
  // Sinon charger depuis le fichier archive
  char filePath[48];
  snprintf(filePath, sizeof(filePath), "/chart/%04d/%02d/%02d.json",
           year, month, day);
  
  LOG_D(LOG_CHART, "Lecture du fichier: %s", filePath);
  
  if (!LittleFS.exists(filePath)) {
    LOG_W(LOG_CHART, "Fichier non trouve: %s", filePath);
    return "{\"error\":\"Date not found\"}";
  }
  
  File f = LittleFS.open(filePath, "r");
  if (!f) {
    LOG_E(LOG_CHART, "Erreur ouverture du fichier: %s", filePath);
    return "{\"error\":\"Cannot open file\"}";
  }
  
  String content = f.readString();
  f.close();
  
  LOG_I(LOG_CHART, "Fichier lu avec succes: %d bytes", content.length());
  
  return content;
}

// ============================================================================
// LISTE DES DATES DISPONIBLES
// ============================================================================

String getAvailableDates() {
  LOG_D(LOG_CHART, "Recuperation de la liste des dates disponibles...");
  
  DynamicJsonDocument doc(8192);
  JsonArray dates = doc.to<JsonArray>();
  
  // Ajouter le jour en cours si des données existent
  if (chartBufferCount > 0) {
    JsonObject current = dates.createNestedObject();
    current["date"] = String(currentDayFile.year) + "-" + 
                      String(currentDayFile.month) + "-" + 
                      String(currentDayFile.day);
    current["count"] = chartBufferCount;
    current["interval"] = chartIntervalMs;
  }
  
  // Scanner les archives
  File root = LittleFS.open(CHART_DIR);
  if (!root) {
    LOG_E(LOG_CHART, "Erreur ouverture du repertoire %s", CHART_DIR);
    return "[]";
  }
  
  // Parcourir les années
  File yearDir = root.openNextFile();
  while (yearDir) {
    if (yearDir.isDirectory()) {
      String yearName = yearDir.name();
      
      // Parcourir les mois
      File monthDir = yearDir.openNextFile();
      while (monthDir) {
        if (monthDir.isDirectory()) {
          String monthName = monthDir.name();
          
          // Parcourir les jours
          File dayFile = monthDir.openNextFile();
          while (dayFile) {
            if (!dayFile.isDirectory()) {
              String dayName = dayFile.name();
              
              // Extraire la date du nom de fichier
              if (dayName.endsWith(".json")) {
                int year = yearName.substring(yearName.lastIndexOf('/') + 1).toInt();
                int month = monthName.substring(monthName.lastIndexOf('/') + 1).toInt();
                int day = dayName.substring(0, dayName.indexOf('.')).toInt();
                
                JsonObject dateObj = dates.createNestedObject();
                dateObj["date"] = String(year) + "-" + String(month) + "-" + String(day);
                
                // Lire le nombre de points depuis le fichier
                File f = LittleFS.open(dayFile.name(), "r");
                if (f) {
                  StaticJsonDocument<256> fileDoc;
                  if (!deserializeJson(fileDoc, f)) {
                    dateObj["count"] = fileDoc["count"] | 0;
                    dateObj["interval"] = fileDoc["interval"] | 300000;
                  }
                  f.close();
                }
              }
            }
            dayFile = monthDir.openNextFile();
          }
        }
        monthDir = yearDir.openNextFile();
      }
    }
    yearDir = root.openNextFile();
  }
  
  String output;
  serializeJson(doc, output);
  
  LOG_I(LOG_CHART, "Liste des dates generee: %d dates disponibles", dates.size());
  
  return output;
}

// ============================================================================
// INFORMATIONS SUR LE STOCKAGE
// ============================================================================

String getStorageInfo() {
  LOG_D(LOG_CHART, "Recuperation des informations de stockage...");
  
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;
  
  // Calculer la capacité estimée
  int bytesPerPoint = 50;  // Estimation
  int pointsPerDay = (24 * 60 * 60 * 1000) / chartIntervalMs;
  int bytesPerDay = pointsPerDay * bytesPerPoint + 500;  // +500 pour les métadonnées
  
  int maxDays = freeBytes / bytesPerDay;
  
  // Compter les jours actuels
  int currentDays = 0;
  
  File root = LittleFS.open(CHART_DIR);
  if (root) {
    File yearDir = root.openNextFile();
    while (yearDir) {
      if (yearDir.isDirectory()) {
        File monthDir = yearDir.openNextFile();
        while (monthDir) {
          if (monthDir.isDirectory()) {
            File dayFile = monthDir.openNextFile();
            while (dayFile) {
              if (!dayFile.isDirectory() && String(dayFile.name()).endsWith(".json")) {
                currentDays++;
              }
              dayFile = monthDir.openNextFile();
            }
          }
          monthDir = yearDir.openNextFile();
        }
      }
      yearDir = root.openNextFile();
    }
  }
  
  DynamicJsonDocument doc(512);
  doc["totalBytes"] = totalBytes;
  doc["usedBytes"] = usedBytes;
  doc["freeBytes"] = freeBytes;
  doc["currentDays"] = currentDays;
  doc["maxDays"] = maxDays;
  doc["intervalMs"] = chartIntervalMs;
  doc["pointsPerDay"] = pointsPerDay;
  doc["currentPoints"] = chartBufferCount;
  
  String output;
  serializeJson(doc, output);
  
  LOG_I(LOG_CHART, "Stockage: %d/%d jours, %d KB libres", 
        currentDays, maxDays, freeBytes / 1024);
  
  return output;
}

#endif // CHART_STORAGE_H
