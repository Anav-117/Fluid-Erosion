#include "TerrainGenerator.h"
#include "math/vect3d.h"
#include "PerlinNoise2d.h"

TerrainGenerator::TerrainGenerator(PerlinNoise2d noise, float centerX, float centerY, float Z, float length, float width,float pointSize, float minHeight, int octaves, float persistance, float lacunarity) {
	this->centerX = centerX;
	this->centerY = centerY;
	this->Z = Z;
	this->length = length;
	this->width = width;
	this->pointSize = pointSize;
	this->minHeight = minHeight;
	this->octaves = octaves;
	this->persistance = persistance;
	this->lacunarity = lacunarity;
	this->noise = noise;
	this->generatedPoints = std::vector<Vect3d>();
	this->generatedVoxelPoints = std::vector<Vect3d>();
	this->GeneratePoints();
	this->GenerateVoxelPoints();
}

void TerrainGenerator::GeneratePoints() {
	this->generatedPoints.clear();
	for (float y = 0.0; y < this->width; y+=this->pointSize) {
		for (float x = 0.0; x < this->length; x+= this->pointSize) {
			float amplitude = 1;
			float frequency = 1;
			float noiseHeight = 0;

			for (int i = 0; i < this->octaves; i++) {
				float sampleX = x * frequency;
				float sampleY = y * frequency;

				float perlinNoise = this->noise.value(sampleX, sampleY);
				noiseHeight += perlinNoise * amplitude;

				amplitude *= this->persistance;
				frequency *= this->lacunarity;

			}
			this->generatedPoints.push_back(Vect3d(this->centerX + x - this->length / 2, this->centerY + y - this->width / 2, this->Z + noiseHeight + minHeight));
		}
	}
}

void TerrainGenerator::GenerateVoxelPoints() {
	this->generatedVoxelPoints.clear();
	for (int j = 0; j < this->generatedPoints.size(); j++) {
		Vect3d curPoint = this->generatedPoints[j];
		for (float k = this->Z; k < curPoint.z(); k+= this->pointSize) {
			this->generatedVoxelPoints.push_back(Vect3d(curPoint.x(), k, curPoint.y()));
		}
	}
}

std::vector<Vect3d> TerrainGenerator::points(bool regenerate) {
	if (regenerate) {
		this->GeneratePoints();
	}
	return this->generatedPoints;
}

std::vector<Vect3d> TerrainGenerator::voxelPoints(bool regenerate) {
	if (regenerate) {
		this->GeneratePoints();
		this->GenerateVoxelPoints();
	}
	return this->generatedVoxelPoints;
}


