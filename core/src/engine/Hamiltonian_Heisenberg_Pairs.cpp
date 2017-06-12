#ifndef USE_CUDA

#define _USE_MATH_DEFINES
#include <cmath>

#include <Eigen/Dense>

#include <Spirit_Defines.h>
#include <engine/Hamiltonian_Heisenberg_Pairs.hpp>
#include <engine/Vectormath.hpp>
#include <engine/Neighbours.hpp>
#include <data/Spin_System.hpp>
#include <utility/Constants.hpp>

using std::vector;
using std::function;

using namespace Data;
using namespace Utility;

namespace Engine
{
	Hamiltonian_Heisenberg_Pairs::Hamiltonian_Heisenberg_Pairs(
		scalarfield mu_s,
		intfield external_field_indices, scalarfield external_field_magnitudes, vectorfield external_field_normals,
		intfield anisotropy_indices, scalarfield anisotropy_magnitudes, vectorfield anisotropy_normals,
		pairfield exchange_pairs, scalarfield exchange_magnitudes,
		pairfield dmi_pairs, scalarfield dmi_magnitudes, vectorfield dmi_normals,
		scalar ddi_radius,
		quadrupletfield quadruplets, scalarfield quadruplet_magnitudes,
		std::shared_ptr<Data::Geometry> geometry,
		intfield boundary_conditions
	) :
		Hamiltonian(boundary_conditions), geometry(geometry),
		mu_s(mu_s),
		external_field_indices(external_field_indices), external_field_magnitudes(external_field_magnitudes), external_field_normals(external_field_normals),
		anisotropy_indices(anisotropy_indices), anisotropy_magnitudes(anisotropy_magnitudes), anisotropy_normals(anisotropy_normals),
		exchange_pairs(exchange_pairs), exchange_magnitudes(exchange_magnitudes),
		dmi_pairs(dmi_pairs), dmi_magnitudes(dmi_magnitudes), dmi_normals(dmi_normals),
		quadruplets(quadruplets), quadruplet_magnitudes(quadruplet_magnitudes)
	{
		// Atom types
		this->atom_types = intfield(geometry->nos, 0);

		// Renormalize the external field from Tesla to whatever
		for (unsigned int i = 0; i < external_field_magnitudes.size(); ++i)
		{
			this->external_field_magnitudes[i] = this->external_field_magnitudes[i] * Constants::mu_B * mu_s[i];
		}

		// Generate DDI pairs, magnitudes, normals
		this->ddi_pairs = Engine::Neighbours::Get_Pairs_in_Radius(*this->geometry, ddi_radius);
		scalar magnitude;
		Vector3 normal;
		for (unsigned int i = 0; i<ddi_pairs.size(); ++i)
		{
			Engine::Neighbours::DDI_from_Pair(*this->geometry, { ddi_pairs[i].i, ddi_pairs[i].j, ddi_pairs[i].translations }, magnitude, normal);
			this->ddi_magnitudes.push_back(magnitude);
			this->ddi_normals.push_back(normal);
		}

		this->Update_Energy_Contributions();
	}

	void Hamiltonian_Heisenberg_Pairs::Update_Energy_Contributions()
	{
		this->energy_contributions_per_spin = std::vector<std::pair<std::string, scalarfield>>(0);

		// External field
		if (this->external_field_indices.size() > 0)
		{
			this->energy_contributions_per_spin.push_back({"Zeeman", scalarfield(0)});
			this->idx_zeeman = this->energy_contributions_per_spin.size()-1;
		}
		else this->idx_zeeman = -1;
		// Anisotropy
		if (this->anisotropy_indices.size() > 0)
		{
			this->energy_contributions_per_spin.push_back({"Anisotropy", scalarfield(0) });
			this->idx_anisotropy = this->energy_contributions_per_spin.size()-1;
		}
		else this->idx_anisotropy = -1;
		// Exchange
		if (this->exchange_pairs.size() > 0)
		{
			this->energy_contributions_per_spin.push_back({"Exchange", scalarfield(0) });
			this->idx_exchange = this->energy_contributions_per_spin.size()-1;
		}
		else this->idx_exchange = -1;
		// DMI
		if (this->dmi_pairs.size() > 0)
		{
			this->energy_contributions_per_spin.push_back({"DMI", scalarfield(0) });
			this->idx_dmi = this->energy_contributions_per_spin.size()-1;
		}
		else this->idx_dmi = -1;
		// Dipole-Dipole
		if (this->ddi_pairs.size() > 0)
		{
			this->energy_contributions_per_spin.push_back({"DD", scalarfield(0) });
			this->idx_ddi = this->energy_contributions_per_spin.size()-1;
		}
		else this->idx_ddi = -1;
		// Quadruplets
		if (this->quadruplets.size() > 0)
		{
			this->energy_contributions_per_spin.push_back({"Quadruplets", scalarfield(0) });
			this->idx_quadruplet = this->energy_contributions_per_spin.size()-1;
		}
		else this->idx_quadruplet = -1;
	}

