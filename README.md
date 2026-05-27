# ESP32-C6-LCD-1.47 — MQTT Notification Display

A reference firmware for the **Waveshare ESP32-C6-LCD-1.47** board: subscribes to an MQTT topic over WiFi and renders incoming messages on the 172×320 LCD with a green RGB-LED flash on each receive.

Originally a clean template; this branch is the first concrete app built on top of it. The HAL split (`board_hal` / `app_ui` / `net_hal`) is still reusable for other ideas.

## Quick start

```bash
make menuconfig          # → "MQTT Notification" → fill in WiFi SSID + password
make build flash monitor # PORT defaults to /dev/ttyUSB0; override with PORT=/dev/ttyACM0
```

ESP32-C6's USB JTAG enumerates as `/dev/ttyACM0` on Linux, not `/dev/ttyUSB0`.

Publish a test message from any machine:

```bash
mosquitto_pub -h broker.emqx.io -t devarshi/esp32c6/notify -m "hello"
```

The LCD body label updates and the RGB LED flashes green for ~500 ms.

## Configuration

Four Kconfig entries under "MQTT Notification" in `menuconfig`:

| Symbol | Default | Notes |
|---|---|---|
| `NOTIFY_WIFI_SSID` | _empty_ | 2.4 GHz only — C6 doesn't do 5 GHz |
| `NOTIFY_WIFI_PASSWORD` | _empty_ | WPA2-PSK |
| `NOTIFY_MQTT_BROKER_URI` | `mqtt://broker.emqx.io` | Full URI; swap to `mqtts://...` for TLS |
| `NOTIFY_MQTT_TOPIC` | `devarshi/esp32c6/notify` | Anything you publish from elsewhere |

Credentials live in the gitignored `sdkconfig`, never in source.

## Architecture

```
main/main.c
   ├── display_hal_init()    components/board_hal/  — ST7789, backlight PWM
   ├── led_hal_init()        components/board_hal/  — WS2812 over RMT
   ├── ui_manager_init()     components/app_ui/     — LVGL + screen + notification queue
   ├── net_hal_init()        components/net_hal/    — WiFi STA + esp-mqtt
   └── ui_manager_run()      components/app_ui/     — blocking LVGL loop
```

Thread-safety: LVGL is single-task. The MQTT client runs on its own task and posts incoming messages through a FreeRTOS queue; `ui_manager_run()` drains the queue each iteration before calling `lv_timer_handler()`. `led_hal` and `esp_timer` callbacks are task-safe and called directly.

`net_hal_init()` blocks for up to 15 s waiting for WiFi to associate; on timeout it returns an error and the UI still comes up. MQTT auto-reconnects on its own.

## Hardware pins

| Function | GPIO |
|---|---|
| LCD SPI  | SCLK 7, MOSI 6, DC 15, RST 21, CS 14 (SPI2_HOST) |
| Backlight | 22 (LEDC PWM) |
| RGB LED | 8 (WS2812) |
| LCD res | 172 × 320 |

## Partition table

WiFi + MQTT push the binary just past the 1 MB default single-app partition. `partitions.csv` reserves 2 MB for the app on the 4 MB flash; the rest is free for future use (NVS keys, SPIFFS, etc.).

## Make targets

```
make build           # compile
make flash           # upload  (PORT=/dev/ttyACM0 to override)
make monitor         # serial console (Ctrl+] to exit)
make all             # build + flash
make menuconfig      # ESP-IDF Kconfig UI
make deps            # idf.py reconfigure
make clean           # idf.py fullclean
```

## Extending

| Adding | Where |
|---|---|
| New peripheral driver | `components/<name>_hal/` + add to `main/CMakeLists.txt` REQUIRES |
| New screen / widget   | `components/app_ui/src/ui_manager.c` — search for `MODIFY:` markers |
| New notification source | Implement on top of `ui_manager_post_notification()` — safe from any task |
| TLS to broker         | `mqtts://...` URI + bundled root cert via `esp_crt_bundle` |
| JSON payload parsing  | Parse in `mqtt_event_handler` `MQTT_EVENT_DATA` case (`components/net_hal/src/net_hal.c`) |
