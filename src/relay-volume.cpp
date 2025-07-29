#include <Arduino.h>
#include <Wire.h>

#include "relay-volume.h"

int VolumeRelayPulseTime = 0;
short vol_max = 0;
short vol_min = 0;

void VolumeController::volEncLoop(int m) {
  // Non-latching relays don't need pulse timing
  // Volume stays set until changed
}

void VolumeController::begin(short min, short max) {
  vol_min = min;
  vol_max = max;

  Wire.begin();

  // configure volume
  Wire.beginTransmission(MCP_VOLUME_ADDRESS);  //select the first device (volume relays)
  Wire.write(MCP_PORTA);  //select the "A" bank
  Wire.write(MCP_OUTPUT);  //set to outputs
  Wire.endTransmission();  //kill the connection

  Wire.beginTransmission(MCP_VOLUME_ADDRESS);
  Wire.write(MCP_PORTB);  //select the "B" bank
  Wire.write(MCP_OUTPUT);  //set to outputs
  Wire.endTransmission();  //kill the connection
}

void VolumeController::set(int volume) {
  // Constrain volume to 6-bit range (0-63)
  if (volume < vol_min) volume = vol_min;
  if (volume > vol_max) volume = vol_max;
  
  // RelaiXed Passive uses GPB0-GPB5 (6 bits) for volume control
  // Convert volume (0-63) to 6-bit pattern
  byte volumeBits = (byte)(volume & 0x3F);  // Mask to 6 bits
  
  
  // Send 6-bit volume to Port B pins 0-5
  Wire.beginTransmission(MCP_VOLUME_ADDRESS);
  Wire.write(MCP_PORTB_PINS);
  Wire.write(volumeBits);
  byte result = Wire.endTransmission();
  
  if (result != 0) {
    Serial.print("I2C error: ");
    Serial.println(result);
  }
  
  VolumeRelayPulseTime = 0;
}

void VolumeController::endPulse() {
  // Not needed for non-latching relays - they stay in position
  VolumeRelayPulseTime = 0;
}

void VolumeController::writeBits(byte bits) {
  // Write the specified bit pattern to the volume relays
  Wire.beginTransmission(MCP_VOLUME_ADDRESS);
  Wire.write(MCP_PORTB_PINS);
  Wire.write(bits & 0x3F);  // Mask to 6 bits
  Wire.endTransmission();
}

void VolumeController::loop() {
  int m = millis();
  VolumeController::volEncLoop(m);
}