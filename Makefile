# ESP32-C6 Template Makefile
# Self-contained development environment

# ESP-IDF Configuration
IDF_PATH ?= C:/Espressif/frameworks/esp-idf-v5.4.1
PORT ?= COM13
JOBS ?= 16

# Platform detection
ifeq ($(OS),Windows_NT)
    SHELL := powershell.exe
    .SHELLFLAGS := -NoProfile -Command
    IDF_EXPORT := $(IDF_PATH)/export.ps1
    ACTIVATE_CMD := $$env:CMAKE_BUILD_PARALLEL_LEVEL = $(JOBS); & '$(IDF_EXPORT)';
else
    # Linux/macOS paths (common locations)
    IDF_PATH := $(or $(wildcard $(HOME)/esp/esp-idf), $(wildcard /opt/esp-idf), $(wildcard $(HOME)/.espressif/esp-idf))
    IDF_EXPORT := $(IDF_PATH)/export.sh
    ACTIVATE_CMD := export CMAKE_BUILD_PARALLEL_LEVEL=$(JOBS) && source $(IDF_EXPORT) &&
    PORT := /dev/ttyUSB0
endif

# Colors for output (optional)
GREEN = \033[0;32m
NC = \033[0m

.PHONY: help build flash monitor clean menuconfig fullbuild setup deps check-idf

# Default target
help:
	@echo "ESP32-C6 Template - Available Commands:"
	@echo ""
	@echo "Development:"
	@echo "  make build      - Build the project"
	@echo "  make flash      - Flash firmware to ESP32-C6"
	@echo "  make monitor    - Monitor serial output"
	@echo "  make fullbuild  - Build + Flash + Monitor"
	@echo "  make clean      - Clean build files"
	@echo ""
	@echo "Configuration:"
	@echo "  make menuconfig - Open ESP-IDF configuration menu"
	@echo "  make deps       - Install/update dependencies"
	@echo ""
	@echo "Setup:"
	@echo "  make setup      - First-time setup instructions"
	@echo ""
	@echo "Current port: $(PORT)"

# Check if ESP-IDF is available
check-idf:
	@echo "Checking ESP-IDF installation..."
ifeq ($(OS),Windows_NT)
	@if (!(Test-Path "$(IDF_PATH)")) { Write-Host "ESP-IDF not found at $(IDF_PATH)" -ForegroundColor Red; exit 1 }
else
	@test -d "$(IDF_PATH)" || (echo "ESP-IDF not found at $(IDF_PATH)" && exit 1)
endif
	@echo "ESP-IDF found at $(IDF_PATH)"

# Build the project
build: check-idf
	@echo "Building ESP32-C6 project..."
	$(ACTIVATE_CMD) idf.py build

# Flash to device
flash: check-idf
	@echo "Flashing to ESP32-C6 on $(PORT)..."
	$(ACTIVATE_CMD) idf.py -p $(PORT) flash

# Monitor serial output
monitor: check-idf
	@echo "Starting serial monitor on $(PORT)..."
	@echo "Press Ctrl+] to exit"
	$(ACTIVATE_CMD) idf.py -p $(PORT) monitor

# Build, flash, and monitor in one command
fullbuild: check-idf
	@echo "Full build + flash + monitor..."
	$(ACTIVATE_CMD) idf.py build && $(ACTIVATE_CMD) idf.py -p $(PORT) flash && $(ACTIVATE_CMD) idf.py -p $(PORT) monitor

# Clean build files
clean: check-idf
	@echo "Cleaning build files..."
	$(ACTIVATE_CMD) idf.py fullclean

# Open menuconfig
menuconfig: check-idf
	@echo "Opening ESP-IDF configuration..."
	$(ACTIVATE_CMD) idf.py menuconfig

# Install/update dependencies
deps: check-idf
	@echo "Installing/updating dependencies..."
	$(ACTIVATE_CMD) idf.py reconfigure

# Setup instructions
setup:
	@echo "ESP32-C6 Template Setup Instructions:"
	@echo ""
	@echo "1. Quick start:"
	@echo "   make fullbuild    # Build + Flash + Monitor"
	@echo ""
	@echo "2. Development workflow:"
	@echo "   make build        # Build only"
	@echo "   make flash        # Flash only"
	@echo "   make monitor      # Monitor only"
	@echo "   make clean        # Clean build"
	@echo ""
	@echo "3. Configuration:"
	@echo "   make menuconfig   # ESP-IDF settings"
	@echo "   make deps         # Update dependencies"
	@echo ""
	@echo "4. Custom port:"
	@echo "   make flash PORT=/dev/ttyUSB0    # Linux"
	@echo "   make flash PORT=COM5            # Windows"
	@echo ""
	@echo "Hardware: Waveshare ESP32-C6-LCD-1.47"
	@echo "Features: 172x320 ST7789 LCD + LVGL"
	@echo "ESP-IDF: $(IDF_PATH)"
