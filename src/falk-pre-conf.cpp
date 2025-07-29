#include "falk-pre-conf.h"

DeviceSettings sysSettings;

String fw_version = "0.3.14";

Preferences preferences;
int FlashCommit = 0;

int muteState = 0;

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
  
  // Load inputs
  for(int i=0; i<INP_MAX; i++) {
    String key = "inp" + String(i);
    sysSettings.inputs[i].name = preferences.getString((key + "_name").c_str(), "Input " + String(i + 1));
    sysSettings.inputs[i].icon = preferences.getString((key + "_icon").c_str(), "disc");
    sysSettings.inputs[i].enabled = preferences.getBool((key + "_en").c_str(), true);
  }
}