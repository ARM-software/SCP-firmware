/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <checkpoint.h>
#include <cli.h>

#include <fwk_status.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

checkpoint_st checkpoint_table[CHECKPOINT_NUM] = {0};

/* Reset value for bypass. */
static volatile int32_t bypass_reset = CHECKPOINT_DISABLED;

void checkpoint_enable_all(void)
{
    uint32_t i = 0;

    for (i = 0; i < CHECKPOINT_NUM; i++) {
        /*
         * Bypass set to 0 to ensure thread stops at the next checkpoint. Tag
         * is set to empty string to keep things clean.
         */
        checkpoint_table[i].bypass = CHECKPOINT_ENABLED;
        checkpoint_table[i].tag[0] = 0;
    }

    /* Changing reset value to enabled by default. */
    bypass_reset = CHECKPOINT_ENABLED;
}

void checkpoint_disable_all(void)
{
    uint32_t i = 0;

    for (i = 0; i < CHECKPOINT_NUM; i++) {
        /*
         * Bypass is set to -1 and tag is set to an empty string so checkpoint
         * will be skipped.
         */
        checkpoint_table[i].bypass = CHECKPOINT_DISABLED;
        checkpoint_table[i].tag[0] = 0;
    }
}

int32_t checkpoint_register(checkpoint_st **c, char *name)
{
    uint32_t i = 0;

    for (i = 0; i < CHECKPOINT_NUM; i++) {
        if (checkpoint_table[i].in_use == false) {
            checkpoint_table[i].in_use = true;
            checkpoint_table[i].index = i;
            strncpy(checkpoint_table[i].name, name, CHECKPOINT_NAME_LEN);
            checkpoint_table[i].name[CHECKPOINT_NAME_LEN - 1] = 0;
            checkpoint_table[i].bypass = bypass_reset;
            *c = &checkpoint_table[i];
            return FWK_SUCCESS;
        }
    }

    return FWK_E_NOMEM;
}

void checkpoint(checkpoint_st *c, char *file, int32_t line, char *tag)
{
    /* If tags match or if bypass == 0, stop here. */
    if ((c->bypass == CHECKPOINT_ENABLED) ||
        ((tag != NULL) && (strncmp(tag, c->tag, CHECKPOINT_TAG_LEN) == 0))) {
        cli_printf(NONE,
            "SCP Debugger has stopped at a checkpoint.\n"
            "Tag: %s\n  File: %s\n  Line: %d\n",
            tag,
            file,
            line);

        c->tag[0] = 0;
        c->bypass = CHECKPOINT_ENABLED;

        cli_start();

        cli_printf(NONE,
            "SCP execution resume at a checkpoint.\n"
            "Tag: %s\n  File: %s\n  Line: %d\n",
            tag,
            file,
            line);

        return;
    }

    /* If bypass is less than 0 (-1), checkpoint is disabled so return. */
    if (c->bypass == CHECKPOINT_DISABLED)
        return;
    else {
        /* If bypass is greater than 0, decrement and return. */
        c->bypass = c->bypass - 1;
        return;
    }
}
