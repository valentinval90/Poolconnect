/**
 * Pool Connect - OTA Update Manager
 * Version 2.0 FINALE (04/02/2026) 
 */

// Etat global
let otaUpdateInProgress = false;
let otaProgressInterval = null;
let otaReconnectInterval = null;

// ACTIVER LES LOGS CONSOLE
const OTA_DEBUG = true;

function otaLog(level, ...args) {
    if (OTA_DEBUG) {
        const timestamp = new Date().toISOString().substr(11, 12);
        console.log(`[${timestamp}] [OTA-${level}]`, ...args);
    }
}

/**
 * Ouvre le modal OTA
 */
function openOTAModal() {
    otaLog('INFO', 'Opening OTA modal');
    const modal = document.getElementById('otaModal');
    if (modal) {
        modal.classList.add('active');
        loadOTAInfo();
    } else {
        otaLog('ERROR', 'OTA modal element not found!');
    }
}

/**
 * Ferme le modal OTA
 */
function closeOTAModal() {
    otaLog('INFO', 'Closing OTA modal');
    const modal = document.getElementById('otaModal');
    if (modal) {
        modal.classList.remove('active');
    }
}

/**
 * Charge les informations OTA
 */
async function loadOTAInfo() {
    otaLog('INFO', 'Loading OTA info...');
    try {
        const response = await fetch('/api/ota/info');
        otaLog('DEBUG', 'OTA info response status:', response.status);
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        otaLog('DEBUG', 'OTA info data:', data);
        
        // Partition courante
        if (data.current) {
            const el = document.getElementById('otaCurrentPartition');
            if (el) el.textContent = `${data.current.label} (${(data.current.size / 1024 / 1024).toFixed(2)} MB)`;
            otaLog('DEBUG', 'Current partition:', data.current.label);
        }
        
        // Partition de mise a jour
        if (data.update) {
            const el = document.getElementById('otaUpdatePartition');
            if (el) el.textContent = `${data.update.label} (${(data.update.size / 1024 / 1024).toFixed(2)} MB)`;
            otaLog('DEBUG', 'Update partition:', data.update.label);
        }
        
        // Informations sketch
        if (data.sketchSize) {
            const sketchMB = (data.sketchSize / 1024 / 1024).toFixed(2);
            const freeMB = (data.freeSketchSpace / 1024 / 1024).toFixed(2);
            const el = document.getElementById('otaSketchInfo');
            if (el) el.textContent = `${sketchMB} MB (${freeMB} MB libre)`;
            otaLog('DEBUG', `Sketch: ${sketchMB} MB, Free: ${freeMB} MB`);
        }
        
        // Informations filesystem
        if (data.filesystem) {
            const totalMB = (data.filesystem.total / 1024 / 1024).toFixed(2);
            const usedMB = (data.filesystem.used / 1024 / 1024).toFixed(2);
            const el = document.getElementById('otaFilesystemInfo');
            if (el) el.textContent = `${totalMB} MB (${usedMB} MB utilise, ${data.filesystem.usedPercent}%)`;
            otaLog('DEBUG', `Filesystem: ${totalMB} MB total, ${usedMB} MB used (${data.filesystem.usedPercent}%)`);
        }
        
        otaLog('INFO', 'OTA info loaded successfully');
    } catch (error) {
        otaLog('ERROR', 'Failed to load OTA info:', error);
        console.error('OTA: Failed to load info:', error);
    }
}

/**
 * Initialisation au chargement du DOM
 */
