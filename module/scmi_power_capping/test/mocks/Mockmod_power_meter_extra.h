/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKMOD_POWER_METER_EXTRA_H
#define _MOCKMOD_POWER_METER_EXTRA_H

#include "unity.h"
#include "mod_power_meter_extra.h"

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

void Mockmod_power_meter_extra_Init(void);
void Mockmod_power_meter_extra_Destroy(void);
void Mockmod_power_meter_extra_Verify(void);




#define get_power_IgnoreAndReturn(cmock_retval) get_power_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void get_power_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define get_power_StopIgnore() get_power_CMockStopIgnore()
void get_power_CMockStopIgnore(void);
#define get_power_ExpectAnyArgsAndReturn(cmock_retval) get_power_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void get_power_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define get_power_ExpectAndReturn(domain_id, power, cmock_retval) get_power_CMockExpectAndReturn(__LINE__, domain_id, power, cmock_retval)
void get_power_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t domain_id, uint32_t* power, int cmock_to_return);
typedef int (* CMOCK_get_power_CALLBACK)(fwk_id_t domain_id, uint32_t* power, int cmock_num_calls);
void get_power_AddCallback(CMOCK_get_power_CALLBACK Callback);
void get_power_Stub(CMOCK_get_power_CALLBACK Callback);
#define get_power_StubWithCallback get_power_Stub
#define get_power_ExpectWithArrayAndReturn(domain_id, power, power_Depth, cmock_retval) get_power_CMockExpectWithArrayAndReturn(__LINE__, domain_id, power, power_Depth, cmock_retval)
void get_power_CMockExpectWithArrayAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t domain_id, uint32_t* power, int power_Depth, int cmock_to_return);
#define get_power_ReturnThruPtr_power(power) get_power_CMockReturnMemThruPtr_power(__LINE__, power, sizeof(uint32_t))
#define get_power_ReturnArrayThruPtr_power(power, cmock_len) get_power_CMockReturnMemThruPtr_power(__LINE__, power, cmock_len * sizeof(*power))
#define get_power_ReturnMemThruPtr_power(power, cmock_size) get_power_CMockReturnMemThruPtr_power(__LINE__, power, cmock_size)
void get_power_CMockReturnMemThruPtr_power(UNITY_LINE_TYPE cmock_line, uint32_t* power, size_t cmock_size);
#define get_power_IgnoreArg_domain_id() get_power_CMockIgnoreArg_domain_id(__LINE__)
void get_power_CMockIgnoreArg_domain_id(UNITY_LINE_TYPE cmock_line);
#define get_power_IgnoreArg_power() get_power_CMockIgnoreArg_power(__LINE__)
void get_power_CMockIgnoreArg_power(UNITY_LINE_TYPE cmock_line);
#define set_power_change_notif_thresholds_IgnoreAndReturn(cmock_retval) set_power_change_notif_thresholds_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void set_power_change_notif_thresholds_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define set_power_change_notif_thresholds_StopIgnore() set_power_change_notif_thresholds_CMockStopIgnore()
void set_power_change_notif_thresholds_CMockStopIgnore(void);
#define set_power_change_notif_thresholds_ExpectAnyArgsAndReturn(cmock_retval) set_power_change_notif_thresholds_CMockExpectAnyArgsAndReturn(__LINE__, cmock_retval)
void set_power_change_notif_thresholds_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return);
#define set_power_change_notif_thresholds_ExpectAndReturn(id, threshold_low, threshold_high, cmock_retval) set_power_change_notif_thresholds_CMockExpectAndReturn(__LINE__, id, threshold_low, threshold_high, cmock_retval)
void set_power_change_notif_thresholds_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, fwk_id_t id, uint32_t threshold_low, uint32_t threshold_high, int cmock_to_return);
typedef int (* CMOCK_set_power_change_notif_thresholds_CALLBACK)(fwk_id_t id, uint32_t threshold_low, uint32_t threshold_high, int cmock_num_calls);
void set_power_change_notif_thresholds_AddCallback(CMOCK_set_power_change_notif_thresholds_CALLBACK Callback);
void set_power_change_notif_thresholds_Stub(CMOCK_set_power_change_notif_thresholds_CALLBACK Callback);
#define set_power_change_notif_thresholds_StubWithCallback set_power_change_notif_thresholds_Stub
#define set_power_change_notif_thresholds_IgnoreArg_id() set_power_change_notif_thresholds_CMockIgnoreArg_id(__LINE__)
void set_power_change_notif_thresholds_CMockIgnoreArg_id(UNITY_LINE_TYPE cmock_line);
#define set_power_change_notif_thresholds_IgnoreArg_threshold_low() set_power_change_notif_thresholds_CMockIgnoreArg_threshold_low(__LINE__)
void set_power_change_notif_thresholds_CMockIgnoreArg_threshold_low(UNITY_LINE_TYPE cmock_line);
#define set_power_change_notif_thresholds_IgnoreArg_threshold_high() set_power_change_notif_thresholds_CMockIgnoreArg_threshold_high(__LINE__)
void set_power_change_notif_thresholds_CMockIgnoreArg_threshold_high(UNITY_LINE_TYPE cmock_line);

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif
