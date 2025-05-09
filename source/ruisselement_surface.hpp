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

#ifndef RUISSELEMENT_SURFACE_H_INCLUDED
#define RUISSELEMENT_SURFACE_H_INCLUDED


#include "sous_modele.hpp"

#include <fstream>


namespace HYDROTEL
{

	class RUISSELEMENT_SURFACE : public SOUS_MODELE
	{
	public:

		RUISSELEMENT_SURFACE(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~RUISSELEMENT_SURFACE() = 0;

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();


		std::string			_nom_fichier_apport_lateral;

		float*				_netCdf_apport_lateral;
		float*				_netCdf_apport_lateral_uhrh;

		float*				_netCdf_ecoulement_surf;
		float*				_netCdf_ecoulement_hypo;
		float*				_netCdf_ecoulement_base;

	private:

		std::ofstream _fichier_apport_lateral;
		std::ofstream _fichier_apport_lateral_uhrh;

		std::ofstream _fichier_ecoulement_surf;
		std::ofstream _fichier_ecoulement_hypo;
		std::ofstream _fichier_ecoulement_base;
	};

}

#endif

