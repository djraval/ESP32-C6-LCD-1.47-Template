# ESP32-C6-LCD-1.47 Template

A modular template for the **Waveshare ESP32-C6-LCD-1.47** development board with clean hardware abstraction and UI management.

## Features

- **Modular Architecture** - Hardware abstraction layer + UI components
- **ST7789 LCD Display** (172x320 pixels) with LVGL
- **WS2812 RGB LED** control
- **Clean main.c** - High-level application logic only

## Quick Start

### Build and Flash
```bash
make build flash
```

### Control Hardware
```c
#include "led_hal.h"
#include "display_hal.h"

// LED control
led_hal_set_color(255, 0, 0);  // Red
led_hal_clear();               // Turn off

// Display backlight
display_hal_set_backlight(128); // 50% brightness
```

### Modify UI
Edit `components/app_ui/src/ui_manager.c` - look for "MODIFY:" comments.

## File Structure

```
├── components/
│   ├── board_hal/          # Hardware abstraction layer
│   │   ├── include/        # board_config.h, display_hal.h, led_hal.h
│   │   └── src/            # Hardware implementations
│   └── app_ui/             # UI management
│       ├── include/        # ui_manager.h, lvgl_driver.h
│       └── src/            # UI implementations
├── main/
│   ├── main.c              # Application logic
│   └── CMakeLists.txt      # Component dependencies
└── Makefile               # Build shortcuts
```

## Hardware Pins

| Component | GPIO | Function |
|-----------|------|----------|
| LCD | 6,7,14,15,21 | SPI display |
| Backlight | 22 | PWM control |
| RGB LED | 8 | WS2812 |

## Adding Features

### New Hardware
1. Add HAL component in `components/your_hal/`
2. Include in `main/CMakeLists.txt`
3. Initialize in `main.c`

### New Screens
1. Add functions to `components/app_ui/src/ui_manager.c`
2. Declare in `components/app_ui/include/ui_manager.h`

### WiFi/Bluetooth
Create new components following the same pattern.

## Commands

```bash
make build      # Build project
make flash      # Flash to board
make monitor    # View serial output
make clean      # Clean build files
```
