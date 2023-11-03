/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKSCMI_POWER_CAPPING_PROTOCOL_H
#define _MOCKSCMI_POWER_CAPPING_PROTOCOL_H

#include "unity.h"
#include "scmi_power_capping_protocol.h"

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

void Mockscmi_power_capping_protocol_Init(void);
void Mockscmi_power_capping_protocol_Destroy(void);
void Mockscmi_power_capping_protocol_Verify(void);




#define pcapping_protocol_init_Ignore() pcapping_protocol_init_CMockIgnore()
void pcapping_protocol_init_CMockIgnore(void);
#define pcapping_protocol_init_StopIgnore() pcapping_protocol_init_CMockStopIgnore()
void pcapping_protocol_init_CMockStopIgnore(void);
#define pcapping_protocol_init_ExpectAnyArgs() pcapping_protocol_init_CMockExpectAnyArgs(__LINE__)
void pcapping_protocol_init_CMockExpectAnyArgs(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_init_Expect(ctx) pcapping_protocol_init_CMockExpect(__LINE__, ctx)
void pcapping_protocol_init_CMockExpect(UNITY_LINE_TYPE cmock_line, struct mod_scmi_power_capping_context* ctx);
typedef void (* CMOCK_pcapping_protocol_init_CALLBACK)(struct mod_scmi_power_capping_context* ctx, int cmock_num_calls);
void pcapping_protocol_init_AddCallback(CMOCK_pcapping_protocol_init_CALLBACK Callback);
void pcapping_protocol_init_Stub(CMOCK_pcapping_protocol_init_CALLBACK Callback);
#define pcapping_protocol_init_StubWithCallback pcapping_protocol_init_Stub
#define pcapping_protocol_init_ExpectWithArray(ctx, ctx_Depth) pcapping_protocol_init_CMockExpectWithArray(__LINE__, ctx, ctx_Depth)
void pcapping_protocol_init_CMockExpectWithArray(UNITY_LINE_TYPE cmock_line, struct mod_scmi_power_capping_context* ctx, int ctx_Depth);
#define pcapping_protocol_init_ReturnThruPtr_ctx(ctx) pcapping_protocol_init_CMockReturnMemThruPtr_ctx(__LINE__, ctx, sizeof(struct mod_scmi_power_capping_context))
#define pcapping_protocol_init_ReturnArrayThruPtr_ctx(ctx, cmock_len) pcapping_protocol_init_CMockReturnMemThruPtr_ctx(__LINE__, ctx, cmock_len * sizeof(*ctx))
#define pcapping_protocol_init_ReturnMemThruPtr_ctx(ctx, cmock_size) pcapping_protocol_init_CMockReturnMemThruPtr_ctx(__LINE__, ctx, cmock_size)
void pcapping_protocol_init_CMockReturnMemThruPtr_ctx(UNITY_LINE_TYPE cmock_line, struct mod_scmi_power_capping_context* ctx, size_t cmock_size);
#define pcapping_protocol_init_IgnoreArg_ctx() pcapping_protocol_init_CMockIgnoreArg_ctx(__LINE__)
void pcapping_protocol_init_CMockIgnoreArg_ctx(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_domain_init_IgnoreAndReturn(cmock_retval) pcapping_protocol_domain_init_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_domain_init_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_domain_init_StopIgnore() pcapping_protocol_domain_init_CMockStopIgnore()
void pcapping_protocol_domain_init_CMockStopIgnore(void);
#define pcapping_protocol_domain_init_ExpectAnyArgsAndReturn(cmock_retval) pcapping_protocol_domain_init_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_domain_init_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_domain_init_ExpectAndReturn(domain_idx, config, cmock_retval) pcapping_protocol_domain_init_CMockExpectAndReturn(__LINE__, domain_idx, config, cmock_retval)
void pcapping_protocol_domain_init_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, uint32_t domain_idx, const struct mod_scmi_power_capping_domain_config* config, int cmock_to_return);
typedef int (* CMOCK_pcapping_protocol_domain_init_CALLBACK)(uint32_t domain_idx, const struct mod_scmi_power_capping_domain_config* config, int cmock_num_calls);
void pcapping_protocol_domain_init_AddCallback(CMOCK_pcapping_protocol_domain_init_CALLBACK Callback);
void pcapping_protocol_domain_init_Stub(CMOCK_pcapping_protocol_domain_init_CALLBACK Callback);
#define pcapping_protocol_domain_init_StubWithCallback pcapping_protocol_domain_init_Stub
#define pcapping_protocol_domain_init_ExpectWithArrayAndReturn(domain_idx, config, config_Depth, cmock_retval) pcapping_protocol_domain_init_CMockExpectWithArrayAndReturn(__LINE__, domain_idx, config, config_Depth, cmock_retval)
void pcapping_protocol_domain_init_CMockExpectWithArrayAndReturn(UNITY_LINE_TYPE cmock_line, uint32_t domain_idx, const struct mod_scmi_power_capping_domain_config* config, int config_Depth, int cmock_to_return);
#define pcapping_protocol_domain_init_IgnoreArg_domain_idx() pcapping_protocol_domain_init_CMockIgnoreArg_domain_idx(__LINE__)
void pcapping_protocol_domain_init_CMockIgnoreArg_domain_idx(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_domain_init_IgnoreArg_config() pcapping_protocol_domain_init_CMockIgnoreArg_config(__LINE__)
void pcapping_protocol_domain_init_CMockIgnoreArg_config(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_bind_IgnoreAndReturn(cmock_retval) pcapping_protocol_bind_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_bind_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_bind_StopIgnore() pcapping_protocol_bind_CMockStopIgnore()
void pcapping_protocol_bind_CMockStopIgnore(void);
#define pcapping_protocol_bind_ExpectAndReturn(cmock_retval) pcapping_protocol_bind_CMockExpectAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_bind_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
typedef int (* CMOCK_pcapping_protocol_bind_CALLBACK)(int cmock_num_calls);
void pcapping_protocol_bind_AddCallback(CMOCK_pcapping_protocol_bind_CALLBACK Callback);
void pcapping_protocol_bind_Stub(CMOCK_pcapping_protocol_bind_CALLBACK Callback);
#define pcapping_protocol_bind_StubWithCallback pcapping_protocol_bind_Stub
#define pcapping_protocol_start_IgnoreAndReturn(cmock_retval) pcapping_protocol_start_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_start_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_start_StopIgnore() pcapping_protocol_start_CMockStopIgnore()
void pcapping_protocol_start_CMockStopIgnore(void);
#define pcapping_protocol_start_ExpectAnyArgsAndReturn(cmock_retval) pcapping_protocol_start_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_start_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_start_ExpectAndReturn(id, cmock_retval) pcapping_protocol_start_CMockExpectAndReturn(__LINE__, id, cmock_retval)
void pcapping_protocol_start_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t id, int cmock_to_return);
typedef int (* CMOCK_pcapping_protocol_start_CALLBACK)(fwk_id_t id, int cmock_num_calls);
void pcapping_protocol_start_AddCallback(CMOCK_pcapping_protocol_start_CALLBACK Callback);
void pcapping_protocol_start_Stub(CMOCK_pcapping_protocol_start_CALLBACK Callback);
#define pcapping_protocol_start_StubWithCallback pcapping_protocol_start_Stub
#define pcapping_protocol_start_IgnoreArg_id() pcapping_protocol_start_CMockIgnoreArg_id(__LINE__)
void pcapping_protocol_start_CMockIgnoreArg_id(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_process_notification_IgnoreAndReturn(cmock_retval) pcapping_protocol_process_notification_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_process_notification_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_process_notification_StopIgnore() pcapping_protocol_process_notification_CMockStopIgnore()
void pcapping_protocol_process_notification_CMockStopIgnore(void);
#define pcapping_protocol_process_notification_ExpectAnyArgsAndReturn(cmock_retval) pcapping_protocol_process_notification_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_process_notification_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_process_notification_ExpectAndReturn(event, cmock_retval) pcapping_protocol_process_notification_CMockExpectAndReturn(__LINE__, event, cmock_retval)
void pcapping_protocol_process_notification_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_event* event, int cmock_to_return);
typedef int (* CMOCK_pcapping_protocol_process_notification_CALLBACK)(const struct fwk_event* event, int cmock_num_calls);
void pcapping_protocol_process_notification_AddCallback(CMOCK_pcapping_protocol_process_notification_CALLBACK Callback);
void pcapping_protocol_process_notification_Stub(CMOCK_pcapping_protocol_process_notification_CALLBACK Callback);
#define pcapping_protocol_process_notification_StubWithCallback pcapping_protocol_process_notification_Stub
#define pcapping_protocol_process_notification_ExpectWithArrayAndReturn(event, event_Depth, cmock_retval) pcapping_protocol_process_notification_CMockExpectWithArrayAndReturn(__LINE__, event, event_Depth, cmock_retval)
void pcapping_protocol_process_notification_CMockExpectWithArrayAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_event* event, int event_Depth, int cmock_to_return);
#define pcapping_protocol_process_notification_IgnoreArg_event() pcapping_protocol_process_notification_CMockIgnoreArg_event(__LINE__)
void pcapping_protocol_process_notification_CMockIgnoreArg_event(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_process_bind_request_IgnoreAndReturn(cmock_retval) pcapping_protocol_process_bind_request_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_process_bind_request_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_process_bind_request_StopIgnore() pcapping_protocol_process_bind_request_CMockStopIgnore()
void pcapping_protocol_process_bind_request_CMockStopIgnore(void);
#define pcapping_protocol_process_bind_request_ExpectAnyArgsAndReturn(cmock_retval) pcapping_protocol_process_bind_request_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void pcapping_protocol_process_bind_request_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define pcapping_protocol_process_bind_request_ExpectAndReturn(api_id, api, cmock_retval) pcapping_protocol_process_bind_request_CMockExpectAndReturn(__LINE__, api_id, api, cmock_retval)
void pcapping_protocol_process_bind_request_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t api_id, const void** api, int cmock_to_return);
typedef int (* CMOCK_pcapping_protocol_process_bind_request_CALLBACK)(fwk_id_t api_id, const void** api, int cmock_num_calls);
void pcapping_protocol_process_bind_request_AddCallback(CMOCK_pcapping_protocol_process_bind_request_CALLBACK Callback);
void pcapping_protocol_process_bind_request_Stub(CMOCK_pcapping_protocol_process_bind_request_CALLBACK Callback);
#define pcapping_protocol_process_bind_request_StubWithCallback pcapping_protocol_process_bind_request_Stub
#define pcapping_protocol_process_bind_request_ExpectWithArrayAndReturn(api_id, api, api_Depth, cmock_retval) pcapping_protocol_process_bind_request_CMockExpectWithArrayAndReturn(__LINE__, api_id, api, api_Depth, cmock_retval)
void pcapping_protocol_process_bind_request_CMockExpectWithArrayAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t api_id, const void** api, int api_Depth, int cmock_to_return);
#define pcapping_protocol_process_bind_request_ReturnThruPtr_api(api) pcapping_protocol_process_bind_request_CMockReturnMemThruPtr_api(__LINE__, api, sizeof(const void*))
#define pcapping_protocol_process_bind_request_ReturnArrayThruPtr_api(api, cmock_len) pcapping_protocol_process_bind_request_CMockReturnMemThruPtr_api(__LINE__, api, cmock_len * sizeof(*api))
#define pcapping_protocol_process_bind_request_ReturnMemThruPtr_api(api, cmock_size) pcapping_protocol_process_bind_request_CMockReturnMemThruPtr_api(__LINE__, api, cmock_size)
void pcapping_protocol_process_bind_request_CMockReturnMemThruPtr_api(UNITY_LINE_TYPE cmock_line, const void** api, size_t cmock_size);
#define pcapping_protocol_process_bind_request_IgnoreArg_api_id() pcapping_protocol_process_bind_request_CMockIgnoreArg_api_id(__LINE__)
void pcapping_protocol_process_bind_request_CMockIgnoreArg_api_id(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_process_bind_request_IgnoreArg_api() pcapping_protocol_process_bind_request_CMockIgnoreArg_api(__LINE__)
void pcapping_protocol_process_bind_request_CMockIgnoreArg_api(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_set_power_apis_Ignore() pcapping_protocol_set_power_apis_CMockIgnore()
void pcapping_protocol_set_power_apis_CMockIgnore(void);
#define pcapping_protocol_set_power_apis_StopIgnore() pcapping_protocol_set_power_apis_CMockStopIgnore()
void pcapping_protocol_set_power_apis_CMockStopIgnore(void);
#define pcapping_protocol_set_power_apis_ExpectAnyArgs() pcapping_protocol_set_power_apis_CMockExpectAnyArgs(__LINE__)
void pcapping_protocol_set_power_apis_CMockExpectAnyArgs(UNITY_LINE_TYPE cmock_line);
#define pcapping_protocol_set_power_apis_Expect(power_management_apis) pcapping_protocol_set_power_apis_CMockExpect(__LINE__, power_management_apis)
void pcapping_protocol_set_power_apis_CMockExpect(UNITY_LINE_TYPE cmock_line, struct mod_scmi_power_capping_power_apis* power_management_apis);
typedef void (* CMOCK_pcapping_protocol_set_power_apis_CALLBACK)(struct mod_scmi_power_capping_power_apis* power_management_apis, int cmock_num_calls);
void pcapping_protocol_set_power_apis_AddCallback(CMOCK_pcapping_protocol_set_power_apis_CALLBACK Callback);
void pcapping_protocol_set_power_apis_Stub(CMOCK_pcapping_protocol_set_power_apis_CALLBACK Callback);
#define pcapping_protocol_set_power_apis_StubWithCallback pcapping_protocol_set_power_apis_Stub
#define pcapping_protocol_set_power_apis_ExpectWithArray(power_management_apis, power_management_apis_Depth) pcapping_protocol_set_power_apis_CMockExpectWithArray(__LINE__, power_management_apis, power_management_apis_Depth)
void pcapping_protocol_set_power_apis_CMockExpectWithArray(UNITY_LINE_TYPE cmock_line, struct mod_scmi_power_capping_power_apis* power_management_apis, int power_management_apis_Depth);
#define pcapping_protocol_set_power_apis_ReturnThruPtr_power_management_apis(power_management_apis) pcapping_protocol_set_power_apis_CMockReturnMemThruPtr_power_management_apis(__LINE__, power_management_apis, sizeof(struct mod_scmi_power_capping_power_apis))
#define pcapping_protocol_set_power_apis_ReturnArrayThruPtr_power_management_apis(power_management_apis, cmock_len) pcapping_protocol_set_power_apis_CMockReturnMemThruPtr_power_management_apis(__LINE__, power_management_apis, cmock_len * sizeof(*power_management_apis))
#define pcapping_protocol_set_power_apis_ReturnMemThruPtr_power_management_apis(power_management_apis, cmock_size) pcapping_protocol_set_power_apis_CMockReturnMemThruPtr_power_management_apis(__LINE__, power_management_apis, cmock_size)
void pcapping_protocol_set_power_apis_CMockReturnMemThruPtr_power_management_apis(UNITY_LINE_TYPE cmock_line, struct mod_scmi_power_capping_power_apis* power_management_apis, size_t cmock_size);
#define pcapping_protocol_set_power_apis_IgnoreArg_power_management_apis() pcapping_protocol_set_power_apis_CMockIgnoreArg_power_management_apis(__LINE__)
void pcapping_protocol_set_power_apis_CMockIgnoreArg_power_management_apis(UNITY_LINE_TYPE cmock_line);

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif