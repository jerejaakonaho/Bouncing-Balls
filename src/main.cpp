#include <SFML/Graphics.hpp>
#include "ParticleEngine.hpp"

int main() {
    unsigned int windowHeight{1000};
    unsigned int windowWidth{1000};

	sf::RenderWindow window( sf::VideoMode( { windowHeight, windowWidth } ), "Particle Simulation" );

	sf::Clock fpsClock;
	int frameCount{0};
	sf::Clock dtClock;

	ParticleEngine particleEngine(windowHeight, windowWidth);

	while ( window.isOpen() )
	{
	    float dt = dtClock.restart().asSeconds();

		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();
		}

		window.clear();
		particleEngine.CircleLoop(windowHeight, windowWidth, dt);;
		particleEngine.drawParticles(window);
		frameCount++;
		if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
		    float fps  = frameCount / fpsClock.restart().asSeconds();
			window.setTitle("Particle Simulation, FPS: " + std::to_string(static_cast<int>(fps)));
			frameCount = 0;
		}
		window.display();
	}
}
