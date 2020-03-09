/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/i2c_driver.h>
#include <internal/i2c_reg.h>
#include <internal/i2c_reg_access.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define FILE_GRP_ID DBG_DRV_I2C

#define DEBUG_PrintError(...)

struct I2C_REG_FUNC_TABLE tables[2] = {
    { f_i2c_write_BSR,       f_i2c_write_UNDEFINED, f_i2c_write_BCR,
      f_i2c_write_BC2R,      f_i2c_write_ADR,       f_i2c_write_DAR,
      f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED,
      f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED,
      f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED,
      f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED,
      f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED,
      f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED, f_i2c_write_UNDEFINED,
      f_i2c_write_CCR,       f_i2c_write_CSR,       f_i2c_write_FSR,

      f_i2c_read_BSR,        f_i2c_read_UNDEFINED,  f_i2c_read_BCR,
      f_i2c_read_BC2R,       f_i2c_read_ADR,        f_i2c_read_DAR,
      f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,
      f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,
      f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,
      f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,
      f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,
      f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,  f_i2c_read_UNDEFINED,
      f_i2c_read_CCR,        f_i2c_read_CSR,        f_i2c_read_FSR },

    { f_i2c_sp1_write_BSR,       f_i2c_sp1_write_BS2R,
      f_i2c_sp1_write_BCR,       f_i2c_sp1_write_BC2R,
      f_i2c_sp1_write_ADR,       f_i2c_sp1_write_DAR,
      f_i2c_sp1_write_NFR,       f_i2c_sp1_write_TLWR,
      f_i2c_sp1_write_TLW2R,     f_i2c_sp1_write_THWR,
      f_i2c_sp1_write_THW2R,     f_i2c_sp1_write_TBFR,
      f_i2c_sp1_write_TBF2R,     f_i2c_sp1_write_TRSR,
      f_i2c_sp1_write_TRS2R,     f_i2c_sp1_write_TSHR,
      f_i2c_sp1_write_TSH2R,     f_i2c_sp1_write_TPSR,
      f_i2c_sp1_write_TPS2R,     f_i2c_sp1_write_TLWRH,
      f_i2c_sp1_write_THWRH,     f_i2c_sp1_write_TRSRH,
      f_i2c_sp1_write_TSHRH,     f_i2c_sp1_write_TPSRH,
      f_i2c_sp1_write_UNDEFINED, f_i2c_sp1_write_UNDEFINED,
      f_i2c_sp1_write_UNDEFINED,

      f_i2c_sp1_read_BSR,        f_i2c_sp1_read_BS2R,
      f_i2c_sp1_read_BCR,        f_i2c_sp1_read_BC2R,
      f_i2c_sp1_read_ADR,        f_i2c_sp1_read_DAR,
      f_i2c_sp1_read_NFR,        f_i2c_sp1_read_TLWR,
      f_i2c_sp1_read_TLW2R,      f_i2c_sp1_read_THWR,
      f_i2c_sp1_read_THW2R,      f_i2c_sp1_read_TBFR,
      f_i2c_sp1_read_TBF2R,      f_i2c_sp1_read_TRSR,
      f_i2c_sp1_read_TRS2R,      f_i2c_sp1_read_TSHR,
      f_i2c_sp1_read_TSH2R,      f_i2c_sp1_read_TPSR,
      f_i2c_sp1_read_TPS2R,      f_i2c_sp1_read_TLWRH,
      f_i2c_sp1_read_THWRH,      f_i2c_sp1_read_TRSRH,
      f_i2c_sp1_read_TSHRH,      f_i2c_sp1_read_TPSRH,
      f_i2c_sp1_read_UNDEFINED,  f_i2c_sp1_read_UNDEFINED,
      f_i2c_sp1_read_UNDEFINED }
};

