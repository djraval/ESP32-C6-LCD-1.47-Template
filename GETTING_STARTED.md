# Getting Started with ESP32-C6

This guide will help you build and flash your first ESP32-C6 application.

## What You'll Learn
- How to build and flash your first ESP32 program
- Understanding the basic project structure
- Making simple changes to see results

## Prerequisites
1. ESP-IDF installed (see [ESP-IDF Installation Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/))
2. VS Code with ESP-IDF extension
3. ESP32-C6 board connected to your computer

## Quick Start

### Step 1: Open the Project
1. Open VS Code
2. Open this folder: `ESP32C6`
3. VS Code should automatically detect it's an ESP-IDF project

### Step 2: Build the Project
1. Press `Ctrl+Shift+P` (Windows) or `Cmd+Shift+P` (Mac)
2. Type "ESP-IDF: Build project"
3. Press Enter and wait for build to complete

### Step 3: Flash to Your ESP32-C6
1. Connect your ESP32-C6 to USB port
2. Press `Ctrl+Shift+P` again
3. Type "ESP-IDF: Flash device"
4. Press Enter and wait for flashing

### Step 4: Monitor the Output
1. Press `Ctrl+Shift+P` again
2. Type "ESP-IDF: Monitor device"
3. You should see:
   ```
   Hello World from ESP32-C6!
   ESP32-C6 is running...
   ESP32-C6 is running...
   ```

You have successfully programmed your ESP32-C6.

## Making Your First Change

Let's modify the program to print your name:

1. Open the main file: Click on `main/main.c`
2. Find this line (around line 35):
   ```c
   ESP_LOGI(TAG, "Hello World from ESP32-C6!");
   ```
3. Change it to:
   ```c
   ESP_LOGI(TAG, "Hello from [YOUR NAME]'s ESP32-C6!");
   ```
4. Save the file (Ctrl+S)
5. Build and flash again (repeat Steps 2-4 above)

You should now see your personalized message.

## Project Structure

```
ESP32C6/
├── main/
│   └── main.c          ← Your code goes here (start here!)
├── CMakeLists.txt      ← Build configuration (don't touch)
├── sdkconfig           ← Auto-generated settings (ignore this!)
└── README.md           ← Detailed documentation
```

For beginners: Only focus on `main/main.c` - that's where your code lives.

## What About the Other Files?

Don't worry about these files initially - they're automatically managed:

- `sdkconfig`: A large configuration file (2000+ lines) - ESP-IDF manages this
- `build/` folder: Compiled code - created automatically
- `.vscode/` folder: VS Code settings - already configured for you
- `sdkconfig.defaults`: Project defaults - only for advanced users

Focus on writing code in `main/main.c` and using VS Code commands.

## Next Steps

Once you're comfortable with the basics:

1. Learn ESP-IDF basics: [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/)
2. Try examples: Look at ESP-IDF example projects
3. Add features: WiFi, Bluetooth, sensors

## Troubleshooting

- Build fails: Check that ESP-IDF is properly installed
- Flash fails: Make sure ESP32-C6 is connected and drivers installed
- No output: Check serial port connection and monitor settings

## Summary

You now have a working ESP32-C6 development environment. Start experimenting with the code in `main/main.c` to build IoT projects.

---
*This is a beginner-friendly introduction. For advanced configuration, see README.md*
