/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno DMC-400 module.
 */

#ifndef MOD_JUNO_DMC400_H
#define MOD_JUNO_DMC400_H

#include "scp_config.h"

#include <mod_timer.h>

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \ingroup GroupJunoModule
 * \defgroup GroupJunoDMC400 DMC-400 Driver
 * \{
 */

/*!
 * \brief Time-out when programming the DDR in us.
 */
#define TIMEOUT_DMC400_US                   (1000 * 1000)

/*!
 * \brief Delay after remove DDR PORESET in cycles.
 */
#define DELAY_DDR_PORESET_CYCLES            10

/*!
 * \brief Delay after direct commands to initialize LPDDR3 in cycles.
 */
#define DELAY_DDR_INIT_LPDDR3_CYCLES        100

/*!
 * \brief User config twrdata_delay.
 */
#define DMC_USER_CONFIG_TWRDATA_DELAY       UINT32_C(0x00000001)

/*!
 * \brief User config twrdata_en_delay.
 */
#define DMC_USER_CONFIG_TWRDATA_EN_DELAY    UINT32_C(0x00000002)

/*!
 * \brief DMC-400 register definitions.
 */
struct mod_juno_dmc400_reg {
    /*!
     * \cond
     * @{
     */
    FWK_R   uint32_t MEMC_STATUS;
    FWK_R   uint32_t MEMC_CONFIG;
    FWK_W   uint32_t MEMC_CMD;
            uint8_t  RESERVED0[0x010 - 0x00C];
    FWK_RW  uint32_t ADDRESS_CONTROL;
    FWK_RW  uint32_t DECODE_CONTROL;
    FWK_RW  uint32_t FORMAT_CONTROL;
            uint8_t  RESERVED1[0x020 - 0x01C];
    FWK_RW  uint32_t LOW_POWER_CONTROL;
            uint8_t  RESERVED2[0x030 - 0x024];
    FWK_RW  uint32_t TURNAROUND_PRIORITY;
    FWK_RW  uint32_t HIT_PRIORITY;
    FWK_RW  uint32_t QOS0_CONTROL;
    FWK_RW  uint32_t QOS1_CONTROL;
    FWK_RW  uint32_t QOS2_CONTROL;
    FWK_RW  uint32_t QOS3_CONTROL;
    FWK_RW  uint32_t QOS4_CONTROL;
    FWK_RW  uint32_t QOS5_CONTROL;
    FWK_RW  uint32_t QOS6_CONTROL;
    FWK_RW  uint32_t QOS7_CONTROL;
    FWK_RW  uint32_t QOS8_CONTROL;
    FWK_RW  uint32_t QOS9_CONTROL;
    FWK_RW  uint32_t QOS10_CONTROL;
    FWK_RW  uint32_t QOS11_CONTROL;
    FWK_RW  uint32_t QOS12_CONTROL;
            uint8_t  RESERVED3[0x070 - 0x06C];
    FWK_RW  uint32_t QOS13_CONTROL;
    FWK_RW  uint32_t QOS14_CONTROL;
    FWK_RW  uint32_t QOS15_CONTROL;
    FWK_RW  uint32_t TIMEOUT_CONTROL;
    FWK_RW  uint32_t QUEUE_CONTROL;
            uint8_t  RESERVED4[0x088 - 0x084];
    FWK_RW  uint32_t WRITE_PRIORITY_CONTROL;
    FWK_RW  uint32_t WRITE_PRIORITY_CONTROL2;
    FWK_RW  uint32_t READ_PRIORITY_CONTROL;
    FWK_RW  uint32_t READ_PRIORITY_CONTROL2;
    FWK_RW  uint32_t ACCESS_ADDRESS_MATCH_31_00;
    FWK_RW  uint32_t ACCESS_ADDRESS_MATCH_63_32;
    FWK_RW  uint32_t ACCESS_ADDRESS_MASK_31_00;
    FWK_RW  uint32_t ACCESS_ADDRESS_MASK_63_32;
            uint8_t  RESERVED5[0x100 - 0x0A8];
    FWK_R   uint32_t CHANNEL_STATUS;
            uint8_t  RESERVED6[0x108 - 0x104];
    FWK_W   uint32_t DIRECT_CMD;
            uint8_t  RESERVED7[0x110 - 0x10C];
    FWK_R   uint32_t MR_DATA;
            uint8_t  RESERVED8[0x120 - 0x114];
    FWK_RW  uint32_t REFRESH_CONTROL;
            uint8_t  RESERVED9[0x128 - 0x124];
    FWK_RW  uint32_t INTERRUPT_CONTROL;
            uint8_t  RESERVED10[0x130 - 0x12C];
    FWK_W   uint32_t INTERRUPT_CLR;
            uint8_t  RESERVED11[0x138 - 0x134];
    FWK_R   uint32_t INTERRUPT_STATUS;
            uint8_t  RESERVED12[0x140 - 0x13C];
    FWK_R   uint32_t INTERRUPT_INFO;
            uint8_t  RESERVED13[0x148 - 0x144];
    FWK_RW  uint32_t MODE_CONTROL;
            uint8_t  RESERVED14[0x200 - 0x14C];
    FWK_RW  uint32_t T_REFI;
    FWK_RW  uint32_t T_RFC;
    FWK_RW  uint32_t T_MRR;
    FWK_RW  uint32_t T_MRW;
            uint8_t  RESERVED15[0x218 - 0x210];
    FWK_RW  uint32_t T_RCD;
    FWK_RW  uint32_t T_RAS;
    FWK_RW  uint32_t T_RP;
    FWK_RW  uint32_t T_RPALL;
    FWK_RW  uint32_t T_RRD;
    FWK_RW  uint32_t T_FAW;
    FWK_RW  uint32_t READ_LATENCY;
    FWK_RW  uint32_t T_RTR;
    FWK_RW  uint32_t T_RTW;
    FWK_RW  uint32_t T_RTP;
    FWK_RW  uint32_t WRITE_LATENCY;
    FWK_RW  uint32_t T_WR;
    FWK_RW  uint32_t T_WTR;
    FWK_RW  uint32_t T_WTW;
    FWK_RW  uint32_t T_ECKD;
    FWK_RW  uint32_t T_XCKD;
    FWK_RW  uint32_t T_EP;
    FWK_RW  uint32_t T_XP;
    FWK_RW  uint32_t T_ESR;
    FWK_RW  uint32_t T_XSR;
    FWK_RW  uint32_t T_SRCKD;
    FWK_RW  uint32_t T_CKSRD;
            uint8_t  RESERVED16[0x300 - 0x270];
    FWK_RW  uint32_t T_RDDATA_EN;
    FWK_RW  uint32_t T_PHYWRLAT;
    FWK_RW  uint32_t RDLVL_CONTROL;
    FWK_RW  uint32_t RDLVL_MRS;
    FWK_W   uint32_t RDLVL_DIRECT;
            uint8_t  RESERVED17[0x318 - 0x314];
    FWK_RW  uint32_t T_RDLVL_EN;
    FWK_RW  uint32_t T_RDLVL_RR;
            uint8_t  RESERVED18[0x328 - 0x320];
    FWK_RW  uint32_t WRLVL_CONTROL;
    FWK_RW  uint32_t WRLVL_MRS;
    FWK_W   uint32_t WRLVL_DIRECT;
            uint8_t  RESERVED19[0x338 - 0x334];
    FWK_RW  uint32_t T_WRLVL_EN;
    FWK_RW  uint32_t T_WRLVL_WW;
            uint8_t  RESERVED20[0x348 - 0x340];
    FWK_RW  uint32_t PHY_CONTROL_POWER;
            uint8_t  RESERVED21[0x350 - 0x34C];
    FWK_RW  uint32_t PHY_UPDATE_CONTROL;
            uint8_t  RESERVED22[0x358 - 0x354];
    FWK_RW  uint32_t T_LPRESP;
            uint8_t  RESERVED23[0x400 - 0x35C];
    FWK_R   uint32_t USER_STATUS;
    FWK_RW  uint32_t USER_CONFIG0;
    FWK_RW  uint32_t USER_CONFIG1;
            uint8_t  RESERVED24[0xE00 - 0x40C];
    FWK_RW  uint32_t INTEG_CFG;
            uint8_t  RESERVED25[0xE08 - 0xE04];
    FWK_W   uint32_t INTEG_OUTPUTS;
            uint8_t  RESERVED26[0xFD0 - 0xE0C];
    FWK_R   uint32_t PERIPH_ID_4;
            uint8_t  RESERVED27[0xFE0 - 0xFD4];
    FWK_R   uint32_t PERIPH_ID_0;
    FWK_R   uint32_t PERIPH_ID_1;
    FWK_R   uint32_t PERIPH_ID_2;
    FWK_R   uint32_t PERIPH_ID_3;
    FWK_R   uint32_t COMPONENT_ID_0;
    FWK_R   uint32_t COMPONENT_ID_1;
    FWK_R   uint32_t COMPONENT_ID_2;
    FWK_R   uint32_t COMPONENT_ID_3;
    /*!
    * \endcond
    * @}
    */
};

/*!
 * \brief Mask used to issue a GO command (CONFIG to READY state).
 */
#define DMC400_CMD_GO                               UINT32_C(0x00000003)

/*!
 * \brief Mask used to issue a CONFIGURE command.
 */
#define DMC400_CMD_CONFIG                           UINT32_C(0x00000000)

/*!
 * \brief Mask used to issue a SLEEP command.
 */
#define DMC400_CMD_SLEEP                            UINT32_C(0x00000001)

/*!
 * \brief Mask used for the memory controller status bitfield.
 */
#define DMC_MEMC_STATUS                             UINT32_C(0x00000003)

/*!
 * \brief Create the ADDRESS_CONTROL value.
 *
 * \param CHANN Number of channel address bits.
 * \param CHIP Number of chip-select bits on each interface.
 * \param BANK Number of bank address bits.
 * \param ROW Number of row address bits.
 * \param COL Number of column address bits.
 *
 * \return The ADDRESS_CONTROL value.
 */
#define ADDRESS_CONTROL_VAL(CHANN, CHIP, BANK, ROW, COL)    (((CHANN) << 28) | \
                                                             ((CHIP) << 24) | \
                                                             ((BANK) << 16) | \
                                                             ((ROW) << 8) | \
                                                             (COL))

