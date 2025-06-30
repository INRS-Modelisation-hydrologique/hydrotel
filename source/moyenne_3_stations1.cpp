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

#include "moyenne_3_stations1.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "station_meteo.hpp"
#include "transforme_coordonnee.hpp"
#include "util.hpp"
#include "version.hpp"

#include <fstream>


using namespace std;


namespace HYDROTEL
{

	MOYENNE_3_STATIONS1::MOYENNE_3_STATIONS1(SIM_HYD& sim_hyd)
		: INTERPOLATION_DONNEES(sim_hyd, "MOYENNE 3 STATIONS1")
	{
	}

	MOYENNE_3_STATIONS1::~MOYENNE_3_STATIONS1()
	{
	}

	void MOYENNE_3_STATIONS1::ChangeNbParams(const ZONES& zones)
	{
		_gradient_precipitations.resize(zones.PrendreNbZone(), 0.0f);
		_gradient_temperature.resize(zones.PrendreNbZone(), 0.0f);
		_passage_pluie_neige.resize(zones.PrendreNbZone(), 1);
	}

	void MOYENNE_3_STATIONS1::ChangeGradientPrecipitation(size_t index, float gradient_precipitation)
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		_gradient_precipitations[index] = gradient_precipitation;
	}

	void MOYENNE_3_STATIONS1::ChangeGradientTemperature(size_t index, float gradient_temperature)
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		_gradient_temperature[index] = gradient_temperature;
	}

	void MOYENNE_3_STATIONS1::ChangePassagePluieNeige(size_t index, float passage_pluie_neige)
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		_passage_pluie_neige[index] = passage_pluie_neige;
	}


	float MOYENNE_3_STATIONS1::PrendreGradientPrecipitation(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		return _gradient_precipitations[index];
	}

	float MOYENNE_3_STATIONS1::PrendreGradientTemperature(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		return _gradient_temperature[index];
	}

	float MOYENNE_3_STATIONS1::PrendrePassagePluieNeige(size_t index) const
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		return _passage_pluie_neige[index];
	}

	void MOYENNE_3_STATIONS1::Initialise()
	{
		if (!LecturePonderation())
		{
			CalculePonderation();
			SauvegardePonderation();
		}

		_sim_hyd.PrendreStationsMeteo().LectureDonnees_v1(
			_sim_hyd.PrendreDateDebut(), 
			_sim_hyd.PrendreDateFin(), 
			_sim_hyd.PrendrePasDeTemps());

		INTERPOLATION_DONNEES::Initialise();
	}

	void MOYENNE_3_STATIONS1::Calcule()
	{
		RepartieDonnees();
		PassagePluieNeige();

		INTERPOLATION_DONNEES::Calcule();
	}

	void MOYENNE_3_STATIONS1::Termine()
	{
		INTERPOLATION_DONNEES::Termine();
	}

	void MOYENNE_3_STATIONS1::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES MOYENNE_3_STATIONS");

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES MOYENNE_3_STATIONS", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		getline_mod(fichier, ligne); // commentaire

		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			int ident;
			float gradient_temp, gradient_precip, passage_pluie_neige;
			char c;

			fichier >> ident >> c;
			fichier >> gradient_temp >> c;
			fichier >> gradient_precip >> c;
			fichier >> passage_pluie_neige;

			size_t index_zone = zones.IdentVersIndex(ident);

			ChangeGradientTemperature(index_zone, gradient_temp);
			ChangeGradientPrecipitation(index_zone, gradient_precip);
			ChangePassagePluieNeige(index_zone, passage_pluie_neige);
		}
	}

	void MOYENNE_3_STATIONS1::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL;" + _sim_hyd._nomFichierParametresGlobal );

		fichier.exceptions(ios::failbit | ios::badbit);

		bool bOK = false;

		try{

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, 1);

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "MOYENNE 3 STATIONS")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide MOYENNE 3 STATIONS.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide MOYENNE 3 STATIONS. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						ChangeGradientTemperature(index_zone, vValeur[1]);
						ChangeGradientPrecipitation(index_zone, vValeur[2]);
						ChangePassagePluieNeige(index_zone, vValeur[3]);
					}
				}

				bOK = true;
				break;
			}

			++no_ligne;
		}

		fichier.close();

		}
		catch(...)
		{
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; MOYENNE 3 STATIONS;" + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele MOYENNE 3 STATIONS");
	}

	void MOYENNE_3_STATIONS1::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModeleWithoutVersion() << endl;
		fichier << endl;

		fichier << "UHRH ID;GRADIENT TEMPERATURE(C/100m);GRADIENT PRECIPITATION(mm/100m);PASSAGE PLUIE NEIGE(C);" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << PrendreGradientTemperature(index) << ';';
			fichier << PrendreGradientPrecipitation(index) << ';';
			fichier << PrendrePassagePluieNeige(index) << endl;
		}
	}

	bool MOYENNE_3_STATIONS1::LecturePonderation()
	{
		return LecturePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}

	void MOYENNE_3_STATIONS1::SauvegardePonderation()
	{
		return SauvegardePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}

	void MOYENNE_3_STATIONS1::CalculePonderation()
	{
		return CalculePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}

	bool MOYENNE_3_STATIONS1::LecturePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& pond)
	{
		string nom_fichier = RemplaceExtension(stations.PrendreNomFichier(), "p3s");

		if (!FichierExiste(nom_fichier))
			return false;

		ifstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PONDERATION P3S; " + nom_fichier);

		size_t nb_station;
		fichier >> nb_station;
		
		fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		
		if (nb_station != stations.PrendreNbStation())
			return false;

		for (size_t index = 0; index < nb_station; ++index)
		{
			string ident;
			double x, y, z;

			fichier >> ident >> x >> y >> z;

			STATION* station = stations.Recherche(ident);

			if (station == nullptr)
				return false;

			if (!AlmostEqual(x, station->PrendreCoordonnee().PrendreX(), 0.0001) || 
				!AlmostEqual(y, station->PrendreCoordonnee().PrendreY(), 0.0001) || 
				!AlmostEqual(z, station->PrendreCoordonnee().PrendreZ(), 0.0001))
				return false;
		}

		MATRICE<float> ponderation(zones.PrendreNbZone(), stations.PrendreNbStation(), 0.0f);

		for (size_t ligne = 0; ligne < zones.PrendreNbZone(); ++ligne)
		{
			int ident_zone;
			fichier >> ident_zone;

			if (zones.Recherche(ident_zone) == nullptr)
				return false;

			for (size_t colonne = 0; colonne < nb_station; ++colonne)
			{
				fichier >> ponderation(ligne, colonne);				
			}
		}

		pond = ponderation;

		return true;
	}

	void MOYENNE_3_STATIONS1::SauvegardePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& ponderation)
	{
		string nom_fichier = RemplaceExtension(stations.PrendreNomFichier(), "p3s");

		ofstream fichier(nom_fichier);
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		const size_t nb_station = stations.PrendreNbStation();

		fichier << nb_station << endl;

		for (size_t index = 0; index < nb_station; ++index)
		{
			COORDONNEE coordonnee = stations[index]->PrendreCoordonnee();

			fichier << fixed 
				    << stations[index]->PrendreIdent() << ' '
				    << coordonnee.PrendreX() << ' '
				    << coordonnee.PrendreY() << ' '
				    << coordonnee.PrendreZ() << endl;
		}

		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ' ';

			for (size_t n = 0; n < nb_station; ++n)			
				fichier << ponderation(index, n) << ' ';
			
			fichier << endl;
		}
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//utilise des vecteur plutot qu'une map pour trier les distances pour augmenter la rapidité d'execution
	//egalement le map considère la clé identique à 4 decimale de la valeur, ce qui fait qu'il peut manquer des 
	//stations ds le map des distance. S'il y a 3 stations au total, on peut finir avec 2 stations dans le map 
	//et cela n'est pas permis.

	void MOYENNE_3_STATIONS1::CalculePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& pond)
	{
		struct StationInfo
		{
			double _distance;
			size_t _index_station;

			StationInfo(double distance, size_t index_station)
				: _distance(distance)
				, _index_station(index_station)
			{
				if (_distance == 0.0)
					_distance = 0.00001;
			}
		};

		COORDONNEE coordonnee;
		vector<double> vDistances;
		double minDistance, facteur;
		size_t index, minIndex1, minIndex2, minIndex3, i, j, nbStation;
		int ligne, colonne, ident, iNoData;

		nbStation = stations.PrendreNbStation();

		if (nbStation < 3)
			throw ERREUR("Erreur; MOYENNE 3 STATIONS; il doit y avoir au minimum 3 stations meteorologiques disponibles.");

		const RASTER<int>& grille = zones.PrendreGrille();
		TRANSFORME_COORDONNEE trans_coord(stations.PrendreProjection(), grille.PrendreProjection());
		MATRICE<float> ponderation(zones.PrendreNbZone(), nbStation, 0.0f);

		vector<COORDONNEE> coordonnees(nbStation);
		for (index=0; index<nbStation; index++)
			coordonnees[index] = trans_coord.TransformeXYZ(stations[index]->PrendreCoordonnee());

		const int nb_ligne = static_cast<int>(grille.PrendreNbLigne());
		const int nb_colonne = static_cast<int>(grille.PrendreNbColonne());

		iNoData = grille.PrendreNoData();

		for (ligne=0; ligne<nb_ligne; ligne++)
		{
			for (colonne=0; colonne<nb_colonne; colonne++)
			{
				ident = grille(ligne, colonne);
				if(ident != iNoData)
				{
					coordonnee = grille.LigColVersCoordonnee(ligne, colonne);

					//obtient les distances pour toutes les stations
					CalculDistanceEx(coordonnees, coordonnee, &vDistances);

					//obtient l'index des 3 stations les plus près (plus faible distance)
					minIndex1 = (size_t)-1;
					minIndex2 = (size_t)-1;
					minIndex3 = (size_t)-1;

					//station1
					minDistance = 1000000000.0;					
					for(i=0; i<nbStation; i++)
					{
						if(vDistances.at(i) < minDistance)
						{
							minDistance = vDistances.at(i);
							minIndex1 = i;
						}
					}
					if(minIndex1 == ((size_t)-1))
						throw ERREUR("Error: MOYENNE 3 STATIONS: CalculePonderation: minIndex1 == ((size_t)-1)");

					StationInfo st1(minDistance, minIndex1);

					//station2
					minDistance = 1000000000.0;
					for(i=0; i<nbStation; i++)
					{
						if(i != minIndex1 && vDistances.at(i) < minDistance)
						{
							minDistance = vDistances.at(i);
							minIndex2 = i;
						}
					}
					if(minIndex2 == ((size_t)-1))
						throw ERREUR("Error: MOYENNE 3 STATIONS: CalculePonderation: minIndex2 == ((size_t)-1)");

					StationInfo st2(minDistance, minIndex2);

					//station3
					minDistance = 1000000000.0;
					for(i=0; i<nbStation; i++)
					{
						if(i != minIndex1 && i != minIndex2 && vDistances.at(i) < minDistance)
						{
							minDistance = vDistances.at(i);
							minIndex3 = i;
						}
					}
					if(minIndex3 == ((size_t)-1))
						throw ERREUR("Error: MOYENNE 3 STATIONS: CalculePonderation: minIndex3 == ((size_t)-1)");

					StationInfo st3(minDistance, minIndex3);

					facteur = 1.0 / (1.0 / st1._distance + 1.0 / st2._distance + 1.0 / st3._distance);

					ponderation(zones.IdentVersIndex(ident), st1._index_station)+= static_cast<float>(facteur / st1._distance);
					ponderation(zones.IdentVersIndex(ident), st2._index_station)+= static_cast<float>(facteur / st2._distance);
					ponderation(zones.IdentVersIndex(ident), st3._index_station)+= static_cast<float>((1.0 - facteur / st1._distance - facteur / st2._distance));
				}
			}
		}

		for (j=0; j<ponderation.PrendreNbLigne(); j++)
		{
			for (i=0; i<ponderation.PrendreNbColonne(); i++)
				ponderation(j, i) = ponderation(j, i) / zones[j].PrendreNbPixel();
		}

		pond = ponderation;
	}


	void MOYENNE_3_STATIONS1::RepartieDonnees()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		STATIONS_METEO& stations_meteo = _sim_hyd.PrendreStationsMeteo();

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			float fGradient;

			float tmin = 0.0f;
			float tmax = 0.0f;
			float pluie = 0.0f;
			float neige = 0.0f;

			float tmin_jour = 0.0f;
			float tmax_jour = 0.0f;

			for (size_t index_station = 0; index_station < stations_meteo.PrendreNbStation(); ++index_station)
			{
				float ponderation = _ponderation(index_zone, index_station);

				if (ponderation > 0.0f)
				{
					auto station_meteo = static_cast<STATION_METEO*>(stations_meteo[index_station]);

					float diff_alt = static_cast<float>(zones[index_zone].PrendreAltitude() - station_meteo->PrendreCoordonnee().PrendreZ());
					auto donnee_station = station_meteo->PrendreDonnees(date_courante, pas_de_temps);

					float tmin_station = donnee_station.PrendreTMin();
					if (tmin_station > VALEUR_MANQUANTE)
					{
						fGradient = PrendreGradientTemperature(index_zone);
						tmin+= (tmin_station + fGradient * diff_alt / 100.0f) * ponderation;
					}

					float tmax_station = donnee_station.PrendreTMax();
					if (tmax_station > VALEUR_MANQUANTE)
						tmax+= (tmax_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;

					float pluie_station = donnee_station.PrendrePluie();
					if (pluie_station > VALEUR_MANQUANTE && pluie_station > 0.0f)
						pluie+= pluie_station  * ponderation * (1 + (PrendreGradientPrecipitation(index_zone) / 1000.0f) / 100.0f * diff_alt);

					float neige_station = donnee_station.PrendreNeige();
					if (neige_station > VALEUR_MANQUANTE && neige_station > 0.0f)
						neige+= neige_station * ponderation * (1 + (PrendreGradientPrecipitation(index_zone) / 1000.0f) / 100.0f * diff_alt);

					auto temp_jour = station_meteo->PrendreTemperatureJournaliere(date_courante);

					float tmin_jour_station = temp_jour.first;
					if (tmin_jour_station > VALEUR_MANQUANTE)
						tmin_jour+= (tmin_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;

					float tmax_jour_station = temp_jour.second;
					if (tmax_jour_station > VALEUR_MANQUANTE)
						tmax_jour+= (tmax_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;
				}
			}

			zone.ChangeTemperature(tmin, tmax);
			zone.ChangePluie(max(pluie, 0.0f));
			zone.ChangeNeige(max(neige, 0.0f));

			zone.ChangeTemperatureJournaliere(tmin_jour, tmax_jour);
		}
	}

	void MOYENNE_3_STATIONS1::PassagePluieNeige()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();
		
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			float pluie = zone.PrendrePluie();
			float neige = zone.PrendreNeige();

			const float tmin = zone.PrendreTMin();
			const float tmax = zone.PrendreTMax();

			const float cst_pluie_en_neige = PrendrePassagePluieNeige(index_zone);

			if (pas_de_temps == 1)
			{
				float densite_neige = CalculDensiteNeige(tmin) / DENSITE_EAU;
				if (tmin > cst_pluie_en_neige)
				{
					// toute la neige en pluie
					pluie+= neige * densite_neige;
					neige = 0.0f;
				}
				else
				{
					// toute la pluie en neige
					neige+= pluie / densite_neige;
					pluie = 0.0f;
				}
			}
			else
			{
				float taux_transformation;

				if (tmax < cst_pluie_en_neige) 
				{
					taux_transformation = 0.0f;
				}
				else if (tmin >= cst_pluie_en_neige) 
				{
					taux_transformation = 1.0f;
				}
				else  
				{
					taux_transformation = min(1.0f, (tmax - cst_pluie_en_neige) / (tmax - tmin));
				}

				const float tmoy = (tmax + tmin) / 2.0f;

				float densite_neige = CalculDensiteNeige(tmoy) / DENSITE_EAU;

				float precipitation = pluie + neige;

				pluie = taux_transformation * precipitation;
				neige = (1.0f - taux_transformation) * precipitation / densite_neige;
			}

			zone.ChangePluie(pluie);
			zone.ChangeNeige(neige);
		}		
	}

}
