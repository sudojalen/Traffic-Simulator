#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>

/*    SIMULATION PARAMETERS    */
const float PIXELS_PER_METER = 20.0f;
const float TIME_STEP = 1.0f / 60.0f;


enum LightState { RED, GREEN, YELLOW };

class TrafficLight {
public:
	sf::CircleShape shape;
	LightState state;
	float timer;

	TrafficLight(float x, float y) {
		shape.setRadius(15.f);
		shape.setPosition(x, y);
		shape.setFillColor(sf::Color::Red);
		state = RED;
		timer = 0;
	}

	void update(float dt) {
		timer += dt;

		if (state == GREEN && timer > 8.0f) {
			state = YELLOW;
			shape.setFillColor(sf::Color::Yellow);
			timer = 0;
		}
		else if (state == YELLOW && timer > 2.5f) {
			state = RED;
			shape.setFillColor(sf::Color::Red);
			timer = 0;
		}
		else if (state == RED && timer > 5.0f) {
			state = GREEN;
			shape.setFillColor(sf::Color::Green);
			timer = 0;
		}
	}
};



class Vehicle {
public:
	sf::RectangleShape shape;
	sf::RectangleShape sensor;
	float x, y;
	float velocity{ 0 };
	float maxSpeed{ 0 };
	float acceleration{ 0 };
	float brakingPower{ 0 };
	bool runsYellowLight;

	virtual ~Vehicle() {}

	Vehicle(float startX, float startY) {
		x = startX;
		y = startY;
		velocity = 0;
		runsYellowLight = false;
		sensor.setSize(sf::Vector2f(60.f, 10.f));
		sensor.setFillColor(sf::Color(255, 0, 0, 100));
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

		sensor.setPosition(x + 40.f, y + 5.f);
	}
};


class Car : public Vehicle {	// derived class, quick and agile
public:
	Car(float startX, float startY) : Vehicle(startX, startY) {
		maxSpeed = 150.0f;
		acceleration = 120.0f;
		brakingPower = 200.0f;

		shape.setSize(sf::Vector2f(20.f, 10.f));
		shape.setFillColor(sf::Color::Blue);
		shape.setPosition(x, y);
	}
};

class Truck : public Vehicle {	// derived class, slow and heavy
public:
	Truck(float startX, float startY) : Vehicle(startX, startY) {
		maxSpeed = 100.0f;
		acceleration = 100.0f;
		brakingPower = 150.0f;
		runsYellowLight = true;

		shape.setSize(sf::Vector2f(40.f, 15.f));
		shape.setFillColor(sf::Color::Red);
		shape.setPosition(x, y);
	}
};



/*    SHARED VARIABLES    */
std::mutex dataMutex;
std::vector<Vehicle*> fleet;
TrafficLight light(600.f, 250.f);
bool isSimulationRunning = true;

/*    PHYSICS LOOP    */
void PhysicsLoop() {
	sf::Clock physicsClock;
	float spawnTimer{ 0.0f };

	while (isSimulationRunning) {
		sf::Time elapsed = physicsClock.restart();
		float dt = elapsed.asSeconds();

		// LOCK data
		{
			std::lock_guard<std::mutex> lock(dataMutex);

			// garbage collection
			if (!fleet.empty() && fleet.front()->x > 800) {
				delete fleet.front();
				fleet.erase(fleet.begin());
			}

			// spawn logic
			spawnTimer += dt;
			if (spawnTimer > 2.0f) {
				if (fleet.empty() || fleet.back()->x > 100.f) {
					int r = rand() % 2;
					if (r == 0) fleet.push_back(new Car(-100.f, 300.f));
					else fleet.push_back(new Truck(-100.f, 300.f));
					spawnTimer = 0;
				}
			}

			// update light
			light.update(dt);

			// update fleet
			for (size_t i = 0; i < fleet.size(); i++) {
				float limitLineX = light.shape.getPosition().x;
				bool mustStop = false;

				if (light.state == RED) {
					if (fleet[i]->x < limitLineX && (limitLineX - fleet[i]->x) < 150) {
						mustStop = true;
					}
				}
				else if (light.state == YELLOW) {
					if (fleet[i]->runsYellowLight == false) {
						if (fleet[i]->x < limitLineX && (limitLineX - fleet[i]->x) < 150) {
							mustStop = true;
						}
					}
				}

				for (size_t j{ 0 }; j < fleet.size(); j++) {
					if (i == j) continue;	// dont check youself
					if (fleet[i]->sensor.getGlobalBounds().intersects(fleet[j]->shape.getGlobalBounds())) {
						mustStop = true;
						break;	// we found another car so stop
					}
				}
				// update this specific car
				fleet[i]->update(dt, !mustStop);
			}
		}	// UNLOCK

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}



int main() {
	/*    1. SETUP WINDOW    */
	sf::RenderWindow window(sf::VideoMode(800, 600), "TRAFFIC SIMULATION");
	window.setFramerateLimit(60);

	/*    2. LAUNGH PHYSICS THREAD    */
	std::thread bgThread(PhysicsLoop);

	/*    3. SIMLUATION LOOP    */
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		// RENDER section
		window.clear(sf::Color(50, 50, 50));
		
		{	// LOCK
			std::lock_guard<std::mutex> lock(dataMutex);

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
				window.draw(v->sensor);	// for debugging
			}
		}	// UNLOCK
		window.display();
	}

	isSimulationRunning = false;
	bgThread.join();

	return 0;
}