Coding Style
============

A coding style has been defined to give a consistent look to the source code and
thus help code readability and review.

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

Non-static names should be prefixed with the name of their translation unit to
avoid name collisions.

It is acceptable to use the following common placeholder names for loop indices:
 - `i`
 - `j`
 - `k`

`xyz_idx` names are commonly used for indices that live longer than a single
loop.

License
-------
All files must begin with a license header of the following form:

Arm SCP/MCP Software
Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.

SPDX-License-Identifier: BSD-3-Clause

Inclusions
----------
Header file inclusions should follow a consistent sequence, defined by scope
from local to global:

- Private firmware headers (`"config_scmi.h"`)
- Public non-project headers (`<cmsis.h>`)
- Private module headers (`<internal/scmi.h>)
- Public module headers (`<mod_scmi.h>)
- Public framework headers (`<fwk_list.h>`)
- Public architecture headers (`<arch_exceptions.h>`)
- Public firmware headers (`<fmw_cmsis.h>`)
- Public standard library headers (`<stddef.h>`)

For each group, order the individual headers alphabetically and separate the
blocks logically.

Header files (`.h` files) should include the headers needed for them to compile
and only these ones.

Translation units (`.c` files) should not rely on indirect inclusion to provide
names they have otherwise not included themselves.

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

Doxygen Comments
----------------
The project APIs are documented using Doxygen comments.

It is mandatory to document every API exposed to other elements of the project.
By default, the provided Doxygen configuration omits undocumented elements from
the compiled documentation. APIs are documented in the header files exposing
them.

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
