#ifndef SNES_H
#define SNES_H

#include <sndEMU/dsp.h>
#include <sndEMU/SPC_DSP.h>
#include <stdc++.h>
#include <SFML/Audio.hpp>
using namespace sf;
#define print(x) std::cout << x << std::endl

/*necessary variables*/
typedef unsigned int uint;
uint smppos = 0x200;
SPC_DSP* dsp = new SPC_DSP;
char* aram = (char*)calloc(0x10000, sizeof(char*));
uint dirposition = 0;
uint smplimit = 0;
int sample_count = 32 * 32000;
spc_dsp_sample_t* output = (spc_dsp_sample_t*)calloc(2 * sample_count, sizeof(spc_dsp_sample_t)); //buffer5
int out() {
    dsp->set_output(output, 2 * sample_count);
    if (output == NULL) { print("you are fucking stupid"); return -1; }
}



unsigned char BRR_SAWTOOTH[] = {
    0xB0, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
    0xB3, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
};

unsigned char c700sqwave[] = {
    0b10000100, 0x00, 0x00, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
    0b11000000, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
    0b11000000, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
    0b11000000, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
    0b11000011, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77
};

// Constants for the DSP registers.

#define vvoll(n) (n<<4)
#define vvolr(n) (n<<4)+1
#define vpl(n) (n<<4)+2
#define vph(n) (n<<4)+3
#define vsrcn(n) (n<<4)+4
#define vadsr1(n) (n<<4)+5
#define vadsr2(n) (n<<4)+6
#define vgain(n) (n<<4)+7
#define venvx(n) (n<<4)+8
#define voutx(n) (n<<4)+9
#define len(x) *(&x+1)-x
#define lobit(x) x&0xff
#define hibit(x) x>>8
int vxkon;

enum {
    MVOLL = 0x0C,
    MVOLR = 0x1C,
    FLG = 0x6C,
    DIR = 0x5D,
    KON = 0x4C,
    KOF = 0x5C,
    ADSR = 128,
    PMON = 0x2D,
    NON = 0x3D,
    ESA = 0x6D,
    RESET = 128,
    MUTE = 64,
    ECEN = 32,
    EON = 0x4D,
};

void smplimitcheck(uint amount);
int getsmplimit();
void advance(unsigned int samples); void advance();
void smp2aram(unsigned char* sample, unsigned int length, unsigned int lppoint);
void tick(unsigned int ticks); void tick();
int note(uint length_in_ticks, uint voice, bool hz, uint p, uint srcn, uint voll, uint volr, uint adsr1, uint adsr2);
int note(uint length_in_ticks, uint voice, bool hz, uint p, uint srcn, uint vol, uint adsr1, uint adsr2);
void cmajor();

class SPC700 : public SoundStream {
private:
    /*SFML*/
    bool onGetData(Chunk& data) override {
        const int count = 2;
        data.samples = &sfOut[sfCSample];
        if (sfCSample + count <= sfOut.size()) {
            data.sampleCount = count;
            data.samples += count;
            return true;
        }
        else {
            data.sampleCount = sfOut.size() - sfCSample;
            sfCSample = sfOut.size();
            return false;
        }
    }
    void onSeek(Time offset) override {
        sfCSample = static_cast<std::size_t>(offset.asSeconds() * getSampleRate() * getChannelCount());
    }

    std::vector<std::int16_t> sfOut;
    std::size_t sfCSample{};
    struct sample {
        uint dirstart, loloop, hiloop;
    };

    /*DSP*/
    SPC_DSP* d = new SPC_DSP;
    char* ar = (char*)calloc(0x10000, 0x10000);
    uint dirposition = 0;
    sf::SoundBuffer buf;

public:
    typedef SPC_DSP::sample_t sample_t;
    sample_t* out = (sample_t*)calloc(2, sizeof(sample_t));
    SPC700() {
#undef w(x,y)
#undef r(x)
#undef lobit(x)
#undef hibit(x)
#define w(x,y) d->write(x,y)
#define r(x) d->read(x)
#define lobit(x) x&255
#define hibit(x) x>>8
        d->init(ar);
        d->set_output(out, 2);
        for (int i = 0; i < SPC_DSP::register_count; i++) {
            if (i == FLG) w(FLG, 0x60);
            else if (i == ESA) w(ESA, 0x80);
            else if (i == KOF) w(KOF, 0xff);
            else w(i, 0);
        }
        w(DIR, 0xff); w(MVOLL, 127); w(MVOLR, 127); w(FLG, ECEN);
    }
    void load(const SoundBuffer& out) {
        sfOut.assign(out.getSamples(), out.getSamples() + out.getSampleCount());
        sfCSample = 0;
        initialize(out.getChannelCount(), out.getSampleRate(), { SoundChannel::FrontLeft,SoundChannel::FrontRight });
    }

    SPC700::sample newsample(unsigned char* sample, uint length, uint lppoint) {
        SPC700::sample x = { 0,0,0 };
        memcpy(ar + smppos, sample, length);
        x.dirstart = 0xff00 + dirposition;
        ar[0xff00 + dirposition++] = lobit(smppos);
        ar[0xff00 + dirposition++] = hibit(smppos);
        ar[0xff00 + dirposition++] = lobit(lppoint + smppos); x.loloop = hibit(smppos);
        ar[0xff00 + dirposition++] = hibit(lppoint + smppos); x.hiloop = hibit(smppos);
        smppos += length;
        return x;
    }

    void changeloop(SPC700::sample& sample, uint newloop) {
        ar[sample.dirstart + 2] = lobit(newloop); sample.loloop = lobit(newloop);
        ar[sample.dirstart + 3] = hibit(newloop); sample.loloop = hibit(newloop);
    }

    void pitch(uint voice, uint pitch) {
        w(vpl(voice), lobit(pitch));
        w(vph(voice), hibit(pitch) & 0x3f);
    }

    void advance() { d->run(32); d->set_output(out, 2); }
    void advance(uint samples) {
        for (int i = 0; i < samples; i++) {
            advance();
        }
    }

    void tick() {
        for (int i = 0; i < 166.6875f; i++) {
            advance(); //5334 clocks, hence that weird fucking float number
        }
    }
    void tick(uint ticks) {
        for (int i = 0; i < ticks; i++) {
            tick();
        }
    }

    //usage: tick(note(length_in_ticks, voice, hz, p, srcn, voll, volr, adsr1, adsr2));
    int note(uint length_in_ticks, uint voice, bool hz, uint p, uint srcn, uint voll, uint volr, uint adsr1, uint adsr2) {
        vxkon++;
        w(vvoll(voice), std::trunc(voll / vxkon));
        w(vvolr(voice), std::trunc(volr / vxkon));
        w(vsrcn(voice), 1);
        if (hz == 1) pitch((4096 * (p / 32000)) & 0x3fff, voice);
        else pitch(p & 0x3fff, voice);
        w(vadsr1(voice), adsr1);
        w(vadsr2(voice), adsr2);
        w(KOF, 0 << voice); w(KON, 1 << voice);
        return length_in_ticks;
    }

    //usage: tick(note(length_in_ticks, voice, hz, p, srcn, vol, adsr1, adsr2));
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

protected:
    uint smppos = 0x200;
    uint vxkon = 0;
};



#endif