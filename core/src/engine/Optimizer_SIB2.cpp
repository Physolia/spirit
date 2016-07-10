#include "Optimizer_SIB2.h"

#include "Vectormath.h"

using namespace Utility;

namespace Engine
{
	void Optimizer_SIB2::Configure(std::vector<std::shared_ptr<Data::Spin_System>> systems, std::shared_ptr<Engine::Force> force_call)
	{
		Optimizer::Configure(systems, force_call);
		//this->virtualforce = std::vector<std::vector<double>>(this->noi, std::vector<double>(3 * this->nos));	// [noi][3*nos]
		this->spins_temp = std::vector<std::vector<double>>(this->noi, std::vector<double>(3 * this->nos));	// [noi][3*nos]
		//if (systems.size() > 1) Log.Send(Log_Level::L_ERROR, Log_Sender::LLG, "THE OPTIMIZER_SIB2 CANNOT HANDLE MORE THAN 1 IMAGE CORRECTLY !!");
	}

	void Optimizer_SIB2::Step()
	{
		std::shared_ptr<Data::Spin_System> s;
		// This is probably quite inefficient?? CHECK IF THATS THE CASE!
		for (int i = 0; i < this->noi; ++i)
		{
			this->configurations[i] = systems[i]->spins;
		}

		// Random Numbers
		for (int img = 0; img < this->noi; ++img)
		{
			s = systems[img];

			double h = s->llg_parameters->dt;
			double rh = std::sqrt(h);
			double alpha = s->llg_parameters->damping;
			double temperature = s->llg_parameters->temperature;

			double nx, ny, nz;
			double Hx, Hy, Hz;
			double Cx, Cy, Cz;
			double Rx, Ry, Rz;
			double ax, ay, az;
			double Ax, Ay, Az;
			double detMi;
			double D = std::sqrt(2.0*alpha / (1.0 + alpha * alpha) * temperature);

			auto R = std::vector<double>(3 * nos);
			this->Gen_Xi(*s, R, D);

			Cx = s->llg_parameters->stt_polarisation_normal[0] * s->llg_parameters->stt_magnitude;
			Cy = s->llg_parameters->stt_polarisation_normal[1] * s->llg_parameters->stt_magnitude;
			Cz = s->llg_parameters->stt_polarisation_normal[2] * s->llg_parameters->stt_magnitude;

			this->force_call->Calculate(configurations, force);

			for (int i = 0; i < s->nos; ++i)
			{
				nx = s->spins[i]; ny = s->spins[i + s->nos]; nz = s->spins[i + 2 * s->nos];
				Hx = force[img][i]; Hy = force[img][i + s->nos]; Hz = force[img][i + 2*s->nos];
				Rx = R[i]; Ry = R[i + s->nos]; Rz = R[i + 2 * s->nos];

				Ax = 0.5*h * (-Hx - alpha * (ny*Hz - nz*Hy));
				Ay = 0.5*h * (-Hy - alpha * (nz*Hx - nx*Hz));
				Az = 0.5*h * (-Hz - alpha * (nx*Hy - ny*Hx));

				Ax = Ax + 0.5*h * (-alpha*Cx + (ny*Cz - nz*Cy));
				Ay = Ay + 0.5*h * (-alpha*Cy + (nz*Cx - nx*Cz));
				Az = Az + 0.5*h * (-alpha*Cz + (nx*Cy - ny*Cx));

				Ax = Ax + 0.5*rh * D * (-Rx - alpha*(ny*Rz - nz*Ry));
				Ay = Ay + 0.5*rh * D * (-Ry - alpha*(nz*Rx - nx*Rz));
				Az = Az + 0.5*rh * D * (-Rz - alpha*(nx*Ry - ny*Rx));

				ax = nx + ny*Az - nz*Ay;
				ay = ny + nz*Ax - nx*Az;
				az = nz + nx*Ay - ny*Ax;

				Hx = Ax*Ax;
				Hy = Ay*Ay;
				Hz = Az*Az;
				Rx = Ay*Az;
				Ry = Ax*Az;
				Rz = Ax*Ay;

				detMi = 1.0 / (1.0 + Hx + Hy + Hz);

				nx = nx + (ax*(1.0 + Hx) + ay*(Rz + Az) + az*(Ry - Ay)) * detMi;
				ny = ny + (ax*(Rz - Az) + ay*(1.0 + Hy) + az*(Rx + Ax)) * detMi;
				nz = nz + (ax*(Ry + Ay) + ay*(Rx - Ax) + az*(1.0 + Hz)) * detMi;

				spins_temp[img][i] = 0.5*nx;
				spins_temp[img][i + s->nos] = 0.5*ny;
				spins_temp[img][i + 2*s->nos] = 0.5*nz;
			}

			this->force_call->Calculate(configurations, force);

			for (int i = 0; i < s->nos; ++i)
			{
				nx = spins_temp[img][i]; ny = spins_temp[img][i + s->nos]; nz = spins_temp[img][i + 2 * s->nos];
				Hx = force[img][i]; Hy = force[img][i + s->nos]; Hz = force[img][i + 2 * s->nos];
				Rx = R[i]; Ry = R[i + s->nos]; Rz = R[i + 2 * s->nos];

				Ax = 0.5*h * (-Hx - alpha * (ny*Hz - nz*Hy));
				Ay = 0.5*h * (-Hy - alpha * (nz*Hx - nx*Hz));
				Az = 0.5*h * (-Hz - alpha * (nx*Hy - ny*Hx));

				Ax = Ax + 0.5*h * (-alpha*Cx + (ny*Cz - nz*Cy));
				Ay = Ay + 0.5*h * (-alpha*Cy + (nz*Cx - nx*Cz));
				Az = Az + 0.5*h * (-alpha*Cz + (nx*Cy - ny*Cx));

				Ax = Ax + 0.5*rh * D * (-Rx - alpha*(ny*Rz - nz*Ry));
				Ay = Ay + 0.5*rh * D * (-Ry - alpha*(nz*Rx - nx*Rz));
				Az = Az + 0.5*rh * D * (-Rz - alpha*(nx*Ry - ny*Rx));

				nx = s->spins[i]; ny = s->spins[i + s->nos]; nz = s->spins[i + 2 * s->nos];

				ax = nx + ny*Az - nz*Ay;
				ay = ny + nz*Ax - nx*Az;
				az = nz + nx*Ay - ny*Ax;

				Hx = Ax*Ax;
				Hy = Ay*Ay;
				Hz = Az*Az;
				Rx = Ay*Az;
				Ry = Ax*Az;
				Rz = Ax*Ay;

				detMi = 1.0 / (1.0 + Hx + Hy + Hz);

				s->spins[i] = (ax*(1.0 + Hx) + ay*(Rz + Az) + az*(Ry - Ay)) * detMi;
				s->spins[i + s->nos] = (ax*(Rz - Az) + ay*(1.0 + Hy) + az*(Rx + Ax)) * detMi;
				s->spins[i + 2 * s->nos] = (ax*(Ry + Ay) + ay*(Rx - Ax) + az*(1.0 + Hz)) * detMi;

			}
		}
	}

	void Optimizer_SIB2::Gen_Xi(Data::Spin_System & s, std::vector<double> & xi, double eps)
	{
		//for (int i = 0; i < 3*s.nos; ++i) {
		//	// PRNG gives RN int [0,1] -> [-1,1] -> multiply with eps
		//	xi[i] = (s.llg_parameters->distribution_int(s.llg_parameters->prng) * 2 - 1)*eps;
		//}//endfor i
		for (int dim = 0; dim < 3; ++dim) {
			for (int i = 0; i < s.nos; ++i) {
				// PRNG gives RN int [0,1] -> [-1,1] -> multiply with eps
				xi[dim*s.nos + i] = (s.llg_parameters->distribution_int(s.llg_parameters->prng) * 2 - 1)*eps;
			}//endfor i
		}//enfor dim

	}//end Gen_Xi
}