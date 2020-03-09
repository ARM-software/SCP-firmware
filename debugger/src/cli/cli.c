/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cli.h>
#include <cli_config.h>
#include <cli_fifo.h>
#include <cli_platform.h>

#include <fwk_list.h>
#include <fwk_mm.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* String holding console prompt. */
static char cli_prompt[CLI_CONFIG_PROMPT_BUF_SIZE] = { 0 };
static char cli_prompt_size = 0;

/* Buffer to store a command as it is typed. */
static char cli_input_buffer[CLI_CONFIG_COMMAND_BUF_SIZE] = { 0 };

/* Array holding pointers to arguments after a command is parsed. */
static char *cli_args[CLI_CONFIG_MAX_NUM_ARGUMENTS] = { 0 };

/* Starting history buffer index. */
static uint32_t history_index = 0;
/* Buffer used to store the command history. */
static char *cli_history[CLI_CONFIG_HISTORY_LENGTH] = { 0 };
static char cli_history_buffer
    [CLI_CONFIG_HISTORY_LENGTH * CLI_CONFIG_COMMAND_BUF_SIZE] = { 0 };

/* Default terminal size variables, these are updated at each enter press. */
static uint32_t cli_terminal_width = CLI_CONFIG_DEFAULT_TERM_W;
static uint32_t cli_terminal_height = CLI_CONFIG_DEFAULT_TERM_H;

/* Current state of the CLI. */
volatile cli_state_et cli_state = CLI_NOT_READY;

/* Extern link to buffer holding installed CLI commands.  In cli_commands.h */
extern cli_command_st cli_commands[];

/* Print buffer structures. */
static fifo_st cli_print_fifo = { 0 };
static char cli_print_fifo_buffer[CLI_CONFIG_PRINT_BUFFER_SIZE] = { 0 };
static bool overflow = false;

size_t strnlen(s, maxlen) register const char *s;
size_t maxlen;
{
    register const char *e;
    size_t n;

    for (e = s, n = 0; *e && n < maxlen; e++, n++)
        ;
    return n;
}

struct command_ctx {
    cli_command_st cmd;
    struct fwk_slist_node list_node;
};

/* A linked list used for extend the existing command at run time */
static struct fwk_slist cli_commands_list;

/* This array holds the common command available for all platforms */
extern cli_command_st cli_commands[];

int cli_command_register(cli_command_st new_cmd)
{
    struct fwk_slist *node = NULL;
    struct command_ctx *c = NULL;

    FWK_LIST_FOR_EACH(&cli_commands_list, node,
        struct command_ctx, list_node, c) {
        if (strcmp(new_cmd.command, c->cmd.command) == 0)
            /* Command exist. Skip registration */
            return FWK_SUCCESS;
    }

    c = fwk_mm_calloc(1, sizeof(struct command_ctx));
    if (c == NULL)
        return FWK_E_NOMEM;

    c->cmd = new_cmd;
    fwk_list_push_tail(&cli_commands_list, &c->list_node);

    return FWK_SUCCESS;
}

int cli_command_init(void)
{
    uint32_t index;
    int status;

    fwk_list_init(&cli_commands_list);

    /* Add the common commands to the commands linked list */
    for (index = 0; cli_commands[index].command != 0; index++) {
        status = cli_command_register(cli_commands[index]);
        if (status != FWK_SUCCESS)
            return status;
    }
    return FWK_SUCCESS;
}

/*
 * TBA: Replace calls to strncmp with calls to cli_strncmp.
 * For some reason calls to strncmp crash for seemingly no reason on the
 * FPGA, so we'll use cli_strncmp until that gets fixed or we move over to
 * silicon.
 */
int32_t cli_strncmp(const char *s1, const char *s2, uint32_t limit)
{
    for (uint32_t i = 0; i < limit; i++) {
        if (s1[i] != s2[i])
            /*
             * If first string greater than second, return positive, if second
             * string greater than first, return negative.
             */
            return s1[i] - s2[i];
        else if (s1[i] == 0)
            /*
             * We know characters are equal, so if one of them is equal to zero
             * both are, so return zero to indicate a match.
             */
            return 0;
    }

    /* Limit reached, return 0 as first string and second string match. */
    /* up to the required character limit.                              */
    return 0;
}

/*****************************************************************************/
/* Private function prototypes.                                              */
/*****************************************************************************/

/*
 * cli_main
 *   Description
 *     Main loop for CLI thread.  Handles receiving and dispatching commands.
 *   Parameters
 *     void const *argument
 *       Argument passed at initialization, not used.
 */
static void cli_main(void const *argument);

/*
 * cli_format
 *   Description
 *     Sends formatting escape sequences to the terminal or writes them into a
 *     string depending on how it's used.
 *   Parameters
 *     cli_option_et options
 *       Formatting options to use.
 *     char *buffer
 *       If this value is not null, escape sequences are placed into this
 *       buffer instead of being sent to the terminal immediately.
 *   Return
 *     uint32_t: FWK_SUCCESS if it works, something else if it
 *     doesn't.
 */
static uint32_t cli_format(cli_option_et options, char *buffer, uint32_t size);