#define SET_BSR(VAL) i2c_reg->set_BSR(packet_info, (VAL))
#define SET_BS2R(VAL) i2c_reg->set_BS2R(packet_info, (VAL))
#define SET_BCR(VAL) i2c_reg->set_BCR(packet_info, (VAL))
#define SET_BC2R(VAL) i2c_reg->set_BC2R(packet_info, (VAL))
#define SET_ADR(VAL) i2c_reg->set_ADR(packet_info, (VAL))
#define SET_DAR(VAL) i2c_reg->set_DAR(packet_info, (VAL))
#define SET_NFR(VAL) i2c_reg->set_NFR(packet_info, (VAL))
#define SET_TLWR(VAL) i2c_reg->set_TLWR(packet_info, (VAL))
#define SET_TLW2R(VAL) i2c_reg->set_TLW2R(packet_info, (VAL))
#define SET_THWR(VAL) i2c_reg->set_THWR(packet_info, (VAL))
#define SET_THW2R(VAL) i2c_reg->set_THW2R(packet_info, (VAL))
#define SET_TBFR(VAL) i2c_reg->set_TBFR(packet_info, (VAL))
#define SET_TBF2R(VAL) i2c_reg->set_TBF2R(packet_info, (VAL))
#define SET_TRSR(VAL) i2c_reg->set_TRSR(packet_info, (VAL))
#define SET_TRS2R(VAL) i2c_reg->set_TRS2R(packet_info, (VAL))
#define SET_TSHR(VAL) i2c_reg->set_TSHR(packet_info, (VAL))
#define SET_TSH2R(VAL) i2c_reg->set_TSH2R(packet_info, (VAL))
#define SET_TPSR(VAL) i2c_reg->set_TPSR(packet_info, (VAL))
#define SET_TPS2R(VAL) i2c_reg->set_TPS2R(packet_info, (VAL))
#define SET_TLWRH(VAL) i2c_reg->set_TLWRH(packet_info, (VAL))
#define SET_THWRH(VAL) i2c_reg->set_THWRH(packet_info, (VAL))
#define SET_TRSRH(VAL) i2c_reg->set_TRSRH(packet_info, (VAL))
#define SET_TSHRH(VAL) i2c_reg->set_TSHRH(packet_info, (VAL))
#define SET_TPSRH(VAL) i2c_reg->set_TPSRH(packet_info, (VAL))
#define SET_CCR(VAL) i2c_reg->set_CCR(packet_info, (VAL))
#define SET_CSR(VAL) i2c_reg->set_CSR(packet_info, (VAL))
#define SET_FSR(VAL) i2c_reg->set_FSR(packet_info, (VAL))

#define GET_BSR() i2c_reg->get_BSR(packet_info)
#define GET_BS2R() i2c_reg->get_BSR(packet_info)
#define GET_BCR() i2c_reg->get_BCR(packet_info)
#define GET_BC2R() i2c_reg->get_BC2R(packet_info)
#define GET_ADR() i2c_reg->get_ADR(packet_info)
#define GET_DAR() i2c_reg->get_DAR(packet_info)
#define GET_NFR() i2c_reg->get_NFR(packet_info)
#define GET_TLWR() i2c_reg->get_TLWR(packet_info)
#define GET_TLW2R() i2c_reg->get_TLW2R(packet_info)
#define GET_THWR() i2c_reg->get_THWR(packet_info)
#define GET_THW2R() i2c_reg->get_THW2R(packet_info)
#define GET_TBFR() i2c_reg->get_TBFR(packet_info)
#define GET_TBF2R() i2c_reg->get_TBF2R(packet_info)
#define GET_TRSR() i2c_reg->get_TRSR(packet_info)
#define GET_TRS2R() i2c_reg->get_TRS2R(packet_info)
#define GET_TSHR() i2c_reg->get_TSHR(packet_info)
#define GET_TSH2R() i2c_reg->get_TSH2R(packet_info)
#define GET_TPSR() i2c_reg->get_TPSR(packet_info)
#define GET_TPS2R() i2c_reg->get_TPS2R(packet_info)
#define GET_TLWRH() i2c_reg->get_TLWRH(packet_info)
#define GET_THWRH() i2c_reg->get_THWRH(packet_info)
#define GET_TRSRH() i2c_reg->get_TRSRH(packet_info)
#define GET_TSHRH() i2c_reg->get_TSHRH(packet_info)
#define GET_TPSRH() i2c_reg->get_TPSRH(packet_info)
#define GET_CCR() i2c_reg->get_CCR(packet_info)
#define GET_CSR() i2c_reg->get_CSR(packet_info)
#define GET_FSR() i2c_reg->get_FSR(packet_info)

