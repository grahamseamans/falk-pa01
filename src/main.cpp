/*
Simple Preamp Controller
- Direct relay control
- OLED display
- Rotary encoders for input/volume
- IR remote control with learning mode
- Serial flashing only
*/
#include <Arduino.h>
#include <driver/gpio.h>
#include <Wire.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// OTHER INTERNAL CLASSES

// SYSTEM CONFIGURATION
#include "config.h"
#include "hardware-config.h"
#include "realtime-state.h"

#include "relay-input.h"
InputController input;

#include "relay-volume.h"
VolumeController volume;

#include "display-ssd1322.h"
Display display;

#include "ESP32Encoder.h"
ESP32Encoder volEnc;  // for the volume rotary encoder
ESP32Encoder inpEnc;  // for the input rotary encoder (broken)

// WiFi removed - simple preamp now

// to handle mute state
int muteButtonState;
int lastmuteButtonState = HIGH;
int muteDebounceTime = 0;

// track encoder state for delta detection
int lastVolumeEncoderValue = 0;

// IR receiver
IRrecv irrecv(IR_RECEIVER_PIN);
decode_results irResults;

// IR programming mode
enum IRProgState {
  IR_NORMAL,
  IR_PROG_VOL_UP,
  IR_PROG_VOL_DOWN, 
  IR_PROG_INPUT_UP,
  IR_PROG_INPUT_DOWN,
  IR_PROG_REPEAT,
  IR_PROG_COMPLETE
};
IRProgState irProgState = IR_NORMAL;
unsigned long irProgTimeout = 0;
#define IR_REPEAT_SAMPLES 20
uint32_t repeatCodeHistory[IR_REPEAT_SAMPLES];
int repeatCodeCount = 0;

// Volume button long press detection
unsigned long volumeButtonPressTime = 0;
int lastVolumeButtonState = HIGH;
int volumeButtonState = HIGH;
// Button timing moved to config.h

// Track last IR code for repeat handling
uint32_t lastIRCode = 0;

// Function to find most frequent code in the repeat history
uint32_t findMostFrequentCode(uint32_t* codes, int count) {
  uint32_t mostFrequent = 0;
  int maxCount = 0;
  
  for (int i = 0; i < count; i++) {
    int frequency = 0;
    for (int j = 0; j < count; j++) {
      if (codes[i] == codes[j]) {
        frequency++;
      }
    }
    
    if (frequency > maxCount) {
      maxCount = frequency;
      mostFrequent = codes[i];
    }
  }
  
  Serial.printf("Most frequent code: 0x%08X (appeared %d times out of %d)\n", 
                mostFrequent, maxCount, count);
  return mostFrequent;
}

// OTA update removed - use serial flashing

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Booting...");

  // start the display controller
  display.begin();

  // setup the power control elements
  pinMode(POWER_CONTROL, OUTPUT);                // this is the power control for the 5V circuit
  digitalWrite(POWER_CONTROL, HIGH);             // set it high to power the 5V elements
  pinMode(POWER_BUTTON, INPUT_PULLDOWN);         // configure the power button
  esp_sleep_enable_ext0_wakeup(POWER_BUTTON, 1); // let the power button wake the MCU

  delay(250);

  // configure the MCP27013 ICs
  input.begin(INP_MAX);
  volume.begin(VOL_MIN, VOL_MAX);

  // turn everything off, this gives us a chance to correctly set the volume to the max startup volume, if set
  input.set(0);
  delay(5);

  initPreferences();
  restoreSettings();

  // use the pullup resistors, this means we can connect ground to the encoders
  ESP32Encoder::useInternalWeakPullResistors = UP;

  // input encoder is broken - not configured

  // configure the volume encoder (actually input encoder hardware)
  volEnc.attachFullQuad(VOL_ENCODER_A, VOL_ENCODER_B);
  lastVolumeEncoderValue = volEnc.getCount();

  // configure the mute button
  pinMode(MUTE_BUTTON, INPUT);

  // the relays *should* match our stored values (since they're latching) but we can't be sure
  // so we set them to these values so the screen and relays match
  // relays.set(sysSettings.volume);
  volume.set(sysSettings.volume);
  input.set(sysSettings.input);

  // initialize IR receiver
  irrecv.enableIRIn();
  Serial.println("IR receiver enabled on pin " + String(IR_RECEIVER_PIN));

  // update the display
  display.updateScreen();
}

