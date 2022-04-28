/* AUTOGENERATED FILE. DO NOT EDIT. */
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "internal/Mockfwk_core_internal.h"

static const char* CMockString___fwk_get_current_event = "__fwk_get_current_event";
static const char* CMockString___fwk_init = "__fwk_init";
static const char* CMockString___fwk_put_event = "__fwk_put_event";
static const char* CMockString___fwk_put_event_light = "__fwk_put_event_light";
static const char* CMockString___fwk_put_notification = "__fwk_put_notification";
static const char* CMockString___fwk_run_main_loop = "__fwk_run_main_loop";
static const char* CMockString_event = "event";
static const char* CMockString_event_count = "event_count";

typedef struct _CMOCK___fwk_init_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  int ReturnVal;
  size_t Expected_event_count;

} CMOCK___fwk_init_CALL_INSTANCE;

typedef struct _CMOCK___fwk_run_main_loop_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;

} CMOCK___fwk_run_main_loop_CALL_INSTANCE;

typedef struct _CMOCK___fwk_get_current_event_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  const struct fwk_event* ReturnVal;

} CMOCK___fwk_get_current_event_CALL_INSTANCE;

typedef struct _CMOCK___fwk_put_notification_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  int ReturnVal;
  struct fwk_event* Expected_event;
  char ReturnThruPtr_event_Used;
  struct fwk_event* ReturnThruPtr_event_Val;
  size_t ReturnThruPtr_event_Size;

} CMOCK___fwk_put_notification_CALL_INSTANCE;

typedef struct _CMOCK___fwk_put_event_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  int ReturnVal;
  struct fwk_event* Expected_event;
  char ReturnThruPtr_event_Used;
  struct fwk_event* ReturnThruPtr_event_Val;
  size_t ReturnThruPtr_event_Size;

} CMOCK___fwk_put_event_CALL_INSTANCE;

typedef struct _CMOCK___fwk_put_event_light_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  int ReturnVal;
  struct fwk_event_light* Expected_event;
  char ReturnThruPtr_event_Used;
  struct fwk_event_light* ReturnThruPtr_event_Val;
  size_t ReturnThruPtr_event_Size;

} CMOCK___fwk_put_event_light_CALL_INSTANCE;

static struct Mockfwk_core_internalInstance
{
  char __fwk_init_CallbackBool;
  CMOCK___fwk_init_CALLBACK __fwk_init_CallbackFunctionPointer;
  int __fwk_init_CallbackCalls;
  CMOCK_MEM_INDEX_TYPE __fwk_init_CallInstance;
  char __fwk_run_main_loop_CallbackBool;
  CMOCK___fwk_run_main_loop_CALLBACK __fwk_run_main_loop_CallbackFunctionPointer;
  int __fwk_run_main_loop_CallbackCalls;
  CMOCK_MEM_INDEX_TYPE __fwk_run_main_loop_CallInstance;
  char __fwk_get_current_event_CallbackBool;
  CMOCK___fwk_get_current_event_CALLBACK __fwk_get_current_event_CallbackFunctionPointer;
  int __fwk_get_current_event_CallbackCalls;
  CMOCK_MEM_INDEX_TYPE __fwk_get_current_event_CallInstance;
  char __fwk_put_notification_CallbackBool;
  CMOCK___fwk_put_notification_CALLBACK __fwk_put_notification_CallbackFunctionPointer;
  int __fwk_put_notification_CallbackCalls;
  CMOCK_MEM_INDEX_TYPE __fwk_put_notification_CallInstance;
  char __fwk_put_event_CallbackBool;
  CMOCK___fwk_put_event_CALLBACK __fwk_put_event_CallbackFunctionPointer;
  int __fwk_put_event_CallbackCalls;
  CMOCK_MEM_INDEX_TYPE __fwk_put_event_CallInstance;
  char __fwk_put_event_light_CallbackBool;
  CMOCK___fwk_put_event_light_CALLBACK __fwk_put_event_light_CallbackFunctionPointer;
  int __fwk_put_event_light_CallbackCalls;
  CMOCK_MEM_INDEX_TYPE __fwk_put_event_light_CallInstance;
} Mock;

extern jmp_buf AbortFrame;

