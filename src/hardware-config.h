#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// ====== HARDWARE CONFIGURATION ====== //
// Pin assignments and hardware-specific constants

// ====== DISPLAY PINS ====== //
#define SCREEN_CS               5
#define SCREEN_DC               15
#define SCREEN_RST              13

// ====== BUTTON PINS ====== //
#define MUTE_BUTTON             34  // Input button - used for mute and IR programming
#define POWER_BUTTON            GPIO_NUM_33
#define POWER_CONTROL           GPIO_NUM_32

// ====== ENCODER PINS ====== //
// Broken hardware (not used)
#define INP_ENCODER_A           14  // Broken
#define INP_ENCODER_B           27  // Broken

// Working hardware (input encoder used for volume)
#define VOL_ENCODER_A           26  // Input encoder A - used for volume  
#define VOL_ENCODER_B           25  // Input encoder B - used for volume

// ====== IR RECEIVER ====== //
#define IR_RECEIVER_PIN         12

// ====== HARDWARE LIMITS ====== //
#define VOL_MIN                 0
#define VOL_MAX                 63  // 6-bit for RelaiXed Passive (GPB0-GPB5)
#define INP_MIN                 1
#define INP_MAX                 4   // 4 physical input relays

#endif // HARDWARE_CONFIG_H