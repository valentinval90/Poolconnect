/* 
 * POOL CONNECT - CHART WEB HANDLERS
 * API REST pour le système de graphique historique
 * chart_web_handlers.h   V1.0
 * 
 * À AJOUTER dans web_handlers.h avant le #endif final
 */

#ifndef CHART_WEB_HANDLERS_H
#define CHART_WEB_HANDLERS_H

// ============================================================================
// API CHART - DONNÉES D'UN JOUR SPÉCIFIQUE
// ============================================================================

void handleApiChartData() {
  LOG_WEB_REQUEST("GET", "/api/chart/data");
  
  // Paramètres: ?date=YYYY-MM-DD
  if (!server.hasArg("date")) {
    LOG_E(LOG_WEB, "Parametre 'date' manquant");
    server.send(400, "text/plain", "Missing date parameter");
    return;
  }
  
  String dateStr = server.arg("date");
  LOG_D(LOG_WEB, "Recuperation des donnees pour la date: %s", dateStr.c_str());
  
  // Parser la date YYYY-MM-DD
  int year = dateStr.substring(0, 4).toInt();
  int month = dateStr.substring(5, 7).toInt();
  int day = dateStr.substring(8, 10).toInt();
  
  if (year < 2020 || year > 2030 || month < 1 || month > 12 || day < 1 || day > 31) {
    LOG_E(LOG_WEB, "Format de date invalide: %s", dateStr.c_str());
    server.send(400, "text/plain", "Invalid date format");
    return;
  }
  
  LOG_V(LOG_WEB, "Date parsee: %04d-%02d-%02d", year, month, day);
  
  String jsonData = getChartDataForDate(year, month, day);
  
  if (jsonData.indexOf("error") >= 0) {
    LOG_W(LOG_WEB, "Donnees non trouvees pour %s", dateStr.c_str());
    server.send(404, "application/json", jsonData);
  } else {
    LOG_I(LOG_WEB, "Donnees envoyees: %d bytes", jsonData.length());
    server.send(200, "application/json", jsonData);
  }
}

// ============================================================================
// API CHART - LISTE DES DATES DISPONIBLES
// ============================================================================

void handleApiChartAvailableDates() {
  LOG_WEB_REQUEST("GET", "/api/chart/available-dates");
  
  LOG_D(LOG_WEB, "Generation de la liste des dates disponibles...");
  
  String jsonDates = getAvailableDates();
  
  LOG_I(LOG_WEB, "Liste des dates generee: %d bytes", jsonDates.length());
  
  server.send(200, "application/json", jsonDates);
}

// ============================================================================
// API CHART - INFORMATIONS SUR LE STOCKAGE
// ============================================================================

void handleApiChartStorageInfo() {
  LOG_WEB_REQUEST("GET", "/api/chart/storage-info");
  
  LOG_D(LOG_WEB, "Generation des informations de stockage...");
  
  String jsonInfo = getStorageInfo();
  
  LOG_V(LOG_WEB, "Informations de stockage generees: %d bytes", jsonInfo.length());
  
  server.send(200, "application/json", jsonInfo);
}

// ============================================================================
// API CHART - FORCER L'ARCHIVAGE (MANUEL)
// ============================================================================

void handleApiChartForceArchive() {
  LOG_WEB_REQUEST("POST", "/api/chart/force-archive");
  
  LOG_I(LOG_WEB, "Demande d'archivage manuel...");
  
  bool success = forceArchiveNow();
  
  if (success) {
    LOG_I(LOG_WEB, "Archivage manuel reussi");
    server.send(200, "text/plain", "Archive created successfully");
  } else {
    LOG_E(LOG_WEB, "Echec de l'archivage manuel");
    server.send(500, "text/plain", "Archive failed");
  }
}

// ============================================================================
// API CHART - SUPPRIMER UN JOUR SPÉCIFIQUE
// ============================================================================

