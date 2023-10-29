#pragma once

#include "math/vect3d.h" 
#include <vector>

class Particle {
	Vect3d position;
	Vect3d velocity;
	float mass;
	float density;
	float pressureForce;
	Vect3d pressure;
	float viscosityForce;
	Vect3d viscosity;
	Vect3d externalForce;
	Vect3d color;
	std::vector<Particle> Kernel;

public:
	Particle() {
		position = Vect3d(0, 0, 0);
	}

	Particle(Vect3d pos){
		position = pos;
	}

	Vect3d GetPos() { return position; }

	void SetPos(Vect3d pos) { position = pos; }

	Vect3d GetVelocity() { return velocity; }

	void SetVelocity(Vect3d vel) { velocity = vel; }

	Vect3d GetColor() { return color; }

	void SetColor(Vect3d col) { color = col; }

	float GetMass() { return mass; }

	void SetMass(float m) { mass = m; }

	float GetDensity() { return density; }

	void SetDensity(float d) {  density = d; }

	float GetPressureForce() { return pressureForce; }

	void SetPressureForce(float p) { pressureForce = p; }

	float GetViscosityForce() { return viscosityForce; }

	void SetViscosityForce(float v) { viscosityForce = v; }

	Vect3d GetPressure() { return pressure; }

	void SetPressure(Vect3d p) { pressure = p; }

	Vect3d GetViscosity() { return viscosity; }

	void SetViscosity(Vect3d v) { viscosity = v; }

	Vect3d GetExternalForce() { return externalForce; }

	void SetExternalForce(Vect3d e) { externalForce = e; }

	std::vector<Particle> GetKernel() { return Kernel; }

	void SetKernel(std::vector<Particle> k) { Kernel = k; }

	void ClearKernel() { Kernel.clear(); }
};

