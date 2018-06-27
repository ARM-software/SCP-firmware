/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_errno.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_sds.h>

/* Arbitrary, 16 bit value that indicates a valid SDS Memory Region */
#define REGION_SIGNATURE 0xAA7A
/* The minor version of the SDS schema supported by this implementation */
#define SUPPORTED_VERSION_MINOR 0x0
/* The major version of the SDS schema supported by this implementation */
#define SUPPORTED_VERSION_MAJOR 0x1

/* Minimum required byte alignment for fields within structures */
#define MIN_FIELD_ALIGNMENT 4
/* Minimum required byte alignment for structures within the region */
#define MIN_STRUCT_ALIGNMENT 8
/* Minimum structure size in bytes */
#define MIN_STRUCT_SIZE 4

/* Header containing Shared Data Structure metadata */
struct structure_header {
    /*
     * Compound identifier value consisting of a linear structure ID, a major
     * version, and a minor version. Together these uniquely identify the
     * structure.
     */
    uint32_t id;

    /*
     * Flag indicating whether the structure's content is valid. Always false
     * initially, the flag is set to true when the structure is finalized.
     */
    bool valid : 1;

    /*
     * Size, in bytes, of the structure. The size of the header is not included,
     * only the structure content. If padding was used to maintain alignment
     * during the structure's creation then the additional space used for the
     * padding will be reflected in this value.
     */
    uint32_t size : 23;

    /* Reserved */
    uint32_t reserved : 8;
};

/* Region Descriptor describing the SDS Memory Region */
struct region_descriptor {
    /*
     * Field used to determine the presence, or absence, of an SDS Memory Region
     * and Region Descriptor. When the region is initialized the signature field
     * is given a fixed value according to the REGION_SIGNATURE definition.
     */
    uint16_t signature;

    /*
     * The total number of structures, finalized or otherwise, within the SDS
     * Memory Region.
     */
    uint16_t structure_count : 8;

    /* The minor version component of the implemented SDS specification */
    uint16_t version_minor : 4;

    /* The major version component of the implemented SDS specification */
    uint16_t version_major : 4;

    /*
     * The total size of the SDS Memory Region, in bytes. The value includes the
     * size of the Region Descriptor because the descriptor is located within
     * the region that it describes. This is in contrast to structure headers,
     * which are considered to be prepended to the memory allocated for the
     * structure content.
     */
    uint32_t region_size;
};

/* Module context structure*/
struct sds_ctx {
    /* Pointer to the module configuration. */
    const struct mod_sds_config *module_config;

    /* Pointer to the base of the SDS Memory Region. */
    uint8_t *mem_base;

    /* Size of the SDS Memory Region in bytes. */
    unsigned int mem_size;

    /* Remaining, unused memory in the SDS Memory Region, in bytes. */
    unsigned int mem_free;

    /* Pointer to the next free memory address in the SDS Memory Region. */
    uint8_t *mem_next_free;

    /* Pointer to the Region Descriptor structure at the memory region base. */
    struct region_descriptor *region_desc;
};

/* Module context */
static struct sds_ctx ctx;

/*
 * Static functions
 */

/*
 * Perform some tests to determine whether any of the fields within a Structure
 * Header contain obviously invalid data.
 */
static bool header_is_valid(struct structure_header *header)
{
    if (header->id == 0)
        return false; /* Zero is not a valid identifier */

    if ((header->id >> MOD_SDS_ID_VERSION_MAJOR_POS) == 0)
        return false; /* 0 is not a valid major version */

    if (header->size < FWK_ALIGN_NEXT(MIN_STRUCT_SIZE, MIN_STRUCT_ALIGNMENT))
        return false; /* Padded structure size is less than the minimum */

    if (header->size > (ctx.mem_size - sizeof(struct structure_header)))
        /* Structure exceeds the capacity of the SDS Memory Region */
        return false;

    if ((header->size % MIN_STRUCT_ALIGNMENT) != 0)
        return false; /* Structure does not meet alignment requirements */

    return true;
}

static bool validate_structure_access(uint32_t structure_size, uint32_t offset,
                                      size_t access_size)
{
    if ((offset >= structure_size) || (access_size > structure_size))
        return FWK_E_PARAM;

    if ((structure_size - offset) < access_size)
        return FWK_E_PARAM;

    return FWK_SUCCESS;
}

