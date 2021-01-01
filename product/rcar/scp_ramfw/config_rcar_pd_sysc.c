/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_rcar_pd_sysc.h>
#include <config_power_domain.h>

#include <mod_rcar_pd_sysc.h>

#include <fwk_element.h>
#include <fwk_module.h>

static struct fwk_element rcar_pd_sysc_element_table[] = {
    [RCAR_PD_SYSC_ELEMENT_IDX_A3IR] =
        {
            .name = "a3ir",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x180,
                .chan_bit = 0,
                .isr_bit = R8A7795_PD_A3IR,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_3DGE] =
        {
            .name = "3dg-e",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x100,
                .chan_bit = 4,
                .isr_bit = R8A7795_PD_3DG_E,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_3DGD] =
        {
            .name = "3dg-d",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x100,
                .chan_bit = 3,
                .isr_bit = R8A7795_PD_3DG_D,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_3DGC] =
        {
            .name = "3dg-c",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x100,
                .chan_bit = 2,
                .isr_bit = R8A7795_PD_3DG_C,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_3DGB] =
        {
            .name = "3dg-b",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x100,
                .chan_bit = 1,
                .isr_bit = R8A7795_PD_3DG_B,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_3DGA] =
        {
            .name = "3dg-a",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x100,
                .chan_bit = 0,
                .isr_bit = R8A7795_PD_3DG_A,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_A2VC1] =
        {
            .name = "a2vc1",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x3c0,
                .chan_bit = 1,
                .isr_bit = R8A7795_PD_A2VC1,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_A3VC] =
        {
            .name = "a3vc",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x380,
                .chan_bit = 0,
                .isr_bit = R8A7795_PD_A3VC,
                .always_on = true,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_CR7] =
        {
            .name = "cr7",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x240,
                .chan_bit = 0,
                .isr_bit = R8A7795_PD_CR7,
                .always_on = false,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_A3VP] =
        {
            .name = "a3vp",
            .data = &((struct mod_rcar_pd_sysc_config){
                .pd_type = RCAR_PD_TYPE_DEVICE,
                .chan_offs = 0x340,
                .chan_bit = 0,
                .isr_bit = R8A7795_PD_A3VP,
                .always_on = true,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_ALWAYS_ON] =
        {
            .name = "always_on",
            .data = &((struct mod_rcar_pd_sysc_config) {
                .pd_type = RCAR_PD_TYPE_ALWAYS_ON,
                .always_on = true,
            }),
        },
    [RCAR_PD_SYSC_ELEMENT_IDX_COUNT] = { 0 }, /* Termination entry */
};

static const struct fwk_element *rcar_pd_sysc_get_element_table(fwk_id_t mod)
{
    return rcar_pd_sysc_element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_rcar_pd_sysc = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_pd_sysc_get_element_table),
};
