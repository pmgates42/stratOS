#ifndef APP_TEST_H
#define APP_TEST_H

#include "unity.h"

void APP_test_setup(void);
void APP_test_teardown(void);

#define APP_test_begin() UnityBegin(__FILE__)
#define APP_test_run(test_function) RUN_TEST(test_function, __LINE__)
#define APP_test_end() UnityEnd()

#define APP_test_assert_greater_than_uint32(expected, actual) \
    TEST_ASSERT_GREATER_THAN_UINT32((expected), (actual))
#define APP_test_assert_greater_or_equal_uint32(expected, actual) \
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32((expected), (actual))
#define APP_test_assert_less_or_equal_uint64(expected, actual) \
    TEST_ASSERT_LESS_OR_EQUAL_UINT64((expected), (actual))
#define APP_test_ignore_message(message) TEST_IGNORE_MESSAGE(message)

#endif
