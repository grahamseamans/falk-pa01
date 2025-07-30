#ifndef CONFIG_H
#define CONFIG_H

// ====== USER CONFIGURABLE SETTINGS ====== //
// Edit these values and reflash to customize your preamp

// ====== INPUT CONFIGURATION ====== //
#define INPUT_NAMES {"Input 1", "Input 2", "Input 3", "Input 4"}
#define INPUT_DEFAULT 1

// ====== DISPLAY CONFIGURATION ====== //
#define DISPLAY_ABSOLUTE false         // true = show 0-63, false = show percentage
#define DISPLAY_DIM_ENABLED true       // auto-dim after timeout
#define DISPLAY_DIM_TIMEOUT 10000      // ms - how long before screen dims
#define DISPLAY_OFF_TIMEOUT 180000     // ms - how long before screen turns off completely (3 minutes)

// ====== BUTTON CONFIGURATION ====== //
#define BUTTON_DEBOUNCE_DELAY 50       // ms - button debounce time
#define BUTTON_LONG_PRESS_TIME 2000    // ms - long press for IR programming

// ====== IR REMOTE CONFIGURATION ====== //
#define IR_PROGRAMMING_TIMEOUT 30000   // ms - timeout for IR programming mode

// ====== VOLUME CONFIGURATION ====== //
#define VOLUME_DEFAULT 26              // default startup volume

// ====== SYSTEM CONFIGURATION ====== //
#define AUTO_SAVE_DELAY 3000           // ms - delay before saving state to flash

#endif // CONFIG_H