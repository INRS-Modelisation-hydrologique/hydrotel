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

#ifndef FONTE_NEIGE_H_INCLUDED
#define FONTE_NEIGE_H_INCLUDED


#include "sous_modele.hpp"

#include <fstream>


namespace HYDROTEL
{

	class FONTE_NEIGE : public SOUS_MODELE
	{
	public:
		FONTE_NEIGE(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~FONTE_NEIGE() = 0;

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();


		std::string			_nom_fichier_apport;

		std::string			_nom_fichier_couvert_nival;
		std::string			_nom_fichier_hauteur_neige;
		std::string			_nom_fichier_albedo_neige;

		float*				_netCdf_apport;

	private:
		std::ofstream _fichier_apport;
	};

}

#endif
