/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <stdint.h>

/*!
 * \addtogroup GroupCLIDebugger
 * @{
 */

/*!
 * Length of checkpoint thread names.
 *
 */
#define CHECKPOINT_NAME_LEN 16

/*!
 * Length of checkpoint tags.
 *
 */
#define CHECKPOINT_TAG_LEN 32

/*!
 * \brief Checkpoint bypass value.
 *
 */
enum cp_bypass_value {
    /*! Stop execution when checkpoint is reached. */
    CHECKPOINT_ENABLED = 0,
    /*! Ignore checkpoint */
    CHECKPOINT_DISABLED = -1
};

/*!
 * \brief Checkpoint descriptor
 *
 * \details This structure holds the proparities of a checkpoint.
 *      It should be populated by calling checkpoint_register function.
 *
 */
typedef struct {
    /*! Index in the checkpoint table */
    uint32_t index;
    /*! Description of the function registering */
    char name[CHECKPOINT_NAME_LEN];
    /*! Tag is used to identify the checkpoint */
    char tag[CHECKPOINT_TAG_LEN];
    /*! Enable or disable the checkpoint */
    volatile int32_t bypass;
    /*! Valid checkpoint in the checkpoint table */
    bool in_use;
    /*! Execution pasused at this checkpoint */
    volatile bool pause;
} checkpoint_st;

/*!
 * \brief Enables all checkpoints in the system.
 *
 */
void checkpoint_enable_all(void);

/*!
 * \brief Disables all checkpoints in the system.
 *
 */
void checkpoint_disable_all(void);

/*!
 * \brief Request a new checkpoint structure
 *
 * \param c Pointer to a checkpoint_st pointer, is set when the function returns
 *          successfully.
 * \param name Name or description of the function registering. Must be less
 *             than CHECKPOINT_NAME_LEN characters in length.
 * \retval FWK_SUCCESS if operation is successful.
 * \retval FWK_E_NOMEM when checkpoints limit is reached.
 *
 */
int32_t checkpoint_register(checkpoint_st **c, char *name);

/*!
 * \brief Insert a checkpoint
 *
 * \details Checkpoint function for use within code. When this function is
 *      called and checkpoints are disabled, it will return immediately.
 *      If checkpoints are enabled, it will print a message indicating that
 *      it is pausing and activate the CLI. When the command line exists This
 *      function will return and normal execution is resumed.
 *
 * \param c Pointer to valid checkpoint structure, obtained through a call to
 *          checkpoint_register.
 * \param file Name of the file in which the checkpoint exists,
 *             use __FILE__ macro.
 * \param line Line number of the checkpoint, use __LINE__ macro.
 * \param tag Checkpoint tag string.
 */
void checkpoint(checkpoint_st *c, char *file, int32_t line, char *tag);

/*!
 * @}
 */

#endif /* CHECKPOINT_H */
