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

#include "thiessen2.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "station_meteo.hpp"
#include "transforme_coordonnee.hpp"
#include "util.hpp"
#include "version.hpp"

#include <fstream>
#include <set>


using namespace std;


namespace HYDROTEL
{

	THIESSEN2::THIESSEN2(SIM_HYD& sim_hyd)
		: INTERPOLATION_DONNEES(sim_hyd, "THIESSEN2")
	{
		_pSim_hyd = &sim_hyd;
		_ponderation2 = nullptr;

		_gradient_station_temp = -0.5f;		//default values
		_gradient_station_precip = 0.5f;	//
	}


	THIESSEN2::~THIESSEN2()
	{
	}

	void THIESSEN2::ChangeNbParams(const ZONES& zones)
	{
		_gradient_precipitations.resize(zones.PrendreNbZone(), 0.0f);
		_gradient_temperature.resize(zones.PrendreNbZone(), 0.0f);
		_passage_pluie_neige.resize(zones.PrendreNbZone(), 1.0f);
	}


	void THIESSEN2::ChangeGradientPrecipitation(size_t index, float gradient_precipitation)
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		_gradient_precipitations[index] = gradient_precipitation;
	}

	void THIESSEN2::ChangeGradientTemperature(size_t index, float gradient_temperature)
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		_gradient_temperature[index] = gradient_temperature;
	}

	void THIESSEN2::ChangePassagePluieNeige(size_t index, float passage_pluie_neige)
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		_passage_pluie_neige[index] = passage_pluie_neige;
	}


	float THIESSEN2::PrendreGradientPrecipitation(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		return _gradient_precipitations[index];
	}

	float THIESSEN2::PrendreGradientTemperature(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		return _gradient_temperature[index];
	}

	float THIESSEN2::PrendrePassagePluieNeige(size_t index) const
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		return _passage_pluie_neige[index];
	}

	void THIESSEN2::Initialise()
	{
		if(_ponderation2)
			delete [] _ponderation2;

		if (!LecturePonderation())
		{
			CalculePonderation();
			SauvegardePonderation();
		}

		_sim_hyd.PrendreStationsMeteo().LectureDonnees_v2(
			_sim_hyd.PrendreDateDebut(), 
			_sim_hyd.PrendreDateFin(), 
			_sim_hyd.PrendrePasDeTemps());

		INTERPOLATION_DONNEES::Initialise();
	}


	void THIESSEN2::Calcule()
	{
		RepartieDonnees();
		PassagePluieNeige();

		INTERPOLATION_DONNEES::Calcule();
	}


	void THIESSEN2::Termine()
	{
		if(_ponderation2)
			delete [] _ponderation2;

		INTERPOLATION_DONNEES::Termine();
	}


	void THIESSEN2::RepartieDonnees()
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
		float tmin_station, tmax_station, pluie_station, neige_station, tmin_jour_station, tmax_jour_station, fValTemp;

		nbStationTotal = stations_meteo.PrendreNbStation();

		for (index = 0; index < _pondIndexUhrh.size(); index++)
		{
			index_zone = _pondIndexUhrh[index];
			
			pZone = &zones[index_zone];

			tmin = VALEUR_MANQUANTE;
			tmax = VALEUR_MANQUANTE;
			pluie = VALEUR_MANQUANTE;
			neige = VALEUR_MANQUANTE;
			tmin_jour = VALEUR_MANQUANTE;
			tmax_jour = VALEUR_MANQUANTE;

			nbStation = _pondIndexStations[index].size();
			for (index_station = 0; index_station < nbStation; index_station++)
			{
				ponderation = static_cast<float>(_ponderation2[index_zone*nbStationTotal+_pondIndexStations[index][index_station]]);
				pStation = static_cast<STATION_METEO*>(stations_meteo[_pondIndexStations[index][index_station]]);

				altStation = static_cast<float>(pStation->PrendreCoordonnee().PrendreZ());
				diff_alt = pZone->PrendreAltitude() - altStation;

				donnee_station = pStation->PrendreDonnees(date_courante, pas_de_temps);

				tmin_station = donnee_station.PrendreTMin();
				if (tmin_station > VALEUR_MANQUANTE)
				{
					if(tmin == VALEUR_MANQUANTE)
						tmin = 0.0f;

					tmin+= (tmin_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;
				}

				tmax_station = donnee_station.PrendreTMax();
				if (tmax_station > VALEUR_MANQUANTE)
				{
					if(tmax == VALEUR_MANQUANTE)
						tmax = 0.0f;

					tmax+= (tmax_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;
				}

				pluie_station = donnee_station.PrendrePluie();
				if (pluie_station > VALEUR_MANQUANTE)
				{
					if(pluie == VALEUR_MANQUANTE)
						pluie = 0.0f;

					if(pluie_station != 0.0f)
					{
						fValTemp = pluie_station * ponderation * (1.0f + (PrendreGradientPrecipitation(index_zone) / 1000.0f) / 100.0f * diff_alt);
						if(fValTemp > 0.0f)
							pluie+= fValTemp;
					}
				}

				neige_station = donnee_station.PrendreNeige();
				if (neige_station > VALEUR_MANQUANTE)
				{
					if(neige == VALEUR_MANQUANTE)
						neige = 0.0f;

					if(neige_station != 0.0f)
					{
						fValTemp = neige_station * ponderation * (1.0f + (PrendreGradientPrecipitation(index_zone) / 1000.0f) / 100.0f * diff_alt);
						if(fValTemp > 0.0f)
							neige+= fValTemp;
					}
				}

				temp_jour = pStation->PrendreTemperatureJournaliere(date_courante);

				tmin_jour_station = temp_jour.first;
				if (tmin_jour_station > VALEUR_MANQUANTE)
				{
					if(tmin_jour == VALEUR_MANQUANTE)
						tmin_jour = 0.0f;

					tmin_jour+= (tmin_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;
				}

				tmax_jour_station = temp_jour.second;
				if (tmax_jour_station > VALEUR_MANQUANTE)
				{
					if(tmax_jour == VALEUR_MANQUANTE)
						tmax_jour = 0.0f;

					tmax_jour+= (tmax_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;
				}
			}

			if(pluie == VALEUR_MANQUANTE || neige == VALEUR_MANQUANTE || tmin == VALEUR_MANQUANTE || tmax == VALEUR_MANQUANTE || tmin_jour == VALEUR_MANQUANTE || tmax_jour == VALEUR_MANQUANTE)
			{
				ostringstream oss;
				oss.str("");
				oss << "Erreur interpolation donnees meteo: aucune donnees disponible pour uhrh " << pZone->PrendreIdent() << ", " << date_courante.PrendreAnnee() << "/" << date_courante.PrendreMois() << "/" << date_courante.PrendreJour() << ".";
				throw ERREUR(oss.str());
			}

			pZone->ChangeTemperature(tmin, tmax);
			pZone->ChangeTemperatureJournaliere(tmin_jour, tmax_jour);
			pZone->ChangePluie(pluie);
			pZone->ChangeNeige(neige);
		}
	}


	void THIESSEN2::PassagePluieNeige()
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
	
	
	bool THIESSEN2::LecturePonderation(STATIONS& stations, ZONES& zones, MATRICE<double>& ponderation)
	{
		STATION* station;
		vector<size_t> indexStation;
		double x, y, z;
		size_t idxZone, index;
		string ident;
		bool bSimule;
		int ident_zone;
		
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

		for (index = 0; index < nb_station; ++index)
		{
			fichier >> ident >> x >> y >> z;

			station = stations.Recherche(ident);
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

		MATRICE<double> pond(zones.PrendreNbZone(), stations.PrendreNbStation(), 0.0);
		vector<size_t> idxZoneSim = _pSim_hyd->PrendreZonesSimules();

		_pondIndexUhrh.clear();
		_pondIndexStations.clear();
		_ponderation2 = new double[zones.PrendreNbZone() * nb_station];

		for (ligne = 0; ligne < zones.PrendreNbZone(); ++ligne)
		{
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

				if(bSimule && pond(ligne, colonne) != 0.0)
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

	bool THIESSEN2::LecturePonderation()
	{
		return LecturePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}
	
	
	void THIESSEN2::SauvegardePonderation(STATIONS& stations, ZONES& zones, MATRICE<double>& ponderation)
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
	
	void THIESSEN2::SauvegardePonderation()
	{
		SauvegardePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}
	
	
	void THIESSEN2::CalculePonderation(STATIONS& stations, ZONES& zones, MATRICE<double>& ponderation, bool bDisplay)
	{
		vector<double> uhrhPondValue;
		vector<size_t> index_stations;
		COORDONNEE coordonnee;
		size_t i, j, nbStation, idxNearestStation, idx, index, nbZone;
		bool bSimule;
		int ligne, colonne, ident, iNoData;

		nbStation = stations.PrendreNbStation();
		if (nbStation < 1)
			throw ERREUR("Error: THIESSEN: there must be at least 1 weather station available.");

		const RASTER<int>& grille = zones.PrendreGrille();

		iNoData = grille.PrendreNoData();
		nbZone = zones.PrendreNbZone();
		
		TRANSFORME_COORDONNEE trans_coord(stations.PrendreProjection(), grille.PrendreProjection());

		MATRICE<double> pond(zones.PrendreNbZone(), nbStation, 0.0);
		
		vector<size_t> idxZoneSim = _pSim_hyd->PrendreZonesSimules();		

		vector<size_t> indexStation;

		_pondIndexUhrh.clear();
		_pondIndexStations.clear();

		_ponderation2 = new double[zones.PrendreNbZone() * nbStation];

		vector<COORDONNEE> coordonnees(nbStation);
		for (index = 0; index < nbStation; index++)
			coordonnees[index] = trans_coord.TransformeXYZ(stations[index]->PrendreCoordonnee());

		uhrhPondValue.resize(nbZone, 0.0);
		for(index=0; index!=nbZone; index++)
			uhrhPondValue[index] = 1.0 / zones[index].PrendreNbPixel();

		const int nb_ligne = static_cast<int>(grille.PrendreNbLigne());
		const int nb_colonne = static_cast<int>(grille.PrendreNbColonne());

		//nbPixelTotal = nb_ligne * nb_colonne;
		//pixelEnCours = 0;

		if(bDisplay)
		{
			std::cout << std::endl;
			std::cout << "computing stations/rhhu weightings (thiessen)..." << std::endl;
			//std::cout << "pixel " << pixelEnCours << "/" << nbPixelTotal << '\r' << std::flush;
		}

		for (ligne=0; ligne!=nb_ligne; ligne++)
		{
			for (colonne=0; colonne!=nb_colonne; colonne++)
			{
				ident = grille(ligne, colonne);

				if(ident != iNoData)
				{
					coordonnee = grille.LigColVersCoordonnee(ligne, colonne);
					
					idxNearestStation = GetIndexNearestCoord(coordonnees, coordonnee);
					idx = zones._vIdentVersIndex[abs(ident)];

					pond(idx, idxNearestStation)+= uhrhPondValue[idx];
				}

				//if(bDisplay)
				//{
				//	++pixelEnCours;
				//	std::cout << "pixel " << pixelEnCours << "/" << nbPixelTotal << '\r' << std::flush;
				//}
			}
		}

		for(i=0; i!=pond.PrendreNbLigne(); i++)
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
				//pond(i, j)/= zones[i].PrendreNbPixel();
				
				_ponderation2[i*nbStation+j] = pond(i, j);

				if(bSimule && pond(i, j) != 0.0)
					indexStation.push_back(j);
			}

			if(bSimule)
			{
				_pondIndexUhrh.push_back(i);
				_pondIndexStations.push_back(indexStation);
			}
		}

		ponderation = pond;
	}

	void THIESSEN2::CalculePonderation()
	{
		CalculePonderation(_sim_hyd.PrendreStationsMeteo(), _sim_hyd.PrendreZones(), _ponderation);
	}


	void THIESSEN2::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		istringstream iss;
		float gradient_temp, gradient_precip, passage_pluie_neige;
		char c;
		int ident;

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THIESSEN");

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

		//check if gradient station parameters exist (added in version 4.1.8). If not exist default values apply		
		getline_mod(fichier, ligne); // commentaire ou gradient
		if(ligne.size() > 37 && ligne.substr(0, 37) == "GRADIENT TEMPERATURE STATION(C/100m);")
		{
			ligne = ligne.substr(37, string::npos);
			iss.clear();
			iss.str(ligne);
			iss >> _gradient_station_temp;

			getline_mod(fichier, ligne);
			if(ligne.size() > 40 && ligne.substr(0, 40) == "GRADIENT PRECIPITATION STATION(mm/100m);")
			{
				ligne = ligne.substr(40, string::npos);
				iss.clear();
				iss.str(ligne);
				iss >> _gradient_station_precip;
			}
			else
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THIESSEN", 5);

			getline_mod(fichier, ligne);	//empty line
			getline_mod(fichier, ligne);	//header
		}

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


	void THIESSEN2::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal );

		bool bOK = false;

		try{

		vector<float> vValeur;
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
		int ident, xStart;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "THIESSEN")
			{
				//check if gradient station parameters exist (added in version 4.1.8). If not exist default values apply
				++no_ligne;
				getline_mod(fichier, ligne);
				vValeur = extrait_fvaleur(ligne, ";");

				if(vValeur.size() == 2)
				{
					_gradient_station_temp = vValeur[0];
					_gradient_station_precip = vValeur[1];

					xStart = 0;
				}
				else
				{
					if(vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide THIESSEN.");

					x = 0;

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide THIESSEN. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						ChangeGradientTemperature(index_zone, vValeur[1]);
						ChangeGradientPrecipitation(index_zone, vValeur[2]);
						ChangePassagePluieNeige(index_zone, vValeur[3]);
					}

					xStart = 1;
				}

				for(x=xStart; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					vValeur = extrait_fvaleur(ligne, ";");

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
		catch(const ERREUR_LECTURE_FICHIER& ex)
		{
			fichier.close();
			throw ex;
		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal + "; THIESSEN");
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele THIESSEN");
	}


	void THIESSEN2::SauvegardeParametres()
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

		fichier << "GRADIENT TEMPERATURE STATION(C/100m);" << _gradient_station_temp << endl;
		fichier << "GRADIENT PRECIPITATION STATION(mm/100m);" << _gradient_station_precip << endl;
		fichier << endl;

		fichier << "UHRH ID;GRADIENT TEMPERATURE(C/100m);GRADIENT PRECIPITATION(mm/100m);PASSAGE PLUIE NEIGE(C)" << endl;
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
