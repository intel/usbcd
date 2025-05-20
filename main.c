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

extern int init_daemon();
extern int init_system();
extern int init_system_thread();
extern gboolean handle_udev(GIOChannel *source, GIOCondition condition, gpointer user_data) ;


static gboolean verbose = FALSE;
static GOptionEntry entries[] =
{
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL },
  { NULL }
};


int main(int argc, char **argv) {
    GError *error = NULL;
    GOptionContext *context;
    int ret;

    context = g_option_context_new("- usbc daemon");
    g_option_context_add_main_entries(context, entries, NULL);
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(EXIT_FAILURE);
    }


    ret = init_system();

    if(ret <0){
        fprintf(stderr, "unable to init system\n");
        exit(EXIT_FAILURE);
    }


    init_system_thread();
    
    
    return 0;
}
