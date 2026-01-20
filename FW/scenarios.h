/* 
 * POOL CONNECT - SCÉNARIOS PRÉ-CONFIGURÉS
 * Configurations types prêtes à l'emploi
 */

#ifndef SCENARIOS_H
#define SCENARIOS_H

#include <Arduino.h>
#include "types.h"

// ============================================================================
// DÉFINITION DES SCÉNARIOS
// ============================================================================

enum ScenarioType {
  SCENARIO_SUMMER_BASIC,          // Été basique (filtration simple)
  SCENARIO_SUMMER_INTENSIVE,      // Été intensif (chaleur + électrolyse)
  SCENARIO_SPRING_FALL,           // Printemps/Automne
  SCENARIO_WINTER_MINIMAL,        // Hiver minimal
  SCENARIO_ECO_MODE,              // Mode économique
  SCENARIO_CONTINUOUS_24H         // Filtration continue 24h
};

struct Scenario {
  const char* name;
  const char* description;
  ScenarioType type;
};

const Scenario SCENARIOS[] = {
  {
    "☀️ Été Basique",
    "Filtration standard 8h/jour, électrolyse adaptée",
    SCENARIO_SUMMER_BASIC
  },
  {
    "🔥 Été Intensif",
    "Filtration 12h+, électrolyse renforcée, PAC active",
    SCENARIO_SUMMER_INTENSIVE
  },
  {
    "🍂 Printemps/Automne",
    "Filtration modérée 6h, électrolyse réduite",
    SCENARIO_SPRING_FALL
  },
  {
    "❄️ Hivernage Actif",
    "Filtration minimale 2h, hors-gel uniquement",
    SCENARIO_WINTER_MINIMAL
  },
  {
    "💚 Mode Économique",
    "Optimisé consommation, durée = Temp/2",
    SCENARIO_ECO_MODE
  },
  {
    "🔄 Filtration 24h",
    "Continue avec cycles adaptés température",
    SCENARIO_CONTINUOUS_24H
  }
};

const int SCENARIO_COUNT = 6;

// ============================================================================
// GÉNÉRATION TIMER DEPUIS SCÉNARIO
// ============================================================================

