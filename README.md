# ESP32-C6-LCD-1.47 Template

Ready-to-use ESP32-C6 project template for the **Waveshare ESP32-C6-LCD-1.47** board. Includes complete drivers for the 1.47" LCD, RGB LED, and comprehensive text rendering.

**Status: TESTED & WORKING** - Builds, flashes, and runs successfully on hardware.

## Quick Start

## Board Features
- **1.47" LCD Display** (172×320, ST7789) with text rendering
- **RGB LED** (WS2812-style) with HSV support
- **TF Card Slot** for storage
- **WiFi 6 + Bluetooth 5** ready
- **4MB Flash** + multiple GPIO pins

## What's Included

### Hardware Drivers
- **LCD Display**: ✅ Complete ST7789 driver with text rendering (`components/lcd_st7789/`)
- **RGB LED**: ✅ Complete WS2812 RMT driver with RGB/HSV support (`components/rgb_led/`)
- **Pin Mapping**: ✅ Complete GPIO definitions (`main/board_pins.h`)

### Ready-to-Use Examples
- **Hello World**: Text rendering demo (`main/main.c`)
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
│   ├── main.c              # Hello World with text rendering demo
│   └── board_pins.h        # Complete GPIO pin definitions
├── components/             # Complete hardware drivers
│   ├── lcd_st7789/         # LCD display driver with text rendering
│   │   ├── README.md       # Complete LCD API documentation
│   │   └── include/        # Header files
│   └── rgb_led/            # RGB LED driver with HSV support
│       ├── README.md       # Complete RGB LED API documentation
│       └── include/        # Header files
├── examples/               # Advanced examples
│   └── board_demo/         # Full board demo
└── docs/                   # Comprehensive documentation
    ├── GETTING_STARTED.md  # Step-by-step setup guide
    └── PIN_REFERENCE.md    # Complete pin reference
```

## Documentation

### Quick References
- **[Getting Started Guide](docs/GETTING_STARTED.md)** - Complete setup and first steps
- **[Pin Reference](docs/PIN_REFERENCE.md)** - GPIO pins, peripherals, and configurations
- **[LCD Driver Guide](components/lcd_st7789/README.md)** - Complete LCD API with text rendering
- **[RGB LED Guide](components/rgb_led/README.md)** - Complete RGB LED API with HSV support

### Using the Template

1. **Start Here**: Read [Getting Started Guide](docs/GETTING_STARTED.md)
2. **Hardware Setup**: Check [Pin Reference](docs/PIN_REFERENCE.md)
3. **LCD Development**: See [LCD Driver Guide](components/lcd_st7789/README.md)
4. **LED Effects**: See [RGB LED Guide](components/rgb_led/README.md)
5. **Advanced Features**: Try `examples/board_demo/`

## Driver Usage Guides

### LCD Display Driver (`lcd_st7789`)

Complete ST7789 driver with text rendering, safe area management, and bezel compensation.

#### Basic Setup
```c
#include "lcd_st7789.h"

// Initialize with default configuration
lcd_config_t config = lcd_get_default_config();
esp_err_t result = lcd_init(&config);

// Set backlight (0-100%)
lcd_set_backlight(75);
```

#### Drawing Functions
```c
// Fill entire screen
lcd_fill_screen(LCD_COLOR_BLACK);

// Fill safe area (avoids curved bezel edges)
lcd_fill_safe_area(LCD_COLOR_BLUE);

// Draw rectangles
lcd_draw_rectangle(10, 10, 50, 30, LCD_COLOR_RED);
lcd_draw_safe_rectangle(5, 5, 100, 50, LCD_COLOR_GREEN);  // Safe area coordinates

