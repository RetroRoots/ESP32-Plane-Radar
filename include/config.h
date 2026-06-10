#pragma once

#include <cstdint>

#include <driver/gpio.h>

namespace config {

// --- Wi-Fi portal ---
constexpr char kPortalApName[] = "PlaneRadar-Setup";
constexpr char kPortalIp[] = "192.168.4.1";
/** mDNS host (no ".local" suffix); browser: http://plane-radar.local */
constexpr char kPortalHostname[] = "plane-radar";
constexpr char kPortalHostUrl[] = "plane-radar.local";

/** Per-attempt STA connect wait (ms); retried kWifiConnectAttempts times. */
constexpr unsigned long kWifiConnectAttemptMs = 15000;
constexpr uint8_t kWifiConnectAttempts = 3;
constexpr unsigned long kWifiPortalTimeoutSec = 0;  // 0 = no timeout while configuring
constexpr unsigned long kWifiConnectingFrameMs = 50;
/** Wait after disconnect before reconnecting (avoids portal on brief drops). */
constexpr unsigned long kWifiDownGraceMs = 4000;
/** Minimum interval between background reconnect tries. */
constexpr unsigned long kWifiReconnectIntervalMs = 15000;

// --- BOOT button (ESP32-C3 Super Mini, active LOW) ---
constexpr gpio_num_t kBootPin = GPIO_NUM_0;
constexpr unsigned long kBootResetHoldMs = 3000UL;
/** Ignore BOOT taps shorter than this (debounce). */
constexpr unsigned long kBootTapMinMs = 40UL;

// --- Display: GC9A01 1.28" round 240×240 (SPI) ---
constexpr gpio_num_t kDisplayPinRst = GPIO_NUM_14;
constexpr gpio_num_t kDisplayPinCs = GPIO_NUM_9;
constexpr gpio_num_t kDisplayPinDc = GPIO_NUM_8;
constexpr gpio_num_t kDisplayPinMosi = GPIO_NUM_11;  // display SDA
constexpr gpio_num_t kDisplayPinSclk = GPIO_NUM_10;  // display SCL
constexpr gpio_num_t kDisplayPinBl = GPIO_NUM_2;     // display backlight

constexpr int kDisplayWidth = 240;
constexpr int kDisplayHeight = 240;

constexpr uint32_t kDisplaySpiWriteHz = 40000000;
// GC9A01 modules often need invert + BGR for correct black/green output
constexpr bool kDisplayInvert = true;
constexpr bool kDisplayRgbOrder = true;

// --- Radar center defaults (overridden via WiFi setup portal) ---
constexpr double kDefaultRadarLat = 52.3676;
constexpr double kDefaultRadarLon = 4.9041;

/** Poll adsb.fi (API public limit: 1 req/s). */
constexpr unsigned long kAdsbFetchIntervalMs = 3000;
/** Legacy scale unused — fetch uses radar::fetchRadiusKm() to screen edge. */
constexpr float kAdsbFetchRadiusScale = 1.0f;
/** false = hide aircraft with alt_baro "ground"; true = show them too. */
constexpr bool kAdsbShowGroundAircraft = false;

// --- UI colors (RGB565) — status screens ---
constexpr uint16_t kColorBlack = 0x0000;
constexpr uint16_t kColorYellow = 0xFFE0;
constexpr uint16_t kTextOnYellow = kColorBlack;
constexpr uint16_t kTextOnBlack = 0xFFFF;

// --- Airports ---
struct Airport {
  const char* code;
  double lat;
  double lon;
};

constexpr Airport kAirports[] = {
  {"LHR", 51.4700, -0.4543}, // London Heathrow
  {"LGW", 51.1481, -0.1903}, // London Gatwick
  {"MAN", 53.3539, -2.2750}, // Manchester
  {"STN", 51.8850, 0.2350},  // London Stansted
  {"LTN", 51.8747, -0.3683}, // London Luton
  {"BHX", 52.4539, -1.7481}, // Birmingham
  {"BRS", 51.3828, -2.7192}, // Bristol
  {"NCL", 55.0375, -1.6917}, // Newcastle
  {"LBA", 53.8659, -1.6606}, // Leeds Bradford
  {"EMA", 52.8311, -1.3281}, // East Midlands
  {"LCY", 51.5053, 0.0553},  // London City
  {"SOU", 50.9503, -1.3567}, // Southampton
  {"BOH", 50.7800, -1.8297}, // Bournemouth
  {"LPL", 53.3336, -2.8497}, // Liverpool John Lennon
  {"NWI", 52.6758, 1.2828},  // Norwich
  {"EXT", 50.7344, -3.4139}, // Exeter
  {"SEN", 51.5714, 0.6956},  // London Southend
  {"MME", 54.5092, -1.4294}, // Teesside
  {"NQY", 50.4406, -4.9953}, // Newquay / Cornwall
  {"HUY", 53.5744, -0.3508}  // Humberside
};
constexpr size_t kAirportCount = sizeof(kAirports) / sizeof(kAirports[0]);

}  // namespace config
