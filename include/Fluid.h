#pragma once

#include "Particle.h"
#include <vector>
#include "TerrainPoint.h"

class Fluid {
public:
	Fluid(int particleMatrixSize[]);
	std::vector<std::vector<std::vector<Particle>>> GetParticles() { return fluidParticles; }
	void GenerateKernel(Particle* part);
	void UpdateDenstiy(Particle* part);
	void UpdatePressure(Particle* part);
	void UpdateViscosity(Particle* part);
	void UpdateExternalForce(Particle* part);
	void AdvectParticles();
	void SetTime(float time);
	void SetTerrain(std::vector<std::vector<TerrainPoint>> t);
	std::vector<std::vector<TerrainPoint>> GetTerrain() { return terrain; }
	Vect3d Reflect(Vect3d I, Vect3d N);
	Vect3d TerrainZTangent(TerrainPoint T);
	Vect3d TerrainXTangent(TerrainPoint T);
	Vect3d TerrainNormal(TerrainPoint T);


	std::vector <std::vector<std::vector<Particle>>> fluidParticles;
	double ParticleSize;
	double kernelRadius;
	double viscosIndex;
	double KernelScale;
	double KernelScalePressure;
	double KernelScaleViscous;
	float time;
	std::vector<Particle> Kernel;
	double stiffness;
	double restDensity;
	double bounceDamping;
	std::vector<std::vector<TerrainPoint>> terrain;
};