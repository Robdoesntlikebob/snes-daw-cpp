#include <SNES.h>
#define w(x,y) dsp->write(x,y)

/*functions*/

void smplimitcheck(uint amount) {
    smplimit += amount;
    if (smplimit > sample_count) {
        out();
        smplimit = 0;
    }
}
int getsmplimit() { return smplimit; }

void pitch(int p, int voice) {
    w(vpl(voice), p & 0xff);
    w(vph(voice), (p >> 8)&0x3f);
}


void advance(unsigned int samples) { dsp->run(32 * samples); /*smplimitcheck(32*samples);*/ }
void advance() { dsp->run(32); /*smplimitcheck(32);*/
}


void smp2aram(unsigned char* sample, unsigned int length, unsigned int lppoint) {
    memcpy(aram+smppos, sample, length);
    aram[0x1000 + dirposition++] = lobit(smppos);
    aram[0x1000 + dirposition++] = hibit(smppos);
    aram[0x1000 + dirposition++] = lobit(lppoint+smppos);
    aram[0x1000 + dirposition++] = hibit(lppoint+smppos);
    smppos += length;
}

void tick(unsigned int ticks) {dsp->run(5334 * ticks); /*smplimitcheck(5334*ticks);*/
}
void tick() {
    dsp->run(5334); /*smplimitcheck(5334);*/
}

//usage: tick(note(length_in_ticks, voice, hz, p, srcn, voll, volr, adsr1, adsr2))
int note(uint length_in_ticks, uint voice, bool hz, uint p, uint srcn, uint voll, uint volr, uint adsr1, uint adsr2) {
    vxkon++;
    w(vvoll(voice), std::trunc(voll/vxkon));
    w(vvolr(voice), std::trunc(volr / vxkon));
    w(vsrcn(voice), 1);
    if (hz == 1) pitch((4096 * (p / 32000)) & 0x3fff, voice);
    else pitch(p & 0x3fff, voice);
    w(vadsr1(voice), adsr1);
    w(vadsr2(voice), adsr2);
    w(KOF, 0<<voice); w(KON, 1<<voice);
    return length_in_ticks;
}

//usage: tick(note(length_in_ticks, voice, hz, p, srcn, vol, adsr1, adsr2))
int note(uint length_in_ticks, uint voice, bool hz, uint p, uint srcn, uint vol, uint adsr1, uint adsr2) {
    vxkon++;
    w(vvoll(voice), std::trunc(vol / vxkon));
    w(vvolr(voice), std::trunc(vol / vxkon));
    w(vsrcn(voice), 1);
    if (hz == 1) pitch((4096 * (p / 32000)) & 0x3fff, voice);
    else pitch(p & 0x3fff, voice);
    w(vadsr1(voice), adsr1);
    w(vadsr2(voice), adsr2);
    w(KOF, 0 << voice); w(KON, 1 << voice);
    return length_in_ticks;
}

void cmajor() {
    w(vvoll(0), 0x3F); w(vvolr(0), 0x3F); //1. VXVOL
    w(vvoll(1), 0x3F); w(vvolr(1), 0x3F); //1. VXVOL
    w(vsrcn(0), 1); w(vsrcn(1), 1); //2. VXSRCN
    w(vadsr1(0), ADSR + 0xA); //3. VXADSR(1+2)
    w(vadsr2(0), 0xe0);
    w(vadsr1(1), ADSR + 0xA); //3. VXADSR(1+2)
    w(vadsr2(1), 0xe0);
    pitch(0x10bf, 0); //4. VXP (function)
    pitch(0xe00, 1); //4. VXP (function)
    w(KOF, 0b11111100); //5. KOF
    w(KON, 3); //6. KON
    dsp->run(32 * 32000 / 4 - (256 * 32)); //RUN
    w(KOF, 3); advance(256); pitch(0x12bf, 0); pitch(0x1000, 1); w(KOF, 0); w(KON, 3);
    dsp->run(32 * 32000 / 4 - (256 * 32)); w(KON, 0);
    w(KOF, 3); advance(256); pitch(0x14ff, 0); pitch(0x10bf, 1); w(KOF, 0); w(KON, 3);
    dsp->run(32 * 32000 / 4 - (256 * 32)); w(KON, 0);
    w(KOF, 3); advance(256); pitch(0x167f, 0); pitch(0x12bf, 1); w(KOF, 0); w(KON, 3);
    dsp->run(32 * 32000 / 4 - (256 * 32)); w(KON, 0);
    w(KOF, 3); advance(256); pitch(0x18ff, 0); pitch(0x14ff, 1); w(KOF, 0); w(KON, 3);
    dsp->run(32 * 32000 / 4 - (256 * 32)); w(KON, 0);
    w(KOF, 3); advance(256); pitch(0x1c40, 0); pitch(0x167f, 1); w(KOF, 0); w(KON, 3);
    dsp->run(32 * 32000 / 4 - (256 * 32)); w(KON, 0);
    w(KOF, 3); advance(256); pitch(0x2000, 0); pitch(0x18ff, 1); w(KOF, 0); w(KON, 3);
    dsp->run(32 * 32000 / 4 - (256 * 32)); w(KON, 0);
    w(KOF, 3); advance(256); pitch(0x217f, 0); pitch(0x14ff, 1); w(KOF, 0); w(KON, 3);
    dsp->run(32 * 32000 / 4 - (256 * 32)); w(KON, 0);
    w(KOF, 3); advance(256);
}

/*main*/


//int main() {
//#define r(x) dsp->read(x)
//#define w(x,y) dsp->write(x,y)
//
//    /*initialising DSP and registers*/
//    if (dsp == NULL) { std::cout << "no DSP lol" << std::endl; }
//    dsp->init(aram);
//    if (aram == NULL) { std::cout << "no ARAM lol" << std::endl; }
//
//    /*DSP*/
//    out();
//    dsp->reset();
//
//
//    /*load instruments*/
//    smp2aram(BRR_SAWTOOTH, len(BRR_SAWTOOTH), 0);
//    smp2aram(c700sqwave, len(c700sqwave), 9);
//
//    /*initialisation*/
//    for (int i = 0; i < 0x80; i++) {
//        if (i == FLG) w(FLG, 0x60);
//        else if (i == ESA) w(ESA, 0x80);
//        else if (i == KOF) w(KOF, 0xff);
//        else w(i, 0);
//    }
//    w(FLG, ECEN); w(DIR, 0x10); w(MVOLL, 127); w(MVOLR, 127);
//
//    /*hopefully creating the DSP sound*/
//    tick(note(190, 0, 0, 0x107f, 1, 127, ADSR + 0xA, 0xE0));
//    w(KON, 0); w(KOF, 1); tick(2);
//    tick(note(190, 0, 0, 0x12af, 1, 127, ADSR + 0xA, 0xE0));
//    w(KON, 0); w(KOF, 1); advance(256);
//
//    /*debug (remove when not needed)*/
//    int generated_count = dsp->sample_count() / 2;
//    printf("Generated %d samples\n", generated_count);
//
//    for (int i = 0; i < 10; i++) {
//        printf("%02d: %04hx %04hx\n", i, output[i * 2], output[1 + i * 2]);
//    }
//
//
//    /*SFML side of playing sound*/
//    sf::SoundBuffer buf(output, 32*32000, 2, 32000, { sf::SoundChannel::FrontLeft,sf::SoundChannel::FrontRight });
//    sf::SoundBuffer bufwav("its gonna sound like shit trust me bro.wav");
//    sf::Sound snd(buf);
//    buf.saveToFile("debug.wav");
//    snd.play();
//    sf::sleep(sf::milliseconds(2100));
//
//}