static inline bool i2c_fifo_isfull(I2C_ST_FIFO_t *instance)
{
    return (instance->LIMIT == instance->SIZE);
}

static inline bool i2c_fifo_islimit(I2C_ST_FIFO_t *instance)
{
    return (instance->INDEX == instance->LIMIT);
}

static inline bool i2c_fifo_isnextlimit(I2C_ST_FIFO_t *instance)
{
    return ((instance->INDEX + 1) >= instance->LIMIT);
}

static inline void i2c_fifo_initialize(I2C_ST_FIFO_t *instance)
{
    instance->LIMIT = instance->INDEX = 0;
}

static I2C_ST_FIFO_t *i2c_fifo_registry(
    I2C_ST_FIFO_t *instance,
    char *buf,
    bool *attribute,
    int size)
{
    instance->BUFF = buf;
    instance->ATTR = attribute;
    instance->SIZE = size;
    i2c_fifo_initialize(instance);

    return instance;
}

static bool i2c_fifo_push_data(I2C_ST_FIFO_t *instance, char data, bool attr)
{
    bool bFull = i2c_fifo_isfull(instance);

    if (!bFull) {
        if (instance->BUFF)
            instance->BUFF[instance->LIMIT] = data;
        if (instance->ATTR)
            instance->ATTR[instance->LIMIT] = attr;
        ++instance->LIMIT;
    }

    return !bFull;
}

static bool i2c_fifo_get_send_data(
    I2C_ST_FIFO_t *instance,
    char *data,
    bool *attr)
{
    bool limit = i2c_fifo_islimit(instance);

    if (!limit) {
        *data = (instance->BUFF) ? instance->BUFF[instance->INDEX] : 0x00;
        *attr = (instance->ATTR) ? instance->ATTR[instance->INDEX] : false;
        ++instance->INDEX;
    }

    return !limit;
}

static bool i2c_fifo_put_recv_data(
    I2C_ST_FIFO_t *instance,
    char data, bool attr)
{
    bool limit = i2c_fifo_islimit(instance);

    if (!limit) {
        if (instance->BUFF)
            instance->BUFF[instance->INDEX] = data;
        if (instance->ATTR)
            instance->ATTR[instance->INDEX] = attr;
        ++instance->INDEX;
    }

    return !limit;
}

I2C_ST_PACKET_t *i2c_packet_initialize(I2C_ST_PACKET_t *packet)
{
    i2c_fifo_initialize(&packet->CTRL);
    i2c_fifo_initialize(&packet->DATA);

    return packet;
}

I2C_ST_PACKET_t *i2c_packet_set_payload(
    I2C_ST_PACKET_t *packet,
    char *buf,
    int size)
{
    i2c_fifo_registry(&packet->DATA, buf, NULL, size);
    packet->DATA.LIMIT = size;

    return packet;
}

I2C_ST_PACKET_t *i2c_packet_set_control(
    I2C_ST_PACKET_t *packet,
    uint32_t address,
    bool read)
{
    I2C_UN_SLVADDR_t addr = { 0 };

    addr.DATA = 0;

    addr.bit.BITFIELD_READ = read;
    addr.bit.BITFIELD_ADDR = (uint8_t)address;
    i2c_fifo_push_data(&packet->CTRL, (char)addr.DATA, true);

    return packet;
}