void muteLoop(int m, DeviceSettings* settings)
{
  int reading = digitalRead(MUTE_BUTTON);

  // Handle button press/release timing for mute functionality
  if (reading != lastVolumeButtonState) {
    if (reading == LOW) {
      volumeButtonPressTime = m;
    } else {
      // Button released
      if (volumeButtonPressTime > 0) {
        unsigned long pressDuration = m - volumeButtonPressTime;
        
        if (irProgState != IR_NORMAL) {
          // Exit IR programming mode
          irProgState = IR_NORMAL;
          Serial.println("Exiting IR programming mode");
          // Display update will be handled by applySettingsChanges
        } else if (pressDuration < BUTTON_LONG_PRESS_TIME) {
          // Short press - toggle mute (only if we didn't already enter IR programming)
          settings->muted = !settings->muted;
          if (!settings->muted) {
            // When unmuting, sync encoder position
            lastVolumeEncoderValue = volEnc.getCount();
          }
        }
        // Long press already handled while button was held down
      }
      volumeButtonPressTime = 0;
    }
    lastVolumeButtonState = reading;
  }
  
  // Check for long press while button is still held down
  if (reading == LOW && volumeButtonPressTime > 0 && irProgState == IR_NORMAL) {
    unsigned long pressDuration = m - volumeButtonPressTime;
    if (pressDuration >= BUTTON_LONG_PRESS_TIME) {
      // Long press detected while holding - enter IR programming mode
      irProgState = IR_PROG_VOL_UP;
      irProgTimeout = m + IR_PROGRAMMING_TIMEOUT;
      Serial.println("Entering IR programming mode");
      display.irProgMode("Press VOL UP on remote");
      volumeButtonPressTime = 0; // Prevent retriggering
    }
  }
}

void irProgLoop(int m)
{
  // Check for IR programming timeout
  if (irProgState != IR_NORMAL && m > irProgTimeout) {
    irProgState = IR_NORMAL;
    Serial.println("IR programming timed out");
    // Display update will be handled by applySettingsChanges
  }
}

void irLoop(int m, DeviceSettings* settings) {
  if (irrecv.decode(&irResults)) {
    uint32_t code = irResults.value;
    Serial.printf("IR received: 0x%08X (lastIRCode: 0x%08X, repeat: 0x%08X)\n", 
                  code, lastIRCode, settings->irRepeat);
    
    if (irProgState != IR_NORMAL) {
      // In programming mode - learn the code
      if (code != 0) {
        // For repeat learning, accept any code including 0xFFFFFFFF
        // For button learning, ignore obvious repeat codes
        bool shouldLearn = (irProgState == IR_PROG_REPEAT) || (code != 0xFFFFFFFF);
        
        if (shouldLearn) {
          switch (irProgState) {
          case IR_PROG_VOL_UP:
            settings->irVolUp = code;
            irProgState = IR_PROG_VOL_DOWN;
            display.irProgMode("Press VOL DOWN on remote");
            Serial.printf("Learned VOL UP: 0x%08X\n", code);
            break;
            
          case IR_PROG_VOL_DOWN:
            settings->irVolDown = code;
            irProgState = IR_PROG_INPUT_UP;
            display.irProgMode("Press INPUT UP on remote");
            Serial.printf("Learned VOL DOWN: 0x%08X\n", code);
            break;
            
          case IR_PROG_INPUT_UP:
            settings->irInputUp = code;
            irProgState = IR_PROG_INPUT_DOWN;
            display.irProgMode("Press INPUT DOWN on remote");
            Serial.printf("Learned INPUT UP: 0x%08X\n", code);
            break;
            
          case IR_PROG_INPUT_DOWN:
            settings->irInputDown = code;
            irProgState = IR_PROG_REPEAT;
            repeatCodeCount = 0; // Reset counter
            display.irProgMode("HOLD any button for repeat code");
            Serial.printf("Learned INPUT DOWN: 0x%08X\n", code);
            Serial.println("Now collecting 20 codes for repeat pattern...");
            break;
            
          case IR_PROG_REPEAT:
            // Collect codes for histogram analysis
            if (repeatCodeCount < IR_REPEAT_SAMPLES) {
              repeatCodeHistory[repeatCodeCount] = code;
              repeatCodeCount++;
              Serial.printf("Collected code %d/%d: 0x%08X\n", repeatCodeCount, IR_REPEAT_SAMPLES, code);
              
              if (repeatCodeCount == IR_REPEAT_SAMPLES) {
                // We have enough codes, find the most frequent one
                settings->irRepeat = findMostFrequentCode(repeatCodeHistory, IR_REPEAT_SAMPLES);
                irProgState = IR_NORMAL;
                saveSettings();
                display.irProgComplete();
                Serial.printf("Learned REPEAT: 0x%08X\n", settings->irRepeat);
                Serial.println("IR programming complete!");
              } else {
                // Update display with progress
                char progressMsg[50];
                sprintf(progressMsg, "Collecting codes: %d/%d", repeatCodeCount, IR_REPEAT_SAMPLES);
                display.irProgMode(progressMsg);
              }
            }
            break;
            
          default:
            break;
          }
        }
      }
    } else {
      // Normal operation - process IR commands 
      if (code == settings->irRepeat && settings->irRepeat != 0) {
        // Repeat code - only repeat if last action was volume
        if (lastIRCode == settings->irVolUp) {
          int newVol = settings->volume + 1;
          if (newVol <= VOL_MAX) settings->volume = newVol;
          Serial.println("IR: Volume up (repeat)");
        } else if (lastIRCode == settings->irVolDown) {
          int newVol = settings->volume - 1;
          if (newVol >= VOL_MIN) settings->volume = newVol;
          Serial.println("IR: Volume down (repeat)");
        }
        // Otherwise ignore repeat (for input buttons)
      } else if (code == settings->irVolUp && code != 0) {
        int newVol = settings->volume + 1;
        if (newVol <= VOL_MAX) settings->volume = newVol;
        Serial.println("IR: Volume up");
        lastIRCode = code;
      } else if (code == settings->irVolDown && code != 0) {
        int newVol = settings->volume - 1;
        if (newVol >= VOL_MIN) settings->volume = newVol;
        Serial.println("IR: Volume down");
        lastIRCode = code;
      } else if (code == settings->irInputUp && code != 0) {
        int newInput = settings->input + 1;
        if (newInput <= INP_MAX) {
          settings->input = newInput;
          Serial.printf("IR: Input up to %d\n", newInput);
        } else {
          Serial.println("IR: Input up - already at maximum input");
        }
        lastIRCode = code;
      } else if (code == settings->irInputDown && code != 0) {
        int newInput = settings->input - 1;
        if (newInput >= INP_MIN) {
          settings->input = newInput;
          Serial.printf("IR: Input down to %d\n", newInput);
        } else {
          Serial.println("IR: Input down - already at minimum input");
        }
        lastIRCode = code;
      }
    }
    
    irrecv.resume(); // Ready for next IR signal
  }
}