/*
 * Search the SDS Memory Region for a given structure ID and return a
 * copy of the Structure Header that holds its information. Optionally, a
 * uint8_t pointer pointer may be provided to retrieve a pointer to the base
 * address of the structure content.
 *
 * The validity of the header is checked before its contents are returned. These
 * tests check that the structure size is sane, and that the entire contents of
 * the structure are contained within the SDS Memory Region.
 *
 * If the address of the structure header in the SDS Memory Region is needed
 * (as opposed to a copy of the header contents) then this can be calculated by
 * subtracting the size of the header from the structure base address obtained
 * from this function.
 *
 * If a structure with the given ID is not present then FWK_E_PARAM is returned.
 */
static int get_structure_info(uint32_t structure_id,
                              struct structure_header *header,
                              uint8_t **structure_base)
{
   unsigned int struct_idx;
   struct structure_header current_header;
   uint32_t offset;

   offset = sizeof(struct region_descriptor);

   /* Iterate over structure headers to find one with a matching ID */
   for (struct_idx = 0; struct_idx < ctx.region_desc->structure_count;
       struct_idx++) {
       current_header = *(struct structure_header *)(ctx.mem_base + offset);
       if (!header_is_valid(&current_header))
           return FWK_E_DATA;

       if (current_header.id == structure_id) {
           if (structure_base != NULL)
               *structure_base = ((uint8_t *)(ctx.mem_base + offset)) +
                   sizeof(struct structure_header);

           *header = current_header;
           return FWK_SUCCESS;
       }

       offset += current_header.size;
       offset += sizeof(struct structure_header);
       if (offset >= ctx.mem_size)
           return FWK_E_RANGE;
   }

   return FWK_E_PARAM;
}

/*
 * Search the SDS Memory Region to determine if a structure with the given ID
 * is present.
 */
static bool structure_exists(uint32_t structure_id)
{
    int status;
    struct structure_header header;

    status = get_structure_info(structure_id, &header, NULL);
    return status == FWK_SUCCESS;
}

static int struct_alloc(uint32_t structure_id, size_t size)
{
    struct structure_header *header = NULL;
    unsigned int padded_size;
    unsigned int status;

    if (size < MIN_STRUCT_SIZE)
        return FWK_E_PARAM;

    padded_size = FWK_ALIGN_NEXT(size, MIN_STRUCT_ALIGNMENT);

    if (padded_size + sizeof(*header) > ctx.mem_free) {
        status = FWK_E_NOMEM;
        goto exit;
    }

    if (structure_exists(structure_id)) {
        status = FWK_E_RANGE;
        goto exit;
    }

    /* Create the Structure Header */
    header = (struct structure_header *)ctx.mem_next_free;
    header->id = structure_id;
    header->size = padded_size;
    header->valid = false;
    ctx.mem_next_free += sizeof(*header);
    ctx.mem_free -= sizeof(*header);

    /* Zero the memory reserved for the structure, avoiding the header */
    memset(ctx.mem_next_free, 0, padded_size);
    ctx.mem_next_free += padded_size;
    ctx.mem_free -= padded_size;

    /* Increment the structure count within the region descriptor */
    ctx.region_desc->structure_count++;

    status = FWK_SUCCESS;

exit:
    return status;
}

/*
 * Read an SDS Memory Region to determine its properties (through the Region
 * Descriptor) such as its size limit, its schema version and the current number
 * of structures stored within the region. Any Structure Headers that are
 * found within the region will be checked for validity.
 *
 * Once the existing contents have been evaluated the memory region will be
 * resized if the RAM Firmware has defined a different region size than the ROM
 * Firmware. The resizing process only takes place if the new region size is
 * not the same as the existing size. When resizing, the Region Descriptor's
 * 'region size' field is modified but the contents of the region are not
 * modified in any way.
 *
 * Finally, the total size of the structures and headers that are present
 * is subtracted from the size of the memory region to determine the amount
 * of free memory that remains.
 */
