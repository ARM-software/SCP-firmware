/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKFWK_IO_H
#define _MOCKFWK_IO_H

#include "unity.h"
#include "fwk_io.h"

/* Ignore the following warnings, since we are copying code */
#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic push
#endif
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wduplicate-decl-specifier"
#endif

void Mockfwk_io_Init(void);
void Mockfwk_io_Destroy(void);
void Mockfwk_io_Verify(void);




#define fwk_io_open_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_open_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_open_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_open_ExpectAndReturn(stream, id, mode, cmock_retval) fwk_io_open_CMockExpectAndReturn(__LINE__, stream, id, mode, cmock_retval)
void fwk_io_open_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct fwk_io_stream* stream, fwk_id_t id, enum fwk_io_mode mode, int cmock_to_return);
#define fwk_io_getch_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_getch_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_getch_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_getch_ExpectAndReturn(stream, ch, cmock_retval) fwk_io_getch_CMockExpectAndReturn(__LINE__, stream, ch, cmock_retval)
void fwk_io_getch_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_io_stream* stream, char* ch, int cmock_to_return);
#define fwk_io_putch_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_putch_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_putch_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_putch_ExpectAndReturn(stream, ch, cmock_retval) fwk_io_putch_CMockExpectAndReturn(__LINE__, stream, ch, cmock_retval)
void fwk_io_putch_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_io_stream* stream, char ch, int cmock_to_return);
#define fwk_io_read_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_read_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_read_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_read_ExpectAndReturn(stream, read, buffer, size, count, cmock_retval) fwk_io_read_CMockExpectAndReturn(__LINE__, stream, read, buffer, size, count, cmock_retval)
void fwk_io_read_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_io_stream* stream, size_t* read, void* buffer, size_t size, size_t count, int cmock_to_return);
#define fwk_io_write_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_write_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_write_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_write_ExpectAndReturn(stream, written, buffer, size, count, cmock_retval) fwk_io_write_CMockExpectAndReturn(__LINE__, stream, written, buffer, size, count, cmock_retval)
void fwk_io_write_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_io_stream* stream, size_t* written, const void* buffer, size_t size, size_t count, int cmock_to_return);
#define fwk_io_puts_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_puts_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_puts_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_puts_ExpectAndReturn(stream, str, cmock_retval) fwk_io_puts_CMockExpectAndReturn(__LINE__, stream, str, cmock_retval)
void fwk_io_puts_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_io_stream* stream, const char* str, int cmock_to_return);
#define fwk_io_vprintf_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_vprintf_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_vprintf_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_vprintf_ExpectAndReturn(stream, format, args, cmock_retval) fwk_io_vprintf_CMockExpectAndReturn(__LINE__, stream, format, args, cmock_retval)
void fwk_io_vprintf_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_io_stream* stream, const char* format, va_list args, int cmock_to_return);
#define fwk_io_printf_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_printf_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_printf_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_printf_ExpectAndReturn(stream, format, cmock_retval) fwk_io_printf_CMockExpectAndReturn(__LINE__, stream, format, cmock_retval)
void fwk_io_printf_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_io_stream* stream, const char* format, int cmock_to_return);
#define fwk_io_close_ExpectAnyArgsAndReturn(cmock_retval) fwk_io_close_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_io_close_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_io_close_ExpectAndReturn(stream, cmock_retval) fwk_io_close_CMockExpectAndReturn(__LINE__, stream, cmock_retval)
void fwk_io_close_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct fwk_io_stream* stream, int cmock_to_return);
#define fwk_io_init_ExpectAndReturn(cmock_retval) fwk_io_init_CMockExpectAndReturn(__LINE__, cmock_retval)
void fwk_io_init_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif
