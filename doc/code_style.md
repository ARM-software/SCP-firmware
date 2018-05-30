Coding Style
============

To maintain consistency within the SCP/MCP software source code a series of
rules and guidelines have been created; these form the project's coding style.

Encoding
--------

The source code must use the UTF-8 encoding. Comments, documentation and strings
may use non-ASCII characters when required (e.g. Greek letters used for units).

Naming
------
Function, variable, file name and type names must:
- Be written in lower-case
- Have compound words separated by underline characters
- Have descriptive names, avoiding contractions where possible
(e.g.cluster_count instead of clus_cnt)

Avoid using:
- Camel case syntax (e.g. cssClusterCount)
- Hungarian notation, encoding types within names (e.g. int iSize)

Functions, macros, types and defines must have the "fwk_" prefix (upper case for
macros and defines) to identify framework API.

It is fine and encouraged to use a variable named "i" (index) for loops.

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

Inclusions
----------
Header file inclusions should follow a consistent sequence, defined as:

- Standard library (stdbool, stdint, etc)
- Framework components (fwk_<component>.h)
- Modules

For each group, order the individual headers alphabetically.

Indentation and Scope
---------------------
Indentation is made of spaces, 4 characters long with each line being at most 80
characters long.
Following K&R style, the open-brace goes on the same line as the statement:

\code
if (x == y) {
    (...)
}
\endcode

The only exception is for functions, which push the opening brace to the
following line:

\code
void function_a(int x, int y)
{
  (...)
}
\endcode

Similarly, the case and default keywords should be aligned with the switch
statement:

\code
switch (option) {
case 1:
    (...)
    break;
default:
    (...)
    break;
}
\endcode

Conditional statements with single line of code should not use braces,
preferring indentation only. A statement that spans multiple lines must use
braces to improve readability:

\code
if (condition_a == true)
    function_call_a();

if (condition_b == true) {
    function_call_b(long_variable_name_x |
                    long_variable_name_y);
}
\endcode

In a chain of if-else statements involving multi-line and single-line blocks,
it is acceptable to mix statements with and without braces:

\code
if (condition == [a]) {
    function_call_a(long_variable_name_x |
                    long_variable_name_y);
} else if (condition == [b])
    function_call_b();
\endcode

Empty loop statements should use "continue" instead of empty braces or single
semi-colon:

\code
while (condition == false)
  continue;
\endcode

Multi-line statements should align on the openning delimiter:

\code
long_variable_name = (long_variable_value << LONG_CONSTANT_POS) &
                      LONG_CONSTANT_MASK;
\endcode

In case the code extends beyond 80 columns, the first line can wrap creating a
new indented block:
\code
                    long_variable_name =
                        (long_variable_value << LONG_CONSTANT_POS) &
                         LONG_CONSTANT_MASK;
\endcode

When a stacked multi-line statement aligns with the next code level, leave a
blank line to highlight the separation:

\code
if (condition_a ||
    condition_b ||
    condition_c) {

    do_something();
}
\endcode

Function definitions should follow the same approach:
\code
int foo(unsigned int param_a,
        unsigned param_b,
        unsigned param_c)
{
    ...
}
\endcode

Preprocessor statements should be aligned with the code they are related to:

\code
#ifdef HAS_FOO
int foo(void)
{
    #ifdef HAS_BAR
    return bar();

    #else
    return -1;

    #endif
}
#endif
\endcode

Where preprocessor statements are nested and they target the same code stream,
indentation is allowed but the hash symbol must be left aligned with the code
stream:

\code
#ifdef HAS_FOO
int foo(void)
{
    #ifdef HAS_BAR
    return bar();

    #else
    #   ifdef DEFAULT_ERROR
    return -1;

    #   else
    return 0

    #   endif
    #endif
}
#endif
\endcode

__Note__ Such constructions like the example above should be avoided if
possible.

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

Comments
--------
To ensure a consistent look, the preferred style for single-line comments is to
use the C89 style of paired forward-slashes and asterisks:

\code
/* A short, single-line comment. */
\endcode

For multi-line comments the same applies, adding an asterisk on each new line:

\code
/*
 * This is a multi-line comment
 * where each line starts with
 * an asterisk.
 */
\endcode

\#if 0 is preferred for commenting out blocks of code where it is necessary to
do so.

\code
void function_a(int x, int y)
{
  (...)
}

#if 0
void function_b(int x, int y)
{
  (...)
}
#endif

\endcode

Macros and Constants
--------------------
All names of macros and constants must be written in upper-case to differentiate
them from functions and variables.

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

Doxygen Comments
----------------
The project APIs are documented using Doxygen comments.

It is mandatory to document every API exposed to other elements of the project.
By default, the provided Doxygen configuration omits undocumented elements from
the compiled documentation.

At a minimum:
- All functions and structures must have at least a "\brief" tag.
- All functions must document their parameters (if any) with the "\param" tag.
- All functions should use the "\return" or "\retval" tags to document their
return value. When the return is void, simply give "None" as the return value.

Alignment and indentation:
- Documentation must also obey the 80 columns limit.
- Multiple lines of documentation on an entry (e.g. details) must be indented
using the equivalent of two 4-space based tabs (see example below).

Function documentation example:
\code
/*!
 * \brief Enable the watchdog.
 *
 * \details This function enables the watchdog. If m_wdog_set_interval() has
 *      not been called beforehand then the watchdog defaults to a 500
 *      millisecond timeout period.
 *
 * \return None.
 */
void m_wdog_enable(void);
\endcode

Structure documentation example:
\code
/*!
 * \brief Queue item
 */
typedef struct _fwk_dlinks_t {
    /*! Pointer to the next item */
    struct _fwk_dlinks_t *next;

    /*! Pointer to the previous item */
    struct _fwk_dlinks_t *prev;
} fwk_dlinks_t;
\endcode

Python based tools
------------------

Python based tools must follow the
[PEP8](https://www.python.org/dev/peps/pep-0008/) specification.
