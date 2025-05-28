# Getting Started Guide

Complete guide to using the ESP32-C6-LCD-1.47 template project.

## Prerequisites

### Required Software
- **ESP-IDF v5.4.1+** - [Installation Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/index.html)
- **Git** - For cloning and version control
- **Serial Terminal** - Built into ESP-IDF or use PuTTY/screen

### Hardware
- **Waveshare ESP32-C6-LCD-1.47** development board
- **USB Type-C cable** for programming and power
- **Computer** with Windows, Linux, or macOS

## Quick Setup

### 1. Environment Setup
```bash
# Activate ESP-IDF environment (run this in each new terminal)
get_idf

# Verify installation
idf.py --version
```

### 2. Project Setup
```bash
# Navigate to project directory
cd ESP32C6

# Set target (first time only)
idf.py set-target esp32c6

# Build the project
idf.py build
```

### 3. Flash and Run
```bash
# Flash and monitor (replace COM13 with your port)
idf.py -p COM13 flash monitor

# Linux/Mac example
idf.py -p /dev/ttyUSB0 flash monitor
```

### 4. Expected Output
```
ESP32-C6 Hello World with LCD Display starting...
=== Waveshare ESP32-C6-LCD-1.47 ===
ESP32-C6 chip with 1 CPU core(s), WiFi/BLE
Silicon revision: 0
Flash size: 4 MB
Free heap: 327680 bytes
LCD initialized successfully
Text rendering demo: multiple sizes and cases
Safe area: 160x304 pixels, avoiding curved bezel edges
LCD ready for display
Visual feedback started! Watch the LCD screen cycle through colors every 5 seconds!
```

## Your First Modification

Let's customize the hello world message:

### 1. Open the Main File
Edit `main/main.c` and find the hello task around line 139:

```c
ESP_LOGI(TAG, "Hello World! Counter: %d", counter);
```

### 2. Personalize It
Change it to:
```c
ESP_LOGI(TAG, "Hello [YOUR NAME]! Counter: %d", counter);
```

### 3. Rebuild and Flash
```bash
idf.py -p COM13 build flash monitor
```

You should see your personalized message in the console output!

## Understanding the Project Structure

```
ESP32C6/
├── main/                   # Main application
│   ├── main.c             # Hello world with LCD demo
│   └── board_pins.h       # GPIO pin definitions
├── components/            # Hardware drivers
│   ├── lcd_st7789/        # LCD display driver
│   │   ├── lcd_st7789.c   # Driver implementation
│   │   ├── include/       # Header files
│   │   └── README.md      # LCD driver documentation
│   └── rgb_led/           # RGB LED driver
│       ├── rgb_led.c      # Driver implementation
│       ├── include/       # Header files
│       └── README.md      # RGB LED driver documentation
├── examples/              # Example projects
│   └── board_demo/        # Advanced board demo
├── docs/                  # Documentation
│   ├── GETTING_STARTED.md # This file
│   └── PIN_REFERENCE.md   # Complete pin reference
└── README.md              # Project overview
```

## Adding LCD Display

### Basic LCD Usage
```c
#include "lcd_st7789.h"

void app_main(void) {
    // Initialize LCD
    lcd_config_t config = lcd_get_default_config();
    esp_err_t result = lcd_init(&config);
    
    if (result == ESP_OK) {
        // Set backlight and clear screen
        lcd_set_backlight(75);
        lcd_fill_screen(LCD_COLOR_BLACK);
        
        // Display text in safe area
        lcd_draw_safe_string(5, 5, "Hello ESP32-C6!", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        
        // Display larger text
        lcd_draw_safe_string_scaled(5, 25, "Big Text!", LCD_COLOR_RED, LCD_COLOR_BLACK, 2);
    }
}
```

### Text with Automatic Layout
```c
void display_system_info(void) {
    lcd_fill_safe_area(LCD_COLOR_BLACK);
    
    uint16_t y_pos = 5;
    lcd_text_bounds_t bounds;
    
    // Title
    lcd_draw_safe_string_scaled(5, y_pos, "System Info", LCD_COLOR_WHITE, LCD_COLOR_BLACK, 2);
    y_pos += 20;
    
    // System details with automatic positioning
    lcd_printf_safe_ex(5, y_pos, LCD_COLOR_GREEN, LCD_COLOR_BLACK, 1, &bounds,
                      "Chip: ESP32-C6\nFlash: 4MB\nHeap: %ld bytes",
                      esp_get_free_heap_size());
    y_pos += bounds.height + 5;
    
    // Status bar
    lcd_draw_safe_rectangle(5, y_pos, 150, 10, LCD_COLOR_GREEN);
    lcd_draw_safe_string(10, y_pos + 1, "SYSTEM OK", LCD_COLOR_BLACK, LCD_COLOR_GREEN);
}
```

## Adding RGB LED

