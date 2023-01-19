Coding Rules
============

To maintain consistency within the SCP/MCP software source code and to reduce
the risk of bugs, a series of rules and guidelines have been created; these form
the project's coding rules.

General Considerations
----------------------
The software follows the ISO/IEC 9899:2011 standard (C11). This is enforced
through the use of compilation flags and all warnings being considered as
errors. Compilation flags are also used to avoid constructs usually considered
questionable, and that are easy to avoid.

For robustness and reliability reasons, dynamic memory allocation is allocate-
only and must be done through the facilities provided by the firmware framework.
The intent is for memory allocations to be done during the pre-runtime phase or
early in the runtime phase based on configuration data or hardware detection.
Allocated memory cannot be freed or reallocated.

Static Analysis
---------------
This project uses `cppcheck` to do a static analysis of the codebase for
possible programmer error and other sources of bugs. This analysis occurs as a
part of the standard continuous integration flow on pull requests. If the
analyser identifies potential issues, the patch will be automatically rejected.

To invoke `cppcheck` manually, use:

``` bash
cppcheck --xml \
         --enable=all \
         --force \
         --suppressions-list="${WORKDIR}/tools/cppcheck_suppress_list.txt" \
         --includes-file="${WORKDIR}/framework/include/" \
         -I "${WORKDIR}/CMSIS" \
         -i cmsis \
         -U__CSMC__ \
         ${WORKDIR}
```

The CMSIS submodule is not checked. Errors are printed in XML format on the
error output.

Certain checks have been manually suppressed. Checks are generally only
disabled if they have been triaged as false positives. The list of suppressed
checks can be found [here](../tools/cppcheck_suppress_list.txt).

## Static analysis using `cmake`

When this project is built using `cmake`, `cppcheck` is performed automatically
for the current build using the list of suppressed checks mentioned in the
previous section.
`cppcheck` can be run standalone with the current build configuration using
the following commands:

``` bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -B ${BUILD_DIR} ... <cmake configuation extra parameters>

cmake --build ${BUILD_DIR} -- -n

cppcheck --xml \
         --enable=all \
         --force \
         --suppressions-list="${WORKDIR}/tools/cppcheck_suppress_list.txt" \
         --project="${BUILD_DIR}/compile_commands.json"
```

MISRAC-2012
-----------
The SCP-firmware is attempting to achieve compliance with MISRAC-2012.

In order to provide a reference, Juno platform is taken as a first platform to
attempt alignment with the guidance. At the same time, a MISRAC-2012 checker
is currently being run upon PR submissions.

To date, *all* violations of the **Mandatory Rules** have been **resolved**:

* Rule 9.1,
* Rule 12.5
* Rule 13.6
* Rules 17.3, 17.4, 17.6,
* Rule 19.1,
* Rules 21.13, 21.17, 21.18, 21.19, 21.20,
* Rules 22.2, 22.4, 22.5, 22.6

Other *Required Directives* have also had *all* their violations **resolved**:

* Directive 1.1
* Directive 2.1
* Directive 3.1
* Directives 4.1, 4.3, 4.7, 4.11, 4.14

Other *Required Rules* have also had *all* their violations **resolved**:

* Rules 1.1, 1.3
* Rule 3.2
* Rule 4.1
* Rules 5.2, 5.3, 5.4, 5.5, 5.6, 5.7
* Rules 6.1, 6.2
* Rules 7.1, 7.3, 7.4
* Rules 8.1, 8.8, 8.10, 8.12
* Rules 9.2, 9.4
* Rules 10.1, 10.2, 10.3
* Rules 11.2, 11.7, 11.9
* Rules 13.1, 13.2, 13.5
* Rules 14.1, 15.2, 15.3, 15.6
* Rules 16.2, 16.5, 16.7
* Rules 17.2, 17.7
* Rules 18.1, 18.6
* Rules 20.2, 20.3, 20.4, 20.6, 20.8, 20.11, 20.13, 20.14
* Rules 21.4, 21.5, 21.7, 21.8, 21.9, 21.10, 21.11, 21.14, 21.16
* Rules 22.3, 22.7, 22.8, 22.9, 22.10

