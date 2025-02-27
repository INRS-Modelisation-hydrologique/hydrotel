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

#ifndef STATISTIQUES_H_INCLUDED
#define STATISTIQUES_H_INCLUDED


#include "sim_hyd.hpp"

#include <map>


namespace HYDROTEL
{

	class STATISTIQUES
	{
	public:
		/// construit un objet statistiques et calcul les statistiques
		STATISTIQUES(SIM_HYD& sim_hyd, const std::string& nom_fichier_parametres);

		~STATISTIQUES();

	private:
		void LectureParametres();
		void LectureDonnees();
				
		SIM_HYD* _sim_hyd;
		std::string _nom_fichier_parametres;
		std::map<int, std::string> _troncon_station;
	};

}

#endif
