#include "sndEMU/dsp.h"
#include "sndEMU/SPC_DSP.h"
#include "sndEMU/SPC_Filter.h"
#include "sndEMU/brrcodec.h"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include <iostream>
#include <stdio.h>
#include <stdc++.h>
#include <Windows.h>

#define print(x) std::cout<<x<<endl;

static SPC_DSP* dsp = spc_dsp_new();
typedef SPC_DSP::uint8_t u8;
typedef spc_dsp_sample_t smp_t;
u8 aram[65536];
using namespace std;
u8 dirpos = 0, spos = 0;
u8 aram_dir;
u8 srcn;
u8 smppos = 0;
//u8* sample_ptr = aram_dir + srcn[smp] * 4;

#define len(x) *(&x+1)-x

short c700sinewave[] = {
	0b00000000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0b10101100, 0x06, 0x11, 0x10, 0x00, 0x0F, 0xFF, 0xFF, 0xFE,
	0b01101000, 0xEA, 0xAC, 0xCE, 0xF1, 0x14, 0x36, 0x66, 0x77,
	0b01101000, 0x76, 0x55, 0x41, 0x2F, 0xEE, 0xBB, 0xAA, 0x99,
	0b01101000, 0xEA, 0xAC, 0xCE, 0xF1, 0x14, 0x36, 0x66, 0x77,
	0b01101011, 0x76, 0x55, 0x41, 0x2F, 0xEE, 0xBB, 0xAA, 0x99
};

short c700sqwave[] = {
	0b10000100, 0x00, 0x00, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
	0b11000000, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
	0b11000000, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
	0b11000000, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
	0b11000011, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77
};

short instruments[];

struct instrument {
	u8 start;
	u8 loop;
	short data[];
};

u8 brr2aram(short ins[], int length) {
	instrument x;

	int lp = 0;
	for (lp; lp < length; lp+=9) {
		if (ins[lp] & 0b00000010) { x.loop = (int)ins[lp]; print(x.loop) }
	}
		for (smppos; smppos < 1;smppos++) {
			aram[smppos] = ins[smppos];
		} //Loading data into ARAM block

	aram[aram_dir] = smppos & 255;
	aram[aram_dir + 1] = smppos >> 8;
	aram[aram_dir + 2] = lp & 255;
	aram[aram_dir + 3] = lp >> 8;

	print("loop point: " << lp << " (must be 54 and 45 in that order)\nstart point: " << smppos << " (must be 55 on the second run)");
	smppos++;
	aram_dir += 0x100; return smppos, aram_dir;
}

void main() {
	aram_dir = (u8)dsp->read(0x5d) << 8;
	if (!dsp) { dsp = spc_dsp_new(); }
	dsp->init(aram);
	dsp->write(0x5d, 0x67);
	dsp->write(0x04, srcn);
	brr2aram(c700sinewave, len(c700sinewave)); //adds the c700sinewave instrument
	brr2aram(c700sqwave, len(c700sqwave)); //adds the c700sqwave instrument
	vector<spc_dsp_sample_t> outbuf(1080);
	dsp->set_output(outbuf.data(), 512);
	dsp->write(0x4d, 0b00000001);
	dsp->mute_voices(0);
	dsp->run(10240000);
	print("samples written: " << dsp->sample_count() << " (has to be 54)");
	print("kon: " << dsp->check_kon());
	print("Hello from CPP");
}
