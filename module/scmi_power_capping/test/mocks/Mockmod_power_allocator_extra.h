/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKMOD_POWER_ALLOCATOR_EXTRA_H
#define _MOCKMOD_POWER_ALLOCATOR_EXTRA_H

#include "unity.h"
#include "mod_power_allocator_extra.h"

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

void Mockmod_power_allocator_extra_Init(void);
void Mockmod_power_allocator_extra_Destroy(void);
void Mockmod_power_allocator_extra_Verify(void);




#define get_cap_IgnoreAndReturn(cmock_retval) get_cap_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void get_cap_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define get_cap_StopIgnore() get_cap_CMockStopIgnore()
void get_cap_CMockStopIgnore(void);
#define get_cap_ExpectAnyArgsAndReturn(cmock_retval) get_cap_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void get_cap_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define get_cap_ExpectAndReturn(domain_id, cap, cmock_retval) get_cap_CMockExpectAndReturn(__LINE__, domain_id, cap, cmock_retval)
void get_cap_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t domain_id, uint32_t* cap, int cmock_to_return);
typedef int (* CMOCK_get_cap_CALLBACK)(fwk_id_t domain_id, uint32_t* cap, int cmock_num_calls);
void get_cap_AddCallback(CMOCK_get_cap_CALLBACK Callback);
void get_cap_Stub(CMOCK_get_cap_CALLBACK Callback);
#define get_cap_StubWithCallback get_cap_Stub
#define get_cap_ExpectWithArrayAndReturn(domain_id, cap, cap_Depth, cmock_retval) get_cap_CMockExpectWithArrayAndReturn(__LINE__, domain_id, cap, cap_Depth, cmock_retval)
void get_cap_CMockExpectWithArrayAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t domain_id, uint32_t* cap, int cap_Depth, int cmock_to_return);
#define get_cap_ReturnThruPtr_cap(cap) get_cap_CMockReturnMemThruPtr_cap(__LINE__, cap, sizeof(uint32_t))
#define get_cap_ReturnArrayThruPtr_cap(cap, cmock_len) get_cap_CMockReturnMemThruPtr_cap(__LINE__, cap, cmock_len * sizeof(*cap))
#define get_cap_ReturnMemThruPtr_cap(cap, cmock_size) get_cap_CMockReturnMemThruPtr_cap(__LINE__, cap, cmock_size)
void get_cap_CMockReturnMemThruPtr_cap(UNITY_LINE_TYPE cmock_line, uint32_t* cap, size_t cmock_size);
#define get_cap_IgnoreArg_domain_id() get_cap_CMockIgnoreArg_domain_id(__LINE__)
void get_cap_CMockIgnoreArg_domain_id(UNITY_LINE_TYPE cmock_line);
#define get_cap_IgnoreArg_cap() get_cap_CMockIgnoreArg_cap(__LINE__)
void get_cap_CMockIgnoreArg_cap(UNITY_LINE_TYPE cmock_line);
#define set_cap_IgnoreAndReturn(cmock_retval) set_cap_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void set_cap_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define set_cap_StopIgnore() set_cap_CMockStopIgnore()
void set_cap_CMockStopIgnore(void);
#define set_cap_ExpectAnyArgsAndReturn(cmock_retval) set_cap_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void set_cap_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define set_cap_ExpectAndReturn(domain_id, cap, cmock_retval) set_cap_CMockExpectAndReturn(__LINE__, domain_id, cap, cmock_retval)
void set_cap_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t domain_id, uint32_t cap, int cmock_to_return);
typedef int (* CMOCK_set_cap_CALLBACK)(fwk_id_t domain_id, uint32_t cap, int cmock_num_calls);
void set_cap_AddCallback(CMOCK_set_cap_CALLBACK Callback);
void set_cap_Stub(CMOCK_set_cap_CALLBACK Callback);
#define set_cap_StubWithCallback set_cap_Stub
#define set_cap_IgnoreArg_domain_id() set_cap_CMockIgnoreArg_domain_id(__LINE__)
void set_cap_CMockIgnoreArg_domain_id(UNITY_LINE_TYPE cmock_line);
#define set_cap_IgnoreArg_cap() set_cap_CMockIgnoreArg_cap(__LINE__)
void set_cap_CMockIgnoreArg_cap(UNITY_LINE_TYPE cmock_line);

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif
