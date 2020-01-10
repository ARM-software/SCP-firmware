/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_bootloader.h>
#include <mod_sds.h>

#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Offset within the SDS structure where the valid flag is located. */
#define BOOTLOADER_STRUCT_VALID_POS           0
/* Offset within the SDS structure where the image offset is located. */
#define BOOTLOADER_STRUCT_IMAGE_OFFSET_POS    4
/* Offset within the SDS structure where the image size is located. */
#define BOOTLOADER_STRUCT_IMAGE_SIZE_POS      8

#define IMAGE_FLAGS_VALID_MASK 0x1

/* Module context */
struct bootloader_ctx {
    const struct mod_bootloader_config *module_config;
    const struct mod_sds_api *sds_api;
};

static struct bootloader_ctx module_ctx;

/*
 * Module API
 */

static int load_image(void)
{
    extern noreturn void mod_bootloader_boot(
        uintptr_t destination,
        void *source,
        size_t size,
        volatile uint32_t *vtor);

    int status;
    void *image_base;
    uint32_t image_flags;
    uint32_t image_offset;
    uint32_t image_size;

    if (module_ctx.module_config->source_base == 0)
        return FWK_E_PARAM;
    if (module_ctx.module_config->destination_base == 0)
        return FWK_E_PARAM;
    if (module_ctx.module_config->source_size == 0)
        return FWK_E_PARAM;
    if (module_ctx.module_config->destination_size == 0)
        return FWK_E_PARAM;
    if (module_ctx.module_config->sds_struct_id == 0)
        return FWK_E_PARAM;

    /*
     * Wait until Trusted Firmware writes the image metadata and sets the
     * data valid flag.
     */
    while (true) {
        status = module_ctx.sds_api->struct_read(
            module_ctx.module_config->sds_struct_id,
            BOOTLOADER_STRUCT_VALID_POS, &image_flags, sizeof(image_flags));

        if (status != FWK_SUCCESS)
            return status;
        if (image_flags & IMAGE_FLAGS_VALID_MASK)
            break;
    }

    /* The image metadata from Trusted Firmware can now be read and validated */
    status = module_ctx.sds_api->struct_read(
        module_ctx.module_config->sds_struct_id,
        BOOTLOADER_STRUCT_IMAGE_OFFSET_POS, &image_offset,
        sizeof(image_offset));

    if (status != FWK_SUCCESS)
        return status;
    status = module_ctx.sds_api->struct_read(
        module_ctx.module_config->sds_struct_id,
        BOOTLOADER_STRUCT_IMAGE_SIZE_POS, &image_size, sizeof(image_size));

    if (status != FWK_SUCCESS)
        return status;

    if (image_size == 0)
        return FWK_E_SIZE;
    if ((image_offset % 4) != 0)
        return FWK_E_ALIGN;
    if (image_offset > module_ctx.module_config->source_size)
        return FWK_E_SIZE;

    /* Read the image header now that its base address is known */
    image_base = (void *)((uint8_t *)module_ctx.module_config->source_base +
                          image_offset);

    fwk_interrupt_global_disable(); /* We are relocating the vector table */

    FWK_LOG_FLUSH();

    mod_bootloader_boot(
        module_ctx.module_config->destination_base,
        image_base,
        image_size,
        &SCB->VTOR);
}

static const struct mod_bootloader_api bootloader_api = {
    .load_image = load_image,
};

/*
 * Framework handlers
 */

static int bootloader_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    /* Store a pointer to the module config within the module context */
    module_ctx.module_config = data;

    return FWK_SUCCESS;
}

static int bootloader_bind(fwk_id_t id, unsigned int call_number)
{
    int status;

    /* Only the first round of binding is used (round number is zero-indexed) */
    if (call_number == 1)
        return FWK_SUCCESS;

    if (fwk_module_is_valid_element_id(id))
        /* No element-level binding required */
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SDS),
                             FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
                             &module_ctx.sds_api);

    return status;
}

static int bootloader_process_bind_request(fwk_id_t requester_id, fwk_id_t id,
    fwk_id_t api_id, const void **api)
{
    if (api == NULL)
        return FWK_E_PARAM;

    if (!fwk_module_is_valid_module_id(id))
        return FWK_E_PARAM;

    *api = &bootloader_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_bootloader = {
    .name = "Bootloader",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .event_count = 0,
    .init = bootloader_init,
    .bind = bootloader_bind,
    .process_bind_request = bootloader_process_bind_request,
};
