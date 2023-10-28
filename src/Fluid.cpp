#include "Fluid.h"
#include <iostream>
#include <vector>
#include <math.h>

#define g -9.8
#define pi 3.14159265359

void Fluid::SetTIme(float time) {
	this->time = time;
}

Fluid::Fluid(int particleMatrixSize[]) {
	for (int i = 0; i < particleMatrixSize[0]; i++) {
		std::vector<Particle> vec;
		for (int j = 0; j < particleMatrixSize[1]; j++) {
			ParticleSize = 1;
			kernelRadius = ParticleSize * 2;

			//Setting Kernel values
			float h_9 = pow(kernelRadius, 9);
			float h_6 = pow(kernelRadius, 6);

			KernelScale = 315 / (64 * pi * h_9);
			KernelScalePressure = 45 / (pi * h_6);
			KernelScaleViscous = 45 / (pi * h_6);

			Particle part(Vect3d(i, j, -50));
			part.SetMass(5);
			part.SetVelocity(Vect3d(0, 0, 0));
			part.SetPressure(Vect3d(1, 1, 1));
			part.SetViscosity(Vect3d(1, 1, 1));
			viscosIndex = 0.01;
			vec.push_back(part);
		}
		fluidParticles.push_back(vec);
	}

	GenerateKernel();

	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			UpdateDenstiy(&fluidParticles[i][j]);
			UpdatePressure(&fluidParticles[i][j]);
			UpdateViscosity(&fluidParticles[i][j]);
			UpdateExternalForce(&fluidParticles[i][j]);
		}
	}
}

void Fluid::GenerateKernel() {
	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			Particle* part = &fluidParticles[i][j];
			std::vector<Particle> Kernel;
			for (int k = 0; k < fluidParticles.size(); k++) {
				for (int l = 0; l < fluidParticles[i].size(); l++) {
					if (&fluidParticles[k][l] == part) {
						continue;
					}

					float dist = (part->GetPos() - fluidParticles[k][l].GetPos()).Length();
					if (dist < kernelRadius) {
						Kernel.push_back(fluidParticles[k][l]);
					}
				}
			}
			part->SetKernel(Kernel);
		}
	}
}

void Fluid::UpdateDenstiy(Particle* part) {
	float density = 0;
	std::vector<Particle> Kernel = part->GetKernel();

	for (int i = 0; i < Kernel.size(); i++) {
		float dist = (part->GetPos() - Kernel[i].GetPos()).Length();
		float kernelVal = (kernelRadius * kernelRadius) - (dist * dist);
		density += Kernel[i].GetMass() * KernelScale * (kernelVal * kernelVal * kernelVal);
	}

	part->SetDensity(density);
}

void Fluid::UpdatePressure(Particle* part) {
	Vect3d pressure = Vect3d(0, 0, 0);
	float pressureForce = 0;

	std::vector<Particle> Kernel = part->GetKernel();

	for (int i = 0; i < Kernel.size(); i++) {
		pressureForce = Kernel[i].GetMass() * ((part->GetPressure().Length() + Kernel[i].GetPressure().Length()) / (2 * Kernel[i].GetPressure().Length()) * KernelScalePressure * pow((kernelRadius - (part->GetPos() - Kernel[i].GetPos()).Length()), 2));
		pressure += pressureForce * (part->GetPos() - Kernel[i].GetPos()).GetNormalized();
	}

	pressure = -1 * pressure;

	part->SetPressure(pressure);
}

void Fluid::UpdateViscosity(Particle* part) {
	Vect3d viscosity = Vect3d(0, 0, 0);
	float viscosityForce = 0;

	std::vector<Particle> Kernel = part->GetKernel();

	for (int i = 0; i < Kernel.size(); i++) {
		viscosityForce = Kernel[i].GetMass() * ((Kernel[i].GetViscosity().Length() - part->GetViscosity().Length()) / Kernel[i].GetDensity() * KernelScaleViscous * (kernelRadius - (part->GetPos() - Kernel[i].GetPos()).Length()));
		viscosity = viscosityForce * (part->GetPos() - Kernel[i].GetPos()).GetNormalized();
	}

	viscosity = viscosIndex * viscosity;

	part->SetViscosity(viscosity);
}

void Fluid::UpdateExternalForce(Particle* part) {
	Vect3d force = Vect3d(0, -g * part->GetMass(), 0);

	part->SetExternalForce(force);
}

void Fluid::AdvectParticles() {
	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			Vect3d Force = fluidParticles[i][j].GetPressure() + fluidParticles[i][j].GetViscosity() + fluidParticles[i][j].GetExternalForce();
			std::cout << Force.x() << " : " << Force.y() << " : " << Force.z() << "\n";
			Vect3d accel = Force / fluidParticles[i][j].GetMass();
			Vect3d velocity = fluidParticles[i][j].GetVelocity() + accel * time;
			fluidParticles[i][j].SetVelocity(velocity);
			Vect3d position = fluidParticles[i][j].GetPos() + velocity * time;
			if (position.y() < 20) {
				fluidParticles[i][j].SetPos(position);
			}
		}
	}

	GenerateKernel();

	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			UpdateDenstiy(&fluidParticles[i][j]);
			UpdatePressure(&fluidParticles[i][j]);
			UpdateViscosity(&fluidParticles[i][j]);
			UpdateExternalForce(&fluidParticles[i][j]);
		}
	}
}