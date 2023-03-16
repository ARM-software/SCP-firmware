# Device Tree Configuration Support 

The cmake build system for the SCP firmware has been updated to allow for
modules to be configured using device tree definitions.

To achieve this a number of changes have been made, to integrate some device
tree reading tools from the Zephyr project into the build processes of the SCP
firmware, alongside changes to the build system and modules to optionally
use device tree to populate configuration files.

At present this is a hybrid configuation build process allowing the mixing of modules configured
using the device tree and those configured using directly written C structure tables.

Device tree support is a build time  process only.  There is no support for accessing device tree
elements at runtime, due to the impracticalities of loading potentially large binary device tre
objects and code to interrogate them into small memory footprint devices.

## Device tree processing outline

The device tree processing converts device tree files into a set of C macros representing
the individual data values in the device tree nodes and attributes. These macros can then
be used in C code to initialise variables use to configure the SCP system.

The device tree usage is therefore a **build** time process. This happens during the CMake
generation process, where the header files containing the C macros are formed and copied into the
correct locations for use during the product compile time phase.

## Device Tree in SCP

SCP modules all contain configuration tables - a set of statically defined structures that are
written for each module used in each target in each product. Therefore for a common module such as
'sds', used by juno, which has 3 targets, there will be a config_sds.c source file in each of the
target directories.

The device tree support is designed to populate these tables using the device tree information.
As these tables contain both hardware and software configuration, this will be reflected in the
device tree support.

Further, for common modules, there will be a common config_dt_<module>.c file, filled by
the device tree information, and removing the need for multiple config source files for the
same module.


## SCP Build System Changes

The SCP cmake build system has been adjusted to integrate new tooling to process the
device tree source files and allow use of the data in the configuration code.


### Imports From Zephyr

The main Zephyr python based device tree processing library is imported into the SCP
project at the location ` ./contrib/python-devicetree/src/devicetree`

Header files containing additional macros to help with processing the generated elements
can be found at:

- `framework/include/dt_sys/util*` : files copied direct from the Zephyr project
- `framework/include/fwk_devicetree.h` : Main macro API file for using the generated DT data.
  file adapted from the Zephyr file, with Zephyr specfics removed.

A modified version of the Zephyr DT reading script has been developed (`tools/dts/dt_gen_config.py`).
This has many of the same features as the original script with necessary adjustments for the SCP
firmware build process.


This script generates a set of complex macros representing DT nodes and attributes.
These are then interpreted using the macros in `fwk_devicetree.h` to provide static values
in configuration data tables.

Further common build sources from Zephyr are added to `dts/bindings`, `dts/include` and `dts/misc`.
These consist of some baseline `.dtsi` files and initial `.yaml` bindings that are re-used.

While the device tree source files are designed to be compiled correctly by the standard device tree compiler,
the Zephyr .yaml for device tree is a slightly custom format and does not currently follow all of the
dt-schema used by the linux kernel and main device tree project. This is likely to  be addressed in future.

### Additional Python Pre-Requisites

The following additional pre-requisites are required for the full DT build flow to work:-

1) pcpp - python based C pre processor. Can be installed from pypi. (https://pypi.org/project/pcpp/)

2) cxxheaderparser - python based processor for C headers. Fetch from
https://github.com/robotpy/cxxheaderparser and run the setup to generate the required version file.

The DT build process will check that all pre-requisites (pcpp, cxxheaderparser and devicetree)  are
present before starting processing.


### The Device tree build flow

#### Updates to the base build process

The project root `CMakelists.txt` file uses a number of new global CMake variables which are used to trigger DT generation and module support.

- `SCP_DT_CONFIG_MODULES_STD` : list of  modules to be configured with DT, using a static data table accessor.
- `SCP_DT_CONFIG_MODULES_DYN` : list of modules to be configured with DT, using a dynamic accessor allowing runtime configuration table adjustments.
- `SCP_DT_CONFIG_MODULES_ALL` : combination of the above two variables, automatically created..
- `SCP_FIRMWARE_DTS_SOURCE`   : The main product .dts source file
- `SCP_FIRMWARE_DTS_INCLUDE`  : Include paths for the .dts includes - may be .h or .dtsi files


The framework `\framework\CMakelists.txt` file is updated to use the DT module variables and for each module listed in SCP_DT_CONFIG_MODULES_ALL adjusts the generated  `config_<module>` variables to be `config_dt_<module>` variables in the `fwk_module_list.c` file.

If `SCP_FIRMWARE_DTS_SOURCE` is set the root `CMakelists.txt` will include the `\cmake\dts.cmake` file,
responsible for running the DTS build.

This generation code also creates the `fwk_dt_config_dyn.h` `fwk_dt_mod_idx_gen.h` and `fwk-dt-options.dtsi` files, used in the DT compile phase. A DT specific variable `SCP_DT_BIND_H_GEN_DEFS` is also created with the a set of generated defines BUILD_HAS_MOD_... to be passed to the DT build phase.