FlexibleTimer createTimerFromScenario(ScenarioType type) {
  FlexibleTimer timer;
  
  // Configuration commune
  for (int i = 0; i < 7; i++) timer.days[i] = true; // Tous les jours
  timer.enabled = true;
  timer.lastTriggeredDay = -1;
  timer.context.state = TIMER_IDLE;
  timer.context.currentActionIndex = 0;
  
  switch(type) {
    
    // ========================================================================
    // ÉTÉ BASIQUE
    // ========================================================================
    case SCENARIO_SUMMER_BASIC:
    {
      timer.name = "Été Basique";
      timer.id = millis() & 0x7FFFFFFF;
      
      // Démarrage 9h00
      timer.startTime.type = START_FIXED;
      timer.startTime.hour = 9;
      timer.startTime.minute = 0;
      
      // Conditions: Température > 15°C, pas de fuite
      timer.conditionCount = 2;
      timer.conditions[0].type = CONDITION_TEMP_MIN;
      timer.conditions[0].value = 15.0;
      timer.conditions[0].required = true;
      timer.conditions[1].type = CONDITION_NO_LEAK;
      timer.conditions[1].required = true;
      
      // Actions
      timer.actionCount = 0;
      
      // 1. Démarrer pompe
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0; // Pompe
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].delayMinutes = 0;
      timer.actions[timer.actionCount].description = "Pompe ON";
      timer.actionCount++;
      
      // 2. Attendre 5 minutes
      timer.actions[timer.actionCount].type = ACTION_WAIT_DURATION;
      timer.actions[timer.actionCount].delayMinutes = 5;
      timer.actions[timer.actionCount].description = "Stabilisation 5 min";
      timer.actionCount++;
      
      // 3. Démarrer électrolyseur
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 1; // Électrolyseur
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].delayMinutes = 0;
      timer.actions[timer.actionCount].description = "Électrolyse ON";
      timer.actionCount++;
      
      // 4. Mesurer température
      timer.actions[timer.actionCount].type = ACTION_MEASURE_TEMP;
      timer.actions[timer.actionCount].delayMinutes = 0;
      timer.actions[timer.actionCount].description = "Mesure température";
      timer.actionCount++;
      
      // 5. Durée automatique (Temp/2)
      timer.actions[timer.actionCount].type = ACTION_AUTO_DURATION;
      timer.actions[timer.actionCount].customEquation.useCustom = false;
      timer.actions[timer.actionCount].customEquation.expression = "waterTemp / 2";
      timer.actions[timer.actionCount].description = "Filtration auto (Temp/2)";
      timer.actionCount++;
      
      // 6. Arrêter électrolyseur
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 1;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].delayMinutes = 0;
      timer.actions[timer.actionCount].description = "Électrolyse OFF";
      timer.actionCount++;
      
      // 7. Arrêter pompe
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].delayMinutes = 0;
      timer.actions[timer.actionCount].description = "Pompe OFF";
      timer.actionCount++;
      
      break;
    }
    
    // ========================================================================
    // ÉTÉ INTENSIF
    // ========================================================================
    case SCENARIO_SUMMER_INTENSIVE:
    {
      timer.name = "Été Intensif";
      timer.id = millis() & 0x7FFFFFFF;
      
      // Démarrage 8h00
      timer.startTime.type = START_FIXED;
      timer.startTime.hour = 8;
      timer.startTime.minute = 0;
      
      // Conditions: Température > 18°C
      timer.conditionCount = 2;
      timer.conditions[0].type = CONDITION_TEMP_MIN;
      timer.conditions[0].value = 18.0;
      timer.conditions[0].required = true;
      timer.conditions[1].type = CONDITION_NO_LEAK;
      timer.conditions[1].required = true;
      
      // Actions
      timer.actionCount = 0;
      
      // Pompe + PAC
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Pompe ON";
      timer.actionCount++;
      
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 4; // PAC
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].delayMinutes = 2;
      timer.actions[timer.actionCount].description = "PAC ON";
      timer.actionCount++;
      
      // Attente stabilisation
      timer.actions[timer.actionCount].type = ACTION_WAIT_DURATION;
      timer.actions[timer.actionCount].delayMinutes = 5;
      timer.actions[timer.actionCount].description = "Stabilisation";
      timer.actionCount++;
      
      // Électrolyseur
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 1;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Électrolyse ON";
      timer.actionCount++;
      
      // Mesure temp
      timer.actions[timer.actionCount].type = ACTION_MEASURE_TEMP;
      timer.actions[timer.actionCount].description = "Mesure température";
      timer.actionCount++;
      
      // Durée adaptée avec boost
      timer.actions[timer.actionCount].type = ACTION_AUTO_DURATION;
      timer.actions[timer.actionCount].customEquation.useCustom = true;
      timer.actions[timer.actionCount].customEquation.expression = "waterTemp / 2 + 2";
      timer.actions[timer.actionCount].description = "Filtration intensif";
      timer.actionCount++;
      
      // Arrêts
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 1;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].description = "Électrolyse OFF";
      timer.actionCount++;
      
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 4;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].description = "PAC OFF";
      timer.actionCount++;
      
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].description = "Pompe OFF";
      timer.actionCount++;
      
      break;
    }
    
    // ========================================================================
    // PRINTEMPS/AUTOMNE
    // ========================================================================
    case SCENARIO_SPRING_FALL:
    {
      timer.name = "Printemps/Automne";
      timer.id = millis() & 0x7FFFFFFF;
      
      timer.startTime.type = START_FIXED;
      timer.startTime.hour = 10;
      timer.startTime.minute = 0;
      
      timer.conditionCount = 2;
      timer.conditions[0].type = CONDITION_TEMP_MIN;
      timer.conditions[0].value = 10.0;
      timer.conditions[0].required = true;
      timer.conditions[1].type = CONDITION_NO_LEAK;
      timer.conditions[1].required = true;
      
      timer.actionCount = 0;
      
      // Pompe
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Pompe ON";
      timer.actionCount++;
      
      // Stabilisation
      timer.actions[timer.actionCount].type = ACTION_WAIT_DURATION;
      timer.actions[timer.actionCount].delayMinutes = 5;
      timer.actions[timer.actionCount].description = "Stabilisation";
      timer.actionCount++;
      
      // Électrolyse réduite
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 1;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Électrolyse ON";
      timer.actionCount++;
      
      // Mesure
      timer.actions[timer.actionCount].type = ACTION_MEASURE_TEMP;
      timer.actions[timer.actionCount].description = "Mesure température";
      timer.actionCount++;
      
      // Durée réduite
      timer.actions[timer.actionCount].type = ACTION_AUTO_DURATION;
      timer.actions[timer.actionCount].customEquation.useCustom = true;
      timer.actions[timer.actionCount].customEquation.expression = "waterTemp / 3 + 2";
      timer.actions[timer.actionCount].description = "Filtration modérée";
      timer.actionCount++;
      
      // Arrêts
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 1;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].description = "Électrolyse OFF";
      timer.actionCount++;
      
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].description = "Pompe OFF";
      timer.actionCount++;
      
      break;
    }
    
    // ========================================================================
    // HIVERNAGE
    // ========================================================================
    case SCENARIO_WINTER_MINIMAL:
    {
      timer.name = "Hivernage Actif";
      timer.id = millis() & 0x7FFFFFFF;
      
      timer.startTime.type = START_FIXED;
      timer.startTime.hour = 12;
      timer.startTime.minute = 0;
      
      timer.conditionCount = 2;
      timer.conditions[0].type = CONDITION_TEMP_EXT_MIN;
      timer.conditions[0].value = 2.0; // Hors-gel
      timer.conditions[0].required = true;
      timer.conditions[1].type = CONDITION_NO_LEAK;
      timer.conditions[1].required = true;
      
      timer.actionCount = 0;
      
      // Pompe seulement
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Pompe ON (hors-gel)";
      timer.actionCount++;
      
      // Durée fixe 2h
      timer.actions[timer.actionCount].type = ACTION_WAIT_DURATION;
      timer.actions[timer.actionCount].delayMinutes = 120;
      timer.actions[timer.actionCount].description = "Filtration 2h";
      timer.actionCount++;
      
      // Arrêt
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].description = "Pompe OFF";
      timer.actionCount++;
      
      break;
    }
    
    // ========================================================================
    // MODE ÉCO
    // ========================================================================
    case SCENARIO_ECO_MODE:
    {
      timer.name = "Mode Économique";
      timer.id = millis() & 0x7FFFFFFF;
      
      timer.startTime.type = START_FIXED;
      timer.startTime.hour = 22; // Heures creuses
      timer.startTime.minute = 0;
      
      timer.conditionCount = 1;
      timer.conditions[0].type = CONDITION_NO_LEAK;
      timer.conditions[0].required = true;
      
      timer.actionCount = 0;
      
      // Pompe
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Pompe ON";
      timer.actionCount++;
      
      // Stabilisation courte
      timer.actions[timer.actionCount].type = ACTION_WAIT_DURATION;
      timer.actions[timer.actionCount].delayMinutes = 3;
      timer.actions[timer.actionCount].description = "Stabilisation";
      timer.actionCount++;
      
      // Mesure
      timer.actions[timer.actionCount].type = ACTION_MEASURE_TEMP;
      timer.actions[timer.actionCount].description = "Mesure température";
      timer.actionCount++;
      
      // Durée optimisée
      timer.actions[timer.actionCount].type = ACTION_AUTO_DURATION;
      timer.actions[timer.actionCount].customEquation.useCustom = true;
      timer.actions[timer.actionCount].customEquation.expression = "waterTemp / 2.5";
      timer.actions[timer.actionCount].description = "Filtration économique";
      timer.actionCount++;
      
      // Arrêt
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = false;
      timer.actions[timer.actionCount].description = "Pompe OFF";
      timer.actionCount++;
      
      break;
    }
    
    // ========================================================================
    // FILTRATION 24H
    // ========================================================================
    case SCENARIO_CONTINUOUS_24H:
    {
      timer.name = "Filtration 24h";
      timer.id = millis() & 0x7FFFFFFF;
      
      timer.startTime.type = START_FIXED;
      timer.startTime.hour = 0;
      timer.startTime.minute = 0;
      
      timer.conditionCount = 1;
      timer.conditions[0].type = CONDITION_NO_LEAK;
      timer.conditions[0].required = true;
      
      timer.actionCount = 0;
      
      // Pompe
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 0;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Pompe ON";
      timer.actionCount++;
      
      // Stabilisation
      timer.actions[timer.actionCount].type = ACTION_WAIT_DURATION;
      timer.actions[timer.actionCount].delayMinutes = 5;
      timer.actions[timer.actionCount].description = "Stabilisation";
      timer.actionCount++;
      
      // Électrolyseur
      timer.actions[timer.actionCount].type = ACTION_RELAY;
      timer.actions[timer.actionCount].relay = 1;
      timer.actions[timer.actionCount].state = true;
      timer.actions[timer.actionCount].description = "Électrolyse ON";
      timer.actionCount++;
      
      // Mesure
      timer.actions[timer.actionCount].type = ACTION_MEASURE_TEMP;
      timer.actions[timer.actionCount].description = "Mesure température";
      timer.actionCount++;
      
      // Durée 24h (sera automatiquement bouclé)
      timer.actions[timer.actionCount].type = ACTION_AUTO_DURATION;
      timer.actions[timer.actionCount].customEquation.useCustom = true;
      timer.actions[timer.actionCount].customEquation.expression = "24";
      timer.actions[timer.actionCount].description = "Cycle 24h continu";
      timer.actionCount++;
      
      // PAS d'arrêt pour continuité
      
      break;
    }
  }
  
  return timer;
}

// ============================================================================
// OBTENIR LISTE DES SCÉNARIOS (POUR API)
// ============================================================================

String getScenariosJSON() {
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.to<JsonArray>();
  
  for (int i = 0; i < SCENARIO_COUNT; i++) {
    JsonObject obj = arr.createNestedObject();
    obj["id"] = i;
    obj["name"] = SCENARIOS[i].name;
    obj["description"] = SCENARIOS[i].description;
    obj["type"] = (int)SCENARIOS[i].type;
  }
  
  String output;
  serializeJson(doc, output);
  return output;
}

#endif // SCENARIOS_H
