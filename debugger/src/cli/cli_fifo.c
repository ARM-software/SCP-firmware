/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cli_config.h>
#include <cli_fifo.h>

#include <stdbool.h>
#include <stdint.h>

uint32_t fifo_init(fifo_st *fifo, char *buf, uint32_t buf_size)
{
    if (fifo == 0 || buf == 0 || buf_size == 0)
        return FWK_E_PARAM;

    fifo->get_ptr = 0;
    fifo->put_ptr = 0;
    fifo->count = 0;
    fifo->high_water = 0;
    fifo->reset_high_water = false;
    fifo->buf_size = buf_size;
    fifo->buf = buf;
    return FWK_SUCCESS;
}

uint32_t fifo_get(fifo_st *fifo, char *val)
{
    /* Checking parameters. */
    if (fifo == 0 || val == 0)
        return FWK_E_PARAM;

    /* Making sure FIFO isn't empty. */
    if (fifo->get_ptr != fifo->put_ptr) {
        *val = fifo->buf[fifo->get_ptr];
        fifo->get_ptr = (fifo->get_ptr + 1) % fifo->buf_size;
        fifo->count = fifo->count - 1;
        /* Tracking FIFO high-water mark. */
        if ((fifo->reset_high_water == true) && (fifo->count == 0)) {
            fifo->high_water = 0;
            fifo->reset_high_water = false;
        }
        return FWK_SUCCESS;
    }

    return FWK_E_DATA;
}

uint32_t fifo_put(fifo_st *fifo, char *val)
{
    /* Checking parameters. */
    if (fifo == 0 || val == 0)
        return FWK_E_PARAM;

    uint32_t newPutPtr = (fifo->put_ptr + 1) % fifo->buf_size;

    /* Making sure FIFO isn't full. */
    if (newPutPtr != fifo->get_ptr) {
        fifo->buf[fifo->put_ptr] = *val;
        fifo->put_ptr = newPutPtr;
        fifo->count = fifo->count + 1;
        /* Tracking FIFO high-water mark. */
        if (fifo->count > fifo->high_water && fifo->reset_high_water == false)
            fifo->high_water = fifo->count;

        return FWK_SUCCESS;
    }

    return FWK_E_NOMEM;
}

uint32_t fifo_free_space(fifo_st *fifo)
{
    /* Checking parameters. */
    if (fifo == 0)
        return FWK_E_PARAM;

    return fifo->buf_size - fifo->count - 1;
}

uint32_t fifo_count(fifo_st *fifo)
{
    /* Checking parameters. */
    if (fifo == 0)
        return FWK_E_PARAM;

    return fifo->count;
}

uint32_t fifo_capacity(fifo_st *fifo)
{
    /* Checking parameters. */
    if (fifo == 0)
        return FWK_E_PARAM;

    return fifo->buf_size - 1;
}

uint32_t fifo_high_water(fifo_st *fifo)
{
    /* Checking parameters. */
    if (fifo == 0)
        return FWK_E_PARAM;

    return fifo->high_water;
}

uint32_t fifo_high_water_reset(fifo_st *fifo)
{
    /* Checking parameters. */
    if (fifo == 0)
        return FWK_E_PARAM;

    fifo->reset_high_water = true;
    return FWK_SUCCESS;
}

uint32_t fifo_empty(fifo_st *fifo)
{
    /* Check for valid pointer. */
    if (fifo == 0)
        return FWK_E_PARAM;

    fifo->get_ptr = 0;
    fifo->put_ptr = 0;
    fifo->count = 0;
    fifo->high_water = 0;
    fifo->reset_high_water = false;

    return FWK_SUCCESS;
}
