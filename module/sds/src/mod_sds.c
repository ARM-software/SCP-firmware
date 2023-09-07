/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_sds.h>

#ifdef BUILD_HAS_MOD_CLOCK
#    include <mod_clock.h>
#endif

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

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
/* Minimum structure size (including padding) in bytes */
#define MIN_ALIGNED_STRUCT_SIZE \
    FWK_ALIGN_NEXT(MIN_STRUCT_SIZE, MIN_STRUCT_ALIGNMENT)
/* Minimum region size in bytes */
#define MIN_REGION_SIZE (sizeof(struct region_descriptor) + \
                         MIN_ALIGNED_STRUCT_SIZE)
/* Module name to use in log messages */
#define MODULE_NAME "[SDS]"

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
    struct {
        /* Remaining, unused memory in the SDS Memory Region, in bytes. */
        size_t free_mem_size;

        /* Pointer to the next free memory address in the SDS Memory Region. */
        volatile char *free_mem_base;
    } *regions;

    /*
     * Number of notifications subscribed and to wait for before the SDS data
     * is published.
     */
    unsigned int wait_on_notifications;
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
static bool header_is_valid(const volatile struct region_descriptor *region,
                            const volatile struct structure_header *header)
{
    const volatile char *region_tail, *struct_tail;

    if (header->id == 0) {
        return false; /* Zero is not a valid identifier */
    }

    if ((header->id >> MOD_SDS_ID_VERSION_MAJOR_POS) == 0) {
        return false; /* 0 is not a valid major version */
    }

    if (header->size < MIN_ALIGNED_STRUCT_SIZE) {
        return false; /* Padded structure size is less than the minimum */
    }

    region_tail = (const volatile char *)region + region->region_size;
    struct_tail = (const volatile char *)header
        + sizeof(struct structure_header)
        + header->size;
    if (struct_tail > region_tail) {
        /* Structure exceeds the capacity of the SDS Memory Region */
        return false;
    }

    if ((header->size % MIN_STRUCT_ALIGNMENT) != 0) {
        return false; /* Structure does not meet alignment requirements */
    }

    return true;
}

