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

#include "station_meteo_netcdf.hpp"
#include "constantes.hpp"
#include "erreur.hpp"


using namespace std;



namespace HYDROTEL
{
	//STATION_METEO_NETCDF::STATION_METEO_NETCDF(int ncid, int type, const string& nom_fichier, size_t lig, size_t col)
	//	: STATION_METEO(nom_fichier)
	//	, _ncid(ncid)
	//	, _iType(type)
	//	, _lig(lig)
	//	, _col(col)
	//	, _iIndexStation(-1)
	//{
	//	// lecture du nombre de pas

	//	int status, timeid;

	//	status = nc_inq_dimid(_ncid, "time", &timeid);
	//	if (status != NC_NOERR)
	//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);

	//	size_t timelength;

	//	status = nc_inq_dimlen(_ncid, timeid, &timelength);
	//	if (status != NC_NOERR)
	//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);

	//	// determiner le pas de temps

	//	status = nc_inq_varid(_ncid, "time", &timeid);
	//	if (status != NC_NOERR)
	//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);

	//	vector<double> times(timelength);
	//	status = nc_get_var_double(_ncid, timeid, &times[0]);
	//	if (status != NC_NOERR)
	//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);

	//	_date_debut = DATE_HEURE(1900, 1, 1, 0);
	//	_date_debut.AdditionHeure( static_cast<int>(24 * times[0]) );

	//	_pas_de_temps = static_cast<unsigned short>( (times[1] - times[0]) * 24 );

	//	// mise en cache des id tmin, tmax et pr

	//	status = nc_inq_varid(_ncid, "tmin", &_tminid);
	//	if (status != NC_NOERR)
	//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);

	//	status = nc_inq_varid(_ncid, "tmax", &_tmaxid);
	//	if (status != NC_NOERR)
	//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);

	//	status = nc_inq_varid(_ncid, "pr", &_prid);
	//	if (status != NC_NOERR)
	//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);
	//}

	//STATION_METEO_NETCDF::STATION_METEO_NETCDF(int ncid, int type, const string& nom_fichier, int indexStation, const string& sTimeField, const string& sTMinField, const string& sTMaxField, const string& sPrecipField)
	//STATION_METEO_NETCDF::STATION_METEO_NETCDF(int ncid, int type, const string& nom_fichier, int indexStation, string sTimeField, string sTMinField, string sTMaxField, string sPrecipField)
	//STATION_METEO_NETCDF::STATION_METEO_NETCDF(int ncid, int type, const string& nom_fichier, int indexStation, string sTimeField, string sTMinField, string sTMaxField, string sPrecipField)

	//, _ncid(ncid)
	//	, _iType(type)
	//	, _lig(0)
	//	, _col(0)
	//	, _iIndexStation(indexStation)

	STATION_METEO_NETCDF::STATION_METEO_NETCDF(const string& nom_fichier, STATIONS_METEO* pStations) : STATION_METEO(nom_fichier)
	{
		// lecture du nombre de pas

		//int status, timeid;

		//status = nc_inq_dimid(_ncid, sTimeField.c_str(), &timeid);
		//if (status != NC_NOERR)
		//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		//size_t timelength;

		//status = nc_inq_dimlen(_ncid, timeid, &timelength);
		//if (status != NC_NOERR)
		//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		//// determiner le pas de temps

		//status = nc_inq_varid(_ncid, sTimeField.c_str(), &timeid);
		//if (status != NC_NOERR)
		//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		//vector<double> times(timelength);
		//status = nc_get_var_double(_ncid, timeid, &times[0]);
		//if (status != NC_NOERR)
		//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		//_date_debut = DATE_HEURE(1900, 1, 1, 0);
		//_date_debut.AdditionHeure(static_cast<int>(24 * times[0]));

		//_pas_de_temps = static_cast<unsigned short>((times[1] - times[0]) * 24);

		//// mise en cache des id tmin, tmax et pr

		//status = nc_inq_varid(_ncid, sTMinField.c_str(), &_tminid);
		//if (status != NC_NOERR)
		//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		//status = nc_inq_varid(_ncid, sTMaxField.c_str(), &_tmaxid);
		//if (status != NC_NOERR)
		//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		//status = nc_inq_varid(_ncid, sPrecipField.c_str(), &_prid);
		//if (status != NC_NOERR)
		//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		_pStations = pStations;
	}