I2C_ST_PACKET_t *i2c_packet_set_address(
    I2C_ST_PACKET_t *packet,
    int address,
    int size)
{
    switch (size) {
    case 4:
        i2c_fifo_push_data(&packet->CTRL, (char)(address >> 24), false);
        /* fallthrough */
    case 3:
        i2c_fifo_push_data(&packet->CTRL, (char)(address >> 16), false);
        /* fallthrough */
    case 2:
        i2c_fifo_push_data(&packet->CTRL, (char)(address >> 8), false);
        /* fallthrough */
    case 1:
        i2c_fifo_push_data(&packet->CTRL, (char)(address >> 0), false);
        break;
    default:
        packet = NULL;
        break;
    }

    return packet;
}

static bool i2c_packet_get_send_data(
    I2C_ST_PACKET_t *packet,
    char *data,
    bool *attr)
{
    bool result;

    if (i2c_fifo_islimit(&packet->CTRL))
        result = i2c_fifo_get_send_data(&packet->DATA, data, attr);
    else
        result = i2c_fifo_get_send_data(&packet->CTRL, data, attr);

    return result;
}

static bool i2c_packet_put_recv_data(I2C_ST_PACKET_t *packet, char data)
{
    bool result = i2c_fifo_put_recv_data(&packet->DATA, data, false);

    return result;
}

static bool i2c_handler_buserror(
    I2C_ST_PACKET_INFO_t *packet_info,
    const I2C_UN_BSR_t *bsr_reg,
    I2C_UN_BCR_t *bcr_reg)
{
    bool result = false;

    if (packet_info->MASTER_CODE_FLAG) {
        packet_info->MASTER_CODE_FLAG = false;
        if (!(bsr_reg->bit_COMMON.LRB))
            result = true;
    }

    return result;
}

static bool i2c_handler_normal(
    I2C_ST_PACKET_INFO_t *packet_info,
    const I2C_UN_BSR_t *bsr_reg,
    I2C_UN_BCR_t *bcr_reg,
    bool *timeout,
    bool *nak_detect)
{
    bool result = false;
    char data;
    bool attr;
    uint32_t count = 0;

    *timeout = false;
    *nak_detect = false;

    struct I2C_REG_FUNC_TABLE *i2c_reg = &tables[packet_info->TYPE];

    if (bcr_reg->bit_COMMON.MSS) {
        if (packet_info->MASTER_CODE_FLAG) {
            I2C_UN_BS2R_t unBS2R = { 0 };
            packet_info->MASTER_CODE_FLAG = 0;
            unBS2R.DATA = GET_BS2R();
            if (!(unBS2R.bit_F_I2C_SP1.MAS)) {
                bcr_reg->bit_COMMON.MSS = 0;
            } else if (
                i2c_packet_get_send_data(&packet_info->PACKET, &data, &attr) ==
                false) {
                bcr_reg->bit_COMMON.MSS = 0;
            } else {
                do {
                    unBS2R.DATA = GET_BS2R();
                    __DSB();
                    count++;
                    if (count > I2C_POLLING_LIMIT) {
                        DEBUG_PrintError(
                            "Error in %s. polling timeout "
                            "occurred. BS2R=0x%02x",
                            __func__,
                            unBS2R.DATA);
                        *timeout = true;
                        return false;
                    }
                } while (!(unBS2R.bit_F_I2C_SP1.HS));

                bcr_reg->bit_COMMON.SCC = attr ? 1 : 0;
                SET_DAR(data);
                __DSB();
            }
        } else if (bsr_reg->bit_COMMON.TRX) {
            if (bsr_reg->bit_COMMON.LRB) {
                bcr_reg->bit_COMMON.MSS = 0;
                *nak_detect = true;
            } else if (
                i2c_packet_get_send_data(&packet_info->PACKET, &data, &attr) ==
                false) {
                bcr_reg->bit_COMMON.MSS = 0;
            } else {
                bcr_reg->bit_COMMON.SCC = attr ? 1 : 0;
                SET_DAR(data);
                __DSB();
            }
        } else {
            if (bsr_reg->bit_COMMON.FBT) {
                bcr_reg->bit_COMMON.ACK =
                    i2c_fifo_isnextlimit(&packet_info->PACKET.DATA) ?
                    0 /*NAK*/ :
                    1 /*ACK*/;
            } else if (
                i2c_packet_put_recv_data(
                    &packet_info->PACKET, (char)GET_DAR()) == false) {
                /* Buffer Full */
                bcr_reg->bit_COMMON.MSS = 0; /* issue STOP CONDITION */
            } else {
                bcr_reg->bit_COMMON.MSS =
                    i2c_fifo_islimit(&packet_info->PACKET.DATA) ? 0 /*SC*/ :
                                                                  1 /*---*/;
                bcr_reg->bit_COMMON.ACK =
                    i2c_fifo_isnextlimit(&packet_info->PACKET.DATA) ?
                    0 /*NAK*/ :
                    1 /*ACK*/;
            }
        }

        if (bcr_reg->bit_COMMON.MSS == 0)
            result = true;

    } else {
        /* %MN0 SLAVE MODE is not supported */
    }

    return result;
}

