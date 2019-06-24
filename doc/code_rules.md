Coding Rules
============

To maintain consistency within the SCP/MCP software source code a series of
rules and guidelines have been created; these form the project's coding rules.

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
Import "stdint.h" (part of the C Standard Library) for exact-width integer types
(uint8_t, uint16_t, etc). These types can be used wherever the width of an
integer needs to be specified explicitly.

Import "stdbool.h" (also part of the C Standard Library) whenever a "boolean"
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

Device Register Definitions
---------------------------
The format for structures representing memory-mapped device registers is
standardized.

- The file containing the device structure must include stdint.h to gain access
to the uintxx_t and UINTxx_C definitions.
- The file containing the device structure must include fwk_macros.h to gain
access to the FWK_R, FWK_W and FWK_RW macros.
- All non-reserved structure fields must be prefixed with one of the above
macros, defining the read/write access level.
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
