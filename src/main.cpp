/**
 * Plane Radar — WiFi setup, then radar UI on the round GC9A01 display.
 */

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "hardware/display.h"
#include "services/adsb_client.h"
#include "services/radar_location.h"
#include "services/radar_rotation.h"
#include "services/web_settings.h"
#include "services/wifi_setup.h"
#include "ui/radar_display.h"
#include "ui/radar_range.h"
#include "ui/status_screens.h"

namespace {

bool g_radar_visible = false;
unsigned long g_wifi_down_since = 0;
unsigned long g_last_reconnect_ms = 0;
unsigned long g_last_adsb_fetch_ms = 0;

// --- Touch & IP Display State ---
constexpr gpio_num_t kTouchIntPin = GPIO_NUM_5;
constexpr gpio_num_t kTouchRstPin = GPIO_NUM_13;
volatile bool s_touch_tap_pending = false;
unsigned long g_ip_display_until_ms = 0;
String g_local_ip_str;

void IRAM_ATTR onTouchIsr() {
  s_touch_tap_pending = true;
}

void touchInit() {
  pinMode(kTouchRstPin, OUTPUT);
  digitalWrite(kTouchRstPin, LOW);
  delay(10);
  digitalWrite(kTouchRstPin, HIGH);
  delay(50);
  pinMode(kTouchIntPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(kTouchIntPin), onTouchIsr, FALLING);
}

void showRadarIfConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    g_radar_visible = false;
    return;
  }
  ui::radarDisplayDraw();
  g_radar_visible = true;
}

void onRangeTap() {
  ui::radar::rangeNext();
  char range_label[12];
  ui::radar::formatCurrentRing3Label(range_label, sizeof(range_label));
  Serial.printf("Range: %s (outer ~%.0f km)\n", range_label,
                ui::radar::rangeCurrent().outer_km);

  if (g_radar_visible && WiFi.status() == WL_CONNECTED) {
    ui::radarDisplayDraw();
  }
}

void handleBootButton() {
  bootButtonPollLongPress();
  if (bootButtonConsumeTap()) {
    onRangeTap();
  }
}

void fetchAndDrawAircraft() {
  const float fetch_km = ui::radar::fetchRadiusKm();
  if (!services::adsb::fetchUpdate(services::location::lat(),
                                   services::location::lon(), fetch_km)) {
    handleBootButton();
    return;
  }
  ui::radarDisplayRefreshAircraft();
  handleBootButton();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Plane Radar");

  // Enable backlight for Waveshare ESP32-S3 Touch LCD 1.28
  pinMode(config::kDisplayPinBl, OUTPUT);
  digitalWrite(config::kDisplayPinBl, HIGH);

  bootButtonInit();
  displayInit();
  touchInit();
  if (wifiShowsSetupScreenOnBoot()) {
    statusScreenPortal();
  }
  services::location::init();
  services::radar_rotation::init();
  ui::radar::rangeInit();

  if (wifiSetupConnect()) {
    services::web_settings::init();
    showRadarIfConnected();
  }
}

void loop() {
  if (s_touch_tap_pending) {
    s_touch_tap_pending = false;
    // Only trigger if radar is running and IP isn't already showing
    if (WiFi.status() == WL_CONNECTED && g_ip_display_until_ms == 0) {
      g_local_ip_str = "IP: " + WiFi.localIP().toString();
      g_ip_display_until_ms = millis() + 5000;
      g_radar_visible = false;
      statusScreenConnectingBegin(g_local_ip_str.c_str());
    }
  }

  if (g_ip_display_until_ms > 0) {
    if (millis() >= g_ip_display_until_ms) {
      g_ip_display_until_ms = 0;
      if (WiFi.status() == WL_CONNECTED) {
        showRadarIfConnected(); // 5 seconds passed, redraw radar
      }
    }
    services::web_settings::handle(); // Keep background web server alive
    delay(10);
    return; // Suspend normal radar UI updates
  }

  handleBootButton();

  if (WiFi.status() != WL_CONNECTED) {
    if (g_radar_visible) {
      Serial.println("WiFi lost — will reconnect");
      g_radar_visible = false;
    }

    if (g_wifi_down_since == 0) {
      g_wifi_down_since = millis();
    }

    const unsigned long down_ms = millis() - g_wifi_down_since;
    if (down_ms >= config::kWifiDownGraceMs &&
        millis() - g_last_reconnect_ms >= config::kWifiReconnectIntervalMs) {
      g_last_reconnect_ms = millis();
      if (wifiReconnect()) {
        g_wifi_down_since = 0;
        showRadarIfConnected();
      }
    }
  } else {
    g_wifi_down_since = 0;
    services::web_settings::handle();
    if (!g_radar_visible) {
      showRadarIfConnected();
    } else if (millis() - g_last_adsb_fetch_ms >= config::kAdsbFetchIntervalMs) {
      g_last_adsb_fetch_ms = millis();
      fetchAndDrawAircraft();
    }
  }

  delay(10);
}