/*!
 * \brief Mask used for CRNTCLKDIV bitfield.
 */
#define DMCCLK_CONTROL_CRNTCLKDIV                   UINT32_C(0x000F0000)

/*!
 * \brief Mask used for CRNTCLK bitfield.
 */
#define DMCCLK_CONTROL_CRNTCLK                      UINT32_C(0x0000F000)

/*!
 * \brief Mask used for CLKDIV bitfield.
 */
#define DMCCLK_CONTROL_CLKDIV                       UINT32_C(0x000000F0)

/*!
 * \brief Mask used for CLKSEL bitfield.
 */
#define DMCCLK_CONTROL_CLKSEL                       UINT32_C(0x0000000F)

/*!
 * \brief Setting used to request SYSINCLK as a clock source.
 */
#define DMCCLK_CONTROL_CLKSEL_SYSINCLK              UINT32_C(0x00000002)

/*!
 * \brief User config core_prstn.
 */
#define DMC_USER_CONFIG_CORE_PRSTN                  UINT32_C(0x00000010)

/*!
 * \brief User config core_srstn.
 */
#define DMC_USER_CONFIG_CORE_SRSTN                  UINT32_C(0x00000020)

/*!
 * \brief User config dfi_init_start.
 */
#define DMC_USER_CONFIG_DFI_INIT_START              UINT32_C(0x00000040)

