/* 
 * POOL CONNECT - LED & BUZZER
 * Gestion de la LED de statut et du buzzer
 */

#ifndef LED_BUZZER_H
#define LED_BUZZER_H

#include <Arduino.h>
#include <FastLED.h>
#include "globals.h"

// ============================================================================
// LED
// ============================================================================

void setLEDStatus(LEDStatus status) {
  switch(status) {
    case LED_BOOTING:
      leds[0] = CRGB::Blue;
      break;
    case LED_WIFI_CONNECTING:
      leds[0] = millis() % 500 < 250 ? CRGB::Blue : CRGB::Black;
      break;
    case LED_WIFI_CONNECTED:
      leds[0] = CRGB::Green;
      break;
    case LED_ERROR:
      leds[0] = CRGB::Red;
      break;
    case LED_RUNNING:
      leds[0] = CRGB::Cyan;
      break;
    case LED_ALARM:
      leds[0] = millis() % 400 < 200 ? CRGB::Red : CRGB::Black;
      break;
  }
  FastLED.show();
}

void ledActivity() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
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
  if (buzzerMuted || !sysConfig.buzzerEnabled) return;
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < count - 1) delay(100);
  }
}

void buzzerAlarm() {
  if (buzzerMuted || !sysConfig.buzzerEnabled) return;
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

#endif // LED_BUZZER_H
