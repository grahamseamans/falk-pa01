#include "realtime-state.h"

// ====== GLOBAL STATE VARIABLES ====== //
DeviceSettings sysSettings;
Preferences preferences;

// ====== DEBUGGING SEQUENCE TRACKING ====== //
static uint32_t saveSequence = 0;
static uint32_t bootSequence = 0;

// ====== DEBUG PRINTING FUNCTION ====== //
void debugPrintSettings(const DeviceSettings* settings, const char* context) {
  Serial.printf("DEBUG [%s]: volume=%d, input=%d, muted=%d, irVolUp=0x%08X, irVolDown=0x%08X, irInputUp=0x%08X, irInputDown=0x%08X, irRepeat=0x%08X\n",
                context, settings->volume, settings->input, settings->muted, 
                settings->irVolUp, settings->irVolDown, settings->irInputUp, 
                settings->irInputDown, settings->irRepeat);
}

// ====== ATOMIC STATE STRUCTURE ====== //
struct AtomicState {
  uint32_t magic = 0x46414C4B;  // "FALK" in ASCII
  uint32_t version = SETTINGS_VERSION;  // Struct version for compatibility
  uint32_t checksum;            // Data integrity check
  DeviceSettings data;          // Actual settings data
};

// ====== CHECKSUM CALCULATION ====== //
uint32_t calculateChecksum(const DeviceSettings* settings) {
  // Simple XOR checksum of all 32-bit words in the settings
  uint32_t checksum = 0;
  const uint32_t* data = (const uint32_t*)settings;
  size_t words = sizeof(DeviceSettings) / sizeof(uint32_t);
  
  for (size_t i = 0; i < words; i++) {
    checksum ^= data[i];
  }
  
  return checksum;
}

// ====== ATOMIC SAVE/RESTORE FUNCTIONS ====== //

void initPreferences() {
  Serial.println("INIT: Opening preferences namespace 'falk-pre'");
  if (!preferences.begin("falk-pre", false)) {
    Serial.println("INIT: FAILED to open preferences namespace!");
  } else {
    Serial.println("INIT: Preferences namespace opened successfully");
  }
}

void saveSettings() {
  saveSequence++;
  Serial.printf("==== SAVE SEQUENCE #%d ====\n", saveSequence);
  
  // Debug print what we're trying to save
  debugPrintSettings(&sysSettings, "SAVE-ATTEMPT");
  
  // Prepare atomic state structure
  AtomicState state;
  state.data = sysSettings;
  state.checksum = calculateChecksum(&state.data);
  
  Serial.printf("SAVE: Attempting to save volume=%d, input=%d, magic=0x%08X, version=%d, checksum=0x%08X\n",
                state.data.volume, state.data.input, state.magic, state.version, state.checksum);
  
  // Single atomic write (preferences should already be open)
  size_t written = preferences.putBytes("state", &state, sizeof(state));
  
  if (written == sizeof(state)) {
    Serial.printf("SAVE: putBytes() reported success - %d bytes written\n", written);
    
    // IMMEDIATE READ-BACK VERIFICATION
    AtomicState readback;
    size_t readbackSize = preferences.getBytes("state", &readback, sizeof(readback));
    
    if (readbackSize == sizeof(readback)) {
      Serial.printf("SAVE: Read-back successful - %d bytes read\n", readbackSize);
      debugPrintSettings(&readback.data, "SAVE-READBACK");
      
      // Verify readback matches what we tried to save
      if (memcmp(&state, &readback, sizeof(state)) == 0) {
        Serial.println("SAVE: Read-back MATCHES save data - SUCCESS");
      } else {
        Serial.println("SAVE: Read-back DIFFERS from save data - CORRUPTION DETECTED");
        Serial.printf("SAVE: Original checksum=0x%08X, readback checksum=0x%08X\n", 
                      state.checksum, readback.checksum);
      }
    } else {
      Serial.printf("SAVE: Read-back FAILED - got %d bytes, expected %d\n", readbackSize, sizeof(readback));
    }
  } else {
    Serial.printf("SAVE: putBytes() FAILED: wrote %d of %d bytes\n", written, sizeof(state));
  }
  
  Serial.printf("==== END SAVE SEQUENCE #%d ====\n", saveSequence);
}

void restoreSettings() {
  bootSequence++;
  Serial.printf("==== BOOT SEQUENCE #%d ====\n", bootSequence);
  
  AtomicState state;
  bool stateValid = false;
  
  // Try to load atomic state (preferences should already be open)
  size_t readSize = preferences.getBytesLength("state");
  
  if (readSize == sizeof(AtomicState)) {
    size_t actualRead = preferences.getBytes("state", &state, sizeof(state));
    
    Serial.printf("LOAD: Read %d bytes, magic=0x%08X, version=%d, checksum=0x%08X, volume=%d, input=%d\n",
                  actualRead, state.magic, state.version, state.checksum, state.data.volume, state.data.input);
    
    debugPrintSettings(&state.data, "BOOT-LOAD");
    
    uint32_t expectedChecksum = calculateChecksum(&state.data);
    Serial.printf("LOAD: Expected checksum=0x%08X, calculated checksum=0x%08X\n", state.checksum, expectedChecksum);
    
    if (actualRead == sizeof(state) && 
        state.magic == 0x46414C4B &&
        state.version == SETTINGS_VERSION &&
        state.checksum == calculateChecksum(&state.data)) {
      stateValid = true;
      Serial.printf("LOAD: Valid atomic state loaded - volume=%d, input=%d\n", state.data.volume, state.data.input);
    } else {
      if (state.magic != 0x46414C4B) {
        Serial.printf("LOAD: State corrupted - bad magic number: got 0x%08X, expected 0x46414C4B\n", state.magic);
      } else if (state.version != SETTINGS_VERSION) {
        Serial.printf("LOAD: Settings version mismatch: saved=%d, current=%d - using defaults\n", 
                      state.version, SETTINGS_VERSION);
      } else {
        Serial.println("LOAD: State corrupted - checksum mismatch");
      }
    }
  } else if (readSize == 0) {
    Serial.println("LOAD: No saved settings found (first boot)");
  } else {
    Serial.printf("LOAD: State size mismatch: expected %d, got %d\n", sizeof(AtomicState), readSize);
  }
  
  if (stateValid) {
    // Use loaded state
    sysSettings = state.data;
    debugPrintSettings(&sysSettings, "RESTORE-SUCCESS");
  } else {
    // Use config defaults - first boot or corrupted state
    Serial.println("LOAD: Using default settings");
    sysSettings = {};  // Uses struct defaults from realtime-state.h
    debugPrintSettings(&sysSettings, "RESTORE-DEFAULTS");
  }
  
  Serial.printf("==== END BOOT SEQUENCE #%d ====\n", bootSequence);
}