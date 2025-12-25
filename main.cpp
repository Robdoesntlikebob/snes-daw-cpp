#include <sndEMU/dsp.h>
#include <sndEMU/SPC_DSP.h>
#include <sndEMU/SPC_Filter.h>
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <iostream>
#include <stdio.h>
#include <stdc++.h>
#include <Windows.h>
#include <SDL3/SDL.h>

#ifndef print(x)
#define print(x) std::cout<<x<<std::endl
#endif
#ifndef len(x)
#define len(x) *(&x+1)-x
#endif
#ifndef w(x,y)
#define w(x,y) dsp->write(x,y)
#endif
#ifndef r(x,y)
#define r(x) dsp->read(x)
#endif

static int aram[65536];
static SPC_DSP* dsp = new SPC_DSP;
static SPC_Filter* f = new SPC_Filter;

SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 32000 };
SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);

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

void loadAudio() {
	SDL_Init(SDL_INIT_AUDIO);
	SDL_ResumeAudioStreamDevice(stream);
	dsp->init(aram);
	dsp->reset();
	dsp->soft_reset();
}

void killAudio() {
	SDL_DestroyAudioStream(stream);
	SDL_Quit();
}

void demo(/*c700sinewave only for now*/) {

	//load c700sinewave into ARAM DIR
	w(dsp->r_dir, 0x67);
	int dir = (r(dsp->r_dir)) << 8;
	//loop detection
	int lp = 0;
	for (lp; lp < len(c700sinewave); lp += 9) {
		if (c700sinewave[lp] & 2) {
			lp = c700sinewave[lp]; break;
		}
	}
	aram[dir + 2] = lp & 255;
	aram[dir + 3] = lp >> 8;

	//Plays the sound (hopefully)
	#define BUF 2048
	short buffer[BUF];
	w(0x04,0); //V0SRCN, instrument 0 (supposedly 'c700sinewave')
	w(0x4d, 0b00000001); //KON for V0 only
	dsp->set_output(buffer, BUF);
	for (int i = 0; i < BUF; i++) {
		static int ptr = 0;
		buffer[i] = c700sinewave[ptr++];
		if (ptr == len(c700sinewave)) ptr = lp;
	}
	SDL_PutAudioStreamData(stream, buffer, sizeof(buffer));
	while (SDL_GetAudioStreamQueued(stream) > 0) SDL_Delay(1);
	killAudio();
	dsp->run(1024); print(dsp->sample_count());
}


void main() {
	print("Hello from CPP");
	loadAudio();
	demo();
}