static int reinitialize_memory_region(void)
{
    unsigned int struct_idx;
    uint32_t mem_used;
    struct structure_header header;

    if (ctx.region_desc->signature != REGION_SIGNATURE)
        return FWK_E_DATA;

    if (ctx.region_desc->version_major != SUPPORTED_VERSION_MAJOR)
        return FWK_E_DATA;

    mem_used = sizeof(struct region_descriptor);

    for (struct_idx = 0; struct_idx < ctx.region_desc->structure_count;
        struct_idx++) {
        header = *(struct structure_header *)(ctx.mem_base + mem_used);

        if (!header_is_valid(&header))
            return FWK_E_DATA; /* Unexpected invalid header */

        mem_used += header.size;
        mem_used += sizeof(struct structure_header);
        if (mem_used > ctx.region_desc->region_size)
            return FWK_E_SIZE;
    }

    if (ctx.mem_size < mem_used)
        return FWK_E_SIZE;

    /*
     * Update the SDS Memory Region size (if different between ROM and RAM) and
     * the amount of free memory left in the SDS Memory Region.
     */
    if (ctx.region_desc->region_size != ctx.mem_size)
        ctx.region_desc->region_size = ctx.mem_size;

    ctx.mem_free = ctx.mem_size - mem_used;

    /*
     * Initialize the pointer that holds the next free memory location for a
     * Shared Data Structure. This follows the last Shared Data Structure, or
     * the Region Descriptor if there are no existing structures.
     */
    ctx.mem_next_free = ctx.mem_base + mem_used;

    return FWK_SUCCESS;
}

/*
 * Initialize an empty SDS Memory Region so that it is ready for use.
 */
static int create_memory_region(void)
{
    unsigned int min_storage;

    /*
     * Check that the SDS Memory Region is large enough to support both the
     * Region Descriptor and a single Shared Data Structure of the minimum
     * permitted size.
     */
    min_storage = sizeof(struct region_descriptor) +
        FWK_ALIGN_NEXT(MIN_STRUCT_SIZE, MIN_STRUCT_ALIGNMENT);
    if (ctx.mem_size < min_storage)
        return FWK_E_NOMEM;

    ctx.mem_free = ctx.mem_size - sizeof(struct region_descriptor);

    /*
     * Update the Region Descriptor
     */
    ctx.region_desc->signature = REGION_SIGNATURE;
    ctx.region_desc->structure_count = 0;
    ctx.region_desc->version_major = SUPPORTED_VERSION_MAJOR;
    ctx.region_desc->version_minor = SUPPORTED_VERSION_MINOR;
    ctx.region_desc->region_size = ctx.mem_size;

    /*
     * Initialize the pointer that holds the next free memory location for
     * placing a Structure Header. Initially this follows the Region Descriptor
     * directly.
     */
    ctx.mem_next_free = ctx.mem_base + sizeof(struct region_descriptor);
    assert(((uintptr_t)ctx.mem_next_free %
        MIN_STRUCT_ALIGNMENT) == 0);

    return FWK_SUCCESS;
}

static int struct_write(uint32_t structure_id, unsigned int offset,
                        const void *data, size_t size)
{
    int status;
    uint8_t *structure_base;
    struct structure_header header;

    assert(data != NULL);
    assert(size != 0);

    /* Look up the Structure Header by its identifier */
    status = get_structure_info(structure_id, &header, &structure_base);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Perform sanity checks on the field location and data size before invoking
     * memcpy.
     */
    status = validate_structure_access(header.size, offset, size);
    if (status != FWK_SUCCESS)
        return status;

    memcpy(structure_base + offset, data, size);
    return FWK_SUCCESS;
}

static int struct_finalize(uint32_t structure_id)
{
    int status;
    uint8_t *structure_base;
    struct structure_header header;
    struct structure_header *header_mem;

    /* Check that the structure being finalized exists */
    status = get_structure_info(structure_id, &header, &structure_base);
    if (status != FWK_SUCCESS)
        return status;

    /* Update the valid flag of the header within the SDS Memory Region */
    header_mem = (struct structure_header *)(structure_base - sizeof(header));
    header_mem->valid = true;

    return FWK_SUCCESS;
}

static int struct_init(const struct mod_sds_structure_desc *struct_desc)
{
    int status = FWK_SUCCESS;

    /* If the structure does not already exist, allocate it. */
    if (!structure_exists(struct_desc->id)) {
        status = struct_alloc(struct_desc->id, struct_desc->size);
        if (status != FWK_SUCCESS)
            return status;
    }

    if (struct_desc->payload != NULL) {
        status = struct_write(struct_desc->id, 0, struct_desc->payload,
                              struct_desc->size);
        if (status != FWK_SUCCESS)
            return status;
    }

    /* Finalize the structure immediately if required */
    if (struct_desc->finalize)
        status = struct_finalize(struct_desc->id);

    return status;
}

