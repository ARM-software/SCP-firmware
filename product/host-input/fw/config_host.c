/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdbool.h>
#include <glib.h>
#include <glib-unix.h>
#include <arch_main.h>

/* Glib helpers */

typedef struct {
    GMainLoop *loop;
} VuScmi;

VuScmi scmi = {0};

static gboolean verbose;

static GOptionEntry options[] = {
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be more verbose in output", NULL},
    { NULL }
};

static gboolean hangup(gpointer user_data)
{
    GMainLoop *loop = (GMainLoop *) user_data;
    g_info("%s: caught hangup/quit signal, quitting main loop", __func__);
    g_main_loop_quit(loop);
    return true;
}

/* Main loop */

int main(int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;

    context = g_option_context_new("host-user emulation of SCMI device");
    g_option_context_add_main_entries(context, options, "host-user-scmi");
    if (!g_option_context_parse(context, &argc, &argv, &error))
    {
        g_printerr("option parsing failed: %s\n", error->message);
        exit(1);
    }

    if (verbose) {
        g_log_set_handler(NULL, G_LOG_LEVEL_MASK, g_log_default_handler, NULL);
        g_setenv("G_MESSAGES_DEBUG", "all", true);
    } else {
        g_log_set_handler(NULL,
                          G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR,
                          g_log_default_handler, NULL);
    }

    /*
     * Create the main loop first so all the various sources can be
     * added. SCMI server can add it's GSource watches.
     */
    scmi.loop = g_main_loop_new(NULL, FALSE);

    /* Catch exit signals */
    g_unix_signal_add(SIGHUP, hangup, scmi.loop);
    g_unix_signal_add(SIGINT, hangup, scmi.loop);
    g_unix_signal_add(SIGTERM, hangup, scmi.loop);

    /* Initialize scmi server */
    scmi_arch_init();

    /* Enter main loop */
    g_message("entering main loop, awaiting messages");
    g_main_loop_run(scmi.loop);
    g_message("finished main loop, cleaning up");

    /* Stop scmi server */
    scmi_arch_deinit();

    /* Stop main loop and exit */
    g_main_loop_unref(scmi.loop);
}
