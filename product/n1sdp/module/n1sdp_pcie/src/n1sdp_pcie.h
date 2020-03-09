/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_PCIE_H
#define N1SDP_PCIE_H

#include <internal/pcie_ctrl_apb_reg.h>

#include <mod_timer.h>

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * Definitions of PCIe APB register offsets from global configuration
 * base address.
 */

/*
 * Offset from ctrl_base implementing PCIe controller
 * configuration registers.
 */
#define APB_OFFSET_CTRL_REGS             0x00040000U
/*
 * Offset from ctrl_base implementing PCIe physical layer
 * configuration registers.
 */
#define APB_OFFSET_PHY_REGS              0x00000000U
/*
 * Offset from global_config_base implementing Root Port/End Point's
 * PCIe configuration space registers.
 */
#define APB_OFFSET_RP_EP_CONFIG_REGS     0x00000000U
/*
 * Offset from global_config_base implementing PCIe controller's
 * local management configuration registers.
 */
#define APB_OFFSET_LM_REGS               0x00100000U
/*
 * Offset from global_config_base implementing Root Complex's
 * AXI address translation registers.
 */
#define APB_OFFSET_RC_AXI_CONFIG_REGS    0x00400000U
/*
 * Offset from global_config_base implementing End Point's
 * AXI address translation registers.
 */
#define APB_OFFSET_EP_AXI_CONFIG_REGS    0x00400840U
/*
 * Offset between SCP and AP's view of memory space in System Access Port.
 * AP's view  : 0x40000000 - 0x7FFFFFFF
 * SCP's view : 0x60000000 - 0x9FFFFFFF
 */
#define SCP_AP_AXI_OFFSET                0x20000000U

/* NIC400 base address definition */
#define NIC400_SOC_GPV_BASE              UINT32_C(0x84000000)

/*
 * PCIe timeout values for PHY, controller & link training.
 * Timeout values specified in microseconds.
 * Note: Execution will block for the specified timeout. If the timeout
 * is too long switch to alarm API instead of blocking.
 */
#define PCIE_PHY_PLL_LOCK_TIMEOUT      UINT32_C(100)
#define PCIE_CTRL_RC_RESET_TIMEOUT     UINT32_C(100)
#define PCIE_LINK_TRAINING_TIMEOUT     UINT32_C(100000)
#define PCIE_LINK_RE_TRAINING_TIMEOUT  UINT32_C(1000000)

/* PCIe controller power on timeout (in microseconds) */
#define PCIE_POWER_ON_TIMEOUT          UINT32_C(10)

/* PCIe configuration space offset definitions */
#define PCIE_CLASS_CODE_OFFSET         0x8
#define PCIE_LINK_CTRL_STATUS_OFFSET   0xD0
#define PCIE_LINK_CTRL_STATUS_2_OFFSET 0xF0
#define PCIE_DEV_CTRL_STATUS_OFFSET    0xC8
#define PCIE_VC_RESOURCE_CTRL_0_OFFSET 0x4D4
#define PCIE_VC_RESOURCE_CTRL_1_OFFSET 0x4E0

/* PCIe configuration space link control status register definitions */
#define PCIE_LINK_CTRL_LINK_RETRAIN_MASK    0x20
#define PCIE_LINK_CTRL_NEG_SPEED_MASK       0xF
#define PCIE_LINK_CTRL_NEG_WIDTH_MASK       0x3F
#define PCIE_LINK_CTRL_NEG_SPEED_POS        16
#define PCIE_LINK_CTRL_NEG_WIDTH_POS        20

/* PCIe configuration space link control status register 2 definitions */
#define PCIE_LINK_CTRL_2_TARGET_SPEED_MASK  0xF
#define PCIE_LINK_CTRL_2_TARGET_SPEED_GEN4  0x4

/* PCIe device control & status register definitions */
#define PCIE_DEV_CTRL_MAX_PAYLOAD_SHIFT 5

/* PCIe virtual channel register definitions */
#define PCIE_VC_CTRL_VCEN_MASK         (1U << 31)
#define PCIE_VC_VCID_SHIFT             24

/* PCIe class code for PCI bridge */
#define PCIE_CLASS_CODE_PCI_BRIDGE     UINT32_C(0x06040000)

/* PCIe controller local management (LM) register offsets */
#define PCIE_LM_RC_BAR_CONFIG_REG      UINT32_C(0x300)
#define PCIE_LM_RC_CCIX_CTRL_REG       UINT32_C(0xCA4)

/* CCIX CONTROL Values */
#define CCIX_CTRL_EN_OPT_TLP           (1U << 16)
#define CCIX_CTRL_CSTT_V0_V1           (1U << 17)
#define CCIX_CTRL_CAW                  (8U << 24)

/*
 * CCIX Consortium defined Vendor ID. This vendor id is embedded in
 * the tlp packets.
 * Non Compliance to this value would cause the CCIX capable PCIe
 * controllers to drop the incoming packet.
 * This value is same as what Xilinx Hood FPGA expects.
 * To be revisited after consortium finalizes the value.
 */
#define CCIX_VENDER_ID                 (0x1E2C)

/* PCIe LM root complex bar configuration register bit masks */
#define TYPE1_PREF_MEM_BAR_ENABLE_MASK      (1U << 17)
#define TYPE1_PREF_MEM_BAR_SIZE_32BIT_MASK  (0U << 18)
#define TYPE1_PREF_MEM_BAR_SIZE_64BIT_MASK  (1U << 18)
#define TYPE1_PREF_IO_BAR_ENABLE_MASK       (1U << 19)
#define TYPE1_PREF_IO_BAR_SIZE_32BIT_MASK   (0U << 20)
#define TYPE1_PREF_IO_BAR_SIZE_64BIT_MASK   (1U << 20)

/*
 * Root port's config space cannot be written directly with its base address.
 * Bit 21 has to be set in the base address to enable writing to root port's
 * config register. This macro defines the mask with bit 21 set to be used
 * with root port write function
 */
#define ROOT_PORT_WRITE_ENABLE         UINT32_C(0x00200000)

/*
 * ECAM space per bus
 * 32 devices * 8 functions * 4kB config space = 1MB
 */
#define MAX_ECAM_SPACE_PER_BUS         (1 * FWK_MIB)

/* Maximum bus levels for type 0 and type 1 transactions */
#define MAX_TYPE0_BUS_LEVELS           2
#define MAX_TYPE1_BUS_LEVELS           16

/* Maximum AXI space for type 0 and type 1 transactions */
#define AXI_ECAM_TYPE0_SIZE            (MAX_TYPE0_BUS_LEVELS * \
                                        MAX_ECAM_SPACE_PER_BUS)
#define AXI_ECAM_TYPE1_SIZE            (MAX_TYPE1_BUS_LEVELS * \
                                        MAX_ECAM_SPACE_PER_BUS)

/* Maximum AXI space for MMIO64, MMIO32 & IO transactions */
#define AXI_MMIO64_SIZE                (128 * FWK_GIB)
#define AXI_MMIO32_SIZE                (64 * FWK_MIB)
#define AXI_IO_SIZE                    (16 * FWK_MIB)

/*
 * PCIe AXI slave ECAM memory mapping
 */
#define PCIE_AXI_ECAM_TYPE0_OFFSET     UINT32_C(0)

#define PCIE_AXI_ECAM_TYPE1_OFFSET     (PCIE_AXI_ECAM_TYPE0_OFFSET + \
                                           AXI_ECAM_TYPE0_SIZE)

/*
 * CCIX AXI slave ECAM memory mapping
 */
#define CCIX_AXI_ECAM_TYPE0_OFFSET     UINT32_C(0)

#define CCIX_AXI_ECAM_TYPE1_OFFSET     (CCIX_AXI_ECAM_TYPE0_OFFSET + \
                                           AXI_ECAM_TYPE0_SIZE)

/* PCIe AXI slave MMIO32 & IO offset addresses */
#define PCIE_AXI_MMIO32_OFFSET         (PCIE_AXI_ECAM_TYPE1_OFFSET + \
                                            AXI_ECAM_TYPE1_SIZE)
#define PCIE_AXI_IO_OFFSET             (PCIE_AXI_MMIO32_OFFSET + \
                                            AXI_MMIO32_SIZE)

/* CCIX AXI slave MMIO32 & IO offset addresses */
#define CCIX_AXI_MMIO32_OFFSET         (CCIX_AXI_ECAM_TYPE1_OFFSET + \
                                            AXI_ECAM_TYPE1_SIZE)
#define CCIX_AXI_IO_OFFSET             (CCIX_AXI_MMIO32_OFFSET + \
                                            AXI_MMIO32_SIZE)

