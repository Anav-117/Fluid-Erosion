#include <random>
#include <vector>
#include "PerlinNoise2d.h"

float Vec2d::dot(Vec2d z) {
	return this->x * z.x + this->y * z.y;
}

PerlinNoise2d::PerlinNoise2d(int seed, int gridSize, int scaleX, int scaleY, int offsetX, int offsetY) {
	this->gridSize = gridSize;
	this->seed = seed;
	this->scaleX = scaleX;
	this->scaleY = scaleY;
	this->offsetX = offsetX;
	this->offsetY = offsetY;

	srand(seed);

	for (int i = 0; i < gridSize * gridSize; i++) {
		gradients.push_back({ (float)((rand() % 4) / 4.0 * 2.0 - 1.0), (float)((rand() % 4) / 4.0 * 2.0 - 1.0) });
	}
}

float PerlinNoise2d::lerp(float a, float b, float lerpAmt) {
	return a * (1 - lerpAmt) + lerpAmt * b;
}

float PerlinNoise2d::smooth(float val) {
	return val * val * val * (val * (val * 6 - 15) + 10);;
}

float PerlinNoise2d::value(float x, float y) {
	x *= this->scaleX;
	y *= this->scaleY;

	x = abs(x + this->offsetX);
	y = abs(y + this->offsetY);

	int int_x = (int)x;
	int int_y = (int)y;

	float frac_x = x - int_x;
	float frac_y = y - int_y;

	int_x = int_x % (this->gridSize - 1);
	int_y = int_y % (this->gridSize - 1);


	Vec2d gradient_tl = gradients[int_y * this->gridSize + int_x];
	Vec2d gradient_tr = gradients[int_y * this->gridSize + int_x + 1];
	Vec2d gradient_bl = gradients[(int_y + 1) * this->gridSize + int_x];
	Vec2d gradient_br = gradients[(int_y + 1) * this->gridSize + int_x + 1];

	Vec2d distance_tl = { frac_x, frac_y };
	Vec2d distance_tr = { frac_x - 1, frac_y };
	Vec2d distance_bl = { frac_x, frac_y - 1 };
	Vec2d distance_br = { frac_x - 1, frac_y - 1 };

	float dot_tl = gradient_tl.dot(distance_tl);
	float dot_tr = gradient_tr.dot(distance_tr);
	float dot_bl = gradient_bl.dot(distance_bl);
	float dot_br = gradient_br.dot(distance_br);

	frac_x = this->smooth(frac_x);
	frac_y = this->smooth(frac_y);

	float lerp_l = lerp(dot_tl, dot_bl, frac_y);
	float lerp_r = lerp(dot_tr, dot_br, frac_y);

	float out = (lerp(lerp_l, lerp_r, frac_x) + 1.0) / 2.0;


	return out;
}