document.addEventListener('DOMContentLoaded', function() {
    otaLog('INFO', 'Initializing OTA module');
    
    const firmwareInput = document.getElementById('otaFirmwareFile');
    const filesystemInput = document.getElementById('otaFilesystemFile');
    
    if (firmwareInput) {
        firmwareInput.addEventListener('change', function(e) {
            otaLog('DEBUG', 'Firmware file selected');
            handleOTAFileSelect(e, 'firmware');
        });
    } else {
        otaLog('ERROR', 'Firmware input element not found!');
    }
    
    if (filesystemInput) {
        filesystemInput.addEventListener('change', function(e) {
            otaLog('DEBUG', 'Filesystem file selected');
            handleOTAFileSelect(e, 'filesystem');
        });
    } else {
        otaLog('ERROR', 'Filesystem input element not found!');
    }
    
    // Fermer le modal si clic en dehors
    const modal = document.getElementById('otaModal');
    if (modal) {
        modal.addEventListener('click', function(e) {
            if (e.target === modal) {
                closeOTAModal();
            }
        });
    }
    
    // Fermer avec Escape
    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape') {
            const modal = document.getElementById('otaModal');
            if (modal && modal.classList.contains('active')) {
                closeOTAModal();
            }
        }
    });
    
    otaLog('INFO', 'OTA module initialized');
});

/**
 * Gere la selection de fichier
 */
function handleOTAFileSelect(event, type) {
    const file = event.target.files[0];
    if (!file) {
        otaLog('WARN', 'No file selected');
        return;
    }
    
    otaLog('INFO', `File selected: ${file.name}, size: ${file.size} bytes, type: ${type}`);
    
    // Verifier l'extension
    if (!file.name.endsWith('.bin')) {
        otaLog('ERROR', 'Invalid file extension:', file.name);
        alert('⚠️ Veuillez selectionner un fichier .bin');
        event.target.value = '';
        return;
    }
    
    // Verifier la taille
    const maxSize = type === 'firmware' ? 3 * 1024 * 1024 : 10 * 1024 * 1024;
    if (file.size > maxSize) {
        otaLog('ERROR', `File too large: ${file.size} bytes (max: ${maxSize})`);
        alert(`⚠️ Fichier trop gros. Taille maximale: ${maxSize / 1024 / 1024} MB`);
        event.target.value = '';
        return;
    }
    
    // Afficher les infos du fichier
    const sizeMB = (file.size / 1024 / 1024).toFixed(2);
    const infoDiv = document.getElementById(type === 'firmware' ? 'otaFirmwareInfo' : 'otaFilesystemInfo');
    if (infoDiv) {
        infoDiv.textContent = `📁 Selectionne: ${file.name} (${sizeMB} MB)`;
        infoDiv.classList.add('show');
    }
    
    // Message adapte selon le type
    let message = `🚀 Telecharger la mise a jour ${type === 'firmware' ? 'Firmware' : 'Filesystem'}?\n\n`;
    message += `Fichier: ${file.name}\n`;
    message += `Taille: ${sizeMB} MB\n\n`;
    
    if (type === 'filesystem') {
        message += `⚠️ IMPORTANT:\n`;
        message += `L'upload du filesystem peut prendre 2-3 minutes.\n`;
        message += `L'ESP32 ne repondra pas pendant ce temps (c'est NORMAL).\n`;
        message += `NE PAS COUPER L'ALIMENTATION !\n\n`;
    }
    
    message += `L'appareil redemarrera apres la mise a jour.`;
    
    if (confirm(message)) {
        otaLog('INFO', 'User confirmed upload');
        uploadOTAFile(file, type);
    } else {
        otaLog('INFO', 'User cancelled upload');
        event.target.value = '';
        if (infoDiv) infoDiv.classList.remove('show');
    }
}

/**
 * Upload un fichier OTA
 */
