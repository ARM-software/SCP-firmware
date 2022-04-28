/* AUTOGENERATED FILE. DO NOT EDIT. */
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "Mockfwk_string.h"

static const char* CMockString_ch = "ch";
static const char* CMockString_count = "count";
static const char* CMockString_dest = "dest";
static const char* CMockString_fwk_str_memcpy = "fwk_str_memcpy";
static const char* CMockString_fwk_str_memset = "fwk_str_memset";
static const char* CMockString_fwk_str_strncpy = "fwk_str_strncpy";
static const char* CMockString_src = "src";

typedef struct _CMOCK_fwk_str_memset_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  void* Expected_dest;
  int Expected_ch;
  size_t Expected_count;

} CMOCK_fwk_str_memset_CALL_INSTANCE;

typedef struct _CMOCK_fwk_str_memcpy_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  void* Expected_dest;
  const void* Expected_src;
  size_t Expected_count;

} CMOCK_fwk_str_memcpy_CALL_INSTANCE;

typedef struct _CMOCK_fwk_str_strncpy_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  char ExpectAnyArgsBool;
  char* Expected_dest;
  const char* Expected_src;
  size_t Expected_count;

} CMOCK_fwk_str_strncpy_CALL_INSTANCE;

static struct Mockfwk_stringInstance
{
  CMOCK_MEM_INDEX_TYPE fwk_str_memset_CallInstance;
  CMOCK_MEM_INDEX_TYPE fwk_str_memcpy_CallInstance;
  CMOCK_MEM_INDEX_TYPE fwk_str_strncpy_CallInstance;
} Mock;

extern jmp_buf AbortFrame;

void Mockfwk_string_Verify(void)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_MEM_INDEX_TYPE call_instance;
  call_instance = Mock.fwk_str_memset_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString_fwk_str_memset);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  call_instance = Mock.fwk_str_memcpy_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString_fwk_str_memcpy);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  call_instance = Mock.fwk_str_strncpy_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString_fwk_str_strncpy);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
}

void Mockfwk_string_Init(void)
{
  Mockfwk_string_Destroy();
}

void Mockfwk_string_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
}

void fwk_str_memset(void* dest, int ch, size_t count)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_fwk_str_memset_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString_fwk_str_memset);
  cmock_call_instance = (CMOCK_fwk_str_memset_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.fwk_str_memset_CallInstance);
  Mock.fwk_str_memset_CallInstance = CMock_Guts_MemNext(Mock.fwk_str_memset_CallInstance);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (!cmock_call_instance->ExpectAnyArgsBool)
  {
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_memset,CMockString_dest);
    if (cmock_call_instance->Expected_dest == NULL)
      { UNITY_TEST_ASSERT_NULL(dest, cmock_line, CMockStringExpNULL); }
    else
      { UNITY_TEST_ASSERT_EQUAL_HEX8_ARRAY(cmock_call_instance->Expected_dest, dest, 1, cmock_line, CMockStringMismatch); }
  }
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_memset,CMockString_ch);
    UNITY_TEST_ASSERT_EQUAL_INT(cmock_call_instance->Expected_ch, ch, cmock_line, CMockStringMismatch);
  }
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_memset,CMockString_count);
    UNITY_TEST_ASSERT_EQUAL_MEMORY((void*)(&cmock_call_instance->Expected_count), (void*)(&count), sizeof(size_t), cmock_line, CMockStringMismatch);
  }
  }
  UNITY_CLR_DETAILS();
}

void CMockExpectParameters_fwk_str_memset(CMOCK_fwk_str_memset_CALL_INSTANCE* cmock_call_instance, void* dest, int ch, size_t count);
void CMockExpectParameters_fwk_str_memset(CMOCK_fwk_str_memset_CALL_INSTANCE* cmock_call_instance, void* dest, int ch, size_t count)
{
  cmock_call_instance->Expected_dest = dest;
  cmock_call_instance->Expected_ch = ch;
  memcpy((void*)(&cmock_call_instance->Expected_count), (void*)(&count),
         sizeof(size_t[sizeof(count) == sizeof(size_t) ? 1 : -1])); /* add size_t to :treat_as_array if this causes an error */
}

