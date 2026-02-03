/* 
 * POOL CONNECT - CHART ARCHIVER
 * Système d'archivage automatique et gestion de la mémoire
 * chart_archiver.h   V1.0
 */

#ifndef CHART_ARCHIVER_H
#define CHART_ARCHIVER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <time.h>
#include "config.h"
#include "logging.h"
#include "chart_storage.h"

// ============================================================================
// CONSTANTES
// ============================================================================

#define MIN_FREE_SPACE_KB 1024      // Minimum 1 MB libre
#define CHECK_INTERVAL_MS 60000     // Vérifier toutes les minutes

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

int lastCheckDay = -1;
unsigned long lastMemoryCheck = 0;

// ============================================================================
// DÉCLARATIONS FORWARD
// ============================================================================

void checkAndPurgeOldData();

// ============================================================================
// VÉRIFICATION QUOTIDIENNE (À MINUIT)
// ============================================================================

void checkDailyArchive() {
  time_t now;
  time(&now);
  struct tm* timeinfo = localtime(&now);
  
  int currentDay = timeinfo->tm_mday;
  int currentHour = timeinfo->tm_hour;
  int currentMinute = timeinfo->tm_min;
  
  // Vérifier si on vient de passer minuit (entre 00:00 et 00:05)
  if (currentDay != lastCheckDay && currentHour == 0 && currentMinute < 5) {
    LOG_SEPARATOR();
    LOG_I(LOG_CHART, "========================================");
    LOG_I(LOG_CHART, "ARCHIVAGE AUTOMATIQUE - NOUVEAU JOUR");
    LOG_I(LOG_CHART, "========================================");
    LOG_I(LOG_CHART, "Date actuelle: %04d-%02d-%02d %02d:%02d", 
          timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, currentDay,
          currentHour, currentMinute);
    LOG_SEPARATOR();
    
    // Archiver le jour précédent
    if (archiveCurrentDay()) {
      LOG_I(LOG_CHART, "Archivage quotidien reussi");
    } else {
      LOG_E(LOG_CHART, "Echec de l'archivage quotidien");
    }
    
    lastCheckDay = currentDay;
    
    // Vérifier l'espace disque et purger si nécessaire
    checkAndPurgeOldData();
    
    LOG_SEPARATOR();
  }
}

// ============================================================================
// VÉRIFICATION ET PURGE DE LA MÉMOIRE
// ============================================================================

void checkAndPurgeOldData() {
  LOG_I(LOG_CHART, "Verification de l'espace disque...");
  
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;
  size_t freeKB = freeBytes / 1024;
  
  LOG_I(LOG_CHART, "Espace: %d KB utilises / %d KB total (%d KB libres)", 
        usedBytes / 1024, totalBytes / 1024, freeKB);
  
  if (freeKB > MIN_FREE_SPACE_KB) {
    LOG_I(LOG_CHART, "Espace disque suffisant - Pas de purge necessaire");
    return;
  }
  
  LOG_W(LOG_CHART, "Espace disque faible (%d KB < %d KB) - Demarrage de la purge", 
        freeKB, MIN_FREE_SPACE_KB);
  LOG_SEPARATOR();
  
  // Stratégie de purge : supprimer les mois les plus anciens
  // Parcourir les années de la plus ancienne à la plus récente
  
  bool purgeSuccess = false;
  int purgedMonths = 0;
  
  File root = LittleFS.open(CHART_DIR);
  if (!root) {
    LOG_E(LOG_CHART, "Erreur ouverture du repertoire %s", CHART_DIR);
    return;
  }
  
  // Trouver l'année la plus ancienne
  int oldestYear = 9999;
  File yearDir = root.openNextFile();
  
  while (yearDir) {
    if (yearDir.isDirectory()) {
      String yearName = yearDir.name();
      int year = yearName.substring(yearName.lastIndexOf('/') + 1).toInt();
      
      if (year > 0 && year < oldestYear) {
        oldestYear = year;
      }
    }
    yearDir = root.openNextFile();
  }
  
  if (oldestYear == 9999) {
    LOG_W(LOG_CHART, "Aucune archive trouvee - Impossible de purger");
    return;
  }
  
  LOG_I(LOG_CHART, "Annee la plus ancienne trouvee: %d", oldestYear);
  
  // Parcourir cette année et trouver le mois le plus ancien
  char oldestYearPath[32];
  snprintf(oldestYearPath, sizeof(oldestYearPath), "/chart/%04d", oldestYear);
  
  File oldYearDir = LittleFS.open(oldestYearPath);
  if (!oldYearDir) {
    LOG_E(LOG_CHART, "Erreur ouverture du repertoire %s", oldestYearPath);
    return;
  }
  
  int oldestMonth = 13;
  File monthDir = oldYearDir.openNextFile();
  
  while (monthDir) {
    if (monthDir.isDirectory()) {
      String monthName = monthDir.name();
      int month = monthName.substring(monthName.lastIndexOf('/') + 1).toInt();
      
      if (month > 0 && month < oldestMonth) {
        oldestMonth = month;
      }
    }
    monthDir = oldYearDir.openNextFile();
  }
  
  if (oldestMonth == 13) {
    LOG_W(LOG_CHART, "Aucun mois trouve dans l'annee %d", oldestYear);
    return;
  }
  
  LOG_I(LOG_CHART, "Mois le plus ancien: %04d-%02d", oldestYear, oldestMonth);
  
  // Supprimer ce mois
  char oldestMonthPath[48];
  snprintf(oldestMonthPath, sizeof(oldestMonthPath), "/chart/%04d/%02d", 
           oldestYear, oldestMonth);
  
  LOG_W(LOG_CHART, "Suppression du mois: %s", oldestMonthPath);
  
  // Compter et supprimer tous les fichiers du mois
  File monthToDelete = LittleFS.open(oldestMonthPath);
  if (monthToDelete) {
    int deletedFiles = 0;
    File dayFile = monthToDelete.openNextFile();
    
    while (dayFile) {
      if (!dayFile.isDirectory()) {
        String filePath = String(dayFile.name());
        dayFile.close();
        
        if (LittleFS.remove(filePath)) {
          deletedFiles++;
          LOG_V(LOG_CHART, "Fichier supprime: %s", filePath.c_str());
        }
      }
      dayFile = monthToDelete.openNextFile();
    }
    
    monthToDelete.close();
    
    LOG_I(LOG_CHART, "%d fichiers supprimes", deletedFiles);
    
    // Supprimer le répertoire du mois
    if (LittleFS.rmdir(oldestMonthPath)) {
      LOG_I(LOG_CHART, "Repertoire supprime: %s", oldestMonthPath);
      purgedMonths++;
      purgeSuccess = true;
    } else {
      LOG_W(LOG_CHART, "Impossible de supprimer le repertoire (peut contenir des sous-dossiers)");
    }
  }
  
  // Vérifier si l'année est maintenant vide
  File checkYearDir = LittleFS.open(oldestYearPath);
  if (checkYearDir) {
    File anyMonth = checkYearDir.openNextFile();
    if (!anyMonth) {
      // Année vide, la supprimer
      if (LittleFS.rmdir(oldestYearPath)) {
        LOG_I(LOG_CHART, "Annee vide supprimee: %s", oldestYearPath);
      }
    }
    checkYearDir.close();
  }
  
  // Vérifier l'espace libéré
  size_t newFreeBytes = LittleFS.totalBytes() - LittleFS.usedBytes();
  size_t freedBytes = newFreeBytes - freeBytes;
  
  LOG_SEPARATOR();
  LOG_I(LOG_CHART, "Purge terminee: %d mois supprimes", purgedMonths);
  LOG_I(LOG_CHART, "Espace libere: %d KB", freedBytes / 1024);
  LOG_I(LOG_CHART, "Espace libre total: %d KB", newFreeBytes / 1024);
  LOG_SEPARATOR();
  
  // Si toujours pas assez d'espace, réessayer
  if (newFreeBytes / 1024 < MIN_FREE_SPACE_KB && purgeSuccess) {
    LOG_W(LOG_CHART, "Espace toujours insuffisant - Nouvelle purge...");
    checkAndPurgeOldData();  // Récursif
  }
}

