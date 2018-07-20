/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI575_CORE_H
#define SGI575_CORE_H

unsigned int sgi575_core_get_core_per_cluster_count(unsigned int cluster);
unsigned int sgi575_core_get_core_count(void);
unsigned int sgi575_core_get_cluster_count(void);

#endif /* SGI575_CORE_H */