/* AXI inbound region data */
#define AXI_IB_REGION_BASE             UINT64_C(0)
#define AXI_IB_REGION_SIZE_MSB         48

/*
 * PCIe Descriptor Register definitions
 */
#define TRANS_TYPE_MEM_IO              0x2
#define TRANS_TYPE_IO                  0x6
#define TRANS_TYPE_0_CFG               0xA
#define TRANS_TYPE_1_CFG               0xB
#define TRANS_TYPE_NORMAL              0xC
#define TRANS_TYPE_VDM                 0xD

#define PCIE_ATTR_ID_BASED_ORDERING    0x4
#define PCIE_ATTR_RELAXED_ORDERING     0x2
#define PCIE_ATTR_NO_SNOOP             0x1

#define ATS_UNTRANSLATED               0
#define ATS_TRANSLATION_REQUEST        1
#define ATS_TRANSLATED                 2

#define TRAFFIC_CLASS_0                0
#define TRAFFIC_CLASS_1                1
#define TRAFFIC_CLASS_2                2
#define TRAFFIC_CLASS_3                3
#define TRAFFIC_CLASS_4                4
#define TRAFFIC_CLASS_5                5
#define TRAFFIC_CLASS_6                6
#define TRAFFIC_CLASS_7                7

#define FORCE_ECRC                     1
#define DISABLE_ECRC                   0

#define INPUT_REQ_ID_FROM_ADDR         1

#define PASID_PRESENT                  1
#define PASID_NOT_PRESENT              0

#define PRIV_MODE_ACCESS_REQUEST       1
#define NO_PRIV_MODE_ACCESS            0

#define EXEC_MODE_ACCESS_REQUEST       1
#define NO_EXEC_MODE_ACCESS_REQUEST    0

/*
 * Maximum AXI outbound regions that can be configured
 */
#define AXI_OB_REGIONS_MAX             32

/*
 * Total register size of each outbound region
 */
#define AXI_OB_REGISTER_SET_SIZE       (8 * 4)

/*
 * Total register count of each outbound region
 */
#define AXI_OB_REGISTER_COUNT          8

/*
 * Maximum AXI inbound regions that can be configured
 */
#define AXI_IB_REGIONS_MAX             3

/*
 * Offset from AXI region configuration base where inbound region
 * configuration register starts
 */
#define AXI_IB_REGION_REGS_OFFSET      (0x800)

/*
 * Total register size of each inbound region
 */
#define AXI_IB_REGISTER_SET_SIZE       (2 * 4)

/*
 * AXI address translation register definitions
 */
#define AXI_LOW_ADDR_BIT_POS           8
#define AXI_HIGH_ADDR_BIT_POS          32
#define AXI_ADDR_NUM_BITS_MAX          ((1 << 6) - 1)

#define TX_PRESET_VALUE                0x4

#define GEN3_OFFSET_MIN                0x30C
#define GEN3_OFFSET_MAX                0x32C
#define GEN3_PRESET                    0x8

#define GEN4_OFFSET_MIN                0x9E0
#define GEN4_OFFSET_MAX                0x9F0
#define GEN4_PRESET                    0x4

/*
 * PCI express extended capability header definitions
 */
#define EXT_CAP_ID_MASK                UINT32_C(0xFFFF)
#define EXT_CAP_ID_POS                 UINT32_C(0)
#define EXT_CAP_NEXT_CAP_MASK          UINT32_C(0xFFF)
#define EXT_CAP_NEXT_CAP_POS           UINT32_C(20)
#define EXT_CAP_START_OFFSET           UINT32_C(0x100)

#define EXT_CAP_ID_ATS                 UINT16_C(0x0F)
#define EXT_CAP_ID_PRI                 UINT16_C(0x13)

/*
 * AXI override values for PCIe & CCIX root ports
 */
#define AXI_OVRD_VAL_PCIE              UINT32_C(0x00303F3B)
#define AXI_OVRD_VAL_CCIX              UINT32_C(0x00303F3B)

/*
 * AXI outbound region register set definitions
 */

/*
 * Input AXI to outbound PCIe Address translation registers
 */
struct axi_ob_addr0 {
    /* Number of bits passed through from AXI address to PCIe address */
    uint32_t num_bits:6;
    uint32_t reserved:2;
    /* Bits [31:8] of PCIe address */
    uint32_t address_bits:24;
};

struct axi_ob_addr1 {
    /* Bits [63:32] of PCIe address */
    uint32_t address_bits;
};

