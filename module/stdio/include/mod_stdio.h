
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_STDIO_H
#define MOD_STDIO_H

#include <stdio.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModuleStdio Standard Input/Output Driver
 *
 * \brief I/O backend using the C standard library input/output interface.
 *
 * \details This module enables a firmware implementation to plug the standard
 *      input/output stream into the framework I/O implementation. For the host
 *      platform this is the primary way to read and write input and output
 *      data, but this module may also be used in combination with Arm Compiler
 *      6 to do input/output over a semihosting interface.
 *
 * \{
 */

/*!
 * \brief Input/output element type.
 *
 * \details Standard I/O elements may take the form of an existing open file
 *      stream or, alternatively, a file path. This enumeration determines the
 *      type of the element and, therefore, which configuration parameter will
 *      be used.
 */
enum mod_stdio_element_type {
    /*!
     * \brief Stream.
     *
     * \details Represents an already-open file stream (a `FILE *`). This can be
     *      used to configure an element as `stdin` or `stdout`.
     */
    MOD_STDIO_ELEMENT_TYPE_STREAM,

    /*!
     * \brief File.
     *
     * \details Represents a file to open. This opens a new stream when the
     *      I/O entity is opened, and closes the stream when the I/O entity is
     *      closed.
     */
    MOD_STDIO_ELEMENT_TYPE_PATH,
};

/*!
 * \brief File.
 */
struct mod_stdio_file {
    /*!
     * \brief Filesystem path.
     *
     * \details Valid values depend on the C standard library implementation,
     *      but generally follow the rules of the host operating system.
     */
    const char *path;

    /*!
     * \brief File mode to open the file with.
     *
     * \details Because the framework I/O modes are more limited than those
     *      supported by the C standard library, this allows you to select a
     *      more specific mode to open the file with.
     *
     * \note The framework I/O mode is ignored in this case.
     */
    const char *mode;
};

/*!
 * \brief Element configuration data.
 */
struct mod_stdio_element_cfg {
    /*!
     * \brief Element type.
     *
     * \details Determines the fields used to configure this element.
     */
    enum mod_stdio_element_type type;

    /*!
     * \brief Element data.
     */
    union {
        /*!
         * \brief `FILE *` representing an existing stream.
         */
        FILE *stream;

        /*!
         * \brief File to open.
         */
        struct mod_stdio_file file;
    };
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_STDIO_H */
