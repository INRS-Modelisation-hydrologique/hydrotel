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

#ifndef BILAN_VERTICAL_H_INCLUDED
#define BILAN_VERTICAL_H_INCLUDED


#include "sous_modele.hpp"

#include <fstream>


namespace HYDROTEL
{

	class BILAN_VERTICAL : public SOUS_MODELE
	{
	public:
		BILAN_VERTICAL(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~BILAN_VERTICAL() = 0;

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		
		std::string _nom_fichier_production_base;
		std::string _nom_fichier_production_hypo;
		std::string _nom_fichier_production_surf;

		float*				_netCdf_prodSurf;
		float*				_netCdf_prodHypo;
		float*				_netCdf_prodBase;

	private:
		std::ofstream _fichier_production_surf;
		std::ofstream _fichier_production_hypo;
		std::ofstream _fichier_production_base;
	};

}

#endif
