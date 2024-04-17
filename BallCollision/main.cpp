#include "SFML/Graphics.hpp"
#include "SFML/System/Vector2.hpp"
#include "MiddleAverageFilter.h"
#include <iostream>

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MIN_BALLS = 80;
constexpr int MAX_BALLS = 100;
constexpr int MIN_BALLS_RADIUS = 5;
constexpr int MAX_BALLS_RADIUS = 30;
constexpr int SPEED_LIMIT = 150;

Math::MiddleAverageFilter<float, 100> fpscounter;

struct Ball
{
	sf::Vector2f position;
	sf::Vector2f direction;
	float radius;
	float mass;
};

void drawBall(sf::RenderWindow& window, const Ball& ball)
{
	sf::CircleShape gball;
	gball.setRadius(ball.radius);
	//with the pivot shifted to the center of the circle
	gball.setPosition(ball.position.x - ball.radius, ball.position.y - ball.radius);
	window.draw(gball);
}

int randMinMax(int min, int max) {
	int range = max - min + 1;
	return rand() % range + min;
}

void moveBall(Ball& ball, float deltaTime)
{
	if (ball.direction.x > SPEED_LIMIT)
		ball.direction.x = SPEED_LIMIT;

	if (ball.direction.y > SPEED_LIMIT)
		ball.direction.y = SPEED_LIMIT;

	ball.position += ball.direction * deltaTime;
}

void drawFPS(sf::RenderWindow& window, float fps)
{
	char c[32];
	snprintf(c, 32, "FPS: %f", fps);
	std::string string(c);
	sf::String str(c);
	window.setTitle(str);
}

float pivotDistance(Ball& ball1, Ball& ball2) {
	return sqrtf((ball1.position.x - ball2.position.x) * (ball1.position.x - ball2.position.x)
		+ (ball1.position.y - ball2.position.y) * (ball1.position.y - ball2.position.y));
}

void handleBallCollision(Ball& ball1, Ball& ball2) {
	float distance = pivotDistance(ball1, ball2);
	float edgeDistance = distance - ball1.radius - ball2.radius;

	//ball bounce calculation
	if (edgeDistance < 0) {
		sf::Vector2f overlap = (edgeDistance * 0.5f) * (ball1.position - ball2.position) / distance;
		ball1.position -= overlap;
		ball2.position += overlap;

		sf::Vector2f normal = (ball2.position - ball1.position) / distance;
		sf::Vector2f k = ball1.direction - ball2.direction;
		float p = (normal.x * k.x + normal.y * k.y) / (ball1.mass + ball2.mass) * 2.0f;
		ball1.direction = ball1.direction - p * ball2.mass * normal;
		ball2.direction = ball2.direction + p * ball1.mass * normal;
	}
}

void handleWallCollision(Ball& ball) {
	//check for collision with wall x
	if (ball.position.x - ball.radius < 0) {
		ball.direction.x *= -1;
		ball.position.x = ball.radius;
	}
	if (ball.position.x + ball.radius > WINDOW_X) {
		ball.direction.x *= -1;
		ball.position.x = WINDOW_X - ball.radius;
	}

	//check for collision with wall y
	if (ball.position.y - ball.radius < 0) {
		ball.direction.y *= -1;
		ball.position.y = ball.radius;
	}
	if (ball.position.y + ball.radius > WINDOW_Y) {
		ball.direction.y *= -1;
		ball.position.y = WINDOW_Y - ball.radius;
	}
}

bool overlapCheck(std::vector<Ball>& balls, Ball& ball) {
	for (int i = 0; i < balls.size(); ++i) {
		float distance = pivotDistance(balls[i], ball);

		if (distance - ball.radius - balls[i].radius < 0) {
			return true;
		}
	}

	return false;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
	srand(time(NULL));

	std::vector<Ball> balls;

	// randomly initialize balls
	int ballCount = randMinMax(MIN_BALLS, MAX_BALLS);
	for (int i = 0; i < ballCount; ++i)
	{
		Ball newBall;
		newBall.radius = randMinMax(MIN_BALLS_RADIUS, MAX_BALLS_RADIUS);
		newBall.direction.x = randMinMax(-100, 100);
		newBall.direction.y = randMinMax(-100, 100);
		newBall.mass = newBall.radius * 10;

		//attempts to position the ball without overlap with previous ones
		while (overlapCheck(balls, newBall))
		{
			newBall.position.x = randMinMax(MAX_BALLS_RADIUS, WINDOW_X - MAX_BALLS_RADIUS);
			newBall.position.y = randMinMax(MAX_BALLS_RADIUS, WINDOW_Y - MAX_BALLS_RADIUS);
		}

		balls.push_back(newBall);
	}

	sf::Clock clock;
	float lastime = clock.restart().asSeconds();

	while (window.isOpen())
	{
		sf::Event event;
		window.pollEvent(event);
		if (event.type == sf::Event::Closed)
		{
			window.close();
		}

		float current_time = clock.getElapsedTime().asSeconds();
		float deltaTime = current_time - lastime;
		fpscounter.push(1.0f / (current_time - lastime));
		lastime = current_time;

		window.clear();

		//handling collisions with walls and other balls
		for (int i = 0; i < ballCount; ++i) {
			handleWallCollision(balls[i]);

			for (int j = i + 1; j < ballCount; ++j) {
				handleBallCollision(balls.at(i), balls.at(j));
			}

			moveBall(balls[i], deltaTime);
			drawBall(window, balls[i]);
		}

		window.display();
	}

	return 0;
}
