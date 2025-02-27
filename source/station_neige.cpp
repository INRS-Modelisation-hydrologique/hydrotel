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

#include "station_neige.hpp"

#include "erreur.hpp"


namespace HYDROTEL
{

	STATION_NEIGE::STATION_NEIGE(const std::string& nom_fichier)
		: STATION(nom_fichier)
	{
	}

	STATION_NEIGE::~STATION_NEIGE()
	{
	}

	bool STATION_NEIGE::DonneeDisponible(const DATE_HEURE& date)
	{
		auto pos = _donnees.find(date);
		return pos != _donnees.end() ? true : false;
	}

	STATION_NEIGE::DONNEE_NEIGE STATION_NEIGE::PrendreDonnee(const DATE_HEURE& date)
	{
		DONNEE_NEIGE donnees;

		if (_donnees.find(date) != _donnees.end())
			donnees = _donnees[date];
		else
			throw ERREUR("STATION_NEIGE::PrendreDonnee(): invalid date");

		return donnees;
	}

}
