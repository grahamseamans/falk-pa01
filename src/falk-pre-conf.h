#ifndef INITIAL_SETUP
#define INITIAL_SETUP

#include "Arduino.h"
#include <Preferences.h>

// ====== MINIMUM AND MAXIMUM VOLUME ====== //

#define VOL_MIN                 0
#define VOL_MAX                 63  // Changed to 6-bit for RelaiXed Passive (GPB0-GPB5)

// ====== HOW MANY INPUTS ====== //

#define INP_MIN                 1
#define INP_MAX                 4

// ====== DISPLAY BEHAVIOR ====== //

#define SCREEN_TIMEOUT          10000

// ====== BUTTON SETTINGS ====== //

#define BUTTON_DEBOUNCE_DELAY   50

// ====== FLASH WEAR OUT PREVENTION ====== //

#define COMMIT_TIMEOUT          3000

// ====== DEFINE PINS ====== //

// WiFi button removed

#define SCREEN_CS               5
#define SCREEN_DC               15
#define SCREEN_RST              13

#define MUTE_BUTTON             34  // Input button - used for mute and IR programming

// Broken hardware (not used)
#define INP_ENCODER_A           14  // Broken
#define INP_ENCODER_B           27  // Broken

// Working hardware (input encoder used for volume)
#define VOL_ENCODER_A           26  // Input encoder A - used for volume  
#define VOL_ENCODER_B           25  // Input encoder B - used for volume

#define POWER_BUTTON            GPIO_NUM_33
#define POWER_CONTROL           GPIO_NUM_32

#define IR_RECEIVER_PIN         12

// ====== SETTINGS (INCL. DEFAULTS) ====== //
typedef struct {
  String name;
  String icon = "disc";
  int enabled = 1;
} DeviceInput;

typedef struct {
  int16_t volume = 26;
  int16_t input = 1;
  DeviceInput inputs [INP_MAX];
  uint16_t saved = 0;
  int dim = 1;
  int maxVol = VOL_MAX;
  int maxStartVol = VOL_MAX;
  short absoluteVol = 0;
  int muted = 0;
  // IR remote codes
  uint32_t irVolUp = 0;
  uint32_t irVolDown = 0;
  uint32_t irInputUp = 0;
  uint32_t irInputDown = 0;
  uint32_t irRepeat = 0;
} DeviceSettings;

extern DeviceSettings sysSettings;

extern int FlashCommit;

// FIRMWARE VERSION (THIS SW)
extern String fw_version;

#include "display-ssd1322.h"
extern Display display;


#include "ESP32Encoder.h"
// for the volume rotary encoder
extern ESP32Encoder volEnc;

// for the input rotary encoder
extern ESP32Encoder inpEnc;

void saveSettings();
void restoreSettings();
void applySettingsChanges(const DeviceSettings& oldSettings, const DeviceSettings& newSettings);
#endif