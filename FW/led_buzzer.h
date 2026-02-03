/* 
 * POOL CONNECT - LED & BUZZER
 * Gestion de la LED de statut et du buzzer
 * led_buzzer.h   V0.2
 */

#ifndef LED_BUZZER_H
#define LED_BUZZER_H

#include <Arduino.h>
#include <FastLED.h>
#include "globals.h"
#include "config.h"
#include "logging.h"

// ============================================================================
// LED
// ============================================================================

void setLEDStatus(LEDStatus status) {
  const char* statusNames[] = {
    "BOOTING",
    "WIFI_CONNECTING",
    "WIFI_CONNECTED",
    "ERROR",
    "RUNNING",
    "ALARM"
  };
  
  LOG_D(LOG_SYSTEM, "Changement de statut LED: %s", statusNames[status]);
  
  switch(status) {
    case LED_BOOTING:
      leds[0] = CRGB::Blue;
      LOG_V(LOG_SYSTEM, "LED: Bleu fixe (Demarrage)");
      break;
    case LED_WIFI_CONNECTING:
      leds[0] = millis() % 500 < 250 ? CRGB::Blue : CRGB::Black;
      LOG_V(LOG_SYSTEM, "LED: Bleu clignotant (Connexion WiFi)");
      break;
    case LED_WIFI_CONNECTED:
      leds[0] = CRGB::Green;
      LOG_V(LOG_SYSTEM, "LED: Vert fixe (WiFi connecte)");
      break;
    case LED_ERROR:
      leds[0] = CRGB::Red;
      LOG_E(LOG_SYSTEM, "LED: Rouge fixe (ERREUR)");
      break;
    case LED_RUNNING:
      leds[0] = CRGB::Cyan;
      LOG_V(LOG_SYSTEM, "LED: Cyan fixe (Systeme en marche)");
      break;
    case LED_ALARM:
      leds[0] = millis() % 400 < 200 ? CRGB::Red : CRGB::Black;
      LOG_W(LOG_SYSTEM, "LED: Rouge clignotant (ALARME)");
      break;
  }
  FastLED.show();
}

void ledActivity() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  static bool firstCall = true;
  
  if (firstCall) {
    LOG_V(LOG_SYSTEM, "LED: Mode activite (clignotement vert toutes les 2s)");
    firstCall = false;
  }
  
  if (millis() - lastBlink > 2000) {
    leds[0] = ledState ? CRGB::Green : CRGB::Black;
    FastLED.show();
    ledState = !ledState;
    lastBlink = millis();
  }
}

// ============================================================================
// BUZZER
// ============================================================================

void buzzerBeep(int count = 1) {
  if (buzzerMuted) {
    LOG_V(LOG_SYSTEM, "Buzzer desactive (mute)");
    return;
  }
  
  if (!sysConfig.buzzerEnabled) {
    LOG_V(LOG_SYSTEM, "Buzzer desactive (configuration)");
    return;
  }
  
  LOG_D(LOG_SYSTEM, "Buzzer: %d bip(s)", count);
  
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < count - 1) delay(100);
  }
  
  LOG_V(LOG_SYSTEM, "Buzzer: sequence terminee");
}

void buzzerAlarm() {
  if (buzzerMuted) {
    LOG_W(LOG_SYSTEM, "ALARME: Buzzer desactive (mute) - Signal sonore non emis");
    return;
  }
  
  if (!sysConfig.buzzerEnabled) {
    LOG_W(LOG_SYSTEM, "ALARME: Buzzer desactive (configuration) - Signal sonore non emis");
    return;
  }
  
  LOG_W(LOG_SYSTEM, "ALARME BUZZER: 3 bips longs");
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
  
  LOG_W(LOG_SYSTEM, "Alarme buzzer terminee");
}

#endif // LED_BUZZER_H