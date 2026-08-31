# usbcd
USB-C Daemon (Developed with the assistance of Github Co-pilot)

## Overview
usbcd is a USB-C daemon that monitors USB device events and provides notifications for various USB-C scenarios including billboard enumeration, charger notifications, and bandwidth management.

## Dependencies
This project requires the following libraries and development tools:

### System Requirements
- Ubuntu 24.04 LTS or compatible Linux distribution
- CMake 3.0.0 or higher
- GCC compiler
- pkg-config

### Required Libraries
- libudev (USB device monitoring)
- glib-2.0 (GLib utilities and data structures)
- libnotify (Desktop notifications)

## Installation on Ubuntu 24.04 / Ubuntu 26.04

### 1. Update System Packages
```bash
sudo apt update
sudo apt upgrade -y
```

### 2. Install Build Tools
```bash
sudo apt install -y build-essential cmake pkg-config git
```

### 3. Install Required Development Libraries
```bash
sudo apt install -y \
    libudev-dev \
    libglib2.0-dev \
    libnotify-dev
```

### 4. Verify Dependencies (Optional)
Check that all required packages are installed:
```bash
pkg-config --exists libudev && echo "libudev: OK" || echo "libudev: MISSING"
pkg-config --exists glib-2.0 && echo "glib-2.0: OK" || echo "glib-2.0: MISSING"
pkg-config --exists libnotify && echo "libnotify: OK" || echo "libnotify: MISSING"
```

## Building the Project

### 1. Clone the Repository (if not already done)
```bash
git clone <repository-url>
cd usbcd
```

### 2. Create Build Directory
```bash
mkdir -p build
cd build
```

### 3. Configure with CMake
```bash
cmake ..
```

### 4. Compile the Project
```bash
make
```

Alternatively, you can build with multiple cores for faster compilation:
```bash
make -j$(nproc)
```

### 5. Build in Source Directory (Alternative Method)
If you prefer to build directly in the source directory:
```bash
cmake .
make
```

## Running the Daemon

### 1. Basic Execution
```bash
./usbcd
```

### 2. Configuration
The daemon uses the `usbcd.conf` configuration file for settings. Make sure this file is in the same directory as the executable or modify the path in the source code as needed.

#### Log Level Configuration
You can control the logging verbosity by editing the `usbcd.conf` file:

```ini
[general]
LogLevel = info
```

Supported log levels:
- `debug` - Detailed debugging information
- `info` - General information messages (default)
- `warning` - Warning messages only

### 3. Running as a System Service (Automatic Startup)
To run usbcd as a systemd service that starts automatically on boot:

#### Create systemd service file:
```bash
sudo cp usbcd.service /etc/systemd/system/usbcd.service
```

Alternatively, you can create the file manually:
```bash
sudo nano /etc/systemd/system/usbcd.service
```
Then copy the content from the `usbcd.service` file provided in this repository.

#### Install the daemon binary:
```bash
sudo cp usbcd /usr/local/bin/
sudo chmod +x /usr/local/bin/usbcd
```

#### Copy configuration file:
```bash
sudo cp usbcd.conf /etc/usbcd.conf
```

**Important**: The systemd service is configured to run from `/etc` directory to find the configuration file. Make sure `usbcd.conf` is placed in `/etc/usbcd.conf` for the service to work properly.

#### Enable and start the service:
```bash
# Reload systemd to recognize the new service
sudo systemctl daemon-reload

# Enable the service to start on boot
sudo systemctl enable usbcd.service

# Start the service immediately
sudo systemctl start usbcd.service
```

#### Verify the service is running:
```bash
# Check service status
sudo systemctl status usbcd.service

# View service logs
sudo journalctl -u usbcd.service -f
```

#### Service management commands:
```bash
# Stop the service
sudo systemctl stop usbcd.service

# Restart the service
sudo systemctl restart usbcd.service

# Disable automatic startup
sudo systemctl disable usbcd.service

# Check if service is enabled
sudo systemctl is-enabled usbcd.service
```

### 4. Manual Background Execution (Alternative)
To run the daemon manually in the background:
```bash
./usbcd &
```

To stop a background process:
```bash
# Find the process ID
ps aux | grep usbcd
# Kill the process (replace XXXX with actual PID)
kill XXXX
```

## Features
- **USB Device Monitoring**: Monitors USB device insertion/removal events
- **Billboard Device Support**: Handles USB-C billboard device enumeration
- **Desktop Notifications**: Provides user notifications for USB-C events
- **Charger Detection**: Notifies users about charger connection status
- **Bandwidth Management**: Monitors and reports USB-C bandwidth usage

### Future Enhancements
- **USB-C Alternate Modes Display**: Planned enhancements to show detailed alternate mode information (contributions welcome!)
- **Enhanced Mode Detection**: Better detection and display of DisplayPort, Thunderbolt, HDMI, and other alternate modes
- **Improved User Interface**: More intuitive notifications and status reporting

## Troubleshooting

### Permission Issues
If you encounter permission issues when monitoring USB devices, you may need to run with appropriate privileges:
```bash
sudo ./usbcd
```

### Missing Dependencies
If you get compilation errors about missing headers, ensure all development packages are installed:
```bash
sudo apt install -y libudev-dev libglib2.0-dev libnotify-dev
```

### CMake Version Issues
If CMake version is too old:
```bash
sudo apt install -y cmake
# Or install a newer version from Kitware's repository if needed
```

## Development

### Clean Build
To clean and rebuild:
```bash
make clean
make
```

### Debug Build
For debug builds with additional symbols:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## License
MIT License - See LICENSE file for details.

## Authors
- Saranya Gopal <saranya.gopal@intel.com>
- Rajaram Regupathy <rajaram.regupathy@intel.com>

Developed with the assistance of Github Copilot.

## Contributing

We welcome and encourage contributions to the usbcd project! This is an open-source initiative aimed at improving USB-C device management and user experience on Linux systems.

### Areas Where We Need Help

We are particularly interested in contributions in the following areas:

- **USB-C Alternate Modes Display**: Enhancing the user interface to display detailed information about USB-C alternate modes (DisplayPort, Thunderbolt, HDMI, etc.)
- **Mode Detection and Reporting**: Improving detection and reporting of various USB-C modes and capabilities
- **User Interface Improvements**: Better notification systems and user feedback mechanisms
- **Cross-Platform Support**: Extending support to other Linux distributions
- **Documentation**: Improving installation guides, troubleshooting, and usage documentation
- **Testing**: Adding unit tests, integration tests, and platform compatibility testing

### How to Contribute

1. **Fork the repository** and create a feature branch
2. **Follow the existing code style** and add appropriate logging
3. **Test your changes** thoroughly on different systems
4. **Add documentation** for new features
5. **Submit a pull request** with a clear description of your changes

All contributions should include proper `Signed-off-by` tags as shown in the commit history.