void Mockfwk_core_internal_Verify(void)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_MEM_INDEX_TYPE call_instance;
  call_instance = Mock.__fwk_init_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString___fwk_init);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  if (Mock.__fwk_init_CallbackFunctionPointer != NULL)
  {
    call_instance = CMOCK_GUTS_NONE;
    (void)call_instance;
  }
  call_instance = Mock.__fwk_run_main_loop_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString___fwk_run_main_loop);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  if (Mock.__fwk_run_main_loop_CallbackFunctionPointer != NULL)
  {
    call_instance = CMOCK_GUTS_NONE;
    (void)call_instance;
  }
  call_instance = Mock.__fwk_get_current_event_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString___fwk_get_current_event);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  if (Mock.__fwk_get_current_event_CallbackFunctionPointer != NULL)
  {
    call_instance = CMOCK_GUTS_NONE;
    (void)call_instance;
  }
  call_instance = Mock.__fwk_put_notification_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString___fwk_put_notification);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  if (Mock.__fwk_put_notification_CallbackFunctionPointer != NULL)
  {
    call_instance = CMOCK_GUTS_NONE;
    (void)call_instance;
  }
  call_instance = Mock.__fwk_put_event_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString___fwk_put_event);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  if (Mock.__fwk_put_event_CallbackFunctionPointer != NULL)
  {
    call_instance = CMOCK_GUTS_NONE;
    (void)call_instance;
  }
  call_instance = Mock.__fwk_put_event_light_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString___fwk_put_event_light);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  if (Mock.__fwk_put_event_light_CallbackFunctionPointer != NULL)
  {
    call_instance = CMOCK_GUTS_NONE;
    (void)call_instance;
  }
}

void Mockfwk_core_internal_Init(void)
{
  Mockfwk_core_internal_Destroy();
}

void Mockfwk_core_internal_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
}

int __fwk_init(size_t event_count)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK___fwk_init_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString___fwk_init);
  cmock_call_instance = (CMOCK___fwk_init_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.__fwk_init_CallInstance);
  Mock.__fwk_init_CallInstance = CMock_Guts_MemNext(Mock.__fwk_init_CallInstance);
  if (!Mock.__fwk_init_CallbackBool &&
      Mock.__fwk_init_CallbackFunctionPointer != NULL)
  {
    int cmock_cb_ret = Mock.__fwk_init_CallbackFunctionPointer(event_count, Mock.__fwk_init_CallbackCalls++);
    UNITY_CLR_DETAILS();
    return cmock_cb_ret;
  }
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (!cmock_call_instance->ExpectAnyArgsBool)
  {
  {
    UNITY_SET_DETAILS(CMockString___fwk_init,CMockString_event_count);
    UNITY_TEST_ASSERT_EQUAL_MEMORY((void*)(&cmock_call_instance->Expected_event_count), (void*)(&event_count), sizeof(size_t), cmock_line, CMockStringMismatch);
  }
  }
  if (Mock.__fwk_init_CallbackFunctionPointer != NULL)
  {
    cmock_call_instance->ReturnVal = Mock.__fwk_init_CallbackFunctionPointer(event_count, Mock.__fwk_init_CallbackCalls++);
  }
  UNITY_CLR_DETAILS();
  return cmock_call_instance->ReturnVal;
}

void CMockExpectParameters___fwk_init(CMOCK___fwk_init_CALL_INSTANCE* cmock_call_instance, size_t event_count);
void CMockExpectParameters___fwk_init(CMOCK___fwk_init_CALL_INSTANCE* cmock_call_instance, size_t event_count)
{
  memcpy((void*)(&cmock_call_instance->Expected_event_count), (void*)(&event_count),
         sizeof(size_t[sizeof(event_count) == sizeof(size_t) ? 1 : -1])); /* add size_t to :treat_as_array if this causes an error */
}

void __fwk_init_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_init_CALL_INSTANCE));
  CMOCK___fwk_init_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_init_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_init_CallInstance = CMock_Guts_MemChain(Mock.__fwk_init_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ReturnVal = cmock_to_return;
  cmock_call_instance->ExpectAnyArgsBool = (char)1;
}

void __fwk_init_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, size_t event_count, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_init_CALL_INSTANCE));
  CMOCK___fwk_init_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_init_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_init_CallInstance = CMock_Guts_MemChain(Mock.__fwk_init_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  CMockExpectParameters___fwk_init(cmock_call_instance, event_count);
  cmock_call_instance->ReturnVal = cmock_to_return;
}