// Input encoder is broken - input switching only via IR remote

void volumeLoop(int m, DeviceSettings* settings)
{
  if (settings->muted == 0)
  {
    // Check for encoder movement
    int currentEncoderValue = volEnc.getCount();
    int delta = currentEncoderValue - lastVolumeEncoderValue;

    if (delta != 0)
    {
      // Encoder moved, update volume by the delta
      int newVol = settings->volume + delta;
      
      // Constrain to hardware limits
      if (newVol < VOL_MIN) newVol = VOL_MIN;
      if (newVol > VOL_MAX) newVol = VOL_MAX;
      
      settings->volume = newVol;
      lastVolumeEncoderValue = currentEncoderValue;
    }
  }
  volume.loop();
}

int lastPowerButtonState = LOW;
int powerButtonState = LOW;
int powerDebounceTime = 0;
void powerLoop(int m)
{
  int reading = digitalRead(POWER_BUTTON);

  if (reading != lastPowerButtonState)
  {
    powerDebounceTime = m;
  }

  if ((m - powerDebounceTime) > BUTTON_DEBOUNCE_DELAY)
  {
    if (reading != powerButtonState)
    {
      powerButtonState = reading;

      if (powerButtonState == HIGH)
      {
        // need to power down...
        Serial.println("Going into sleep mode...");

        // power off the 5V circuit
        digitalWrite(POWER_CONTROL, LOW);
        // put the display to sleep
        display.off();
        // wait quarter second (helps with debounce, otherwise the MCU wakes again...)
        delay(250);
        // zzzzzzzzzz
        esp_deep_sleep_start();
      }
    }
  }
  lastPowerButtonState = reading;
}

void loop()
{
  int m = millis();
  
  // Store old settings for comparison
  DeviceSettings oldSettings = sysSettings;

  // Each function modifies only the settings struct
  volumeLoop(m, &sysSettings);
  muteLoop(m, &sysSettings);
  irLoop(m, &sysSettings);
  irProgLoop(m);
  powerLoop(m);

  // Apply any changes to hardware/display/flash inline
  bool needsFlashSave = false;
  bool needsDisplayUpdate = false;
  
  // Check for volume changes
  if (oldSettings.volume != sysSettings.volume) {
    // Implement break-before-make relay switching to prevent pops
    byte oldBits = (byte)(oldSettings.volume & 0x3F);
    byte newBits = (byte)(sysSettings.volume & 0x3F);
    
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
    volume.set(sysSettings.volume);
    
    needsDisplayUpdate = true;
    needsFlashSave = true;
    Serial.printf("Volume changed: %d->%d (0x%02X->0x%02X)\n", 
                  oldSettings.volume, sysSettings.volume, oldBits, newBits);
  }
  
  // Check for input changes  
  if (oldSettings.input != sysSettings.input) {
    input.set(sysSettings.input);
    needsDisplayUpdate = true;
    needsFlashSave = true;
    Serial.printf("Input changed to: %d\n", sysSettings.input);
  }
  
  // Check for mute state changes
  if (oldSettings.muted != sysSettings.muted) {
    if (sysSettings.muted) {
      volEnc.pauseCount();
      volume.writeBits(0x00);  // Mute by turning off all volume relays
      Serial.println("Muted");
    } else {
      volEnc.resumeCount();
      volume.writeBits(sysSettings.volume & 0x3F);  // Restore actual volume
      Serial.println("Unmuted");
    }
    needsDisplayUpdate = true;
    needsFlashSave = true;  // Save mute state
  }
  
  // Update display if anything changed that affects it
  if (needsDisplayUpdate) {
    display.updateScreen();
  }
  
  // Trigger flash save if any persistent setting changed
  if (needsFlashSave) {
    FlashCommit = m;
  }

  // Handle delayed flash saves
  if ((FlashCommit > 0) && (m > FlashCommit + AUTO_SAVE_DELAY))
  {
    saveSettings();
    FlashCommit = 0;
  }

  display.loop();
}