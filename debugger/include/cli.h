/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CLI_H_
#define _CLI_H_

/*****************************************************************************/
/* Include Files                                                             */
/*****************************************************************************/

#include "cli_config.h"
#include "cli_fifo.h"

#include <stdbool.h>
#include <stdint.h>

/*****************************************************************************/
/* Enumerated Types                                                          */
/*****************************************************************************/

/*
 * cli_state_et
 *   Description
 *     Describes the current state of the CLI.
 *   Members
 *     CLI_NOT_READY
 *       CLI has not been initialized and is not running.
 *     CLI_READY
 *       Data structures have been initialized, threads can register and print,
 *       but print operations will be slow as they will have to use the UART
 *       hardware.
 *     CLI_RUNNING
 *       The CLI thread is up and running, threads can register and print, and
 *       print operations will be fast as they will write into a FIFO buffer
 *       rather than deal with the UART hardware.
 */
typedef enum {
    CLI_NOT_READY = 0x0,
    CLI_READY = 0x1,
    CLI_RUNNING = 0x2
} cli_state_et;

/* Masks for formatting options. */
#define CLI_RESET_MASK (0x00000003)
#define CLI_FORMAT_MASK (0x0000FFFC)
#define CLI_BG_COLOR_MASK (0x00FF0000)
#define CLI_TEXT_COLOR_MASK (0xFF000000)
#define CLI_ALL_MASK (CLI_FORMAT_MASK | CLI_BG_COLOR_MASK | CLI_TEXT_COLOR_MASK)
#define CLI_BG_COLOR_SHIFT (16)
#define CLI_TEXT_COLOR_SHIFT (24)

/*
 * cli_option_et
 *   Description
 *     These values are used apply special formatting to printed text in
 *     cli_client_printf.  Options can be OR'd together to apply more than one
 *     at a time.  You cannot use multiple background colors and multiple text
 *     colors together though, but a single background and a single text color
 *     is just fine.
 *   Members
 *     NONE
 *       Default terminal formatting, same as using 0.
 *     CLEAR_DISPLAY
 *       Deletes everything on the display before printing.
 *     RESET_CURSOR
 *       Moves the cursor to the top left position (1,1).
 *     BOLD
 *       Makes text bold or "bright" depending on the terminal application.
 *     UNDERLINE
 *       Underlines text.
 *     BG_<color>
 *       Text background color, only one of these can be used at a time.
 *     TEXT_<color>
 *       Text color, only one of these can be used at a time.
 */
typedef enum {
    NONE = 0x00000000,
    CLEAR_DISPLAY = 0x00000001,
    RESET_CURSOR = 0x00000002,
    BOLD = 0x00000004,
    UNDERLINE = 0x00000008,
    BG_BLACK = 0x001E0000,
    BG_RED = 0x001F0000,
    BG_GREEN = 0x00200000,
    BG_YELLOW = 0x00210000,
    BG_BLUE = 0x00220000,
    BG_MAGENTA = 0x00230000,
    BG_CYAN = 0x00240000,
    BG_WHITE = 0x00250000,
    TEXT_BLACK = 0x1E000000,
    TEXT_RED = 0x1F000000,
    TEXT_GREEN = 0x20000000,
    TEXT_YELLOW = 0x21000000,
    TEXT_BLUE = 0x22000000,
    TEXT_MAGENTA = 0x23000000,
    TEXT_CYAN = 0x24000000,
    TEXT_WHITE = 0x25000000
} cli_option_et;

/*****************************************************************************/
/* Structure Types                                                           */
/*****************************************************************************/

/*!
 * \brief CLI commands structure cli_command_st.
 *
 * \details Structure that stores a CLI command, it includes a help string so
 *      help functionality is uniform across commands as well as a string used
 *      to define the command used to invoke the stored function pointer.
 */
typedef struct {
    /*! Pointer to string that is typed to invoke a command. */
    const char *command;
    /*! Pointer to string containing help information for a command. */
    const char *help;
    /*! Pointer to function that is invoked for this command. */
    int32_t (*handler)(int32_t argc, char **argv);
    /*!
     * For complex commands that need more than a single help string, set
     * this to true.  This will keep the CLI from intercepting the --help and
     * -h flags and will instead pass them to the command along with the rest
     * of the arguments.
     */
    bool ignore_help_flag;
} cli_command_st;

/*****************************************************************************/
/* Public Function Prototypes                                                */
/*****************************************************************************/

