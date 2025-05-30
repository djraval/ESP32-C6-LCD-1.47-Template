# ESP32-C6-LCD-1.47 Template

Minimal ESP32-C6 template for the **Waveshare ESP32-C6-LCD-1.47** development board with LVGL graphics.

## Quick Start

```bash
make fullbuild   # Build + Flash + Monitor
```

The Makefile automatically:
- Finds and activates ESP-IDF environment
- Sets the correct port (COM13)
- Downloads LVGL dependencies
- Builds, flashes, and monitors

### Other Commands

```bash
make help        # Show all available commands
make build       # Build only
make flash       # Flash only
make monitor     # Monitor only
make clean       # Clean build
make setup       # Detailed setup instructions
```

### Custom Port

```bash
make flash PORT=/dev/ttyUSB0    # Linux
make flash PORT=COM5            # Windows
```

> **Note**: LVGL dependencies are automatically downloaded during first build via ESP-IDF Component Manager.

## Hardware

- **Board**: Waveshare ESP32-C6-LCD-1.47
- **Display**: 1.47" ST7789 LCD (172×320 pixels)
- **LED**: WS2812 RGB LED
- **MCU**: ESP32-C6 (WiFi 6, Bluetooth 5, IEEE 802.15.4)

## Features

- Minimal main.c with LVGL graphics
- Hardware-accelerated display with DMA
- Custom drivers for learning (components/)
- Cross-platform Makefile for development
- Automatic dependency management

## Prerequisites

- ESP-IDF v5.4.1+

## Project Structure

```
ESP32C6/
├── main/
│   ├── main.c              # Minimal LVGL application (151 lines)
│   └── CMakeLists.txt      # Build configuration
├── components/             # Custom drivers for learning
│   ├── lcd_st7789/         # Custom ST7789 SPI driver
│   └── rgb_led/            # Custom WS2812 RMT driver
├── Makefile                # Self-contained development environment
└── README.md               # This file
```

## ESP-IDF Configuration

The Makefile automatically detects and configures ESP-IDF:

- **Windows**: `C:/Espressif/frameworks/esp-idf-v5.4.1`
- **Linux/macOS**: `~/esp/esp-idf`, `/opt/esp-idf`, or `~/.espressif/esp-idf`

To use a different ESP-IDF installation:
```bash
make build IDF_PATH=/path/to/your/esp-idf
```

## Usage

The main application displays "ESP32-C6 Template Ready!" on the LCD using LVGL.

To customize:
1. Edit `main/main.c` to add your UI elements
2. Use LVGL widgets for professional graphics
3. Reference custom drivers in `components/` for learning

## License

MIT License - Feel free to use in your projects!
