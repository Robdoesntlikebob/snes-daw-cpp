#include <main.h>

using namespace sf;
u8* aram = new u8[65536];
static SPC_DSP* dsp = new SPC_DSP;
static SPC_Filter* f = new SPC_Filter;

int smppos, dspDIR;
int dir = 0x6700;

void toaram(short* sample, short length) {
	w(dsp->r_dir, 0x67);
	int lp = 0;

	//loop detection starts here
	for (int i = 0; i < len(c700sinewave); i += 9) {
		if (c700sinewave[i] & 2) {
			lp = i; break; print(lp);
		}
	}//loop detection ends here

	//start of writing samples
	for (int x = 0; x < length;x++) {
		aram[x] == sample[x];
		smppos += 1;
	}//end of writing samples

	aram[dir] = smppos & 255;
	aram[dir + 1] = smppos >> 8;
	aram[dir + 2] = lp & 255;
	aram[dir + 3] = lp >> 8;

	dir += 0x100;

}

void demo(int srcn) {
	#define BUF 1024
	spc_dsp_sample_t buffer[BUF];
	w(0x04, srcn); //V0SRCN, instrument 0 (supposedly 'c700sinewave')
	w(dsp->r_kon, 0b00000001); //KON for V0 only
	w(dsp->r_mvoll, 0x7f);
	w(dsp->r_mvolr, 0x7f);
	dsp->set_output(buffer, BUF);
	dsp->run((BUF*16)-32);

	/*debug: check that sound is actually being written to buffer*/
	for (auto i : buffer) {
		print(buffer[i]);
	}
	if (dsp->check_kon() == 1) { print("how tf is KON on tho? KON = " << dsp->check_kon()); }
	print("DSP sample count: "<<dsp->sample_count());
}

void main() {
	dsp->init(aram);
	dsp->reset();
	dsp->soft_reset();
	w(dsp->r_dir, 0x67);
	toaram(c700sinewave, len(c700sinewave));
	printf("0x%04X\n",dir);
	toaram(c700sqwave, len(c700sqwave));
	printf("0x%04X\n",dir);

	print("Hello from SMPiano.");
	RenderWindow window(VideoMode({ 1280,720 }), "SMPiano", Style::Default);
	RectangleShape demobtn({});
	demobtn.setSize({ 150, 50 });
	demobtn.setOrigin(demobtn.getLocalBounds().getCenter());
	bool pressed = false;
	while (window.isOpen()) {
		auto mpos = Vector2f(Mouse::getPosition(window));
		while (const std::optional event = window.pollEvent()) {
			#define event(n) event->is<Event::##n>()
			if (event(Closed)) {
				window.close();
			}
			if (event(Resized)) {
				View view(sf::FloatRect({ 0,0 }, Vector2f(window.getSize())));
				window.setView(view);
			}
			if (event(MouseButtonPressed) && demobtn.getGlobalBounds().contains(mpos) && Mouse::isButtonPressed(Mouse::Button::Left)) {
				demo(1);
			}
		}//end of if statements
		window.clear(Color(0xb5, 0xb6, 0xe4, 255)); //set bg (always keep up here)

		auto [wwidth, wheight] = window.getSize();

		demobtn.setPosition({wwidth / 2.0f, wheight / 2.0f});
		window.draw(demobtn);
		if (demobtn.getGlobalBounds().contains(mpos)) {
			if (Mouse::isButtonPressed(Mouse::Button::Left)) {
				demobtn.setFillColor(Color(0x40, 0x40, 0x40));
			}
			else { demobtn.setFillColor(Color(0x7f, 0x7f, 0x7f)); }
		}
		else { demobtn.setFillColor(Color(0xff, 0xff, 0xff)); }
		window.display(); //update
	}//end of window
}//end of main()