void i2c_enable(I2C_ST_PACKET_INFO_t *packet_info)
{
    struct I2C_REG_FUNC_TABLE *i2c_reg = &tables[packet_info->TYPE];

    I2C_UN_BCR_t bcr_reg = { 0 };
    bcr_reg.DATA = GET_BCR();

    bcr_reg.bit_COMMON.BER = 0; /* Clear bus error status */
    SET_BCR(bcr_reg.DATA);
    __DSB();

    if (packet_info->TYPE == I2C_TYPE_F_I2C_SP1) {
        I2C_UN_BC2R_t unBC2R = { 0 };
        unBC2R.DATA = GET_BC2R();
        unBC2R.bit_F_I2C_SP1.EN = 1;
        SET_BC2R(unBC2R.DATA);
    } else {
        I2C_UN_CCR_t unCCR = { 0 };
        unCCR.DATA = GET_CCR();
        unCCR.bit_F_I2C.EN = 1;
        SET_CCR(unCCR.DATA);
    }
    __DSB();

    return;
}

static uint32_t i2c_handler_common(
    I2C_ST_PACKET_INFO_t *packet_info,
    const I2C_UN_BSR_t *bsr_reg,
    I2C_UN_BCR_t *bcr_reg)
{
    uint32_t result = 0;
    volatile uint8_t tmp;
    bool tmp_BER = false;
    bool timeout = false;
    bool nak_detect = false;

    struct I2C_REG_FUNC_TABLE *i2c_reg = &tables[packet_info->TYPE];

    if (bcr_reg->bit_COMMON.BER) { /* Bus Error */
        tmp_BER = true;

        if (i2c_handler_buserror(packet_info, bsr_reg, bcr_reg))
            result = I2C_EN_EVF_MC_BUSERROR;
        else
            result = I2C_EN_EVF_BUSERROR;

    } else if (bcr_reg->bit_COMMON.INT) { /* Normal Interrupt */
        if (i2c_handler_normal(
                packet_info, bsr_reg, bcr_reg, &timeout, &nak_detect)) {
            result = I2C_EN_EVF_FINISH;
        }

        if (timeout)
            result = I2C_EN_EVF_TIMEOUT;
        if (nak_detect)
            result = I2C_EN_EVF_NAK;
    }

    /* clear interrupt cause */
    bcr_reg->bit_COMMON.BER = 0;
    bcr_reg->bit_COMMON.INT = 0;
    SET_BCR(bcr_reg->DATA);
    __DSB();

    if (tmp_BER)
        i2c_enable(packet_info);

    /*===== [Important] T_PLINTR : INTR delays 3PCLK CYCLE =====*/
    tmp = GET_BSR(); /* read interrupt status */
    tmp = GET_BSR(); /* read interrupt status */
    tmp = GET_BSR(); /* read interrupt status */
    (void)tmp;

    return result;
}