/*
 * cli_get_command
 *   Description
 *     Function called in cli_main that receives bytes from the UART,
 *     echos them back, and deals with backspace, escape sequences, etc.
 *     It returns the received command when the enter key is pressed.
 *   Parameters
 *     char *buffer
 *       Buffer in which the received command is placed.
 *     uint32_t buffer_size
 *       Total size, in bytes, of the command buffer.
 *     uint32_t *command_length
 *       The length of the command from the terminal is placed in this pointer.
 *     char history[][]
 *       Pointer to command history buffer.
 *     uint32_t history_index
 *       Index of the most recent item in the history buffer.
 *     uint32_t history_size
 *       Size of command history buffer.
 *     bool *exit
 *       Exit CLI was requested.
 *   Return
 *     uint32_t: FWK_SUCCESS if it works, something else if it
 *     doesn't.
 */
static uint32_t cli_get_command(
    char *buffer,
    uint32_t buffer_size,
    uint32_t *command_length,
    char **history,
    uint32_t history_index,
    uint32_t history_size,
    bool *exit);

/*
 * cli_backspace
 *   Description
 *     Handles backspace characters from keyboard and makes sure lines wrap
 *     properly.
 *   Parameters
 *     uint32_t *cursor
 *       Pointer to cursor position variable.
 *     uint32_t width
 *       Width of terminal window.
 *   Return
 *     uint32_t: FWK_SUCCESS if it works, something else if it
 *     doesn't.
 */
static uint32_t cli_backspace(uint32_t *cursor, uint32_t width);

/*
 * cli_split
 *   Description
 *     Takes the command buffer, replaces whitespace characters with null
 *     characters, and builds an array of pointers to each individual argument.
 *     Arguments surrounded by parenthesis are taken verbatim, others are split
 *     up by whitespace.
 *   Parameters
 *     char **argbuf
 *       Buffer in which pointers to individual arguments are placed.  Null
 *       terminated.
 *     uint32_t argbuf_size
 *       Number of entries in argbuf, can be configured with the
 *       MAX_NUM_ARGUMENTS macro above.
 *     char *command
 *       Command string to be parsed.
 *     uint32_t command_length
 *       Length of the command.
 *     char *whitespace
 *       Null-terminated string of characters to consider whitespace.
 *   Return
 *     uint32_t: FWK_SUCCESS if it works, something else if it
 *     doesn't.
 */
static uint32_t cli_split(
    char **argbuf,
    uint32_t argbuf_size,
    char *command,
    uint32_t command_length,
    const char *whitespace);

/*
 * cli_command_dispatch
 *   Description
 *     Takes the argument list provided by cli_split and determines which
 *     command to execute.
 *   Parameters
 *     char **args
 *       Pointer to an array of strings containing the arguments received from
 *       the terminal.
 *   Return
 *     uint32_t: FWK_SUCCESS if it works, something else if it
 *     doesn't.
 */
static uint32_t cli_command_dispatch(char **args);

/*
 * cli_debug_output
 *   Description
 *     Handles the debug printout mode of the CLI.  It runs until a Ctrl+C
 *     press is received.
 *   Return
 *     uint32_t: FWK_SUCCESS if it works, something else if it
 *     doesn't.
 */
static uint32_t cli_debug_output(void);

/*
 * cli_error_handler
 *   Description
 *     Prints information about cli_ret_et return values.
 *   Parameters
 *     cli_ret_et status
 *       Error code to print information about.
 */
static void cli_error_handler(uint32_t status);

/*
 * cli_val2str
 *   Description
 *     Converts a number variable to a string representation of the value.
 *   Parameters
 *     char **outstr
 *       Pointer to string in which to put the result.
 *     char *smax
 *       Pointer to the address after the last byte in outstr, prevents
 *       a buffer overflow.
 *     uint32_t value
 *       Value to convert to a string.
 *     uint32_t base
 *       Base of the result, usually base 2, 10, or 16.
 *     int32_t fill
 *       How many characters you want the result to take up.
 */
static void cli_val2str(
    char **outstr,
    char *smax,
    uint32_t value,
    uint32_t base,
    int32_t fill);

/*
 * cli_snprintf_arg
 *   Description
 *     Same as cli_snprintf, but uses va_list arguments.
 *   Parameters
 *     char *s
 *       Buffer to build new string in.
 *     char *smax
 *       s + len(s), points to address after the last character in s.  This
 *       keeps the function from exceeding the boundaries of the array.
 *     const char *fmt
 *       Formatted text to parse.
 *     va_list *args
 *       Pointer to initialized va_list structure.
 */
static void cli_snprintf_arg(
    char *s,
    char *smax,
    const char *fmt,
    va_list *args);

volatile uint32_t automation_mode = 0;

/*****************************************************************************/
/* Public Function Definitions (see cli.h for descriptions)                  */
/*****************************************************************************/

uint32_t cli_init(void)
{
    int32_t status = FWK_SUCCESS;
    char *prompt = CLI_PROMPT;

    /* This function can only run when current state is CLI_NOT_READY. */
    if (cli_state != CLI_NOT_READY)
        return FWK_E_STATE;

    /* Store CLI prompt text. */
    if (strlen(prompt) > (CLI_CONFIG_PROMPT_BUF_SIZE - 1))
        return FWK_E_NOMEM;

    strncpy(cli_prompt, prompt, CLI_CONFIG_PROMPT_BUF_SIZE);
    cli_prompt_size = strlen(prompt);

    /* Initializing platform UART. */
    status = cli_platform_uart_init();
    if (status != FWK_SUCCESS)
        return status;

    /* Initialize print buffer FIFO. */
    status = fifo_init(
        &cli_print_fifo, cli_print_fifo_buffer, CLI_CONFIG_PRINT_BUFFER_SIZE);
    if (status != FWK_SUCCESS)
        return status;

    status = cli_command_init();
    if (status != FWK_SUCCESS)
        return status;

    /* Setting state to CLI_READY. */
    cli_state = CLI_READY;

    return FWK_SUCCESS;
}

