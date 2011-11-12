#include "mustang.h"

Mustang::Mustang()
{
    amp_hand = NULL;

    // "apply efect" command
    memset(execute, 0x00, LENGTH);
    execute[0] = 0x1c;
    execute[1] = 0x03;

    memset(prev_array, 0x00, LENGTH*4);
    for(int i = 0; i < 4; i++)
    {
        prev_array[i][0] = 0x1c;
        prev_array[i][1] = 0x03;
        prev_array[i][6] = prev_array[i][7] = prev_array[i][21] = 0x01;
        prev_array[i][20] = 0x08;
        prev_array[i][FXSLOT] = 0xff;
    }
}

Mustang::~Mustang()
{
    this->stop_amp();
}

int Mustang::start_amp(char list[][32], char *name, struct amp_settings *amp_set, struct fx_pedal_settings *effects_set)
{
    int ret, recieved;
    unsigned char array[LENGTH];
    unsigned char recieved_data[294][LENGTH], data[6][LENGTH];
    memset(recieved_data, 0x00, 294*LENGTH);

    if(amp_hand == NULL)
    {
        // initialize libusb
        ret = libusb_init(NULL);
        if (ret)
            return ret;

        // get handle for the device
        if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, OLD_USB_PID)) == NULL)
            if((amp_hand = libusb_open_device_with_vid_pid(NULL, USB_VID, NEW_USB_PID)) == NULL)
            {
                libusb_exit(NULL);
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
    }

    // initialization which is needed if you want
    // to get any replies from the amp in the future
    memset(array, 0x00, LENGTH);
    array[1] = 0xc3;
    libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);

    memset(array, 0x00, LENGTH);
    array[0] = 0x1a;
    array[1] = 0x03;
    libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);

    if(list != NULL || name != NULL || amp_set != NULL || effects_set != NULL)
    {
        int i = 0, j = 0;
        memset(array, 0x00, LENGTH);
        array[0] = 0xff;
        array[1] = 0xc1;
        libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);

        for(i = 0; recieved; i++)
        {
            libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);
            memcpy(recieved_data[i], array, LENGTH);
        }

        int max_to_receive;
        i > 142 ? max_to_receive = 200 : max_to_receive = 48;
        if(list != NULL)
            for(i = 0, j = 0; i<max_to_receive; i+=2, j++)
                memcpy(list[j], recieved_data[i]+16, 32);

        if(name != NULL || amp_set != NULL || effects_set != NULL)
        {
            for(j = 0; j < 6; i++, j++)
                memcpy(data[j], recieved_data[i], LENGTH);
            decode_data(data, name, amp_set, effects_set);
        }
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
        if(ret && (ret != LIBUSB_ERROR_NO_DEVICE))
            return ret;

        if(ret != LIBUSB_ERROR_NO_DEVICE)
        {
            // re-attach kernel driver
            ret = libusb_attach_kernel_driver(amp_hand, 0);
            if(ret)
                return ret;
        }

        // close opened interface
        libusb_close(amp_hand);
        amp_hand = NULL;

        // stop using libusb
        libusb_exit(NULL);
    }

    return 0;
}

int Mustang::set_effect(struct fx_pedal_settings value)
{
    int ret, recieved;    // variables used when sending
    unsigned char slot;    // where to put the effect
    unsigned char temp[LENGTH], array[LENGTH] = {
        0x1c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // clear effect on previous DSP before setting a new one
    int k=0;
    for (int i = 0; i < 4; i++)
    {
        if (prev_array[i][FXSLOT] == value.fx_slot || prev_array[i][FXSLOT] == (value.fx_slot+4))
        {
            memcpy(array, prev_array[i], LENGTH);
            prev_array[i][FXSLOT] = 0xff;
            k++;
            break;
        }
    }
    array[EFFECT] = 0x00;
    array[KNOB1] = 0x00;
    array[KNOB2] = 0x00;
    array[KNOB3] = 0x00;
    array[KNOB4] = 0x00;
    array[KNOB5] = 0x00;
    array[KNOB6] = 0x00;
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &recieved, TMOUT);
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &recieved, TMOUT);
    //DEBUG