- `fwk_dt_config_dyn.h` : contains callbacks for modules listed in the `SCP_DT_CONFIG_MODULES_DYN` list. This allows common modules used in the product to have the configuration tables adjusted at runtime in product specific code.
- `fwk_dt_mod_idx_gen.h` : set of #define values of the module indexes for inclusion in the DT source files.
- `fwk-dt-options.dtsi` : generated optional .dtsi includes - allows device tree includes to depend on build defines for optional configuration elements.

e.g. If a module such as pl011 is selected in the SCP_DT_CONFIG_MODULES_DYN list then callbacks are generated in 
`fwk_dt_config_dyn.h` as:

    #define FWK_MODULE_GEN_DYNAMIC_PL011
    extern const struct fwk_element *dyn_get_element_table_pl011(fwk_id_t module_id);
    extern const struct fwk_element *_static_get_element_table_pl011();

The staic callback is always implemented in the module source `./module/pl011/config_dt_pl011.c` :

    /* access data table from dynamic element callback */
    #ifdef FWK_MODULE_GEN_DYNAMIC_PL011 
    const struct fwk_element *_static_get_element_table_pl011() {
        return pl011_element_table;
    }
    #endif

    /* config structure */
    struct fwk_module_config config_dt_pl011 = {
        #ifdef FWK_MODULE_GEN_DYNAMIC_PL011
        .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_pl011),
        #else
        .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(pl011_element_table),
        #endif
    };


and the product must then implement the dynamic part of the callback (e.g. in ` ./product/juno/scp_ramfw/config_dt_juno_pl011.c`):

    /* example dynamic module config table implementation function */
    #ifdef FWK_MODULE_GEN_DYNAMIC_PL011
    const struct fwk_element *dyn_get_element_table_pl011(fwk_id_t module_id)
    {
        struct fwk_element *table = _static_get_element_table_pl011();

        /* do some work on the table  */
        
        return table;
    }
    #endif

In this way products can alter module element tables at runtime, even if the module is a common library module.

#### The `dts.cmake` file

This sets up and executes all the operations needed to compile and generate the header files used to access device tree values in the C source files.

The file sets up the various include source, and generated file destination paths. There are 4 key operations.

1. Run the `dt_check_py_modules.py` script. Ensures that the rest of the python scripts can be correctly executed.

2. Run the `dt_gen_enum_defines.py` script. This takes a list of .h include files and extract all the #define expressions, and converts any enum definitions into #define expressions to allow inclusion into the .dts/.dtsi files. This allows the same values used in the C code, to be used as values in the device tree source. The output of this file is the `fwk_dt_gen_bindings.h` file, which appears in the `\build\<product>/.../framework/include` directory for inclusion in the later phases.

3. Runs the C preprocessor on the .dti / .dtsi files, plus and empty.c file, stopping at the end of the pre-processing phase. This combines all the .dts / .dtsi / .h files in the devicetree source in a pre-processed output needed for the device tree script to understand.

4. Run the `dt_gen_config.py` script on the output of the previous command. This generates the set of macros based on the device tree into the `fwk_dt_config.h` file.


#### Enabling DT in the product

These `SCP_DT_CONFIG_..` list variables are set in the product `Firmware.cmake` file, where the standard module lists are created. The lists are in addition to the standard module lists

