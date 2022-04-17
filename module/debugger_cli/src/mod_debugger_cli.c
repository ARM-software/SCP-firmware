/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cli.h>
#include <cli_platform.h>

#include <mod_debugger_cli.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_module.h>
#include <fwk_status.h>

enum debugger_cli_internal_event_idx {
    DEBUGGER_CLI_INTERNAL_EVENT_IDX_ENTER_DEBUGGER,
    DEBUGGER_CLI_INTERNAL_EVENT_IDX_COUNT
};

static const fwk_id_t debugger_cli_event_id_request =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DEBUGGER_CLI,
        DEBUGGER_CLI_INTERNAL_EVENT_IDX_ENTER_DEBUGGER);

static struct mod_timer_alarm_api *alarm_api;

static void alarm_callback(uintptr_t module_idx)
{
    int status;
    char ch = 0;
    struct fwk_event *event;

    /* Get the pending character (if any) from the UART without blocking */
    status = fwk_io_getch(fwk_io_stdin, &ch);

    if (status == FWK_SUCCESS) {
        /* Ctrl-E has been pressed */
        if (ch == 0x05) {
            /* Send out an event to start the CLI not inside an ISR */
            event = &((struct fwk_event){
                .source_id = FWK_ID_MODULE(module_idx),
                .target_id = FWK_ID_MODULE(module_idx),
                .id = debugger_cli_event_id_request
            });

            status = fwk_put_event(event);

            fwk_assert(status == FWK_SUCCESS);
        }
    }
}

static int start_alarm(fwk_id_t id)
{
    const struct mod_debugger_cli_module_config *module_config;

    /* Retrieve the module config as specified by the platform config.c file */
    module_config = fwk_module_get_data(id);

    /* Start the UART polling alarm, recurring with the given time period */
    return alarm_api->start(module_config->alarm_id, module_config->poll_period,
        MOD_TIMER_ALARM_TYPE_PERIODIC, alarm_callback,
        fwk_id_get_module_idx(id));
}

static int stop_alarm(fwk_id_t id)
{
    const struct mod_debugger_cli_module_config *module_config;

    /* Retrieve the module config as specified by the platform config.c file */
    module_config = fwk_module_get_data(id);

    /* Stop the UART polling alarm */
    return alarm_api->stop(module_config->alarm_id);
}

static int debugger_cli_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

static int debugger_cli_element_init(fwk_id_t element_id, unsigned int unused,
                                    const void *data)
{
    return FWK_SUCCESS;
}

static int debugger_cli_bind(fwk_id_t id, unsigned int round)
{
    const struct mod_debugger_cli_module_config *module_config;

    /* Only bind in the first round of calls */
    if (round > 0)
        return FWK_SUCCESS;

    /* Retrieve the module config as specified by the platform config.c file */
    module_config = fwk_module_get_data(id);

    /* Bind to the specified alarm in order to poll the UART */
    return fwk_module_bind(module_config->alarm_id, MOD_TIMER_API_ID_ALARM,
        &alarm_api);
}

static int debugger_cli_start(fwk_id_t id)
{
    return start_alarm(id);
}

static int debugger_cli_process_event(const struct fwk_event *event,
                                      struct fwk_event *resp_event)
{
    int status;
    int start_alarm_status;

    switch (event->id.event.event_idx) {
    case DEBUGGER_CLI_INTERNAL_EVENT_IDX_ENTER_DEBUGGER:
        /* Start the CLI, blocking the rest of the event queue */
        cli_print("[CLI_DEBUGGER_MODULE] Entering CLI\n");

        status = stop_alarm(event->target_id);
        if (status != FWK_SUCCESS) {
            goto exit_cli;
        }

        status = cli_start();

        /* start the alarm regardless of the cli's return status */
        start_alarm_status = start_alarm(event->target_id);
        if (status == FWK_SUCCESS) {
            status = start_alarm_status;
        }

exit_cli:
        cli_print("\n[CLI_DEBUGGER_MODULE] Exiting CLI\n");
        return status;
    default:
        return FWK_E_PARAM;
    }
}

const struct fwk_module module_debugger_cli = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = DEBUGGER_CLI_INTERNAL_EVENT_IDX_COUNT,
    .api_count = 0,
    .init = debugger_cli_init,
    .element_init = debugger_cli_element_init,
    .bind = debugger_cli_bind,
    .start = debugger_cli_start,
    .process_event = debugger_cli_process_event
};