//    qDebug("set: DSP: %d, slot: %d, effect: %d, EMPTY", array[DSP], array[FXSLOT], array[EFFECT]);
    if(value.effect_num == EMPTY)
        return ret;

    if(value.put_post_amp)    // put effect in a slot after amplifier
        slot = value.fx_slot + 4;
    else
        slot = value.fx_slot;

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
    switch (value.effect_num) {
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
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &recieved, TMOUT);
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &recieved, TMOUT);
    //DEBUG
//    qDebug("set: DSP: %d, slot: %d, effect: %d", array[DSP], array[FXSLOT], array[EFFECT]);

    // save current settings
    memcpy(prev_array[array[DSP]-6], array, LENGTH);


    //DEBUG
//    FILE *f;
//    static char trynum=0;
//    char mes[16];
//    sprintf(mes, "test%d.bin",trynum);
//    f=fopen(mes,"w");
//    fwrite(array, sizeof(array), 1, f);
//    //fwrite(execute, sizeof(execute), 1, f);
//    fclose(f);
//    trynum++;


    return ret;
}

int Mustang::set_amplifier(struct amp_settings value)
{
    int ret, recieved;
    unsigned char array[LENGTH] = {
        0x1c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xaa, 0xa2, 0x80, 0x63, 0x99, 0x80, 0xb0, 0x00,
        0x80, 0x80, 0x80, 0x80, 0x07, 0x07, 0x07, 0x05,
        0x00, 0x07, 0x07, 0x01, 0x00, 0x01, 0x5e, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

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

    array[BRIGHTNESS] = value.brightness?1:0;

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
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);

    return ret;
}

int Mustang::save_on_amp(char *name, int slot)
{
    int ret, recieved;
    unsigned char array[LENGTH];

    memset(array, 0x00, LENGTH);
    array[0] = 0x1c;
    array[1] = 0x01;
    array[2] = 0x03;
    array[SAVE_SLOT] = slot;
    array[6] = 0x01;
    array[7] = 0x01;

    if(strlen(name) > 31)
        name[31] = 0x00;

    for(unsigned int i = 16, j = 0; name[j] != 0x00; i++,j++)
        array[i] = name[j];

    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);
    load_memory_bank(slot, NULL, NULL, NULL);

    return ret;
}

int Mustang::load_memory_bank(int slot, char *name, struct amp_settings *amp_set, struct fx_pedal_settings *effects_set)
{
    int ret, recieved;
    unsigned char array[LENGTH], data[6][LENGTH];

    memset(array, 0x00, LENGTH);
    array[0] = 0x1c;
    array[1] = 0x01;
    array[2] = 0x01;
    array[SAVE_SLOT] = slot;
    array[6] = 0x01;

    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    for(int i = 0; recieved; i++)
    {
        libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);
        if(i < 6)
            memcpy(data[i], array, LENGTH);
    }

    if(name != NULL || amp_set != NULL || effects_set != NULL)
        decode_data(data, name, amp_set, effects_set);

    return ret;
}

