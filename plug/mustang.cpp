#include "mustang.h"

#include <stdio.h>
#include <QtDebug>

Mustang::Mustang()
{
    // "apply efect" command
    execute[0] = 0x1c;
    execute[1] = 0x03;
    for(int i = 2; i < LENGTH; i++)
    {
        execute[i] = 0x00;
    }

    prev_array[0][0] = 0x00;
    prev_array[1][0] = 0x00;
    prev_array[2][0] = 0x00;
    prev_array[3][0] = 0x00;
    for(int i = 0; i < 4; i++)
    {
        prev_array[i][FXSLOT] = 0xff;
    }
}

Mustang::~Mustang()
{
    this->stop_amp();
}

int Mustang::start_amp()
{
    int ret;

    // initialize libusb
    ret = libusb_init(NULL);
    if (ret)
    {
        return ret;
    }

    // get handle for the device
    amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, USB_PID);
    if(amp_hand == NULL)
    {
        stop_amp();
        return -100;
    }

    // detach kernel driver
    ret = libusb_kernel_driver_active(amp_hand, 0);
    if(ret)
    {
        ret = libusb_detach_kernel_driver(amp_hand, 0);
        if(ret)
        {
            stop_amp();
            return ret;
        }
    }

    // claim the device
    ret = libusb_claim_interface(amp_hand, 0);
    if(ret)
    {
        stop_amp();
        return ret;
    }

    return 0;
}

int Mustang::stop_amp()
{
    int ret;

    if(amp_hand != NULL)
    {
        // release claimed interface
        ret = libusb_release_interface(amp_hand, 0);
        if(ret)
        {
            return ret;
        }

        // re-attach kernel driver
        ret = libusb_attach_kernel_driver(amp_hand, 0);

        // close opened interface
        libusb_close(amp_hand);
        amp_hand = NULL;
        //printf("amp stopped\n");
    }

    // stop using libusb
    libusb_exit(NULL);

    return 0;
}

