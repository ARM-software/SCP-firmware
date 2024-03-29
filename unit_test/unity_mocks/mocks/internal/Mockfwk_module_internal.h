/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKFWK_MODULE_INTERNAL_H
#define _MOCKFWK_MODULE_INTERNAL_H

#include "unity.h"
#include "internal/fwk_module.h"

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

void Mockfwk_module_internal_Init(void);
void Mockfwk_module_internal_Destroy(void);
void Mockfwk_module_internal_Verify(void);




#define fwk_module_start_ExpectAndReturn(cmock_retval) fwk_module_start_CMockExpectAndReturn(__LINE__, cmock_retval)
void fwk_module_start_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
typedef int (* CMOCK_fwk_module_start_CALLBACK)(int cmock_num_calls);
void fwk_module_start_AddCallback(CMOCK_fwk_module_start_CALLBACK Callback);
void fwk_module_start_Stub(CMOCK_fwk_module_start_CALLBACK Callback);
#define fwk_module_start_StubWithCallback fwk_module_start_Stub
#define fwk_module_stop_ExpectAndReturn(cmock_retval) fwk_module_stop_CMockExpectAndReturn(__LINE__, cmock_retval)
void fwk_module_stop_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
typedef int (* CMOCK_fwk_module_stop_CALLBACK)(int cmock_num_calls);
void fwk_module_stop_AddCallback(CMOCK_fwk_module_stop_CALLBACK Callback);
void fwk_module_stop_Stub(CMOCK_fwk_module_stop_CALLBACK Callback);
#define fwk_module_stop_StubWithCallback fwk_module_stop_Stub
#define fwk_module_get_ctx_ExpectAnyArgsAndReturn(cmock_retval) fwk_module_get_ctx_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_module_get_ctx_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, struct fwk_module_context* cmock_to_return);
#define fwk_module_get_ctx_ExpectAndReturn(id, cmock_retval) fwk_module_get_ctx_CMockExpectAndReturn(__LINE__, id, cmock_retval)
void fwk_module_get_ctx_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t id, struct fwk_module_context* cmock_to_return);
typedef struct fwk_module_context* (* CMOCK_fwk_module_get_ctx_CALLBACK)(fwk_id_t id, int cmock_num_calls);
void fwk_module_get_ctx_AddCallback(CMOCK_fwk_module_get_ctx_CALLBACK Callback);
void fwk_module_get_ctx_Stub(CMOCK_fwk_module_get_ctx_CALLBACK Callback);
#define fwk_module_get_ctx_StubWithCallback fwk_module_get_ctx_Stub
#define fwk_module_get_state_ExpectAnyArgsAndReturn(cmock_retval) fwk_module_get_state_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_module_get_state_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define fwk_module_get_state_ExpectAndReturn(id, state, cmock_retval) fwk_module_get_state_CMockExpectAndReturn(__LINE__, id, state, cmock_retval)
void fwk_module_get_state_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t id, enum fwk_module_state* state, int cmock_to_return);
typedef int (* CMOCK_fwk_module_get_state_CALLBACK)(fwk_id_t id, enum fwk_module_state* state, int cmock_num_calls);
void fwk_module_get_state_AddCallback(CMOCK_fwk_module_get_state_CALLBACK Callback);
void fwk_module_get_state_Stub(CMOCK_fwk_module_get_state_CALLBACK Callback);
#define fwk_module_get_state_StubWithCallback fwk_module_get_state_Stub
#define fwk_module_get_state_ReturnThruPtr_state(state) fwk_module_get_state_CMockReturnMemThruPtr_state(__LINE__, state, sizeof(enum fwk_module_state))
#define fwk_module_get_state_ReturnArrayThruPtr_state(state, cmock_len) fwk_module_get_state_CMockReturnMemThruPtr_state(__LINE__, state, cmock_len * sizeof(*state))
#define fwk_module_get_state_ReturnMemThruPtr_state(state, cmock_size) fwk_module_get_state_CMockReturnMemThruPtr_state(__LINE__, state, cmock_size)
void fwk_module_get_state_CMockReturnMemThruPtr_state(UNITY_LINE_TYPE cmock_line, enum fwk_module_state* state, size_t cmock_size);
#define fwk_module_get_element_ctx_ExpectAnyArgsAndReturn(cmock_retval) fwk_module_get_element_ctx_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void fwk_module_get_element_ctx_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, struct fwk_element_ctx* cmock_to_return);
#define fwk_module_get_element_ctx_ExpectAndReturn(element_id, cmock_retval) fwk_module_get_element_ctx_CMockExpectAndReturn(__LINE__, element_id, cmock_retval)
void fwk_module_get_element_ctx_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t element_id, struct fwk_element_ctx* cmock_to_return);
typedef struct fwk_element_ctx* (* CMOCK_fwk_module_get_element_ctx_CALLBACK)(fwk_id_t element_id, int cmock_num_calls);
void fwk_module_get_element_ctx_AddCallback(CMOCK_fwk_module_get_element_ctx_CALLBACK Callback);
void fwk_module_get_element_ctx_Stub(CMOCK_fwk_module_get_element_ctx_CALLBACK Callback);
#define fwk_module_get_element_ctx_StubWithCallback fwk_module_get_element_ctx_Stub
#define fwk_module_reset_Expect() fwk_module_reset_CMockExpect(__LINE__)
void fwk_module_reset_CMockExpect(UNITY_LINE_TYPE cmock_line);
typedef void (* CMOCK_fwk_module_reset_CALLBACK)(int cmock_num_calls);
void fwk_module_reset_AddCallback(CMOCK_fwk_module_reset_CALLBACK Callback);
void fwk_module_reset_Stub(CMOCK_fwk_module_reset_CALLBACK Callback);
#define fwk_module_reset_StubWithCallback fwk_module_reset_Stub

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif
