/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_macros.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void *memset(void *s, int c, size_t count)
{
    char *xs = s;
    while (count--)
        *xs++ = (char)c;
    return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    /* copy per 1 byte */
    const char *p = src;
    char *q = dst;

    while (n--) {
        *q++ = *p++;
    }

    return dst;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && src[i] != 0; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}

char *strchr(const char *str, int c)
{
    do {
        if (*str == (char)c)
            return (char *)str;
        str++;
    } while (*str);

    return NULL;
}

size_t strlen(const char *str)
{
    char *tmp = (char *)str;
    size_t counter = 0;
    while (*tmp++)
        ++counter;
    return counter;
}

static void uint_to_str(unsigned int i, char *buf, int base)
{
    char const digit_10[] = "0123456789";
    char const digit_16[] = "0123456789abcdef";
    unsigned int shifter = i;
    char const *digit;

    if (base == 10)
        digit = digit_10;
    else
        digit = digit_16;

    do {
        ++buf;
        shifter = shifter / base;
    } while (shifter);

    *buf = '\0';

    do {
        *--buf = digit[i % base];
        i = i / base;
    } while (i);
}

static void int_to_str(int i, char *buf, int base)
{
    int sign = i;

    if (i < 0) {
        i = -i;
        buf++;
    }

    uint_to_str((unsigned int)i, buf, base);

    if (sign < 0)
        *--buf = '-';
}

static int isdigit(char c)
{
    return (int)(c >= '0' && c <= '9');
}

static int handle_num(char type, char *buf, va_list *args)
{
    int int_num;
    unsigned int uint_num;

    switch (type) {
    case 'u':
        uint_num = va_arg(*args, unsigned int);
        uint_to_str(uint_num, buf, 10);
        break;
    case 'd':
        int_num = va_arg(*args, int);
        int_to_str(int_num, buf, 10);
        break;
    case 'x':
        uint_num = va_arg(*args, unsigned int);
        uint_to_str(uint_num, buf, 16);
        break;
    default:
        return 1;
        break;
    }

    return 0;
}

int vsnprintf(char *str, size_t n, const char *format, va_list args)
{
    char *pos;
    char *s;
    char *tmp = str;
    size_t length = 0;
    int num_length, min_length;
    char num_buf[12];
    int not_implemented;

    for (pos = (char *)format; *pos != '\0'; pos++) {
        while ((*pos != '%') && (*pos != '\0') && (length < n)) {
            *tmp++ = *pos++;
            length++;
        }

        if (length == n)
            break;

        if (*pos == '\0') {
            *tmp = '\0';
            break;
        }

        pos++;

        not_implemented = 0;

        switch (*pos) {
        case 's':
            s = va_arg(args, char *);
            strncpy(tmp, s, n - length);
            break;
        case '0':
            if (isdigit(*(pos + 1)) && (*(pos + 1) > '0')) {
                pos++;
            } else {
                not_implemented = 1;
                break;
            }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            min_length = (int)(*pos - '0');

            if (handle_num(*(pos + 1), num_buf, &args)) {
                if (*(pos - 1) == '0')
                    pos--;

                not_implemented = 1;
                break;
            }

            num_length = (int)strlen(num_buf);

            if (num_length < min_length) {
                while (num_length >= 0)
                    num_buf[min_length--] = num_buf[num_length--];

                if (*(pos - 1) == '0') {
                    if (num_buf[0] == '-') {
                        min_length++;
                        while (min_length > 0)
                            num_buf[min_length--] = '0';
                    } else {
                        while (min_length >= 0)
                            num_buf[min_length--] = '0';
                    }
                } else {
                    while (min_length >= 0)
                        num_buf[min_length--] = ' ';
                }
            }
            strncpy(tmp, num_buf, n - length);
            pos++;
            break;
        default:
            if (handle_num(*pos, num_buf, &args))
                not_implemented = 1;
            else
                strncpy(tmp, num_buf, n - length);
            break;
        }

        if (not_implemented) {
            va_arg(args, unsigned int);
            *tmp++ = '%';
            length++;
            pos--;
        } else {
            while ((*tmp != '\0') && (length < n)) {
                tmp++;
                length++;
            }
        }
    }

    if (tmp == str) {
        *tmp = '\0';
    } else if (length == n) {
        tmp--;
        if (*tmp != '\0')
            *tmp = '\0';
        else
            length--;
    } else if (*(tmp - 1) != '\0') {
        *tmp = '\0';
    } else {
        length--;
    }

    return (int)length;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    int counter;
    va_list args;
    va_start(args, format);
    counter = vsnprintf(str, size, format, args);
    va_end(args);
    return counter;
}

void __assert_fail(
    const char *assertion,
    const char *file,
    unsigned int line,
    const char *function)
{
    while (1)
        continue;
}