The list of *Required Rules and Directives* that are currently being
**deviated** from can be seen below:

* Directives 4.10, 4.12
* Rule 2.1
* Rule 3.1
* Rules 8.2, 8.4, 8.5, 8.6, 8.14
* Rules 10.7, 10.8
* Rules 11.1, 11.3, 11.6, 11.8
* Rule 12.2
* Rules 14.2, 14.3
* Rules 16.1, 16.3, 16.4, 16.6
* Rules 17.1, 17.7
* Rules 18.2, 18.3, 18.7, 18.8
* Rule 20.12
* Rules 21.1, 21.2, 21.3, 21.6, 21.15

*Advisory rules* are currently **not considered/treated**.

Please note that new patches will have to be compliant with the current status
of resolved rules/directives.

C Standard Library
------------------
When developing a module, only the following headers of the C standard library
are allowed to be included:
`<limits.h>`, `<stdarg.h>`, `<stdbool.h>`, `<stddef.h>`, `<stdint.h>`,
`<stdio.h>`, `<stdlib.h>`, `<string.h>` and `<inttypes.h>`.

Concerning the library functions defined in `<stdio.h>`, only `snprintf()` may
be used.

Concerning the library functions defined in `<stdlib.h>`, only `bsearch()`,
`qsort()`, `abs()` and `rand()` may be used.

Concerning the library functions defined in `<string.h>`, `strcat()` and
`strcpy()` cannot be used. Use `strncat()` and `strncpy()` instead.

If not already defined by another standard library header, include `<stddef.h>`
and not `<stdlib.h>` to define `size_t`.

Additionally, the framework wraps the following standard library header files:
`<stdalign.h>`, `<stdnoreturn.h>`, `<assert.h>` and `<errno.h>`. These header
files must not be directly included in module code, as extended alternatives are
provided by the framework.

Header Files
------------
The contents of a header file should be wrapped in an 'include guard' to prevent
accidental multiple inclusion of a single header. The definition name should be
the upper-case file name followed by "_H". An example for fwk_mm.h follows:

```c
#ifndef FWK_MM_H
#define FWK_MM_H

(...)

#endif /* FWK_MM_H */
```

The closing endif statement should be followed directly by a single-line comment
which replicates the full guard name. In long files this helps to clarify what
is being closed.

Space between definition inside the header file should be a single line only.

If a unit (header or C file) requires a header, it must include that header
instead of relying on an indirect inclusion from one of the headers it already
includes.

Types
-----
Import `<stdint.h>` (part of the C Standard Library) for exact-width integer
types (`uint8_t`, `uint16_t`, etc). These types can be used wherever the width
of an integer needs to be specified explicitly.

Use `uintptr_t` to handle addresses as integers.

Import `<stdbool.h>` (also part of the C Standard Library) whenever a "boolean"
type is needed.

Avoid defining custom types with the "typedef" keyword where possible.
Structures (struct) and enumerators (enum) should be declared and used with
their respective keyword identifiers. If custom types are used then they must
have the suffix "_t" appended to their type name where it is defined. This makes
it easier to recognize types that have been defined using "typedef" when they
appear in the code.

When using sizeof() pass the variable name as the parameter to be evaluated, and
not its type. This prevents issues arising if the type of the variable changes
but the sizeof() parameter is not updated.

```c
size_t size;
unsigned int counter;

/* Preferred over sizeof(int) */
size = sizeof(counter);
```

Use the `const` type qualifier as appropriate to specify values that cannot be
modified. Quick reminder:

- `const xyz_t object`, `object` is an object of type xyz_t which value cannot
be modified.
- `const xyz_t *ptr` or `xyz_t const *ptr`, `ptr` is a pointer to a constant
object of type xyz_t. The value of the object cannot be modified, the value of
the pointer can be modified.
- `xyz_t * const ptr`, `ptr` is a constant pointer to an object of type xyz_t.
The value of the object can be modified, the value of the pointer cannot be
modified.
- `const xyz_t * const ptr` or `xyz_t const * const ptr`, `ptr` is a constant
pointer to a constant object of type xyz_t. The value of the object and the
pointer cannot be modified.

https://cdecl.org may help if in doubt.

Static storage qualifier
------------------------
Declare functions and variables private to a C file as static.

Operator Precedence
-------------------
Do not rely on the implicit precedence and associativity of C operators. Use
parenthesis to make precedence and associativity explicit:

```c
if ((a == 'a') || (x == 'x')) {
    do_something();
}
```

Parenthesis around a unary operator and its operand may be omitted:

```c
if (!a || &a) {
    do_something();
}
```

Macros and Constants
--------------------
Logical groupings of constants should be defined as enumerations, with a common
prefix, so that they can be used as parameter types. To find out the number of
items in an "enum", make the last entry to be \<prefix\>_COUNT.

```c
enum command_id {
    COMMAND_ID_VERSION,
    COMMAND_ID_PING,
    COMMAND_ID_EXIT,
    /* Do not add entries after this line */
    COMMAND_ID_COUNT
};

void process_cmd(enum command_id id)
{
    (...)
}
```

Prefer inline functions instead of macros.

Initialization
--------------
When local variables require being initialized to 0, please use their respective
type related initializer value:
- 0 (zero) for integers
- 0.0 for float/double
- \0 for chars
- NULL for pointers
- false for booleans (stdbool.h)

Array and structure initialization should use designated initializers. These
allow elements to be initialized using array indexes or structure field names
and without a fixed ordering.

Array initialization example:
```c
uint32_t clock_frequencies[3] = {
    [2] = 800,
    [0] = 675
};
```

Structure initialization example:
```c
struct clock clock_cpu = {
    .name = "CPU",
    .frequency = 800,
};
```

Integers
--------
To mitigate the risk of integer wrap-around, conversion or truncation errors:

- represent integer values that should never be negative with the `unsigned`
type that you expect to hold all possible values.

- represent integer values that may be negative with the `signed` type that you
expect to hold all possible values.

- when taking untrusted integer input, ensure you check them against the lower
and upper bound of the integer type you store them in.

Device Register Definitions
---------------------------
The format for structures representing memory-mapped device registers is
standardized.

- The file containing the device structure must include `<stdint.h>` to gain
access to the uintxx_t and UINTxx_C definitions.
- The file containing the device structure must include `<fwk_macros.h>` to
gain access to the FWK_R, FWK_W and FWK_RW macros.
- All non-reserved structure fields must be prefixed with one of the above
macros, defining the read/write access level.
- Avoid C structure bit-fields when representing hardware registers - how
bit-fields are represented in memory is implementation-defined.
- Bit definitions should be declared via UINTxx_C macros.
- Bit definitions must be prefixed by the register name it relates to. If bit
definitions apply to multiple registers, then the name must be as common as
possible and a comment must explicit show which registers it applies to.
- The structure name for the programmer's view must follow the pattern "struct
<device_name>_reg { ...registers... };"

```c
#include <stdint.h>
#include <fwk_macros.h>

struct devicename_reg {
    /* Readable and writable register */
    FWK_RW uint32_t CONFIG;
           uint32_t RESERVED1;

    /* Write-only register */
    FWK_W  uint32_t IRQ_CLEAR;

    /* Read-only register */
    FWK_R  uint32_t IRQ_STATUS;
           uint32_t RESERVED2[0x40];
};

/* Register bit definitions */
#define CONFIG_ENABLE UINT32_C(0x00000001)
#define CONFIG_SLEEP UINT32_C(0x00000002)

#define IRQ_STATUS_ALERT UINT32_C(0x00000001)
```

__Note:__ A template file can be found in doc/template/device.h
