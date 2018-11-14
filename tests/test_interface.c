#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <interface.h>
#include <midi.h>
#include <types.h>
#include <wraps.h>

#define STATUS_CC 0xB0

static void test_interface_tick_passes_note_on_to_midi_processor(void** state)
{
    for (int chan = 0; chan < MAX_MIDI_CHANS; chan++) {
        u8 expectedStatus = 0x90 + chan;
        u8 expectedData = 60;
        u8 expectedData2 = 127;

        will_return(__wrap_comm_read, expectedStatus);
        will_return(__wrap_comm_read, expectedData);
        will_return(__wrap_comm_read, expectedData2);

        expect_value(__wrap_midi_noteOn, chan, chan);
        expect_value(__wrap_midi_noteOn, pitch, expectedData);
        expect_value(__wrap_midi_noteOn, velocity, expectedData2);

        interface_tick();
    }
}

static void test_interface_tick_passes_note_off_to_midi_processor(void** state)
{
    u8 expectedStatus = 0x80;
    u8 expectedData = 60;
    u8 expectedData2 = 127;

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedData);
    will_return(__wrap_comm_read, expectedData2);

    expect_value(__wrap_midi_noteOff, chan, 0);

    interface_tick();
}

static void test_interface_does_nothing_for_control_change(void** state)
{
    u8 expectedStatus = 0xA0;
    u8 expectedData = 106;
    u8 expectedData2 = 127;

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedData);
    will_return(__wrap_comm_read, expectedData2);

    interface_tick();
    interface_tick();
    interface_tick();
}

static void test_interface_sets_unknown_event_for_system_messages(void** state)
{
    u8 expectedStatus = 0xF0;

    will_return(__wrap_comm_read, expectedStatus);

    interface_tick();

    assert_int_equal(interface_lastUnknownStatus(), expectedStatus);
}

static void test_interface_sets_unknown_CC(void** state)
{
    u8 expectedStatus = STATUS_CC;
    u8 expectedController = 0x9;
    u8 expectedValue = 0x50;

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedController);
    will_return(__wrap_comm_read, expectedValue);

    interface_tick();

    ControlChange* cc = interface_lastUnknownCC();

    assert_int_equal(cc->controller, expectedController);
    assert_int_equal(cc->value, expectedValue);
}

static void test_interface_does_not_set_unknown_CC_for_known_CC(void** state)
{
    u8 expectedStatus = STATUS_CC;
    u8 expectedController = 0x7;
    u8 expectedValue = 0x80;

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedController);
    will_return(__wrap_comm_read, expectedValue);

    expect_value(__wrap_midi_channelVolume, chan, 0);
    expect_value(__wrap_midi_channelVolume, volume, expectedValue);

    interface_tick();

    ControlChange* cc = interface_lastUnknownCC();
    assert_int_not_equal(cc->controller, expectedController);
    assert_int_not_equal(cc->value, expectedValue);
}

static void test_interface_sets_channel_volume(void** state)
{
    u8 expectedStatus = STATUS_CC;
    u8 expectedController = 0x7;
    u8 expectedValue = 0x50;

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedController);
    will_return(__wrap_comm_read, expectedValue);

    expect_value(__wrap_midi_channelVolume, chan, 0);
    expect_value(__wrap_midi_channelVolume, volume, expectedValue);

    interface_tick();
}

static void test_interface_sets_pan(void** state)
{
    u8 expectedStatus = STATUS_CC;
    u8 expectedController = 0x0A;
    u8 expectedValue = 0xFF;

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedController);
    will_return(__wrap_comm_read, expectedValue);

    expect_value(__wrap_midi_pan, chan, 0);
    expect_value(__wrap_midi_pan, pan, expectedValue);

    interface_tick();
}

static void test_interface_initialises_synth(void** state)
{
    expect_function_call(__wrap_synth_init);
    interface_init();
}

static void test_interface_sets_fm_algorithm(void** state)
{
    u8 expectedStatus = STATUS_CC;
    u8 expectedController = 0x0E;
    u8 expectedValue = 0x01;

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedController);
    will_return(__wrap_comm_read, expectedValue);

    expect_value(__wrap_synth_algorithm, channel, 0);
    expect_value(__wrap_synth_algorithm, algorithm, expectedValue);

    interface_tick();
}
