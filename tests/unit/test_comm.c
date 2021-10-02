#include "cmocka_inc.h"

#include "comm.h"

static const u16 MAX_COMM_IDLE = 0x28F;
static const u16 MAX_COMM_BUSY = 0x28F;

static int test_comm_setup(UNUSED void** state)
{
    __real_comm_init();

    return 0;
}

static void switch_comm_type_to_everdrive(void)
{
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read();
    __real_comm_reset_counts();
}

static void test_comm_reads_from_serial_when_ready(UNUSED void** state)
{
    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_pro_read_ready, 0);
    will_return(__wrap_comm_serial_read_ready, 1);
    will_return(__wrap_comm_serial_read, 50);

    u8 read = __real_comm_read();

    assert_int_equal(read, 50);
}

static void test_comm_reads_when_ready(UNUSED void** state)
{
    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_pro_read_ready, 0);
    will_return(__wrap_comm_serial_read_ready, 0);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);

    u8 read = __real_comm_read();

    assert_int_equal(read, 50);
}

static void test_comm_writes_when_ready(UNUSED void** state)
{
    const u8 test_data = 50;

    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read();

    will_return(__wrap_comm_everdrive_write_ready, 0);
    will_return(__wrap_comm_everdrive_write_ready, 1);
    expect_value(__wrap_comm_everdrive_write, data, test_data);

    __real_comm_write(test_data);
}

static void test_comm_idle_count_is_correct(UNUSED void** state)
{
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read();

    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);

    __real_comm_read();
    u16 idle = __real_comm_idle_count();

    assert_int_equal(idle, 2);
}

static void test_comm_busy_count_is_correct(UNUSED void** state)
{
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read();
    __real_comm_reset_counts();

    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    will_return(__wrap_comm_everdrive_read, 50);

    __real_comm_read();
    __real_comm_read();
    u16 busy = __real_comm_busy_count();

    assert_int_equal(busy, 2);
}

static void test_comm_clamps_idle_count(UNUSED void** state)
{
    switch_comm_type_to_everdrive();

    for (u16 i = 0; i < MAX_COMM_IDLE + 1; i++) {
        will_return(__wrap_comm_everdrive_read_ready, 0);
    }
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);

    __real_comm_read();
    u16 idle = __real_comm_idle_count();

    assert_int_equal(idle, MAX_COMM_IDLE);
}

static void test_comm_clamps_busy_count(UNUSED void** state)
{
    switch_comm_type_to_everdrive();

    for (u16 i = 0; i < MAX_COMM_BUSY + 1; i++) {
        will_return(__wrap_comm_everdrive_read_ready, 1);
        will_return(__wrap_comm_everdrive_read, 50);
        __real_comm_read();
    }

    u16 busy = __real_comm_busy_count();

    assert_int_equal(busy, MAX_COMM_BUSY);
}