void __fwk_init_AddCallback(CMOCK___fwk_init_CALLBACK Callback)
{
  Mock.__fwk_init_CallbackBool = (char)1;
  Mock.__fwk_init_CallbackFunctionPointer = Callback;
}

void __fwk_init_Stub(CMOCK___fwk_init_CALLBACK Callback)
{
  Mock.__fwk_init_CallbackBool = (char)0;
  Mock.__fwk_init_CallbackFunctionPointer = Callback;
}

void __fwk_run_main_loop(void)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK___fwk_run_main_loop_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString___fwk_run_main_loop);
  cmock_call_instance = (CMOCK___fwk_run_main_loop_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.__fwk_run_main_loop_CallInstance);
  Mock.__fwk_run_main_loop_CallInstance = CMock_Guts_MemNext(Mock.__fwk_run_main_loop_CallInstance);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  UNITY_CLR_DETAILS();
  for (;;) {

  }
}

void __fwk_run_main_loop_CMockExpect(UNITY_LINE_TYPE cmock_line)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_run_main_loop_CALL_INSTANCE));
  CMOCK___fwk_run_main_loop_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_run_main_loop_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_run_main_loop_CallInstance = CMock_Guts_MemChain(Mock.__fwk_run_main_loop_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
}

void __fwk_run_main_loop_AddCallback(CMOCK___fwk_run_main_loop_CALLBACK Callback)
{
  Mock.__fwk_run_main_loop_CallbackBool = (char)1;
  Mock.__fwk_run_main_loop_CallbackFunctionPointer = Callback;
}

void __fwk_run_main_loop_Stub(CMOCK___fwk_run_main_loop_CALLBACK Callback)
{
  Mock.__fwk_run_main_loop_CallbackBool = (char)0;
  Mock.__fwk_run_main_loop_CallbackFunctionPointer = Callback;
}

const struct fwk_event* __fwk_get_current_event(void)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK___fwk_get_current_event_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString___fwk_get_current_event);
  cmock_call_instance = (CMOCK___fwk_get_current_event_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.__fwk_get_current_event_CallInstance);
  Mock.__fwk_get_current_event_CallInstance = CMock_Guts_MemNext(Mock.__fwk_get_current_event_CallInstance);
  if (!Mock.__fwk_get_current_event_CallbackBool &&
      Mock.__fwk_get_current_event_CallbackFunctionPointer != NULL)
  {
    const struct fwk_event* cmock_cb_ret = Mock.__fwk_get_current_event_CallbackFunctionPointer(Mock.__fwk_get_current_event_CallbackCalls++);
    UNITY_CLR_DETAILS();
    return cmock_cb_ret;
  }
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (Mock.__fwk_get_current_event_CallbackFunctionPointer != NULL)
  {
    cmock_call_instance->ReturnVal = Mock.__fwk_get_current_event_CallbackFunctionPointer(Mock.__fwk_get_current_event_CallbackCalls++);
  }
  UNITY_CLR_DETAILS();
  return cmock_call_instance->ReturnVal;
}

void __fwk_get_current_event_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, const struct fwk_event* cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_get_current_event_CALL_INSTANCE));
  CMOCK___fwk_get_current_event_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_get_current_event_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_get_current_event_CallInstance = CMock_Guts_MemChain(Mock.__fwk_get_current_event_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ReturnVal = cmock_to_return;
}

void __fwk_get_current_event_AddCallback(CMOCK___fwk_get_current_event_CALLBACK Callback)
{
  Mock.__fwk_get_current_event_CallbackBool = (char)1;
  Mock.__fwk_get_current_event_CallbackFunctionPointer = Callback;
}

void __fwk_get_current_event_Stub(CMOCK___fwk_get_current_event_CALLBACK Callback)
{
  Mock.__fwk_get_current_event_CallbackBool = (char)0;
  Mock.__fwk_get_current_event_CallbackFunctionPointer = Callback;
}

