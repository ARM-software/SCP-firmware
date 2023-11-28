/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERFACE_IO_BLOCK
#define INTERFACE_IO_BLOCK

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupInterfaces Interfaces
 * @{
 */

/*!
 * \defgroup GroupIoBlock IO Block module interface
 *
 * \brief Interface definition for IO Block drivers.
 *
 * \details This provides an generic interface for the modules to bind to a
 *          platform specific IO Block driver module.
 * @{
 */

/*!
 * \brief List the types of carveouts supported by the IO Blocks.
 */
enum interface_io_block_carveout_type {
    /*! IO Block carveout type ECAM. */
    INTERFACE_IO_BLOCK_CARVEOUT_TYPE_ECAM,
    /*! IO Block carveout type MMIOL. */
    INTERFACE_IO_BLOCK_CARVEOUT_TYPE_MMIOL,
    /*! IO Block carveout type MMIOH. */
    INTERFACE_IO_BLOCK_CARVEOUT_TYPE_MMIOH,
    /*! Maximum number of carveouts types. */
    INTERFACE_IO_BLOCK_CARVEOUT_TYPE_MAX,
};

/*!
 * \brief List the types of access allowed on the mapped carveouts.
 */
enum interface_io_block_access_type {
    /*! IO Block Access type Non secure. */
    INTERFACE_IO_BLOCK_ACCESS_TYPE_NS,
    /*! IO Block Access type Secure. */
    INTERFACE_IO_BLOCK_ACCESS_TYPE_S,
    /*! IO Block Access type Realm. */
    INTERFACE_IO_BLOCK_ACCESS_TYPE_RL,
    /*! IO Block Access type Root. */
    INTERFACE_IO_BLOCK_ACCESS_TYPE_RT,
    /*! Maximum numbers of access types. */
    INTERFACE_IO_BLOCK_ACCESS_TYPE_MAX
};

/*!
 * \brief Defines base address and size of carveout.
 */
struct interface_io_block_carveout_info {
    /*! Start of the carveout. */
    uint64_t base;
    /*! Size of the carveout. */
    uint64_t size;
    /*! Carveout type: ECAM, MMIOL, MMIOH. */
    enum interface_io_block_carveout_type carveout_type;
    /*! Type of access allowed on the carveout. */
    enum interface_io_block_access_type access_type;
    /*!
     * Target node id. The rootport node to be programmed when bifurcation mode
     * is enabled.
     */
    uint16_t target_id;
    /*! Index of mapped carveout region. */
    uint8_t region_id;
};

/*!
 * \brief Information used to configure memory regions in IO Block.
 */
struct interface_io_block_setup_mmap {
    /*! Base register address for the IO Block. */
    uint64_t io_block_address;
    /*! Regions to be mapped in the IO Block. */
    struct interface_io_block_carveout_info *carveout_info;
    /*! Number of regions. */
    size_t region_count;
    /*! Source node ID. Regions are programmed in source node. */
    uint16_t source_id;
};

/*!
 * \brief Interface to manage mappings in the IO Block.
 */
struct interface_io_block_memmap_api {
    /*!
     * \brief Program the given memory carveout in the IO Block.
     *
     * \param mmap Memory map information of the regions to be mapped in the IO
     *             Block.
     *
     * \retval ::FWK_SUCCESS on successfully mapping the region.
     * \retval ::FWK_E_DATA if mapping region is invalid.
     * \return One of the standard framework status codes.
     */
    int (*map_region)(struct interface_io_block_setup_mmap *mmap);
    /*!
     * \brief Remove the carveout mapping from the IO block.
     *
     * \param mmap Memory map information of the regions mapped in the IO Block.
     *
     * \retval ::FWK_SUCCESS on successfully mapping the region.
     * \retval ::FWK_E_DATA if mapping region is invalid.
     * \return One of the standard framework status codes.
     */
    int (*unmap_region)(struct interface_io_block_setup_mmap *mmap);
};

#endif /* INTERFACE_IO_BLOCK */
