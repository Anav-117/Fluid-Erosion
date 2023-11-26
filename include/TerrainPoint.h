#pragma once

#include "math/vect3d.h" 
#include <vector>

class TerrainPoint {
public:
	Vect3d pt;
	bool isSurface;
	int coordi;
	int coordj;
	int coordk;
	Vect3d normal;
	bool isEroded;
	bool isDeposited;

	TerrainPoint() {
		pt = Vect3d(0, 0, 0);
		normal = Vect3d(0, 0, 0);
		isSurface = false;
		coordi = 0;
		coordj = 0;
		coordk = 0;
		isEroded = false;
		isDeposited = false;
	}

	TerrainPoint(Vect3d point) {
		pt = point;
		normal = Vect3d(0, 0, 0);
		isSurface = false;
		coordi = 0;
		coordj = 0;
		coordk = 0;
		isEroded = false;
		isDeposited = false;
	}

	void SetCoords(int i, int j, int k) {
		coordi = i;
		coordj = j;
		coordk = k;
	}

	void SetNormal(Vect3d n) {
		normal = n;
	}

};

