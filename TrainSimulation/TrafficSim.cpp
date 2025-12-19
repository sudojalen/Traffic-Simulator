#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

/*    SIMULATION PARAMETERS    */
const float PIXELS_PER_METER = 20.0f;
const float TIME_STEP = 1.0f / 60.0f;

class TraficLight {
public:
	sf::CircleShape shape;
	int state; // 0 = RED, 1 = GREEN
	float timer;

	TraficLight(float x, float y) {
		shape.setRadius(15.f);
		shape.setPosition(x, y);
		shape.setFillColor(sf::Color::Red);
		state = 0;
		timer = 0;
	}

	void update(float dt) {
		timer += dt;
		if (timer > 10.0f) {		// 'x' second lights
			timer = 0;
			state = (state + 1) % 2;
			shape.setFillColor(state == 0 ? sf::Color::Red : sf::Color::Green);
		}
	}
};


class Car {
public:
	sf::RectangleShape shape;
	float x, y;
	float velocity;
	float maxSpeed;

	Car(float startX, float startY) {
		x = startX;
		y = startY;
		velocity = 0;
		maxSpeed = 150.0f;	// Roughly 30 mph after adjustment

		shape.setSize(sf::Vector2f(40.f, 20.f));
		shape.setFillColor(sf::Color::Blue);
		shape.setPosition(x, y);
	}

	void update(float dt, bool isGreenLight) {
		if (isGreenLight) {
			// accelerate
			if (velocity < maxSpeed) velocity += 100.0f * dt;
		}
		else {
			// brake
			if (velocity > 0) velocity -= 400.0f * dt;
			if (velocity < 0) velocity = 0;
		}

		x += velocity * dt;	// Update position based on velocity over time

		if (x > 800) x = -50;	// Rest position if car moves off screen

		shape.setPosition(x, y);
	}
};

int main() {
	/*    1. SETUP WINDOW    */
	sf::RenderWindow window(sf::VideoMode(800, 600), "TRAFFIC SIMULATION");
	window.setFramerateLimit(60.0f);

	/*    2. CREATE SIM OBJECTS    */
	// Car myCar(50.0f, 300.0f);
	std::vector<Car> fleet;
	float spawnTimer = 0.0f;
	TraficLight light(600.f, 250.f);

	sf::Clock clock;

	/*    3. SIMLUATION LOOP    */
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		// physics section
		sf::Time elapsed = clock.restart();
		float dt = elapsed.asSeconds();

		// logic: spawner
		spawnTimer += dt;
		if (spawnTimer > 5.0f) {	// spawn ever 'x' seconds
			if (fleet.empty() || fleet.back().x > 50.0f) {
				fleet.push_back(Car(-50.f, 300.f));
				spawnTimer = 0;
			}
		}

		// update traffic light (independant of cars)
		light.update(dt);


		// logic: fleet
		for (size_t i = 0; i < fleet.size(); i++) {
			float limitLineX = light.shape.getPosition().x;
			bool mustStop = false;

			if (light.state == 0) {
				if (fleet[i].x < limitLineX && (limitLineX - fleet[i].x) < 200) {
					mustStop = true;
				}
			}

			if (i > 0) {
				Car& leader = fleet[i - 1];
				float distanceToLeader = leader.x - fleet[i].x;

				if (distanceToLeader < 100.0f && distanceToLeader > 0) {
					mustStop = true;
				}
			}
			// update this specific car
			fleet[i].update(dt, !mustStop);
		}

		/*    RENDER SECTION    */
		window.clear(sf::Color(50, 50, 50));

		// draw road
		sf::RectangleShape roadStrip(sf::Vector2f(800.f, 100.f));
		roadStrip.setPosition(0, 260);
		roadStrip.setFillColor(sf::Color(30, 30, 30));
		window.draw(roadStrip);

		// draw light
		window.draw(light.shape);

		// draw fleet
		for (Car& car : fleet) {
			window.draw(car.shape);
		}

		window.display();

	}
	return 0;
}