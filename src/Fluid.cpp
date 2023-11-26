#include "Fluid.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <random>
#include <algorithm>
#include <ctime>
#include <omp.h>

#define g 9.8
#define pi 3.14159265359
#define closeness 10000

void Fluid::SetTerrain(std::vector<std::vector<TerrainPoint>> t) {
	terrain = t;
	//std::cout << terrain.size()<<"\n";
}

void Fluid::SetTime(float time) {
	this->time = time;
}

Fluid::Fluid(int particleMatrixSize[]) {
	srand(std::time(NULL));
	time = 0;
	source = Vect3d(-1.5f, -0.5f, 0.0f);
	sizes[0] = particleMatrixSize[0]; sizes[1] = particleMatrixSize[1]; sizes[2] = particleMatrixSize[2];
	for (int i = 0; i < particleMatrixSize[0]; i++) {
		std::vector<std::vector<Particle>> vec2D;
		for (int j = 0; j < particleMatrixSize[1]; j++) {
			std::vector<Particle> vec;
			for (int k = 0; k < particleMatrixSize[2]; k++) {
				ParticleSize = 9.0f / ((float)sizes[0] * (float)sizes[1]);
				kernelRadius = ParticleSize * 4;

				//Setting Kernel values
				float h_9 = pow(kernelRadius, 9);
				float h_6 = pow(kernelRadius, 6);

				KernelScale = 315.0 / (64.0 * pi * h_9);
				KernelScalePressure = 45.0 / (pi * h_6);
				KernelScaleViscous = 45.0 / (pi * h_6);

				float x_off = (3.0 / (float)sizes[0]);
				float y_off = (3.0 / (float)sizes[1]);

				//Particle part(source);
				Vect3d pos = Vect3d(-1.5f, -0.5f, (rand() % 1000) * 1.0f / closeness);
				Particle part(pos);
				/*Vect3d(
				rand() % 100 * 0.001,
				rand() % 100 * 0.001,
				rand() % 100 * 0.001));*/// Vect3d(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 3.0f - 1.5f, 2.0f, static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 3.0f - 1.5f));
				//Vect3d pos = part.GetPos();
				//std::cout << pos.x() << " : " << pos.y() << " : " << pos.z() << "\n";
				part.SetMass(6500);
				restDensity = 1000;
				float vel = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				Vect3d jitter = 0.1f * Vect3d(static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
				part.SetStartingVelocity(-vel * (source)+jitter);
				part.SetVelocity(part.GetStartingVelocity());
				part.SetDensity(-1.0f);
				part.SetPressureForce(-1.0f);
				part.SetPressure(Vect3d(0, 0, 0));
				part.SetViscosity(Vect3d(0, 0, 0));
				stiffness = 0.02;
				viscosIndex = 0.25;
				bounceDamping = 1.0f;

				vec.push_back(part);
			}
			vec2D.push_back(vec);
		}
		fluidParticles.push_back(vec2D);
	}
	
	ParticlePtr* Kernel = new ParticlePtr[sizes[0] * sizes[1] * sizes[2]];
	#pragma omp parallel for collapse(3)
	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			for (int k = 0; k < fluidParticles[i][j].size(); k++) {
				int kernelSize = 0;
				GenerateKernel(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdateDenstiy(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdatePressure(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdateViscosity(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdateExternalForce(&fluidParticles[i][j][k]);
			}
		}
	}
}

void Fluid::GenerateKernel(Particle* part, ParticlePtr Kernel[], int& kernelSize) {
	kernelSize = 0;
	#pragma omp parallel for collapse(3)
	for (int k = 0; k < fluidParticles.size(); k++) {
		for (int l = 0; l < fluidParticles[k].size(); l++) {
			for (int m = 0; m < fluidParticles[k][l].size(); m++) {
				if (&fluidParticles[k][l][m] == part) {
					continue;
				}

				Vect3d t = part->GetPos() - fluidParticles[k][l][m].GetPos();
				float dist = (part->GetPos() - fluidParticles[k][l][m].GetPos()).Length();
				if (dist < kernelRadius) {
					Kernel[kernelSize] = &fluidParticles[k][l][m];
					kernelSize++;
				}
			}
		}
	}
}

void Fluid::UpdateDenstiy(Particle* part, ParticlePtr Kernel[], int kernelSize) {
	float density = part->GetMass() * KernelScale * pow(kernelRadius, 6);

	#pragma omp parallel for
	for (int i = 0; i < kernelSize; i++) {
		float dist = (part->GetPos() - Kernel[i]->GetPos()).Length();
		float kernelVal = (kernelRadius * kernelRadius) - (dist * dist);
		density += Kernel[i]->GetMass() * KernelScale * (kernelVal * kernelVal * kernelVal);
		if (density > 10000000000) {
			std::cout << std::endl;
		}
	}

	part->SetDensity(density);
	//std::cout << density << "\n";

	part->SetPressureForce(std::max(stiffness * (density - restDensity), 0.0));
}

void Fluid::UpdatePressure(Particle* part, ParticlePtr Kernel[], int kernelSize) {
	Vect3d pressure = Vect3d(0, 0, 0);
	float pressureForce = 0;

	#pragma omp parallel for
	for (int i = 0; i < kernelSize; i++) {
		float pf = Kernel[i]->GetPressureForce();
		if (pf <= 0.00001f) {
			continue;
		}
		float t = (kernelRadius - (part->GetPos() - Kernel[i]->GetPos()).Length());
		pressureForce = Kernel[i]->GetMass() *
			((part->GetPressureForce() + Kernel[i]->GetPressureForce()) / (2.0f * pf)) *
			KernelScalePressure * t * t;
		pressure -= pressureForce * (part->GetPos() - Kernel[i]->GetPos()).GetNormalized();
	}

	//pressure = -1 * pressure;

	part->SetPressure(pressure);
}

void Fluid::UpdateViscosity(Particle* part, ParticlePtr Kernel[], int kernelSize) {
	Vect3d viscosity = Vect3d(0, 0, 0);
	float viscosityForce = 0;

	#pragma omp parallel for
	for (int i = 0; i < kernelSize; i++) {
		float d = Kernel[i]->GetDensity();
		if (d <= 0.00001f) {
			continue;
		}
		viscosity = Kernel[i]->GetMass() *
			((Kernel[i]->GetVelocity() - part->GetVelocity()) / d *
				KernelScaleViscous *
				(kernelRadius - (part->GetPos() - Kernel[i]->GetPos()).Length()));
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

Vect3d Fluid::GetFriction(TerrainPoint closestPoint, Particle fluidParticle) {
	return fluidParticle.velocity * 0.001; // TODO: use the correct formula
}

void Fluid::FixOutliers(TerrainPoint &tp, Particle &fp) {
	if (fp.velocity.GetY() > 10.0f) {
		fp.velocity.SetY(0.0f);
	}
	if (tp.pt.GetY() > fp.position.GetY() * 10) {
		fp.position.SetY(tp.pt.GetY());
	}
}

bool Fluid::ShouldDeposit(TerrainPoint &pt) {
	Vect3d avg = Vect3d(0,0,0);
	int c = 0;
	for (int i = 1; i < 5; i++) {
		if (pt.coordi - i >=0) {
			avg += terrain[pt.coordi - i][pt.coordj].pt;
			c++;
		}
		if (pt.coordi + i < terrain.size()) {
			avg += terrain[pt.coordi + i][pt.coordj].pt;
			c++;
		}
		if (pt.coordj - i >= 0) {
			avg += terrain[pt.coordi][pt.coordj - i].pt;
			c++;
		}
		if (pt.coordj + i < terrain[pt.coordi].size()) {
			avg += terrain[pt.coordi][pt.coordj + i].pt;
			c++;
		}
	}
	avg /= c;
	if (pt.pt.GetY() > avg.GetY() + 0.005f * 5) {
		return false;
	}
	return true;
}

void Fluid::AdvectParticles() {
	#pragma omp parallel for collapse(3)
	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			for (int k = 0; k < fluidParticles[i][j].size(); k++) {
				Particle fp = fluidParticles[i][j][k];

				Vect3d accel = fp.GetExternalForce() + fp.GetPressure() / fp.GetDensity() + fp.GetViscosity() / fp.GetDensity();
				//Vect3d accel = Force / fluidParticles[i][j].GetMass();
				Vect3d velocity = (fp.GetVelocity() + accel * time / 2.0f);
				Vect3d position = fp.GetPos() + velocity * time;
				velocity = (fp.GetVelocity() + accel * time / 2.0f);

				if (velocity.GetY() > 10) {
					//std::cout << i << j << k << std::endl;
				}

				float closest = 10000;
				float closestHeight = -15.f;
				TerrainPoint closestPoint;
				int row, col;

				#pragma omp parallel for collapse(2)
				for (int s = 0; s < terrain.size(); s++) {
					for (int t = 0; t < terrain[s].size(); t++) {
						Vect3d terrainPt = Vect3d(terrain[s][t].pt.x(), 0, terrain[s][t].pt.z());
						Vect3d pos = Vect3d(position.x(), 0, position.z());
						float distance = (terrainPt - pos).Dot(terrainPt - pos);
						if (distance < closest && distance < 0.05f) {
							closest = distance;
							closestHeight = terrain[s][t].pt.y();
							closestPoint = terrain[s][t];
							row = s;
							col = t;
						}
					}
				}

				if (position.y() <= closestHeight + 0.05f) {
					position.v[1] = closestHeight + 0.05f;
					//velocity.v[1] = -1.0f * velocity.v[1];
					velocity -= closestPoint.normal * velocity.Dot(closestPoint.normal);
					velocity -= GetFriction(closestPoint, fp);

					int erodeProb = rand() % 10 + 1;

					if (velocity.Length() >= 1.3 && (closestPoint.pt - position).Length() <= 0.1f && erodeProb <= 2) {
						fluidParticles[i][j][k].AddErodedParticle(closestPoint);
						//terrain[row].erase(terrain[row].begin() + col);
						//terrain[row][col].pt.v[1] -= 0.005f;
						terrain[row][col].pt -= 0.005f * terrain[row][col].normal;
						terrain[row][col].isEroded = true;
					}

					int depositProb = rand() % 10 + 1;

					if (velocity.Length() <= 0.5 && velocity.Length() > 0.2 && fluidParticles[i][j][k].deposit.size() > 0 && depositProb < 2 && ShouldDeposit(terrain[row][col])) {
						fluidParticles[i][j][k].GetDepositedParticle();
						//std::cout << fluidParticles[i][j][k].deposit.size() << "\n";
						//terrain[row][col].pt.v[1] += 0.005f;
						terrain[row][col].pt += 0.005f * terrain[row][col].normal;
						terrain[row][col].isDeposited = true;
					}

				}

				//std::cout << "Pos = " << position.x() << " : " << position.y() << " : " << position.z()<<"\n";

				if (position.y() <= -1.5f) {
					Vect3d startV = fluidParticles[i][j][k].GetStartingVelocity();
					//fluidParticles[i][j][k] = init();
					position = Vect3d(-1.5f, -0.5f, (rand() % 1000) * 1.0f / closeness);
					velocity = startV;
					fluidParticles[i][j][k].deposit.clear();
					//position.v[1] = -1.5f;
					//velocity.v[1] = -1.0f * bounceDamping * velocity.v[1];
				}
				//if (position.x() <= -1.5f) {
				//	position = source;
				//	//position.v[0] = -1.5f;
				//	//velocity.v[0] = -1.0f * bounceDamping * velocity.v[0];
				//}
				//else if (position.x() >= 1.5f) {
				//	position = source;
				//	//position.v[0] = 1.5f;
				//	//velocity.v[0] = -1.0f * bounceDamping * velocity.v[0];
				//}
				//if (position.z() <= -1.5f) {
				//	position = source;
				//	//position.v[2] = -1.5f;
				//	//velocity.v[2] = -1.0f * bounceDamping * velocity.v[2];
				//}
				//else if (position.z() >= 1.5f) {
				//	position = source;
				//	//position.v[2] = 1.5f;
				//	//velocity.v[2] = -1.0f * bounceDamping * velocity.v[2];
				//}

				FixOutliers(closestPoint, fluidParticles[i][j][k]);

				fluidParticles[i][j][k].SetVelocity(velocity);

				//std::cout << position.x() << " : " << position.y() << " : " << position.z() << "\n"; 

				fluidParticles[i][j][k].SetPos(position);
			}
		}
	}

	

	ParticlePtr* Kernel = new ParticlePtr[sizes[0] * sizes[1] * sizes[2]];
	#pragma omp parallel for collapse(3)
	for (int i = 0; i < fluidParticles.size(); i++) {
		for (int j = 0; j < fluidParticles[i].size(); j++) {
			for (int k = 0; k < fluidParticles[i][j].size(); k++) {
				int kernelSize = 0;
				GenerateKernel(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdateDenstiy(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdatePressure(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdateViscosity(&fluidParticles[i][j][k], Kernel, kernelSize);
				UpdateExternalForce(&fluidParticles[i][j][k]);
			}
		}
	}
}