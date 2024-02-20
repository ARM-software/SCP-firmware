/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      PPU_v1 unit test support.
 */

#include <ppu_v1.h>

/* For the alarm API*/
int start_alarm_api(
    fwk_id_t alarm_id,
    unsigned int milliseconds,
    enum mod_timer_alarm_type type,
    void (*callback)(uintptr_t param),
    uintptr_t param);
int stop_alarm_api(fwk_id_t alarm_id);

/* From ppu_v1.h*/
void ppu_v1_init(struct ppu_v1_reg *ppu);

int ppu_v1_set_power_mode(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode ppu_mode,
    struct ppu_v1_timer_ctx *timer_ctx);

int ppu_v1_request_power_mode(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode ppu_mode);

int ppu_v1_request_operating_mode(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_opmode op_mode);

void ppu_v1_opmode_dynamic_enable(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_opmode min_dyn_mode);

void ppu_v1_dynamic_enable(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode min_dyn_state);

void ppu_v1_lock_off_enable(struct ppu_v1_reg *ppu);

void ppu_v1_lock_off_disable(struct ppu_v1_reg *ppu);

enum ppu_v1_mode ppu_v1_get_power_mode(struct ppu_v1_reg *ppu);

enum ppu_v1_mode ppu_v1_get_programmed_power_mode(struct ppu_v1_reg *ppu);

enum ppu_v1_opmode ppu_v1_get_operating_mode(struct ppu_v1_reg *ppu);

enum ppu_v1_opmode ppu_v1_get_programmed_operating_mode(struct ppu_v1_reg *ppu);

bool ppu_v1_is_dynamic_enabled(struct ppu_v1_reg *ppu);

bool ppu_v1_is_locked(struct ppu_v1_reg *ppu);

bool ppu_v1_is_power_devactive_high(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode ppu_mode);

bool ppu_v1_is_op_devactive_high(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_op_devactive op_devactive);

void ppu_v1_off_unlock(struct ppu_v1_reg *ppu);

void ppu_v1_disable_devactive(struct ppu_v1_reg *ppu);

void ppu_v1_disable_handshake(struct ppu_v1_reg *ppu);

void ppu_v1_interrupt_mask(struct ppu_v1_reg *ppu, unsigned int mask);

void ppu_v1_additional_interrupt_mask(
    struct ppu_v1_reg *ppu,
    unsigned int mask);

void ppu_v1_interrupt_unmask(struct ppu_v1_reg *ppu, unsigned int mask);

void ppu_v1_additional_interrupt_unmask(
    struct ppu_v1_reg *ppu,
    unsigned int mask);

bool ppu_v1_is_additional_interrupt_pending(
    struct ppu_v1_reg *ppu,
    unsigned int mask);

void ppu_v1_ack_interrupt(struct ppu_v1_reg *ppu, unsigned int mask);

void ppu_v1_ack_additional_interrupt(struct ppu_v1_reg *ppu, unsigned int mask);

void ppu_v1_set_input_edge_sensitivity(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode ppu_mode,
    enum ppu_v1_edge_sensitivity edge_sensitivity);

enum ppu_v1_edge_sensitivity ppu_v1_get_input_edge_sensitivity(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode ppu_mode);

void ppu_v1_ack_power_active_edge_interrupt(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode ppu_mode);

bool ppu_v1_is_power_active_edge_interrupt(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_mode ppu_mode);

void ppu_v1_set_op_active_edge_sensitivity(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_op_devactive op_devactive,
    enum ppu_v1_edge_sensitivity edge_sensitivity);

enum ppu_v1_edge_sensitivity ppu_v1_get_op_active_edge_sensitivity(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_op_devactive op_devactive);

void ppu_v1_ack_op_active_edge_interrupt(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_op_devactive op_devactive);

bool ppu_v1_is_op_active_edge_interrupt(
    struct ppu_v1_reg *ppu,
    enum ppu_v1_op_devactive op_devactive);

bool ppu_v1_is_dyn_policy_min_interrupt(struct ppu_v1_reg *ppu);

unsigned int ppu_v1_get_num_opmode(struct ppu_v1_reg *ppu);

unsigned int ppu_v1_get_arch_id(struct ppu_v1_reg *ppu);
