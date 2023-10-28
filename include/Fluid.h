#pragma once

#include "Particle.h"
#include <vector>

class Fluid {
public:
	Fluid(int particleMatrixSize[]);
	std::vector<std::vector<Particle>> GetParticles() { return fluidParticles; }
	void GenerateKernel();
	void UpdateDenstiy(Particle* part);
	void UpdatePressure(Particle* part);
	void UpdateViscosity(Particle* part);
	void UpdateExternalForce(Particle* part);
	void AdvectParticles();
	void SetTIme(float time);

private:
	std::vector<std::vector<Particle>> fluidParticles;
	float ParticleSize;
	float kernelRadius;
	float viscosIndex;
	float KernelScale;
	float KernelScalePressure;
	float KernelScaleViscous;
	float time;
};