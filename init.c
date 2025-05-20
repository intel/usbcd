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
#include "opm.h"
#include <glib/gprintf.h>

void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data) {
    g_printf("%s\n", message);
}

static void update_daemon_conf()
{
    GKeyFile *keyfile = g_key_file_new();
    if (g_key_file_load_from_file(keyfile, "usbcd.conf", G_KEY_FILE_NONE, NULL)) {
        gchar *log_level = g_key_file_get_string(keyfile, "general", "LogLevel", NULL);
        if (g_strcmp0(log_level, "debug") == 0) {
            g_log_set_handler(NULL, G_LOG_LEVEL_DEBUG | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
        } else if (g_strcmp0(log_level, "info") == 0) {
            g_log_set_handler(NULL, G_LOG_LEVEL_INFO | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
        } else {
            g_log_set_handler(NULL, G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
        }
        g_free(log_level);
    } else {
        g_log(NULL, G_LOG_LEVEL_WARNING, "Failed to load configuration file: usbcd.conf");
    }
    g_key_file_free(keyfile);
}

int init_system()
{
    update_daemon_conf();
    // Other initialization code...
    return 0;
}
