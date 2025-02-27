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

#ifndef INTERPOLATION_DONNEES_H_INCLUDED
#define INTERPOLATION_DONNEES_H_INCLUDED


#include "sous_modele.hpp"

#include <fstream>
#include <vector>


namespace HYDROTEL
{

	class INTERPOLATION_DONNEES : public SOUS_MODELE
	{
	public:
		INTERPOLATION_DONNEES(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~INTERPOLATION_DONNEES() = 0;

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();


		std::string _nom_fichier_tmin;
		std::string _nom_fichier_tmax;
		std::string _nom_fichier_tmin_jour;
		std::string _nom_fichier_tmax_jour;
		std::string _nom_fichier_pluie;
		std::string _nom_fichier_neige;

		bool		_bSauvegarde;

		float*				_netCdf_tmin;
		float*				_netCdf_tmax;
		float*				_netCdf_tminjour;
		float*				_netCdf_tmaxjour;
		float*				_netCdf_pluie;
		float*				_netCdf_neige;

	private:
		void AppliquerCorrections(DATE_HEURE date);

		std::ofstream _fichier_tmin;
		std::ofstream _fichier_tmax;
		std::ofstream _fichier_pluie;
		std::ofstream _fichier_neige;

		std::ofstream _fichier_tmin_jour;
		std::ofstream _fichier_tmax_jour;

		std::vector<CORRECTION*> _corrections_pluie;
		std::vector<CORRECTION*> _corrections_neige;
		std::vector<CORRECTION*> _corrections_temperature;
	};

}

#endif