int __fwk_put_notification(struct fwk_event* event)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK___fwk_put_notification_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString___fwk_put_notification);
  cmock_call_instance = (CMOCK___fwk_put_notification_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.__fwk_put_notification_CallInstance);
  Mock.__fwk_put_notification_CallInstance = CMock_Guts_MemNext(Mock.__fwk_put_notification_CallInstance);
  if (!Mock.__fwk_put_notification_CallbackBool &&
      Mock.__fwk_put_notification_CallbackFunctionPointer != NULL)
  {
    int cmock_cb_ret = Mock.__fwk_put_notification_CallbackFunctionPointer(event, Mock.__fwk_put_notification_CallbackCalls++);
    UNITY_CLR_DETAILS();
    return cmock_cb_ret;
  }
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (!cmock_call_instance->ExpectAnyArgsBool)
  {
  {
    UNITY_SET_DETAILS(CMockString___fwk_put_notification,CMockString_event);
    UNITY_TEST_ASSERT_EQUAL_MEMORY((void*)(cmock_call_instance->Expected_event), (void*)(event), sizeof(struct fwk_event), cmock_line, CMockStringMismatch);
  }
  }
  if (Mock.__fwk_put_notification_CallbackFunctionPointer != NULL)
  {
    cmock_call_instance->ReturnVal = Mock.__fwk_put_notification_CallbackFunctionPointer(event, Mock.__fwk_put_notification_CallbackCalls++);
  }
  if (cmock_call_instance->ReturnThruPtr_event_Used)
  {
    UNITY_TEST_ASSERT_NOT_NULL(event, cmock_line, CMockStringPtrIsNULL);
    memcpy((void*)event, (void*)cmock_call_instance->ReturnThruPtr_event_Val,
      cmock_call_instance->ReturnThruPtr_event_Size);
  }
  UNITY_CLR_DETAILS();
  return cmock_call_instance->ReturnVal;
}

void CMockExpectParameters___fwk_put_notification(CMOCK___fwk_put_notification_CALL_INSTANCE* cmock_call_instance, struct fwk_event* event);
void CMockExpectParameters___fwk_put_notification(CMOCK___fwk_put_notification_CALL_INSTANCE* cmock_call_instance, struct fwk_event* event)
{
  cmock_call_instance->Expected_event = event;
  cmock_call_instance->ReturnThruPtr_event_Used = 0;
}

void __fwk_put_notification_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_put_notification_CALL_INSTANCE));
  CMOCK___fwk_put_notification_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_notification_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_put_notification_CallInstance = CMock_Guts_MemChain(Mock.__fwk_put_notification_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ReturnVal = cmock_to_return;
  cmock_call_instance->ExpectAnyArgsBool = (char)1;
}

void __fwk_put_notification_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct fwk_event* event, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_put_notification_CALL_INSTANCE));
  CMOCK___fwk_put_notification_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_notification_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_put_notification_CallInstance = CMock_Guts_MemChain(Mock.__fwk_put_notification_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  CMockExpectParameters___fwk_put_notification(cmock_call_instance, event);
  cmock_call_instance->ReturnVal = cmock_to_return;
}

void __fwk_put_notification_AddCallback(CMOCK___fwk_put_notification_CALLBACK Callback)
{
  Mock.__fwk_put_notification_CallbackBool = (char)1;
  Mock.__fwk_put_notification_CallbackFunctionPointer = Callback;
}

void __fwk_put_notification_Stub(CMOCK___fwk_put_notification_CALLBACK Callback)
{
  Mock.__fwk_put_notification_CallbackBool = (char)0;
  Mock.__fwk_put_notification_CallbackFunctionPointer = Callback;
}

void __fwk_put_notification_CMockReturnMemThruPtr_event(UNITY_LINE_TYPE cmock_line, struct fwk_event* event, size_t cmock_size)
{
  CMOCK___fwk_put_notification_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_notification_CALL_INSTANCE*)CMock_Guts_GetAddressFor(CMock_Guts_MemEndOfChain(Mock.__fwk_put_notification_CallInstance));
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringPtrPreExp);
  cmock_call_instance->ReturnThruPtr_event_Used = 1;
  cmock_call_instance->ReturnThruPtr_event_Val = event;
  cmock_call_instance->ReturnThruPtr_event_Size = cmock_size;
}

