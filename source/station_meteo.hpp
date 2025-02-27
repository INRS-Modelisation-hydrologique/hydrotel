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

#ifndef STATION_METEO_H_INCLUDED
#define STATION_METEO_H_INCLUDED


#include "date_heure.hpp"
#include "donnee_meteo.hpp"
#include "station.hpp"

#include <vector>


namespace HYDROTEL
{

	class STATION_METEO : public STATION
	{
	public:
		STATION_METEO(const std::string& nom_fichier);
		virtual ~STATION_METEO();

		// lecture des donnees meteo
		virtual void LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps) = 0;

		// retourne les donnees meteo
		virtual DONNEE_METEO PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps) = 0;

		// change les donnees meteo
		virtual void ChangeDonnees(const DONNEE_METEO& donnees, const DATE_HEURE& date_heure, unsigned short pas_de_temps) = 0;

		// retourne la temperature journaliere
		virtual std::pair<float, float> PrendreTemperatureJournaliere(const DATE_HEURE& date_heure) = 0;
	};

}

#endif