/*
 * Module API
 */

static int sds_struct_write(uint32_t structure_id, unsigned int offset,
                            const void *data, size_t size)
{
    int status;

    status = fwk_module_check_call(fwk_module_id_sds);
    if (status != FWK_SUCCESS)
        return status;

    if (data == NULL)
        return FWK_E_PARAM;

    if (size == 0)
        return FWK_E_PARAM;

    return struct_write(structure_id, offset, data, size);
}

static int sds_struct_read(uint32_t structure_id, unsigned int offset,
                           void *data, size_t size)
{
    int status;
    uint8_t *structure_base;
    struct structure_header header;

    status = fwk_module_check_call(fwk_module_id_sds);
    if (status != FWK_SUCCESS)
        return status;

    if (data == NULL)
        return FWK_E_PARAM;

    if (size == 0)
        return FWK_E_PARAM;

    /* Check if a structure with this ID exists */
    status = get_structure_info(structure_id, &header, &structure_base);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Perform sanity checks on the field location and data size before invoking
     * memcpy.
     */
     status = validate_structure_access(header.size, offset, size);
     if (status != FWK_SUCCESS)
        return status;

    memcpy(data, structure_base + offset, size);
    return FWK_SUCCESS;
}

static int sds_struct_finalize(uint32_t structure_id)
{
    int status;

    status = fwk_module_check_call(fwk_module_id_sds);
    if (status != FWK_SUCCESS)
        return status;

    return struct_finalize(structure_id);
}

static const struct mod_sds_api module_api = {
    .struct_write = sds_struct_write,
    .struct_read = sds_struct_read,
    .struct_finalize = sds_struct_finalize,
};

/*
 * Framework handlers
 */

static int sds_init(fwk_id_t module_id, unsigned int element_count,
                    const void *data)
{
    if (data == NULL)
        return FWK_E_PARAM;

    ctx.module_config = data;

    assert((MIN_STRUCT_ALIGNMENT % MIN_FIELD_ALIGNMENT) == 0);

    if (ctx.module_config->region_base_address == 0)
        return FWK_E_PARAM;
    if (((uintptr_t)ctx.module_config->region_base_address %
        MIN_STRUCT_ALIGNMENT) > 0)
        return FWK_E_PARAM;

    ctx.mem_base = (uint8_t *)ctx.module_config->region_base_address;
    ctx.mem_size = ctx.module_config->region_size;
    ctx.region_desc = (struct region_descriptor *)ctx.mem_base;

    return FWK_SUCCESS;
}

static int sds_element_init(fwk_id_t element_id, unsigned int unused,
                            const void *data)
{
    return FWK_SUCCESS;
}

static int sds_process_bind_request(fwk_id_t requester_id, fwk_id_t id,
                                    fwk_id_t api_id, const void **api)
{
    if (!fwk_module_is_valid_module_id(requester_id))
        return FWK_E_ACCESS;

    *api = &module_api;
    return FWK_SUCCESS;
}

static int sds_start(fwk_id_t id)
{
    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    /* Register the module for clock state notifications */
    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        ctx.module_config->clock_id,
        id);
}

static int sds_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct clock_notification_params *params;
    int element_idx;
    int element_count;
    const struct mod_sds_structure_desc *struct_desc;

    assert(fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    params = (struct clock_notification_params *)event->params;
    if (params->new_state != MOD_CLOCK_STATE_RUNNING)
        return FWK_SUCCESS;

    /* Either reinitialize the memory region, or create it for the first time */
    status = reinitialize_memory_region();
    if (status != FWK_SUCCESS) {
        status = create_memory_region();
        if (status != FWK_SUCCESS)
            return status;
    }

    element_count = fwk_module_get_element_count(event->target_id);
    for (element_idx = 0; element_idx < element_count; ++element_idx) {
        struct_desc = fwk_module_get_data(
            fwk_id_build_element_id(event->target_id, element_idx));

        status = struct_init(struct_desc);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_sds = {
    .name = "Shared Data Storage",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .event_count = 0,
    .init = sds_init,
    .element_init = sds_element_init,
    .process_bind_request = sds_process_bind_request,
    .start = sds_start,
    .process_notification = sds_process_notification
};
