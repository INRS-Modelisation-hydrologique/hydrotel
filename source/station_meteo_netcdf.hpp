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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef STATION_METEO_NETCDF_INCLUDED
#define STATION_METEO_NETCDF_INCLUDED

#include "stations_meteo.hpp"
#include "station_meteo.hpp"


#include <netcdf.h>


namespace HYDROTEL
{

	class STATION_METEO_NETCDF : public STATION_METEO
	{
	public:
		//STATION_METEO_NETCDF(int ncid, int type, const std::string& nom_fichier, size_t lig, size_t col);
		//STATION_METEO_NETCDF(int ncid, int type, const std::string& nom_fichier, int indexStation, const std::string& sTimeField, const std::string& sTMinField, const std::string& sTMaxField, const std::string& sPrecipField);
		//STATION_METEO_NETCDF(int ncid, int type, const std::string& nom_fichier, int indexStation, std::string sTimeField, std::string sTMinField, std::string sTMaxField, std::string sPrecipField);
		//STATION_METEO_NETCDF(int ncid, int type, const std::string& nom_fichier, int indexStation, std::string sTimeField, std::string sTMinField, std::string sTMaxField, std::string sPrecipField);

		STATION_METEO_NETCDF(const std::string& nom_fichier, STATIONS_METEO* pStations);

		virtual ~STATION_METEO_NETCDF();

		virtual void LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);

		virtual DONNEE_METEO PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual void ChangeDonnees(const DONNEE_METEO& donnee_meteo, const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual std::pair<float, float> PrendreTemperatureJournaliere(const DATE_HEURE& date_heure);

	private:

		STATIONS_METEO* _pStations;

		int _iType;	//0=STATION, 1=GRID, 2=GRID_EXTENT

		int _ncid;

		int _iIndexStation;

		size_t _lig;
		size_t _col;

		DATE_HEURE _date_debut;
		unsigned short _pas_de_temps; // NOTE: correspond au pas de temps du fichier
		int _nb_donnees;

		int _tminid;
		int _tmaxid;
		int _prid;
	};


} // HYDROTEL

#endif