static int validate_structure_access(
    uint32_t structure_size,
    uint32_t offset,
    size_t access_size)
{
    if ((offset >= structure_size) || (access_size > structure_size)) {
        return FWK_E_PARAM;
    }

    if ((structure_size - offset) < access_size) {
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

/*
 * Search the SDS Memory Region(s) for a given structure ID and return a
 * copy of the Structure Header that holds its information. Optionally, a
 * char pointer pointer may be provided to retrieve a pointer to the base
 * address of the structure content.
 *
 * The validity of the header is checked before its contents are returned. These
 * tests check that the structure size is sane, and that the entire contents of
 * the structure are contained within its SDS Memory Region.
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
                              volatile char **structure_base)
{
   volatile struct structure_header *current_header;
   size_t offset, region_size, struct_count, region_idx, struct_idx;
   const struct mod_sds_config *config;
   volatile struct region_descriptor *region_desc;
   volatile char *region_base;

    config = fwk_module_get_data(fwk_module_id_sds);
    fwk_assert(config != NULL);

   for (region_idx = 0; region_idx < config->region_count; region_idx++) {
       region_base = (volatile char *)config->regions[region_idx].base;
       region_desc = (volatile struct region_descriptor *)region_base;
       region_size = region_desc->region_size;
       struct_count = region_desc->structure_count;

       offset = sizeof(struct region_descriptor);
       /* Iterate over structure headers to find one with a matching ID */
       for (struct_idx = 0; struct_idx < struct_count; struct_idx++) {
           current_header = (volatile struct structure_header *)(
                region_base + offset);
           if (!header_is_valid(region_desc, current_header)) {
               return FWK_E_DATA;
           }

           if (current_header->id == structure_id) {
               if (structure_base != NULL) {
                   *structure_base = ((volatile char *)current_header
                                      + sizeof(struct structure_header));
               }

               *header = *current_header;
               return FWK_SUCCESS;
           }

           offset += current_header->size;
           offset += sizeof(struct structure_header);
           if (offset >= region_size) {
               return FWK_E_RANGE;
           }
       }
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

static int struct_alloc(const struct mod_sds_structure_desc *struct_desc)
{
    const struct mod_sds_config *config;
    volatile struct structure_header *header = NULL;
    volatile struct region_descriptor *region_desc;
    unsigned int padded_size;
    int status = FWK_SUCCESS;
    size_t *free_mem_size;
    volatile char **free_mem_base;
    const size_t region_idx = struct_desc->region_id;

    if (struct_desc->size < MIN_STRUCT_SIZE) {
        status = FWK_E_PARAM;
        goto exit;
    }

    config = fwk_module_get_data(fwk_module_id_sds);
    if (config == NULL) {
        return FWK_E_PARAM;
    }
    fwk_assert(region_idx < config->region_count);

    padded_size =
        (unsigned int)FWK_ALIGN_NEXT(struct_desc->size, MIN_STRUCT_ALIGNMENT);

    region_desc = (volatile struct region_descriptor *)(
        config->regions[region_idx].base);
    free_mem_base = &(ctx.regions[region_idx].free_mem_base);
    free_mem_size = &(ctx.regions[region_idx].free_mem_size);

    if ((padded_size + sizeof(*header)) > *free_mem_size) {
        status = FWK_E_NOMEM;
        goto exit;
    }

    if (structure_exists(struct_desc->id)) {
        status = FWK_E_RANGE;
        goto exit;
    }

    /* Create the Structure Header */
    header = (volatile struct structure_header *)(*free_mem_base);
    header->id = struct_desc->id;
    header->size = padded_size;
    header->valid = false;
    *free_mem_base += sizeof(*header);
    *free_mem_size -= sizeof(*header);

    /* Zero the memory reserved for the structure, avoiding the header */
    for (unsigned int i = 0; i < padded_size; i++) {
        (*free_mem_base)[i] = '\0';
    }
    *free_mem_base += padded_size;
    *free_mem_size -= padded_size;

    /* Increment the structure count within the region descriptor */
    region_desc->structure_count++;

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
static int reinitialize_memory_region(
    const struct mod_sds_region_desc *region_config, unsigned int region_idx)
{
    unsigned int struct_idx;
    uint32_t mem_used;
    volatile struct structure_header *header;
    volatile struct region_descriptor *region_desc;

    region_desc = (volatile struct region_descriptor *)(region_config->base);
    if (region_desc->signature != REGION_SIGNATURE) {
        return FWK_E_DATA;
    }

    if (region_desc->version_major != SUPPORTED_VERSION_MAJOR) {
        return FWK_E_DATA;
    }

    mem_used = (uint32_t)sizeof(struct region_descriptor);
    for (struct_idx = 0; struct_idx < region_desc->structure_count;
        struct_idx++) {
        header = (volatile struct structure_header *)(
            (volatile char *)region_config->base + mem_used);

        if (!header_is_valid(region_desc, header)) {
            return FWK_E_DATA; /* Unexpected invalid header */
        }

        mem_used += header->size;
        mem_used += (uint32_t)sizeof(struct structure_header);
        if (mem_used > region_desc->region_size) {
            return FWK_E_SIZE;
        }
    }

    if (mem_used > region_config->size) {
        return FWK_E_SIZE;
    }

    /*
     * The SDS memory region size might differ between ROM and RAM images. In
     * that case, as ctx has been loaded by the RAM image while the SDS region
     * was setup by the ROM image:
     *   - region_config->region_size is the new size expected by the RAM
     *     image;
     *   - region_desc->region_size is the old size from the ROM image;
     */
    region_desc->region_size = (uint32_t)region_config->size;
    ctx.regions[region_idx].free_mem_size = region_config->size - mem_used;
    ctx.regions[region_idx].free_mem_base =
        (volatile char *)region_config->base + mem_used;

    return FWK_SUCCESS;
}

/*
 * Initialize an empty SDS Memory Region so that it is ready for use.
 */
static int create_memory_region(const struct mod_sds_region_desc* region_config,
                                unsigned int region_idx)
{
    volatile struct region_descriptor *region_desc;

    if (region_config->size < MIN_REGION_SIZE) {
        return FWK_E_NOMEM;
    }

    region_desc = (volatile struct region_descriptor *)region_config->base;
    region_desc->signature = REGION_SIGNATURE;
    region_desc->structure_count = 0;
    region_desc->version_major = SUPPORTED_VERSION_MAJOR;
    region_desc->version_minor = SUPPORTED_VERSION_MINOR;
    region_desc->region_size = (uint32_t)region_config->size;

    ctx.regions[region_idx].free_mem_size = region_config->size
        - sizeof(struct region_descriptor);
    ctx.regions[region_idx].free_mem_base =
        (volatile char *)region_config->base + sizeof(struct region_descriptor);
    fwk_assert(((uintptr_t)ctx.regions[region_idx].free_mem_base %
        MIN_STRUCT_ALIGNMENT) == 0);

    return FWK_SUCCESS;
}

static int struct_write(uint32_t structure_id, unsigned int offset,
                        const void *data, size_t size)
{
    int status;
    volatile char *structure_base;
    struct structure_header header;

    fwk_assert(data != NULL);
    fwk_assert(size != 0);

    /* Look up the Structure Header by its identifier */
    status = get_structure_info(structure_id, &header, &structure_base);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = validate_structure_access(header.size, offset, size);
    if (status != FWK_SUCCESS) {
        return status;
    }

    for (unsigned int i = 0; i < size; i++) {
        structure_base[offset + i] = ((const char*)data)[i];
    }

    return FWK_SUCCESS;
}

static int struct_finalize(uint32_t structure_id)
{
    int status;
    volatile char *structure_base;
    struct structure_header header;
    volatile struct structure_header *header_mem;

    /* Check that the structure being finalized exists */
    status = get_structure_info(structure_id, &header, &structure_base);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Update the valid flag of the header within the SDS Memory Region */
    header_mem = (volatile struct structure_header *)(
        structure_base - sizeof(header));
    header_mem->valid = true;

    return FWK_SUCCESS;
}

static int struct_init(const struct mod_sds_structure_desc *struct_desc)
{
    int status = FWK_SUCCESS;

    /* If the structure does not already exist, allocate it. */
    if (!structure_exists(struct_desc->id)) {
        status = struct_alloc(struct_desc);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    if (struct_desc->payload != NULL) {
        status = struct_write(struct_desc->id, 0, struct_desc->payload,
                              struct_desc->size);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /* Finalize the structure immediately if required */
    if (struct_desc->finalize) {
        status = struct_finalize(struct_desc->id);
    }

    return status;
}

static int init_sds(void)
{
    const struct mod_sds_config *config;
    int status;
    int element_idx;
    int element_count;
    const struct mod_sds_structure_desc *struct_desc;
    unsigned int region_idx;
    const struct mod_sds_region_desc *region_config;
    unsigned int notification_count;
    struct fwk_event notification_event = {
        .id = mod_sds_notification_id_initialized,
        .source_id = fwk_module_id_sds,
    };

    config = fwk_module_get_data(fwk_module_id_sds);

    for (region_idx = 0; region_idx < config->region_count; region_idx++) {
        region_config = &(config->regions[region_idx]);
        /*
         * Either reinitialize the memory region,
         * or create it for the first time
         */
        status = reinitialize_memory_region(region_config, region_idx);
        if (status != FWK_SUCCESS) {
            status = create_memory_region(region_config, region_idx);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }

    element_count = fwk_module_get_element_count(fwk_module_id_sds);
    for (element_idx = 0; element_idx < element_count; ++element_idx) {
        struct_desc = fwk_module_get_data(fwk_id_build_element_id(
            fwk_module_id_sds, (unsigned int)element_idx));

        status = struct_init(struct_desc);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return fwk_notification_notify(&notification_event, &notification_count);
}

/*
 * Module API
 */

static int sds_struct_write(uint32_t structure_id, unsigned int offset,
                            const void *data, size_t size)
{
    if (data == NULL) {
        return FWK_E_PARAM;
    }

    if (size == 0) {
        return FWK_E_PARAM;
    }

    return struct_write(structure_id, offset, data, size);
}

static int sds_struct_read(uint32_t structure_id, unsigned int offset,
                           void *data, size_t size)
{
    int status;
    volatile char *structure_base;
    struct structure_header header;

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    if (size == 0) {
        return FWK_E_PARAM;
    }

    /* Check if a structure with this ID exists */
    status = get_structure_info(structure_id, &header, &structure_base);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = validate_structure_access(header.size, offset, size);
    if (status != FWK_SUCCESS) {
        return status;
    }

    for (unsigned int i = 0; i < size; i++) {
        ((char*)data)[i] = structure_base[offset + i];
    }

    return FWK_SUCCESS;
}

static int sds_struct_finalize(uint32_t structure_id)
{
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
    const struct mod_sds_config *config;
    void *region_base;
    unsigned int region_idx;

    if (data == NULL) {
        return FWK_E_PANIC;
    }

    config = (const struct mod_sds_config *)data;

    fwk_assert((MIN_STRUCT_ALIGNMENT % MIN_FIELD_ALIGNMENT) == 0);

    for (region_idx = 0; region_idx < config->region_count; region_idx++) {
        region_base = config->regions[region_idx].base;
        if (region_base == NULL) {
            return FWK_E_PARAM;
        }
        if (((uintptr_t)region_base % MIN_STRUCT_ALIGNMENT) > 0) {
            return FWK_E_PARAM;
        }
    }

    ctx.regions = fwk_mm_alloc(config->region_count, sizeof(ctx.regions[0]));
    if (ctx.regions == NULL) {
        return FWK_E_NOMEM;
    }

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
    if (!fwk_module_is_valid_module_id(requester_id)) {
        return FWK_E_ACCESS;
    }

    *api = &module_api;
    return FWK_SUCCESS;
}

static int sds_start(fwk_id_t id)
{
    const struct mod_sds_config *config;
    int status = FWK_SUCCESS;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    config = fwk_module_get_data(fwk_module_id_sds);
    ctx.wait_on_notifications = 0;

#ifdef BUILD_HAS_MOD_CLOCK
    if (fwk_optional_id_is_defined(config->clock_id) &&
        !fwk_id_is_equal(config->clock_id, FWK_ID_NONE)) {
        /* Register the module for clock state notifications */
        status = fwk_notification_subscribe(
            mod_clock_notification_id_state_changed, config->clock_id, id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(MODULE_NAME "Failed to subscribe clock notification");
            return status;
        }
        ctx.wait_on_notifications++;
    }
#endif

    if ((fwk_id_type_is_valid(config->platform_notification.source_id)) &&
        (!fwk_id_is_equal(
            config->platform_notification.source_id, FWK_ID_NONE))) {
        status = fwk_notification_subscribe(
            config->platform_notification.notification_id,
            config->platform_notification.source_id,
            id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(MODULE_NAME
                         "Failed to subscribe platform "
                         "notification");
            return status;
        }
        ctx.wait_on_notifications++;
    }

    if (ctx.wait_on_notifications == 0) {
        status = init_sds();
    }

    return status;
}

#ifdef BUILD_HAS_MOD_CLOCK
static int mod_sds_clock_changed(const struct fwk_event *event)
{
    int status;
    const struct clock_notification_params *params =
        (const void *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        status = fwk_notification_unsubscribe(
            event->id, event->source_id, event->target_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(MODULE_NAME
                         "Failed to unsubscribe clock "
                         "notification");
            return status;
        }
        ctx.wait_on_notifications--;
    }

    return FWK_SUCCESS;
}
#endif

static int sds_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    const struct mod_sds_config *config =
        fwk_module_get_data(fwk_module_id_sds);
    int status = FWK_SUCCESS;

    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));
    fwk_assert(ctx.wait_on_notifications != 0);

#ifdef BUILD_HAS_MOD_CLOCK
    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        status = mod_sds_clock_changed(event);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }
#endif

    if (fwk_id_is_equal(
            event->id, config->platform_notification.notification_id)) {
        status = fwk_notification_unsubscribe(
            event->id, event->source_id, event->target_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(MODULE_NAME
                         "Failed to unsubscribe platform "
                         "notification");
            return status;
        }
        ctx.wait_on_notifications--;
    }

    if (ctx.wait_on_notifications == 0) {
        status = init_sds();
    }

    return status;
}

/* Module descriptor */
const struct fwk_module module_sds = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .event_count = 0,
    .notification_count = (unsigned int)MOD_SDS_NOTIFICATION_IDX_COUNT,
    .init = sds_init,
    .element_init = sds_element_init,
    .process_bind_request = sds_process_bind_request,
    .start = sds_start,
    .process_notification = sds_process_notification
};
