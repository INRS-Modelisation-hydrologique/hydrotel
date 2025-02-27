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

#ifndef STATION_METEO_GIBSI_H_INCLUDED
#define STATION_METEO_GIBSI_H_INCLUDED


#include "station_meteo.hpp"


namespace HYDROTEL
{

	class STATION_METEO_GIBSI : public STATION_METEO
	{
	public:
		STATION_METEO_GIBSI(const std::string& nom_fichier, bool bAutoInverseTMinTMax = false);
		virtual ~STATION_METEO_GIBSI();

		virtual void LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);

		virtual DONNEE_METEO PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual void ChangeDonnees(const DONNEE_METEO& donnee_meteo, const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual std::pair<float, float> PrendreTemperatureJournaliere(const DATE_HEURE& date_heure);
    
	public:
		bool				_bAutoInverseTMinTMax;

	private:
		//std::vector<DONNEE_METEO> _donnees_meteo;

		std::vector<float> _tmin;			// C
		std::vector<float> _tmax;			// C
		std::vector<float> _pluie;			// mm
		std::vector<float> _neige;			// mm
		
		size_t _nb_donnee;

		DATE_HEURE _date_debut;
		unsigned short _pas_de_temps;		//pas de temps de la simulation
	};

}

#endif