void fwk_str_memset_CMockExpectAnyArgs(UNITY_LINE_TYPE cmock_line)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_fwk_str_memset_CALL_INSTANCE));
  CMOCK_fwk_str_memset_CALL_INSTANCE* cmock_call_instance = (CMOCK_fwk_str_memset_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.fwk_str_memset_CallInstance = CMock_Guts_MemChain(Mock.fwk_str_memset_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ExpectAnyArgsBool = (char)1;
}

void fwk_str_memset_CMockExpect(UNITY_LINE_TYPE cmock_line, void* dest, int ch, size_t count)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_fwk_str_memset_CALL_INSTANCE));
  CMOCK_fwk_str_memset_CALL_INSTANCE* cmock_call_instance = (CMOCK_fwk_str_memset_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.fwk_str_memset_CallInstance = CMock_Guts_MemChain(Mock.fwk_str_memset_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  CMockExpectParameters_fwk_str_memset(cmock_call_instance, dest, ch, count);
}

void fwk_str_memcpy(void* dest, const void* src, size_t count)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_fwk_str_memcpy_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString_fwk_str_memcpy);
  cmock_call_instance = (CMOCK_fwk_str_memcpy_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.fwk_str_memcpy_CallInstance);
  Mock.fwk_str_memcpy_CallInstance = CMock_Guts_MemNext(Mock.fwk_str_memcpy_CallInstance);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (!cmock_call_instance->ExpectAnyArgsBool)
  {
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_memcpy,CMockString_dest);
    if (cmock_call_instance->Expected_dest == NULL)
      { UNITY_TEST_ASSERT_NULL(dest, cmock_line, CMockStringExpNULL); }
    else
      { UNITY_TEST_ASSERT_EQUAL_HEX8_ARRAY(cmock_call_instance->Expected_dest, dest, 1, cmock_line, CMockStringMismatch); }
  }
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_memcpy,CMockString_src);
    if (cmock_call_instance->Expected_src == NULL)
      { UNITY_TEST_ASSERT_NULL(src, cmock_line, CMockStringExpNULL); }
    else
      { UNITY_TEST_ASSERT_EQUAL_HEX8_ARRAY(cmock_call_instance->Expected_src, src, 1, cmock_line, CMockStringMismatch); }
  }
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_memcpy,CMockString_count);
    UNITY_TEST_ASSERT_EQUAL_MEMORY((void*)(&cmock_call_instance->Expected_count), (void*)(&count), sizeof(size_t), cmock_line, CMockStringMismatch);
  }
  }
  UNITY_CLR_DETAILS();
}

void CMockExpectParameters_fwk_str_memcpy(CMOCK_fwk_str_memcpy_CALL_INSTANCE* cmock_call_instance, void* dest, const void* src, size_t count);
void CMockExpectParameters_fwk_str_memcpy(CMOCK_fwk_str_memcpy_CALL_INSTANCE* cmock_call_instance, void* dest, const void* src, size_t count)
{
  cmock_call_instance->Expected_dest = dest;
  cmock_call_instance->Expected_src = src;
  memcpy((void*)(&cmock_call_instance->Expected_count), (void*)(&count),
         sizeof(size_t[sizeof(count) == sizeof(size_t) ? 1 : -1])); /* add size_t to :treat_as_array if this causes an error */
}