int Mustang::decode_data(unsigned char data[6][LENGTH], char *name, struct amp_settings *amp_set, struct fx_pedal_settings *effects_set)
{
    if(name != NULL)
    {
        // NAME
        memset(name, 0x00, 32);
        for(int i = 0, j = 16; data[0][j] != 0x00; i++, j++)
            name[i] = data[0][j];
    }


    if(amp_set != NULL)
    {
        // AMPLIFIER
        switch(data[1][AMPLIFIER])
        {
        case 0x67:
            amp_set->amp_num = FENDER_57_DELUXE;
            break;

        case 0x64:
            amp_set->amp_num = FENDER_59_BASSMAN;
            break;

        case 0x7c:
            amp_set->amp_num = FENDER_57_CHAMP;
            break;

        case 0x53:
            amp_set->amp_num = FENDER_65_DELUXE_REVERB;
            break;

        case 0x6a:
            amp_set->amp_num = FENDER_65_PRINCETON;
            break;

        case 0x75:
            amp_set->amp_num = FENDER_65_TWIN_REVERB;
            break;

        case 0x72:
            amp_set->amp_num = FENDER_SUPER_SONIC;
            break;

        case 0x61:
            amp_set->amp_num = BRITISH_60S;
            break;

        case 0x79:
            amp_set->amp_num = BRITISH_70S;
            break;

        case 0x5e:
            amp_set->amp_num = BRITISH_80S;
            break;

        case 0x5d:
            amp_set->amp_num = AMERICAN_90S;
            break;

        case 0x6d:
            amp_set->amp_num = METAL_2000;
            break;
        }

        amp_set->gain = data[1][GAIN];
        amp_set->volume = data[1][VOLUME];
        amp_set->treble = data[1][TREBLE];
        amp_set->middle = data[1][MIDDLE];
        amp_set->bass = data[1][BASS];
        amp_set->cabinet = data[1][CABINET];
        amp_set->noise_gate = data[1][NOISE_GATE];
        amp_set->master_vol = data[1][MASTER_VOL];
        amp_set->gain2 = data[1][GAIN2];
        amp_set->presence = data[1][PRESENCE];
        amp_set->threshold = data[1][THRESHOLD];
        amp_set->depth = data[1][DEPTH];
        amp_set->bias = data[1][BIAS];
        amp_set->sag = data[1][SAG];
        amp_set->brightness = data[1][BRIGHTNESS]?true:false;
    }


    if(effects_set != NULL)
    {
        // EFFECTS
        for(int i = 2; i < 6; i++)
        {
            int j=0;

            prev_array[data[i][DSP]-6][0] = 0x1c;
            prev_array[data[i][DSP]-6][1] = 0x03;
            prev_array[data[i][DSP]-6][FXSLOT] = data[i][FXSLOT];
            prev_array[data[i][DSP]-6][DSP] = data[i][DSP];
            prev_array[data[i][DSP]-6][19] = data[i][19];
            prev_array[data[i][DSP]-6][20] = data[i][20];

            switch(data[i][FXSLOT])
            {
            case 0x00:
            case 0x04:
                j = 0;
                break;

            case 0x01:
            case 0x05:
                j = 1;
                break;

            case 0x02:
            case 0x06:
                j = 2;
                break;

            case 0x03:
            case 0x07:
                j = 3;
                break;
            }

            effects_set[j].fx_slot = j;
            effects_set[j].knob1 = data[i][KNOB1];
            effects_set[j].knob2 = data[i][KNOB2];
            effects_set[j].knob3 = data[i][KNOB3];
            effects_set[j].knob4 = data[i][KNOB4];
            effects_set[j].knob5 = data[i][KNOB5];
            effects_set[j].knob6 = data[i][KNOB6];
            if(data[i][FXSLOT] > 0x03)
                effects_set[j].put_post_amp = true;
            else
                effects_set[j].put_post_amp = false;

            switch(data[i][EFFECT])
            {
            case 0x00:
                effects_set[j].effect_num = EMPTY;
                break;

            case 0x3c:
                effects_set[j].effect_num =  OVERDRIVE;
                break;

            case 0x49:
                effects_set[j].effect_num = FIXED_WAH;
                break;

            case 0x4a:
                effects_set[j].effect_num = TOUCH_WAH;
                break;

            case 0x1a:
                effects_set[j].effect_num = FUZZ;
                break;

            case 0x1c:
                effects_set[j].effect_num = FUZZ_TOUCH_WAH;
                break;

            case 0x88:
                effects_set[j].effect_num = SIMPLE_COMP;
                break;

            case 0x07:
                effects_set[j].effect_num = COMPRESSOR;
                break;

            case 0x12:
                effects_set[j].effect_num = SINE_CHORUS;
                break;

            case 0x13:
                effects_set[j].effect_num = TRIANGLE_CHORUS;
                break;

            case 0x18:
                effects_set[j].effect_num = SINE_FLANGER;
                break;

            case 0x19:
                effects_set[j].effect_num = TRIANGLE_FLANGER;
                break;

            case 0x2d:
                effects_set[j].effect_num = VIBRATONE;
                break;

            case 0x40:
                effects_set[j].effect_num = VINTAGE_TREMOLO;
                break;

            case 0x41:
                effects_set[j].effect_num = SINE_TREMOLO;
                break;

            case 0x22:
                effects_set[j].effect_num = RING_MODULATOR;
                break;

            case 0x29:
                effects_set[j].effect_num = STEP_FILTER;
                break;

            case 0x4f:
                effects_set[j].effect_num = PHASER;
                break;

            case 0x1f:
                effects_set[j].effect_num = PITCH_SHIFTER;
                break;

            case 0x16:
                effects_set[j].effect_num = MONO_DELAY;
                break;

            case 0x43:
                effects_set[j].effect_num = MONO_ECHO_FILTER;
                break;

            case 0x48:
                effects_set[j].effect_num = STEREO_ECHO_FILTER;
                break;

            case 0x44:
                effects_set[j].effect_num = MULTITAP_DELAY;
                break;

            case 0x45:
                effects_set[j].effect_num = PING_PONG_DELAY;
                break;

            case 0x15:
                effects_set[j].effect_num = DUCKING_DELAY;
                break;

            case 0x46:
                effects_set[j].effect_num = REVERSE_DELAY;
                break;

            case 0x2b:
                effects_set[j].effect_num = TAPE_DELAY;
                break;

            case 0x2a:
                effects_set[j].effect_num = STEREO_TAPE_DELAY;
                break;

            case 0x24:
                effects_set[j].effect_num = SMALL_HALL_REVERB;
                break;

            case 0x3a:
                effects_set[j].effect_num = LARGE_HALL_REVERB;
                break;

            case 0x26:
                effects_set[j].effect_num = SMALL_ROOM_REVERB;
                break;

            case 0x3b:
                effects_set[j].effect_num = LARGE_ROOM_REVERB;
                break;

            case 0x4e:
                effects_set[j].effect_num = SMALL_PLATE_REVERB;
                break;

            case 0x4b:
                effects_set[j].effect_num = LARGE_PLATE_REVERB;
                break;

            case 0x4c:
                effects_set[j].effect_num = AMBIENT_REVERB;
                break;

            case 0x4d:
                effects_set[j].effect_num = ARENA_REVERB;
                break;

            case 0x21:
                effects_set[j].effect_num = FENDER_63_SPRING_REVERB;
                break;

            case 0x0b:
                effects_set[j].effect_num = FENDER_65_SPRING_REVERB;
                break;
            }
        }
    }

    return 0;
}

