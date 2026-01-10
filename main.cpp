#include <main.h>

using namespace sf;
u8* aram = new u8[65536];
static SPC_DSP* dsp = new SPC_DSP;
static SPC_Filter* f = new SPC_Filter;


void demo(/*c700sinewave only for now*/) {

	w(dsp->r_dir, 0x67);
	int dir = (r(dsp->r_dir)) << 8; //load c700sinewave into ARAM DIR
	int lp = 0;
	for (int i = 0; i < len(c700sinewave); i += 9) {
		if (c700sinewave[i] & 2) {
			lp = i; break;
		}
	}//loop detection
	aram[dir + 2] = lp & 255;
	aram[dir + 3] = lp >> 8;
	//Writing to ARAM DIR

	//Plays the sound (hopefully)
#define BUF 2048
	short buffer[BUF];
	w(0x04, 0); //V0SRCN, instrument 0 (supposedly 'c700sinewave')
	w(0x4d, 0b00000001); //KON for V0 only
	//Writing to buffer
	for (int i = 0; i < BUF; i++) {
		static int ptr = 0;
		if (ptr % 9 == 0) { ptr++;  continue; }
		buffer[i] = c700sinewave[ptr++];
		buffer[i] = (buffer[i] << 8) - 32768;
		if (ptr == len(c700sinewave)) ptr = lp;
	}
	dsp->set_output(buffer, BUF);
	dsp->run(32736); print("DSP sample count: "<<dsp->sample_count());
}

void main() {
	dsp->init(aram);
	dsp->reset();
	dsp->soft_reset();
	print("Hello from CPP");
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
				demo();
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