	void Hamiltonian_Heisenberg_Pairs::Energy_Contributions_per_Spin(const vectorfield & spins, std::vector<std::pair<std::string, scalarfield>> & contributions)
	{
		if (contributions.size() != this->energy_contributions_per_spin.size())
		{
			contributions = this->energy_contributions_per_spin;
		}
		
		int nos = spins.size();
		for (auto& contrib : contributions)
		{
			// Allocate if not already allocated
			if (contrib.second.size() != nos) contrib.second = scalarfield(nos, 0);
			// Otherwise set to zero
			else Vectormath::fill(contrib.second, 0);
		}

		// External field
		if (this->idx_zeeman >=0 )     E_Zeeman(spins, contributions[idx_zeeman].second);

		// Anisotropy
		if (this->idx_anisotropy >=0 ) E_Anisotropy(spins, contributions[idx_anisotropy].second);

		// Exchange
		if (this->idx_exchange >=0 )   E_Exchange(spins, contributions[idx_exchange].second);
		// DMI
		if (this->idx_dmi >=0 )        E_DMI(spins,contributions[idx_dmi].second);
		// DD
		if (this->idx_ddi >=0 )        E_DDI(spins, contributions[idx_ddi].second);
		// Quadruplets
		if (this->idx_quadruplet >=0 ) E_Quadruplet(spins, contributions[idx_quadruplet].second);
	}

	void Hamiltonian_Heisenberg_Pairs::E_Zeeman(const vectorfield & spins, scalarfield & Energy)
	{
		for (unsigned int i = 0; i < this->external_field_indices.size(); ++i)
		{
			int ispin = external_field_indices[i];
			#ifdef SPIRIT_ENABLE_DEFECTS
			if (this->atom_types[ispin] >= 0)
			#endif
			Energy[ispin] -= this->external_field_magnitudes[i] * this->external_field_normals[i].dot(spins[ispin]);
		}
	}

	void Hamiltonian_Heisenberg_Pairs::E_Anisotropy(const vectorfield & spins, scalarfield & Energy)
	{
		for (unsigned int i = 0; i < this->anisotropy_indices.size(); ++i)
		{
			int ispin = anisotropy_indices[i];
			#ifdef SPIRIT_ENABLE_DEFECTS
			if (this->atom_types[ispin] >= 0)
			#endif
			Energy[ispin] -= this->anisotropy_magnitudes[i] * std::pow(anisotropy_normals[i].dot(spins[ispin]), 2.0);
		}
	}