async function uploadOTAFile(file, type) {
    if (otaUpdateInProgress) {
        otaLog('WARN', 'Update already in progress!');
        alert('⚠️ Une mise a jour est deja en cours');
        return;
    }
    
    otaLog('INFO', '========================================');
    otaLog('INFO', `Starting ${type.toUpperCase()} upload`);
    otaLog('INFO', `File: ${file.name}, Size: ${file.size} bytes`);
    
    otaUpdateInProgress = true;
    showOTAProgress(type);
    
    try {
        const formData = new FormData();
        formData.append('file', file);
        
        otaLog('DEBUG', 'FormData created, file appended');
        
        const endpoint = type === 'firmware' ? '/api/ota/firmware' : '/api/ota/filesystem';
        otaLog('INFO', `Endpoint: ${endpoint}`);
        
        // Pour le filesystem, on NE surveille PAS la progression (l'ESP32 sera bloque)
        if (type === 'filesystem') {
            otaLog('INFO', 'Filesystem upload - showing waiting message');
        } else {

            otaLog('INFO', 'Firmware upload - starting SIMULATED progress');
            
            const estimatedDuration = Math.max(15, Math.ceil(file.size / 50000));
            otaLog('INFO', `Estimated duration: ${estimatedDuration} seconds`);
            
            let simulatedPercentage = 0;
            const simulationStartTime = Date.now();
            
            otaProgressInterval = setInterval(() => {
                const elapsed = (Date.now() - simulationStartTime) / 1000;
                simulatedPercentage = Math.min(95, Math.floor((elapsed / estimatedDuration) * 100));
                
                const bar = document.getElementById('otaProgressBar');
                const text = document.getElementById('otaProgressText');
                const details = document.getElementById('otaProgressDetails');
                
                if (bar) {
                    bar.style.width = simulatedPercentage + '%';
                    bar.textContent = simulatedPercentage + '%';
                }
                
                if (text) {
                    text.textContent = `⏳ Upload firmware en cours... ${simulatedPercentage}%`;
                }
                
                if (details) {
                    const remainingSeconds = Math.max(0, Math.ceil(estimatedDuration - elapsed));
                    const currentKB = Math.floor((file.size * simulatedPercentage / 100) / 1024);
                    const totalKB = Math.floor(file.size / 1024);
                    details.textContent = `${currentKB} / ${totalKB} KB • ~${remainingSeconds}s restant`;
                }
                
                if (simulatedPercentage > 0 && simulatedPercentage % 10 === 0) {
                    otaLog('DEBUG', `Simulated progress: ${simulatedPercentage}%`);
                }
            }, 200);
        }
        
        // Upload avec timeout tres long pour le filesystem
        const timeoutMs = 180000; // 3 minutes max
        
        otaLog('DEBUG', 'Starting upload request...');
        otaLog('DEBUG', `File size: ${file.size} bytes`);
        
        // SOLUTION: Utiliser XMLHttpRequest pour avoir onprogress
        const urlWithSize = `${endpoint}?filesize=${file.size}`;
        otaLog('DEBUG', `Request URL: ${urlWithSize}`);
        const startTime = Date.now();
        
        // Créer une Promise qui wrap XMLHttpRequest
        const uploadPromise = new Promise((resolve, reject) => {
            const xhr = new XMLHttpRequest();
            let networkUploadComplete = false;  // Track la fin de l'upload réseau
            
            // Événement de progression de l'upload (côté réseau)
            xhr.upload.addEventListener('progress', (event) => {
                if (event.lengthComputable) {
                    const percentComplete = Math.round((event.loaded / event.total) * 100);
                    
                    // ============================================================
                    // CORRECTION: Différencier upload réseau vs écriture flash
                    // ============================================================
                    if (type === 'firmware') {
                        const bar = document.getElementById('otaProgressBar');
                        const text = document.getElementById('otaProgressText');
                        const details = document.getElementById('otaProgressDetails');
                        
                        if (!networkUploadComplete) {
                            // Phase 1: Upload réseau (rapide - quelques secondes)
                            if (bar) {
                                bar.style.width = percentComplete + '%';
                                bar.textContent = percentComplete + '%';
                            }
                            
                            if (text) {
                                text.textContent = `📤 Envoi des données... ${percentComplete}%`;
                            }
                            
                            if (details) {
                                const loadedMB = (event.loaded / 1024 / 1024).toFixed(2);
                                const totalMB = (event.total / 1024 / 1024).toFixed(2);
                                details.textContent = `${loadedMB} / ${totalMB} MB envoyés au réseau`;
                            }
                            
                            // Marquer la fin de l'upload réseau à 100%
                            if (percentComplete >= 100) {
                                networkUploadComplete = true;
                                otaLog('INFO', 'Network upload complete - ESP32 is now writing to flash...');
                                
                                // Remettre la barre à 0 pour la phase d'écriture flash
                                if (bar) {
                                    bar.style.width = '0%';
                                    bar.textContent = '0%';
                                }
                                
                                if (text) {
                                    text.textContent = '⏳ Écriture dans la mémoire flash... 0%';
                                }
                                
                                if (details) {
                                    details.textContent = 'Attente du début de l\'écriture...';
                                }
                            }
                        }
                        // ========================================================
                        // Une fois networkUploadComplete = true, on ne touche PLUS la barre ici
                        // C'est updateOTAProgress() qui prend le relais
                        // ========================================================
                        
                        // Log périodique de l'upload réseau
                        if (!networkUploadComplete && percentComplete % 25 === 0) {
                            otaLog('DEBUG', `Network upload: ${percentComplete}%`);
                        }
                    } else {
                        // Pour le filesystem, garder l'ancien comportement
                        const bar = document.getElementById('otaProgressBar');
                        const text = document.getElementById('otaProgressText');
                        const details = document.getElementById('otaProgressDetails');
                        
                        if (bar) {
                            bar.style.width = percentComplete + '%';
                            bar.textContent = percentComplete + '%';
                        }
                        
                        if (text) {
                            text.textContent = `⏳ Upload ${type} en cours...`;
                        }
                        
                        if (details) {
                            const loadedMB = (event.loaded / 1024 / 1024).toFixed(2);
                            const totalMB = (event.total / 1024 / 1024).toFixed(2);
                            const elapsed = ((Date.now() - startTime) / 1000).toFixed(1);
                            details.textContent = `${loadedMB} / ${totalMB} MB (${elapsed}s)`;
                        }
                        
                        // Log périodique
                        if (percentComplete % 10 === 0) {
                            otaLog('DEBUG', `Upload progress: ${percentComplete}%`);
                        }
                    }
                }
            });
            
            // Événement de fin
            xhr.addEventListener('load', () => {
                if (xhr.status === 200) {
                    try {
                        const result = JSON.parse(xhr.responseText);
                        resolve(result);
                    } catch (e) {
                        resolve({ success: true });
                    }
                } else {
                    reject(new Error(`HTTP ${xhr.status}: ${xhr.statusText}`));
                }
            });
            
            // Événement d'erreur
            xhr.addEventListener('error', () => {
                reject(new Error('Network error'));
            });
            
            // Événement de timeout
            xhr.addEventListener('timeout', () => {
                reject(new Error('Upload timeout'));
            });
            
            // Configurer et envoyer
            xhr.open('POST', urlWithSize, true);
            xhr.timeout = timeoutMs;
            xhr.send(formData);
        });
        
        // Attendre la fin de l'upload
        const result = await uploadPromise;
        
        const uploadDuration = ((Date.now() - startTime) / 1000).toFixed(2);
        otaLog('INFO', `Upload completed in ${uploadDuration}s`);
        otaLog('DEBUG', 'Response data:', result);
        
        // ================================================================
        // Arrêter la simulation et passer la barre à 100%
        // ================================================================
        if (type === 'firmware' && otaProgressInterval) {
            clearInterval(otaProgressInterval);
            otaProgressInterval = null;
            
            const bar = document.getElementById('otaProgressBar');
            const text = document.getElementById('otaProgressText');
            const details = document.getElementById('otaProgressDetails');
            
            if (bar) {
                bar.style.width = '100%';
                bar.textContent = '100%';
                bar.style.background = 'linear-gradient(90deg, var(--success), #229954)';
            }
            
            if (text) {
                text.textContent = '✅ Upload terminé avec succès !';
            }
            
            if (details) {
                const uploadDuration = ((Date.now() - startTime) / 1000).toFixed(1);
                details.textContent = `Upload complété en ${uploadDuration}s`;
            }
        }
        
        if (result.success) {
            otaLog('INFO', 'Upload successful!');
            
            // ================================================================
            // CORRECTION: Appeler la bonne fonction selon le type
            // ================================================================
            if (type === 'firmware') {
                // Pour firmware: attendre reconnexion
                waitForFirmwareReboot();
            } else {
                // Pour filesystem: garder l'ancien comportement
                waitForReconnection();
            }
        } else {
            throw new Error(result.error || 'Echec de la mise a jour');
        }
        
    } catch (error) {
        otaLog('ERROR', 'Upload failed:', error);
        console.error('OTA: Upload failed:', error);
        
        // ================================================================
        // Arrêter la simulation en cas d'erreur
        // ================================================================
        if (type === 'firmware' && otaProgressInterval) {
            clearInterval(otaProgressInterval);
            otaProgressInterval = null;
        }
        
        // Pour le filesystem, une erreur de fetch est NORMALE
        if (type === 'filesystem' && (error.name === 'TypeError' || error.name === 'Error')) {
            otaLog('INFO', 'Filesystem upload - connection error (expected for filesystem)');
            console.log('OTA: Connexion perdue (attendu pour filesystem)');
            waitForReconnection();
        } else {
            otaLog('ERROR', 'Unexpected error:', error.message);
            alert('❌ Echec de la mise a jour: ' + error.message);
            resetOTAState();
        }
    }
}

