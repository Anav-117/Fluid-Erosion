#pragma once
#include <vector>

struct Vec2d
{
    float x, y;
    float dot(Vec2d z);
};

class PerlinNoise2d {
private:
    int gridSize, seed, scaleX, scaleY, offsetX, offsetY;
    std::vector<Vec2d> gradients;

    float lerp(float a, float b, float lerpAmt);
    float smooth(float val);

public:
    PerlinNoise2d(int seed = 0, int gridSize = 32, int scaleX = 10, int scaleY = 10, int offsetX = 10, int offsetY = 10);
    float value(float x, float y);
};