void handleApiChartDeleteDay() {
  LOG_WEB_REQUEST("DELETE", "/api/chart/data");
  
  if (!server.hasArg("date")) {
    LOG_E(LOG_WEB, "Parametre 'date' manquant");
    server.send(400, "text/plain", "Missing date parameter");
    return;
  }
  
  String dateStr = server.arg("date");
  LOG_W(LOG_WEB, "Demande de suppression pour la date: %s", dateStr.c_str());
  
  // Parser la date
  int year = dateStr.substring(0, 4).toInt();
  int month = dateStr.substring(5, 7).toInt();
  int day = dateStr.substring(8, 10).toInt();
  
  // Construire le chemin du fichier
  char filePath[48];
  snprintf(filePath, sizeof(filePath), "/chart/%04d/%02d/%02d.json",
           year, month, day);
  
  LOG_I(LOG_WEB, "Tentative de suppression: %s", filePath);
  
  if (!LittleFS.exists(filePath)) {
    LOG_W(LOG_WEB, "Fichier non trouve: %s", filePath);
    server.send(404, "text/plain", "Date not found");
    return;
  }
  
  if (LittleFS.remove(filePath)) {
    LOG_I(LOG_WEB, "Fichier supprime avec succes: %s", filePath);
    server.send(200, "text/plain", "Day deleted successfully");
  } else {
    LOG_E(LOG_WEB, "Erreur lors de la suppression: %s", filePath);
    server.send(500, "text/plain", "Delete failed");
  }
}

// ============================================================================
// API CHART - EXPORTER EN CSV
// ============================================================================

void handleApiChartExportCSV() {
  LOG_WEB_REQUEST("GET", "/api/chart/export-csv");
  
  if (!server.hasArg("date")) {
    LOG_E(LOG_WEB, "Parametre 'date' manquant");
    server.send(400, "text/plain", "Missing date parameter");
    return;
  }
  
  String dateStr = server.arg("date");
  LOG_D(LOG_WEB, "Export CSV pour la date: %s", dateStr.c_str());
  
  // Parser la date
  int year = dateStr.substring(0, 4).toInt();
  int month = dateStr.substring(5, 7).toInt();
  int day = dateStr.substring(8, 10).toInt();
  
  // Récupérer les données JSON
  String jsonData = getChartDataForDate(year, month, day);
  
  if (jsonData.indexOf("error") >= 0) {
    LOG_W(LOG_WEB, "Donnees non trouvees pour %s", dateStr.c_str());
    server.send(404, "text/plain", "Date not found");
    return;
  }
  
  // Parser le JSON pour créer le CSV
  DynamicJsonDocument doc(100000);
  DeserializationError err = deserializeJson(doc, jsonData);
  
  if (err) {
    LOG_E(LOG_WEB, "Erreur parsing JSON: %s", err.c_str());
    server.send(500, "text/plain", "JSON parse error");
    return;
  }
  
  // Construire le CSV
  String csv = "Timestamp,Date,Time,Water Temp (C),Pressure (BAR),Pump,Electro,Light,Valve,PAC,Cover Open,Active Timers\n";
  
  JsonArray points = doc["points"];
  
  for (JsonObject p : points) {
    unsigned long timestamp = p["t"];
    
    // Convertir timestamp en date/heure
    time_t t = timestamp;
    struct tm* timeinfo = localtime(&t);
    
    char dateTime[32];
    strftime(dateTime, sizeof(dateTime), "%Y-%m-%d,%H:%M:%S", timeinfo);
    
    csv += String(timestamp) + "," + String(dateTime) + ",";
    csv += String(p["wt"].as<float>(), 1) + ",";
    csv += String(p["pr"].as<float>(), 2) + ",";
    csv += String(p["rp"].as<bool>() ? 1 : 0) + ",";
    csv += String(p["re"].as<bool>() ? 1 : 0) + ",";
    csv += String(p["rl"].as<bool>() ? 1 : 0) + ",";
    csv += String(p["rv"].as<bool>() ? 1 : 0) + ",";
    csv += String(p["rh"].as<bool>() ? 1 : 0) + ",";
    csv += String(p["co"].as<bool>() ? 1 : 0) + ",";
    csv += String(p["at"].as<int>()) + "\n";
  }
  
  LOG_I(LOG_WEB, "CSV genere: %d lignes, %d bytes", points.size(), csv.length());
  
  // Envoyer avec header de téléchargement
  char filename[32];
  snprintf(filename, sizeof(filename), "poolconnect_%04d-%02d-%02d.csv", year, month, day);
  
  server.sendHeader("Content-Disposition", "attachment; filename=" + String(filename));
  server.send(200, "text/csv", csv);
  
  LOG_I(LOG_WEB, "Export CSV termine: %s", filename);
}

#endif // CHART_WEB_HANDLERS_H
