# ESP32 Preamp Build System
.PHONY: build clean flash monitor help

# Default target
build:
	@echo "Building firmware..."
	~/.local/bin/pio run

# Flash firmware via USB
flash: build
	@echo "Flashing firmware via USB..."
	~/.local/bin/pio run --target upload

# Nuclear flash - erase OTA data first to ensure clean boot
nuke: build
	@echo "Erasing OTA data partition..."
	~/.local/bin/pio pkg exec -- esptool.py --chip esp32 erase_region 0xd000 0x2000
	@echo "OTA data erased. Now flashing firmware..."
	~/.local/bin/pio run --target upload

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	~/.local/bin/pio run --target clean

# Monitor serial output
monitor:
	@echo "Starting serial monitor..."
	~/.local/bin/pio device monitor

# Force rebuild of everything
rebuild: clean build

# Show help
help:
	@echo "Available targets:"
	@echo "  build    - Build firmware (default)" 
	@echo "  flash    - Build and flash via USB"
	@echo "  monitor  - Start serial monitor"
	@echo "  clean    - Clean build files"
	@echo "  rebuild  - Clean and build"
	@echo "  help     - Show this help"