/*
 * Outbound PCIe descriptor registers
 */
struct axi_ob_desc0 {
    /* Outbound transaction type */
    uint32_t trans_type:4;
    /* Outbound PCIe attributes */
    uint32_t attr:3;
    /*
     * Address Translation Service
     * Config TLP - reserved
     * Mem/IO TLP - one of the address transalation types
     */
    uint32_t ats:2;
    uint32_t reserved1:8;
    uint32_t traffic_class:3;
    /* Poison mem write for memory TLP, reserved for other TLPs */
    uint32_t poison_mem_write:1;
    /* Force generation of ECRC for every TLP */
    uint32_t force_ecrc:1;
    uint32_t reserved2:1;
    /* Use bus/dev number from addrx for config TLP & descx for mem TLP */
    uint32_t bus_dev_num_from_addr_desc:1;
    /* Function number in ARI mode or Dev+Function number in non-ARI mode */
    uint32_t dev_func_num:8;
};

struct axi_ob_desc1 {
    /* Bus number if bus_dev_num_from_addr_desc in desc0 is set */
    uint32_t bus_number:8;
    uint32_t reserved:24;
};

struct axi_ob_desc2 {
    /* Steering tag for the hint */
    uint32_t steering_tag:8;
    /* Index bit */
    uint32_t index_bit:1;
    /* PH value associated with the hint */
    uint32_t ph_value:2;
    /* Transaction Processing Hint length */
    uint32_t tph_length:1;
    /* TPH request */
    uint32_t tph_req:1;
    uint32_t reserved:19;
};

struct axi_ob_desc3 {
    /* Process Address Space ID present identifier */
    uint32_t pasid_present_bit:1;
    /* PASID value */
    uint32_t pasid_value:20;
    /* Privilege mode access request */
    uint32_t priv_mode_access_req:1;
    /* Execute mode access request */
    uint32_t exec_mode_access_req:1;
    uint32_t reserved:9;
};

/*
 * Input AXI base address registers
 */
struct axi_ob_axi_base_addr0 {
    /* Region size = 2 ^ (region_sz + 1) */
    uint32_t region_sz:6;
    uint32_t reserved:2;
    /* Bits [31:8] of AXI base address */
    uint32_t axi_base_address:24;
};

struct axi_ob_axi_base_addr1 {
    /* Bits [63:32] of AXI base address */
    uint32_t axi_base_address;
};

/*
 * AXI outbound region register set
 */
struct axi_ob_config {
    struct axi_ob_addr0 addr0;
    struct axi_ob_addr1 addr1;
    struct axi_ob_desc0 desc0;
    struct axi_ob_desc1 desc1;
    struct axi_ob_desc2 desc2;
    struct axi_ob_desc3 desc3;
    struct axi_ob_axi_base_addr0 axi_base_addr0;
    struct axi_ob_axi_base_addr1 axi_base_addr1;
};

/*
 * Identifiers of PCIe initialization stages
 */
enum pcie_init_stage {
    /* PCIe controller power ON stage */
    PCIE_INIT_STAGE_PCIE_POWER_ON,

    /* CCIX controller power ON stage */
    PCIE_INIT_STAGE_CCIX_POWER_ON,

    /* PHY initialization stage */
    PCIE_INIT_STAGE_PHY,

    /* Controller initialization stage */
    PCIE_INIT_STAGE_CTRL,

    /* Link training stage */
    PCIE_INIT_STAGE_LINK_TRNG,

    /* Link re-training stage for GEN4 speed */
    PCIE_INIT_STAGE_LINK_RE_TRNG,

    /* PCIe initialization stages */
    PCIE_INIT_STAGE_COUNT,
};

/*
 * Identifiers of PCIe Generation
 */
enum pcie_gen {
    PCIE_GEN_1,
    PCIE_GEN_2,
    PCIE_GEN_3,
    PCIE_GEN_4,
};
/*
 * Structure defining data to be passed to timer API
 */
struct pcie_wait_condition_data {
    void *ctrl_apb;
    enum pcie_init_stage stage;
};

/*
 * Driver function prototypes
 */

/*
 * Brief - Function to setup an outbound region for translating
 *         incoming AXI address to outgoing PCIe address.
 *
 * param - axi_config_base_addr - APB address of AXI configuration space
 * param - axi_base_addr - AXI incoming base address
 * param - region_size - Region size of the outbound region
 * param - trans_type - Transaction type this region will generate when
 *                      accessed
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_RANGE - if the maximum outbound region is exceeded
 */
