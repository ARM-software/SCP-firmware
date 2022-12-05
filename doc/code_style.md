# Coding Style

To maintain consistency within the SCP/MCP software source code a series of
rules and guidelines have been created; these form the project's coding style.

## Style

### Unicode

The source code must use the UTF-8 encoding. Comments, documentation and strings
may use non-ASCII characters when required (e.g. Greek letters used for units).

### License Headers

All files must begin with a license header of the following form, where first
year describes the year the file was first upstreamed, and the second year the
year when the file was last updated:

Arm SCP/MCP Software
Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.

SPDX-License-Identifier: BSD-3-Clause

### Clang Format

To aid in establishing a uniform style across the code-base, this project uses
Clang Format. When contributing patches, care should be taken to ensure that
code has been formatted according to the rules laid down in the .clang-format
file, and that deviations occur only where the tool is unable to format code
reasonably.

You can automatically format any staged changes with:

```sh
git clang-format
```

One common deviation is found when defining a list of harware registers, for
example:

```c
struct some_reg {
             uint8_t   RESERVED0[0x820 - 0x818];
     FWK_RW  uint32_t  REGISTER0;
     FWK_RW  uint32_t  REGIRTER1;
};
```

The above indentation, although easy to read, will fail the formatting style.
To preserve the legacy format and allow silent operation of the tool, only such
definitions may be preceded and followed by the following special comments:

```c
// clang-format off
struct some_reg {
             uint8_t   RESERVED0[0x820 - 0x818];
     FWK_RW  uint32_t  REGISTER0;
     FWK_RW  uint32_t  REGIRTER1;
};
// clang-format on
```

### Braces

Conditional statements and iteration statements with a single line or
multiple lines of code must be surrounded by braces:

```c
if (condition) {
    function_call(
        long_variable_name_x,
        long_variable_name_y,
        long_variable_name_z);
}
```

This rule applies also to an if-else chain:

```c
if (condition) {
    function_call(
        long_variable_name_x,
        long_variable_name_y,
        long_variable_name_z);
} else {
    function_call_b();
}
```

Empty loop statements must use `continue`:

```c
while (condition) {
  continue;
}
```

### Operators

When using operators like `sizeof` and `alignof`, where possible use the
value-based version over the type-based version:

```c
int counter;

sizeof(counter); /* Preferred over sizeof(int) */
```

### Operator Precedence

Do not rely on the implicit precedence and associativity of operators. Use
parentheses to make precedence and associativity explicit:

```c
if ((a == 'a') || (x == 'x')) {
    do_something();
}
```

Parentheses around a unary operator and its operand may be omitted:

```c
if (!a && *b) {
    do_something();
}
```

## Conventions

### Header Guards

All headers must be wrapped with include guards to prevent accidental multiple
definitions of header contents. The definition name should be the upper-case
file name followed by `_H`. An example for a file named `fwk_mm.h` follows:

```c
#ifndef FWK_MM_H
#define FWK_MM_H

(...)

#endif /* FWK_MM_H */
```

### Inclusion Policy

The closing `endif` statement should be followed directly by a single-line
comment which replicates the full guard name. In long files this helps to
clarify what is being closed.

*Public* headers must be included directly *except* in the following situations:

- The header is already included by the counterpart to the current C file (e.g.
  `x/src/mod_x.c` does not need to include `<stdint.h>` if `x/include/mod_x.h`
  includes it)
- The header is an internal header and is included as part of another public
  header (e.g. `x/src/mod_x.c` does not need to include `x/include/internal/x.h`
  if another header includes it)

These rules do not apply to *private* headers (headers under a `src/`
directory).

### Macros and Constants

All macro and constant names must be written in upper-case to differentiate
them from functions and non-constants.

Logical groupings of constants should be defined as enumerations with a common
prefix, so that they can be used as parameter types and in constant expressions.
Enumerations that contain a "number of elements" value must use the `_COUNT`
suffix, and the final enumeration value should include a comma.

```c
enum command_id {
    COMMAND_ID_VERSION,
    COMMAND_ID_PING,
    COMMAND_ID_EXIT,
    COMMAND_ID_COUNT,
};

void process_cmd(enum command_id id)
{
    (...)
}
```

### Symbol Naming

Function, variable, file name and type names must:

- Be written in lower-case
- Have compound words separated by underline characters (`_`)
- Have descriptive names, avoiding contractions where possible (e.g.
    `cluster_count` instead of `clus_cnt`)

Avoid using:

- Camel case syntax (e.g. `cssClusterCount`)
- Hungarian notation, encoding types within names (e.g. `int iSize`)

The following prefixes are reserved for these components:

- Architecture: `arch_`
- Framework: `fwk_`
- Module: `mod_`
  - Module description: `module_`
  - Module configuration: `config_`
