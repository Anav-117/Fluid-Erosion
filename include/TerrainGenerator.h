#pragma once
#include <vector>
#include "math/vect3d.h"
#include "PerlinNoise2d.h"


class TerrainGenerator {
private:
	int octaves;
	float centerX, centerY, Z, width, length, minHeight, persistance, lacunarity, pointSize;
	PerlinNoise2d noise;
	std::vector<Vect3d> generatedPoints;
	std::vector<Vect3d> generatedVoxelPoints;

public:
	TerrainGenerator(PerlinNoise2d noise, float centerX, float centerY, float Z, float length, float width,float pointSize = 0.05, float minHeight = 0.0, int octaves = 3, float persistance = 0.3, float lacunarity = 10.0);
	void GeneratePoints();
	void GenerateVoxelPoints();
	std::vector<Vect3d> TerrainGenerator::points(bool regenerate = false);
	std::vector<Vect3d> TerrainGenerator::voxelPoints(bool regenerate = false);
};