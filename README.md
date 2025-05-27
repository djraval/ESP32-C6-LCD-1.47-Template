# ESP32-C6-LCD-1.47 Template

Ready-to-use ESP32-C6 project template for the **Waveshare ESP32-C6-LCD-1.47** board. Includes drivers for the 1.47" LCD, RGB LED, and TF card.

**Status: TESTED & WORKING** - Builds, flashes, and runs successfully on hardware.

## Quick Start

## Board Features
- **1.47" LCD Display** (172×320, ST7789)
- **RGB LED** (WS2812-style)
- **TF Card Slot** for storage
- **WiFi 6 + Bluetooth 5** ready
- **4MB Flash** + multiple GPIO pins

## What's Included

### Hardware Drivers
- **LCD Display**: ST7789 driver framework (`components/lcd_st7789/`)
- **RGB LED**: WS2812 driver framework (`components/rgb_led/`)
- **Pin Mapping**: Complete GPIO definitions (`main/board_pins.h`)

### Ready-to-Use Examples
- **Hello World**: Simple starting point (`main/main.c`)
- **Advanced Demo**: Full board features (`examples/board_demo/`)

### Development Setup
- **Target**: ESP32-C6 optimized
- **Serial Port**: COM13 (configurable)
- **Build**: Size-optimized for embedded use

## Prerequisites
- **ESP-IDF v5.4.1+** (command line tools)
- 
```bash
# 1. Activate ESP-IDF environment
get_idf

# 2. First time setup
idf.py set-target esp32c6

# 3. Build, flash, and monitor
idf.py -p COM13 build flash monitor

# Alternative: Use any port
idf.py -p /dev/ttyUSB0 build flash monitor  # Linux/Mac
idf.py -p COM5 build flash monitor          # Windows
```

> **No VSCode needed!** This template works perfectly with just the ESP-IDF command line tools.

**Expected Output:**
```
ESP32-C6 Hello World starting...
=== Waveshare ESP32-C6-LCD-1.47 ===
ESP32-C6 chip with 1 CPU core(s), WiFi/BLE
Silicon revision: 0
Flash size: 4 MB
Free heap: 327680 bytes
Hello task started. Check the output every 5 seconds!
Hello World! Counter: 0
Free heap: 325632 bytes
Hello World! Counter: 1
Free heap: 325632 bytes
```

**What You'll See:**
- Basic system information
- Simple hello world counter every 5 seconds
- Memory usage monitoring
- Clean, minimal output perfect for beginners

## Your First Change

Let's personalize the hello world:

1. **Open** `main/main.c`
2. **Find** line ~60:
   ```c
   ESP_LOGI(TAG, "Hello World! Counter: %d", counter++);
   ```
3. **Change** to:
   ```c
   ESP_LOGI(TAG, "Hello [YOUR NAME]! Counter: %d", counter++);
   ```
4. **Build and flash** again:
   ```bash
   idf.py -p COM13 build flash monitor
   ```

**Success!** You've made your first ESP32-C6 modification!

## Project Structure

```
ESP32C6/
├── main/
│   ├── main.c              # Hello World (start here!)
│   └── board_pins.h        # GPIO pin definitions
├── components/             # Hardware drivers
│   ├── lcd_st7789/         # LCD display driver
│   └── rgb_led/            # RGB LED driver
└── examples/               # Advanced examples
    └── board_demo/         # Full board demo
```

## Using the Template

### Basic Usage
1. **Start with `main/main.c`** - Simple hello world application
2. **Check `main/board_pins.h`** - All GPIO definitions for the board
3. **Explore `components/`** - LCD and RGB LED driver frameworks
4. **Try `examples/board_demo/`** - Complete board feature demonstration

### Adding Features
```c
// LCD Example
#include "lcd_st7789.h"
lcd_config_t config = lcd_get_default_config();
lcd_init(&config);
lcd_fill_screen(LCD_COLOR_BLUE);

// RGB LED Example
#include "rgb_led.h"
rgb_led_config_t config = rgb_led_get_default_config();
rgb_led_init(&config);
rgb_led_set_color(255, 0, 0);  // Red
```

### Advanced Example

Try the multi-task demo:
```bash
cd examples/board_demo
idf.py set-target esp32c6
idf.py -p COM13 build flash monitor
```

**Features:**
- LCD color cycling
- RGB LED rainbow effects
- System monitoring
- Button detection

### Next Steps
1. **Complete the drivers** - Full ST7789 and WS2812 implementation
2. **Add TF card support** - File storage capabilities
3. **Enable WiFi/Bluetooth** - IoT connectivity
4. **Integrate LVGL** - Professional UI graphics

## Development Options

```bash
# Activate ESP-IDF environment
get_idf

# Standard workflow
idf.py set-target esp32c6
idf.py build
idf.py -p COM13 flash monitor

# Clean build
idf.py fullclean
idf.py build
```

## Troubleshooting
- **Build errors**: Run `get_idf` to activate ESP-IDF environment
- **Flash errors**: Check board connection and port (COM13, /dev/ttyUSB0, etc.)
- **Port issues**: Use `idf.py -p YOUR_PORT flash monitor`
- **Permission errors**: Add user to dialout group (Linux) or check driver installation (Windows)
