#include <main.h>
#include <SNES.h>
using namespace sf;

void main() {
	SPC700* emu = new SPC700;
	SoundBuffer data(emu->out, sizeof(SPC700::sample_t));
	Sound snd(data);
	snd.play();

	//UI side
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
				print("bro its not on yet tf");
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

