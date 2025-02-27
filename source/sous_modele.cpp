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

#include "sous_modele.hpp"


using namespace std;


namespace HYDROTEL
{

	SOUS_MODELE::SOUS_MODELE(SIM_HYD& sim_hyd, const std::string& nom)
		: _sim_hyd(sim_hyd)
		, _nom(nom)
	{
	}

	SOUS_MODELE::~SOUS_MODELE()
	{
	}

	std::string SOUS_MODELE::PrendreNomSousModele() const
	{
		return _nom;
	}

	void SOUS_MODELE::ChangeNomFichierParametres(string nom_fichier_parametres)
	{
		_nom_fichier_parametres = nom_fichier_parametres;
	}

	string SOUS_MODELE::PrendreNomFichierParametres() const
	{
		return _nom_fichier_parametres;
	}

}