e.g. Fragment from the juno scp_ramfw Firmware.cmake file:

    # select device tree configured modules
    # These use the .dts configured variants
    # Order is not important for this list
    list(APPEND SCP_DT_CONFIG_MODULES_DYN "clock")
    list(APPEND SCP_DT_CONFIG_MODULES_STD "pl011")
    list(APPEND SCP_DT_CONFIG_MODULES_STD "mock-clock")

    # setup Device Tree sources 
    # the root source for this firmware variant
    set(SCP_FIRMWARE_DTS_SOURCE "${CMAKE_CURRENT_LIST_DIR}/juno-scp_ramfw.dts")

    # the includes needed:
    # the current include dir
    list(PREPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../include")
    # the dts/include/juno specific .dtsi files for this product
    list(PREPEND SCP_FIRMWARE_DTS_INCLUDE "${CMAKE_SOURCE_DIR}/dts/include/juno")


`Firmware.cmake` is also used to set the devicetree source file path variable `SCP_FIRMWARE_DTS_SOURCE`. By convention a .dts file will appear in the product directory. This will generally be very simple, including .dtsi include files as the main product device tree source. Any includes will be present in the  `/dts/include` and `/dts/include/<product>` directories.

e.g. the `juno-scp_ramfw.dts` file:

    #include "base.dtsi"

    /* device tree file for Juno RAM SCP firmware variant */
    #include "juno-scp-base.dtsi"
    #include "juno-scp-ramfw.dtsi"

    / {
       model = "Juno Development Platform (r0-r2)";
       compatible = "arm,juno";


    };

appears in `/product/juno/scp_ramfw` with the .dtsi files appearing in the `/dts/include` and `/dts/include/juno` directories. 


The product `CMakelists.txt` file can then be adjusted to remove all the config_<module>.c files for DT selected modules, and add any callback handlers for modules selected with SCP_DT_CONFIG_MODULES_DYN

Additional headers that must be processed in the `dt_gen_enum_defines.py` can also be added here. 

If there are build time options that necessitate additional optional .dtsi include files then the variables SCP_MODULE_DT_OPTIONS_DEFINE_REQ and SCP_MODULE_DT_OPTIONS_FILE_REQ can be set to list defines and the appropriate file to include.

e.g from juno/scp_romfw/CMakelists.txt:

    # add in juno specific headers for device tree bindings processing,
    # that are not include by other juno module builds
    list(APPEND SCP_DT_BIND_H_GEN_FROM_INCL
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_ppu_idx.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/juno_sds.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/software_mmap.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/scp_mmap.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/system_clock.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/system_mmap.h"
    )
    
    # create a list of optional .dtsi includes that will be added to the final dt compile
    # These are dependent on compile time defines which are generated later,
    # so add to define list and file list for later processing
    list(APPEND SCP_MODULE_DT_OPTIONS_DEFINE_REQ "BUILD_MODE_DEBUG")
    list(APPEND SCP_MODULE_DT_OPTIONS_FILE_REQ "juno-scp-romfw-debug-opts.dtsi")

When the build sets `BUILD_MODE_DEBUG`, the file `juno-scp-romfw-debug-opts.dtsi` will be added as an include in the `fwk-dt-options.dtsi` file.

The variables set must also be exported to the parent scope:

    # ensure include lists pushed to parent scope
    set(SCP_DT_BIND_H_GEN_FROM_INCL ${SCP_DT_BIND_H_GEN_FROM_INCL} PARENT_SCOPE)
    set(SCP_MODULE_DT_OPTIONS_DEFINE_REQ ${SCP_MODULE_DT_OPTIONS_DEFINE_REQ} PARENT_SCOPE)
    set(SCP_MODULE_DT_OPTIONS_FILE_REQ ${SCP_MODULE_DT_OPTIONS_FILE_REQ} PARENT_SCOPE)


#### Using Devicetree to configure a module.

There are a number of steps to creating the configuration files for a module to use devicetree.

1. Create appropriate .yaml bindings - or import from another source.
2. Create a config_dt_<module>.c file that can extract the information from the devicetree macros.
3. Adjust the module CMakelists to build this file if the module is part of the SCP_DT_CONFIG_MODULES_ALL list, and add any headers that may be useful to be processed by the `dt_gen_enum_defines.py` script to the `SCP_DT_BIND_H_GEN_FROM_INCL` list.

e.g. The /module/sds/CMakelists.txt adds:

    if(SCP_MODULE IN_LIST SCP_DT_CONFIG_MODULES_ALL)
        target_sources(${SCP_MODULE_TARGET}
                   PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/config_dt_sds.c")

        # add header file to list used to generate defines for dt include 
        list(APPEND SCP_DT_BIND_H_GEN_FROM_INCL "${CMAKE_CURRENT_SOURCE_DIR}/include/mod_sds.h")
        set(SCP_DT_BIND_H_GEN_FROM_INCL "${SCP_DT_BIND_H_GEN_FROM_INCL}" PARENT_SCOPE)
    
    endif()

By convention the config source file is `config_dt_<module>.c`, and **must** contain the `config_dt_<module>` variable that will be declared in the `fwk_module_list.c` file.

The module can now be used in a product by selecting in the `Firmware.cmake` file, and adding nodes in the DT source.

#### Summary of files used in Devicetree support

Used in C source code:

- `fwk_devicetree.h` : C header containing the macros to interpret the generated device tree information.
- `fwk_dt_config.h` : C header containing the macros generated from the device tree source.
- `fwk_dt_config_dyn.h` : C header containing callbacks for dynamic configured modules.
- `fwk_dt_config_common.h` : C header containing additional macros to interpret SCP specific DT elements. Also includes above three headers so can be used as only header in a C source impementing DT support.


Used in .dts / .dtsi files

- `fwk_dt_gen_bindings.h` : Generated header file containing #define values for use in device tree.
- `fwk_dt_mod_idx_gen.h` : Header containing #define values for the module index for all modules in the product.
- `fwk_dt_bindings.h` : Header file containing any additional fixed #define values for device tree support. Also includes the above two files and is included in `base.dtsi`, which must be the first include in any product .dts file.
- `fwk-dt-options.dtsi` : file generated with additional include .dtsi files according to build options for the product.

#### Additional information.

The  `fwk_devicetree.h` and `fwk_dt_config_common.h` have doxygen comments describing the usage of the macros.

More information on the device tree bindings and usage can be seen in the documentation for the Zephyr project.