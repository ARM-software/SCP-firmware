/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

Description of Files

  cli_commands_core.c
    Contains all code implementing core executable commands for the debug
    console. If you wish to add new commands, create a new command structure and
    add links to the command functions in the structure then call the command
    registration function cli_command_register.

  cli_config.h
    Definitions of compile-time command line interface settings, such as buffer
    sizes, default prompt text, platform return codes, etc.

  cli_fifo.c
    Function definitions for creating and accessing a FIFO buffer.  Used to
    store characters from the keyboard before they can be read by the CLI
    thread as well as acting as a print buffer to store characters before
    they can be sent to the UART.

  cli_fifo.h
    Header file containing function prototypes and descriptions for cli_fifo.c.

  cli_platform.h
    Function prototypes and descriptions for platform-specific functions.  If
    you wish to port this CLI, start by reading this file and then implement
    the functions in the platform directory.

  cli.c
    Core CLI function definitions.  Contains both public and private CLI
    functions.  Private function prototypes and descriptions are also here.

  cli.h
    CLI enumerated types, structure types, and public function prototypes and
    descriptions.  To access CLI functionality from other files, you only
    need to include this file.

Using the Console

  There are two mechanisms to enter the console. At compile time using
  checkpoints and at run time using Ctrl+E press. To exit the console, Ctrl+D
  should be pressed. Note that CTRL+E only works once the modules have been
  started, as the module monitoring the keystrokes starts last.
  Once inside the console, all dequeuing of events in the queue is blocked. The
  framework will resume handling events after exiting the console.
  By default the CLI is in command mode, to exit command mode and enter debug,
  mode press Ctrl+C. To return to command mode, press Ctrl+C again.  Know that,
  while in command mode, the print buffers can fill up quickly.
  Once this happens, all debug data from the time the print buffers fill up to
  the time you reenter debug mode and the CLI can empty the buffers will be lost
  If a buffer fills up, you will see a message like "CONSOLE ERROR: Print buffer
  overflow."

Printing From Other Threads

  cli_printf and cli_bprintf are formatted printing functions, and cli_print and
  cli_bprint are non-formatted basic printing functions.  The cli_bprint
  functions are buffered, meaning print data is placed into a FIFO buffer rather
  than immediately put on the UART.  This allows the print functions to return
  very quickly rather than having to wait on the UART, but prints are delayed
  until the processor has time to print them.

  Note that the maximum size of individual print text is limited by the
  size of the scratch buffer (defined in cli_config.h) and how much space your
  print buffer has.  Text style options take up more space still.

  Also, to use this CLI to print from a thread will require enough extra stack
  space to allocate the scratch buffer.

CLI formatted print options

  This CLI is not 100% compliant with typical C formatted print strings, it is
  based on a very lightweight but rudimentary version of snprintf, so some fancy
  formatting options will not work as intended. (or at all)  Basic specifiers
  supported are listed below.

    Specifier   Output                                               Example
      d/i         Signed decimal integer                               -438
      u           Unsigned decimal integer                             2841
      x/X         Uppercase hex value (lower case not implemented)     4AB1
      c           Single character                                     e
      s           Null-terminated string of characters                 example
      l           Used within a specifier, indicates 64 bit value.
      0<width>    Left-pads with zeros, width is required with this.   00004AB1

  These all work in the simplest use case, but things like right-justifying,
  floating point values, and anything not fairly simple probably won't work so
  make sure to test things first.

  If you want to know exactly how strings are parsed, see the function
  'cli_snprintf_arg' in cli_module.c.
