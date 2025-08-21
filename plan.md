```markdown
# Detailed Implementation Plan for the Smart Kühlschrank Firmware

This plan describes the creation of a modular C firmware application that reads sensor values from files in the "Workspace" directory, displays data and warnings on a 2×40 I2C display, and dynamically adjusts its logging level via a simulated button. The firmware will run on an embedded Linux platform, and the development environment will be emulated using Docker and QEMU.

---

## Directory and File Structure

Create a new directory in the project root named `firmware/` containing the following files:

- **smart_fridge.c** – Contains the main application logic.
- **sensor.h & sensor.c** – Encapsulate functions to read sensor values (temperature, door state, energy) from files.
- **display.h & display.c** – Provide I2C display driver functions to initialize, update, and show warnings (formatted into two rows of 40 characters; the first character of row one indicates the active debug level).
- **logging.h & logging.c** – Implement a logging module with levels (DEBUG, INFO, WARNING, ERROR) and functions to log messages and dynamically adjust the logging level upon a button simulation.
- **config.h** – Defines global constants such as file paths (for sensor files and button press simulation), threshold values (e.g., maximum temperature, door open duration), and enum definitions for logging levels.
- **Makefile** – Contains targets for building the firmware (`all`, `clean`) and compiles with strict flags (e.g., `-Wall -Werror`).
- **Dockerfile** – Builds the firmware within a Docker container. It installs necessary packages (gcc, make, QEMU) and compiles the code using the Makefile.
- **run_emulator.sh** – A shell script to launch the emulated environment with QEMU (using a command such as `qemu-system-arm -M versatilepb -kernel smart_fridge` with appropriate flags) and log output for debugging.
- **DEBUG_TESTING.md** – Documents the strategy for debugging and testing the firmware, including simulation instructions, error reproduction steps, and use of tools like Valgrind for memory management checks.

---

## File-Wise Changes Outline

### 1. firmware/config.h
- **Define Constants:**  
  - Sensor file paths: e.g., `#define TEMPERATURE_FILE "Workspace/temperature.txt"`, `DOOR_FILE`, `ENERGY_FILE`, `BUTTON_FILE`.
  - Threshold values: e.g., `#define MAX_TEMP_THRESHOLD 8.0`, `#define DOOR_OPEN_THRESHOLD 30` (seconds).
  - Enumerate logging levels (DEBUG, INFO, WARNING, ERROR).

### 2. firmware/logging.h & logging.c
- **Functionality:**
  - Declare `void log_message(int level, const char *message);`
  - Implement a setter function `void set_log_level(int new_level);` to update the global debug level.
  - Ensure that every file operation or sensor anomaly is logged appropriately with error checking.
  - Check for a simulated button press (reading from `BUTTON_FILE`) to increment the debug level and display it on the I2C output.

### 3. firmware/sensor.h & sensor.c
- **Sensor Data Handling:**
  - Define a structure:
    ```c
    typedef struct {
      float temperature;
      int doorOpen; // e.g., 1 for open, 0 for closed
      float energy;
    } SensorData;
    ```
  - Implement `int read_sensor_values(SensorData *data);`
    - Open each sensor file from the "Workspace" directory.
    - Validate file content and handle missing or malformed files with error codes.
    - Use file timestamps to only re-read when modifications occur.

### 4. firmware/display.h & display.c
- **I2C Display Functions:**
  - Implement `void init_display(void);`
  - Implement `void update_display(const SensorData data, int currentLogLevel);`
    - Format a display buffer for two lines: first line shows the current debug level (as the first character) followed by sensor values (e.g., "T: 5C/D: closed/E: 120W"), and the second line shows warnings if thresholds are exceeded.
  - Use string formatting ensuring fixed-length output (40 characters per row) for a clear, consistent UI.

### 5. firmware/smart_fridge.c
- **Main Application Workflow:**
  - Initialize modules by calling `init_display()`, configuring logging, and preparing sensor systems.
  - Start an infinite loop that:
    - Checks sensor files every second using `read_sensor_values()` (leveraging file timestamps for efficiency).
    - Evaluates the sensor data against thresholds (e.g., temperature too high, door open too long) and logs warnings via `log_message()` coupled with `display_warning()` if needed.
    - Monitors the button press simulation to adjust the debug level.
    - Uses `sleep(1)` for time control between iterations.
  - Include proper error handling at every step (e.g., file open errors, parsing errors) to prevent crashing.

### 6. firmware/Makefile
- **Build Setup:**
  - Set variables for compiler and flags.
  - Define rules to compile individual .c files and link them into an executable.
  - Provide a `clean` target to remove generated binaries.

### 7. firmware/Dockerfile
- **Emulation Environment:**
  - Use an Ubuntu base image.
  - Install dependencies: gcc, make, qemu-system-arm.
  - Copy firmware source files into the container.
  - Run `make` to compile the firmware.
  - Set the container entrypoint to execute `run_emulator.sh`.

### 8. firmware/run_emulator.sh
- **Emulator Script:**
  - Write a script that:
    - Invokes QEMU with appropriate parameters (e.g., `qemu-system-arm -M versatilepb -kernel smart_fridge`).
    - Redirects and logs output to aid debugging.
    - Cleans up after execution if necessary.

### 9. firmware/DEBUG_TESTING.md
- **Testing Strategy Documentation:**
  - Explain how to simulate sensor file modifications manually in the "Workspace" folder.
  - Detail how to trigger the button press simulation for logging level changes.
  - Enumerate testing steps for each module using unit tests where applicable.
  - Describe error handling tests (missing/corrupt files) and memory leak checks (using Valgrind or similar tools).

---

## Docker & QEMU Integration

- The **Dockerfile** and **run_emulator.sh** will allow developers to build and test the firmware in an emulated environment that mimics the embedded Linux platform.
- Developers can build the Docker container, run it, and monitor log outputs and emulated display output, ensuring the firmware responds correctly to simulated sensor changes and button presses.

---

## UI/UX Considerations for the I2C Display

- The I2C display is text-only with two rows of 40 characters.
- **Row 1:** Begins with the current debug level (displayed as a digit) followed by real-time sensor readings (formatted with labels for temperature, door state, and energy consumption).
- **Row 2:** Displays warnings when thresholds are exceeded (e.g., "Temp high!" or "Door open too long!").
- The text layout is kept clean and balanced using fixed-width formatting to ensure readability even when sensor values update.

---

## Strategy for Debugging and Testing

- **Sensor Simulation:** Manually update the sensor files in the "Workspace" directory and verify that the firmware detects changes and logs the corresponding events.
- **Error Condition Handling:** Test scenarios with missing or invalid sensor files to confirm that error messages are logged and safe default values are used.
- **Logging & Debug Level:** Press the simulated button (via updating the `BUTTON_FILE`) to ensure that logging levels can be incremented and that the new level is displayed on the I2C screen.
- **Memory Management:** Perform memory leak checks using Valgrind and ensure all file handles are properly closed.
- **Docker/QEMU Testing:** Build and run the Docker container; use QEMU to emulate the target hardware and validate that the firmware behaves as expected in an environment similar to the production system.

---

## Summary

- Developed a modular C firmware for a smart refrigerator that monitors sensor files and displays readings/warnings on an I2C display.
- Key components include sensor reading (with error checks), dynamic logging with adjustable debug levels, and formatted I2C display output.
- The Dockerfile and run_emulator.sh integrate QEMU to simulate the embedded environment for testing.
- All modules contain robust error handling and adhere to best practices in memory management and logging.
- A dedicated DEBUG_TESTING.md file documents the debugging strategy for both software and hardware faults.