int Mustang::save_effects(int slot, char name[24], int number_of_effects, struct fx_pedal_settings effects[2])
{
    int ret, recieved;
    unsigned char fxknob, repeat;
    unsigned char temp[LENGTH], array[LENGTH] = {
        0x1c, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    execute[2]=0x00;  // why this must be here?

    if(number_of_effects > 2)
        repeat = 1;
    else
        repeat = number_of_effects;

    for(int i = 0; i < repeat; i++)
        if(effects[i].effect_num < SINE_CHORUS)
            return -1;

    if(effects[0].effect_num>=SINE_CHORUS && effects[0].effect_num<=PITCH_SHIFTER)
    {
        fxknob = 0x01;
        repeat = 1;  //just to be sure
    }
    else
        fxknob = 0x02;
    array[FXKNOB] = fxknob;

    array[SAVE_SLOT] = slot;

    // set and send the name
    if(name[24] != 0x00)
        name[24] = 0x00;
    for(int i = 0, j = 16; name[i] != 0x00; i++, j++)
        array[j] = name[i];
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &recieved, TMOUT);

    array[1] = 0x03;
    array[6] = 0x00;
    memset(array+16, 0x00, LENGTH-16);
    for(int i = 0; i < repeat; i++)
    {
        array[19] = 0x00;
        array[20] = 0x08;
        array[21] = 0x01;
        array[KNOB6] = 0x00;

        if(effects[i].put_post_amp)
            array[FXSLOT] = effects[i].fx_slot+4;
        else
            array[FXSLOT] = effects[i].fx_slot;
        array[KNOB1] = effects[i].knob1;
        array[KNOB2] = effects[i].knob2;
        array[KNOB3] = effects[i].knob3;
        array[KNOB4] = effects[i].knob4;
        array[KNOB5] = effects[i].knob5;
        // some effects have more knobs
        if (effects[i].effect_num == MONO_ECHO_FILTER ||
                effects[i].effect_num == STEREO_ECHO_FILTER ||
                effects[i].effect_num == TAPE_DELAY ||
                effects[i].effect_num == STEREO_TAPE_DELAY)
        {
            array[KNOB6] = effects[i].knob6;
        }

        // fill the form with missing data
        switch (effects[i].effect_num) {
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
                array[KNOB4] = 0x01;
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
                array[KNOB5] = 0x01;
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
        // send packet
        ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
        libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &recieved, TMOUT);
    }

    execute[FXKNOB] = fxknob;
    ret = libusb_interrupt_transfer(amp_hand, 0x01, execute, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, temp, LENGTH, &recieved, TMOUT);
    execute[FXKNOB] = 0x00;

    return 0;
}

