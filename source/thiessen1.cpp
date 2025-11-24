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

#include "thiessen1.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "station_meteo.hpp"
#include "transforme_coordonnee.hpp"
#include "util.hpp"
#include "version.hpp"

#include <algorithm>
#include <fstream>
#include <set>


using namespace std;


namespace HYDROTEL
{

	THIESSEN1::THIESSEN1(SIM_HYD& sim_hyd)
		: INTERPOLATION_DONNEES(sim_hyd, "THIESSEN1")
	{
		_pSim_hyd = &sim_hyd;
		_ponderation2 = nullptr;
	}


	THIESSEN1::~THIESSEN1()
	{
	}


	void THIESSEN1::ChangeNbParams(const ZONES& zones)
	{
		_gradient_precipitations.resize(zones.PrendreNbZone(), 0.0f);
		_gradient_temperature.resize(zones.PrendreNbZone(), 0.0f);
		_passage_pluie_neige.resize(zones.PrendreNbZone(), 1);
	}


	void THIESSEN1::ChangeGradientPrecipitation(size_t index, float gradient_precipitation)
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		_gradient_precipitations[index] = gradient_precipitation;
	}

	void THIESSEN1::ChangeGradientTemperature(size_t index, float gradient_temperature)
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		_gradient_temperature[index] = gradient_temperature;
	}

	void THIESSEN1::ChangePassagePluieNeige(size_t index, float passage_pluie_neige)
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		_passage_pluie_neige[index] = passage_pluie_neige;
	}


	float THIESSEN1::PrendreGradientPrecipitation(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		return _gradient_precipitations[index];
	}

	float THIESSEN1::PrendreGradientTemperature(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		return _gradient_temperature[index];
	}

	float THIESSEN1::PrendrePassagePluieNeige(size_t index) const
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		return _passage_pluie_neige[index];
	}


	void THIESSEN1::Initialise()
	{
		if(_ponderation2)
			delete [] _ponderation2;

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


	void THIESSEN1::Calcule()
	{
		RepartieDonnees();
		PassagePluieNeige();

		INTERPOLATION_DONNEES::Calcule();
	}


	void THIESSEN1::Termine()
	{
		if(_ponderation2)
			delete [] _ponderation2;

		INTERPOLATION_DONNEES::Termine();
	}


	void THIESSEN1::RepartieDonnees()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		STATIONS_METEO& stations_meteo = _sim_hyd.PrendreStationsMeteo();

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		std::pair<float, float> temp_jour;
		DONNEE_METEO donnee_station;
		STATION_METEO* pStation;
		ZONE* pZone;
		size_t nbStation, nbStationTotal, index_station, index, index_zone;
		float ponderation, tmin, tmax, pluie, neige, tmin_jour, tmax_jour, altStation, diff_alt;
		float tmin_station, tmax_station, pluie_station, neige_station, tmin_jour_station, tmax_jour_station;

		nbStationTotal = stations_meteo.PrendreNbStation();

		for (index = 0; index < _pondIndexUhrh.size(); index++)
		{
			index_zone = _pondIndexUhrh[index];
			
			pZone = &zones[index_zone];

			tmin = 0.0f;
			tmax = 0.0f;
			pluie = 0.0f;
			neige = 0.0f;
			tmin_jour = 0.0f;
			tmax_jour = 0.0f;

			nbStation = _pondIndexStations[index].size();
			for (index_station = 0; index_station < nbStation; index_station++)
			{
				ponderation = _ponderation2[index_zone*nbStationTotal+_pondIndexStations[index][index_station]];
				pStation = static_cast<STATION_METEO*>(stations_meteo[_pondIndexStations[index][index_station]]);

				altStation = static_cast<float>(pStation->PrendreCoordonnee().PrendreZ());
				diff_alt = pZone->PrendreAltitude() - altStation;

				donnee_station = pStation->PrendreDonnees(date_courante, pas_de_temps);

				tmin_station = donnee_station.PrendreTMin();
				if (tmin_station > VALEUR_MANQUANTE)
					tmin+= (tmin_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;

				tmax_station = donnee_station.PrendreTMax();
				if (tmax_station > VALEUR_MANQUANTE)
					tmax+= (tmax_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;

				pluie_station = donnee_station.PrendrePluie();
				if (pluie_station > VALEUR_MANQUANTE && pluie_station > 0.0f)
					pluie+= pluie_station  * ponderation * (1.0f + (PrendreGradientPrecipitation(index_zone) / 1000.0f) / 100.0f * diff_alt);

				neige_station = donnee_station.PrendreNeige();
				if (neige_station > VALEUR_MANQUANTE && neige_station > 0.0f)
					neige+= neige_station * ponderation * (1.0f + (PrendreGradientPrecipitation(index_zone) / 1000.0f) / 100.0f * diff_alt);

				temp_jour = pStation->PrendreTemperatureJournaliere(date_courante);

				tmin_jour_station = temp_jour.first;
				if (tmin_jour_station > VALEUR_MANQUANTE)
					tmin_jour+= (tmin_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;

				tmax_jour_station = temp_jour.second;
				if (tmax_jour_station > VALEUR_MANQUANTE)
					tmax_jour+= (tmax_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;
			}

			pZone->ChangeTemperature(tmin, tmax);
			pZone->ChangePluie(max(pluie, 0.0f));
			pZone->ChangeNeige(max(neige, 0.0f));

			pZone->ChangeTemperatureJournaliere(tmin_jour, tmax_jour);
		}
	}


	void THIESSEN1::PassagePluieNeige()
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


	bool THIESSEN1::LecturePonderation()
	{
		return LecturePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}
	
	
	//bool THIESSEN1::LecturePonderation(STATIONS_NEIGE& stations, ZONES& zones, MATRICE<float>& ponderation)
	bool THIESSEN1::LecturePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& ponderation)
	{
		vector<size_t> indexStation;
		size_t idxZone;
		bool bSimule;
		
		string nom_fichier = RemplaceExtension(stations.PrendreNomFichier(), "pth");

		if (!FichierExiste(nom_fichier))
			return false;

		ifstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		size_t nb_station, ligne, colonne;
		fichier >> nb_station;
		
		fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		
		if (nb_station != stations.PrendreNbStation())
		{
			fichier.close();
			return false;
		}

		for (size_t index = 0; index < nb_station; ++index)
		{
			string ident;
			double x, y, z;

			fichier >> ident >> x >> y >> z;

			STATION* station = stations.Recherche(ident);

			if (station == nullptr)
			{
				fichier.close();
				return false;
			}

			if (!AlmostEqual(x, station->PrendreCoordonnee().PrendreX(), 0.0001) || 
				!AlmostEqual(y, station->PrendreCoordonnee().PrendreY(), 0.0001) || 
				!AlmostEqual(z, station->PrendreCoordonnee().PrendreZ(), 0.0001))
			{
				fichier.close();
				return false;
			}
		}

		MATRICE<float> pond(zones.PrendreNbZone(), stations.PrendreNbStation(), 0.0f);
		vector<size_t> idxZoneSim = _pSim_hyd->PrendreZonesSimules();

		_pondIndexUhrh.clear();
		_pondIndexStations.clear();
		_ponderation2 = new float[zones.PrendreNbZone() * nb_station];

		for (ligne = 0; ligne < zones.PrendreNbZone(); ++ligne)
		{
			int ident_zone;
			fichier >> ident_zone;

			if (zones.Recherche(ident_zone) == nullptr)
			{
				fichier.close();
				return false;
			}

			idxZone = zones.IdentVersIndex(ident_zone);

			if(find(begin(idxZoneSim), end(idxZoneSim), idxZone) != end(idxZoneSim))
			{
				bSimule = true;
				indexStation.clear();
			}
			else
				bSimule = false;

			for (colonne = 0; colonne < nb_station; ++colonne)
			{
				fichier >> pond(ligne, colonne);
				
				_ponderation2[ligne*nb_station+colonne] = pond(ligne, colonne);

				if(bSimule && pond(ligne, colonne) != 0.0f)
					indexStation.push_back(colonne);
			}

			if(bSimule)
			{
				_pondIndexUhrh.push_back(idxZone);
				_pondIndexStations.push_back(indexStation);
			}
		}

		fichier.close();

		ponderation = pond;
		return true;
	}


	void THIESSEN1::SauvegardePonderation()
	{
		SauvegardePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}
	
	
	void THIESSEN1::SauvegardePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& ponderation)
	{
		string nom_fichier = RemplaceExtension(stations.PrendreNomFichier(), "pth");

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

		fichier.close();
	}

	
	void THIESSEN1::CalculePonderation()
	{
		CalculePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation, "THIESSEN1");
	}
	
	
	void THIESSEN1::CalculePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& ponderation, string sOrigin)
	{
		vector<size_t> index_stations;
		COORDONNEE coordonnee;
		size_t i, j, nbStation;
		bool bSimule;
		int ligne, colonne, ident;

		nbStation = stations.PrendreNbStation();
		if (nbStation < 1)
			throw ERREUR("Erreur; THIESSEN; il doit y avoir au minimum 1 station disponible.");

		const RASTER<int>& grille = zones.PrendreGrille();
		
		TRANSFORME_COORDONNEE trans_coord(stations.PrendreProjection(), grille.PrendreProjection());

		MATRICE<float> pond(zones.PrendreNbZone(), nbStation, 0.0f);
		
		vector<size_t> idxZoneSim = _pSim_hyd->PrendreZonesSimules();		

		vector<size_t> indexStation;

		_pondIndexUhrh.clear();
		_pondIndexStations.clear();

		_ponderation2 = new float[zones.PrendreNbZone() * nbStation];

		vector<COORDONNEE> coordonnees(nbStation);
		for (size_t index = 0; index < nbStation; ++index)
			coordonnees[index] = trans_coord.TransformeXYZ(stations[index]->PrendreCoordonnee());

		const int nb_ligne = static_cast<int>(grille.PrendreNbLigne());
		const int nb_colonne = static_cast<int>(grille.PrendreNbColonne());

		std::cout << endl << "Computing stations/rhhu weightings (thiessen) (" << sOrigin << ")...   " << GetCurrentTimeStr() << flush;
		_listLog.push_back("Computing stations/rhhu weightings (thiessen) (" + sOrigin + ")...   " + GetCurrentTimeStr());
		
		if(_pSim_hyd->_bLogPerf)
			_pSim_hyd->_logPerformance.AddStep("Computing stations/rhhu weightings (thiessen)");

		for (ligne = 0; ligne < nb_ligne; ++ligne)
		{
			for (colonne = 0; colonne < nb_colonne; ++colonne)
			{
				ident = grille(ligne, colonne);

				if (ident != 0 && ident != grille.PrendreNoData())
				{
					coordonnee = grille.LigColVersCoordonnee(ligne, colonne);					
					index_stations = CalculDistance_v1(coordonnees, coordonnee);
					
					i = zones.IdentVersIndex(ident);
					j = index_stations[0];					

					++pond(i, j);
				}
			}
		}

		for(i=0; i<pond.PrendreNbLigne(); i++)
		{
			if(find(begin(idxZoneSim), end(idxZoneSim), i) != end(idxZoneSim))
			{
				bSimule = true;
				indexStation.clear();
			}
			else
				bSimule = false;

			for(j=0; j<pond.PrendreNbColonne(); j++)
			{
				pond(i, j)/= zones[i].PrendreNbPixel();
				
				_ponderation2[i*nbStation+j] = pond(i, j);

				if(bSimule && pond(i, j) != 0.0f)
					indexStation.push_back(j);
			}

			if(bSimule)
			{
				_pondIndexUhrh.push_back(i);
				_pondIndexStations.push_back(indexStation);
			}
		}

		ponderation = pond;
		
		
		if(_pSim_hyd->_bLogPerf)
			_pSim_hyd->_logPerformance.AddStep("Completed");
	}


	void THIESSEN1::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		int ident;
		float gradient_temp, gradient_precip, passage_pluie_neige;
		char c;

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THIESSEN");

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THIESSEN", 1);
		}

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		getline_mod(fichier, ligne); // commentaire

		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier >> ident >> c;
			fichier >> gradient_temp >> c;
			fichier >> gradient_precip >> c;
			fichier >> passage_pluie_neige;

			size_t index_zone = zones.IdentVersIndex(ident);

			ChangeGradientTemperature(index_zone, gradient_temp);
			ChangeGradientPrecipitation(index_zone, gradient_precip);
			ChangePassagePluieNeige(index_zone, passage_pluie_neige);
		}

		fichier.close();
	}


	void THIESSEN1::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal );

		fichier.exceptions(ios::failbit | ios::badbit);

		bool bOK = false;

		try{

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, 1);
		}

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "THIESSEN")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 4)
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide THIESSEN.");
					}

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide THIESSEN. Les ID de groupe doivent etre en ordre croissant.");
					}

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
			fichier.close();
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal + "; THIESSEN");
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele THIESSEN");
	}


	void THIESSEN1::SauvegardeParametres()
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

		fichier.close();
	}

}
