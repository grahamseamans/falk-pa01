#include "falk-pre-conf.h"
#include "relay-volume.h"
#include "relay-input.h"

// External references to controller instances
extern VolumeController volume;
extern InputController input;

DeviceSettings sysSettings;

String fw_version = "0.3.14";

Preferences preferences;
int FlashCommit = 0;


//the following need to be widely available since we need to be able to access them from wifi.cpp
Display display;

// for the volume rotary encoder
ESP32Encoder volEnc;

// for the input rotary encoder
ESP32Encoder inpEnc;

void saveSettings() {
  // Save settings using Preferences API directly
  preferences.putUShort("volume", sysSettings.volume);
  preferences.putUShort("input", sysSettings.input);
  preferences.putUShort("saved", sysSettings.saved);
  preferences.putBool("dim", sysSettings.dim);
  preferences.putUShort("maxVol", sysSettings.maxVol);
  preferences.putUShort("maxStartVol", sysSettings.maxStartVol);
  preferences.putBool("absoluteVol", sysSettings.absoluteVol);
  
  // Save IR codes
  preferences.putUInt("irVolUp", sysSettings.irVolUp);
  preferences.putUInt("irVolDown", sysSettings.irVolDown);
  preferences.putUInt("irInputUp", sysSettings.irInputUp);
  preferences.putUInt("irInputDown", sysSettings.irInputDown);
  preferences.putUInt("irRepeat", sysSettings.irRepeat);
  
  // Save inputs
  for (int i = 0; i < INP_MAX; i++) {
    String key = "inp" + String(i);
    preferences.putString((key + "_name").c_str(), sysSettings.inputs[i].name);
    preferences.putString((key + "_icon").c_str(), sysSettings.inputs[i].icon);
    preferences.putBool((key + "_en").c_str(), sysSettings.inputs[i].enabled);
  }
}

void restoreSettings() {
  //start preferences
  preferences.begin("falk-pre", false);
  //preferences.clear();

  // Load settings using Preferences API directly
  sysSettings.volume = preferences.getUShort("volume", 26);
  sysSettings.input = preferences.getUShort("input", 1);
  sysSettings.saved = preferences.getUShort("saved", 0);
  sysSettings.dim = preferences.getBool("dim", true);
  sysSettings.maxVol = preferences.getUShort("maxVol", VOL_MAX);
  sysSettings.maxStartVol = preferences.getUShort("maxStartVol", VOL_MAX);
  sysSettings.absoluteVol = preferences.getBool("absoluteVol", false);
  
  // Load IR codes
  sysSettings.irVolUp = preferences.getUInt("irVolUp", 0);
  sysSettings.irVolDown = preferences.getUInt("irVolDown", 0);
  sysSettings.irInputUp = preferences.getUInt("irInputUp", 0);
  sysSettings.irInputDown = preferences.getUInt("irInputDown", 0);
  sysSettings.irRepeat = preferences.getUInt("irRepeat", 0);
  
  // Load inputs
  for(int i=0; i<INP_MAX; i++) {
    String key = "inp" + String(i);
    sysSettings.inputs[i].name = preferences.getString((key + "_name").c_str(), "Input " + String(i + 1));
    sysSettings.inputs[i].icon = preferences.getString((key + "_icon").c_str(), "disc");
    sysSettings.inputs[i].enabled = preferences.getBool((key + "_en").c_str(), true);
  }
}

void applySettingsChanges(const DeviceSettings& oldSettings, const DeviceSettings& newSettings) {
  bool needsFlashSave = false;
  bool needsDisplayUpdate = false;
  
  // Check for volume changes
  if (oldSettings.volume != newSettings.volume) {
    // Implement break-before-make relay switching to prevent pops
    byte oldBits = (byte)(oldSettings.volume & 0x3F);
    byte newBits = (byte)(newSettings.volume & 0x3F);
    
    // Calculate which bits are changing
    byte bitsToTurnOff = oldBits & ~newBits;  // Was 1, becoming 0
    
    // Step 1: Turn off bits that need to turn off (break)
    if (bitsToTurnOff != 0) {
      volume.writeBits(oldBits & ~bitsToTurnOff);  // Remove bits turning off
      delay(5);  // Brief delay to let relays settle
    }
    
    byte bitsToTurnOn = ~oldBits & newBits;   // Was 0, becoming 1
    
    // Step 2: Turn on bits that need to turn on (make)
    if (bitsToTurnOn != 0) {
      volume.writeBits(newBits);  // Write final target pattern
    }
    
    // Step 3: Final sync to prevent drift
    volume.set(newSettings.volume);
    
    needsDisplayUpdate = true;
    needsFlashSave = true;
    Serial.printf("Volume changed: %d->%d (0x%02X->0x%02X)\n", 
                  oldSettings.volume, newSettings.volume, oldBits, newBits);
  }
  
  // Check for input changes  
  if (oldSettings.input != newSettings.input) {
    input.set(newSettings.input);
    needsDisplayUpdate = true;
    needsFlashSave = true;
    Serial.print("Input changed to: ");
    Serial.println(newSettings.input);
  }
  
  // Check for mute state changes
  if (oldSettings.muted != newSettings.muted) {
    if (newSettings.muted) {
      volEnc.pauseCount();
      volume.writeBits(0x00);  // Mute by turning off all volume relays
      Serial.println("Muted");
    } else {
      volEnc.resumeCount();
      volume.writeBits(newSettings.volume & 0x3F);  // Restore actual volume
      Serial.println("Unmuted");
    }
    needsDisplayUpdate = true;
    // Mute state doesn't need flash save - it's transient
  }
  
  // Update display if anything changed that affects it
  if (needsDisplayUpdate) {
    display.updateScreen();
  }
  
  // Trigger flash save if any persistent setting changed
  if (needsFlashSave) {
    FlashCommit = millis();
  }
}