int Mustang::update(char *filename)
{
    int ret, recieved;
    unsigned char array[LENGTH], number = 0;
    FILE *file;
    file = fopen(filename, "rb");
    struct timespec sleep;
    sleep.tv_nsec = NANO_SEC_SLEEP;
    sleep.tv_sec = 0;

    if(amp_hand == NULL)
    {
        // initialize libusb
        ret = libusb_init(NULL);
        if (ret)
            return ret;

        // get handle for the device
        amp_hand = libusb_open_device_with_vid_pid(NULL, USB_UPDATE_VID, USB_UPDATE_PID);
        if(amp_hand == NULL)
        {
            libusb_exit(NULL);
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
    }
    else
        return -200;

    // send date when firmware was created
    fseek(file, 0x1a, SEEK_SET);
    memset(array, 0x00, LENGTH);
    array[0] = 0x02;
    array[1] = 0x03;
    array[2] = 0x01;
    array[3] = 0x06;
    fread(array+4, 1, 11, file);
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);
    nanosleep(&sleep, NULL);

    // send firmware
    fseek(file, 0x110, SEEK_SET);
    for(;;)
    {
        memset(array, 0x00, LENGTH);
        array[0] = array[1] = 0x03;
        array[2] = number;
        number++;
        array[3] = fread(array+4, 1, LENGTH-8, file);
        ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
        libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);
        nanosleep(&sleep, NULL);
        if(feof(file))  // if reached end of the file
            break;  // exit loop
    }
    fclose(file);

    // send "finished" packet
    memset(array, 0x00, LENGTH);
    array[0] = 0x04;
    array[1] = 0x03;
    ret = libusb_interrupt_transfer(amp_hand, 0x01, array, LENGTH, &recieved, TMOUT);
    libusb_interrupt_transfer(amp_hand, 0x81, array, LENGTH, &recieved, TMOUT);

    // release claimed interface
    ret = libusb_release_interface(amp_hand, 0);
    if(ret)
        return ret;

    // re-attach kernel driver
    ret = libusb_attach_kernel_driver(amp_hand, 0);
    if(ret)
        return ret;

    // close opened interface
    libusb_close(amp_hand);
    amp_hand = NULL;

    // stop using libusb
    libusb_exit(NULL);

    return 0;
}
