#include "display-ssd1322.h"

#include "config.h"
#include "hardware-config.h"
#include "realtime-state.h"

// THIS BLOCK NEEDED FOR THE DISPLAY
#include <SPI.h>
#include "U8g2lib.h"
// ^^ \libraries\U8g2\src\clib\u8g2.h <-- uncomment #define U8G2_16BIT


#include "rubik.h"

// for the display
// this uses the default SPI interface as defined by the manufacturer
// NOTE: for the Adafruit HUZZAH32, use the *labelled pins*, not the standard ones for the ESP32
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R2, SCREEN_CS, SCREEN_DC, SCREEN_RST);
uint32_t screenTimer = 0;

#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 256

void Display::begin() {
//start the screen object
  u8g2.begin();
}

void Display::loop() {
  if ((screenTimer > 0) && (millis() > screenTimer + DISPLAY_DIM_TIMEOUT)) {
    Display::dimScreen();
  }
}

//function that puts the input and volume on the screen
void Display::updateScreen() {
  char volume [5];
  if ((sysSettings.muted == 0) && sysSettings.volume > 0) {
    uint16_t v;
    if (DISPLAY_ABSOLUTE == false) {
      v = round(((float)sysSettings.volume / (float)VOL_MAX) * 100);
      itoa(v, volume, 10);
      strcat(volume, "%");
    } else {
      v = sysSettings.volume;
      itoa(v, volume, 10);
    }
  } else {
    String s = "MUTE";
    strcpy(volume, s.c_str());
  }
  u8g2.clearBuffer();					// clear the internal memory

  //u8g2.setFont(u8g2_font_fub25_tf);
  //u8g2.setFont(u8g2_font_logisoso22_tf);
  u8g2.setFont(rubik_medium_26);

  //vertically centers the text
  uint16_t y = 40;
  //left starting position for the volume (so it's right-aligned)
  uint16_t x = SCREEN_WIDTH - u8g2.getStrWidth(volume);

  const char* inputNames[] = INPUT_NAMES;
  u8g2.drawStr(0, y, inputNames[sysSettings.input - 1]);
  u8g2.drawStr(x, y, volume);

  u8g2.setFont(u8g2_font_crox1h_tr);
  x = SCREEN_WIDTH - u8g2.getStrWidth("VOLUME");
  u8g2.drawStr(0, SCREEN_HEIGHT, "INPUT");
  u8g2.drawStr(x, SCREEN_HEIGHT, "VOLUME");
  

  // WiFi icons removed

  //max brightness
  u8g2.setContrast(255);
  //write data to screen
  u8g2.sendBuffer();
  //dim the display after timeout
  if (DISPLAY_DIM_ENABLED) {
    screenTimer = millis();
  }
}

// WiFi screen removed

// Firmware update screen removed

// Firmware upload screen removed

//called after the timeout elapses, drops screen brightness
void Display::dimScreen() {
  u8g2.setContrast(1);
  screenTimer = 0;
}

void Display::off() {
  u8g2.setPowerSave(1);
}

void Display::irProgMode(const char* message) {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_crox2h_tr);
  String title = "IR PROGRAMMING";
  uint16_t x = 128 - (u8g2.getStrWidth(title.c_str()) / 2);
  u8g2.drawStr(x, 18, title.c_str());

  u8g2.setFont(u8g2_font_crox1h_tr);
  x = 128 - (u8g2.getStrWidth(message) / 2);
  u8g2.drawStr(x, 35, message);

  String exit = "Press VOL button to exit";
  x = 128 - (u8g2.getStrWidth(exit.c_str()) / 2);
  u8g2.drawStr(x, 55, exit.c_str());

  u8g2.setContrast(255);
  u8g2.sendBuffer();
}

void Display::irProgComplete() {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_crox2h_tr);
  String title = "IR COMPLETE";
  uint16_t x = 128 - (u8g2.getStrWidth(title.c_str()) / 2);
  u8g2.drawStr(x, 25, title.c_str());

  u8g2.setFont(u8g2_font_crox1h_tr);
  String message = "Remote programmed!";
  x = 128 - (u8g2.getStrWidth(message.c_str()) / 2);
  u8g2.drawStr(x, 45, message.c_str());

  u8g2.setContrast(255);
  u8g2.sendBuffer();
  
  delay(2000); // Show completion message for 2 seconds
  updateScreen(); // Return to normal display
}