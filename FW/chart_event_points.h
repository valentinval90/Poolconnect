/*
 * ============================================================================
 * CHART EVENT POINTS - Génération de points lors des événements
 * ============================================================================
 * 
 * Ce fichier ajoute la capacité de générer automatiquement des points de 
 * données sur le graphique lors de changements d'état des équipements :
 * 
 * - Activation/désactivation des relais (pompe, électrolyseur, etc.)
 * - Détection de fuite
 * - Changement d'état du volet
 * - Actions des timers
 * 
 * Cela permet d'avoir un graphique précis montrant exactement quand les 
 * équipements changent d'état, sans attendre le prochain intervalle régulier.
 */

#ifndef CHART_EVENT_POINTS_H
#define CHART_EVENT_POINTS_H

#include "chart_storage.h"
#include "globals.h"

// ============================================================================
// FONCTION HELPER - AJOUT DE POINT FORCÉ
// ============================================================================

/**
 * Ajoute un point de données au graphique IMMÉDIATEMENT, sans vérifier l'intervalle.
 * À utiliser lors des événements (changements d'état) pour avoir un graphique précis.
 * 
 * @param waterTemp Température de l'eau
 * @param pressure Pression de l'eau
 * @param relayStates Tableau des états des relais [0-4]
 * @param coverOpen État du volet (true = ouvert)
 * @param activeTimers Nombre de timers actifs
 */
void addChartPointOnEvent(float waterTemp, float pressure, bool* relayStates, 
                          bool coverOpen, uint8_t activeTimers) {
  
  LOG_D(LOG_CHART, "Ajout point sur EVENEMENT...");
  
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
  
  time_t timestamp;
  time(&timestamp);
  point->timestamp = timestamp;
  point->waterTemp = waterTemp;
  point->pressure = pressure;
  point->relayPump = relayStates[0];
  point->relayElectro = relayStates[1];
  point->relayLight = relayStates[2];
  point->relayValve = relayStates[3];
  point->relayPAC = relayStates[4];
  point->coverOpen = coverOpen;
  point->activeTimers = activeTimers;
  
  chartBufferCount++;
  // NE PAS mettre à jour lastChartSave pour permettre le prochain point régulier
  
  LOG_I(LOG_CHART, "Point EVENEMENT ajoute: T=%.1f C, P=%.2f BAR", waterTemp, pressure);
  LOG_V(LOG_CHART, "Buffer: %d/%d points", chartBufferCount, MAX_CHART_POINTS);
  
  // Sauvegarder si buffer presque plein
  if (chartBufferCount % 10 == 0 || chartBufferCount > 100) {
    saveCurrentDayFile();
  }
}

// ============================================================================
// FONCTION HELPER - CAPTURER L'ÉTAT ACTUEL
// ============================================================================

/**
 * Capture l'état actuel de tous les équipements et ajoute un point au graphique.
 * Simplifie l'appel depuis différents endroits du code.
 */
void captureCurrentStateToChart() {
  // Lire les états des relais
  bool relayStates[5];
  for (int i = 0; i < 5; i++) {
    relayStates[i] = (digitalRead(relayPins[i]) == HIGH);
  }
  
  // Compter les timers actifs
  uint8_t activeTimersCount = 0;
  for (int i = 0; i < flexTimerCount; i++) {
    if (flexTimers[i].enabled && flexTimers[i].context.state == TIMER_RUNNING) {
      activeTimersCount++;
    }
  }
  
  // Ajouter le point avec les données actuelles
  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100))) {
    addChartPointOnEvent(waterTemp, waterPressure, relayStates, 
                        coverOpen, activeTimersCount);
    xSemaphoreGive(dataMutex);
  } else {
    LOG_W(LOG_CHART, "Mutex timeout - point evenement ignore");
  }
}

#endif // CHART_EVENT_POINTS_H
