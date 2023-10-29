#include "Fluid.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <random>
#include <algorithm>

#define g 9.8
#define pi 3.14159265359

void Fluid::SetTerrain(std::vector<Vect3d> t) {
	terrain = t;
	//std::cout << terrain.size()<<"\n";
}

void Fluid::SetTime(float time) {
	this->time = time;
}

Fluid::Fluid(int particleMatrixSize[]) {
	time = 0;
	for (int i = 0; i < particleMatrixSize[0]; i++) {
		std::vector<Particle> vec;
		for (int j = 0; j < particleMatrixSize[1]; j++) {
			ParticleSize = 9.0f / ((float)particleMatrixSize[0] * (float)particleMatrixSize[1]);
			kernelRadius = ParticleSize*4;

			//Setting Kernel values
			float h_9 = pow(kernelRadius, 9);
			float h_6 = pow(kernelRadius, 6);

			KernelScale = 315.0 / (64.0 * pi * h_9);
			KernelScalePressure = 45.0 / (pi * h_6);
			KernelScaleViscous = 45.0 / (pi * h_6);

			float x_off = (3.0 / (float)particleMatrixSize[0]);
			float y_off = (3.0 / (float)particleMatrixSize[1]);
			Particle part(Vect3d(0,0,0));// Vect3d(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 3.0f - 1.5f, 2.0f, static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 3.0f - 1.5f));
			//Vect3d pos = part.GetPos();
			//std::cout << pos.x() << " : " << pos.y() << " : " << pos.z() << "\n";
			part.SetMass(6500);
			restDensity = 1000;
			part.SetVelocity(Vect3d(static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
			part.SetPressure(Vect3d(0, 0, 0));
			part.SetViscosity(Vect3d(0, 0, 0));
			stiffness = 0.02;
			viscosIndex = 0.25;
			bounceDamping = 0.6f;
			vec.push_back(part);
		}
		fluidParticles.push_back(vec);
	}

	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			GenerateKernel(&fluidParticles[i][j]);
			UpdateDenstiy(&fluidParticles[i][j]);
			UpdatePressure(&fluidParticles[i][j]);
			UpdateViscosity(&fluidParticles[i][j]);
			UpdateExternalForce(&fluidParticles[i][j]);
		}
	}
}

void Fluid::GenerateKernel(Particle* part) {
	Kernel.clear();
	for (int k = 0; k < fluidParticles.size(); k++) {
		for (int l = 0; l < fluidParticles[k].size(); l++) {
			if (&fluidParticles[k][l] == part) {
				continue;
			}

			float dist = (part->GetPos() - fluidParticles[k][l].GetPos()).Length();
			if (dist < kernelRadius) {
				Kernel.push_back(fluidParticles[k][l]);
			}
		}
	}
}

void Fluid::UpdateDenstiy(Particle* part) {
	float density = part->GetMass() * KernelScale * pow(kernelRadius, 6);

	for (int i = 0; i < Kernel.size(); i++) {
		float dist = (part->GetPos() - Kernel[i].GetPos()).Length();
		float kernelVal = (kernelRadius * kernelRadius) - (dist * dist);
		density += Kernel[i].GetMass() * KernelScale * (kernelVal * kernelVal * kernelVal);
	}

	part->SetDensity(density);
	//std::cout << density << "\n";

	part->SetPressureForce(std::max(stiffness * (density - restDensity), 0.0));
}

void Fluid::UpdatePressure(Particle* part) {
	Vect3d pressure = Vect3d(0, 0, 0);
	float pressureForce = 0;

	for (int i = 0; i < Kernel.size(); i++) {
		pressureForce = Kernel[i].GetMass() * 
			((part->GetPressureForce() + Kernel[i].GetPressureForce()) / (2.0f * Kernel[i].GetPressureForce())) * 
			KernelScalePressure * pow((kernelRadius - (part->GetPos() - Kernel[i].GetPos()).Length()), 2);
		pressure -= pressureForce * (part->GetPos() - Kernel[i].GetPos()).GetNormalized();
	}

	//pressure = -1 * pressure;

	part->SetPressure(pressure);
}

void Fluid::UpdateViscosity(Particle* part) {
	Vect3d viscosity = Vect3d(0, 0, 0);
	float viscosityForce = 0;

	for (int i = 0; i < Kernel.size(); i++) {
		viscosity = Kernel[i].GetMass() * 
			((Kernel[i].GetVelocity() - part->GetVelocity()) / Kernel[i].GetDensity() * 
				KernelScaleViscous * 
				(kernelRadius - (part->GetPos() - Kernel[i].GetPos()).Length()));
	}

	viscosity = viscosIndex * viscosity;

	part->SetViscosity(viscosity);
}

void Fluid::UpdateExternalForce(Particle* part) {
	Vect3d force = Vect3d(0, -g, 0);

	part->SetExternalForce(force);
}

Vect3d Fluid::Reflect(Vect3d I, Vect3d N) {
	Vect3d v_parallel = N * (I.x() * N.x() + I.y() * N.y() + I.z() * N.z());
	Vect3d v_perp = I - v_parallel;

	return v_parallel - v_perp;
}

void Fluid::AdvectParticles() {
	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			Vect3d accel = fluidParticles[i][j].GetExternalForce() + fluidParticles[i][j].GetPressure()/fluidParticles[i][j].GetDensity() + fluidParticles[i][j].GetViscosity()/fluidParticles[i][j].GetDensity();
			//Vect3d accel = Force / fluidParticles[i][j].GetMass();
			Vect3d velocity = (fluidParticles[i][j].GetVelocity() + accel * time/2.0f);
			Vect3d position = fluidParticles[i][j].GetPos() + velocity * time;
			velocity = (fluidParticles[i][j].GetVelocity() + accel * time / 2.0f);

			float closest = 10000;
			float closestHeight = 0;
			for (int k = 0; k < terrain.size(); k++) {
				if ((terrain[k] - position).Length() < closest) {
					closest = (terrain[k] - position).Length();
					closestHeight = terrain[k].y();
				}
			}

			if (position.y() <= closestHeight) {
				position.v[1] = closestHeight;
				velocity.v[1] = -1.0f * bounceDamping * velocity.v[1];
			}

			if (position.y() <= -1.5f) {
				position.v[1] = -1.5f;
				velocity.v[1] = -1.0f * bounceDamping * velocity.v[1];
			}
			if (position.x() <= -1.5f) {
				position.v[0] = -1.5f;
				velocity.v[0] = -1.0f * bounceDamping * velocity.v[0];
			}
			else if (position.x() >= 1.5f) {
				position.v[0] = 1.5f;
				velocity.v[0] = -1.0f * bounceDamping * velocity.v[0];
			}
			if (position.z() <= -1.5f) {
				position.v[2] = -1.5f;
				velocity.v[2] = -1.0f * bounceDamping * velocity.v[2];
			}
			else if (position.z() >= 1.5f) {
				position.v[2] = 1.5f;
				velocity.v[2] = -1.0f * bounceDamping * velocity.v[2];
			}

			fluidParticles[i][j].SetVelocity(velocity);

			//std::cout << position.x() << " : " << position.y() << " : " << position.z() << "\n"; 

			fluidParticles[i][j].SetPos(position);
		}
	}

	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			GenerateKernel(&fluidParticles[i][j]);
			UpdateDenstiy(&fluidParticles[i][j]);
			UpdatePressure(&fluidParticles[i][j]);
			UpdateViscosity(&fluidParticles[i][j]);
			UpdateExternalForce(&fluidParticles[i][j]);
		}
	}
}