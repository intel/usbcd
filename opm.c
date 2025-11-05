/**
* # MIT License
* 
* Copyright (c) 2024 Intel Corporation
*
* Author: Saranya Gopal <saranya.gopal@intel.com>
* Author: Rajaram Regupathy <rajaram.regupathy@intel.com>
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including but not limited to the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <libnotify/notify.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libudev.h>
#include <glib.h>

int handle_notification(const char *summary, const char *body) {
    NotifyNotification *n;

    notify_init("Basics");

    n = notify_notification_new(summary, body, NULL);
    notify_notification_set_timeout(n, 3000); // 3 seconds

    if (!notify_notification_show(n, NULL)) {
        g_log(NULL, G_LOG_LEVEL_ERROR, "failed to send notification");
        return 1;
    }

    return 0;
}

// Function to identify likely failed alternate modes based on vendor/product ID
static void identify_failed_alternate_modes(const char *idVendor, const char *idProduct, char *notification_body, size_t max_size) {
    if (!idVendor) return;
    
    char mode_info[256] = "";
    
    // Common vendor IDs for alternate mode devices
    if (strcmp(idVendor, "8087") == 0) {  // Intel
        strcat(mode_info, "Likely failed: Thunderbolt/USB4 alternate mode. ");
    } else if (strcmp(idVendor, "0bda") == 0) {  // Realtek
        strcat(mode_info, "Likely failed: DisplayPort alternate mode. ");
    } else if (strcmp(idVendor, "17e9") == 0) {  // DisplayLink
        strcat(mode_info, "Likely failed: DisplayPort alternate mode. ");
    } else if (strcmp(idVendor, "056a") == 0) {  // Wacom
        strcat(mode_info, "Likely failed: HID alternate mode. ");
    } else if (strcmp(idVendor, "05ac") == 0) {  // Apple
        strcat(mode_info, "Likely failed: Thunderbolt/DisplayPort alternate mode. ");
    } else if (strcmp(idVendor, "0451") == 0) {  // Texas Instruments
        strcat(mode_info, "Likely failed: USB-C PD alternate mode. ");
    }
    
    if (strlen(mode_info) > 0) {
        strncat(notification_body, "\n", max_size - strlen(notification_body) - 1);
        strncat(notification_body, mode_info, max_size - strlen(notification_body) - 1);
    }
}

// Function to read billboard alternate mode information from sysfs
static void read_billboard_alternate_mode_info(struct udev_device *dev, char *notification_body, size_t max_size) {
    const char *device_path = udev_device_get_syspath(dev);
    char sysfs_path[512];
    FILE *file;
    char buffer[256];
    
    if (!device_path) return;
    
    // Try to read VID (Vendor ID) and PID (Product ID) for additional context
    const char *idVendor = udev_device_get_sysattr_value(dev, "idVendor");
    const char *idProduct = udev_device_get_sysattr_value(dev, "idProduct");
    
    if (idVendor && idProduct) {
        char vid_pid_info[128];
        snprintf(vid_pid_info, sizeof(vid_pid_info), 
                 "\nVID:PID = %s:%s", idVendor, idProduct);
        strncat(notification_body, vid_pid_info, max_size - strlen(notification_body) - 1);
        
        // Identify common alternate mode SVIDs based on vendor
        identify_failed_alternate_modes(idVendor, idProduct, notification_body, max_size);
    }
    
    // Try to read additional USB descriptors if available
    snprintf(sysfs_path, sizeof(sysfs_path), "%s/descriptors", device_path);
    file = fopen(sysfs_path, "rb");
    if (file) {
        // Read and parse USB descriptors for Billboard Capability Descriptor
        // Billboard devices should have a Billboard Capability Descriptor that contains
        // information about failed alternate modes
        g_log(NULL, G_LOG_LEVEL_DEBUG, "Found descriptors file: %s", sysfs_path);
        fclose(file);
    }
}

// Enhanced billboard enumeration with detailed alternate mode failure information
int handle_billboard_enumeration(struct udev_device *dev) {
    char notification_body[512];
    const char *device_path = udev_device_get_syspath(dev);
    const char *product = udev_device_get_sysattr_value(dev, "product");
    const char *manufacturer = udev_device_get_sysattr_value(dev, "manufacturer");
    const char *bm_attributes = udev_device_get_sysattr_value(dev, "bmAttributes");
    
    // Initialize notification body with basic information
    snprintf(notification_body, sizeof(notification_body), 
             "USB-C device failed to enter Alternate Mode");
    
    // Add device identification if available
    if (product || manufacturer) {
        char device_info[256];
        snprintf(device_info, sizeof(device_info), "\nDevice: %s%s%s",
                 manufacturer ? manufacturer : "",
                 (manufacturer && product) ? " " : "",
                 product ? product : "Unknown");
        strncat(notification_body, device_info, sizeof(notification_body) - strlen(notification_body) - 1);
    }
    
    // Parse bmAttributes for failure reasons
    if (bm_attributes) {
        int bm_attr_val = (int)strtol(bm_attributes, NULL, 16);
        char failure_info[256] = "";
        
        // Interpret bmAttributes based on USB Billboard Device Class specification
        // Bit 0: Unspecified Error occurred during configuration
        // Bit 1: Could not negotiate configuration with device
        // Bit 2: Insufficient power available for configuration 
        // Bit 3: Device does not support this configuration
        // Bit 4-6: Reserved
        // Bit 7: Device requires more time to complete configuration
        
        if (bm_attr_val & 0x01) {
            strcat(failure_info, "Configuration error occurred. ");
        }
        if (bm_attr_val & 0x02) {
            strcat(failure_info, "Negotiation failed with device. ");
        }
        if (bm_attr_val & 0x04) {
            strcat(failure_info, "Insufficient power for alternate mode. ");
        }
        if (bm_attr_val & 0x08) {
            strcat(failure_info, "Device doesn't support configuration. ");
        }
        if (bm_attr_val & 0x80) {
            strcat(failure_info, "Device needs more time to configure. ");
        }
        
        if (strlen(failure_info) > 0) {
            strncat(notification_body, "\nFailure reason: ", 
                    sizeof(notification_body) - strlen(notification_body) - 1);
            strncat(notification_body, failure_info, 
                    sizeof(notification_body) - strlen(notification_body) - 1);
        }
    }
    
    // Try to read additional billboard-specific information
    read_billboard_alternate_mode_info(dev, notification_body, sizeof(notification_body));
    
    // Add common alternate mode information
    strncat(notification_body, 
            "\nPossible failed modes: DisplayPort, Thunderbolt, HDMI, or other USB-C alternate modes.",
            sizeof(notification_body) - strlen(notification_body) - 1);
    
    g_log(NULL, G_LOG_LEVEL_INFO, "Enhanced Billboard: %s", notification_body);
    
    return handle_notification("USB-C Alternate Mode Failure", notification_body);
}

int handle_bandwidth_notification(const char *tunnel_event) {
    if (strcmp(tunnel_event, "low bandwidth") == 0)
        return handle_notification("Low bandwidth on Thunderbolt bus",
			        "Attached device may not work properly");
    else if (strcmp(tunnel_event, "insufficient bandwidth") == 0)
	return handle_notification("Insufficient bandwidth on Thunderbolt bus",
			        "Attached device cannot be used now");
    return 0;
}

int handle_charger_notification(struct udev_device *dev) {
    const char *charger_type_str = udev_device_get_sysattr_value(dev, "type");
    const char *voltage_now_str = udev_device_get_sysattr_value(dev, "voltage_now");
    const char *current_now_str = udev_device_get_sysattr_value(dev, "current_now");

    if (strcmp(charger_type_str, "USB") != 0)
	return 0; //Not a USB-C charger notification

    if (voltage_now_str && current_now_str) {
        long int voltage_now = strtol(voltage_now_str, NULL, 10);
        long int current_now = strtol(current_now_str, NULL, 10);
        long int power_now = voltage_now * current_now;
        g_log(NULL, G_LOG_LEVEL_DEBUG, "Power Supply: voltage_now=%ld, current_now=%ld, power_now=%ld", voltage_now, current_now, power_now);

	if (!power_now)
		return 1; //power_now zero means invalid
	power_now = power_now / 1000000;

        // Read the sysfs value for required power
        FILE *file = fopen("/sys/class/powercap/intel-rapl:0/constraint_1_power_limit_uw", "r");
        if (file == NULL) {
            g_log(NULL, G_LOG_LEVEL_ERROR, "Unable to open sysfs file: %s", strerror(errno));
            return 1;
        }

        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            long int reqd_power = strtol(buffer, NULL, 10);
            g_log(NULL, G_LOG_LEVEL_DEBUG, "Required Power: reqd_power=%ld", reqd_power);

            // Check if power_now is greater than or less than reqd_power
            if (power_now > reqd_power) {
                g_log(NULL, G_LOG_LEVEL_DEBUG, "Power now (%ld) is greater than required power (%ld)", power_now, reqd_power);
            } else {
                g_log(NULL, G_LOG_LEVEL_INFO, "Power now (%ld) is less than required power (%ld) - may charge slowly", power_now, reqd_power);
                // Create a notification for low power source
                handle_notification("Connected low power source", "Device may charge very slowly");
            }
        } else {
            g_log(NULL, G_LOG_LEVEL_ERROR, "Unable to read sysfs file: %s", strerror(errno));
        }

        fclose(file);
    } else {
        g_log(NULL, G_LOG_LEVEL_WARNING, "Power Supply: Unable to get voltage_now or current_now");
    }

    return 0; // Event handled
}
