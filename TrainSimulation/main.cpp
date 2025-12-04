#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <vector>
#include <cstdlib>

struct Tree {
	sf::RectangleShape trunk;
	sf::CircleShape leaves;		// we will use a trianlge, circle with 3 sides
};

class Train {
public:
	// never change
	const float MASS = 20000.0f;
	const float FRICTION = 500.0f;

	// change every frame
	float position;				// distance from start
	float velocity;				// current speed
	float acceleration;			// how fast speed is changing
	float throttle;				// how much gas are we giving

	Train() {
		position =		0.0f;
		velocity =		0.0f;
		acceleration =	0.0f;
		throttle =		0.0f;
	}


	void update(float dt) {
		// calculate force from engine
		// if throttle is 100%, force is 200,000 newtons
		// if throttle is 50%, force is 100,00 newtons
		float engineForce = (throttle / 100.0f) * 40000.0f;

		// claculate friction
		// friction always pushes OPPOSITE to velocity
		float frictionForce = velocity * FRICTION;

		// calculate net force (aka total push)
		float netForce = engineForce - frictionForce;

		// Newtons second law
		acceleration = netForce / MASS;

		// integration
		velocity = velocity + (acceleration * dt);

		//update position based on velocity
		position = position + (velocity * dt);

	}

};



int main() {
	// Create the window
	sf::RenderWindow window(sf::VideoMode(1000, 600), "Train Sim");

	// Create camera/view
	sf::View camera = window.getDefaultView();

	// Limit framerate
	window.setFramerateLimit(60);

	// FOREST SETUP
	std::vector<Tree> forest;

	for (int i = 0; i < 100; i++) {
		Tree t;

		// random size/height
		float trunkHeight = (rand() % 30) + 50.0f;
		float trunkWidth = 15.0f;

		// setup trunk
		t.trunk.setSize(sf::Vector2f(trunkWidth, trunkHeight));
		t.trunk.setFillColor(sf::Color(101, 67, 33));

		// leaves
		float leafSize = (rand() % 30) + 20.0f;
		t.leaves.setRadius(leafSize);
		t.leaves.setPointCount(3);
		t.leaves.setFillColor(sf::Color(34, 139, 34));
		t.leaves.setOrigin(leafSize, leafSize);

		float xPos = (i * 100.0f + (rand() % 100));
		float yPos = 300.0f - trunkHeight;

		t.trunk.setPosition(xPos, yPos);
		t.leaves.setPosition(xPos + (trunkWidth / 2), yPos);

		forest.push_back(t);
	}


	// Track setup
	sf::RectangleShape track(sf::Vector2f(1000000.0f, 10.0f));				// ground line
	track.setFillColor(sf::Color::Black);
	track.setPosition(0, 400);												// under the train

	sf::RectangleShape railTies(sf::Vector2f(10.0f, 20.0f));
	railTies.setFillColor(sf::Color::Magenta);

	// create train object
	Train myTrain;

	sf::RectangleShape trainShape(sf::Vector2f(100.0f, 60.0f));
	trainShape.setFillColor(sf::Color::Red);

	trainShape.setOrigin(50.0f, 30.0f);

	trainShape.setPosition(100.0f, 400.0f);

	// Init the clock
	sf::Clock clock;

	// GAME LOOP
	while (window.isOpen()) {
		// calculate delta time (how many seconds since last frame)
		sf::Time dtTime = clock.restart();
		float dt = dtTime.asSeconds();


		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
				// if right arrow is held, full throttle
				myTrain.throttle = 100.0f;
			}
			else {
				// otherwise, we coastin
				myTrain.throttle = 0.0f;
			}
		}

		window.clear(sf::Color::Blue);

		// PHYSICS
		myTrain.update(dt);
		float parralaxFactor = 0.98f;
		float treeMoveAmount = myTrain.velocity * dt * parralaxFactor;

		for (auto& tree : forest) {			// Parrallax logic
			tree.trunk.move(treeMoveAmount, 0);
			tree.leaves.move(treeMoveAmount, 0);
		}

		printf("Speed: %.2f m/s  |  Throttle: %.0f%% \r", myTrain.velocity, myTrain.throttle);
		float currentPosMeters = myTrain.position;
		trainShape.setPosition(100.0f + (currentPosMeters * 10.0f), 400.0f);

		// UPDATE CAMERA
		float trainX = trainShape.getPosition().x;
		camera.setCenter(trainX, 300);
		window.setView(camera);

		/* --- RENDER LOOP --- */ 
		// TREES
		for (const auto& tree : forest) {
			window.draw(tree.trunk);
			window.draw(tree.leaves);
		}

		// TRACK
		window.draw(track);							
		for (int i = 0; i < 100; i++) {
			float railTieX = i * 500.0f;

			railTies.setPosition(railTieX, 400.0f);

			window.draw(railTies);
		}
		
		// TRAIN
		window.draw(trainShape);

		// FINAL
		window.display();

	}
	return 0;
}