#include "test_midi.h"

static void test_midi_triggers_psg_note_on(UNUSED void** state)
{
    const u8 midiKeys[] = { 45, 69, 108 };
    const u16 freqs[] = { 110, 440, 4186 };

    for (u8 i = 0; i < sizeof(midiKeys); i++) {
        for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

            u16 expectedFrequency = freqs[i];
            u8 expectedMidiKey = midiKeys[i];
            u8 expectedPsgChan = chan - MIN_PSG_CHAN;

            expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
            expect_value(__wrap_psg_frequency, freq, expectedFrequency);
            expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
            expect_value(__wrap_psg_attenuation, attenuation, 0);

            __real_midi_noteOn(chan, expectedMidiKey, 127);
        }
    }
}

static void test_midi_triggers_psg_note_on_with_velocity(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any(__wrap_psg_frequency, channel);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x3);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_on_with_velocity_and_channel_volume(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);

    expect_any(__wrap_psg_frequency, channel);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x7);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);
}

static void test_midi_changing_volume_during_psg_note_on_respects_velocity(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any(__wrap_psg_frequency, channel);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x3);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);

    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x7);

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_off(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        u8 midiKey = 60;

        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);

        __real_midi_noteOn(chan, midiKey, 127);

        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, midiKey);
    }
}

static void test_midi_drops_psg_key_below_45(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedMidiKey = 44;
        __real_midi_noteOn(chan, expectedMidiKey, 127);
    }
}

static void
test_midi_triggers_psg_note_off_and_volume_change_does_not_cause_psg_channel_to_play(
    UNUSED void** state)
{
    u8 midiKey = 60;
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);

        __real_midi_noteOn(chan, midiKey, 127);

        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, midiKey);

        __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);
    }
}

static void test_midi_psg_note_off_only_triggered_if_specific_note_is_on(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;

    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);
    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 0x106);

    __real_midi_noteOn(chan, 60, 127);

    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);
    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 0x115);

    __real_midi_noteOn(chan, 61, 127);

    __real_midi_noteOff(chan, 60);
}

static void test_midi_channel_volume_sets_psg_attenuation(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 110);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, 1);

    __real_midi_noteOn(MIN_PSG_CHAN, 45, 127);
}

static void test_midi_channel_volume_sets_psg_attenuation_2(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 127);

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 110);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, 0);

    __real_midi_noteOn(MIN_PSG_CHAN, 45, 127);
}

static void test_midi_sets_psg_pitch_bend(UNUSED void** state)
{
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
        expect_value(__wrap_psg_frequency, freq, 262);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(__wrap_psg_attenuation, attenuation, 0);

        __real_midi_noteOn(chan, 60, 127);

        expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
        expect_value(__wrap_psg_frequency, freq, 191);

        __real_midi_pitchBend(chan, 1000);
    }
}

static void test_midi_plays_psg_envelope(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;

    __real_midi_program(chan, 1);

    expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
    expect_value(__wrap_psg_frequency, freq, 262);
    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0);

    __real_midi_noteOn(chan, 60, 127);
}
