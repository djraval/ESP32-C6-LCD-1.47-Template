# Pin Reference Guide

Complete GPIO pin reference for the Waveshare ESP32-C6-LCD-1.47 development board.

## Board Overview

The ESP32-C6-LCD-1.47 is a compact development board featuring:
- ESP32-C6FH4 (QFN32) with 4MB flash
- 1.47" LCD display (ST7789, 172×320)
- RGB LED (WS2812-style)
- TF card slot
- USB Type-C interface

## Pin Assignments

### LCD Display (ST7789) - SPI Interface

| Function | GPIO | Description |
|----------|------|-------------|
| MOSI     | 6    | SPI Master Out Slave In (shared with SD) |
| SCLK     | 7    | SPI Clock (shared with SD) |
| CS       | 14   | Chip Select (LCD only) |
| DC       | 15   | Data/Command select |
| RST      | 21   | Reset |
| BL       | 22   | Backlight control (PWM) |

**SPI Configuration:**
- Host: SPI2_HOST
- Frequency: 80MHz (maximum for ESP32-C6)
- Mode: SPI Mode 0

### RGB LED (WS2812)

| Function | GPIO | Description |
|----------|------|-------------|
| DATA     | 8    | WS2812 data line |

**RMT Configuration:**
- Channel: 0
- Protocol: WS2812 (800kHz)

### TF Card (MicroSD) - SPI Interface

| Function | GPIO | Description |
|----------|------|-------------|
| MISO     | 5    | SPI Master In Slave Out (SD only) |
| MOSI     | 6    | SPI Master Out Slave In (shared with LCD) |
| SCLK     | 7    | SPI Clock (shared with LCD) |
| CS       | 4    | Chip Select (SD only) |

**SPI Configuration:**
- Host: SPI2_HOST (shared with LCD)
- Frequency: 20MHz (recommended for SD cards)

### Built-in Buttons

| Function | GPIO | Description |
|----------|------|-------------|
| BOOT     | 0    | Built-in BOOT button (active low) |

### Available GPIO Pins

These pins are available for user applications:

#### Digital I/O Pins
| GPIO | ADC | Notes |
|------|-----|-------|
| 1    | ADC1_CH0 | Can be used for analog input |
| 2    | ADC1_CH1 | Can be used for analog input |
| 3    | ADC1_CH2 | Can be used for analog input |
| 9    | -    | Digital only |
| 10   | -    | Digital only |
| 11   | -    | Digital only |
| 18   | -    | Recommended for I2C SDA |
| 19   | -    | Recommended for I2C SCL |
| 20   | -    | Digital only |
| 23   | -    | Digital only |

#### Recommended Peripheral Assignments

**I2C (recommended pins):**
- SDA: GPIO 18
- SCL: GPIO 19
- Frequency: 100kHz

**UART1 (additional serial):**
- TX: GPIO 16
- RX: GPIO 17

**ADC (analog input):**
- Channel 0: GPIO 1
- Channel 1: GPIO 2
- Channel 2: GPIO 3

## Pin Usage Helper Functions

The `board_pins.h` file includes helper functions:

### `is_gpio_available(int gpio_num)`
Check if a GPIO pin is available for user applications.

```c
if (is_gpio_available(18)) {
    // GPIO 18 is available for use
    gpio_set_direction(18, GPIO_MODE_OUTPUT);
}
```

### `get_gpio_description(int gpio_num)`
Get a description of what a GPIO pin is used for on the board.

```c
printf("GPIO 8: %s\n", get_gpio_description(8));  // "RGB LED"
printf("GPIO 18: %s\n", get_gpio_description(18)); // "Available"
```

## Pin Conflict Resolution

### SPI Sharing (LCD + SD Card)
The LCD and SD card share SPI pins (MOSI, SCLK) but have separate chip select pins:
- Use different CS pins (LCD: GPIO 14, SD: GPIO 4)
- Configure different SPI frequencies if needed
- Both can be used simultaneously

### UART Conflicts
- UART0: Used for programming/debugging (GPIO 16/17 on some variants)
- UART1: Available on GPIO 16/17 (recommended for external devices)
- UART2: GPIO 20/21 conflicts with LCD_RST, avoid for external use

### I2C Recommendations
- Use GPIO 18 (SDA) and GPIO 19 (SCL) for I2C devices
- These pins are not used by onboard peripherals
- Standard 100kHz frequency recommended

## Power Specifications

| Rail | Voltage | Notes |
|------|---------|-------|
| VCC  | 3.3V    | Main system voltage |
| USB  | 5V      | USB input voltage |
| GPIO | 3.3V    | All GPIO operate at 3.3V |

**Current Limits:**
- GPIO source/sink: 40mA maximum per pin
- Total GPIO current: 200mA maximum
- RGB LED: ~60mA at full white

## Safe Area Definitions (LCD)

The LCD has curved edges that require safe area consideration:

| Parameter | Value | Description |
|-----------|-------|-------------|
| Full Width | 172px | Complete display width |
| Full Height | 320px | Complete display height |
| Safe Width | 160px | Usable width (avoids bezel) |
| Safe Height | 304px | Usable height (avoids bezel) |
| Left Margin | 6px | Pixels to avoid on left |
| Right Margin | 6px | Pixels to avoid on right |
| Top Margin | 8px | Pixels to avoid on top |
| Bottom Margin | 8px | Pixels to avoid on bottom |

## Example Configurations

### Basic GPIO Output
```c
#include "driver/gpio.h"

// Configure GPIO 18 as output
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << 18),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};
gpio_config(&io_conf);

// Set high
gpio_set_level(18, 1);
```

### I2C Configuration
```c
#include "driver/i2c.h"

i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_SDA_PIN,      // GPIO 18
    .scl_io_num = I2C_SCL_PIN,      // GPIO 19
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_FREQ_HZ // 100kHz
};

i2c_param_config(I2C_NUM_0, &conf);
i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
```

### ADC Configuration
```c
#include "esp_adc/adc_oneshot.h"

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
};
adc_oneshot_new_unit(&init_config1, &adc1_handle);

adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_11,
};
adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config); // GPIO 1
```

## Troubleshooting

### Pin Not Working
1. Check if pin is available: `is_gpio_available(pin_num)`
2. Verify pin is not used by onboard peripherals
3. Check GPIO configuration (input/output, pull-up/down)

### SPI Conflicts
1. Ensure different chip select pins for LCD and SD
2. Check SPI host assignment (both use SPI2_HOST)
3. Verify timing requirements for each device

### I2C Issues
1. Use recommended pins (GPIO 18/19)
2. Enable pull-up resistors
3. Check device addresses for conflicts

### Power Issues
1. Check total GPIO current draw
2. Verify 3.3V operation for all devices
3. Consider external power for high-current devices

## References

- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [Waveshare ESP32-C6-LCD-1.47 Wiki](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47)
- [ESP-IDF GPIO Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/gpio.html)
