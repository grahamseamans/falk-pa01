/*
Simple Preamp Controller
- Direct relay control
- OLED display
- Rotary encoders for input/volume
- Serial flashing only
*/
#include <Arduino.h>
#include <driver/gpio.h>
#include <Wire.h>

// OTHER INTERNAL CLASSES

// OUR SYSTEM CONFIGURATION
#include "falk-pre-conf.h"

#include "relay-input.h"
InputController input;

#include "relay-volume.h"
VolumeController volume;

// WiFi removed - simple preamp now

// to handle mute state
int muteButtonState;
int lastmuteButtonState = HIGH;
int muteDebounceTime = 0;

// track encoder state for delta detection
int lastVolumeEncoderValue = 0;

// WiFi removed

// OTA update removed - use serial flashing

void setup()
{
  Serial.begin(9600);
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

  restoreSettings();

  // add some default values if this is our first boot
  if (sysSettings.saved == 0)
  {
    sysSettings.saved = 1;
    for (int i = 0; i < INP_MAX; i++)
    {
      sysSettings.inputs[i].name = "Input " + (String)(i + 1);
    }
  }

  if (sysSettings.volume > sysSettings.maxStartVol)
  {
    sysSettings.volume = sysSettings.maxStartVol;
  }

  // use the pullup resistors, this means we can connect ground to the encoders
  ESP32Encoder::useInternalWeakPullResistors = UP;

  // configure the input encoder
  inpEnc.attachSingleEdge(INP_ENCODER_A, INP_ENCODER_B);
  inpEnc.setCount(sysSettings.input);

  // configure the volume encoder
  volEnc.attachFullQuad(VOL_ENCODER_A, VOL_ENCODER_B);
  lastVolumeEncoderValue = volEnc.getCount();

  // configure the mute button
  pinMode(MUTE_BUTTON, INPUT);

  // the relays *should* match our stored values (since they're latching) but we can't be sure
  // so we set them to these values so the screen and relays match
  // relays.set(sysSettings.volume);
  volume.set(sysSettings.volume);
  input.set(sysSettings.input);

  // update the display
  display.updateScreen();
}

void muteLoop(int m)
{
  int reading = digitalRead(MUTE_BUTTON);

  if (reading != lastmuteButtonState)
  {
    muteDebounceTime = m;
  }

  if ((m - muteDebounceTime) > BUTTON_DEBOUNCE_DELAY)
  {
    if (reading != muteButtonState)
    {
      muteButtonState = reading;

      if (muteButtonState == LOW)
      {
        if (muteState == 0)
        {
          muteState = 1;
          volEnc.pauseCount();
          // write to the screen
          display.updateScreen();
          // mute state changed
        }
        else
        {
          muteState = 0;
          volEnc.resumeCount();
          lastVolumeEncoderValue = volEnc.getCount(); // Reset encoder tracking
          // write to the screen
          display.updateScreen();
          // set the relays
          volume.set(sysSettings.volume);
          // unmute state changed
        }
      }
    }
  }
  lastmuteButtonState = reading;
}

// WiFi loop removed

void setVolume(int newVol)
{
  // Constrain to hardware limits
  if (newVol < VOL_MIN)
    newVol = VOL_MIN;
  if (newVol > VOL_MAX)
    newVol = VOL_MAX;

  Serial.print("setVolume: Setting to ");
  Serial.println(newVol);

  // Update the single source of truth
  sysSettings.volume = newVol;

  // Update all outputs
  volume.set(newVol);               // Hardware relays
  display.updateScreen();           // OLED display

  // Trigger delayed flash commit
  FlashCommit = millis();
}

void inputLoop(int m)
{
  int count = inpEnc.getCount(); // get count from rotary encoder
  if (count != sysSettings.input)
  { // if it's not our current setting
    if (count > INP_MAX)
    { // make sure it's not out of bounds (upper, if so, set to min)
      count = INP_MIN;
    }
    else if (count < INP_MIN)
    { // make sure it's not out of bounds (lower, if so, set to max)
      count = INP_MAX;
    }
    while (sysSettings.inputs[count - 1].enabled == false)
    {          // is the input enabled? If not...
      count++; // check the next one
      if (count > INP_MAX)
      { // are we out of bounds? go to the min
        count = INP_MIN;
      }
    }
    inpEnc.setCount(count);    // set the encoder (failsafe)
    input.set(count);          // set the input
    sysSettings.input = count; // update system settings
    display.updateScreen();    // paint the screen
    // set a delayed commit (this prevents us from wearing out the flash with each detent)
    FlashCommit = m;
  }
  input.loop();
}

void volumeLoop(int m)
{
  if (muteState == 0)
  {
    // Check for encoder movement
    int currentEncoderValue = volEnc.getCount();
    int delta = currentEncoderValue - lastVolumeEncoderValue;

    if (delta != 0)
    {
      // Encoder moved, update volume by the delta
      setVolume(sysSettings.volume + delta);
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

  inputLoop(m);
  volumeLoop(m);
  muteLoop(m);
  // WiFi removed
  powerLoop(m);

  if ((FlashCommit > 0) && (m > FlashCommit + COMMIT_TIMEOUT))
  {
    saveSettings();
    FlashCommit = 0;
  }

  display.loop();
}