/*!
 * \brief User status dfi_init_complete_0.
 */
#define DMC_USER_STATUS_DFI_INIT_COMPLETE_0         UINT32_C(0x00000004)

/*!
 * \brief User status dfi_init_complete_1.
 */
#define DMC_USER_STATUS_DFI_INIT_COMPLETE_1         UINT32_C(0x00000040)

/*!
 * \brief State information for memory channel 0 mask.
 */
#define  DMC_CHANNEL_STATUS_M0_MASK                 UINT32_C(0x0000000F)

/*!
 * \brief State information for memory channel 1 mask.
 */
#define  DMC_CHANNEL_STATUS_M1_MASK                 UINT32_C(0x000000F0)

/*!
 * \brief Memory channel 0 IDLE state bit.
 */
#define  DMC_CHANNEL_STATUS_M0_IDLE                 UINT32_C(0x00000002)

/*!
 * \brief Memory channel 1 IDLE state bit.
 */
#define  DMC_CHANNEL_STATUS_M1_IDLE                 UINT32_C(0x00000020)

/*!
 * \brief Mask used to issue an AUTOREFRESH command.
 */
#define DIRECT_CMD_AUTOREFRESH                      UINT32_C(0x30000000)

/*!
 * \brief Target memory channel 0.
 */
#define DIRECT_CMD_CHANNEL_0_ADDR                   UINT32_C(0 << 24)

/*!
 * \brief Target memory channel 1.
 */