uint32_t cli_start(void)
{
    /* This function can only run when current state is CLI_READY. */
    if (cli_state != CLI_READY)
        return FWK_E_STATE;

    /* Attempt to start thread. */
    cli_main(NULL);

    return FWK_SUCCESS;
}

uint32_t cli_bprintf(cli_option_et options, const char *format, ...)
{
    va_list arg;
    uint32_t buffer_index = 0;
    char scratch_buffer[CLI_CONFIG_SCRATCH_BUFFER_SIZE] = { 0 };

    /* Check parameters. */
    if (format == NULL)
        return FWK_E_PARAM;

    /* Check CLI state. */
    if (cli_state == CLI_NOT_READY)
        return FWK_E_STATE;

    /* Check if we need any escape sequence formatting. */
    if (options != 0) {
        cli_format(options, scratch_buffer, CLI_CONFIG_SCRATCH_BUFFER_SIZE);
        buffer_index = strnlen(scratch_buffer, CLI_CONFIG_SCRATCH_BUFFER_SIZE);
    }

    /* Generated formatted print string. */
    va_start(arg, format);
    cli_snprintf_arg(
        (scratch_buffer + buffer_index),
        (scratch_buffer + CLI_CONFIG_SCRATCH_BUFFER_SIZE),
        format,
        &arg);
    va_end(arg);
    buffer_index = strnlen(scratch_buffer, CLI_CONFIG_SCRATCH_BUFFER_SIZE);

    /* Make sure to return formatting to normal after escape sequences are used.
     */
    if (options != NONE) {
        strncat(
            scratch_buffer,
            "\x1B[0m",
            (CLI_CONFIG_SCRATCH_BUFFER_SIZE - buffer_index));
        buffer_index = strnlen(scratch_buffer, CLI_CONFIG_SCRATCH_BUFFER_SIZE);
    }

    /* Check to make sure we didn't fill up our scratch buffer. */
    if (buffer_index >= CLI_CONFIG_SCRATCH_BUFFER_SIZE)
        return FWK_E_NOMEM;

    /* Send formatted print data to print FIFO. */
    return cli_bprint(scratch_buffer);
}

uint32_t cli_bprint(const char *string)
{
    uint32_t i = 0;
    int32_t status = FWK_SUCCESS;

    /* Check parameters. */
    if (string == NULL)
        return FWK_E_PARAM;

    /* If not ready, return an error. */
    if (cli_state == CLI_NOT_READY)
        return FWK_E_STATE;

    /* If ready but not running, print directly to UART. */
    if (cli_state == CLI_READY)
        return cli_print(string);

    /* Put data into FIFO. */
    for (i = 0; ; i++) {
        if ((string[i] == 0) &&
            (i >= CLI_CONFIG_SCRATCH_BUFFER_SIZE) &&
            (status != FWK_SUCCESS))
            break;
        status = fifo_put(&cli_print_fifo, (char *)&string[i]);
    }

    /* Print an error message if the print buffer is full. */
    if ((status != FWK_SUCCESS) && (overflow == false)) {
        overflow = true;
        cli_print("\x1B[31mCONSOLE ERROR:\x1B[0m Print buffer overflow.\n");
    }

    return status;
}
static char sCLIbuffer[CLI_CONFIG_SCRATCH_BUFFER_SIZE] = { 0 };

static const char str_format_buf_too_small[] =
    "\x1B[31mCONSOLE ERROR:\x1B[0m CLI format print buffer too small.\n";
uint32_t cli_printf(cli_option_et options, const char *format, ...)
{
    va_list arg;
    int32_t status = FWK_SUCCESS;

    /* Checking pointers. */
    if (format == 0)
        return FWK_E_PARAM;

    /* Applying style options. */
    if (options != NONE) {
        status = cli_format(options, 0, 0);
        if (status != FWK_SUCCESS)
            return status;
    }

    /* Printing formatted text. */
    va_start(arg, format);
    cli_snprintf_arg(
        sCLIbuffer,
        sCLIbuffer + CLI_CONFIG_SCRATCH_BUFFER_SIZE,
        (char *)format,
        &arg);
    va_end(arg);

    /* Print format string. */
    status = cli_print((const char *)sCLIbuffer);
    if (status != FWK_SUCCESS)
        return status;

    /* If style options were used, reset everything to default. */
    if (options != NONE) {
        cli_format(NONE, 0, 0);
        if (status != FWK_SUCCESS)
            return status;
    }

    /* Making sure we didn't try to print too much. */
    if (strnlen((char *)sCLIbuffer, CLI_CONFIG_SCRATCH_BUFFER_SIZE) >=
        CLI_CONFIG_SCRATCH_BUFFER_SIZE - 1) {
        status = cli_print(str_format_buf_too_small);
        if (status != FWK_SUCCESS)
            return status;
        return FWK_E_NOMEM;
    }

    return status;
}

uint32_t cli_print(const char *string)
{
    uint32_t index = 0;
    int32_t status = FWK_SUCCESS;

    for (index = 0; string[index] != 0; index++) {
        status = cli_platform_uart_put(&string[index], true);
        if (status != FWK_SUCCESS)
            return status;
    }
    return status;
}

void cli_snprintf(char *s, char *smax, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cli_snprintf_arg(s, smax, fmt, &args);
    va_end(args);

    return;
}

