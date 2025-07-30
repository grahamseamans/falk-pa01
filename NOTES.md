# Project Notes

## TODO
- ✅ Add display timeout: 3 minutes → complete off (`u8g2.setPowerSave(1)`) - COMPLETED
- ✅ Wake display on any user interaction - COMPLETED

## Future Improvements
- **Continuous rotary encoder**: Replace detented encoder for smoother volume control
- **Proper IR debouncing**: Use established debouncing library (like Bounce2) instead of custom timing
- **A/B settings backup**: Implement dual settings storage to prevent corruption

## Recent Fixes (Today)
- ✅ **ESP32Encoder pullup bug**: Library's `useInternalWeakPullResistors = UP` doesn't work
  - **Solution**: Set to `NONE` and manually call `pinMode(pin, INPUT_PULLUP)` after `attachFullQuad()`
  - **Root cause**: Library overrides pin configuration during attach
- ✅ **Encoder jumpiness**: Was caused by missing pullup resistors (pins reading 0V instead of 3.3V)
- ✅ **Screen timeout**: Implemented 2-level system (10s dim, 3min off) with automatic wake-up

## Settings Corruption Bug  
- Caused by power-cycling during flash writes
- Could implement A/B settings backup system
- Default to "muted on channel 1" if settings corrupt