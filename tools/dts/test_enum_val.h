/* test header file for dt_gen_enum_defines.py */

/* standard plain enum */

enum std_enum {
    STD_0,
    STD_1,
    STD_2,
    STD_MAX
};

/* enum with values */

enum val_enum {
    VAL_0 = 5,
    VAL_1 = 25,
    VAL_2 = 55,
    VAL_MAX = 333
};

/* enum with occasional val */

enum val_set_enum {
    VAL_SET_0,
    VAL_SET_1,
    VAL_SET_33=33,
    VAL_SET_34,
    VAL_SET_35,
    VAL_SET_66=66,
    VAL_SET_67
};

/* enum with hex values */

enum val_hex_val {
    VAL_HEX_0 = 0x0,
    VAL_HEX_1 = 0x1,
    VAL_HEX_2 = 0x2,
};

/* enum with occasional hex val */

enum val_set_hex_enum {   
    VAL_HEX_SET_0,
    VAL_HEX_SET_1,
    VAL_HEX_SET_33=0x21,
    VAL_HEX_SET_34,
    VAL_HEX_SET_35,
};


/* enum with end value as string */

enum std_enum {
    STD_0,
    STD_1,
    STD_2,
    STD_MAX = VAL_MAX,
};


/* enum with expressions */
#define  MOD_SDS_ID_VERSION_MAJOR_POS 24

enum juno_sds_struct_id {
    JUNO_SDS_CPU_INFO =             1 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_ROM_VERSION =          2 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_RAM_VERSION =          3 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_PLATFORM_ID =          4 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_RESET_SYNDROME =       5 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_FEATURE_AVAILABILITY = 6 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_CPU_BOOTCTR =          7 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_CPU_FLAGS =            8 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_BOOTLOADER =           9 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
};


/* bad expression */
enum bad_expression {
    STD_0,
    STD_1 = 1 | (9 << >> )
};

/* bad unresolved string value */
enum bad_string_pos {
    STD_0,
    STD_1 = BAD_VAL,
    STD_2,
};
