# RGB LED Driver - ⚠️ PLACEHOLDER IMPLEMENTATION

**IMPORTANT: This is currently a placeholder implementation. The hardware RGB LED exists on GPIO8 but this driver only logs messages and doesn't control the actual LED.**

Framework for the WS2812-style RGB LED on the Waveshare ESP32-C6-LCD-1.47 board.

## Current Status

**✅ Confirmed Hardware**: RGB LED exists on GPIO8 (verified from official Waveshare documentation)
**⚠️ Driver Status**: Placeholder implementation - API exists but doesn't control hardware
**📋 TODO**: Implement actual WS2812 RMT driver

## Planned Features (Not Yet Implemented)

- **RGB Color Control**: Direct RGB values (0-255 each)
- **HSV Color Control**: Hue, Saturation, Value for smooth color transitions
- **Predefined Colors**: Common colors with easy-to-use macros
- **RMT Interface**: Hardware-accelerated WS2812 protocol
- **Single LED**: Optimized for the onboard RGB LED

## LED Specifications

- **Type**: WS2812-style RGB LED
- **Protocol**: WS2812 timing (800kHz)
- **Colors**: 16.7 million (24-bit RGB)
- **Interface**: RMT (Remote Control Transceiver)
- **Count**: 1 LED on board

## Quick Start

```c
#include "rgb_led.h"

void app_main(void) {
    // Initialize with default configuration
    rgb_led_config_t config = rgb_led_get_default_config();
    esp_err_t result = rgb_led_init(&config);
    
    if (result == ESP_OK) {
        // Set LED to red
        rgb_led_set_color(255, 0, 0);
        
        // Wait 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Turn off LED
        rgb_led_off();
    }
}
```

## API Reference

### Initialization

#### `rgb_led_init(const rgb_led_config_t *config)`
Initialize the RGB LED with the specified configuration.

**Parameters:**
- `config`: RGB LED configuration structure

**Returns:** `ESP_OK` on success

#### `rgb_led_get_default_config(void)`
Get the default RGB LED configuration for the board.

**Returns:** Default `rgb_led_config_t` structure with:
- GPIO pin: 8
- LED count: 1
- RMT channel: 0

### RGB Color Control

#### `rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue)`
Set the LED color using RGB values.

**Parameters:**
- `red`: Red component (0-255)
- `green`: Green component (0-255)
- `blue`: Blue component (0-255)

**Returns:** `ESP_OK` on success

**Example:**
```c
rgb_led_set_color(255, 0, 0);    // Red
rgb_led_set_color(0, 255, 0);    // Green
rgb_led_set_color(0, 0, 255);    // Blue
rgb_led_set_color(255, 255, 255); // White
rgb_led_set_color(128, 64, 192);  // Purple
```

#### `rgb_led_off(void)`
Turn off the RGB LED (equivalent to setting RGB to 0,0,0).

**Returns:** `ESP_OK` on success

### HSV Color Control

#### `rgb_led_set_hsv(uint16_t hue, uint8_t saturation, uint8_t value)`
Set the LED color using HSV values for smooth color transitions.

**Parameters:**
- `hue`: Hue angle in degrees (0-359)
- `saturation`: Saturation percentage (0-100)
- `value`: Value/brightness percentage (0-100)

**Returns:** `ESP_OK` on success

**HSV Color Wheel:**
- **0°**: Red
- **60°**: Yellow
- **120°**: Green
- **180°**: Cyan
- **240°**: Blue
- **300°**: Magenta

**Example:**
```c
rgb_led_set_hsv(0, 100, 100);    // Red (full saturation, full brightness)
rgb_led_set_hsv(120, 100, 100);  // Green
rgb_led_set_hsv(240, 100, 100);  // Blue
rgb_led_set_hsv(60, 100, 50);    // Yellow at 50% brightness
rgb_led_set_hsv(180, 50, 100);   // Light cyan (50% saturation)
```

### Predefined Colors

Use these macros with `rgb_led_set_color()`:

```c
RGB_LED_COLOR_RED       // 255, 0, 0
RGB_LED_COLOR_GREEN     // 0, 255, 0
RGB_LED_COLOR_BLUE      // 0, 0, 255
RGB_LED_COLOR_WHITE     // 255, 255, 255
RGB_LED_COLOR_YELLOW    // 255, 255, 0
RGB_LED_COLOR_MAGENTA   // 255, 0, 255
RGB_LED_COLOR_CYAN      // 0, 255, 255
RGB_LED_COLOR_ORANGE    // 255, 165, 0
RGB_LED_COLOR_PURPLE    // 128, 0, 128
RGB_LED_COLOR_OFF       // 0, 0, 0
```

**Example:**
```c
rgb_led_set_color(RGB_LED_COLOR_RED);
rgb_led_set_color(RGB_LED_COLOR_ORANGE);
rgb_led_set_color(RGB_LED_COLOR_OFF);
```

## Common Patterns

### Rainbow Effect
```c
void rainbow_effect(void) {
    for (int hue = 0; hue < 360; hue += 5) {
        rgb_led_set_hsv(hue, 100, 100);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

### Breathing Effect
```c
void breathing_effect(uint16_t hue) {
    // Fade in
    for (int brightness = 0; brightness <= 100; brightness += 2) {
        rgb_led_set_hsv(hue, 100, brightness);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    // Fade out
    for (int brightness = 100; brightness >= 0; brightness -= 2) {
        rgb_led_set_hsv(hue, 100, brightness);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

### Color Cycling
```c
void color_cycling_task(void *pvParameters) {
    uint16_t hue = 0;
    
    while (1) {
        rgb_led_set_hsv(hue, 100, 100);
        hue = (hue + 1) % 360;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### Status Indicator
```c
typedef enum {
    STATUS_OK,
    STATUS_WARNING,
    STATUS_ERROR,
    STATUS_OFF
} system_status_t;

void set_status_led(system_status_t status) {
    switch (status) {
        case STATUS_OK:
            rgb_led_set_color(RGB_LED_COLOR_GREEN);
            break;
        case STATUS_WARNING:
            rgb_led_set_color(RGB_LED_COLOR_YELLOW);
            break;
        case STATUS_ERROR:
            rgb_led_set_color(RGB_LED_COLOR_RED);
            break;
        case STATUS_OFF:
        default:
            rgb_led_off();
            break;
    }
}
```

### Notification Blink
```c
void notification_blink(uint8_t red, uint8_t green, uint8_t blue, int blinks) {
    for (int i = 0; i < blinks; i++) {
        rgb_led_set_color(red, green, blue);
        vTaskDelay(pdMS_TO_TICKS(200));
        rgb_led_off();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Usage
notification_blink(RGB_LED_COLOR_BLUE, 3);  // 3 blue blinks
```

## Configuration

The `rgb_led_config_t` structure contains:

```c
typedef struct {
    int gpio_pin;           // GPIO pin connected to LED (default: 8)
    int led_count;          // Number of LEDs in chain (default: 1)
    int rmt_channel;        // RMT channel to use (default: 0)
} rgb_led_config_t;
```

For the Waveshare ESP32-C6-LCD-1.47 board, use the default configuration:

```c
rgb_led_config_t config = rgb_led_get_default_config();
```

## Performance Notes

- **Update Rate**: Can handle very fast updates (>1kHz)
- **RMT Hardware**: Uses hardware RMT for precise timing
- **Memory Usage**: Minimal RAM usage
- **Power**: LED draws ~60mA at full white brightness

## Troubleshooting

### LED Not Working
1. Check GPIO pin 8 is not used elsewhere
2. Verify RMT channel 0 is available
3. Ensure proper initialization order

### Wrong Colors
1. Check RGB value ranges (0-255)
2. Verify HSV ranges (H: 0-359, S/V: 0-100)
3. Test with predefined colors first

### Flickering
1. Avoid rapid color changes in tight loops
2. Add small delays between updates
3. Use HSV for smooth transitions

## Examples

See `main/main.c` for integration with LCD display and `examples/board_demo/` for advanced LED effects.