// Draw individual pixels
lcd_draw_pixel(100, 100, LCD_COLOR_WHITE);
```

#### Text Rendering
```c
// Basic text (8x8 font)
lcd_draw_string(10, 10, "Hello World", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
lcd_draw_safe_string(5, 5, "Safe Area Text", LCD_COLOR_YELLOW, LCD_COLOR_BLACK);

// Scaled text (2x = 16x16, 3x = 24x24, etc.)
lcd_draw_string_scaled(10, 50, "Big Text", LCD_COLOR_RED, LCD_COLOR_BLACK, 2);
lcd_draw_safe_string_scaled(5, 30, "Safe Big Text", LCD_COLOR_GREEN, LCD_COLOR_BLACK, 3);

// Text with automatic wrapping
lcd_draw_safe_text_wrapped(0, 0, "This long text will automatically wrap to the next line when it reaches the edge!",
                          LCD_COLOR_CYAN, LCD_COLOR_BLACK, 1);

// Printf-style formatting
lcd_printf_safe(0, 50, LCD_COLOR_WHITE, LCD_COLOR_BLACK, 1,
               "Counter: %d\nHeap: %ld bytes\nTime: %d ms",
               counter, esp_get_free_heap_size(), (int)(esp_timer_get_time() / 1000));
```

#### Advanced Text with Bounds
```c
lcd_text_bounds_t bounds;

// Get text dimensions for layout planning
lcd_draw_safe_text_wrapped_ex(0, 0, "Multi-line\ntext example",
                             LCD_COLOR_WHITE, LCD_COLOR_BLACK, 1, &bounds);
printf("Text rendered: %d lines, %dx%d pixels\n", bounds.lines, bounds.width, bounds.height);

// Printf with bounds
lcd_printf_safe_ex(0, bounds.height + 5, LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 1, &bounds,
                  "Next section at Y=%d", bounds.height + 5);
```

#### Colors and Utilities
```c
// Predefined colors (RGB565 format)
LCD_COLOR_BLACK, LCD_COLOR_WHITE, LCD_COLOR_RED, LCD_COLOR_GREEN,
LCD_COLOR_BLUE, LCD_COLOR_YELLOW, LCD_COLOR_MAGENTA, LCD_COLOR_CYAN

// Custom colors
uint16_t custom_color = lcd_rgb_to_565(128, 64, 192);  // Purple
```

#### Safe Area Information
- **Full Display**: 172×320 pixels
- **Safe Area**: 160×304 pixels (avoids curved bezel)
- **Safe Area Offset**: X+6, Y+8 pixels from top-left
- Use `lcd_draw_safe_*` functions for best visual results

### RGB LED Driver (`rgb_led`) - ✅ WORKING

Complete WS2812 RMT driver for the onboard RGB LED on GPIO8.

#### Basic Setup
```c
#include "rgb_led.h"

// Initialize with default configuration
rgb_led_config_t config = rgb_led_get_default_config();
esp_err_t result = rgb_led_init(&config);
```

#### RGB Color Control
```c
// Set RGB values (0-255 each)
rgb_led_set_color(255, 0, 0);      // Red
rgb_led_set_color(0, 255, 0);      // Green
rgb_led_set_color(0, 0, 255);      // Blue
rgb_led_set_color(255, 255, 255);  // White

// Predefined colors (use with rgb_led_set_color)
rgb_led_set_color(RGB_LED_COLOR_RED);      // 255, 0, 0
rgb_led_set_color(RGB_LED_COLOR_GREEN);    // 0, 255, 0
rgb_led_set_color(RGB_LED_COLOR_BLUE);     // 0, 0, 255
rgb_led_set_color(RGB_LED_COLOR_WHITE);    // 255, 255, 255
rgb_led_set_color(RGB_LED_COLOR_YELLOW);   // 255, 255, 0
rgb_led_set_color(RGB_LED_COLOR_MAGENTA);  // 255, 0, 255
rgb_led_set_color(RGB_LED_COLOR_CYAN);     // 0, 255, 255
rgb_led_set_color(RGB_LED_COLOR_ORANGE);   // 255, 165, 0
rgb_led_set_color(RGB_LED_COLOR_PURPLE);   // 128, 0, 128

// Turn off LED
rgb_led_off();
```

#### HSV Color Control
```c
// Set HSV values for smooth color transitions
rgb_led_set_hsv(0, 100, 100);    // Red (hue=0°, full saturation, full brightness)
rgb_led_set_hsv(120, 100, 100);  // Green (hue=120°)
rgb_led_set_hsv(240, 100, 100);  // Blue (hue=240°)
rgb_led_set_hsv(60, 100, 50);    // Yellow at 50% brightness

// Rainbow effect example
for (int hue = 0; hue < 360; hue += 10) {
    rgb_led_set_hsv(hue, 100, 100);
    vTaskDelay(pdMS_TO_TICKS(100));
}
```

#### Color Examples
```c
// Breathing effect
for (int brightness = 0; brightness <= 100; brightness += 5) {
    rgb_led_set_hsv(240, 100, brightness);  // Blue breathing
    vTaskDelay(pdMS_TO_TICKS(50));
}

// Color cycling
uint16_t hue = 0;
while (1) {
    rgb_led_set_hsv(hue, 100, 100);
    hue = (hue + 1) % 360;
    vTaskDelay(pdMS_TO_TICKS(10));
}
```

## Practical Examples

### Complete LCD + RGB LED Demo
```c
#include "lcd_st7789.h"
#include "rgb_led.h"

void app_main(void) {
    // Initialize both drivers
    lcd_config_t lcd_config = lcd_get_default_config();
    rgb_led_config_t led_config = rgb_led_get_default_config();

    lcd_init(&lcd_config);
    rgb_led_init(&led_config);

    lcd_set_backlight(75);
    lcd_fill_screen(LCD_COLOR_BLACK);

    int counter = 0;
    uint16_t hue = 0;

    while (1) {
        // Update LCD with system info
        lcd_fill_safe_area(LCD_COLOR_BLACK);
        lcd_printf_safe(5, 5, LCD_COLOR_WHITE, LCD_COLOR_BLACK, 1,
                       "ESP32-C6 Demo\nCounter: %d\nHeap: %ld\nHue: %d°",
                       counter, esp_get_free_heap_size(), hue);

        // Cycle RGB LED through rainbow
        rgb_led_set_hsv(hue, 100, 100);

        counter++;
        hue = (hue + 10) % 360;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### Text Layout Management
```c
void display_status_screen(void) {
    lcd_fill_safe_area(LCD_COLOR_BLACK);

    uint16_t y_pos = 5;
    lcd_text_bounds_t bounds;

    // Title
    lcd_draw_safe_string_scaled(5, y_pos, "System Status", LCD_COLOR_WHITE, LCD_COLOR_BLACK, 2);
    y_pos += 20; // 2x scale = 16 pixels + 4 spacing

    // System info with automatic layout
    lcd_printf_safe_ex(5, y_pos, LCD_COLOR_GREEN, LCD_COLOR_BLACK, 1, &bounds,
                      "Chip: ESP32-C6\nCores: 1\nFlash: 4MB");
    y_pos += bounds.height + 5;

    // Memory info
    lcd_printf_safe_ex(5, y_pos, LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 1, &bounds,
                      "Free Heap: %ld bytes\nMin Free: %ld bytes",
                      esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    y_pos += bounds.height + 5;

    // Status indicator
    lcd_draw_safe_rectangle(5, y_pos, 150, 10, LCD_COLOR_GREEN);
    lcd_draw_safe_string(10, y_pos + 1, "SYSTEM OK", LCD_COLOR_BLACK, LCD_COLOR_GREEN);
}
```

### Advanced Example

Try the multi-task demo:
```bash
cd examples/board_demo
idf.py set-target esp32c6
idf.py -p COM13 build flash monitor
```

**Features:**
- LCD color cycling with text rendering
- RGB LED rainbow effects with HSV
- System monitoring with bounds calculation
- Button detection

### Next Steps
1. **Add TF card support** - File storage capabilities
2. **Enable WiFi/Bluetooth** - IoT connectivity
3. **Integrate LVGL** - Professional UI graphics
4. **Add sensors** - Temperature, humidity, etc.

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