	STATION_METEO_NETCDF::~STATION_METEO_NETCDF()
	{
	}

	void STATION_METEO_NETCDF::LectureDonnees(const DATE_HEURE& /*debut*/, const DATE_HEURE& /*fin*/, unsigned short pas_de_temps)
	{
		if (pas_de_temps % _pas_de_temps)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		// NOTE: la fonction ne fait rien, la lecture se fait a chaque pas de temps
	}

	void STATION_METEO_NETCDF::ChangeDonnees(const DONNEE_METEO& /*donnee_meteo*/, const DATE_HEURE& /*date_heure*/, unsigned short /*pas_de_temps*/)
	{
		// NOTE: le fichier est ouvert en lecture seulement
	}

	DONNEE_METEO STATION_METEO_NETCDF::PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps)
	{
		size_t time = _date_debut.NbHeureEntre(date_heure) / _pas_de_temps;

		int nb_pas = pas_de_temps / _pas_de_temps;

		double tmin = VALEUR_MANQUANTE;
		double tmax = VALEUR_MANQUANTE;
		double prec = VALEUR_MANQUANTE;

		for (int n = 0; n < nb_pas; ++n)
		{
			size_t start[] = { time + n, _lig, _col };
			size_t count[] = { 1, 1, 1 };

			int status;		
			double minl, maxl, pr;

			status = nc_get_vara_double(_ncid, _tminid, start, count, &minl);
			if (status != NC_NOERR)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier);			

			status = nc_get_vara_double(_ncid, _tmaxid, start, count, &maxl);
			if (status != NC_NOERR)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier);			

			status = nc_get_vara_double(_ncid, _prid, start, count, &pr);
			if (status != NC_NOERR)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier);			

			if (n == 0)
			{
				tmin = minl;
				tmax = maxl;
				prec = pr;
			}
			else
			{
				tmin = min(tmin, minl);
				tmax = max(tmax, maxl);
				prec += pr;
			}
		}

		DONNEE_METEO donnee_meteo;

		donnee_meteo.ChangeTemperature( static_cast<float>(tmin), static_cast<float>(tmax) );
		donnee_meteo.ChangePluie( static_cast<float>(prec) );
		donnee_meteo.ChangeNeige(0);

		return donnee_meteo;
	}

	pair<float, float> STATION_METEO_NETCDF::PrendreTemperatureJournaliere(const DATE_HEURE& date_heure)
	{
		DATE_HEURE date(date_heure.PrendreAnnee(), date_heure.PrendreMois(), date_heure.PrendreJour(), 0);

		int nb_pas = 24 / _pas_de_temps;

		auto donnees = PrendreDonnees(date, 24);

		float tmin = donnees.PrendreTMin();
		float tmax = donnees.PrendreTMax();

		for (int n = 1; n < nb_pas; ++n)
		{
			date.AdditionHeure(_pas_de_temps);
			donnees = PrendreDonnees(date, _pas_de_temps);

			if(tmin <= VALEUR_MANQUANTE)
				tmin = donnees.PrendreTMin();
			else
			{
				if(donnees.PrendreTMin() > VALEUR_MANQUANTE)
					tmin = min(tmin, donnees.PrendreTMin());
			}

			if(tmax <= VALEUR_MANQUANTE)
				tmax = donnees.PrendreTMax();
			else
			{
				if(donnees.PrendreTMax() > VALEUR_MANQUANTE)
					tmax = max(tmax, donnees.PrendreTMax());
			}
		}

		if(tmin <= VALEUR_MANQUANTE || tmax <= VALEUR_MANQUANTE)
			tmin = tmax = VALEUR_MANQUANTE;

		return make_pair(tmin, tmax);
	}

} // HYDROTEL
