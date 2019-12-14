#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "log.h"
#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

extern void __real_log_init(void);
extern void __real_log_info(const char* fmt, ...);
extern void __real_log_warn(const char* fmt, ...);
extern Log* __real_log_dequeue(void);

static int test_log_setup(void** state)
{
    __real_log_init();

    return 0;
}

static void test_log_info_writes_to_log_buffer(UNUSED void** state)
{
    __real_log_info("Test Message %d", 1, 0);

    Log* log = __real_log_dequeue();

    assert_int_not_equal(log->msgLen, 0);
    assert_memory_equal("Test Message 1", log->msg, 15);
}

static void test_log_warn_writes_to_log_buffer(UNUSED void** state)
{
    __real_log_warn("Test Message %d", 1, 0);

    Log* log = __real_log_dequeue();

    assert_int_not_equal(log->msgLen, 0);
    assert_memory_equal("Test Message 1", log->msg, 15);
    assert_int_equal(log->level, Warn);
}

static void test_log_stores_two_logs(UNUSED void** state)
{
    __real_log_info("Test Message %d", 1);
    __real_log_info("Test Message %d", 2);

    Log* log1 = __real_log_dequeue();
    assert_non_null(log1);

    Log* log2 = __real_log_dequeue();
    assert_non_null(log2);

    assert_memory_equal("Test Message 1", log1->msg, 15);
    assert_memory_equal("Test Message 2", log2->msg, 15);
}

static void test_log_stores_multiple_logs_and_overwrites_older(
    UNUSED void** state)
{
    for (u8 i = 1; i <= 15; i++) {
        __real_log_info("Test Message %d", i);
    }

    for (u8 i = 7; i <= 15; i++) {
        print_message("Dequeuing log %d\n", i);
        Log* log = __real_log_dequeue();

        char expectedMsg[15];
        sprintf(expectedMsg, "Test Message %d", i);
        assert_memory_equal(expectedMsg, log->msg, 15);
    }
}

static void
test_log_returns_null_when_no_more_logs_are_available_to_be_dequeued(
    UNUSED void** state)
{
    __real_log_info("Test Message %d", 1);

    assert_non_null(__real_log_dequeue());
    assert_null(__real_log_dequeue());
}