	void Hamiltonian_Heisenberg_Pairs::E_Exchange(const vectorfield & spins, scalarfield & Energy)
	{
		for (unsigned int i_pair = 0; i_pair < exchange_pairs.size(); ++i_pair)
		{
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						if (Vectormath::boundary_conditions_fulfilled(geometry->n_cells, boundary_conditions, translations, exchange_pairs[i_pair].translations))
						{
							int ispin = exchange_pairs[i_pair].i+ Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
							int jspin = exchange_pairs[i_pair].j+ Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, exchange_pairs[i_pair].translations);

							#ifdef SPIRIT_ENABLE_DEFECTS
							if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0)
							{
							#endif
								Energy[ispin] -= 0.5 * exchange_magnitudes[i_pair] * spins[ispin].dot(spins[jspin]);
								Energy[jspin] -= 0.5 * exchange_magnitudes[i_pair] * spins[ispin].dot(spins[jspin]);
							#ifdef SPIRIT_ENABLE_DEFECTS
							}
							#endif
						}
					}
				}
			}
		}
	}

	void Hamiltonian_Heisenberg_Pairs::E_DMI(const vectorfield & spins, scalarfield & Energy)
	{
		for (unsigned int i_pair = 0; i_pair < dmi_pairs.size(); ++i_pair)
		{
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						if (Vectormath::boundary_conditions_fulfilled(geometry->n_cells, boundary_conditions, translations, dmi_pairs[i_pair].translations))
						{
							int ispin = dmi_pairs[i_pair].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
							int jspin = dmi_pairs[i_pair].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, dmi_pairs[i_pair].translations);
							
							#ifdef SPIRIT_ENABLE_DEFECTS
							if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0)
							{
							#endif
							Energy[ispin] -= 0.5 * dmi_magnitudes[i_pair] * dmi_normals[i_pair].dot(spins[ispin].cross(spins[jspin]));
							Energy[jspin] -= 0.5 * dmi_magnitudes[i_pair] * dmi_normals[i_pair].dot(spins[ispin].cross(spins[jspin]));
							#ifdef SPIRIT_ENABLE_DEFECTS
							}
							#endif
						}
					}
				}
			}
		}
	}

	void Hamiltonian_Heisenberg_Pairs::E_DDI(const vectorfield & spins, scalarfield & Energy)
	{
		//scalar mult = -Constants::mu_B*Constants::mu_B*1.0 / 4.0 / M_PI; // multiply with mu_B^2
		scalar mult = 0.5*0.0536814951168; // mu_0*mu_B**2/(4pi*10**-30) -- the translations are in angstr�m, so the |r|[m] becomes |r|[m]*10^-10
		scalar result = 0.0;

		for (unsigned int i_pair = 0; i_pair < ddi_pairs.size(); ++i_pair)
		{
			if (ddi_magnitudes[i_pair] > 0.0)
			{
				for (int da = 0; da < geometry->n_cells[0]; ++da)
				{
					for (int db = 0; db < geometry->n_cells[1]; ++db)
					{
						for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
						{
							std::array<int, 3 > translations = { da, db, dc };
							int ispin = ddi_pairs[i_pair].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
							int jspin = ddi_pairs[i_pair].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, ddi_pairs[i_pair].translations);
							
							#ifdef SPIRIT_ENABLE_DEFECTS
							if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0)
							{
							#endif
							Energy[ispin] -= mult / std::pow(ddi_magnitudes[i_pair], 3.0) *
								(3 * spins[ispin].dot(ddi_normals[i_pair]) * spins[ispin].dot(ddi_normals[i_pair]) - spins[ispin].dot(spins[ispin]));
							Energy[ispin] -= mult / std::pow(ddi_magnitudes[i_pair], 3.0) *
								(3 * spins[ispin].dot(ddi_normals[i_pair]) * spins[ispin].dot(ddi_normals[i_pair]) - spins[ispin].dot(spins[ispin]));
							#ifdef SPIRIT_ENABLE_DEFECTS
							}
							#endif
						}
					}
				}
			}
		}
	}// end DipoleDipole


	void Hamiltonian_Heisenberg_Pairs::E_Quadruplet(const vectorfield & spins, scalarfield & Energy)
	{
		for (unsigned int iquad = 0; iquad < quadruplets.size(); ++iquad)
		{
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						int ispin = quadruplets[iquad].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
						int jspin = quadruplets[iquad].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, quadruplets[iquad].d_j);
						int kspin = quadruplets[iquad].k + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, quadruplets[iquad].d_k);
						int lspin = quadruplets[iquad].l + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, quadruplets[iquad].d_l);
						
						#ifdef SPIRIT_ENABLE_DEFECTS
						if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0 && this->atom_types[kspin] >= 0 && this->atom_types[lspin] >= 0)
						{
						#endif
						Energy[ispin] -= 0.25*quadruplet_magnitudes[iquad] * (spins[ispin].dot(spins[jspin])) * (spins[kspin].dot(spins[lspin]));
						Energy[jspin] -= 0.25*quadruplet_magnitudes[iquad] * (spins[ispin].dot(spins[jspin])) * (spins[kspin].dot(spins[lspin]));
						Energy[kspin] -= 0.25*quadruplet_magnitudes[iquad] * (spins[ispin].dot(spins[jspin])) * (spins[kspin].dot(spins[lspin]));
						Energy[lspin] -= 0.25*quadruplet_magnitudes[iquad] * (spins[ispin].dot(spins[jspin])) * (spins[kspin].dot(spins[lspin]));
						#ifdef SPIRIT_ENABLE_DEFECTS
						}
						#endif
					}
				}
			}
		}
	}



	void Hamiltonian_Heisenberg_Pairs::Gradient(const vectorfield & spins, vectorfield & gradient)
	{
		// Set to zero
		Vectormath::fill(gradient, {0,0,0});

		// External field
		Gradient_Zeeman(gradient);

		// Anisotropy
		Gradient_Anisotropy(spins, gradient);

		// Exchange
		this->Gradient_Exchange(spins, gradient);
		// DMI
		this->Gradient_DMI(spins, gradient);
		// DD
		this->Gradient_DDI(spins, gradient);

		// Quadruplets
		this->Gradient_Quadruplet(spins, gradient);
	}

	void Hamiltonian_Heisenberg_Pairs::Gradient_Zeeman(vectorfield & gradient)
	{
		for (unsigned int i = 0; i < this->external_field_indices.size(); ++i)
		{
			int ispin = external_field_indices[i];
			#ifdef SPIRIT_ENABLE_DEFECTS
			if (this->atom_types[ispin] >= 0)
			#endif
			gradient[ispin] -= this->external_field_magnitudes[i] * this->external_field_normals[i];
		}
	}

	void Hamiltonian_Heisenberg_Pairs::Gradient_Anisotropy(const vectorfield & spins, vectorfield & gradient)
	{
		for (unsigned int i = 0; i < this->anisotropy_indices.size(); ++i)
		{
			int ispin = anisotropy_indices[i];
			#ifdef SPIRIT_ENABLE_DEFECTS
			if (this->atom_types[ispin] >= 0)
			#endif
			gradient[ispin] -= 2.0 * this->anisotropy_magnitudes[i] * this->anisotropy_normals[i] * anisotropy_normals[i].dot(spins[ispin]);
		}
	}

	void Hamiltonian_Heisenberg_Pairs::Gradient_Exchange(const vectorfield & spins, vectorfield & gradient)
	{
		for (unsigned int i_pair = 0; i_pair < exchange_pairs.size(); ++i_pair)
		{
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						if (Vectormath::boundary_conditions_fulfilled(geometry->n_cells, boundary_conditions, translations, exchange_pairs[i_pair].translations))
						{
							int ispin = exchange_pairs[i_pair].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
							int jspin = exchange_pairs[i_pair].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, exchange_pairs[i_pair].translations);
							
							#ifdef SPIRIT_ENABLE_DEFECTS
							if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0)
							{
							#endif
							gradient[ispin] -= exchange_magnitudes[i_pair] * spins[jspin];
							gradient[jspin] -= exchange_magnitudes[i_pair] * spins[ispin];
							#ifdef SPIRIT_ENABLE_DEFECTS
							}
							#endif
						}
					}
				}
			}
		}
	}

	void Hamiltonian_Heisenberg_Pairs::Gradient_DMI(const vectorfield & spins, vectorfield & gradient)
	{
		for (unsigned int i_pair = 0; i_pair < dmi_pairs.size(); ++i_pair)
		{
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						if (Vectormath::boundary_conditions_fulfilled(geometry->n_cells, boundary_conditions, translations, dmi_pairs[i_pair].translations))
						{
							int ispin = dmi_pairs[i_pair].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
							int jspin = dmi_pairs[i_pair].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, dmi_pairs[i_pair].translations);
							
							#ifdef SPIRIT_ENABLE_DEFECTS
							if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0)
							{
							#endif
							gradient[ispin] -= dmi_magnitudes[i_pair] * spins[jspin].cross(dmi_normals[i_pair]);
							gradient[jspin] += dmi_magnitudes[i_pair] * spins[ispin].cross(dmi_normals[i_pair]);
							#ifdef SPIRIT_ENABLE_DEFECTS
							}
							#endif
						}
					}
				}
			}
		}
	}

	void Hamiltonian_Heisenberg_Pairs::Gradient_DDI(const vectorfield & spins, vectorfield & gradient)
	{
		//scalar mult = Constants::mu_B*Constants::mu_B*1.0 / 4.0 / M_PI; // multiply with mu_B^2
		scalar mult = 0.0536814951168; // mu_0*mu_B**2/(4pi*10**-30) -- the translations are in angstr�m, so the |r|[m] becomes |r|[m]*10^-10
		
		for (unsigned int i_pair = 0; i_pair < ddi_pairs.size(); ++i_pair)
		{
			if (ddi_magnitudes[i_pair] > 0.0)
			{
				for (int da = 0; da < geometry->n_cells[0]; ++da)
				{
					for (int db = 0; db < geometry->n_cells[1]; ++db)
					{
						for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
						{
							scalar skalar_contrib = mult / std::pow(ddi_magnitudes[i_pair], 3.0);
							std::array<int, 3 > translations = { da, db, dc };
							if (Vectormath::boundary_conditions_fulfilled(geometry->n_cells, boundary_conditions, translations, ddi_pairs[i_pair].translations))
							{
								int ispin = ddi_pairs[i_pair].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
								int jspin = ddi_pairs[i_pair].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, ddi_pairs[i_pair].translations);
								
								#ifdef SPIRIT_ENABLE_DEFECTS
								if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0)
								{
								#endif
								gradient[ispin] -= skalar_contrib * (3 * ddi_normals[i_pair] * spins[jspin].dot(ddi_normals[i_pair]) - spins[jspin]);
								gradient[jspin] -= skalar_contrib * (3 * ddi_normals[i_pair] * spins[ispin].dot(ddi_normals[i_pair]) - spins[ispin]);
								#ifdef SPIRIT_ENABLE_DEFECTS
								}
								#endif
							}
						}
					}
				}
			}
		}
	}//end Field_DipoleDipole


	void Hamiltonian_Heisenberg_Pairs::Gradient_Quadruplet(const vectorfield & spins, vectorfield & gradient)
	{
		for (unsigned int iquad = 0; iquad < quadruplets.size(); ++iquad)
		{
			int i = quadruplets[iquad].i;
			int j = quadruplets[iquad].j;
			int k = quadruplets[iquad].k;
			int l = quadruplets[iquad].l;
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						int ispin = i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations);
						int jspin = j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, quadruplets[iquad].d_j);
						int kspin = k + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, quadruplets[iquad].d_k);
						int lspin = l + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, quadruplets[iquad].d_l);
						
						#ifdef SPIRIT_ENABLE_DEFECTS
						if (this->atom_types[ispin] >= 0 && this->atom_types[jspin] >= 0 && this->atom_types[kspin] >= 0 && this->atom_types[lspin] >= 0)
						{
						#endif
						gradient[ispin] -= quadruplet_magnitudes[iquad] * spins[jspin] * (spins[kspin].dot(spins[lspin]));
						gradient[jspin] -= quadruplet_magnitudes[iquad] * spins[ispin] * (spins[kspin].dot(spins[lspin]));
						gradient[kspin] -= quadruplet_magnitudes[iquad] * (spins[ispin].dot(spins[jspin])) * spins[lspin];
						gradient[lspin] -= quadruplet_magnitudes[iquad] * (spins[ispin].dot(spins[jspin])) * spins[kspin];
						#ifdef SPIRIT_ENABLE_DEFECTS
						}
						#endif
					}
				}
			}
		}
	}


	void Hamiltonian_Heisenberg_Pairs::Hessian(const vectorfield & spins, MatrixX & hessian)
	{
		int nos = spins.size();

		// Set to zero
		// for (auto& h : hessian) h = 0;
		hessian.setZero();

		// Single Spin elements
		for (int alpha = 0; alpha < 3; ++alpha)
		{
			for (unsigned int i = 0; i < anisotropy_indices.size(); ++i)
			{
				int idx = anisotropy_indices[i];
				// scalar x = -2.0*this->anisotropy_magnitudes[i] * std::pow(this->anisotropy_normals[i][alpha], 2);
				hessian(3*idx + alpha, 3*idx + alpha) += -2.0*this->anisotropy_magnitudes[i]*std::pow(this->anisotropy_normals[i][alpha],2);
			}
		}

		// std::cerr << "calculated hessian" << std::endl;

		// Spin Pair elements
		// Exchange
		for (unsigned int i_pair = 0; i_pair < this->exchange_pairs.size(); ++i_pair)
		{
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						for (int alpha = 0; alpha < 3; ++alpha)
						{
							int ispin = 3 * (exchange_pairs[i_pair].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations)) + alpha;
							int jspin = 3 * (exchange_pairs[i_pair].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, exchange_pairs[i_pair].translations)) + alpha;
							hessian(ispin, jspin) += -exchange_magnitudes[i_pair];
							hessian(jspin, ispin) += -exchange_magnitudes[i_pair];
						}
					}
				}
			}
		}

		// DMI
		for (unsigned int i_pair = 0; i_pair < this->dmi_pairs.size(); ++i_pair)
		{
			for (int da = 0; da < geometry->n_cells[0]; ++da)
			{
				for (int db = 0; db < geometry->n_cells[1]; ++db)
				{
					for (int dc = 0; dc < geometry->n_cells[2]; ++dc)
					{
						std::array<int, 3 > translations = { da, db, dc };
						for (int alpha = 0; alpha < 3; ++alpha)
						{
							for (int beta = 0; beta < 3; ++beta)
							{
								int ispin = 3 * (dmi_pairs[i_pair].i + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations)) + alpha;
								int jspin = 3 * (dmi_pairs[i_pair].j + Vectormath::idx_from_translations(geometry->n_cells, geometry->n_spins_basic_domain, translations, dmi_pairs[i_pair].translations)) + alpha;
								if ((alpha == 0 && beta == 1))
								{
									hessian(ispin, jspin) +=
										-dmi_magnitudes[i_pair] * dmi_normals[i_pair][2];
									hessian(jspin, ispin) +=
										-dmi_magnitudes[i_pair] * dmi_normals[i_pair][2];
								}
								else if ((alpha == 1 && beta == 0))
								{
									hessian(ispin, jspin) +=
										dmi_magnitudes[i_pair] * dmi_normals[i_pair][2];
									hessian(jspin, ispin) +=
										dmi_magnitudes[i_pair] * dmi_normals[i_pair][2];
								}
								else if ((alpha == 0 && beta == 2))
								{
									hessian(ispin, jspin) +=
										dmi_magnitudes[i_pair] * dmi_normals[i_pair][1];
									hessian(jspin, ispin) +=
										dmi_magnitudes[i_pair] * dmi_normals[i_pair][1];
								}
								else if ((alpha == 2 && beta == 0))
								{
									hessian(ispin, jspin) +=
										-dmi_magnitudes[i_pair] * dmi_normals[i_pair][1];
									hessian(jspin, ispin) +=
										-dmi_magnitudes[i_pair] * dmi_normals[i_pair][1];
								}
								else if ((alpha == 1 && beta == 2))
								{
									hessian(ispin, jspin) +=
										-dmi_magnitudes[i_pair] * dmi_normals[i_pair][0];
									hessian(jspin, ispin) +=
										-dmi_magnitudes[i_pair] * dmi_normals[i_pair][0];
								}
								else if ((alpha == 2 && beta == 1))
								{
									hessian(ispin, jspin) +=
										dmi_magnitudes[i_pair] * dmi_normals[i_pair][0];
									hessian(jspin, ispin) +=
										dmi_magnitudes[i_pair] * dmi_normals[i_pair][0];
								}
							}
						}
					}
				}
			}
		}

		//// Dipole-Dipole
		//for (unsigned int i_pair = 0; i_pair < this->DD_indices.size(); ++i_pair)
		//{
		//	// indices
		//	int idx_1 = DD_indices[i_pair][0];
		//	int idx_2 = DD_indices[i_pair][1];
		//	// prefactor
		//	scalar prefactor = 0.0536814951168
		//		* this->mu_s[idx_1] * this->mu_s[idx_2]
		//		/ std::pow(DD_magnitude[i_pair], 3);
		//	// components
		//	for (int alpha = 0; alpha < 3; ++alpha)
		//	{
		//		for (int beta = 0; beta < 3; ++beta)
		//		{
		//			int idx_h = idx_1 + alpha*nos + 3 * nos*(idx_2 + beta*nos);
		//			if (alpha == beta)
		//				hessian[idx_h] += prefactor;
		//			hessian[idx_h] += -3.0*prefactor*DD_normal[i_pair][alpha] * DD_normal[i_pair][beta];
		//		}
		//	}
		//}

		// Quadruplets
	}

	// Hamiltonian name as string
	static const std::string name = "Heisenberg (Pairs)";
	const std::string& Hamiltonian_Heisenberg_Pairs::Name() { return name; }
}

#endif