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

#ifndef STATION_METEO_NETCDF_STATION_INCLUDED
#define STATION_METEO_NETCDF_STATION_INCLUDED


#include "stations_meteo.hpp"
#include "station_meteo.hpp"

#include <netcdf.h>


namespace HYDROTEL
{

	class STATION_METEO_NETCDF_STATION : public STATION_METEO
	{
	public:
		STATION_METEO_NETCDF_STATION(const std::string& nom_fichier, STATIONS_METEO* pStations, size_t lIndexStation);
		STATION_METEO_NETCDF_STATION(const std::string& nom_fichier, STATIONS_METEO* pStations, size_t lIndexLat, size_t lIndexLon);

		virtual ~STATION_METEO_NETCDF_STATION();

		virtual void LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);

		virtual DONNEE_METEO PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual void ChangeDonnees(const DONNEE_METEO& donnee_meteo, const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual std::pair<float, float> PrendreTemperatureJournaliere(const DATE_HEURE& date_heure);

	private:
		STATIONS_METEO* _pStations;

		size_t			_lIndexStation;		//pour  type STATION (0)
		
		size_t			_lIndexLat;			//pour  type GRID (1)
		size_t			_lIndexLon;			//
	};

}

#endif
