#include "DEMLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>

DEMLoader::DEMLoader(float centerX, float centerZ, float Y, float length, float width, float pointSize, float minHeight, float maxHeight) {

	this->centerX = centerX;
	this->centerZ = centerZ;
	this->Y = Y;
	this->length = length;
	this->width = width;
	this->pointSize = pointSize;
	this->minHeight = minHeight;
	this->maxHeight = maxHeight;
	this->generatedPoints = std::vector<std::vector<TerrainPoint>>();



	std::ifstream demFile("SRTM2.asc");
	if (!demFile.is_open()) {
		std::cerr << "Failed to open the DEM file." << std::endl;
		return;
	}

	// Read header information
	std::string headerInfo;

	int cols, rows, maxRows = 1000, maxCols = 1000;
	float xllcorner, yllcorner, cellsize, NODATA_value;

	std::string line;
	std::string key;

	// Read header information line by line
	while (std::getline(demFile, line)) {
		std::istringstream iss(line);
		iss >> key;

		if (key == "ncols") {
			if (!(iss >> cols) || cols <= 0) {
				std::cerr << "Invalid number of columns." << std::endl;
				return;
			}
		}
		else if (key == "nrows") {
			if (!(iss >> rows) || rows <= 0) {
				std::cerr << "Invalid number of rows." << std::endl;
				return;
			}
		}
		else if (key == "xllcorner") {
			iss >> xllcorner;
		}
		else if (key == "yllcorner") {
			iss >> yllcorner;
		}
		else if (key == "cellsize") {
			iss >> cellsize;
		}
		else if (key == "NODATA_value") {
			iss >> NODATA_value;
			break; // Typically the last header entry, so break the loop
		}
	}

	float terrainMaxHeight = -0.000000f;
	float terrainMinHeight = 100000000.0f;

	// Limit the number of rows and columns to the specified maximum
	rows = std::min(rows, (int)(this->length / this->pointSize));
	cols = std::min(cols, (int)(this->width / this->pointSize));

	// Read elevation data
	std::vector<std::vector<float>> elevationData(rows, std::vector<float>(cols, 0.0f));

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			if (!(demFile >> elevationData[i][j])) {
				// Handle errors or NODATA value
				elevationData[i][j] = 0;
			}
			if (terrainMaxHeight < elevationData[i][j]) {
				terrainMaxHeight = elevationData[i][j];
			}
			else if (terrainMinHeight > elevationData[i][j]) {
				terrainMinHeight = elevationData[i][j];
			}
		}
		// If there are more columns in the file, skip the rest of the line
		if (cols < maxCols) {
			demFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	if (terrainMaxHeight == 0) {
		terrainMaxHeight = 1;
	}

	// Close the file
	demFile.close();

	for (int i = 0; i < rows; ++i) {
		std::vector<TerrainPoint> colPoints = std::vector<TerrainPoint>();
		for (int j = 0; j < cols; ++j) {
			colPoints.push_back(TerrainPoint(Vect3d(this->centerX + i*pointSize - this->length / 2, this->Y + (elevationData[i][j] - terrainMinHeight) / (terrainMaxHeight - terrainMinHeight) * (maxHeight - minHeight) + minHeight, this->centerZ + j * pointSize - this->width / 2)));
		}
		generatedPoints.push_back(colPoints);
	}
}

std::vector<std::vector<TerrainPoint>> DEMLoader::getTerrain() {
	return this->generatedPoints;
}