int __fwk_put_event(struct fwk_event* event)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK___fwk_put_event_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString___fwk_put_event);
  cmock_call_instance = (CMOCK___fwk_put_event_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.__fwk_put_event_CallInstance);
  Mock.__fwk_put_event_CallInstance = CMock_Guts_MemNext(Mock.__fwk_put_event_CallInstance);
  if (!Mock.__fwk_put_event_CallbackBool &&
      Mock.__fwk_put_event_CallbackFunctionPointer != NULL)
  {
    int cmock_cb_ret = Mock.__fwk_put_event_CallbackFunctionPointer(event, Mock.__fwk_put_event_CallbackCalls++);
    UNITY_CLR_DETAILS();
    return cmock_cb_ret;
  }
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (!cmock_call_instance->ExpectAnyArgsBool)
  {
  {
    UNITY_SET_DETAILS(CMockString___fwk_put_event,CMockString_event);
    UNITY_TEST_ASSERT_EQUAL_MEMORY((void*)(cmock_call_instance->Expected_event), (void*)(event), sizeof(struct fwk_event), cmock_line, CMockStringMismatch);
  }
  }
  if (Mock.__fwk_put_event_CallbackFunctionPointer != NULL)
  {
    cmock_call_instance->ReturnVal = Mock.__fwk_put_event_CallbackFunctionPointer(event, Mock.__fwk_put_event_CallbackCalls++);
  }
  if (cmock_call_instance->ReturnThruPtr_event_Used)
  {
    UNITY_TEST_ASSERT_NOT_NULL(event, cmock_line, CMockStringPtrIsNULL);
    memcpy((void*)event, (void*)cmock_call_instance->ReturnThruPtr_event_Val,
      cmock_call_instance->ReturnThruPtr_event_Size);
  }
  UNITY_CLR_DETAILS();
  return cmock_call_instance->ReturnVal;
}

void CMockExpectParameters___fwk_put_event(CMOCK___fwk_put_event_CALL_INSTANCE* cmock_call_instance, struct fwk_event* event);
void CMockExpectParameters___fwk_put_event(CMOCK___fwk_put_event_CALL_INSTANCE* cmock_call_instance, struct fwk_event* event)
{
  cmock_call_instance->Expected_event = event;
  cmock_call_instance->ReturnThruPtr_event_Used = 0;
}

void __fwk_put_event_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_put_event_CALL_INSTANCE));
  CMOCK___fwk_put_event_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_event_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_put_event_CallInstance = CMock_Guts_MemChain(Mock.__fwk_put_event_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ReturnVal = cmock_to_return;
  cmock_call_instance->ExpectAnyArgsBool = (char)1;
}

void __fwk_put_event_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct fwk_event* event, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_put_event_CALL_INSTANCE));
  CMOCK___fwk_put_event_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_event_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_put_event_CallInstance = CMock_Guts_MemChain(Mock.__fwk_put_event_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  CMockExpectParameters___fwk_put_event(cmock_call_instance, event);
  cmock_call_instance->ReturnVal = cmock_to_return;
}

void __fwk_put_event_AddCallback(CMOCK___fwk_put_event_CALLBACK Callback)
{
  Mock.__fwk_put_event_CallbackBool = (char)1;
  Mock.__fwk_put_event_CallbackFunctionPointer = Callback;
}

void __fwk_put_event_Stub(CMOCK___fwk_put_event_CALLBACK Callback)
{
  Mock.__fwk_put_event_CallbackBool = (char)0;
  Mock.__fwk_put_event_CallbackFunctionPointer = Callback;
}

void __fwk_put_event_CMockReturnMemThruPtr_event(UNITY_LINE_TYPE cmock_line, struct fwk_event* event, size_t cmock_size)
{
  CMOCK___fwk_put_event_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_event_CALL_INSTANCE*)CMock_Guts_GetAddressFor(CMock_Guts_MemEndOfChain(Mock.__fwk_put_event_CallInstance));
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringPtrPreExp);
  cmock_call_instance->ReturnThruPtr_event_Used = 1;
  cmock_call_instance->ReturnThruPtr_event_Val = event;
  cmock_call_instance->ReturnThruPtr_event_Size = cmock_size;
}

