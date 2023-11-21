#pragma once

#include "Particle.h"
#include <vector>
#include "TerrainPoint.h"
#include <math/vect3d.h>

typedef Particle* ParticlePtr;

class Fluid {
public:
	Fluid(int particleMatrixSize[]);
	std::vector<std::vector<std::vector<Particle>>> GetParticles() { return fluidParticles; }
	void GenerateKernel(Particle* part, ParticlePtr Kernel[], int& kernelSize);
	void UpdateDenstiy(Particle* part, ParticlePtr Kernel[], int kernelSize);
	void UpdatePressure(Particle* part, ParticlePtr Kernel[], int kernelSize);
	void UpdateViscosity(Particle* part, ParticlePtr Kernel[], int kernelSize);
	void UpdateExternalForce(Particle* part);
	void AdvectParticles();
	void SetTime(float time);
	void SetTerrain(std::vector<std::vector<TerrainPoint>> t);
	std::vector<std::vector<TerrainPoint>> GetTerrain() { return terrain; }
	Vect3d Reflect(Vect3d I, Vect3d N);
	Vect3d GetFriction(TerrainPoint closestPoint, Particle fluidParticle);
	bool ShouldErode(TerrainPoint point, Particle fluidParticle);
	bool ShouldDeposit(TerrainPoint point, Particle fluidParticle);
	Vect3d GetSource() { return source; }
	void SetSource(Vect3d src) { source = src; }


	std::vector <std::vector<std::vector<Particle>>> fluidParticles;
	double ParticleSize;
	double kernelRadius;
	double viscosIndex;
	double KernelScale;
	double KernelScalePressure;
	double KernelScaleViscous;
	float time;
	double stiffness;
	double restDensity;
	double bounceDamping;
	std::vector<std::vector<TerrainPoint>> terrain;
	std::vector<TerrainPoint> eroded;
	Vect3d source;
	int sizes[3];

};