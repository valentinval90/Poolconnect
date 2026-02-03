/* 
 * POOL CONNECT - OTA SYSTEM
 * ota_manager.h   V0.4
 * 
 * FEATURES:
 * - Utilisation du système de logging centralisé
 * - Utilisation de Content-Length au lieu de upload.totalSize
 * - Type de partition correct (U_SPIFFS)
 * - Gestion améliorée des erreurs
 * - Correction ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH
 * 
 * CHANGELOG V0.4 (03/02/2026):
 * - Ajout de flags otaResponseSent et otaUploadSuccess pour communication entre callbacks
 * - Les handlers (handleFirmwareUpload/handleFilesystemUpload) ne renvoient PLUS de réponse HTTP
 * - Une SEULE réponse HTTP est envoyée dans le callback final après l'upload complet
 * - Résout le bug: ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH
 * - Redémarrage après 3 secondes pour garantir la réception de la réponse HTTP
 */
#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Update.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <esp_ota_ops.h>
#include "logging.h"

namespace OTAManager {

// Structure pour suivre la progression
struct OTAProgress {
    size_t totalSize;
    size_t currentSize;
    uint8_t percentage;
    bool inProgress;
    String type; // "firmware" ou "filesystem"
    unsigned long startTime;
    String errorMsg;
};

static OTAProgress otaProgress = {0, 0, 0, false, "", 0, ""};

// Variables pour l'upload
static File uploadFile;
static size_t uploadSize = 0;
static String lastError = "";

// ===================================================================
// CORRECTION V0.4: Flags pour communication entre callbacks
// ===================================================================
// Ces flags résolvent le bug ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH
// en garantissant qu'une SEULE réponse HTTP est envoyée.
//
// PROBLÈME RÉSOLU:
// - Avant: handleFirmwareUpload envoyait une réponse + callback final envoyait une réponse
// - Résultat: Double Content-Length header = erreur navigateur
// 
// SOLUTION:
// - handleFirmwareUpload met juste à jour otaUploadSuccess (pas de réponse)
// - Le callback final lit otaUploadSuccess et envoie UNE SEULE réponse
// ===================================================================
static bool otaResponseSent = false;  // Garantit qu'on n'envoie qu'une seule réponse
static bool otaUploadSuccess = false; // Communique le résultat entre callbacks

// Fonction pour obtenir la progression
String getProgressJSON() {
    StaticJsonDocument<256> doc;
    doc["inProgress"] = otaProgress.inProgress;
    doc["type"] = otaProgress.type;
    doc["totalSize"] = otaProgress.totalSize;
    doc["currentSize"] = otaProgress.currentSize;
    doc["percentage"] = otaProgress.percentage;
    
    if (!otaProgress.errorMsg.isEmpty()) {
        doc["error"] = otaProgress.errorMsg;
    }
    
    if (otaProgress.inProgress && otaProgress.startTime > 0) {
        unsigned long elapsed = (millis() - otaProgress.startTime) / 1000;
        doc["elapsed"] = elapsed;
        
        if (otaProgress.currentSize > 0 && elapsed > 0) {
            float speed = otaProgress.currentSize / (float)elapsed;
            doc["speed"] = (int)speed;
            
            if (speed > 0) {
                int remaining = (otaProgress.totalSize - otaProgress.currentSize) / speed;
                doc["remaining"] = remaining;
            }
        }
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

// Callback de progression
void onOTAProgress(size_t current, size_t total) {
    otaProgress.currentSize = current;
    otaProgress.totalSize = total;
    otaProgress.percentage = (total > 0) ? (current * 100) / total : 0;
    
    static uint8_t lastPercentage = 0;
    if (otaProgress.percentage != lastPercentage && otaProgress.percentage % 10 == 0) {
        LOG_OTA_PROGRESS(otaProgress.percentage, current, total);
        lastPercentage = otaProgress.percentage;
    }
}

// Fonction pour dÃ©marrer la mise Ã  jour firmware
bool startFirmwareUpdate(size_t contentLength) {
    LOG_SEPARATOR();
    LOG_I(LOG_OTA, "Starting FIRMWARE OTA update");
    LOG_I(LOG_OTA, "Content-Length: %d bytes (%.2f MB)", contentLength, contentLength / 1024.0 / 1024.0);
    
    // VÃ©rifier l'espace disponible
    if (contentLength == 0) {
        LOG_E(LOG_OTA, "Content-Length is ZERO! Check if file was selected");
        lastError = "Content length is zero";
        return false;
    }
    
    size_t freeSpace = ESP.getFreeSketchSpace();
    LOG_D(LOG_OTA, "Free sketch space: %d bytes (%.2f MB)", freeSpace, freeSpace / 1024.0 / 1024.0);
    
    if (contentLength > freeSpace) {
        LOG_E(LOG_OTA, "Firmware too large! Need %d bytes, only %d available", contentLength, freeSpace);
        lastError = "Firmware too large for available space";
        return false;
    }
    
    // Initialiser la structure de progression
    otaProgress.totalSize = contentLength;
    otaProgress.currentSize = 0;
    otaProgress.percentage = 0;
    otaProgress.inProgress = true;
    otaProgress.type = "firmware";
    otaProgress.startTime = millis();
    otaProgress.errorMsg = "";
    
    // Configurer Update
    Update.onProgress(onOTAProgress);
    
    LOG_D(LOG_OTA, "Calling Update.begin(%d, U_FLASH)...", contentLength);
    if (!Update.begin(contentLength, U_FLASH)) {
        LOG_E(LOG_OTA, "Update.begin FAILED!");
        LOG_E(LOG_OTA, "Error code: %d", Update.getError());
        LOG_E(LOG_OTA, "Error string: %s", Update.errorString());
        lastError = String("Update.begin failed: ") + Update.errorString();
        otaProgress.inProgress = false;
        otaProgress.errorMsg = lastError;
        return false;
    }
    
    LOG_I(LOG_OTA, "Firmware update initialized successfully");
    LOG_D(LOG_OTA, "Update.begin() returned true, ready to receive data");
    return true;
}

// Fonction pour dÃ©marrer la mise Ã  jour filesystem
bool startFilesystemUpdate(size_t contentLength) {
    LOG_SEPARATOR();
    LOG_I(LOG_OTA, "Starting FILESYSTEM OTA update");
    LOG_I(LOG_OTA, "Content-Length: %d bytes (%.2f MB)", contentLength, contentLength / 1024.0 / 1024.0);
    
    // VÃ©rifier l'espace disponible
    if (contentLength == 0) {
        LOG_E(LOG_OTA, "Content-Length is ZERO! Check if file was selected");
        lastError = "Content length is zero";
        return false;
    }
    
    // Info sur le filesystem actuel
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    LOG_D(LOG_OTA, "Current filesystem: %d total, %d used", totalBytes, usedBytes);
    
    // Initialiser la structure de progression
    otaProgress.totalSize = contentLength;
    otaProgress.currentSize = 0;
    otaProgress.percentage = 0;
    otaProgress.inProgress = true;
    otaProgress.type = "filesystem";
    otaProgress.startTime = millis();
    otaProgress.errorMsg = "";
    
    // Configurer Update pour SPIFFS (compatible LittleFS)
    Update.onProgress(onOTAProgress);
    
    LOG_D(LOG_OTA, "Calling Update.begin(%d, U_SPIFFS)...", contentLength);
    if (!Update.begin(contentLength, U_SPIFFS)) {
        LOG_E(LOG_OTA, "Update.begin FAILED!");
        LOG_E(LOG_OTA, "Error code: %d", Update.getError());
        LOG_E(LOG_OTA, "Error string: %s", Update.errorString());
        lastError = String("Update.begin failed: ") + Update.errorString();
        otaProgress.inProgress = false;
        otaProgress.errorMsg = lastError;
        return false;
    }
    
    LOG_I(LOG_OTA, "Filesystem update initialized successfully");
    LOG_D(LOG_OTA, "Update.begin() returned true, ready to receive data");
    return true;
}

// Fonction pour Ã©crire des donnÃ©es pendant la mise Ã  jour
bool writeOTAData(uint8_t* data, size_t len) {
    if (!otaProgress.inProgress) {
        LOG_E(LOG_OTA, "writeOTAData called but OTA not in progress!");
        return false;
    }
    
    LOG_D(LOG_OTA, "Writing %d bytes (total written so far: %d/%d)", 
                 len, otaProgress.currentSize, otaProgress.totalSize);
    
    size_t written = Update.write(data, len);
    if (written != len) {
        LOG_E(LOG_OTA, "Update.write FAILED!");
        LOG_E(LOG_OTA, "Tried to write %d bytes, only wrote %d", len, written);
        LOG_E(LOG_OTA, "Error: %s", Update.errorString());
        lastError = String("Write failed: ") + Update.errorString();
        otaProgress.errorMsg = lastError;
        return false;
    }
    
    // ================================================================
    // CORRECTION CRITIQUE: Mettre à jour manuellement la progression
    // ================================================================
    // Update.onProgress() n'est pas toujours appelé, donc on met à jour
    // manuellement otaProgress à chaque write
    otaProgress.currentSize += written;
    if (otaProgress.totalSize > 0) {
        otaProgress.percentage = (otaProgress.currentSize * 100) / otaProgress.totalSize;
    }
    
    // Log périodique
    static uint8_t lastPercentage = 0;
    if (otaProgress.percentage != lastPercentage && otaProgress.percentage % 10 == 0) {
        LOG_I(LOG_OTA, "Progress: %d%% (%d / %d bytes)", 
              otaProgress.percentage, otaProgress.currentSize, otaProgress.totalSize);
        lastPercentage = otaProgress.percentage;
    }
    // ================================================================
    
    LOG_D(LOG_OTA, "Successfully wrote %d bytes, progress: %d%%", written, otaProgress.percentage);
    return true;
}

// Fonction pour finaliser la mise Ã  jour
bool endOTAUpdate() {
    if (!otaProgress.inProgress) {
        LOG_E(LOG_OTA, "endOTAUpdate called but OTA not in progress!");
        return false;
    }
    
    LOG_I(LOG_OTA, "Finalizing OTA update...");
    LOG_D(LOG_OTA, "Total bytes written: %d/%d", otaProgress.currentSize, otaProgress.totalSize);
    
    if (!Update.end(true)) {
        LOG_E(LOG_OTA, "Update.end FAILED!");
        LOG_E(LOG_OTA, "Error: %s", Update.errorString());
        LOG_E(LOG_OTA, "Has error: %d", Update.hasError());
        lastError = String("Update.end failed: ") + Update.errorString();
        otaProgress.inProgress = false;
        otaProgress.errorMsg = lastError;
        return false;
    }
    
    unsigned long totalTime = (millis() - otaProgress.startTime) / 1000;
    LOG_SEPARATOR();
    LOG_I(LOG_OTA, "OTA UPDATE COMPLETED SUCCESSFULLY!");
    LOG_I(LOG_OTA, "Duration: %lu seconds", totalTime);
    LOG_I(LOG_OTA, "Type: %s", otaProgress.type.c_str());
    LOG_I(LOG_OTA, "Size: %d bytes", otaProgress.totalSize);
    LOG_SEPARATOR();
    
    otaProgress.inProgress = false;
    otaProgress.percentage = 100;
    
    return true;
}

// Fonction pour annuler la mise Ã  jour
void abortOTAUpdate() {
    if (otaProgress.inProgress) {
        Update.abort();
        LOG_I(LOG_OTA, "OTA update ABORTED by user or error");
        otaProgress.inProgress = false;
        otaProgress.errorMsg = "Update aborted";
    }
}

// Fonction pour obtenir des informations sur les partitions
String getPartitionInfo() {
    StaticJsonDocument<512> doc;
    
    // Partition courante
    const esp_partition_t* running = esp_ota_get_running_partition();
    if (running) {
        JsonObject current = doc.createNestedObject("current");
        current["label"] = running->label;
        current["address"] = String(running->address, HEX);
        current["size"] = running->size;
    }
    
    // Prochaine partition de boot
    const esp_partition_t* boot = esp_ota_get_boot_partition();
    if (boot) {
        JsonObject bootPartition = doc.createNestedObject("boot");
        bootPartition["label"] = boot->label;
        bootPartition["address"] = String(boot->address, HEX);
        bootPartition["size"] = boot->size;
    }
    
    // Partition de mise Ã  jour
    const esp_partition_t* update = esp_ota_get_next_update_partition(NULL);
    if (update) {
        JsonObject updatePartition = doc.createNestedObject("update");
        updatePartition["label"] = update->label;
        updatePartition["address"] = String(update->address, HEX);
        updatePartition["size"] = update->size;
    }
    
    // Informations filesystem
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    
    JsonObject fs = doc.createNestedObject("filesystem");
    fs["total"] = totalBytes;
    fs["used"] = usedBytes;
    fs["free"] = totalBytes - usedBytes;
    fs["usedPercent"] = (usedBytes * 100) / totalBytes;
    
    // Informations sketch
    doc["sketchSize"] = ESP.getSketchSize();
    doc["sketchMD5"] = ESP.getSketchMD5();
    doc["freeSketchSpace"] = ESP.getFreeSketchSpace();
    
    String output;
    serializeJson(doc, output);
    return output;
}

// Handler pour le endpoint de progression
void handleOTAProgress(WebServer* server) {
    LOG_D(LOG_OTA, "GET /api/ota/progress");
    server->send(200, "application/json", getProgressJSON());
}

// Handler pour le endpoint d'informations
void handleOTAInfo(WebServer* server) {
    LOG_D(LOG_OTA, "GET /api/ota/info");
    server->send(200, "application/json", getPartitionInfo());
}

// Handler pour l'upload du firmware
void handleFirmwareUpload(WebServer* server) {
    HTTPUpload& upload = server->upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        LOG_SEPARATOR();
        LOG_I(LOG_OTA, "FIRMWARE UPLOAD STARTED");
        LOG_I(LOG_OTA, "Filename: %s", upload.filename.c_str());
        
        // ===============================================================
        // CORRECTION V0.4: Réinitialiser les flags à chaque nouvel upload
        // ===============================================================
        otaResponseSent = false;
        otaUploadSuccess = false;
        lastError = "";
        
        // SOLUTION: Lire la taille depuis le paramÃ¨tre d'URL ?filesize=...
        size_t contentLength = 0;
        if (server->hasArg("filesize")) {
            contentLength = server->arg("filesize").toInt();
            LOG_D(LOG_OTA, "URL parameter filesize: %d bytes", contentLength);
        } else if (server->hasHeader("X-File-Size")) {
            // Fallback 1: essayer header custom (peut ne pas fonctionner)
            contentLength = server->header("X-File-Size").toInt();
            LOG_D(LOG_OTA, "X-File-Size header: %d bytes", contentLength);
        } else {
            // Fallback 2: essayer Content-Length (rarement disponible en multipart)
            contentLength = server->header("Content-Length").toInt();
            LOG_D(LOG_OTA, "Content-Length header: %d bytes", contentLength);
        }
        
        LOG_D(LOG_OTA, "upload.totalSize: %d (often unreliable!)", upload.totalSize);
        
        // Dernier recours: upload.totalSize
        if (contentLength == 0 && upload.totalSize > 0) {
            LOG_D(LOG_OTA, "Using upload.totalSize as fallback");
            contentLength = upload.totalSize;
        }
        
        if (!startFirmwareUpdate(contentLength)) {
            LOG_E(LOG_OTA, "Failed to initialize firmware update!");
            // Ne pas envoyer de rÃ©ponse ici, attendre UPLOAD_FILE_END
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
        LOG_D(LOG_OTA, "UPLOAD_FILE_WRITE: %d bytes", upload.currentSize);
        
        if (!writeOTAData(upload.buf, upload.currentSize)) {
            LOG_E(LOG_OTA, "Failed to write firmware chunk!");
            abortOTAUpdate();
            // Ne pas envoyer de rÃ©ponse ici, attendre UPLOAD_FILE_END
        }
    }
    else if (upload.status == UPLOAD_FILE_END) {
        LOG_I(LOG_OTA, "UPLOAD_FILE_END");
        LOG_D(LOG_OTA, "Total uploaded: %d bytes", upload.totalSize);
        
        // ===============================================================
        // CORRECTION V0.4: Ne PLUS envoyer de réponse HTTP ici !
        // ===============================================================
        // AVANT (v0.3): On envoyait server->send() ici + ESP.restart()
        // PROBLÈME: Le callback final essayait aussi d'envoyer une réponse
        // RÉSULTAT: Double Content-Length = ERR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH
        //
        // MAINTENANT: On met juste à jour le flag otaUploadSuccess
        // La réponse sera envoyée dans le callback final (lignes 491-524)
        // ===============================================================
        
        if (endOTAUpdate()) {
            LOG_I(LOG_OTA, "Firmware update SUCCESSFUL!");
            otaUploadSuccess = true;
            // NE PAS envoyer de réponse ici !
            // NE PAS redémarrer ici non plus !
        } else {
            LOG_E(LOG_OTA, "Failed to finalize firmware update!");
            otaUploadSuccess = false;
            // NE PAS envoyer de réponse ici non plus
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
        LOG_E(LOG_OTA, "UPLOAD_FILE_ABORTED - Client cancelled upload");
        abortOTAUpdate();
        otaUploadSuccess = false;
        lastError = "Upload aborted by client";
        // NE PAS envoyer de réponse ici non plus
    }
}

// Handler pour l'upload du filesystem
void handleFilesystemUpload(WebServer* server) {
    HTTPUpload& upload = server->upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        LOG_SEPARATOR();
        LOG_I(LOG_OTA, "FILESYSTEM UPLOAD STARTED");
        LOG_I(LOG_OTA, "Filename: %s", upload.filename.c_str());
        
        // ===============================================================
        // CORRECTION V0.4: Réinitialiser les flags à chaque nouvel upload
        // ===============================================================
        otaResponseSent = false;
        otaUploadSuccess = false;
        lastError = "";
        
        // SOLUTION: Lire la taille depuis le paramÃ¨tre d'URL ?filesize=...
        size_t contentLength = 0;
        if (server->hasArg("filesize")) {
            contentLength = server->arg("filesize").toInt();
            LOG_D(LOG_OTA, "URL parameter filesize: %d bytes", contentLength);
        } else if (server->hasHeader("X-File-Size")) {
            // Fallback 1: essayer header custom (peut ne pas fonctionner)
            contentLength = server->header("X-File-Size").toInt();
            LOG_D(LOG_OTA, "X-File-Size header: %d bytes", contentLength);
        } else {
            // Fallback 2: essayer Content-Length (rarement disponible)
            contentLength = server->header("Content-Length").toInt();
            LOG_D(LOG_OTA, "Content-Length header: %d bytes", contentLength);
        }
        
        LOG_D(LOG_OTA, "upload.totalSize: %d (often unreliable!)", upload.totalSize);
        
        // Dernier recours
        if (contentLength == 0 && upload.totalSize > 0) {
            LOG_D(LOG_OTA, "Using upload.totalSize as fallback");
            contentLength = upload.totalSize;
        }
        
        if (!startFilesystemUpdate(contentLength)) {
            LOG_E(LOG_OTA, "Failed to initialize filesystem update!");
            // Ne pas envoyer de rÃ©ponse ici
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
        LOG_D(LOG_OTA, "UPLOAD_FILE_WRITE: %d bytes", upload.currentSize);
        
        if (!writeOTAData(upload.buf, upload.currentSize)) {
            LOG_E(LOG_OTA, "Failed to write filesystem chunk!");
            abortOTAUpdate();
            // Ne pas envoyer de rÃ©ponse ici
        }
    }
    else if (upload.status == UPLOAD_FILE_END) {
        LOG_I(LOG_OTA, "UPLOAD_FILE_END");
        LOG_D(LOG_OTA, "Total uploaded: %d bytes", upload.totalSize);
        
        // ===============================================================
        // CORRECTION V0.4: Ne PLUS envoyer de réponse HTTP ici !
        // ===============================================================
        // Même principe que pour firmware: on met juste à jour le flag
        // La réponse sera envoyée dans le callback final (lignes 528-561)
        // ===============================================================
        
        if (endOTAUpdate()) {
            LOG_I(LOG_OTA, "Filesystem update SUCCESSFUL!");
            otaUploadSuccess = true;
            // NE PAS envoyer de réponse ici !
            // NE PAS redémarrer ici non plus !
        } else {
            LOG_E(LOG_OTA, "Failed to finalize filesystem update!");
            otaUploadSuccess = false;
            // NE PAS envoyer de réponse ici non plus
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED) {
        LOG_E(LOG_OTA, "UPLOAD_FILE_ABORTED - Client cancelled upload");
        abortOTAUpdate();
        otaUploadSuccess = false;
        lastError = "Upload aborted by client";
        // NE PAS envoyer de réponse ici non plus
    }
}

// Fonction pour initialiser les routes OTA
void setupOTARoutes(WebServer* server) {
    LOG_SEPARATOR();
    LOG_I(LOG_OTA, "SETTING UP OTA ROUTES");
    
    // Route pour les informations OTA
    server->on("/api/ota/info", HTTP_GET, [server]() {
        handleOTAInfo(server);
    });
    LOG_D(LOG_OTA, "Route registered: GET /api/ota/info");
    
    // Route pour la progression OTA
    server->on("/api/ota/progress", HTTP_GET, [server]() {
        handleOTAProgress(server);
    });
    LOG_D(LOG_OTA, "Route registered: GET /api/ota/progress");
    
    // Route pour l'upload du firmware
    // ===================================================================
    // CORRECTION V0.4: Envoi de la réponse HTTP unique
    // ===================================================================
    // Le premier callback (ci-dessous) est appelé APRÈS l'upload complet
    // C'est ici qu'on envoie la SEULE et UNIQUE réponse HTTP
    // Le deuxième callback gère l'upload chunk par chunk (handleFirmwareUpload)
    // ===================================================================
    server->on("/api/ota/firmware", HTTP_POST,
        [server]() {
            // ================================================================
            // CALLBACK FINAL - C'est ici qu'on envoie la réponse HTTP unique
            // ================================================================
            LOG_D(LOG_OTA, "POST /api/ota/firmware - Final callback");
            
            // Envoyer UNE SEULE réponse (le flag garantit qu'on ne l'envoie qu'une fois)
            if (!otaResponseSent) {
                otaResponseSent = true;
                
                if (otaUploadSuccess) {
                    // ✅ SUCCÈS: Envoyer réponse OK + redémarrer
                    LOG_I(LOG_OTA, "Sending success response, preparing to reboot...");
                    server->send(200, "application/json", 
                                 "{\"success\":true,\"message\":\"Firmware updated, rebooting...\"}");
                    
                    // IMPORTANT: Forcer l'envoi complet de la réponse HTTP
                    server->client().flush();
                    
                    // Attendre 3 secondes pour garantir que le client reçoit la réponse
                    LOG_I(LOG_OTA, "Waiting 3 seconds before reboot...");
                    delay(3000);
                    
                    LOG_I(LOG_OTA, "Rebooting NOW!");
                    ESP.restart();
                } else {
                    // ❌ ÉCHEC: Envoyer erreur
                    LOG_E(LOG_OTA, "Sending error response");
                    String errorJson = "{\"success\":false,\"error\":\"" + lastError + "\"}";
                    server->send(500, "application/json", errorJson);
                }
            }
        },
        [server]() {
            // Ce callback gère l'upload chunk par chunk
            handleFirmwareUpload(server);
        }
    );
    LOG_D(LOG_OTA, "Route registered: POST /api/ota/firmware");
    
    // Route pour l'upload du filesystem
    // ===================================================================
    // CORRECTION V0.4: Même principe que firmware (réponse unique)
    // ===================================================================
    server->on("/api/ota/filesystem", HTTP_POST,
        [server]() {
            // ================================================================
            // CALLBACK FINAL - Envoi de la réponse HTTP unique
            // ================================================================
            LOG_D(LOG_OTA, "POST /api/ota/filesystem - Final callback");
            
            // Envoyer UNE SEULE réponse
            if (!otaResponseSent) {
                otaResponseSent = true;
                
                if (otaUploadSuccess) {
                    // ✅ SUCCÈS: Envoyer réponse OK + redémarrer
                    LOG_I(LOG_OTA, "Sending success response, preparing to reboot...");
                    server->send(200, "application/json", 
                                 "{\"success\":true,\"message\":\"Filesystem updated, rebooting...\"}");
                    
                    // IMPORTANT: Forcer l'envoi complet de la réponse HTTP
                    server->client().flush();
                    
                    // Attendre 3 secondes pour garantir que le client reçoit la réponse
                    LOG_I(LOG_OTA, "Waiting 3 seconds before reboot...");
                    delay(3000);
                    
                    LOG_I(LOG_OTA, "Rebooting NOW!");
                    ESP.restart();
                } else {
                    // ❌ ÉCHEC: Envoyer erreur
                    LOG_E(LOG_OTA, "Sending error response");
                    String errorJson = "{\"success\":false,\"error\":\"" + lastError + "\"}";
                    server->send(500, "application/json", errorJson);
                }
            }
        },
        [server]() {
            // Ce callback gère l'upload chunk par chunk
            handleFilesystemUpload(server);
        }
    );
    LOG_D(LOG_OTA, "Route registered: POST /api/ota/filesystem");
    
    LOG_I(LOG_OTA, "OTA ROUTES INITIALIZED SUCCESSFULLY");
    LOG_SEPARATOR();
}

} // namespace OTAManager

#endif // OTA_MANAGER_H
