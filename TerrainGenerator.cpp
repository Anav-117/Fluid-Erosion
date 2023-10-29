#include "TerrainGenerator.h"
#include "math/vect3d.h"
#include "PerlinNoise2d.h"

TerrainGenerator::TerrainGenerator(PerlinNoise2d noise, float centerX, float centerZ, float Y, float length, float width, float pointSize, float minHeight, float maxHeight, int octaves, float persistance, float lacunarity) {
	this->centerX = centerX;
	this->centerZ = centerZ;
	this->Y = Y;
	this->length = length;
	this->width = width;
	this->pointSize = pointSize;
	this->minHeight = minHeight;
	this->maxHeight = maxHeight;
	this->octaves = octaves;
	this->persistance = persistance;
	this->lacunarity = lacunarity;
	this->noise = noise;
	this->generatedPoints = std::vector<Vect3d>();
	this->generatedVoxelPoints = std::vector<std::vector<Vect3d>>();
	this->GeneratePoints();
	this->GenerateVoxelPoints();
}

void TerrainGenerator::GeneratePoints() {
	this->generatedPoints.clear();
	for (float z = 0.0; z < this->width; z+=this->pointSize) {
		for (float x = 0.0; x < this->length; x+= this->pointSize) {
			float amplitude = 1.0;
			float frequency = 1.0;
			float noiseHeight = 0.0;

			for (int i = 0; i < this->octaves; i++) {
				float sampleX = x * frequency;
				float sampleZ = z * frequency;

				float perlinNoise = this->noise.value(sampleX, sampleZ);
				noiseHeight += perlinNoise * amplitude;

				amplitude *= this->persistance;
				frequency *= this->lacunarity;
			}
			this->generatedPoints.push_back(Vect3d(this->centerX + x - this->length / 2, this->Y + noiseHeight * (maxHeight - minHeight) + minHeight, this->centerZ + z - this->width / 2));
		}
	}
}

void TerrainGenerator::GenerateVoxelPoints() {
	this->generatedVoxelPoints.clear();
	for (int j = 0; j < this->generatedPoints.size(); j++) {
		Vect3d curPoint = this->generatedPoints[j];
		std::vector<Vect3d> vp;
		for (float k = this->Y; k < curPoint.y(); k+= this->pointSize) {
			vp.push_back(Vect3d(curPoint.x(), k, curPoint.z()));
		}
		this->generatedVoxelPoints.push_back(vp);
	}
}

std::vector<Vect3d> TerrainGenerator::points(bool regenerate) {
	if (regenerate) {
		this->GeneratePoints();
	}
	return this->generatedPoints;
}

std::vector<std::vector<Vect3d>> TerrainGenerator::voxelPoints(bool regenerate) {
	if (regenerate) {
		this->GeneratePoints();
		this->GenerateVoxelPoints();
	}
	return this->generatedVoxelPoints;
}


