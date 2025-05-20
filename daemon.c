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
* in the Software without restriction, including without limitation the rights
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <libudev.h>
#include <sys/select.h>
#include <libnotify/notify.h>
#include "opm.h"

int usbcd_udev_parser(struct udev_device *dev) {
    const char *subsystem = udev_device_get_subsystem(dev);
    const char *action = udev_device_get_action(dev);

    if (strcmp(subsystem, "usb") == 0) {
    	const char *interfaceClass = udev_device_get_sysattr_value(dev, "bInterfaceClass");
	if (interfaceClass && strcmp(interfaceClass, "11") == 0) {
            g_log(NULL, G_LOG_LEVEL_INFO, "Billboard scenario");
            return handle_billboard_enumeration();
	}
    } else if (strcmp(subsystem, "power_supply") == 0 && strcmp(action, "remove") != 0) {
        return handle_charger_notification(dev);
    } else if (strcmp(subsystem, "thunderbolt") == 0) {
	const char *tunnel_event = udev_device_get_property_value(dev, "TUNNEL_EVENT");
	if (tunnel_event)
	    return handle_bandwidth_notification(tunnel_event);
    }

    return 0; // Event not handled
}

void listen_to_typec_events() {
    struct udev *udev;
    struct udev_monitor *mon;
    struct udev_device *dev;
    int fd;

    // Initialize udev
    udev = udev_new();
    if (!udev) {
        g_log(NULL, G_LOG_LEVEL_ERROR, "Cannot create udev context.");
        exit(EXIT_FAILURE);
    }

    // Create a udev monitor for the Type-C connector class
    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "typec", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb_power_delivery", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(mon, "thunderbolt", NULL);
    udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", NULL); // Add power_supply subsystem
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);

    // Main loop
    while (1) {
        dev = udev_monitor_receive_device(mon);
        if (dev) {
            // Handle the device event
            usbcd_udev_parser(dev);
            udev_device_unref(dev);
        }
    }
}

gpointer typec_listen_function(gpointer data) {
    GMainLoop *main_loop = (GMainLoop *)data;

    listen_to_typec_events();

    g_main_loop_quit(main_loop);

    return NULL;
}

int init_system_thread()
{
    GMainLoop *main_loop = g_main_loop_new(NULL, FALSE);

    GThread *typec_listen_thread = g_thread_new("USB-C listener thread", typec_listen_function, main_loop);

    g_main_loop_run(main_loop);

    g_thread_join(typec_listen_thread);
    g_main_loop_unref(main_loop);
    
    return 0;
}