/**
 * Cette fonction gère la reconnexion et le rechargement automatique de la page
 */
function waitForFirmwareReboot() {
    otaLog('INFO', 'Firmware update complete, waiting for ESP32 reboot...');
    console.log('OTA: Attente du redémarrage de l\'ESP32...');
    
    const bar = document.getElementById('otaProgressBar');
    const text = document.getElementById('otaProgressText');
    const details = document.getElementById('otaProgressDetails');
    
    // Afficher le succès
    if (bar) {
        bar.style.width = '100%';
        bar.textContent = '100%';
        bar.style.background = 'linear-gradient(90deg, var(--success), #229954)';
    }
    
    if (text) {
        text.textContent = '✅ Firmware installé ! Redémarrage en cours...';
    }
    
    if (details) {
        details.innerHTML = `
            <div style="background: rgba(39,174,96,0.1); padding: 15px; border-radius: 8px; margin-top: 10px;">
                <p><strong>✅ Mise à jour firmware réussie !</strong></p>
                <p>L'ESP32 redémarre... Reconnexion dans <span id="reconnectCounter">10</span>s</p>
            </div>
        `;
    }
    
    // Compte à rebours
    let countdown = 10;
    const counterEl = document.getElementById('reconnectCounter');
    const countdownInterval = setInterval(() => {
        countdown--;
        if (counterEl) counterEl.textContent = countdown;
        if (countdown <= 0) clearInterval(countdownInterval);
    }, 1000);
    
    // Attendre 10 secondes puis essayer de reconnecter
    setTimeout(() => {
        otaLog('INFO', 'Starting reconnection checks...');
        checkReconnection();  // ← Cette fonction va recharger la page automatiquement
    }, 10000);
}