I2C_ERR_t i2c_handler_polling(I2C_ST_PACKET_INFO_t *packet_info)
{
    I2C_UN_BSR_t bsr_reg = { 0 }; /* +00H BSR BUS STATUS REGISTER */
    I2C_UN_BCR_t bcr_reg = { 0 }; /* +04H BCR BUS CONTROL REGISTER */
    uint32_t result;
    I2C_ERR_t ercd = I2C_ERR_OK;
    uint32_t count = 0;

    struct I2C_REG_FUNC_TABLE *i2c_reg = &tables[packet_info->TYPE];

    for (result = 0; result == 0; /*nop*/) {
        do {
            bcr_reg.DATA = GET_BCR();
            __DSB();
            count++;
            if (count > I2C_POLLING_LIMIT) {
                DEBUG_PrintError(
                    "Error in %s. polling timeout occurred. "
                    "BCR=0x%02x",
                    __func__,
                    bcr_reg.DATA);
                return I2C_ERR_POLLING;
            }
        } while (!(bcr_reg.bit_COMMON.BER || bcr_reg.bit_COMMON.INT));
        bsr_reg.DATA = GET_BSR();
        result = i2c_handler_common(packet_info, &bsr_reg, &bcr_reg);
    }

    switch (result) {
    case I2C_EN_EVF_BUSERROR:
        ercd = I2C_ERR_BER;
        break;
    case I2C_EN_EVF_MC_BUSERROR:
        ercd = I2C_ERR_BER_MC;
        break;
    case I2C_EN_EVF_TIMEOUT:
        ercd = I2C_ERR_POLLING;
        break;
    case I2C_EN_EVF_NAK:
        ercd = I2C_ERR_UNAVAILABLE;
        break;
    default:
        break;
    }

    return ercd;
}

