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

#ifndef ACHEMINEMENT_RIVIERE_H_INCLUDED
#define ACHEMINEMENT_RIVIERE_H_INCLUDED


#include "sous_modele.hpp"
#include "milieu_humide_riverain.hpp"

#include <fstream>


namespace HYDROTEL
{

	class ACHEMINEMENT_RIVIERE : public SOUS_MODELE
	{
	public:
		ACHEMINEMENT_RIVIERE(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~ACHEMINEMENT_RIVIERE() = 0;

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		std::string _nom_fichier_debit_amont;
		std::string _nom_fichier_debit_aval;

		OUTPUT*				_pOutput;

		float*				_netCdf_debitaval;
		float*				_netCdf_debitamont;

		float*				_netCdf_prelevements_pression;

	private:
		std::ofstream		_fichier_debit_aval;
		std::ofstream		_fichier_debit_amont;

		std::ofstream		_fichier_hauteur_aval;

		std::ofstream		_fichier_prelevements_pression;
	};

}

#endif
