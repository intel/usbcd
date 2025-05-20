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

int handle_billboard_enumeration() {
    return handle_notification("USB Billboard device detected",
                               "USB-C/PD device failed to enter Alternate Mode and has enumerated as Billboard device");
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
        g_log(NULL, G_LOG_LEVEL_INFO, "Power Supply: voltage_now=%ld, current_now=%ld, power_now=%ld", voltage_now, current_now, power_now);

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
            g_log(NULL, G_LOG_LEVEL_INFO, "Required Power: reqd_power=%ld", reqd_power);

            // Check if power_now is greater than or less than reqd_power
            if (power_now > reqd_power) {
                g_log(NULL, G_LOG_LEVEL_INFO, "Power now (%ld) is greater than required power (%ld)", power_now, reqd_power);
            } else {
                g_log(NULL, G_LOG_LEVEL_INFO, "Power now (%ld) is less than required power (%ld)", power_now, reqd_power);
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
