#ifndef REALTIME_STATE_H
#define REALTIME_STATE_H

#include "Arduino.h"
#include <Preferences.h>
#include "config.h"

// ====== SETTINGS VERSION ====== //
// IMPORTANT: Increment this when making breaking changes to DeviceSettings struct
// This ensures incompatible settings are reset to defaults after firmware update
#define SETTINGS_VERSION 1

// ====== RUNTIME STATE STRUCTURE ====== //
typedef struct {
  // Core state that changes during use
  int16_t volume = VOLUME_DEFAULT;
  int16_t input = INPUT_DEFAULT;
  int muted = 0;
  
  // Learned IR remote codes
  uint32_t irVolUp = 0;
  uint32_t irVolDown = 0;
  uint32_t irInputUp = 0;
  uint32_t irInputDown = 0;
  uint32_t irRepeat = 0;
} DeviceSettings;

// ====== GLOBAL STATE VARIABLES ====== //
extern DeviceSettings sysSettings;

// ====== STATE SAVE/RESTORE FUNCTIONS ====== //
void initPreferences();
void saveSettings();
void restoreSettings();

#endif // REALTIME_STATE_H