int axi_outbound_region_setup(uint32_t axi_config_base_addr,
                              uint64_t axi_base_addr,
                              uint32_t region_size,
                              uint8_t trans_type);

/*
 * Brief - Function to setup an inbound region for translating
 *         incoming PCIe address to AXI address.
 *
 * param - axi_config_base_addr - APB address of AXI configuration space
 * param - axi_base_addr - AXI base address
 * param - region_size - Region size of the inbound region
 * param - bar - BAR number to which this inbound region will be setup
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_PARAM - if the passed parameters are invalid
 */
int axi_inbound_region_setup(uint32_t axi_config_base_addr,
                             uint64_t axi_base_addr,
                             uint32_t region_size,
                             uint8_t bar);

/*
 * Brief - Function to check PCIe status in various initialization stages.
 *
 * param - data - Pointer to wait condition data
 *
 * retval - true - if the condition is met
 *          false - if the condition is not met
 */
bool pcie_wait_condition(void *data);

/*
 * Brief - Function to initialize different stages of PCIe module.
 *
 * param - ctrl_apb - Pointer to APB controller register space
 * param - timer_api - Pointer to timer API used for timeout detection
 * param - stage - Identifier of current PCIe initialization stage
 * param - gen - PCIe Generation
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_TIMEOUT - if initialization times out
 */
int pcie_init(struct pcie_ctrl_apb_reg *ctrl_apb,
              struct mod_timer_api *timer_api,
              enum pcie_init_stage stage,
              enum pcie_gen gen);


/*
 * Brief - Function to re-train PCIe link to GEN4 speed.
 *
 * param - ctrl_apb - Pointer to APB controller register space
 * param - rp_ep_config_base - Root port configuration space base address
 * param - timer_api - Pointer to timer API used for timeout detection
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_TIMEOUT - if initialization times out
 */
int pcie_link_retrain(struct pcie_ctrl_apb_reg *ctrl_apb,
                      uint32_t rp_ep_config_base,
                      struct mod_timer_api *timer_api);

/*
 * Brief - Function to initialize PCIe PHY layer.
 *
 * param - pcie_phy_base - Base address of the PHY layer registers
 */
void pcie_phy_init(uint32_t phy_apb_base);

/*
 * Brief - Function to write to Root Port's/End Point's configuration space.
 *
 * param - base - Base address of RP/EP's configuration memory
 * param - offset - Register offset from base (must be word aligned)
 * param - value - Value to write to the configuration space register
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_PARAM - if offset is not word aligned
 */
int pcie_rp_ep_config_write_word(uint32_t base,
                                 uint32_t offset,
                                 uint32_t value);

/*
 * Brief - Function to read from Root Port's/End Point's configuration space.
 *
 * param - base - Base address of RP/EP's configuration memory
 * param - offset - Register offset from base (must be word aligned)
 * param - value - Pointer to hold the read value
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_PARAM - if offset is not word aligned or value is NULL
 */
int pcie_rp_ep_config_read_word(uint32_t base,
                                uint32_t offset,
                                uint32_t *value);

/*
 * Brief - TX Equalization Preset function.
 *
 * param - rp_ep_config_apb_base - Base address of the PCIe configuration
 *                                 APB registers.
 * param - preset - Preset Value
 * param - gen - PCIe generation
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if there is a mismatch between value written
 *                       to and read from the register.
 */
int pcie_set_gen_tx_preset(uint32_t rp_ep_config_apb_base,
                           uint32_t preset,
                           enum pcie_gen gen);

/*
 * Brief - Function to skip an extended capability from capability
 *         linked list.
 *
 * param - base - Base address of RP/EP's configuration memory
 * param - ext_cap_id - Extended capability ID to be skipped
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_SUPPORT - if the specified capability is not present in the
 *                          linked list
 */
int pcie_skip_ext_cap(uint32_t base, uint16_t ext_cap_id);

/*
 * Brief - Function to setup PCIe virtual channels and map to
 *         specified traffic class.
 *
 * param - base - Base address of RP/EP's configuration memory
 * param - vc1_tc - Traffic class value for virtual channel 1
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_PARAM - if the passed parameters are inconsistent
 */
int pcie_vc_setup(uint32_t base, uint8_t vc1_tc);

#endif /* N1SDP_PCIE_H */