#define DIRECT_CMD_CHANNEL_1_ADDR                   UINT32_C(1 << 24)

/*!
 * \brief Target chip select 0.
 */
#define DIRECT_CMD_CHIP_0_ADDR                      UINT32_C(0 << 20)

/*!
 * \brief Target chip select 1.
 */
#define DIRECT_CMD_CHIP_1_ADDR                      UINT32_C(1 << 20)

/*!
 * \brief Program to send a request for write leveling training.
 */
#define WRITE_LEVELING_REQUEST_SEND                 UINT32_C(0x00000001)

/*!
 * \brief Program target channel 0 for write leveling training.
 */
#define WRITE_LEVELING_CHANNEL_0                    UINT32_C(0 << 28)

/*!
 * \brief Program target channel 1 for write leveling training.
 */
#define WRITE_LEVELING_CHANNEL_1                    UINT32_C(1 << 28)

/*!
 * \brief Program target chip 0 for write leveling training.
 */
#define WRITE_LEVELING_CHIP_0                       UINT32_C(0 << 24)

/*!
 * \brief Program target chip 1 for write leveling training.
 */
#define WRITE_LEVELING_CHIP_1                       UINT32_C(1 << 24)

/*!
 * \brief Send a request for read gate training.
 */
#define READ_GATE_TRAINING_REQUEST_SEND             UINT32_C(0x00000002)

/*!
 * \brief Send a request for read eye training.
 */
#define READ_EYE_TRAINING_REQUEST_SEND              UINT32_C(0x00000001)

/*!
 * \brief Program target channel 0 for read leveling training.
 */
#define READ_LEVELING_CHANNEL_0                     UINT32_C(0 << 28)

/*!
 * \brief Program target channel 1 for read leveling training.
 */
#define READ_LEVELING_CHANNEL_1                     UINT32_C(1 << 28)

/*!
 * \brief Program target chip 0 for read leveling training.
 */
#define READ_LEVELING_CHIP_0                        UINT32_C(0 << 24)

/*!
 * \brief Program target chip 1 for read leveling training.
 */
#define READ_LEVELING_CHIP_1                        UINT32_C(1 << 24)

/*!
 * \brief DMC-400 module configuration.
 */
struct mod_juno_dmc400_module_config {
    /*!
     * \brief Element identifier of the timer used for time-out when programming
     *     the DMC-400.
     */
    fwk_id_t timer_id;

    /*! DDR PHY module identifier */
    fwk_id_t ddr_phy_module_id;

    /*! DDR PHY API identifier */
    fwk_id_t ddr_phy_api_id;
};

/*!
 * \brief Element configuration.
 */
struct mod_juno_dmc400_element_config {
    /*! Base address of the DMC-400 device's registers */
    uintptr_t dmc;

    /*! Element identifier of the first DDR PHY-400 device */
    fwk_id_t ddr_phy_0_id;

    /*! Element identifier of the second DDR PHY-400 device */
    fwk_id_t ddr_phy_1_id;

    /*! Version-specific address control */
    uint32_t address_control;

    /*! Version-specific chip count */
    uint32_t ddr_chip_count;

    /*! Flag indicating whether the current platform is an FVP */
    bool is_platform_fvp;

    /*! Power domain identifier */
    fwk_id_t pd_id;
};

/*!
 * \brief API of the DDR PHY associated to the DMC
 */
struct mod_juno_dmc400_ddr_phy_api {
    /*!
     * \brief Configure the DDR PHY device.
     *
     * \param element_id Element identifier corresponding to the device to
     *      configure.
     *
     * \retval FWK_E_DATA No data found for the element.
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*configure_ddr)(fwk_id_t element_id);

    /*!
     * \brief Configure the clock for DDR PHY device.
     *
     * \param module_id Module identifier corresponding to the device to
     *      configure.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*configure_clk)(fwk_id_t module_id);

    /*!
     * \brief Place the DDR PHY device into idle state.
     *
     * \param element_id Element identifier corresponding to the device to
     *      configure.
     *
     * \retval FWK_E_DATA No data found for the element.
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*configure_idle)(fwk_id_t element_id);

    /*!
     * \brief Place the DDR PHY device into retention state.
     *
     * \param module_id Module identifier corresponding to the device to
     *      configure.
     * \param enable Flag to indicate the retention operation.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*configure_retention)(fwk_id_t module_id, bool enable);
};

/*!
 * @}
 */

#endif  /* MOD_JUNO_DMC400_H */
