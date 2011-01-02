#ifndef EFFECTS_ENUM_H
#define EFFECTS_ENUM_H

// enums used for switches (looks nicer
// and is more practical than numbers)

// list of all amplifiers
enum amps
{
    FENDER_57_DELUXE,
    FENDER_59_BASSMAN,
    FENDER_57_CHAMP,
    FENDER_65_DELUXE_REVERB,
    FENDER_65_PRINCETON,
    FENDER_65_TWIN_REVERB,
    FENDER_SUPER_SONIC,
    BRITISH_60S,
    BRITISH_70S,
    BRITISH_80S,
    AMERICAN_90S,
    METAL_2000
};

// list of all effects
enum effects
{
    EMPTY,
    OVERDRIVE,
    FIXED_WAH,
    TOUCH_WAH,
    FUZZ,
    FUZZ_TOUCH_WAH,
    SIMPLE_COMP,
    COMPRESSOR,

    SINE_CHORUS,
    TRIANGLE_CHORUS,
    SINE_FLANGER,
    TRIANGLE_FLANGER,
    VIBRATONE,
    VINTAGE_TREMOLO,
    SINE_TREMOLO,
    RING_MODULATOR,
    STEP_FILTER,
    PHASER,
    PITCH_SHIFTER,

    MONO_DELAY,
    MONO_ECHO_FILTER,
    STEREO_ECHO_FILTER,
    MULTITAP_DELAY,
    PING_PONG_DELAY,
    DUCKING_DELAY,
    REVERSE_DELAY,
    TAPE_DELAY,
    STEREO_TAPE_DELAY,

    SMALL_HALL_REVERB,
    LARGE_HALL_REVERB,
    SMALL_ROOM_REVERB,
    LARGE_ROOM_REVERB,
    SMALL_PLATE_REVERB,
    LARGE_PLATE_REVERB,
    AMBIENT_REVERB,
    ARENA_REVERB,
    FENDER_63_SPRING_REVERB,
    FENDER_65_SPRING_REVERB
};

// list of all cabinets
enum cabinets
{
    OFF,
    cab57DLX,
    cabBSSMN,
    cab65DLX,
    cab65PRN,
    cabCHAMP,
    cab4x12M,
    cab2x12C,
    cab4x12G,
    cab65TWN,
    cab4x12V,
    cabSS212,
    cabSS112
};

#endif // EFFECTS_ENUM_H
