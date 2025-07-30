# Project Notes

## TODO
- Add display timeout: 3 minutes → complete off (`u8g2.setPowerSave(1)`)
- Wake display on any user interaction

## Settings Corruption Bug  
- Caused by power-cycling during flash writes
- Could implement A/B settings backup system
- Default to "muted on channel 1" if settings corrupt