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

#include "station_meteo_netcdf_station.hpp"

#include "sim_hyd.hpp"
#include "constantes.hpp"
#include "erreur.hpp"


using namespace std;


namespace HYDROTEL
{

	STATION_METEO_NETCDF_STATION::STATION_METEO_NETCDF_STATION(const string& nom_fichier, STATIONS_METEO* pStations, size_t lIndexStation) 
		: STATION_METEO(nom_fichier)
	{
		_pStations = pStations;

		_lIndexStation = lIndexStation;

		_lIndexLat = 0;
		_lIndexLon = 0;
	}


	STATION_METEO_NETCDF_STATION::STATION_METEO_NETCDF_STATION(const string& nom_fichier, STATIONS_METEO* pStations, size_t lIndexLat, size_t lIndexLon) 
		: STATION_METEO(nom_fichier)
	{
		_pStations = pStations;
		
		_lIndexLat = lIndexLat;
		_lIndexLon = lIndexLon;

		_lIndexStation = 0;
	}


	STATION_METEO_NETCDF_STATION::~STATION_METEO_NETCDF_STATION()
	{
	}


	void STATION_METEO_NETCDF_STATION::LectureDonnees(const DATE_HEURE&, const DATE_HEURE&, unsigned short pas_de_temps)
	{
		if (pas_de_temps % _pStations->_netCdf_lPasTemps)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		// NOTE: la fonction ne fait rien, la lecture se fait a chaque pas de temps
	}


	void STATION_METEO_NETCDF_STATION::ChangeDonnees(const DONNEE_METEO& /*donnee_meteo*/, const DATE_HEURE& /*date_heure*/, unsigned short /*pas_de_temps*/)
	{
		// NOTE: le fichier est ouvert en lecture seulement
	}


