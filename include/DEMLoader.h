#pragma once

#include <vector>
#include "TerrainPoint.h"

class DEMLoader {
public:
	DEMLoader(float centerX, float centerZ, float Y, float length, float width, float pointSize, float minHeight, float maxHeight);
	std::vector<std::vector<TerrainPoint>> getTerrain();
	std::vector<std::vector<TerrainPoint>> generatedPoints;
	float centerX, centerZ, Y, width, length, minHeight, maxHeight, pointSize;
};