/**
 * Attend la reconnexion apres upload filesystem
 */
function waitForReconnection() {
    otaLog('INFO', 'Waiting for reconnection...');
    console.log('OTA: Attente de reconnexion...');
    
    let countdown = 30;
    const counterEl = document.getElementById('reconnectCounter');
    
    // Compte a rebours
    const countdownInterval = setInterval(() => {
        countdown--;
        if (counterEl) counterEl.textContent = countdown;
        if (countdown <= 0) clearInterval(countdownInterval);
    }, 1000);
    
    // Arreter la fausse progression
    if (window.otaFakeProgressInterval) {
        clearInterval(window.otaFakeProgressInterval);
    }
    
    // Attendre 30 secondes puis essayer de reconnecter
    setTimeout(() => {
        otaLog('INFO', 'Starting reconnection checks...');
        checkReconnection();
    }, 30000);
}

/**
 * Verifie si l'ESP32 a redemarre
 */
async function checkReconnection() {
    otaLog('INFO', 'Checking reconnection...');
    console.log('OTA: Verification de la reconnexion...');
    
    const text = document.getElementById('otaProgressText');
    const details = document.getElementById('otaProgressDetails');
    
    if (text) {
        text.textContent = '🔄 Verification de la reconnexion...';
    }
    
    let attempts = 0;
    const maxAttempts = 20;
    
    otaReconnectInterval = setInterval(async () => {
        attempts++;
        otaLog('DEBUG', `Reconnection attempt ${attempts}/${maxAttempts}`);
        
        if (details) {
            details.textContent = `Tentative ${attempts}/${maxAttempts}...`;
        }
        
        try {
            const response = await fetch('/api/ota/info', { 
                method: 'GET',
                cache: 'no-cache'
            });
            
            otaLog('DEBUG', `Reconnection attempt ${attempts}: status ${response.status}`);
            
            if (response.ok) {
                // Reconnecte !
                otaLog('INFO', 'Reconnection successful!');
                clearInterval(otaReconnectInterval);
                showSuccessAndReboot();
                
                // Recharger la page apres 3 secondes
                setTimeout(() => {
                    otaLog('INFO', 'Reloading page...');
                    window.location.reload();
                }, 3000);
            }
        } catch (error) {
            // Pas encore reconnecte
            otaLog('DEBUG', `Reconnection attempt ${attempts} failed:`, error.message);
            console.log(`OTA: Tentative ${attempts}/${maxAttempts}`);
        }
        
        if (attempts >= maxAttempts) {
            otaLog('ERROR', 'Max reconnection attempts reached');
            clearInterval(otaReconnectInterval);
            if (details) {
                details.innerHTML = `
                    <div style="text-align: center; padding: 20px; background: rgba(231,76,60,0.1); border-radius: 8px;">
                        <p><strong>⚠️ Impossible de se reconnecter</strong></p>
                        <p>L'ESP32 a probablement redemarre mais n'est pas accessible.</p>
                        <p>Verifiez votre reseau WiFi et rechargez la page manuellement.</p>
                        <button class="btn btn-primary" onclick="window.location.reload()">🔄 Recharger la page</button>
                    </div>
                `;
            }
            resetOTAState();
        }
    }, 2000); // Verifier toutes les 2 secondes
}

