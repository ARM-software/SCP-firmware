# Unit Testing
This directory contains a basic unit test framework for the SCP firmware.
Please note that this is under development and subject to change.

## Background
SCP-firmware provides few unit tests for core framework under framework/test
directory. However this can not be used to implement unit tests for other
components such as various modules. Hence a generic unit test framework was
needed. This directory provides necessary unit test framework that can be
used for this case.

This unit test framework is implemented using
[Unity](http://www.throwtheswitch.org/unity) and
[CMock](http://www.throwtheswitch.org/cmock) components.

It is recommended to read the documentation available on those links before
you proceed below.

The build system is implemented using CMake and it uses CTest feature of
CMake to run the tests, although it is not necessary.

---
# Quick Start: Building and executing sample test

- Pre-requisite

  * Unity and CMock:

    This framework depends on external components Unity and CMock

    Execute below instructions to get the git submodules

    ```sh
    $ git submodule update --init --recursive
    ```
   * CMake:

    Read doc/cmake_readme.md for overall setup for cmake (If not done before)
    after that execute below instruction in this directory.

- Build and run unit tests

    ```sh
    $ make -f Makefile.cmake mod_test
     ```

    These unit tests are also included in the test target, alongside framework tests.

    ```sh
    $ make -f Makefile.cmake test
     ```
---
# Description

### Components
```
unit_test
        ├── cfg.yml
        ├── CMakeLists.txt
        ├── unity_mocks
        │   ├── arch_helpers.h
        │   └── mocks
    ├── utils
        │   ├── generate_coverage_report.py
        │   └── zip_coverage_report.sh
        ├── gm.rb
        └── user_guide.md
contrib
        └── cmock


- cfg.yml: Default configuration for generating Mocks
- CMakeLists.txt:
  Toplevel CMakeLists file for building unit test framework and test cases
- unity_mocks: This contains generated mocks for framework component
- utils: This script files will generate code coverage reports in the form of xml & html.
- gm.rb: A ruby script to generate mocks for passed header file.
- user_guide.md: This file.
- cmock: CMock and unity source code
```
# Writing Unit Tests

Documents available at [Unity](http://www.throwtheswitch.org/unity) explains
the main concepts. However as an example, below description explains how a
simple test is implemented for source code ```module/scmi/test/mod_scmi_unit_test.c```

Below is the simple implementation of an unit test for the function
```scmi_base_discover_sub_vendor_handler``` available
in the ```module/scmi/test/mod_scmi_unit_test.c```

```
#include "unity.h"

void setUp (void) {} /* Is run before every test, put unit init calls here. */
void tearDown (void) {} /* Is run after every test, put unit clean-up calls here. */

void test_TheFirst(void)
{
    TEST_IGNORE_MESSAGE("Hello world!"); /* Ignore this test but print a message. */
}
void scmi_test01(void)
{
        int status;
        status = FWK_SUCCESS;
        TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void scmi_test02(void)
{
        int status;
        status = FWK_SUCCESS;
        TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

}

int scmi_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TheFirst);
    RUN_TEST(scmi_test01);
    RUN_TEST(scmi_test02);
    return UNITY_END();

}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_test_main();
}
#endif

```
Sadly above code is not complete but gives enough idea about how Unity
can be used. However from SCP-Firmware point of view there are various problems
that requires some work before above code can be useful. Please see below

```
#include "unity.h"

#include <mod_scmi.h>
#include <internal/mod_scmi.h>
#include <fwk_module_idx.h>
#include <fwk_element.h>

#include <module/scmi/mod_scmi.c>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_function_scmi_base_discover_sub_vendor_handler(void)
{
    fwk_id_t ex = FWK_ID_ELEMENT_INIT(
        FAKE_MODULE_ID,
        FAKE_SERVICE_IDX_OSPM);

    TEST_ASSERT_EQUAL(
        FWK_SUCCESS,
        scmi_base_discover_sub_vendor_handler(ex, NULL)
        );
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_scmi_base_discover_sub_vendor_handler);
    return UNITY_END();
}

```
As you can see above, few extra lines been added in the code above
The important line here, is the inclusion of module source code
in the unit test source file.

```
#include <module/scmi/mod_scmi.c>
```

This is necessary because we would like to bring various global objects
and ```static``` members and methods in unit tests scope which otherwise
would not be available for the function call
```scmi_base_discover_sub_vendor_handler```. Also note, use of
```TEST_ASSERT_EQUAL``` which mark test *FAIL* or *PASS* based on return value
of the function under test.

Sadly, above code is still incomplete because the function under test
```scmi_base_discover_sub_vendor_handler``` calls various other calls
which are not available for this unit test code. One way to make
these function available is to use
[CMock](http://www.throwtheswitch.org/cmock) and generate the mocked
version of the functions on which ```scmi_base_discover_sub_vendor_handler```
is dependent.

For example to generate mocked version of the functions available
in the ```framework/include/fwk_id.h``` Use below command

```sh
ruby gm.rb --mock-path=./framework/mocks
--file_name=../framework/include/fwk_id.h -ocfg.yml
```
and for ```internal``` version of this file
```framework/include/internal/fwk_id.h```

```sh
ruby gm.rb --mock-path=./framework/mocks
--file_name=../framework/include/internal/fwk_id.h -ocfg.yml
-dinternal -s_internal
```
Apart from above mocked code, we need to setup few global objects as well
the complete code can be found in

```
├── module
│   └── scmi
    ├── CMakeLists.txt
    ├── include
    │   ├── internal
    │   │   ├── mod_scmi.h
    │   │   ├── scmi_base.h
    │   │   └── scmi.h
    │   ├── mod_scmi.h
    │   ├── mod_scmi_header.h
    │   └── mod_scmi_std.h
    ├── Module.cmake
    ├── src
    │   ├── Makefile
    │   ├── mod_scmi.c
    │   └── mod_scmi.gcno
    └── test
        ├── fwk_module_idx.h
        ├── mocks
        │   ├── Mockmod_scmi_extra.c
        │   └── Mockmod_scmi_extra.h
        ├── mod_scmi_extra.h
        ├── mod_scmi_unit_test.c
        ├── scmi.cmake
        └── test_fwk_module.c

```

One of the important point to note above is the presence of
```mod_scmi_extra.h``` and its generated mock version
```Mockmod_scmi_extra.h```. This is needed because CMock can
not mock functions which are declared as function pointers in the
interface headers. So this work around helps to generate the mock
functions for these declarations.

## Adding test for new modules

<!--- TODO discover the unit test on the fly without having
to modify this CMAKE file. -->

Adding test for new modules is similar to what is done for module
```scmi```. Following describes few steps which can be followed
to add unit test for a new module

1. Create a directory under ```module``` for new module e.g.
    ```module/new_module```

2. Implement unit test for the module by creating unit test C file e.g.
    ```module/new_module/test/new_module_unit_test.c```

3. Create new_modue.cmake file under ```module/new_module/test``` e.g.
    ```module/new_module/test/new_module.cmake```

4. Modify ```unit_test/CMakeLists.txt``` and below line
    ```#Append new unit tests below here```

5. Finally create required mock sources and headers depending on the
requirement of the module under test. See ```module/scmi``` for reference

    **Note:** The common framework specific mocks should be generated under
```unit_test/unity_mocks/mocks``` and module specific mock should be
generated under module specific unit test directory e.g.
```module/new_module/test/mocks```

### Executing Tests on target hardware or FVP models

All above describes how to build and execute unit tests on host machine.
However if needed test can be executed on hardware/models as well.

Note that to build and execute tests on target require few more steps

1. Since Unity/CMock requires stdio for output the log messages
a bare minimum scp_ramfw is needed for the target under test as example
see product/juno/scp_ut

2. The target firmware must include module/ut to execute the test required

3. Follow TEST_ON_TARGET and TEST_ON_HOST in test/module/scmi/scmi.cmake
to understand setup needed building test that can be executed on target

4. Platform must provide definition for plat_execute_all_tests which
is called by module/ut. See example product/juno/scp_ut/tests_entry.c
