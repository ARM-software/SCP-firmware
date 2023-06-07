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


- cfg.yml: Default configuration for generating Mocks.
- CMakeLists.txt:
  Toplevel CMakeLists file for building unit test framework and test cases.
- unity_mocks: This contains generated mocks for framework component.
- utils: This script files will generate code coverage reports in the form of xml & html.
- gm.rb: A ruby script to generate mocks for passed header file.
- user_guide.md: This file.
- cmock: CMock and unity source code.
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
that requires some work before above code can be useful. Please see below:

```
#include "unity.h"

#include <mod_scmi.h>
#include <internal/mod_scmi.h>
#include <fwk_module_idx.h>
#include <fwk_element.h>

#include UNIT_TEST_SRC

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
As you can see above, few extra lines been added in the code.
The important line here, is the inclusion of module source code
in the unit test source file.

```
#include UNIT_TEST_SRC // equivalent to #include <module/scmi/mod_scmi.c>
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
in the ```framework/include/fwk_module.h``` Use below command

```sh
${SCP_ROOT}/unit_test/gm.rb  -m ${SCP_ROOT}/unit_test/unity_mocks/mocks/
-f fwk_module.h
```
and for ```internal``` version of this file
```framework/include/internal/fwk_module.h```

```sh
${SCP_ROOT}/unit_test/gm.rb -m ${SCP_ROOT}/unit_test/unity_mocks/mocks/
-f internal/fwk_module.h -dinternal -s_internal
```

It's recommended to export the path to gm.rb to PATH for ease of use, but
a relative reference to gm.rb from within the current directory would work
fine.

Apart from above mocked code, we need to setup few global objects as well
the complete code can be found in:

```
└── module
    └── scmi
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
        │   └── mod_scmi_base.c
        └── test
            ├── fwk_module_idx.h
            ├── mocks
            │   ├── Mockmod_scmi_extra.c
            │   ├── Mockmod_scmi_extra.h
            │   └── .clang-format
            ├── mod_scmi_extra.h
            ├── mod_scmi_unit_test.c
            ├── CMakeLists.txt
            └── fwk_module_idx.h

```

One of the important point to note above is the presence of
```mod_scmi_extra.h``` and its generated mock version
```Mockmod_scmi_extra.h```. This is needed because CMock can
not mock functions which are declared as function pointers in the
interface headers. So this work around helps to generate the mock
functions for these declarations.

When generating a new mock, a .clang-format file is placed in the
same directory so that style checks don't evaluate the autogenerated
code contained within. This will also exclude any other files contained
in the directory, so mocks should be placed in seperated mock directories
from hand-written code.

> **Note:**
> Modules depend on **external header files** must include the required
>headers under `module/<module_path>/test/ext/`
>
>Make sure to update CMakeLists.txt of the unit test to add the `ext` folder
>to the unit test includes.
> CMakeLists.txt:
>```cmake
>       ...
>       set(MODULE_UT_INC ${CMAKE_CURRENT_LIST_DIR}/ext)
>       ...
>```
>*Header files inside the `ext` folder can be a modified version for testing
>purposes.*

## Adding test for new modules

A template of minimum required files is provided as a reference for new modules.

See unit_test/template/test

1. Duplicate existing reference test directory as a starting point.

2. Modify *.cmake file for our specific test case:
    a. Change TEST_MODULE to name of module
    b. Add directories of other modules used in test to OTHER_MODULE_INC
    c. Replace framework sources with mocks using replace_with_mock
    d. Anything else

3. Populate config data, typically using a platform's config as starting point

4. Modify ```fwk_module_idx.h``` manual mock

5. Mock any APIs, by mocking the ```mod_{}_extra.h``` file

6. Write SetUp() to initialise ctx structures, bind apis etc.

7. Write test functions

8. Modify ```unit_test/CMakeLists.txt``` to append test under
    ```#Append new unit tests below here```

9. The test should be ready to run under ```make -f Makefile.cmake mod_test```

    **Note:** The common framework specific mocks should be generated under
```unit_test/unity_mocks/mocks``` and module specific mock should be
generated under module specific unit test directory e.g.
```module/new_module/test/mocks```

## fwk_core mock workaround

CMock generates the fwk_run_main_loop mock as a returning function,
because ```noreturn``` is stripped out via cfg.yml. Compilation therefore
complains when this function terminates. To work around this, an infinite
for loop needs to be appended to the function manual when updating
fwk_core's mock.

## Unit testing style guidelines

For the addition and changes for Unit Testing, it is preferable to follow
the guidelines below:
- The format for the functions' names being tested should be as follow:
  ```utest_``` followed by the function being tested and a pass or fail
  expectation, for example: ```utest_function_being_tested_init_success```.
- One test scenario for each test case. For example, if testing one case for a
  function, have a test function for that case only.
- Name the test functions according to the test being performed.

### Executing Tests on target hardware or FVP models

All above describes how to build and execute unit tests on host machine.
However, if needed, tests can be executed on hardware/models as well.

Note that to build and execute tests on target, a few more steps are required:

1. Since Unity/CMock requires stdio for output the log messages
a bare minimum scp_ramfw is needed for the target under test as example
see product/juno/scp_ut

2. The target firmware must include module/ut to execute the test required

3. Follow TEST_ON_TARGET and TEST_ON_HOST in test/module/scmi/CMakeLists.txt
to understand setup needed building test that can be executed on target

4. Platform must provide definition for plat_execute_all_tests which
is called by module/ut. See example product/juno/scp_ut/tests_entry.c

## Unit testing requirement guidelines

Unit Testing (UT) has been introduced with the aim of improving the quality and
reliability of the code.
In this introductory phase, we suggest that contributors become familiar with
the unit testing in SCP-firmware.
In the meantime, while we are not enforcing contibutors to add unit testing on
their additions or modifications, we encourage them to attempt adding UT
whenever compatible with their development.
We foresee that UT will become a mandatory requirement later in the future for
contributions into SCP-firmware project.