void fwk_str_memcpy_CMockExpectAnyArgs(UNITY_LINE_TYPE cmock_line)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_fwk_str_memcpy_CALL_INSTANCE));
  CMOCK_fwk_str_memcpy_CALL_INSTANCE* cmock_call_instance = (CMOCK_fwk_str_memcpy_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.fwk_str_memcpy_CallInstance = CMock_Guts_MemChain(Mock.fwk_str_memcpy_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ExpectAnyArgsBool = (char)1;
}

void fwk_str_memcpy_CMockExpect(UNITY_LINE_TYPE cmock_line, void* dest, const void* src, size_t count)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_fwk_str_memcpy_CALL_INSTANCE));
  CMOCK_fwk_str_memcpy_CALL_INSTANCE* cmock_call_instance = (CMOCK_fwk_str_memcpy_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.fwk_str_memcpy_CallInstance = CMock_Guts_MemChain(Mock.fwk_str_memcpy_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  CMockExpectParameters_fwk_str_memcpy(cmock_call_instance, dest, src, count);
}

void fwk_str_strncpy(char* dest, const char* src, size_t count)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_fwk_str_strncpy_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString_fwk_str_strncpy);
  cmock_call_instance = (CMOCK_fwk_str_strncpy_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.fwk_str_strncpy_CallInstance);
  Mock.fwk_str_strncpy_CallInstance = CMock_Guts_MemNext(Mock.fwk_str_strncpy_CallInstance);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  if (!cmock_call_instance->ExpectAnyArgsBool)
  {
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_strncpy,CMockString_dest);
    UNITY_TEST_ASSERT_EQUAL_STRING(cmock_call_instance->Expected_dest, dest, cmock_line, CMockStringMismatch);
  }
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_strncpy,CMockString_src);
    UNITY_TEST_ASSERT_EQUAL_STRING(cmock_call_instance->Expected_src, src, cmock_line, CMockStringMismatch);
  }
  {
    UNITY_SET_DETAILS(CMockString_fwk_str_strncpy,CMockString_count);
    UNITY_TEST_ASSERT_EQUAL_MEMORY((void*)(&cmock_call_instance->Expected_count), (void*)(&count), sizeof(size_t), cmock_line, CMockStringMismatch);
  }
  }
  UNITY_CLR_DETAILS();
}

void CMockExpectParameters_fwk_str_strncpy(CMOCK_fwk_str_strncpy_CALL_INSTANCE* cmock_call_instance, char* dest, const char* src, size_t count);
void CMockExpectParameters_fwk_str_strncpy(CMOCK_fwk_str_strncpy_CALL_INSTANCE* cmock_call_instance, char* dest, const char* src, size_t count)
{
  cmock_call_instance->Expected_dest = dest;
  cmock_call_instance->Expected_src = src;
  memcpy((void*)(&cmock_call_instance->Expected_count), (void*)(&count),
         sizeof(size_t[sizeof(count) == sizeof(size_t) ? 1 : -1])); /* add size_t to :treat_as_array if this causes an error */
}

void fwk_str_strncpy_CMockExpectAnyArgs(UNITY_LINE_TYPE cmock_line)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_fwk_str_strncpy_CALL_INSTANCE));
  CMOCK_fwk_str_strncpy_CALL_INSTANCE* cmock_call_instance = (CMOCK_fwk_str_strncpy_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.fwk_str_strncpy_CallInstance = CMock_Guts_MemChain(Mock.fwk_str_strncpy_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  cmock_call_instance->ExpectAnyArgsBool = (char)1;
}

void fwk_str_strncpy_CMockExpect(UNITY_LINE_TYPE cmock_line, char* dest, const char* src, size_t count)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_fwk_str_strncpy_CALL_INSTANCE));
  CMOCK_fwk_str_strncpy_CALL_INSTANCE* cmock_call_instance = (CMOCK_fwk_str_strncpy_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.fwk_str_strncpy_CallInstance = CMock_Guts_MemChain(Mock.fwk_str_strncpy_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  cmock_call_instance->ExpectAnyArgsBool = (char)0;
  CMockExpectParameters_fwk_str_strncpy(cmock_call_instance, dest, src, count);
}