/**
 * Affiche le succes et le reboot
 */
function showSuccessAndReboot() {
    otaLog('INFO', 'Update successful!');
    
    const bar = document.getElementById('otaProgressBar');
    const text = document.getElementById('otaProgressText');
    const details = document.getElementById('otaProgressDetails');
    
    if (bar) {
        bar.style.width = '100%';
        bar.textContent = '100%';
        bar.style.background = 'linear-gradient(90deg, var(--success), #229954)';
    }
    
    if (text) {
        text.textContent = '✅ Mise a jour terminee avec succes !';
    }
    
    if (details) {
        details.innerHTML = `
            <div style="text-align: center; padding: 20px; background: rgba(39,174,96,0.1); border-radius: 8px;">
                <p><strong>✅ Mise a jour reussie !</strong></p>
                <p>L'ESP32 a redemarre avec succes.</p>
                <p>🔄 Rechargement de la page dans 3 secondes...</p>
            </div>
        `;
    }
}

/**
 * Reinitialise l'etat OTA
 */
function resetOTAState() {
    otaLog('DEBUG', 'Resetting OTA state');
    otaUpdateInProgress = false;
    
    // Arrêter tous les intervalles
    if (otaProgressInterval) {
        clearInterval(otaProgressInterval);
        otaProgressInterval = null;
    }
    
    if (otaReconnectInterval) {
        clearInterval(otaReconnectInterval);
        otaReconnectInterval = null;
    }
    
    if (window.otaFakeProgressInterval) {
        clearInterval(window.otaFakeProgressInterval);
    }
    
    const firmwareInput = document.getElementById('otaFirmwareFile');
    const filesystemInput = document.getElementById('otaFilesystemFile');
    
    if (firmwareInput) firmwareInput.value = '';
    if (filesystemInput) filesystemInput.value = '';
    
    const firmwareInfo = document.getElementById('otaFirmwareInfo');
    const filesystemInfo = document.getElementById('otaFilesystemInfo');
    if (firmwareInfo) firmwareInfo.classList.remove('show');
    if (filesystemInfo) filesystemInfo.classList.remove('show');
}

