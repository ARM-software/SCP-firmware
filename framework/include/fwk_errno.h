/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Standard return codes.
 */

#ifndef FWK_ERRNO_H
#define FWK_ERRNO_H

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupErrno Return Codes
 * @{
 */

/*! Success */
#define FWK_SUCCESS          0

/*! Invalid parameter(s) */
#define FWK_E_PARAM         -1

/*! Invalid alignment */
#define FWK_E_ALIGN         -2

/*! Invalid size */
#define FWK_E_SIZE          -3

/*! Invalid handler or callback */
#define FWK_E_HANDLER       -4

/*! Invalid access or permission denied */
#define FWK_E_ACCESS        -5

/*! Value out of range */
#define FWK_E_RANGE         -6

/*! Operation timed out */
#define FWK_E_TIMEOUT       -7

/*! Memory allocation failed */
#define FWK_E_NOMEM         -8

/*! Invalid power state */
#define FWK_E_PWRSTATE      -9

/*! Not supported or disabled */
#define FWK_E_SUPPORT       -10

/*! Device error */
#define FWK_E_DEVICE        -11

/*! Handler or resource busy */
#define FWK_E_BUSY          -12

/*! OS error response */
#define FWK_E_OS            -13

/*! Unexpected or invalid data */
#define FWK_E_DATA          -14

/*! Invalid state for the device or component */
#define FWK_E_STATE         -15

/*! Accessing an uninitialized resource */
#define FWK_E_INIT          -16

/*! Configuration overwritten */
#define FWK_E_OVERWRITTEN   -17

/*! Unrecoverable error */
#define FWK_E_PANIC         -18

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_ERRNO_H */