	DONNEE_METEO STATION_METEO_NETCDF_STATION::PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps)
	{
		//ASSERT(pas_de_temps == _pStations->_netCdf_lPasTemps)

		ostringstream oss;
		size_t idxTime, idx;
		float fVal;

		idx = 0;
		idxTime = _pStations->_netCdf_dateDebutVecteur.NbHeureEntre(date_heure) / pas_de_temps;
		
		switch(_pStations->_netCdf_iType)
		{
		case 0:
			idx = idxTime * _pStations->_netCdf_lNbStations + _lIndexStation;
			break;

		case 1:
			idx = (idxTime * _pStations->_netCdf_lNbCoord) + (_lIndexLat * _pStations->_netCdf_lNbLong + _lIndexLon);
		}
		

		//int nb_pas = pas_de_temps / _pas_de_temps;

		//double tmin = VALEUR_MANQUANTE;
		//double tmax = VALEUR_MANQUANTE;
		//double prec = VALEUR_MANQUANTE;

		//for (int n = 0; n < nb_pas; ++n)
		//{
		//	size_t start[] = { time + n, _lig, _col };
		//	size_t count[] = { 1, 1, 1 };

		//	int status;		
		//	double minl, maxl, pr;

		//	status = nc_get_vara_double(_ncid, _tminid, start, count, &minl);
		//	if (status != NC_NOERR)
		//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);			

		//	status = nc_get_vara_double(_ncid, _tmaxid, start, count, &maxl);
		//	if (status != NC_NOERR)
		//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);			

		//	status = nc_get_vara_double(_ncid, _prid, start, count, &pr);
		//	if (status != NC_NOERR)
		//		throw ERREUR_LECTURE_FICHIER(_nom_fichier);			

		//	if (n == 0)
		//	{
		//		tmin = minl;
		//		tmax = maxl;
		//		prec = pr;
		//	}
		//	else
		//	{
		//		tmin = min(tmin, minl);
		//		tmax = max(tmax, maxl);
		//		prec += pr;
		//	}
		//}

		DONNEE_METEO donnee_meteo;

		if ( (_pStations->_netCdf_dataStationTMin[idx] > VALEUR_MANQUANTE && (_pStations->_netCdf_dataStationTMin[idx] < -100.0f || _pStations->_netCdf_dataStationTMin[idx] > 70.0f)) || 
			 (_pStations->_netCdf_dataStationTMax[idx] > VALEUR_MANQUANTE && (_pStations->_netCdf_dataStationTMax[idx] < -100.0f || _pStations->_netCdf_dataStationTMax[idx] > 70.0f)) )
		{
			oss.str("");
			oss << date_heure.PrendreAnnee() << "-" << std::setfill('0') << std::setw(2) << date_heure.PrendreMois() << "-" << std::setfill('0') << std::setw(2) << date_heure.PrendreJour() << " " << std::setfill('0') << std::setw(2) << date_heure.PrendreHeure() << "h";
			throw ERREUR("Erreur source donnees meteo: TMin/TMax: valeur invalide: " + oss.str());
		}

		if(_pStations->_bAutoInverseTMinTMax)
		{
			if (_pStations->_netCdf_dataStationTMin[idx] > VALEUR_MANQUANTE && _pStations->_netCdf_dataStationTMax[idx] > VALEUR_MANQUANTE && _pStations->_netCdf_dataStationTMin[idx] > _pStations->_netCdf_dataStationTMax[idx])
			{
				//inverse automatiquement tmin et tmax s'il sont erroné dans le fichier d'entrée
				fVal = _pStations->_netCdf_dataStationTMin[idx];
				_pStations->_netCdf_dataStationTMin[idx] = _pStations->_netCdf_dataStationTMax[idx];
				_pStations->_netCdf_dataStationTMax[idx] = fVal;
			}

			////si seulement une des valeurs est manquante (tmin ou tmax) ont met les valeurs égale (tmin = tmax)
			//if(tmin == VALEUR_MANQUANTE && tmax > VALEUR_MANQUANTE)
			//	tmin = tmax;
			//else
			//{
			//	if(tmax == VALEUR_MANQUANTE && tmin > VALEUR_MANQUANTE)
			//		tmax = tmin;
			//}
		}
		else
		{
			if(_pStations->_netCdf_dataStationTMin[idx] > VALEUR_MANQUANTE && _pStations->_netCdf_dataStationTMax[idx] > VALEUR_MANQUANTE && _pStations->_netCdf_dataStationTMin[idx] > _pStations->_netCdf_dataStationTMax[idx])
			{
				oss.str("");
				oss << date_heure.PrendreAnnee() << "-" << std::setfill('0') << std::setw(2) << date_heure.PrendreMois() << "-" << std::setfill('0') << std::setw(2) << date_heure.PrendreJour() << " " << std::setfill('0') << std::setw(2) << date_heure.PrendreHeure() << "h";
				throw ERREUR("Erreur source donnees meteo: TMin plus grand que TMax: " + oss.str());
			}
		}

		if(_pStations->_netCdf_dataStationPrecip[idx] > VALEUR_MANQUANTE && _pStations->_netCdf_dataStationPrecip[idx] < 0.0f)
		{
			oss.str("");
			oss << date_heure.PrendreAnnee() << "-" << std::setfill('0') << std::setw(2) << date_heure.PrendreMois() << "-" << std::setfill('0') << std::setw(2) << date_heure.PrendreJour() << " " << std::setfill('0') << std::setw(2) << date_heure.PrendreHeure() << "h";
			throw ERREUR("Erreur source donnees meteo: Pluie invalide: " + oss.str());
		}
		//

		if(_iVersionThiessenMoy3Station == 1)
			donnee_meteo.ChangeTemperature_v1(_pStations->_netCdf_dataStationTMin[idx], _pStations->_netCdf_dataStationTMax[idx]);
		else
			donnee_meteo.ChangeTemperature(_pStations->_netCdf_dataStationTMin[idx], _pStations->_netCdf_dataStationTMax[idx]);

		donnee_meteo.ChangePluie(_pStations->_netCdf_dataStationPrecip[idx]);
		donnee_meteo.ChangeNeige(0.0f);

		return donnee_meteo;
	}


	pair<float, float> STATION_METEO_NETCDF_STATION::PrendreTemperatureJournaliere(const DATE_HEURE& date_heure)
	{
		DATE_HEURE date(date_heure.PrendreAnnee(), date_heure.PrendreMois(), date_heure.PrendreJour(), 0);

		size_t nb_pas = 24 / _pStations->_netCdf_lPasTemps;

		auto donnees = PrendreDonnees(date, static_cast<unsigned short>(_pStations->_netCdf_lPasTemps));

		float tmin = donnees.PrendreTMin();
		float tmax = donnees.PrendreTMax();

		for (size_t n = 1; n < nb_pas; ++n)
		{
			date.AdditionHeure(static_cast<int>(_pStations->_netCdf_lPasTemps));
			donnees = PrendreDonnees(date, static_cast<unsigned short>(_pStations->_netCdf_lPasTemps));

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

}