int Mustang::set_effect(struct fx_pedal_settings value)
{
    int ret, recieved;    // variables used when sending
    unsigned char slot;    // where to put the effect
    unsigned char array[LENGTH] = {    // empty data form
      0x1c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    if(value.put_post_amp)    // put effect in a slot after amplifier
    {
        slot = value.fx_slot + 4;
    }
    else
    {
        slot = value.fx_slot;
    }

    // fill the form with data
    array[FXSLOT] = slot;
    array[KNOB1] = value.knob1;
    array[KNOB2] = value.knob2;
    array[KNOB3] = value.knob3;
    array[KNOB4] = value.knob4;
    array[KNOB5] = value.knob5;
    // some effects have more knobs
    if (value.effect_num == MONO_ECHO_FILTER ||
            value.effect_num == STEREO_ECHO_FILTER ||
            value.effect_num == TAPE_DELAY ||
            value.effect_num == STEREO_TAPE_DELAY)
    {
        array[KNOB6] = value.knob6;
    }

    // fill the form with missing data
    int k=0;
    switch (value.effect_num) {
    case EMPTY:
        for (int i = 0; i < 4; i++)
        {
            if (prev_array[i][FXSLOT] == slot)
            {
                for (int j = 0; j < LENGTH; j++)
                {
                    array[j] = prev_array[i][j];
                }
                k++;
                break;
            }
        }
        if (k == 0)
        {
            return 0;
        }
        array[EFFECT] = 0x00;
        array[KNOB1] = 0x00;
        array[KNOB2] = 0x00;
        array[KNOB3] = 0x00;
        array[KNOB4] = 0x00;
        array[KNOB5] = 0x00;
        array[KNOB6] = 0x00;
        break;

    case OVERDRIVE:
        array[DSP] = 0x06;
        array[EFFECT] = 0x3c;
        break;

    case FIXED_WAH:
        array[DSP] = 0x06;
        array[EFFECT] = 0x49;
        array[19] = 0x01;
        break;

    case TOUCH_WAH:
        array[DSP] = 0x06;
        array[EFFECT] = 0x4a;
        array[19] = 0x01;
        break;

    case FUZZ:
        array[DSP] = 0x06;
        array[EFFECT] = 0x1a;
        break;

    case FUZZ_TOUCH_WAH:
        array[DSP] = 0x06;
        array[EFFECT] = 0x1c;
        break;

    case SIMPLE_COMP:
        array[DSP] = 0x06;
        array[EFFECT] = 0x88;
        array[19] = 0x08;
        if(array[KNOB1] > 0x03)
        {
            array[KNOB1] = 0x03;
        }
        array[KNOB2] = 0x00;
        array[KNOB3] = 0x00;
        array[KNOB4] = 0x00;
        array[KNOB5] = 0x00;
        break;

    case COMPRESSOR:
        array[DSP] = 0x06;
        array[EFFECT] = 0x07;
        break;

    case SINE_CHORUS:
        array[DSP] = 0x07;
        array[EFFECT] = 0x12;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case TRIANGLE_CHORUS:
        array[DSP] = 0x07;
        array[EFFECT] = 0x13;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case SINE_FLANGER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x18;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case TRIANGLE_FLANGER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x19;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case VIBRATONE:
        array[DSP] = 0x07;
        array[EFFECT] = 0x2d;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case VINTAGE_TREMOLO:
        array[DSP] = 0x07;
        array[EFFECT] = 0x40;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case SINE_TREMOLO:
        array[DSP] = 0x07;
        array[EFFECT] = 0x41;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case RING_MODULATOR:
        array[DSP] = 0x07;
        array[EFFECT] = 0x22;
        array[19] = 0x01;
        if(array[KNOB4] > 0x01)
        {
            array[KNOB4] = 0x01;
        }
        break;

    case STEP_FILTER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x29;
        array[19] = 0x01;
        array[20] = 0x01;
        break;

    case PHASER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x4f;
        array[19] = 0x01;
        array[20] = 0x01;
        if(array[KNOB5] > 0x01)
        {
            array[KNOB5] = 0x01;
        }
        break;

    case PITCH_SHIFTER:
        array[DSP] = 0x07;
        array[EFFECT] = 0x1f;
        array[19] = 0x01;
        break;

    case MONO_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x16;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case MONO_ECHO_FILTER:
        array[DSP] = 0x08;
        array[EFFECT] = 0x43;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case STEREO_ECHO_FILTER:
        array[DSP] = 0x08;
        array[EFFECT] = 0x48;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case MULTITAP_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x44;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case PING_PONG_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x45;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case DUCKING_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x15;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case REVERSE_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x46;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case TAPE_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x2b;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case STEREO_TAPE_DELAY:
        array[DSP] = 0x08;
        array[EFFECT] = 0x2a;
        array[19] = 0x02;
        array[20] = 0x01;
        break;

    case SMALL_HALL_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x24;
        break;

    case LARGE_HALL_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x3a;
        break;

    case SMALL_ROOM_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x26;
        break;

    case LARGE_ROOM_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x3b;
        break;

    case SMALL_PLATE_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4e;
        break;

    case LARGE_PLATE_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4b;
        break;

    case AMBIENT_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4c;
        break;

    case ARENA_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x4d;
        break;

    case FENDER_63_SPRING_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x21;
        break;

    case FENDER_65_SPRING_REVERB:
        array[DSP] = 0x09;
        array[EFFECT] = 0x0b;
        break;
    }

    // send packet to the amp
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &recieved, TMOUT);
    //DEBUG
//    qDebug("set: DSP: %d, slot: %d, effect: %d", array[DSP], array[FXSLOT], array[EFFECT]);

    // save current settings
    for (int i = 0; i < LENGTH; i++)
    {
        prev_array[array[DSP]-6][i] = array[i];
    }


    //DEBUG
//    FILE *f;
//    static char trynum=0;
//    char mes[16];
//    sprintf(mes, "test%d.bin",trynum);
//    f=fopen(mes,"w");
//    fwrite(array, sizeof(array), 1, f);
//    //fwrite(FXEXEC, sizeof(FXEXEC), 1, f);
//    fclose(f);
//    trynum++;


    return ret;
}

