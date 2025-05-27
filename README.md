# ESP32-C6 Project

A basic ESP32-C6 project using ESP-IDF v5.4.1 that demonstrates basic functionality with WiFi 6, Bluetooth 5, and IEEE802.15.4 capabilities.

## Hardware

- **Chip**: ESP32-C6FH4 (QFN32) revision v0.1
- **Features**: WiFi 6, BT 5, IEEE802.15.4
- **Crystal**: 40MHz
- **Flash**: 2MB
- **MAC Address**: 9c:9e:6e:7b:3f:34

## Features

- Basic "Hello World" application
- FreeRTOS task demonstration
- ESP-IDF logging system
- Continuous status output every 5 seconds

## Prerequisites

- ESP-IDF v5.4.1 or later
- Python 3.8+
- Git
- VS Code (recommended)

## Setup

1. **Install ESP-IDF**: Follow the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/)

2. **Clone this repository**:
   ```bash
   git clone <your-repo-url>
   cd ESP32C6
   ```

3. **Set up ESP-IDF environment**:
   ```bash
   # Windows PowerShell
   & "C:\Espressif\frameworks\esp-idf-v5.4.1\export.ps1"
   
   # Linux/macOS
   . $HOME/esp/esp-idf/export.sh
   ```

## Build and Flash

1. **Set target** (first time only):
   ```bash
   idf.py set-target esp32c6
   ```

2. **Build the project**:
   ```bash
   idf.py build
   ```

3. **Flash to device**:
   ```bash
   idf.py -p COM13 flash    # Windows
   idf.py -p /dev/ttyUSB0 flash  # Linux
   ```

4. **Monitor output**:
   ```bash
   idf.py -p COM13 monitor  # Windows
   idf.py -p /dev/ttyUSB0 monitor  # Linux
   ```

5. **Build, flash, and monitor in one command**:
   ```bash
   idf.py -p COM13 build flash monitor
   ```

## Project Structure

```
ESP32C6/
├── CMakeLists.txt          # Main project CMake file
├── main/
│   ├── CMakeLists.txt      # Main component CMake file
│   └── main.c              # Application entry point
├── build/                  # Build output (auto-generated)
├── sdkconfig               # Project configuration (auto-generated)
└── README.md               # This file
```

## Configuration

The project is configured for:
- **Serial Port**: COM13 (Windows) - adjust as needed
- **Baud Rate**: 115200
- **Target**: ESP32-C6

## Development

### Adding New Features

1. Modify `main/main.c` for application logic
2. Add new components in `components/` directory if needed
3. Update `CMakeLists.txt` files as required

### VS Code Integration

For the best development experience with VS Code:
1. Install the ESP-IDF extension
2. Configure the extension to use your ESP-IDF installation
3. Use Ctrl+Shift+P and search for "ESP-IDF" commands

## Troubleshooting

- **Port issues**: Ensure the correct serial port is specified
- **Build errors**: Check ESP-IDF environment is properly activated
- **Flash errors**: Verify ESP32-C6 is connected and in download mode

## License

This project is open source. Add your preferred license here.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request
