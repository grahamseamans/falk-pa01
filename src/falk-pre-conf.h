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

#define COMMIT_TIMEOUT          400

// ====== DEFINE PINS ====== //

// WiFi button removed

#define SCREEN_CS               5
#define SCREEN_DC               15
#define SCREEN_RST              13

#define MUTE_BUTTON             35  // Swapped: INPUT button for mute (broken but won't be used)

#define INP_ENCODER_A           14
#define INP_ENCODER_B           27

#define VOL_ENCODER_A           26
#define VOL_ENCODER_B           25

#define POWER_BUTTON            GPIO_NUM_33
#define POWER_CONTROL           GPIO_NUM_32

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
  // WiFi settings removed
} DeviceSettings;

extern DeviceSettings sysSettings;

extern int FlashCommit;

// FIRMWARE VERSION (THIS SW)
extern String fw_version;

#include "display-ssd1322.h"
extern Display display;

extern int muteState;

#include "ESP32Encoder.h"
// for the volume rotary encoder
extern ESP32Encoder volEnc;

// for the input rotary encoder
extern ESP32Encoder inpEnc;

void saveSettings();
void restoreSettings();
void setVolume(int newVol);
#endif