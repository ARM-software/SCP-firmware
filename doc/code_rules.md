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

The framework provides an optional multi-threading runtime, contingent on a
CMSIS-RTOS-based RTOS. Interaction with the RTOS happens exclusively through the
framework.

Static Analysis
---------------
This project uses `cppcheck` to do a static analysis of the codebase for
possible programmer error and other sources of bugs. This analysis occurs as a
part of the standard continuous integration flow on pull requests. If the
analyser identifies potential issues, the patch will be automatically rejected.

To invoke `cppcheck` manually, use:

\code
cppcheck --xml \
         --enable=all \
         --force \
         --suppressions-list="${WORKDIR}/tools/cppcheck_suppress_list.txt" \
         --includes-file="${WORKDIR}/framework/include/" \
         -I "${WORKDIR}/CMSIS" \
         -i cmsis \
         -U__CSMC__ \
         ${WORKDIR}
\endcode

The CMSIS submodule is not checked. Errors are printed in XML format on the
error output.

Certain checks have been manually suppressed. Checks are generally only
disabled if they have been triaged as false positives. The list of suppressed
checks can be found [here](../tools/cppcheck_suppress_list.txt).

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
files must not be directly included in module code. This is because certain
compilers, while themselves C11-compliant, do not provide a full C11 standard
library implementation. In this situation, the framework provides a custom
implementation through these headers.

Header Files
------------
The contents of a header file should be wrapped in an 'include guard' to prevent
accidental multiple inclusion of a single header. The definition name should be
the upper-case file name followed by "_H". An example for fwk_mm.h follows:

\code
#ifndef FWK_MM_H
#define FWK_MM_H

(...)

#endif /* FWK_MM_H */
\endcode

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

\code
size_t size;
unsigned int counter;

/* Preferred over sizeof(int) */
size = sizeof(counter);
\endcode

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

\code
if ((a == 'a') || (x == 'x'))
    do_something();
\endcode

Parenthesis around a unary operator and its operand may be omitted:

\code
if (!a || &a)
    do_something();
\endcode

Macros and Constants
--------------------
Logical groupings of constants should be defined as enumerations, with a common
prefix, so that they can be used as parameter types. To find out the number of
items in an "enum", make the last entry to be \<prefix\>_COUNT.

\code
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
\endcode

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
\code
uint32_t clock_frequencies[3] = {
    [2] = 800,
    [0] = 675
};
\endcode

Structure initialization example:
\code
struct clock clock_cpu = {
    .name = "CPU",
    .frequency = 800,
};
\endcode

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

\code
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
\endcode

__Note:__ A template file can be found in doc/template/device.h
