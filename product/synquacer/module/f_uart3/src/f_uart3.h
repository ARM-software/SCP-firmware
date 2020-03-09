/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef F_UART3_H
#define F_UART3_H

#include <fwk_macros.h>

#include <stdint.h>

/* Normal mode registers */
struct f_uart3_reg {
    FWK_RW uint32_t RFR_TFR; /* R->RFR, W->TFR */
    FWK_RW uint32_t IER;
    FWK_RW uint32_t IIR_FCR; /* R->IIR, W->FCR */
    FWK_RW uint32_t LCR;
    FWK_RW uint32_t MCR;
    FWK_R uint32_t LSR;
    FWK_R uint32_t MSR;
    FWK_RW uint32_t SCR;
};

/* DivLatch access registers */
struct f_uart3_dla_reg {
    FWK_RW uint32_t DLL;
    FWK_RW uint32_t DLM;
    FWK_RW uint32_t IIR_FCR; /* R->IIR, W->FCR */
    FWK_RW uint32_t LCR;
    FWK_RW uint32_t MCR;
    FWK_R uint32_t LSR;
    FWK_R uint32_t MSR;
    FWK_W uint32_t TST;
};

/* Bit definition */
/* Interrupt Enable Register */
#define F_UART3_IER_ERBFI ((uint32_t)(0x1 << 0))
#define F_UART3_IER_ETBEI ((uint32_t)(0x1 << 1))
#define F_UART3_IER_ELSI ((uint32_t)(0x1 << 2))
#define F_UART3_IER_EDSSI ((uint32_t)(0x1 << 3))

/* Interrupt Identification Register */
#define F_UART3_IIR_ID ((uint32_t)(0xF << 0))
#define F_UART3_IIR_FIFO ((uint32_t)(0x3 << 6))
#define F_UART3_IIR_ID_NOINTR (uint32_t(0x1))
#define F_UART3_IIR_ID_RLINE (uint32_t(0x6))
#define F_UART3_IIR_ID_RDATA (uint32_t(0x4))
#define F_UART3_IIR_ID_TIMEOUT (uint32_t(0xC))
#define F_UART3_IIR_ID_TFEMPTY (uint32_t(0x2))
#define F_UART3_IIR_ID_MODEM (uint32_t(0x0))

/* FIFO Control Register */
#define F_UART3_FCR_RXFRST ((uint32_t)(0x1 << 1))
#define F_UART3_FCR_TXFRST ((uint32_t)(0x1 << 2))
#define F_UART3_FCR_DMA ((uint32_t)(0x1 << 3))
#define F_UART3_FCR_RCVR ((uint32_t)(0x3 << 6))

/* Rx FIFO trigger level */
#define F_UART3_FCR_RCVR_1B ((uint32_t)(0x0 << 6))
#define F_UART3_FCR_RCVR_4B ((uint32_t)(0x1 << 6))
#define F_UART3_FCR_RCVR_8B ((uint32_t)(0x2 << 6))
#define F_UART3_FCR_RCVR_14B ((uint32_t)(0x3 << 6))

/* Line Control Register */
#define F_UART3_LCR_WLS ((uint32_t)(0x3 << 0))
#define F_UART3_LCR_STB ((uint32_t)(0x3 << 2))
#define F_UART3_LCR_PEN ((uint32_t)(0x1 << 3))
#define F_UART3_LCR_EPS ((uint32_t)(0x1 << 4))
#define F_UART3_LCR_SP ((uint32_t)(0x1 << 5))
#define F_UART3_LCR_SB ((uint32_t)(0x1 << 6))
#define F_UART3_LCR_DLAB ((uint32_t)(0x1 << 7))
/* Word length */
#define F_UART3_LCR_WLS_5 ((uint32_t)(0x0))
#define F_UART3_LCR_WLS_6 ((uint32_t)(0x1))
#define F_UART3_LCR_WLS_7 ((uint32_t)(0x2))
#define F_UART3_LCR_WLS_8 ((uint32_t)(0x3))

