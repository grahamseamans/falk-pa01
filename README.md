## Open-Pre ESP32 Preamp Controller

ESP32-based preamp controller with web interface, adapted from [FALK PA-01 Advanced](https://github.com/gilphilbert/falk-pa01-advanced) by gilphilbert.

### RelaiXed Passive Modifications
This version has been adapted for RelaiXed Passive 6-bit attenuator control:
- 6-bit volume control via I2C (GPB0-GPB5)
- WiFi button moved from INPUT to VOLUME button
- I2C addressing updated for RelaiXed Passive hardware

### Build
Install PlatformIO CLI (recommended over VSCode for reliability):

```bash
# Install PlatformIO CLI
pip install platformio
# or on macOS with externally-managed Python:
pipx install platformio

# Clone and build
git clone https://github.com/grahamseamans/open-pre.git
cd open-pre
pio run                    # Build firmware
pio run -t buildfs         # Build filesystem image
```

### Installation
The ESP32 requires a specific programming sequence due to the auto-update firmware mechanism:

#### Wiring
- Connect Ground, TX and RX connectors on the ESP32 board to a serial adapter
- **IMPORTANT**: Cross the connections - ESP32 TX goes to adapter RX, ESP32 RX goes to adapter TX
- Connect the serial adapter to your development machine
- Power up the ESP32 board

#### Programming Sequence
This project has unusual build requirements due to auto-update firmware that loads from filesystem on boot:

```bash
# 1. Build both firmware and filesystem
pio run && pio run -t buildfs

# 2. Enter programming mode: Hold BOOT, press EN, release EN, release BOOT
# 3. Upload firmware
pio run -t upload

# 4. Enter programming mode again: Hold BOOT, press EN, release EN, release BOOT  
# 5. Upload filesystem (critical - contains firmware copy for auto-update)
pio run -t uploadfs

# 6. Reset: Press EN to boot normally
```

**Note**: Both firmware AND filesystem uploads are required. The firmware auto-updates from the filesystem on every boot, so skipping the filesystem upload will cause reverts to old firmware.

### Updating the firmware
Once you have the PA-01 connected to your wifi, you can update your PA-01 easily. When you've made changes to the code, simply build the code. If you haven't made changes to the UI, then you can upload the firmware directly using the Web UI:
    `/user/home/project-path/.pio/build/esp32dev/firmware.bin`
If you changed the Web UI, next you will need to build the File System Image. The File System Image will include the new firmware as a bundle. Navigate to the build folder and use the Web UI to upload the bundle:
  `/user/home/project-path/.pio/build/esp32dev/spiffs.bin`
Node: to upload firmware in the web UI of the PA-01, go to "Firmware" and select "Update". Provide the new bin file and it will upload the new firmware and restart the unit.

### Connecting the PA-01 to WiFi
Press and hold the INPUT button for 5 seconds. The PA-01 will start an Access Point (named on the front of the unit) that you can connect to. Browse to [http://192.168.4.1](http://192.168.4.1). Click on "Wireless" and select your WiFi network and enter your key.
Note: The board is based on the ESP32 microcontroller which only supports 2.4GHz WiFi networks.