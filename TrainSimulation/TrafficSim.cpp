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


class Vehicle {
public:
	sf::RectangleShape shape;
	float x, y;
	float velocity;
	float maxSpeed;
	float acceleration;
	float brakingPower;

	virtual ~Vehicle() {}

	Vehicle(float startX, float startY) {
		x = startX;
		y = startY;
		velocity = 0;
	}

	void update(float dt, bool isGreenLight) {
		if (isGreenLight) {
			if (velocity < maxSpeed) velocity += acceleration * dt;
		}
		else {
			if (velocity > 0) velocity -= brakingPower * dt;
			if (velocity < 0) velocity = 0;
		}

		x += velocity * dt;
		shape.setPosition(x, y);
	}
};


class Car : public Vehicle {	// derived class, quick and agile
public:
	Car(float startX, float startY) : Vehicle(startX, startY) {
		maxSpeed = 200.0f;
		acceleration = 150.0f;
		brakingPower = 400.0f;

		shape.setSize(sf::Vector2f(40.f, 20.f));
		shape.setFillColor(sf::Color::Blue);
		shape.setPosition(x, y);
	}
};

class Truck : public Vehicle {	// derived class, slow and heavy
public:
	Truck(float startX, float startY) : Vehicle(startX, startY) {
		maxSpeed = 125.0f;
		acceleration = 100.0f;
		brakingPower = 200.0f;

		shape.setSize(sf::Vector2f(80.f, 30.f));
		shape.setFillColor(sf::Color::Red);
		shape.setPosition(x, y);
	}
};

int main() {
	/*    1. SETUP WINDOW    */
	sf::RenderWindow window(sf::VideoMode(800, 600), "TRAFFIC SIMULATION");
	window.setFramerateLimit(60.0f);

	/*    2. CREATE SIM OBJECTS    */
	std::vector<Vehicle*> fleet;
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

		// garbage collection
		if (!fleet.empty() && fleet.front()->x > 800) {
			delete fleet.front();
			fleet.erase(fleet.begin());	// remove the first element from the vector
		}

		// logic: spawner
		spawnTimer += dt;
		if (spawnTimer > 2.0f) {
			if (fleet.empty() || fleet.back()->x > 100.f) {
				int r = rand() % 2;
				if (r == 0) {
					fleet.push_back(new Car(-100.f, 300.f));
				}
				else {
					fleet.push_back(new Truck(-100.f, 300.f));
				}
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
				if (fleet[i]->x < limitLineX && (limitLineX - fleet[i]->x) < 300) {
					mustStop = true;
				}
			}

			if (i > 0) {
				Vehicle* leader  = fleet[i - 1];
				float distanceToLeader = leader->x - fleet[i]->x;

				if (distanceToLeader < 100.0f && distanceToLeader > 0) {
					mustStop = true;
				}
			}
			// update this specific car
			fleet[i]->update(dt, !mustStop);
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
		for (Vehicle* v : fleet) {
			window.draw(v->shape);
		}

		window.display();

	}
	return 0;
}