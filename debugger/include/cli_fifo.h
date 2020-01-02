/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CLI_FIFO_H_
#define _CLI_FIFO_H_

/*****************************************************************************/
/* Structure Types                                                           */
/*****************************************************************************/

typedef struct {
    uint32_t put_ptr;
    uint32_t get_ptr;
    uint32_t count;
    uint32_t high_water;
    bool reset_high_water;
    uint32_t buf_size;
    char *buf;
} fifo_st;

/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/

/*
 * fifo_init
 *   Description
 *     Initializes a FIFO structure, the caller must supply the buffer memory.
 *   Parameters
 *     fifo_st *fifo
 *       Pointer to a pre-allocated fifo_st structure.
 *     char *buf
 *       Pointer to memory to store FIFO entries.
 *     uint32_t buf_size
 *       Size of the buffer in bytes.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t fifo_init(fifo_st *fifo, char *buf, uint32_t buf_size);

/*
 * fifo_get
 *   Description
 *     Gets a byte from a FIFO and places into the address supplied.
 *   Parameters
 *     fifo_st *fifo
 *       Pointer to fifo_st structure to get the byte from.
 *     char *val
 *       Pointer to location in which to place the retrieved byte.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t fifo_get(fifo_st *fifo, char *val);

/*
 * fifo_put
 *   Description
 *     Places a byte into a FIFO buffer.
 *   Parameters
 *     fifo_st *fifo
 *       Pointer to fifo_st structure to put the byte into.
 *     char *val
 *       Pointer to location from which to get the byte.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t fifo_put(fifo_st *fifo, char *val);

/*
 * fifo_free_space
 *   Description
 *     Returns the number of unused entries in the FIFO buffer.
 *   Parameters
 *     fifo_st *fifo
 *       Pointer to fifo_st structure to get the free space from.
 *   Return
 *     Number of free spaces in the FIFO.
 */
uint32_t fifo_free_space(fifo_st *fifo);

/*
 * fifo_count
 *   Description
 *     Returns the number of bytes currently in a FIFO.
 *   Paremeters
 *     fifo_st *fifo
 *       Buffer from which to get the count.
 *   Return
 *     Number of bytes in the FIFO.
 */
uint32_t fifo_count(fifo_st *fifo);

/*
 * fifo_capacity
 *   Description
 *     Gets the maximum number of bytes that can be stored by a FIFO.
 *   Parameters
 *     fifo_st *fifo
 *       Pointer to FIFO structure from which to get the capacity.
 *   Return
 *     Total capacity of the FIFO, this will be one less than the buffer size.
 */
uint32_t fifo_capacity(fifo_st *fifo);

/*
 * fifo_high_water
 *   Description
 *     Returns the high-water point from the FIFO buffer.
 *   Parameters
 *     fifo_st *fifo
 *       Buffer structure to get the high-water count from.
 *   Return
 *     High water mark from FIFO.
 */
uint32_t fifo_high_water(fifo_st *fifo);

/*
 * fifo_high_water_reset
 *   Description
 *     Resets the high water counter by freezing it until the buffer is empty.
 *   Paremeters
 *     fifo_st *fifo
 *       Buffer structure to reset the high-water count in.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t fifo_high_water_reset(fifo_st *fifo);

/*
 * fifo_empty
 *   Description
 *     Removes every item from the FIFO buffer.
 *   Parameters
 *     fifo_st *fifo
 *       Pointer to fifo_st structure to empty.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t fifo_empty(fifo_st *fifo);

#endif /* _CLI_FIFO_H_ */
