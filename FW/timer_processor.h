/* 
 * POOL CONNECT - TIMER PROCESSOR
 * Déclarations pour le traitement des timers
 */

#ifndef TIMER_PROCESSOR_H
#define TIMER_PROCESSOR_H

#include <Arduino.h>
#include <time.h>

// Fonction principale de traitement des timers
// Implémentation dans le fichier .ino principal
void processFlexTimers(struct tm* timeinfo);

#endif // TIMER_PROCESSOR_H
