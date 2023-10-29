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

Fluid::Fluid(int particleMatrixSize[], bool randomize, float startPositionOffset[], float scatter, int mass, Vect3d force) {
	time = 0;
	for (int i = 0; i < particleMatrixSize[0]; i++) {
		std::vector < std::vector<Particle>> vec2d;
		for (int j = 0; j < particleMatrixSize[1]; j++) {
			std::vector<Particle> vec;
			for (int k = 0; k < particleMatrixSize[2]; k++) {
				ParticleSize = 9.0f / ((float)particleMatrixSize[0] * (float)particleMatrixSize[1]);
				kernelRadius = ParticleSize * 4;

				//Setting Kernel values
				float h_9 = pow(kernelRadius, 9);
				float h_6 = pow(kernelRadius, 6);

				KernelScale = 315.0 / (64.0 * pi * h_9);
				KernelScalePressure = 45.0 / (pi * h_6);
				KernelScaleViscous = 45.0 / (pi * h_6);

				float x_off = (3.0 / (float)particleMatrixSize[0]);
				float y_off = (3.0 / (float)particleMatrixSize[1]);
				Particle part;
				if (randomize) {
					part = Particle(Vect3d(
						rand() % 100 * 0.01 * scatter + startPositionOffset[0],
						rand() % 100 * 0.01 * scatter + startPositionOffset[1],
						rand() % 100 * 0.01 * scatter + startPositionOffset[2]));
				}
				else {
					part = Particle(Vect3d(i * scatter + startPositionOffset[0], j * scatter + startPositionOffset[1], k * scatter + startPositionOffset[2]));// Vect3d(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 3.0f - 1.5f, 2.0f, static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 3.0f - 1.5f));
				}
				//Vect3d pos = part.GetPos();
				//std::cout << pos.x() << " : " << pos.y() << " : " << pos.z() << "\n";
				part.SetMass(mass);
				restDensity = 1000;
				part.SetVelocity(Vect3d(static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
				//part.SetVelocity(Vect3d(0,0,0));
				part.SetPressure(Vect3d(0, 0, 0));
				part.SetViscosity(Vect3d(0, 0, 0));
				stiffness = 0.02;
				viscosIndex = 0.25;
				bounceDamping = 0.6f;
				vec.push_back(part);
			}
			vec2d.push_back(vec);
		}
		fluidParticles.push_back(vec2d);
	}

	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			for (int k = 0; k < fluidParticles[i][j].size(); k++) {
				GenerateKernel(&fluidParticles[i][j][k]);
				UpdateDenstiy(&fluidParticles[i][j][k]);
				UpdatePressure(&fluidParticles[i][j][k]);
				UpdateViscosity(&fluidParticles[i][j][k]);
				UpdateExternalForce(&fluidParticles[i][j][k], force);
			}
		}
	}
}

void Fluid::GenerateKernel(Particle* part) {
	Kernel.clear();
	for (int k = 0; k < fluidParticles.size(); k++) {
		for (int l = 0; l < fluidParticles[k].size(); l++) {
			for (int m = 0; m < fluidParticles[k][l].size(); m++) {
				if (&fluidParticles[k][l][m] == part) {
					continue;
				}

				float dist = (part->GetPos() - fluidParticles[k][l][m].GetPos()).Length();
				if (dist < kernelRadius) {
					Kernel.push_back(fluidParticles[k][l][m]);
				}
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

void Fluid::UpdateExternalForce(Particle* part, Vect3d force) {

	part->SetExternalForce(force);
}

Vect3d Fluid::Reflect(Vect3d I, Vect3d N) {
	Vect3d v_parallel = N * (I.x() * N.x() + I.y() * N.y() + I.z() * N.z());
	Vect3d v_perp = I - v_parallel;

	return v_parallel - v_perp;
}

void Fluid::AdvectParticles(Vect3d force) {
	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			for (int k = 0; k < fluidParticles[i][j].size(); k++) {
				Vect3d accel = fluidParticles[i][j][k].GetExternalForce() + fluidParticles[i][j][k].GetPressure() / fluidParticles[i][j][k].GetDensity() + fluidParticles[i][j][k].GetViscosity() / fluidParticles[i][j][k].GetDensity();
				//Vect3d accel = Force / fluidParticles[i][j][k].GetMass();
				//Vect3d accel = fluidParticles[i][j][k].GetExternalForce();
				Vect3d velocity = (fluidParticles[i][j][k].GetVelocity() + accel * time / 2.0f);
				Vect3d position = fluidParticles[i][j][k].GetPos() + velocity * time;
				velocity = (fluidParticles[i][j][k].GetVelocity() + accel * time / 2.0f);

				float closest = 10000;
				float closestHeight = 0;
				for (int t = 0; t < terrain.size(); t++) {
					if ((terrain[t] - position).Length() < closest) {
						closest = (terrain[t] - position).Length();
						closestHeight = terrain[t].y();
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

				fluidParticles[i][j][k].SetVelocity(velocity);

				//std::cout << position.x() << " : " << position.y() << " : " << position.z() << "\n"; 

				fluidParticles[i][j][k].SetPos(position);
			}
		}
	}

	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			for (int k = 0; k < fluidParticles[i][j].size(); k++) {
				GenerateKernel(&fluidParticles[i][j][k]);
				UpdateDenstiy(&fluidParticles[i][j][k]);
				UpdatePressure(&fluidParticles[i][j][k]);
				UpdateViscosity(&fluidParticles[i][j][k]);
				UpdateExternalForce(&fluidParticles[i][j][k], force);
			}
		}
	}
}