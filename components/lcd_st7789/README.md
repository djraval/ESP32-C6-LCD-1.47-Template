# LCD ST7789 Driver

Custom ST7789 SPI driver for learning low-level display protocols.

## Features

- **Full Display Control**: 172×320 pixel resolution
- **Text Rendering**: 8×8 bitmap font with scaling support (1x to 8x)
- **Safe Area Management**: Automatic bezel compensation for curved display edges
- **Text Layout**: Automatic wrapping, bounds calculation, printf-style formatting
- **Graphics**: Pixel, rectangle, and area fill functions
- **Color Support**: RGB565 format with predefined colors and RGB888 conversion

## Display Specifications

- **Controller**: ST7789
- **Resolution**: 172×320 pixels
- **Colors**: 262K (18-bit RGB)
- **Interface**: SPI (up to 80MHz)
- **Safe Area**: 160×304 pixels (avoids curved bezel edges)

## Quick Start

```c
#include "lcd_st7789.h"

void app_main(void) {
    // Initialize with default configuration
    lcd_config_t config = lcd_get_default_config();
    esp_err_t result = lcd_init(&config);
    
    if (result == ESP_OK) {
        // Set backlight and clear screen
        lcd_set_backlight(75);
        lcd_fill_screen(LCD_COLOR_BLACK);
        
        // Display text
        lcd_draw_safe_string(5, 5, "Hello World!", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
    }
}
```

## API Reference

### Initialization

#### `lcd_init(const lcd_config_t *config)`
Initialize the LCD display with the specified configuration.

**Parameters:**
- `config`: LCD configuration structure

**Returns:** `ESP_OK` on success

#### `lcd_get_default_config(void)`
Get the default LCD configuration for the board.

**Returns:** Default `lcd_config_t` structure

#### `lcd_set_backlight(uint8_t brightness)`
Set the backlight brightness.

**Parameters:**
- `brightness`: Brightness level (0-100%)

**Returns:** `ESP_OK` on success

### Drawing Functions

#### `lcd_fill_screen(uint16_t color)`
Fill the entire screen with a solid color.

**Parameters:**
- `color`: Color in RGB565 format

#### `lcd_fill_safe_area(uint16_t color)`
Fill only the safe area (avoiding curved bezel edges).

**Parameters:**
- `color`: Color in RGB565 format

#### `lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color)`
Draw a single pixel.

**Parameters:**
- `x`, `y`: Pixel coordinates
- `color`: Color in RGB565 format

#### `lcd_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)`
Draw a filled rectangle.

**Parameters:**
- `x`, `y`: Top-left corner coordinates
- `width`, `height`: Rectangle dimensions
- `color`: Color in RGB565 format

#### `lcd_draw_safe_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)`
Draw a rectangle using safe area coordinates.

**Parameters:**
- `x`, `y`: Coordinates relative to safe area
- `width`, `height`: Rectangle dimensions
- `color`: Color in RGB565 format

### Text Rendering

#### Basic Text Functions

#### `lcd_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color)`
Draw a string at specified coordinates.

**Parameters:**
- `x`, `y`: Text position
- `str`: String to draw
- `fg_color`: Text color
- `bg_color`: Background color

#### `lcd_draw_safe_string(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color)`
Draw a string using safe area coordinates.

#### Scaled Text Functions

#### `lcd_draw_string_scaled(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale)`
Draw scaled text (1x=8×8, 2x=16×16, 3x=24×24, etc.).

**Parameters:**
- `scale`: Scale factor (1-8)

#### `lcd_draw_safe_string_scaled(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale)`
Draw scaled text using safe area coordinates.

#### Advanced Text Functions

#### `lcd_draw_safe_text_wrapped(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale)`
Draw text with automatic line wrapping within the safe area.

**Parameters:**
- `str`: Text to draw (can contain `\n` for explicit line breaks)

#### `lcd_printf_safe(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, uint8_t scale, const char *format, ...)`
Printf-style formatted text rendering with automatic wrapping.

**Parameters:**
- `format`: Printf-style format string
- `...`: Arguments for format string

#### Text with Bounds Calculation

#### `lcd_draw_safe_text_wrapped_ex(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale, lcd_text_bounds_t *bounds)`
Draw text and return the rendered bounds for layout management.

**Parameters:**
- `bounds`: Pointer to receive text bounds (can be NULL)

#### `lcd_printf_safe_ex(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, uint8_t scale, lcd_text_bounds_t *bounds, const char *format, ...)`
Printf-style text with bounds calculation.

### Colors

#### Predefined Colors (RGB565)
```c
LCD_COLOR_BLACK     // 0x0000
LCD_COLOR_WHITE     // 0xFFFF
LCD_COLOR_RED       // 0xF800
LCD_COLOR_GREEN     // 0x07E0
LCD_COLOR_BLUE      // 0x001F
LCD_COLOR_YELLOW    // 0xFFE0
LCD_COLOR_MAGENTA   // 0xF81F
LCD_COLOR_CYAN      // 0x07FF
```

#### `lcd_rgb_to_565(uint8_t r, uint8_t g, uint8_t b)`
Convert RGB888 to RGB565 format.

**Parameters:**
- `r`, `g`, `b`: RGB components (0-255)

**Returns:** Color in RGB565 format

## Safe Area System

The display has curved edges that can cut off content. The safe area system ensures your content is always visible:

- **Full Display**: 172×320 pixels
- **Safe Area**: 160×304 pixels
- **Safe Area Offset**: X+6, Y+8 pixels from top-left
- **Bezel Margins**: Top/Bottom: 8px, Left/Right: 6px

**Recommendation**: Use `lcd_draw_safe_*` functions for all user interface elements.

## Text Bounds System

The `lcd_text_bounds_t` structure provides layout information:

```c
typedef struct {
    uint16_t width;   // Width of rendered text in pixels
    uint16_t height;  // Height of rendered text in pixels
    uint16_t lines;   // Number of lines rendered
} lcd_text_bounds_t;
```

Use bounds for dynamic layout:

```c
lcd_text_bounds_t bounds;
lcd_draw_safe_text_wrapped_ex(0, 0, "First section", LCD_COLOR_WHITE, LCD_COLOR_BLACK, 1, &bounds);

// Position next section below the first
uint16_t next_y = bounds.height + 5;  // Add 5 pixels spacing
lcd_draw_safe_string(0, next_y, "Second section", LCD_COLOR_YELLOW, LCD_COLOR_BLACK);
```

## Font Information

- **Base Font**: 8×8 pixel bitmap font
- **Character Range**: ASCII 32-122 (space to 'z')
- **Scaling**: 1x to 8x (8×8 to 64×64 pixels)
- **Features**: Uppercase, lowercase, numbers, symbols

## Performance Notes

- **SPI Speed**: 80MHz maximum for ESP32-C6
- **Text Rendering**: Optimized for fast display without character-by-character flicker
- **Memory Usage**: Minimal RAM usage with efficient bitmap rendering
- **Safe Area**: Slightly slower than direct drawing but ensures visibility

## Examples

See `main/main.c` for a complete working example with text rendering, bounds calculation, and safe area usage.
