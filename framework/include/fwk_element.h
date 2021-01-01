/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Element definitions.
 */

#ifndef FWK_ELEMENT_H
#define FWK_ELEMENT_H

#include <stddef.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \defgroup GroupElement Elements
 * \{
 */

/*!
 * \brief Element descriptor.
 */
struct fwk_element {
    /*! Element name */
    const char *name;

    /*! Number of sub-elements */
    size_t sub_element_count;

    /*!
     * \brief Pointer to element-specific configuration data
     *
     * \details Because each element is expected to have some associated
     *     configuration data this pointer must be non-NULL for the framework
     *     to consider the element as valid.
     *
     *     A fake, non-NULL pointer should be provided in the case where no
     *     element-specific configuration data is available. In this case the
     *     module code must not make any attempt to dereference or store the
     *     pointer during element initialization.
     */
    const void *data;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_ELEMENT_H */