/* Modem Control Register */
#define F_UART3_MCR_DTR ((uint32_t)(0x1 << 0))
#define F_UART3_MCR_RTS ((uint32_t)(0x1 << 1))
#define F_UART3_MCR_OUT1 ((uint32_t)(0x1 << 2))
#define F_UART3_MCR_OUT2 ((uint32_t)(0x1 << 3))
#define F_UART3_MCR_LOOP ((uint32_t)(0x1 << 4))

/* Line Status Register */
#define F_UART3_LSR_DR ((uint32_t)(0x1 << 0))
#define F_UART3_LSR_OE ((uint32_t)(0x1 << 1))
#define F_UART3_LSR_PE ((uint32_t)(0x1 << 2))
#define F_UART3_LSR_FE ((uint32_t)(0x1 << 3))
#define F_UART3_LSR_BI ((uint32_t)(0x1 << 4))
#define F_UART3_LSR_THRE ((uint32_t)(0x1 << 5))
#define F_UART3_LSR_TEMT ((uint32_t)(0x1 << 6))
#define F_UART3_LSR_ERRF ((uint32_t)(0x1 << 7))

/* Modem Status Register */
#define F_UART3_MSR_DCTS ((uint32_t)(0x1 << 0))
#define F_UART3_MSR_DDSR ((uint32_t)(0x1 << 1))
#define F_UART3_MSR_TERI ((uint32_t)(0x1 << 2))
#define F_UART3_MSR_DDCD ((uint32_t)(0x1 << 3))
#define F_UART3_MSR_CTS ((uint32_t)(0x1 << 4))
#define F_UART3_MSR_DSR ((uint32_t)(0x1 << 5))
#define F_UART3_MSR_RI ((uint32_t)(0x1 << 6))
#define F_UART3_MSR_DCD ((uint32_t)(0x1 << 7))

/*
 * Baud rate
 * F_UART3 Clock domain: MAIN_CRG11.CLK6
 *            freq  : 500MHz/8 = 62.5MHz
 *
 * DivRatio = Freq(Hz) / (16 * BaudRate(bps))
 */

/* 230.4 kbps (DivRatio: 16.95) */
#define F_UART3_DLL_230400 ((uint32_t)(0x11))
#define F_UART3_DLM_230400 ((uint32_t)(0x00))

/* 115.2 kbps (DivRatio: 33.91) */
#define F_UART3_DLL_115200 ((uint32_t)(0x22))
#define F_UART3_DLM_115200 ((uint32_t)(0x00))

/* 57.6 kbps (DivRatio: 67.82) */
#define F_UART3_DLL_57600 ((uint32_t)(0x44))
#define F_UART3_DLM_57600 ((uint32_t)(0x00))

/* 38.4 kbps (DivRatio: 101.73) */
#define F_UART3_DLL_38400 ((uint32_t)(0x66))
#define F_UART3_DLM_38400 ((uint32_t)(0x00))

/* 19.2 kbps (DivRatio: 203.45) */
#define F_UART3_DLL_19200 ((uint32_t)(0xCC))
#define F_UART3_DLM_19200 ((uint32_t)(0x00))

/* 9.6 kbps (DivRatio: 409.9) */
#define F_UART3_DLL_9600 ((uint32_t)(0x9A))
#define F_UART3_DLM_9600 ((uint32_t)(0x01))

#define F_UART3_SYSPARAM_BAUD_RATE_9600 (0)
#define F_UART3_SYSPARAM_BAUD_RATE_19200 (1)
#define F_UART3_SYSPARAM_BAUD_RATE_38400 (2)
#define F_UART3_SYSPARAM_BAUD_RATE_57600 (3)
#define F_UART3_SYSPARAM_BAUD_RATE_115200 (4)
#define F_UART3_SYSPARAM_BAUD_RATE_230400 (5)
#define F_UART3_SYSPARAM_BAUD_RATE_MAX (5)

typedef enum {
    F_UART3_NEWLINE_CODE_CRLF = 0,
    F_UART3_NEWLINE_CODE_CR = 1,
    F_UART3_NEWLINE_CODE_LF = 2
} F_UART3_newline_code_t;

#endif /* F_UART3_H */
