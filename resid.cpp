// resid.cpp - 2023, m64
#include <stdio.h>
#include "resid.h"

ReSID::ReSID()
{
    printf("[ReSID] ReSID initializing ...\n");

    sid.set_sampling_parameters(985248, SAMPLE_RESAMPLE_INTERPOLATE, 44100);
    sid.set_chip_model(MOS8580);
    // sid.set_chip_model(MOS6581);
    printf("[ReSID] ReSID chip model: MOS8580\n");

    precalc_constants();
    printf("[ReSID] ReSID initialized\n");
}

ReSID::~ReSID()
{
}

void ReSID::writeRegs(unsigned char *regs, int len)
{
    for(int i=0; i<len; i++) {
        if(regs[i] != shadow_regs[i]) {
            sid.write(i, regs[i]);
            shadow_regs[i] = regs[i];     
        }
    }
}

int ReSID::clock(unsigned int cycles, short *buf, int buflen)
{
    cycle_count delta_t = cycles;
    return sid.clock(delta_t, buf, buflen);
}

void ReSID::precalc_constants()
{
    double d1, d2, d3;

    // SAMPLES_PER_FRAME
    //
    // 44.1 kHz =  22.676... us
    d1 = ((double) 1000.0) / ((double) CFG_AUDIO_SAMPLING_RATE);
    // 50 Hz = 20ms. => 20000us / 22.676 us = 882.00144  
    d1 = ((double) 20.0) / d1;
    SAMPLES_PER_FRAME = (int) d1;
    
    // CYCLES_PER_FRAME
    //
    // 50 Hz = 20ms. 1 cycle = 1,015us => 20000 / 1.015
    d2 = ((double) 20000.0) / ((double) 1.015) + 0.5;
    CYCLES_PER_FRAME = (int) d2;

    // CYCLES_PER_SAMPLE
    //
    // 44.1 kHz =  22.676 us
    d3 = ((double) 1000000.0) / ((double) CFG_AUDIO_SAMPLING_RATE);
    // 1 cycle = 1,015us => 22676 / 1.015
    d3 = d3 / ((double) 1.015);
    CYCLES_PER_SAMPLE = d3;

    // prepare shadow regs
    for(int i=0; i<32; i++) { shadow_regs[i] = 0; }

    printf("[ReSID] samples per frame : %d (%f)\n", SAMPLES_PER_FRAME, d1);
    printf("[ReSID] cycles  per frame : %d (%f)\n", CYCLES_PER_FRAME, d2 - 0.5);
    printf("[ReSID] cycles  per sample: %f\n", CYCLES_PER_SAMPLE);
}