int __fwk_put_event_light(struct fwk_event_light* event)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK___fwk_put_event_light_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString___fwk_put_event_light);
  cmock_call_instance = (CMOCK___fwk_put_event_light_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.__fwk_put_event_light_CallInstance);
  Mock.__fwk_put_event_light_CallInstance = CMock_Guts_MemNext(Mock.__fwk_put_event_light_CallInstance);
  if (!Mock.__fwk_put_event_light_CallbackBool &&
      Mock.__fwk_put_event_light_CallbackFunctionPointer != NULL)
  {
    int cmock_cb_ret = Mock.__fwk_put_event_light_CallbackFunctionPointer(event, Mock.__fwk_put_event_light_CallbackCalls++);
    UNITY_CLR_DETAILS();
    return cmock_cb_ret;
  }
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (!cmock_call_instance->ExpectAnyArgsBool)
  {
  {
    UNITY_SET_DETAILS(CMockString___fwk_put_event_light,CMockString_event);
    UNITY_TEST_ASSERT_EQUAL_MEMORY((void*)(cmock_call_instance->Expected_event), (void*)(event), sizeof(struct fwk_event_light), cmock_line, CMockStringMismatch);
  }
  }
  if (Mock.__fwk_put_event_light_CallbackFunctionPointer != NULL)
  {
    cmock_call_instance->ReturnVal = Mock.__fwk_put_event_light_CallbackFunctionPointer(event, Mock.__fwk_put_event_light_CallbackCalls++);
  }
  if (cmock_call_instance->ReturnThruPtr_event_Used)
  {
    UNITY_TEST_ASSERT_NOT_NULL(event, cmock_line, CMockStringPtrIsNULL);
    memcpy((void*)event, (void*)cmock_call_instance->ReturnThruPtr_event_Val,
      cmock_call_instance->ReturnThruPtr_event_Size);
  }
  UNITY_CLR_DETAILS();
  return cmock_call_instance->ReturnVal;
}

void CMockExpectParameters___fwk_put_event_light(CMOCK___fwk_put_event_light_CALL_INSTANCE* cmock_call_instance, struct fwk_event_light* event);
void CMockExpectParameters___fwk_put_event_light(CMOCK___fwk_put_event_light_CALL_INSTANCE* cmock_call_instance, struct fwk_event_light* event)
{
  cmock_call_instance->Expected_event = event;
  cmock_call_instance->ReturnThruPtr_event_Used = 0;
}

void __fwk_put_event_light_CMockExpectAnyArgsAndReturn(UNITY_LINE_TYPE cmock_line, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_put_event_light_CALL_INSTANCE));
  CMOCK___fwk_put_event_light_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_event_light_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_put_event_light_CallInstance = CMock_Guts_MemChain(Mock.__fwk_put_event_light_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ReturnVal = cmock_to_return;
  cmock_call_instance->ExpectAnyArgsBool = (char)1;
}

void __fwk_put_event_light_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, struct fwk_event_light* event, int cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK___fwk_put_event_light_CALL_INSTANCE));
  CMOCK___fwk_put_event_light_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_event_light_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.__fwk_put_event_light_CallInstance = CMock_Guts_MemChain(Mock.__fwk_put_event_light_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  CMockExpectParameters___fwk_put_event_light(cmock_call_instance, event);
  cmock_call_instance->ReturnVal = cmock_to_return;
}

void __fwk_put_event_light_AddCallback(CMOCK___fwk_put_event_light_CALLBACK Callback)
{
  Mock.__fwk_put_event_light_CallbackBool = (char)1;
  Mock.__fwk_put_event_light_CallbackFunctionPointer = Callback;
}

void __fwk_put_event_light_Stub(CMOCK___fwk_put_event_light_CALLBACK Callback)
{
  Mock.__fwk_put_event_light_CallbackBool = (char)0;
  Mock.__fwk_put_event_light_CallbackFunctionPointer = Callback;
}

void __fwk_put_event_light_CMockReturnMemThruPtr_event(UNITY_LINE_TYPE cmock_line, struct fwk_event_light* event, size_t cmock_size)
{
  CMOCK___fwk_put_event_light_CALL_INSTANCE* cmock_call_instance = (CMOCK___fwk_put_event_light_CALL_INSTANCE*)CMock_Guts_GetAddressFor(CMock_Guts_MemEndOfChain(Mock.__fwk_put_event_light_CallInstance));
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringPtrPreExp);
  cmock_call_instance->ReturnThruPtr_event_Used = 1;
  cmock_call_instance->ReturnThruPtr_event_Val = event;
  cmock_call_instance->ReturnThruPtr_event_Size = cmock_size;
}

