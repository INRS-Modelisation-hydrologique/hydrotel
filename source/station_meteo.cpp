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

#include "station_meteo.hpp"


namespace HYDROTEL
{

	STATION_METEO::STATION_METEO(const std::string& nom_fichier)
		: STATION(nom_fichier)
	{
	}


	STATION_METEO::~STATION_METEO()
	{
	}


	// donnees manquantes annees bissextiles

	//for (unsigned short annee = debut.PrendreAnnee(); annee < fin.PrendreAnnee(); ++annee)
	//{
	//	if (DATE_HEURE::Bissextile(annee))
	//	{
	//		for (unsigned short heure = 0; heure < 24; heure += pas_de_temps)
	//		{
	//			DATE_HEURE date(annee, 2, 29, heure);
	//
	//			for (size_t index = 0; index < nb_station; ++index)
	//			{
	//				auto station = static_pointer_cast<STATION_METEO>(_stations[index]);
	//				station->LectureDonnees(debut, fin, pas_de_temps);
	//
	//				auto donnees = station->PrendreDonnees(date, pas_de_temps);
	//
	//				if (donnees.PrendreTMin() == VALEUR_MANQUANTE &&
	//					donnees.PrendreTMax() == VALEUR_MANQUANTE &&
	//					donnees.PrendrePluie() == VALEUR_MANQUANTE)
	//				{
	//					DATE_HEURE date1(annee, 2, 28, heure);
	//					auto d1 = station->PrendreDonnees(date1, pas_de_temps);
	//
	//					DATE_HEURE date2(annee, 3, 1, heure);
	//					auto d2 = station->PrendreDonnees(date1, pas_de_temps);
	//
	//					donnees.ChangeTemperature( 
	//						(d1.PrendreTMin() + d2.PrendreTMin()) / 2,
	//						(d1.PrendreTMax() + d2.PrendreTMax()) / 2);
	//
	//					donnees.ChangePluie(d1.PrendrePluie() + d2.PrendrePluie());
	//
	//					station->ChangeDonnees(donnees, date, pas_de_temps);
	//				}
	//			}
	//		}
	//	}
	//}

}