// ============================================================================
// VÉRIFICATION PÉRIODIQUE
// ============================================================================

void checkMemoryPeriodic() {
  unsigned long now = millis();
  
  if (now - lastMemoryCheck > CHECK_INTERVAL_MS) {
    lastMemoryCheck = now;
    
    // Vérifier si on doit archiver (changement de jour)
    checkDailyArchive();
    
    // Vérifier l'espace disque toutes les heures
    static int checkCount = 0;
    checkCount++;
    
    if (checkCount >= 60) {  // 60 minutes
      checkCount = 0;
      
      size_t freeBytes = LittleFS.totalBytes() - LittleFS.usedBytes();
      size_t freeKB = freeBytes / 1024;
      
      LOG_V(LOG_CHART, "Verification periodique - Espace libre: %d KB", freeKB);
      
      if (freeKB < MIN_FREE_SPACE_KB) {
        LOG_W(LOG_CHART, "Espace disque faible detecte - Lancement de la purge");
        checkAndPurgeOldData();
      }
    }
  }
}

// ============================================================================
// INITIALISATION
// ============================================================================

void initChartArchiver() {
  LOG_I(LOG_CHART, "Initialisation du systeme d'archivage...");
  
  time_t now;
  time(&now);
  struct tm* timeinfo = localtime(&now);
  
  lastCheckDay = timeinfo->tm_mday;
  lastMemoryCheck = millis();
  
  LOG_I(LOG_CHART, "Jour de reference: %d", lastCheckDay);
  LOG_I(LOG_CHART, "Archivage automatique active (verification a minuit)");
  
  // Vérifier immédiatement l'espace disque
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;
  
  LOG_I(LOG_CHART, "Espace disque: %d KB / %d KB (%d%% utilise)", 
        usedBytes / 1024, totalBytes / 1024, 
        (usedBytes * 100) / totalBytes);
  
  if (freeBytes / 1024 < MIN_FREE_SPACE_KB) {
    LOG_W(LOG_CHART, "Espace disque faible au demarrage - Purge recommandee");
  }
}

// ============================================================================
// FORCER L'ARCHIVAGE (POUR TESTS OU MANUEL)
// ============================================================================

bool forceArchiveNow() {
  LOG_I(LOG_CHART, "Archivage manuel force...");
  
  bool success = archiveCurrentDay();
  
  if (success) {
    LOG_I(LOG_CHART, "Archivage manuel reussi");
    checkAndPurgeOldData();
  } else {
    LOG_E(LOG_CHART, "Echec de l'archivage manuel");
  }
  
  return success;
}

#endif // CHART_ARCHIVER_H