uint32_t cli_getline(
    char *buffer,
    uint32_t buffer_size,
    char **argbuf,
    uint32_t argbuf_size,
    uint32_t cursor_position)
{
    char c = 0;
    uint32_t index = 0;
    int32_t status = FWK_SUCCESS;

    /* Validate parameters. */
    if (buffer == NULL ||
        buffer_size == 0 ||
        cursor_position >= cli_terminal_width ||
        argbuf == NULL ||
        argbuf_size == 0)
        return FWK_E_PARAM;

    /* Zero out buffer and argbuf. */
    memset(buffer, 0, buffer_size);
    memset(argbuf, 0, argbuf_size * sizeof(char **));

    /* Print prompt arrow. */
    status = cli_print(CLI_PROMPT);
    if (status != FWK_SUCCESS)
        return status;

    /* Increment cursor position since we just printed an arrow and space. */
    cursor_position = (cursor_position + 2) % cli_terminal_width;

    /*
     * Loop will terminate when the user presses enter or when an error is
     * generated.  If your cli_platform_uart_get is thread friendly (and it
     * should be if implemented correctly), this loop will have negligible
     * impact on system performance.
     */
    while (1) {
        /* Grab a character from the UART. */
        status = cli_platform_uart_get(&c, true);
        if (status != FWK_SUCCESS)
            return status;

        /*
         * Ignore non-printing characters except for a few we care about.
         * 0x00 - 0x1F are non-printing characters.
         * 0x7F - 0xFF are non-printing characters.
         * Carriage return: \r, 0x0D
         * Newline:         \n, 0x0C
         * Backspace:       \b, 0x08
         * Delete:              0x7F
         */
        if ((c <= 0x1F || c >= 0x7F) && c != '\r' && c != '\n' && c != '\b' &&
            c != 0x7F)
            continue;


        /* If backspace (0x08) or delete (0x7F) character received. */
        if (c == '\b' || c == 0x7F) {
            /* Only accept backspace presses if we're not at the beginning of
             * the string. */
            if (index != 0) {
                status = cli_backspace(&cursor_position, cli_terminal_width);
                if (status != FWK_SUCCESS)
                    return status;

                /* Decrement index and set old last character to null. */
                index = index - 1;
                buffer[index] = 0;
            }
            continue;
        }

        /* If newline received. */
        if (c == '\n' || c == '\r') {
            /* Making sure the rest of the buffer is zero. */
            memset(&(buffer[index]), 0, buffer_size - index);
            status = cli_print("\n");
            if (status != FWK_SUCCESS)
                return status;
            break;
        }

        /* Echo received character to console. */
        status = cli_platform_uart_put(&c, true);
        if (status != FWK_SUCCESS)
            return status;

        cursor_position = (cursor_position + 1) % cli_terminal_width;

        /* Incrementing indices. */
        buffer[index] = c;
        index = index + 1;
        if (index >= buffer_size) {
            /*
             * Add null termination in case the user doesn't check return codes
             * and tries to use the buffer.
             */
            buffer[index - 1] = 0;
            return FWK_E_NOMEM;
        }

        /* Add new null terminator. */
        buffer[index] = 0;
    }

    status = cli_split(argbuf, argbuf_size, buffer, index, " ");
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

/*****************************************************************************/
/* Private Function Definitions                                              */
/*****************************************************************************/

static void cli_main(void const *argument)
{
    int32_t status = FWK_SUCCESS;
    uint32_t last_history_index = CLI_CONFIG_HISTORY_LENGTH - 1;
    uint32_t command_length = 0;
    bool cli_exit = false;

    /* Thread was started successfully, set state to CLI_RUNNING. */
    cli_state = CLI_RUNNING;

    /* Initialize command history buffer pointers. */
    for (uint32_t i = 0; i < CLI_CONFIG_HISTORY_LENGTH; i++)
        cli_history[i] = &cli_history_buffer[i * CLI_CONFIG_COMMAND_BUF_SIZE];

    /* Loop forever. */
    while (1) {
        /* Printing prompt text. */
        cli_printf(NONE, cli_prompt);

        /* Zero out input buffer. */
        memset(cli_input_buffer, 0, CLI_CONFIG_COMMAND_BUF_SIZE);

        /* Get command from terminal UART. */
        status = cli_get_command(
            cli_input_buffer,
            CLI_CONFIG_COMMAND_BUF_SIZE - 1,
            &command_length,
            cli_history,
            history_index,
            CLI_CONFIG_HISTORY_LENGTH,
            &cli_exit);
        if (status != FWK_SUCCESS) {
            cli_error_handler(status);
            continue;
        }

        /* Ctrl+d was pressed to exit cli */
        if (cli_exit) {
            cli_state = CLI_READY;
            return;
        }

        /* Make sure command string is not empty. */
        if (cli_input_buffer[0] == 0)
            continue;


        /* Update history buffer if command is different than the last one. */
        if (cli_strncmp(
                cli_input_buffer,
                cli_history[last_history_index],
                CLI_CONFIG_COMMAND_BUF_SIZE) != 0) {
            strncpy(
                cli_history[history_index],
                cli_input_buffer,
                CLI_CONFIG_COMMAND_BUF_SIZE);
            history_index = (history_index + 1) % CLI_CONFIG_HISTORY_LENGTH;
            last_history_index =
                (last_history_index + 1) % CLI_CONFIG_HISTORY_LENGTH;
        }

        /* Splitting up command into individual argument strings. */
        status = cli_split(
            cli_args,
            CLI_CONFIG_MAX_NUM_ARGUMENTS,
            cli_input_buffer,
            command_length,
            " ");
        if (status != FWK_SUCCESS) {
            cli_error_handler(status);
            continue;
        }

        /* If the user didn't type any valid arguments, don't process it. */
        if (cli_args[0] == 0)
            continue;

        /* Dispatching command for processing. */
        status = cli_command_dispatch(cli_args);
        if (status != FWK_SUCCESS)
            cli_error_handler(status);
    }
}

uint32_t cli_format(cli_option_et options, char *buffer, uint32_t size)
{
    int32_t status = FWK_SUCCESS;
    static char tmp_buf[10] = { 0 };

    if (buffer != NULL)
        /* Add a null terminator before we do anything. */
        buffer[0] = '\0';

    if (automation_mode)
        return status;

    /* If no options given, send SGR default sequence to remove formatting. */
    if (options == NONE) {
        if (buffer != NULL) {
            if (size < 5)
                return FWK_E_NOMEM;
            memcpy(buffer, "\x1B[0m", 5);
            buffer = &(buffer[4]);
            size = size - 4;
        } else
            return cli_print("\x1B[0m");
    }
    /* Clear terminal window. */
    if (options & CLEAR_DISPLAY) {
        if (buffer != NULL) {
            if (size < 5)
                return FWK_E_NOMEM;
            memcpy(buffer, "\x1B[2J", 5);
            buffer = &(buffer[4]);
            size = size - 4;
        } else
            status = cli_print("\x1B[2J");

        if (status != FWK_SUCCESS)
            return status;
    }
    /* Reset cursor. */
    if (options & RESET_CURSOR) {
        if (buffer != NULL) {
            if (size < 7)
                return FWK_E_NOMEM;

            memcpy(buffer, "\x1B[0;0f", 7);
            buffer = &(buffer[6]);
            size = size - 6;
        } else
            status = cli_print("\x1B[0;0f");

        if (status != FWK_SUCCESS)
            return status;
    }
    /* SGR settings. */
    if (options & CLI_ALL_MASK) {
        if (buffer != NULL) {
            if (size < 3)
                return FWK_E_NOMEM;
            memcpy(buffer, "\x1B[", 3);
            buffer = &(buffer[2]);
            size = size - 2;
        } else {
            status = cli_print("\x1B[");
            if (status != FWK_SUCCESS)
                return status;
        }
        /* Bold/bright. */
        if (options & BOLD) {
            if (buffer != NULL) {
                if (size < 3)
                    return FWK_E_NOMEM;

                memcpy(buffer, ";1", 3);
                buffer = &(buffer[2]);
                size = size - 2;
            } else {
                status = cli_print(";1");
                if (status != FWK_SUCCESS)
                    return status;
            }
        }
        /* Underlining. */
        if (options & UNDERLINE) {
            if (buffer != NULL) {
                if (size < 3)
                    return FWK_E_NOMEM;
                memcpy(buffer, ";4", 3);
                buffer = &(buffer[2]);
                size = size - 2;
            } else {
                status = cli_print(";4");
                if (status != FWK_SUCCESS)
                    return status;
            }
        }
        /* Background color. */
        if (options & CLI_BG_COLOR_MASK) {
            if (buffer != NULL) {
                if (size < 4)
                    return FWK_E_NOMEM;

                cli_snprintf(
                    tmp_buf,
                    tmp_buf + 10,
                    (const char *)";%d",
                    (uint16_t)(
                        (options & CLI_BG_COLOR_MASK) >> CLI_BG_COLOR_SHIFT));
                memcpy(buffer, tmp_buf, 4);
                buffer = &(buffer[3]);
                size = size - 3;
            } else {
                status = cli_printf(
                    0,
                    ";%d",
                    (uint16_t)(
                        (options & CLI_BG_COLOR_MASK) >> CLI_BG_COLOR_SHIFT));
                if (status != FWK_SUCCESS)
                    return status;
            }
        }
        /* Foreground color. */
        if (options & CLI_TEXT_COLOR_MASK) {
            if (buffer != NULL) {
                if (size < 4)
                    return FWK_E_NOMEM;
                cli_snprintf(
                    tmp_buf,
                    tmp_buf + 10,
                    (const char *)";%d",
                    (char)((options & CLI_TEXT_COLOR_MASK) >>
                        CLI_TEXT_COLOR_SHIFT));
                memcpy(buffer, tmp_buf, 4);
                buffer = &(buffer[3]);
                size = size - 3;
            } else {
                status = cli_printf(
                    0,
                    ";%d",
                    (char)((options & CLI_TEXT_COLOR_MASK) >>
                        CLI_TEXT_COLOR_SHIFT));
                if (status != FWK_SUCCESS)
                    return status;
            }
        }
        if (buffer != NULL) {
            if (size < 2)
                return FWK_E_NOMEM;
            memcpy(buffer, "m", 2);
        } else {
            status = cli_print("m");
            if (status != FWK_SUCCESS)
                return status;
        }
    }

    return FWK_SUCCESS;
}

static uint32_t cli_get_command(
    char *buffer,
    uint32_t buffer_size,
    uint32_t *command_length,
    char **history,
    uint32_t history_index,
    uint32_t history_size,
    bool *exit)
{
    int32_t status = FWK_SUCCESS;
    uint32_t index = 0;
    uint32_t cursor_index = cli_prompt_size;
    char c = 0;
    static char escape[8] = { 0 };
    uint32_t escape_index = 0;
    uint32_t history_oldest = history_index;
    bool flag_escape_sequence = false;

    (void)escape;

    /* Checking parameters. */
    if (buffer == 0 || buffer_size == 0 || history == 0 ||
        command_length == 0)
        return FWK_E_PARAM;

    if (!automation_mode) {
        /* Getting terminal window size. */
        /* Saving cursor position. */
        cli_print("\x1B[s");
        /* Moving cursor to bottom right position. */
        cli_print("\x1B[999;999f");
        /* Requesting new cursor position. */
        cli_print("\x1B[6n");
        /* Restoring old cursor position. */
        cli_print("\x1B[u");
    }

    while (1) {
        /* Get character from UART. */
        status = cli_platform_uart_get(&c, true);
        if (status != FWK_SUCCESS)
            return status;

        /* If we received a Ctrl+C press, go to debug mode. */
        if (c == '\x03') {
            *command_length = 0;
            return cli_debug_output();
        }

        /* If we received a Ctrl+d press, exit cli. */
        if (c == '\x04') {
            *exit = true;
            return FWK_SUCCESS;
        }

        /* Ignoring non-printing characters except for a few we care about. */
        if (c < 0x20 && c != '\r' && c != '\n' && c != '\b') {
            if (c == 0x1B)
                flag_escape_sequence = true;
            continue;
        }

        /* Dealing with escape sequences. */
        if (flag_escape_sequence == true) {
            escape[escape_index] = c;
            escape[escape_index + 1] = 0;
            escape_index = escape_index + 1;

            /* Escape sequences end with a letter. */
            if ((c > 0x40 && c < 0x5B) || (c > 0x60 && c < 0x7B)) {
                flag_escape_sequence = false;
                escape_index = 0;

                /* Up arrow press. */
                if (c == 'A') {
                    if (((history_oldest + 1) % history_size) !=
                        history_index) {
                        /* Rewind history index by 1. */
                        if (history_index == 0)
                            history_index = history_size - 1;
                        else
                            history_index = history_index - 1;

                        /* If command entry is empty then stop and restore index
                         * value. */
                        if (history[history_index][0] == 0) {
                            history_index = (history_index + 1) % history_size;
                        } else {
                            /* Erasing currently entered command. */
                            for (; index > 0; index--) {
                                status = cli_backspace(
                                    &cursor_index, cli_terminal_width);
                                if (status != FWK_SUCCESS)
                                    return status;
                            }

                            /* Copying history command from history buffer. */
                            strncpy(
                                buffer, history[history_index], buffer_size);

                            /* Printing history command to screen. */
                            while (buffer[index] != 0) {
                                status = cli_platform_uart_put(&buffer[index],
                                    true);
                                if (status != FWK_SUCCESS)
                                    return status;
                                index = index + 1;
                                cursor_index =
                                    (cursor_index + 1) % cli_terminal_width;
                            }
                        }
                    }
                }

                /* Down arrow press. */
                if (c == 'B') {
                    if (history_index != history_oldest) {
                        /* Getting index of history item to load. */
                        history_index = (history_index + 1) % history_size;

                        /* Erasing everything in the currently entered command.
                         */
                        for (; index > 0; index--) {
                            status = cli_backspace(
                                &cursor_index, cli_terminal_width);
                            if (status != FWK_SUCCESS)
                                return status;
                        }

                        /* If we're back to the current command start fresh and
                         * zero buffer. */
                        if (history_index == history_oldest)
                            memset(buffer, 0, buffer_size);
                        else {
                            /* Copying history command from history buffer. */
                            strncpy(
                                buffer, history[history_index], buffer_size);

                            /* Printing history command to screen. */
                            while (buffer[index] != 0) {
                                status = cli_platform_uart_put(&buffer[index],
                                    true);
                                if (status != FWK_SUCCESS)
                                    return status;

                                index = index + 1;
                                cursor_index =
                                    (cursor_index + 1) % cli_terminal_width;
                            }
                        }
                    }
                }

                /* Handling cursor position response sequence. */
                if (c == 'R') {
                    uint32_t i = 0;
                    for (i = 0; escape[i] != '['; i++)
                        ;
                    i++;
                    cli_terminal_height = strtoul(&escape[i], NULL, 0);
                    for (; escape[i] != ';'; i++)
                        ;
                    i++;
                    cli_terminal_width = strtoul(&escape[i], NULL, 0);
                }
            }
            continue;
        }

        /* If backspace (0x08) or delete (0x7F) character received. */
        if (c == 0x08 || c == 0x7F) {
            /* Only accept backspace presses if we're not at the beginning of
             * the string. */
            if (index != 0) {
                status = cli_backspace(&cursor_index, cli_terminal_width);
                if (status != FWK_SUCCESS)
                    return status;
                index = index - 1;
                buffer[index] = 0;
            }
            continue;
        }

        /* If newline received. */
        if (c == '\n' || c == '\r') {
            /* Making sure the rest of the buffer is zero. */
            memset(&(buffer[index]), 0, buffer_size - index);
            *command_length = index;
            return cli_print("\n");
        }

        /* Printing received character to console. */
        status = cli_platform_uart_put(&c, true);
        if (status != FWK_SUCCESS)
            return status;

        /* Incrementing indices. */
        buffer[index] = c;
        index = index + 1;
        if (index >= buffer_size)
            return FWK_E_NOMEM;
        buffer[index] = 0;
        cursor_index = (cursor_index + 1) % cli_terminal_width;
    }
}

static uint32_t cli_backspace(uint32_t *cursor, uint32_t width)
{
    uint32_t status = FWK_SUCCESS;

    /* If cursor is at the first position of a line. */
    if (*cursor == 0) {
        status = cli_printf(0, "\x1B[A\x1B[%dC ", width - 1);
        if (status != FWK_SUCCESS)
            return status;
        *cursor = width - 1;
    } else {
        /* For compatibility, print back, space, back. */
        status = cli_print("\x1B[D \x1B[D");
        if (status != FWK_SUCCESS)
            return status;
        *cursor = *cursor - 1;
    }

    return FWK_SUCCESS;
}

static uint32_t cli_split(
    char **argbuf,
    uint32_t argbuf_size,
    char *command,
    uint32_t command_length,
    const char *whitespace)
{
    uint32_t index = 0;
    uint32_t argbuf_ctr = 0;
    bool flag_paren = false;
    bool flag_last_was_whitespace = true;

    /* Checking pointers. */
    if (argbuf == 0 || command == 0 || argbuf_size == 0 || whitespace == 0)
        return FWK_E_PARAM;

    argbuf[0] = 0;

    for (index = 0; index < command_length; index++) {
        /* If whitespace is encountered outside of parenthesis, change it to
         * null and set flag. */
        if (strchr(whitespace, command[index]) != 0 && flag_paren == false) {
            command[index] = 0;
            flag_last_was_whitespace = true;
            continue;
        }

        /* Handle parenthesis. */
        if (command[index] == '\"') {
            /* If we've reached the end of an arg in parenthesis, reset flag. */
            if (flag_paren == true) {
                flag_paren = false;
                command[index] = 0;
                flag_last_was_whitespace = true;
                continue;
            }

            /* If we receive an opening parenthesis preceded by a whitespace
             * character, mark null and enter parenthesis processing. */
            if (flag_paren == false && flag_last_was_whitespace == true) {
                flag_paren = true;
                command[index] = 0;
                continue;
            }
        }

        /* First regular character after whitespace encountered. */
        if ((strchr(whitespace, command[index]) == 0 || flag_paren == true) &&
            flag_last_was_whitespace == true) {
            flag_last_was_whitespace = false;
            if (argbuf_ctr + 1 >= argbuf_size)
                return FWK_E_NOMEM;
            argbuf[argbuf_ctr] = &command[index];
            argbuf[argbuf_ctr + 1] = 0;
            argbuf_ctr = argbuf_ctr + 1;
        }
    }

    /* If the user forgot to close their parenthesis, return an error. */
    if (flag_paren == true)
        return FWK_E_PARAM;

    return FWK_SUCCESS;
}

static uint32_t cli_command_dispatch(char **args)
{
    uint32_t index = 0;
    uint32_t num_args = 0;
    uint32_t status = FWK_SUCCESS;
    struct fwk_slist *node = NULL;
    struct command_ctx *cc = NULL;
    bool command_found;

    /* Checking pointer. */
    if (args == 0)
        return FWK_E_PARAM;

    /* Special case command: help. */
    if (cli_strncmp(args[0], "help", 5) == 0) {
        FWK_LIST_FOR_EACH(&cli_commands_list, node,
            struct command_ctx, list_node, cc) {
                cli_printf(NONE, "%s\n", cc->cmd.command);
                cli_print(cc->cmd.help);
                cli_print("\n");
        }

        cli_printf(NONE, "help\n");
        cli_print("  Displays this information.\n");
        cli_printf(NONE, "Ctrl+C\n");
        cli_print("  Displays debug output from running threads.\n");
        cli_printf(NONE, "Ctrl+d\n");
        cli_print("  Exit the CLI.\n");
        return FWK_SUCCESS;
    }

    if (cli_strncmp(args[0], "AUTO", 4) == 0) {
        cli_printf(NONE, "AUTO Mode ON\n");
        automation_mode = 1;
        return FWK_SUCCESS;
    }

    if (cli_strncmp(args[0], "OTUA", 4) == 0) {
        cli_printf(NONE, "AUTO Mode OFF\n");
        automation_mode = 0;
        return FWK_SUCCESS;
    }

    /* Searching for command handler. */
    /* Using strcmp here because each entry in args is guaranteed to be null
     * terminated by cli_split. */
    command_found = false;
    FWK_LIST_FOR_EACH(&cli_commands_list, node,
        struct command_ctx, list_node, cc) {
        if (strcmp(args[0], cc->cmd.command) == 0) {
            command_found = true;
            break;
        }
    }
    if (!command_found)
        return FWK_E_SUPPORT;

    /* Handler found, if -h or --help is given just print it's help string and
     * return. */
    if (cc->cmd.ignore_help_flag == false) {
        for (index = 1;
             (args[index] != 0) && (cli_strncmp(args[index], "-h", 3) != 0) &&
             (cli_strncmp(args[index], "--help", 7) != 0);
             index++)
            ;

        if ((args[index] != 0 || cc->cmd.handler == 0) &&
            cc->cmd.help != 0) {
            status = cli_print(cc->cmd.help);
            if (status != FWK_SUCCESS)
                return status;

            /* Print a newline since help strings shouldn't have newlines at the
             * end. */
            cli_print("\n");
            return status;
        }
    }

    /* Counting arguments. */
    for (num_args = 0; args[num_args] != 0; num_args++)
        ;

    /* Calling command handler. */
    /* args is incremented so the command just gets the arguments and not the
     * name of itself. */
    if (cc->cmd.handler != 0)
        return cc->cmd.handler(num_args, args);
    else
        return FWK_E_PARAM;
}

static uint32_t cli_debug_output(void)
{
    char c = 0;
    uint32_t fifo_status = FWK_SUCCESS;

    cli_printf(
        0,
        "\nNow showing debug console output, to return to the command line, "
        "press Ctrl+C.\n");
    while (1) {
        /* Looking for Ctrl+C press. */
        if (cli_platform_uart_get((char *)&c, false) == FWK_SUCCESS) {
            if (c == '\x03') {
                cli_printf(
                    0,
                    "\nNow showing command line, to return to debug output, "
                    "press Ctrl+C.\n");
                return FWK_SUCCESS;
            }
        }

        /* Read from print FIFO. */
        fifo_status = fifo_get(&cli_print_fifo, &c);
        if (fifo_status == FWK_SUCCESS) {
            overflow = false;
            cli_platform_uart_put((char *)&c, true);
        } else
            /* If no characters are available, let other stuff run. */
            cli_platform_delay_ms(0);
    }
}

static void cli_error_handler(uint32_t status)
{
    if (status == FWK_SUCCESS)
        return;

    cli_printf(NONE, "CONSOLE ERROR: ");
    switch (status) {
    case FWK_E_NOMEM:
        cli_print("Buffer size.\n");
        break;
    case FWK_E_PARAM:
        cli_print("Bad argument.\n");
        break;
    case FWK_E_SUPPORT:
        cli_print("Not found.\n");
        break;
    case FWK_E_DATA:
        cli_print("No data available.\n");
        break;
    default:
        cli_print("Unknown error.\n");
        return;
    }
}

static void cli_val2str(
    char **outstr,
    char *smax,
    uint32_t value,
    uint32_t base,
    int32_t fill)
{
    /* Just need enough space to store 64 bit decimal integer */
    unsigned char str[20] = { 0 };
    int i = 0;

    do {
        str[i++] = "0123456789ABCDEF"[value % base];
    } while (value /= base);

    while (--fill >= i) {
        **outstr = '0';
        *outstr = *outstr + 1;
        if (*outstr >= smax)
            return;
    }

    while (--i >= 0) {
        **outstr = str[i];
        *outstr = *outstr + 1;
        if (*outstr >= smax)
            return;
    }
}

static void cli_snprintf_arg(
    char *s,
    char *smax,
    const char *fmt,
    va_list *args)
{
    int bit64 = 0;
    int64_t num = 0;
    uint64_t unum = 0;
    char *str = NULL;
    int fill = 0;
    uint32_t most_significant = 0;
    char c = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            bit64 = 0;
            fill = 0;
        /* Check the format specifier */
loop:
            switch (*fmt) {
            case 'c':
                c = (char)va_arg(*args, int32_t);
                *s++ = c;
                if (s >= smax) {
                    s[-1] = 0;
                    return;
                }
                break;
            case 'i': /* Specifiers i and d do the same thing. */
            case 'd':
                if (bit64) {
                    *s = 0;
                    return;
                }

                num = va_arg(*args, int32_t);

                if (num < 0) {
                    *s++ = '-';
                    if (s >= smax) {
                        s[-1] = 0;
                        return;
                    }
                    unum = (unsigned long int)-num;
                } else {
                    unum = (unsigned long int)num;
                }

                cli_val2str(&s, smax, unum, 10, fill);
                if (s >= smax) {
                    s[-1] = 0;
                    return;
                }
                break;
            case 's':
                str = va_arg(*args, char *);
                while (*str) {
                    *s++ = *str++;
                    if (s >= smax) {
                        s[-1] = 0;
                        return;
                    }
                }
                break;
            case 'x': /* All hex prints use uppercase hex digits. */
            case 'X':
                if (bit64) {
                    unum = va_arg(*args, uint64_t);
                    most_significant = (uint32_t)(unum >> 32);
                    if (most_significant) {
                        cli_val2str(
                            &s,
                            smax,
                            most_significant,
                            16,
                            (fill >= 8) ? fill - 8 : 0);
                        if (s >= smax) {
                            s[-1] = 0;
                            return;
                        }
                        cli_val2str(&s, smax, unum, 16, 8);
                        if (s >= smax) {
                            s[-1] = 0;
                            return;
                        }
                    } else {
                        cli_val2str(&s, smax, unum, 16, fill);
                        if (s >= smax) {
                            s[-1] = 0;
                            return;
                        }
                    }
                } else {
                    unum = va_arg(*args, uint32_t);
                    cli_val2str(&s, smax, unum, 16, fill);
                    if (s >= smax) {
                        s[-1] = 0;
                        return;
                    }
                }
                break;
            case 'l':
                bit64 = 1;
                fmt++;
                goto loop;
            case 'u':
                if (bit64) {
                    *s = 0;
                    return;
                }

                unum = va_arg(*args, uint32_t);

                cli_val2str(&s, smax, unum, 10, fill);
                if (s >= smax) {
                    s[-1] = 0;
                    return;
                }
                break;
            case '0':
                fmt++;
                /* Make sure we have a number for fill length. */
                if (((*fmt) < '0') || ((*fmt) > '9')) {
                    *s = 0;
                    return;
                }
                fill = strtoul((char *)fmt, (char **)&fmt, 0);
                goto loop;
            default:
                /* Exit on any other format specifier */
                *s = 0;
                return;
            }
            fmt++;
            continue;
        }

        *s++ = *fmt++;
        if (s == smax) {
            s[-1] = 0;
            return;
        }
    }
    *s = 0;
}