int Mustang::set_amplifier(struct amp_settings value)
{
    int ret, recieved;
    unsigned char array[LENGTH] = {0x1c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xaa, 0xa2, 0x80, 0x63, 0x99, 0x80, 0xb0, 0x00,
    0x80, 0x80, 0x80, 0x80, 0x07, 0x07, 0x07, 0x05,
    0x00, 0x07, 0x07, 0x01, 0x00, 0x01, 0x5e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    array[DSP] = 0x05;
    array[GAIN] = value.gain;
    array[VOLUME] = value.volume;
    array[TREBLE] = value.treble;
    array[MIDDLE] = value.middle;
    array[BASS] = value.bass;

    if(value.cabinet > 0x0c)
        array[CABINET] = 0x00;
    else
        array[CABINET] = value.cabinet;

    if(value.noise_gate > 0x05)
        array[NOISE_GATE] = 0x00;
    else
        array[NOISE_GATE] = value.noise_gate;

    array[MASTER_VOL] = value.master_vol;
    array[GAIN2] = value.gain2;
    array[PRESENCE] = value.presence;

    if(value.noise_gate == 0x05)
    {
        if(value.threshold > 0x09)
            array[THRESHOLD] = 0x00;
        else
            array[THRESHOLD] = value.threshold;

        array[DEPTH] = value.depth;
    }
    array[BIAS] = value.bias;

    if(value.sag > 0x02)
        array[SAG] = 0x01;
    else
        array[SAG] = value.sag;

    switch (value.amp_num)
    {
    case FENDER_57_DELUXE:
        array[AMPLIFIER] = 0x67;
        array[44] = array[45] = array[46] = 0x01;
        array[50] = 0x01;
        array[54] = 0x53;
        break;

    case FENDER_59_BASSMAN:
        array[AMPLIFIER] = 0x64;
        array[44] = array[45] = array[46] = 0x02;
        array[50] = 0x02;
        array[54] = 0x67;
        break;

    case FENDER_57_CHAMP:
        array[AMPLIFIER] = 0x7c;
        array[44] = array[45] = array[46] = 0x0c;
        array[50] = 0x0c;
        array[54] = 0x00;
        break;

    case FENDER_65_DELUXE_REVERB:
        array[AMPLIFIER] = 0x53;
        array[40] = array[43] = 0x00;
        array[44] = array[45] = array[46] = 0x03;
        array[50] = 0x03;
        array[54] = 0x6a;
        break;

    case FENDER_65_PRINCETON:
        array[AMPLIFIER] = 0x6a;
        array[44] = array[45] = array[46] = 0x04;
        array[50] = 0x04;
        array[54] = 0x61;
        break;

    case FENDER_65_TWIN_REVERB:
        array[AMPLIFIER] = 0x75;
        array[44] = array[45] = array[46] = 0x05;
        array[50] = 0x05;
        array[54] = 0x72;
        break;

    case FENDER_SUPER_SONIC:
        array[AMPLIFIER] = 0x72;
        array[44] = array[45] = array[46] = 0x06;
        array[50] = 0x06;
        array[54] = 0x79;
        break;

    case BRITISH_60S:
        array[AMPLIFIER] = 0x61;
        array[44] = array[45] = array[46] = 0x07;
        array[50] = 0x07;
        array[54] = 0x5e;
        break;

    case BRITISH_70S:
        array[AMPLIFIER] = 0x79;
        array[44] = array[45] = array[46] = 0x0b;
        array[50] = 0x0b;
        array[54] = 0x7c;
        break;

    case BRITISH_80S:
        array[AMPLIFIER] = 0x5e;
        array[44] = array[45] = array[46] = 0x09;
        array[50] = 0x09;
        array[54] = 0x5d;
        break;

    case AMERICAN_90S:
        array[AMPLIFIER] = 0x5d;
        array[44] = array[45] = array[46] = 0x0a;
        array[50] = 0x0a;
        array[54] = 0x6d;
        break;

    case METAL_2000:
        array[AMPLIFIER] = 0x6d;
        array[44] = array[45] = array[46] = 0x08;
        array[50] = 0x08;
        array[54] = 0x75;
        break;
    }

    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &recieved, TMOUT);

    return ret;
}
