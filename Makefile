# ESP32 Preamp Build System
.PHONY: build ota embed clean flash monitor help

# Default target
build: src/web_files.h
	@echo "Building firmware..."
	~/.local/bin/pio run

# Embed web files when they change
src/web_files.h: data/www/* embed_web_files.py
	@echo "Embedding web files..."
	python3 embed_web_files.py


# Flash firmware via USB
flash: build
	@echo "Flashing firmware via USB..."
	~/.local/bin/pio run --target upload

# Deploy via OTA update (assumes ESP32 is running and accessible)
ota: build
	@echo "Deploying via OTA to falk-pa01.local..."
	curl -X POST -F "firmware=@.pio/build/esp32dev/firmware.bin" http://falk-pa01.local/update

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	~/.local/bin/pio run --target clean
	rm -f src/web_files.h

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
	@echo "  ota      - Build and deploy via OTA"
	@echo "  monitor  - Start serial monitor"
	@echo "  clean    - Clean build files"
	@echo "  rebuild  - Clean and build"
	@echo "  help     - Show this help"