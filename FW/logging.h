/* 
 * POOL CONNECT - LOGGING
 * logging.h   V0.3
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "config.h"

// Niveaux de log
enum LogLevel {
    LOG_NONE = 0,
    LOG_ERROR = 1,
    LOG_WARNING = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4,
    LOG_VERBOSE = 5
};

// Catégories de log
#define LOG_SYSTEM    "[SYSTEM]"
#define LOG_NETWORK   "[NETWORK]"
#define LOG_WEB       "[WEB]"
#define LOG_SENSOR    "[SENSOR]"
#define LOG_TIMER     "[TIMER]"
#define LOG_MQTT      "[MQTT]"
#define LOG_STORAGE   "[STORAGE]"
#define LOG_WEATHER   "[WEATHER]"
#define LOG_SCENARIO  "[SCENARIO]"
#define LOG_BACKUP    "[BACKUP]"
#define LOG_CHART     "[CHART]"
#define LOG_OTA       "[OTA]"

// Macros de logging conditionnelles
#if ENABLE_LOGGING

    #define LOG_INIT() Serial.begin(115200); delay(1000)
    
    #define LOG_E(category, format, ...) \
        if(LOG_LEVEL >= LOG_ERROR) { \
            Serial.printf("[ERROR]%s " format "\n", category, ##__VA_ARGS__); \
        }
    
    #define LOG_W(category, format, ...) \
        if(LOG_LEVEL >= LOG_WARNING) { \
            Serial.printf("[WARN]%s " format "\n", category, ##__VA_ARGS__); \
        }
    
    #define LOG_I(category, format, ...) \
        if(LOG_LEVEL >= LOG_INFO) { \
            Serial.printf("[INFO]%s " format "\n", category, ##__VA_ARGS__); \
        }
    
    #define LOG_D(category, format, ...) \
        if(LOG_LEVEL >= LOG_DEBUG) { \
            Serial.printf("[DEBUG]%s " format "\n", category, ##__VA_ARGS__); \
        }
    
    #define LOG_V(category, format, ...) \
        if(LOG_LEVEL >= LOG_VERBOSE) { \
            Serial.printf("[VERBOSE]%s " format "\n", category, ##__VA_ARGS__); \
        }

    // Logs spécifiques pour les échanges web <-> hardware
    #define LOG_WEB_REQUEST(method, endpoint) \
        if(LOG_WEB_REQUESTS && LOG_LEVEL >= LOG_INFO) { \
            Serial.printf("[WEB-REQ] %s %s\n", method, endpoint); \
        }
    
    #define LOG_WEB_RESPONSE(endpoint, status) \
        if(LOG_WEB_REQUESTS && LOG_LEVEL >= LOG_INFO) { \
            Serial.printf("[WEB-RES] %s - Status: %d\n", endpoint, status); \
        }
    
    #define LOG_WEB_DATA(name, value) \
        if(LOG_WEB_DATA_EXCHANGE && LOG_LEVEL >= LOG_DEBUG) { \
            Serial.printf("[WEB-DATA] %s = %s\n", name, value); \
        }
    
    #define LOG_WEB_JSON(json) \
        if(LOG_WEB_DATA_EXCHANGE && LOG_LEVEL >= LOG_VERBOSE) { \
            Serial.printf("[WEB-JSON] %s\n", json); \
        }

    // Logs pour les capteurs
    #define LOG_SENSOR_READ(sensor, value, unit) \
        if(LOG_SENSOR_VALUES && LOG_LEVEL >= LOG_DEBUG) { \
            Serial.printf("[SENSOR] %s = %.2f %s\n", sensor, value, unit); \
        }

    // Logs pour les timers
    #define LOG_TIMER_EVENT(event, details) \
        if(LOG_TIMER_EVENTS && LOG_LEVEL >= LOG_INFO) { \
            Serial.printf("[TIMER] %s: %s\n", event, details); \
        }

    // Logs pour MQTT
    #define LOG_MQTT_PUB(topic, payload) \
        if(LOG_MQTT_MESSAGES && LOG_LEVEL >= LOG_DEBUG) { \
            Serial.printf("[MQTT-PUB] Topic: %s | Payload: %s\n", topic, payload); \
        }
    
    #define LOG_MQTT_SUB(topic, payload) \
        if(LOG_MQTT_MESSAGES && LOG_LEVEL >= LOG_DEBUG) { \
            Serial.printf("[MQTT-SUB] Topic: %s | Payload: %s\n", topic, payload); \
        }

    // Logs pour le stockage
    #define LOG_STORAGE_OP(operation, key, success) \
        if(LOG_STORAGE_OPS && LOG_LEVEL >= LOG_DEBUG) { \
            Serial.printf("[STORAGE] %s '%s' - %s\n", operation, key, success ? "OK" : "FAILED"); \
        }

    // Logs pour la mémoire
    #define LOG_MEMORY() \
        if(LOG_MEMORY_INFO && LOG_LEVEL >= LOG_INFO) { \
            Serial.printf("[MEMORY] Free Heap: %d bytes | Min Free: %d bytes\n", \
                ESP.getFreeHeap(), ESP.getMinFreeHeap()); \
        }

    // Logs pour les opérations OTA
    #define LOG_OTA_OP(operation, details) \
        if(LOG_OTA_OPERATIONS && LOG_LEVEL >= LOG_INFO) { \
            Serial.printf("[OTA] %s: %s\n", operation, details); \
        }
    
    #define LOG_OTA_PROGRESS(percentage, current, total) \
        if(LOG_OTA_OPERATIONS && LOG_LEVEL >= LOG_INFO) { \
            Serial.printf("[OTA] Progress: %d%% (%d/%d bytes)\n", percentage, current, total); \
        }

    // Séparateur visuel
    #define LOG_SEPARATOR() \
        if(LOG_LEVEL >= LOG_INFO) { \
            Serial.println("================================================================================"); \
        }

#else
    // Si le logging est désactivé, les macros ne font rien
    #define LOG_INIT()
    #define LOG_E(category, format, ...)
    #define LOG_W(category, format, ...)
    #define LOG_I(category, format, ...)
    #define LOG_D(category, format, ...)
    #define LOG_V(category, format, ...)
    #define LOG_WEB_REQUEST(method, endpoint)
    #define LOG_WEB_RESPONSE(endpoint, status)
    #define LOG_WEB_DATA(name, value)
    #define LOG_WEB_JSON(json)
    #define LOG_SENSOR_READ(sensor, value, unit)
    #define LOG_TIMER_EVENT(event, details)
    #define LOG_MQTT_PUB(topic, payload)
    #define LOG_MQTT_SUB(topic, payload)
    #define LOG_STORAGE_OP(operation, key, success)
    #define LOG_MEMORY()
    #define LOG_OTA_OP(operation, details)
    #define LOG_OTA_PROGRESS(percentage, current, total)
    #define LOG_SEPARATOR()
#endif

#endif // LOGGING_H