- Firmware: `fmw_`

### Assertions

Use `fwk_assert()` to mark *invariants*. Invariants are logical assertions about
the behaviour of the system which must hold true for the program to be valid.
Invariants are usually easy to communicate to human readers, but not very easy
to communicate to the compiler.

Use `fwk_expect()` to mark *expectations*. Expectations may fail, and they may
or may not be handled, but they are not invariants in that they may reasonably
occur at some point in time.

### Loop Indices

It is acceptable to use the following common placeholder names for loop indices:

- `i`
- `j`
- `k`

### Types

Avoid defining custom types with the `typedef` keyword where possible. Names
defined with `typedef` must use the `_t` suffix.

### Comments

To ensure a consistent look, the preferred style for single-line comments is to
use the C89 style of paired forward-slashes and asterisks, ending with no
punctuation:

```c
/* A short, single-line comment */
```

Multi-line comments should follow a similar style, and always end with
punctuation:

```c
/*
 * This is a very, very, very, very long multi-line comment where each line
 * starts with an asterisk and paragraphs ends with punctation.
 */
```

Doxygen comments follow only the second style, for consistency in the generated
documentation:

```c
/*!
 * \brief Hello, World.
 */
```

Preprocessor `if 0` is preferred for commenting out blocks of code where it is
necessary to do so, as these can be nested (unlike multi-line comment blocks).

```c
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
```

Prefer functions to function-like macros where possible. Avoid using the
`inline` keyword, as inlining sites are better-determined by the compiler.

### Initialization

When literals require explicit literal initialization, avoid implicit casts by
using the proper integer literal suffix or initializer:

```c
void *p = NULL;
unsigned int u = 0u;
float f = 0.0f;
double d = 0.0;
char c = '\0';
```

Array and structure initialization should use designated initializers. These
allow elements to be initialized using array indices or structure field names
without a fixed ordering:

```c
uint32_t clock_frequencies[3] = {
    [2] = 800,
    [0] = 675,
};

struct clock clock_cpu = {
    .name = "CPU",
    .frequency = clock_frequencies[2],
};
```

### Device Register Definitions

The format for structures representing memory-mapped device registers is
standardized:

- Register structure names are suffixed with `_reg`
- All register names must be capitalized
- Non-reserved registers must be one of `FWK_R`, `FWK_W`, or `FWK_RW`
- Bit-field masks and shifts use the `<DEVICE>_<REGISTER>_<FIELD>` name,
  suffixed with either `_MASK` or `_POS`

```c
#include <stdint.h>
#include <fwk_macros.h>

struct my_timer_reg {
    FWK_RW uint32_t CONFIG;
           uint32_t RESERVED1;
    FWK_W  uint32_t IRQ_CLEAR;
    FWK_R  uint32_t IRQ_STATUS;
           uint8_t  RESERVED2[0x40];
};

#define MY_TIMER_CONFIG_ENABLE UINT32_C(0x00000001)
#define MY_TIMER_CONFIG_SLEEP UINT32_C(0x00000002)

#define MY_TIMER_IRQ_STATUS_ALERT UINT32_C(0x00000001)
```

**Note:** A template file can be found in doc/template/device.h

### Doxygen Comments

The project APIs are documented using Doxygen comments.

It is mandatory to document every API exposed to other elements of the project.
By default, the provided Doxygen configuration omits undocumented elements from
the compiled documentation.

At a minimum:

- All functions and structures must have at least a `brief` tag
- All functions must document their parameters (if any) with the `param` tag
  - Input parameters must use `param[in]`
  - Output parameters must use `param[out]`
  - Input/output parameters must use `param[in, out]`
- All functions returning a value should use the `return` and, optionally, the
  `retval` tag to document their return value
  - `return` should document what the *type* of the return value represents
  - `retval` should document what *individual return values* represent

Alignment and indentation:

- Documentation must also obey the 80 column limit
- Multiple lines of documentation on an entry (e.g. details) must be indented
  using the equivalent of two 4-space based tabs from the first column (see
  example below)

Function documentation example:

```c
/*!
 * \brief Enable the watchdog.
 *
 * \details This function enables the watchdog. If mod_wdog_set_interval() has
 *      not been called beforehand then the watchdog defaults to a 500
 *      millisecond timeout period.
 */
void mod_wdog_enable(void);
```

Structure documentation example:

```c
/*!
 * \brief A structure example.
 *
 * \details This is an example of a structure for the code style documentation.
 */
struct mod_structure {
    /*!
     * \brief Example field.
     */
    unsigned int example;
};
```

## Python

Python based tools must follow the [PEP8][pep8] specification. This is enforced
with the `pycodestyle` tool.

Python code must be linted with `pylint`.

[pep8]: https://www.python.org/dev/peps/pep-0008