/**
 * Affiche la zone de progression
 */
function showOTAProgress(type) {
    otaLog('DEBUG', `Showing progress container for ${type}`);
    
    const container = document.getElementById('otaProgressContainer');
    if (container) {
        container.style.display = 'block';
        
        const text = document.getElementById('otaProgressText');
        if (text) {
            const typeLabel = type === 'firmware' ? 'firmware' : 'filesystem';
            text.textContent = `⏳ Preparation de la mise a jour ${typeLabel}...`;
        }
        
        const bar = document.getElementById('otaProgressBar');
        if (bar) {
            bar.style.width = '0%';
            bar.textContent = '0%';
        }
        
        const details = document.getElementById('otaProgressDetails');
        if (details) {
            details.textContent = '';
        }
    } else {
        otaLog('ERROR', 'Progress container not found!');
    }
}

/**
 * Demarre le monitoring de progression (uniquement pour firmware)
 */
function startOTAProgressMonitoring() {
    otaLog('DEBUG', 'Starting progress monitoring');
    otaProgressInterval = setInterval(updateOTAProgress, 500);
}

/**
 * Arrete le monitoring
 */
function stopOTAProgressMonitoring() {
    otaLog('DEBUG', 'Stopping progress monitoring');
    if (otaProgressInterval) {
        clearInterval(otaProgressInterval);
        otaProgressInterval = null;
    }
}

/**
 * Met a jour la progression (uniquement pour firmware)
 */
async function updateOTAProgress() {
    try {
        const response = await fetch('/api/ota/progress', {
            cache: 'no-cache'
        });
        
        if (!response.ok) {
            otaLog('WARN', `Progress fetch failed: ${response.status}`);
            return;
        }
        
        const data = await response.json();
        otaLog('DEBUG', 'Progress data received:', data);
        
        // ================================================================
        // DIAGNOSTIC: Afficher les données reçues dans l'interface
        // ================================================================
        const details = document.getElementById('otaProgressDetails');
        if (details) {
            const now = new Date().toLocaleTimeString();
            details.innerHTML = `<div style="font-family: monospace; font-size: 11px;">
                [${now}] inProgress: ${data.inProgress}, percentage: ${data.percentage}%, 
                current: ${(data.currentSize/1024).toFixed(0)}KB, 
                total: ${(data.totalSize/1024).toFixed(0)}KB
            </div>`;
        }
        // ================================================================
        
        if (data.inProgress) {
            const bar = document.getElementById('otaProgressBar');
            const text = document.getElementById('otaProgressText');
            
            otaLog('DEBUG', `Updating progress bar to ${data.percentage}%`);
            
            // Mettre à jour la barre avec la vraie progression de l'ESP32
            if (bar) {
                bar.style.width = data.percentage + '%';
                bar.textContent = data.percentage + '%';
                otaLog('DEBUG', `Bar updated: width=${bar.style.width}`);
            } else {
                otaLog('ERROR', 'Progress bar element not found!');
            }
            
            // Message cohérent pour la phase d'écriture flash
            if (text) {
                text.textContent = `⏳ Écriture dans la mémoire flash... ${data.percentage}%`;
            }
            
            // Log périodique
            if (data.percentage % 10 === 0) {
                otaLog('INFO', `Flash write progress: ${data.percentage}%`);
            }
        } else {
            otaLog('WARN', 'Progress data received but inProgress=false');
            if (details) {
                details.innerHTML = `<div style="color: orange; font-family: monospace;">
                    ⚠️ inProgress=false (ESP32 dit que l'écriture n'est pas en cours)
                </div>`;
            }
        }
    } catch (error) {
        otaLog('ERROR', 'Error in updateOTAProgress:', error);
        const details = document.getElementById('otaProgressDetails');
        if (details) {
            details.innerHTML = `<div style="color: red; font-family: monospace;">
                ❌ Erreur: ${error.message}
            </div>`;
        }
    }
}
