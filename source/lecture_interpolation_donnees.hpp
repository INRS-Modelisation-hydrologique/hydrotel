//
// HYDROTEL a spatially distributed hydrological model
// Copyright (C) 2013 INRS Eau Terre Environnement
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
// USA
//

#ifndef LECTURE_INTERPOLATION_DONNEES_H_INCLUDED
#define LECTURE_INTERPOLATION_DONNEES_H_INCLUDED


#include "interpolation_donnees.hpp"


namespace HYDROTEL
{

	class LECTURE_INTERPOLATION_DONNEES : public INTERPOLATION_DONNEES
	{
	public:
		LECTURE_INTERPOLATION_DONNEES(SIM_HYD& sim_hyd);
		virtual ~LECTURE_INTERPOLATION_DONNEES();

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		virtual void ChangeNbParams(const ZONES& zones);

		virtual void LectureParametres();

		virtual void SauvegardeParametres();

	private:
		std::ifstream _fichier_tmin;
		std::ifstream _fichier_tmax;
		std::ifstream _fichier_tmin_jour;
		std::ifstream _fichier_tmax_jour;
		std::ifstream _fichier_pluie;
		std::ifstream _fichier_neige;

		std::vector<int>	_vIDFichier;
	};

}

#endif