### Basic LED Control
```c
#include "rgb_led.h"

void app_main(void) {
    // Initialize RGB LED
    rgb_led_config_t config = rgb_led_get_default_config();
    esp_err_t result = rgb_led_init(&config);
    
    if (result == ESP_OK) {
        // Set to red
        rgb_led_set_color(255, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Set to green using HSV
        rgb_led_set_hsv(120, 100, 100);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Turn off
        rgb_led_off();
    }
}
```

### Rainbow Effect
```c
void rainbow_task(void *pvParameters) {
    uint16_t hue = 0;
    
    while (1) {
        rgb_led_set_hsv(hue, 100, 100);
        hue = (hue + 5) % 360;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Create the task in app_main()
xTaskCreate(rainbow_task, "rainbow", 2048, NULL, 5, NULL);
```

## Combining LCD and LED

### Status Display with LED Indicator
```c
#include "lcd_st7789.h"
#include "rgb_led.h"

typedef enum {
    SYSTEM_STARTING,
    SYSTEM_RUNNING,
    SYSTEM_ERROR
} system_state_t;

void update_status_display(system_state_t state, int counter) {
    // Update LCD
    lcd_fill_safe_area(LCD_COLOR_BLACK);
    
    const char* state_text;
    uint16_t text_color;
    
    switch (state) {
        case SYSTEM_STARTING:
            state_text = "Starting...";
            text_color = LCD_COLOR_YELLOW;
            rgb_led_set_color(RGB_LED_COLOR_YELLOW);
            break;
        case SYSTEM_RUNNING:
            state_text = "Running";
            text_color = LCD_COLOR_GREEN;
            rgb_led_set_color(RGB_LED_COLOR_GREEN);
            break;
        case SYSTEM_ERROR:
            state_text = "Error";
            text_color = LCD_COLOR_RED;
            rgb_led_set_color(RGB_LED_COLOR_RED);
            break;
    }
    
    lcd_printf_safe(5, 5, text_color, LCD_COLOR_BLACK, 1,
                   "Status: %s\nCounter: %d\nHeap: %ld bytes",
                   state_text, counter, esp_get_free_heap_size());
}
```

## Common Development Tasks

### Building and Flashing
```bash
# Clean build (if needed)
idf.py fullclean
idf.py build

# Flash only (without monitor)
idf.py -p COM13 flash

# Monitor only (after flashing)
idf.py -p COM13 monitor

# Build, flash, and monitor in one command
idf.py -p COM13 build flash monitor
```

### Debugging
```bash
# Enable verbose output
idf.py -v build

# Check configuration
idf.py menuconfig

# View partition table
idf.py partition-table
```

### Port Detection
```bash
# Windows - List COM ports
mode

# Linux - List USB devices
ls /dev/ttyUSB*
ls /dev/ttyACM*

# macOS - List USB devices
ls /dev/cu.usbserial*
```

## Advanced Examples

### Multi-Task Application
```c
void lcd_task(void *pvParameters) {
    int counter = 0;
    while (1) {
        lcd_printf_safe(5, 5, LCD_COLOR_WHITE, LCD_COLOR_BLACK, 1,
                       "LCD Task\nCounter: %d", counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void led_task(void *pvParameters) {
    uint16_t hue = 0;
    while (1) {
        rgb_led_set_hsv(hue, 100, 100);
        hue = (hue + 1) % 360;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    // Initialize hardware
    lcd_config_t lcd_config = lcd_get_default_config();
    rgb_led_config_t led_config = rgb_led_get_default_config();
    
    lcd_init(&lcd_config);
    rgb_led_init(&led_config);
    
    // Create tasks
    xTaskCreate(lcd_task, "lcd_task", 4096, NULL, 5, NULL);
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
}
```

## Next Steps

1. **Explore Examples**: Try `examples/board_demo/` for advanced features
2. **Read Driver Docs**: Check `components/*/README.md` for detailed API reference
3. **Add Sensors**: Use available GPIO pins for I2C/SPI sensors
4. **Enable WiFi**: Configure networking in `sdkconfig.defaults`
5. **Add LVGL**: Integrate graphics library for professional UI

## Troubleshooting

### Build Errors
- Run `get_idf` to activate ESP-IDF environment
- Check ESP-IDF version: `idf.py --version`
- Try clean build: `idf.py fullclean && idf.py build`

### Flash Errors
- Check USB cable connection
- Verify correct port: `idf.py -p YOUR_PORT flash`
- Try different USB port or cable
- Hold BOOT button while connecting (if needed)

### Display Issues
- Check LCD initialization in console output
- Verify backlight setting: `lcd_set_backlight(75)`
- Use safe area functions: `lcd_draw_safe_*`

### LED Issues
- Check RGB LED initialization
- Test with predefined colors: `RGB_LED_COLOR_RED`
- Verify GPIO 8 is not used elsewhere

## Getting Help

- **ESP-IDF Documentation**: https://docs.espressif.com/projects/esp-idf/
- **Board Wiki**: https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47
- **ESP32-C6 Datasheet**: https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf
