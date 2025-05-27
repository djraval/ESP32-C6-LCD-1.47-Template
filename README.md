# ESP32-C6 Project

A basic ESP32-C6 project using ESP-IDF v5.4.1 that demonstrates basic functionality with WiFi 6, Bluetooth 5, and IEEE802.15.4 capabilities.

## New to IoT Development?
Start here: [GETTING_STARTED.md](GETTING_STARTED.md) - A beginner-friendly guide

## Documentation Structure
- [GETTING_STARTED.md](GETTING_STARTED.md) - Beginner guide (start here!)
- [README.md](README.md) - This file (project overview and setup)
- [ADVANCED.md](ADVANCED.md) - Advanced configuration and assumptions

## Hardware

- **Chip**: ESP32-C6FH4 (QFN32) revision v0.1
- **Features**: WiFi 6, BT 5, IEEE802.15.4
- **Crystal**: 40MHz
- **Flash**: 2MB
- **MAC Address**: 9c:9e:6e:7b:3f:34
- **Serial Port**: COM13 (Windows) - configured as default

## Setup Assumptions & Decisions

This project was configured with the following assumptions and decisions. Update as needed for your environment:

### **Development Environment**
- **OS**: Windows (PowerShell)
- **IDE**: VS Code with ESP-IDF extension
- **ESP-IDF**: v5.4.1 (installed via Windows installer)
- **Installation Path**: `C:\Espressif\frameworks\esp-idf-v5.4.1`
- **Python**: 3.11.2 (from ESP-IDF environment)

### **Serial Communication**
- **Default Port**: COM13 (set in VS Code settings)
- **Flash Baud Rate**: 460800 (VS Code default)
- **Monitor Baud Rate**: 115200 (standard)
- **No port cycling**: Direct connection to COM13

### **Build Configuration**
- **Optimization**: Size over speed (typical embedded approach)
- **Logging**: INFO level for development (good balance)
- **Bootloader**: INFO level logging enabled
- **Compiler**: GCC with C11/C++17 standards
- **Build Type**: Standard application (not RAM-only)

### **Project Structure Decisions**
- **Source Control**: Git with comprehensive .gitignore
- **Configuration**: `sdkconfig.defaults` for team settings, `sdkconfig` ignored
- **VS Code**: Integrated tasks for build/flash/monitor
- **Documentation**: Inline comments explaining all assumptions

### **Security & Features**
- **Security**: None enabled initially (add for production)
- **WiFi**: Not configured (add when needed)
- **Bluetooth**: Not configured (add when needed)
- **OTA**: Not configured (add when needed)
- **Encryption**: Not enabled (add for production)

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

### **Modifying Setup Assumptions**

If your environment differs from the assumptions above, here's how to adapt:

#### **Different Serial Port**
1. **VS Code**: Edit `.vscode/settings.json` → change `"idf.portWin": "COM13"`
2. **Command Line**: Use `idf.py -p YOUR_PORT flash monitor`
3. **Linux/macOS**: Change to `"idf.port": "/dev/ttyUSB0"` in settings

#### **Different ESP-IDF Version**
1. Update `.vscode/settings.json` paths to match your ESP-IDF installation
2. Check `sdkconfig.defaults` for version-specific configurations
3. Run `idf.py reconfigure` after ESP-IDF updates

#### **Different Optimization/Logging**
1. Edit `sdkconfig.defaults` file
2. Run `idf.py reconfigure` to apply changes
3. Or use `idf.py menuconfig` for interactive configuration

#### **Adding WiFi/Bluetooth**
1. Uncomment relevant sections in `sdkconfig.defaults`
2. Add initialization code in `main/main.c`
3. Update `main/CMakeLists.txt` if adding new components

#### **Production Deployment**
1. Enable security features in `sdkconfig.defaults`
2. Change optimization to performance if needed
3. Disable debug logging for production builds
4. Consider OTA update mechanisms

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
