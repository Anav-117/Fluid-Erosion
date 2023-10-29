#pragma once

#include "Particle.h"
#include <vector>

class Fluid {
public:
	Fluid(int particleMatrixSize[], bool randomize, float startPositionOffset[], float scatter, int mass, Vect3d force);
	std::vector<std::vector<std::vector<Particle>>> GetParticles() { return fluidParticles; }
	void GenerateKernel(Particle* part);
	void UpdateDenstiy(Particle* part);
	void UpdatePressure(Particle* part);
	void UpdateViscosity(Particle* part);
	void UpdateExternalForce(Particle* part, Vect3d force);
	void AdvectParticles(Vect3d force);
	void SetTime(float time);
	void SetTerrain(std::vector<Vect3d> t);
	Vect3d Reflect(Vect3d I, Vect3d N);

private:
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
	std::vector<Vect3d> terrain;
};