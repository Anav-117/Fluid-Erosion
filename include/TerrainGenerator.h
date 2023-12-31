#pragma once
#include <vector>
#include "math/vect3d.h"
#include "PerlinNoise2d.h"
#include "TerrainPoint.h"


class TerrainGenerator {
private:
	int octaves;
	float centerX, centerZ, Y, width, length, minHeight, maxHeight, persistance, lacunarity, pointSize;
	PerlinNoise2d noise;
	std::vector<std::vector<TerrainPoint>> generatedPoints;
	std::vector<std::vector<std::vector<TerrainPoint>>> generatedVoxelPoints;

public:
	TerrainGenerator(PerlinNoise2d noise, float centerX, float centerZ, float Y, float length, float width,float pointSize = 0.05, float minHeight = 0.0, float maxHeight = 2.0, int octaves = 3, float persistance = 0.3, float lacunarity = 10.0);
	void GeneratePoints();
	void GenerateVoxelPoints();
	std::vector<std::vector<TerrainPoint>> TerrainGenerator::points(bool regenerate = false);
	std::vector<std::vector<std::vector<TerrainPoint>>> TerrainGenerator::voxelPoints(bool regenerate = false);
};