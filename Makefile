# ESP32-C6 Template Makefile
# Simple cross-platform development environment

# ESP-IDF Configuration
IDF_PATH ?= $(if $(OS),C:/Espressif/frameworks/esp-idf-v5.4.1,$(HOME)/esp/esp-idf)
PORT ?= $(if $(OS),COM13,/dev/ttyUSB0)

# Platform detection for shell commands
ifeq ($(OS),Windows_NT)
    # Windows
    IDF_EXPORT := $(IDF_PATH)/export.ps1
    ACTIVATE_CMD := powershell -ExecutionPolicy Bypass -Command "& '$(IDF_EXPORT)';"
else
    # Linux/macOS
    IDF_EXPORT := $(IDF_PATH)/export.sh
    ACTIVATE_CMD := source $(IDF_EXPORT) &&
endif

.PHONY: help build flash monitor clean menuconfig deps all

# Default target
help:
	@echo "ESP32-C6 Template - Available Commands:"
	@echo ""
	@echo "Development:"
	@echo "  make build      - Build the project"
	@echo "  make flash      - Flash firmware to ESP32-C6"
	@echo "  make monitor    - Monitor serial output"
	@echo "  make clean      - Clean build files"
	@echo "  make all        - Build and flash in one command"
	@echo ""
	@echo "Configuration:"
	@echo "  make menuconfig - Open ESP-IDF configuration menu"
	@echo "  make deps       - Install/update dependencies"
	@echo ""
	@echo "Environment:"
	@echo "  IDF_PATH: $(IDF_PATH)"
	@echo "  PORT: $(PORT)"
	@echo ""
	@echo "Override with: make flash PORT=COM5 or export IDF_PATH=/path/to/esp-idf"

# Build and flash in one command
all: build flash

# Build the project
build:
	@echo "Building ESP32-C6 project..."
	bash -c "$(ACTIVATE_CMD) idf.py build"

# Flash to device
flash:
	@echo "Flashing to ESP32-C6 on $(PORT)..."
	bash -c "$(ACTIVATE_CMD) idf.py -p $(PORT) flash"

# Monitor serial output
monitor:
	@echo "Starting serial monitor on $(PORT)..."
	@echo "Press Ctrl+] to exit"
	bash -c "$(ACTIVATE_CMD) idf.py -p $(PORT) monitor"

# Clean build files
clean:
	@echo "Cleaning build files..."
	bash -c "$(ACTIVATE_CMD) idf.py fullclean"

# Open menuconfig
menuconfig:
	@echo "Opening ESP-IDF configuration..."
	bash -c "$(ACTIVATE_CMD) idf.py menuconfig"

# Install/update dependencies
deps:
	@echo "Installing/updating dependencies..."
	bash -c "$(ACTIVATE_CMD) idf.py reconfigure"