/*
 * cli_init
 *   Description
 *     Public function used to initialize the CLI data structures but not start
 *     the CLI main thread.  This allows other threads to register with the CLI
 *     before the CLI idle thread has actually been created.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t cli_init(void);

/*
 * cli_start
 *   Description
 *     Public function used to create the CLI thread after it's data structures
 *     have been initialized.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t cli_start(void);

/*
 * cli_bprintf
 *   Description
 *     Buffered formatted print function.  Prints generated here are put into a
 *     memory buffer and sent to the UART during CPU idle time rather than
 *     waiting for the UART to accept all the data.  This allows debug print
 *     statements to be very minimally intrusive.
 *   Parameters
 *     cli_option_et
 *       Text formatting options, see definition of cli_option_et above.
 *     const char *format
 *       Formatted text to be printed, like printf.
 *     ...
 *       Additional arguments for formatted text, like printf.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t cli_bprintf(cli_option_et options, const char *format, ...);

/*
 * cli_bprint
 *   Description
 *     Buffered print function that just sends a string directly to the print
 *     buffer with no special formatting.
 *   Parameters
 *     const char *string
 *       Null-terminated string to be printed.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t cli_bprint(const char *format);

/*
 * cli_printf
 *   Description
 *     Prints directly to the debug console without waiting for the CLI thread
 *     to pick it up.  Do not use this in a thread except in the case of dire
 *     errors or information that needs to be printed immediately.
 *   Paremeters
 *     cli_option_et
 *       Text formatting options, see definition of cli_option_et above.
 *     const char *format
 *       Formatted text to be printed, like printf.
 *     ...
 *       Additional arguments for formatted text, like printf.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t cli_printf(cli_option_et options, const char *format, ...);

/*
 * cli_print
 *   Description
 *     Sends a string directly to the debug terminal with no special formatting
 *     or extra processing.
 *   Parameters
 *     const char *string
 *       Null-terminated string to be printed.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t cli_print(const char *string);

/*
 * cli_format_print
 *   Description
 *     Rudimentary formatted print function that takes the place of snprintf and
 *     it's variants.
 *   Parameters
 *     char *s
 *       Buffer to build new string in.
 *     char *smax
 *       s + len(s), points to address after the last character in s.  This
 *       keeps the function from exceeding the boundaries of the array.
 *     const char *fmt
 *       Formatted text to parse.
 *     ...
 *       Arguments for formatted text.
 */
void cli_snprintf(char *s, char *smax, const char *fmt, ...);

/*
 * cli_getline
 *   Description
 *     Retrieves input from terminal while running a CLI command.
 *   Parameters
 *     char *buffer
 *       Buffer used to store typed characters.
 *     uint32_t buffer_size
 *       Size of the buffer, determines how many characters you can type.
 *     char **argbuf
 *       If not NULL, the typed string will be split up into individual
 *       arguments.  In this case, buffer will be unusable as the parser
 *       replaces whitespace with null characters and creates links in argbuf to
 *       each argument.
 *     uint32_t argbuf_size
 *       Size of the argbuf array, determines maximum number of individual
 *       arguments that can be entered.  Ignored if argbuf is NULL.
 *     uint32_t cursor_position
 *       Tells the CLI how far advanced the cursor is on a line at the time, if
 *       in doubt, print a newline then set to 0.
 *   Return
 *     Platform return codes defined in cli_config.h.
 */
uint32_t cli_getline(
    char *buffer,
    uint32_t buffer_size,
    char **argbuf,
    uint32_t argbuf_size,
    uint32_t cursor_position);

/*
 * TBA: Replace calls to strncmp with calls to cli_strncmp.
 * For some reason calls to strncmp crash for seemingly no reason on the
 * FPGA, so we'll use cli_strncmp until that gets fixed or we move over to
 * silicon.
 */
int32_t cli_strncmp(const char *s1, const char *s2, uint32_t limit);

#ifdef BUILD_HAS_DEBUGGER

/*!
 * \brief Register a new CLI command at run time
 *
 * \param new_cmd The new command to register.
 *
 * \retval CLI_SUCCESS Operation succeeded.
 * \retval CLI_ERR_MEM Not enough memory.
 */
int cli_command_register(cli_command_st new_cmd);

#else

/*!
 * \brief Register a new CLI command at run time
 *
 * \param new_cmd The new command to register.
 *
 * \retval CLI_SUCCESS Operation succeeded.
 * \retval CLI_ERR_MEM Not enough memory.
 */
#define cli_command_register(new_cmd) \
    __extension__({ \
        (void)new_cmd; \
        FWK_SUCCESS; \
    })
#endif

#endif /* _CLI_H_ */