I2C_ERR_t i2c_initialize(
    I2C_ST_PACKET_INFO_t *packet_info,
    uint32_t reg_base,
    I2C_TYPE enType,
    const I2C_PARAM_t *param)
{
    I2C_UN_BCR_t bcr_reg = { 0 }; /* +04H BCR BUS CONTROL REGISTER */
    struct I2C_REG_FUNC_TABLE *i2c_reg;

    if (packet_info == NULL)
        return I2C_ERR_PARAM;

    if (reg_base == 0)
        return I2C_ERR_PARAM;

    if ((enType != I2C_TYPE_F_I2C) && (enType != I2C_TYPE_F_I2C_SP1))
        return I2C_ERR_PARAM;

    if (param == NULL)
        return I2C_ERR_PARAM;

    packet_info->I2C_BASE_ADDR = reg_base;
    packet_info->TYPE = enType;
    i2c_reg = &tables[enType];

    i2c_fifo_registry(
        &packet_info->PACKET.CTRL,
        packet_info->CTRL_BUFF,
        packet_info->CTRL_ATTR,
        sizeof(packet_info->CTRL_BUFF));

    if (enType == I2C_TYPE_F_I2C) {
        I2C_UN_FSR_t unFSR = { 0 };
        I2C_UN_CSR_t unCSR = { 0 };
        I2C_UN_CCR_t unCCR = { 0 };

        unFSR.DATA = GET_FSR();
        unFSR.bit_F_I2C.FS = param->I2C_PARAM_F_I2C.FSR_FS;
        SET_FSR(unFSR.DATA);

        unCSR.DATA = GET_CSR();
        unCSR.bit_F_I2C.CS = param->I2C_PARAM_F_I2C.CSR_CS;
        SET_CSR(unCSR.DATA);

        unCCR.DATA = GET_CCR();
        unCCR.bit_F_I2C.CS = param->I2C_PARAM_F_I2C.CCR_CS;
        unCCR.bit_F_I2C.FM = param->I2C_PARAM_F_I2C.CCR_FM;
        SET_CCR(unCCR.DATA);
    } else {
        I2C_UN_NFR_t unNFR = { 0 };
        uint8_t tmp = 0;

        unNFR.bit_F_I2C_SP1.NF = param->I2C_PARAM_F_I2C_SP1.NFR_NF;
        unNFR.bit_F_I2C_SP1.NFH = param->I2C_PARAM_F_I2C_SP1.NFR_NFH;
        SET_NFR(unNFR.DATA);

        tmp = param->I2C_PARAM_F_I2C_SP1.TLWR_TLW;
        SET_TLWR(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TLW2R_TLW;
        SET_TLW2R(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.THWR_THW;
        SET_THWR(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.THW2R_THW;
        SET_THW2R(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TBFR_TBF;
        SET_TBFR(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TBF2R_TBF;
        SET_TBF2R(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TRSR_TRS;
        SET_TRSR(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TRS2R_TRS;
        SET_TRS2R(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TSHR_TSH;
        SET_TSHR(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TSH2R_TSH;
        SET_TSH2R(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TPSR_TPS;
        SET_TPSR(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TPS2R_TPS;
        SET_TPS2R(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TLWRH_TLWH;
        SET_TLWRH(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.THWRH_THWH;
        SET_THWRH(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TRSRH_TRSH;
        SET_TRSRH(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TSHRH_TSHH;
        SET_TSHRH(tmp);

        tmp = param->I2C_PARAM_F_I2C_SP1.TPSRH_TPSH;
        SET_TPSRH(tmp);
    }
    __DSB();

    /*
     * i2c_enable() is moved to each i2c access function
     */

    bcr_reg.DATA = GET_BCR();
    bcr_reg.bit_COMMON.INTE = 0;
    bcr_reg.bit_COMMON.BEIE = 0;
    SET_BCR(bcr_reg.DATA);
    __DSB();

    return I2C_ERR_OK;
}

I2C_ERR_t i2c_exec_transfer(I2C_ST_PACKET_INFO_t *packet_info)
{
    I2C_UN_BCR_t bcr_reg = { 0 }; /* +04H BCR BUS CONTROL REGISTER */
    char data;
    bool attr;
    uint32_t count = 0;

    struct I2C_REG_FUNC_TABLE *i2c_reg = &tables[packet_info->TYPE];

    if (i2c_packet_get_send_data(&packet_info->PACKET, &data, &attr)) {
        I2C_UN_BSR_t bsr_reg = { 0 }; /* +00H BSR BUS STATUS REGISTER */
        do {
            bsr_reg.DATA = GET_BSR();
            __DSB();
            count++;
            if (count > I2C_POLLING_LIMIT) {
                DEBUG_PrintError(
                    "Error in %s. polling timeout occurred. "
                    "BSR=0x%02x",
                    __func__,
                    bsr_reg.DATA);
                return I2C_ERR_POLLING;
            }
        } while (bsr_reg.bit_COMMON.BB);
        SET_DAR(data);
        __DSB();
        bcr_reg.DATA = GET_BCR();
        bcr_reg.bit_COMMON.MSS = 1; /* I2C=MASTER MODE */
        SET_BCR(bcr_reg.DATA);
        __DSB();
    }

    return I2C_ERR_OK;
}

void i2c_disable(I2C_ST_PACKET_INFO_t *packet_info)
{
    struct I2C_REG_FUNC_TABLE *i2c_reg = &tables[packet_info->TYPE];

    if (packet_info->TYPE == I2C_TYPE_F_I2C_SP1) {
        I2C_UN_BC2R_t unBC2R = { 0 };
        unBC2R.DATA = GET_BC2R();
        unBC2R.bit_F_I2C_SP1.EN = 0;
        SET_BC2R(unBC2R.DATA);
    } else {
        I2C_UN_CCR_t unCCR = { 0 };
        unCCR.DATA = GET_BC2R();
        unCCR.bit_F_I2C.EN = 0;
        SET_CCR(unCCR.DATA);
    }
    __DSB();

    return;
}
