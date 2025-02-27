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

#include "stations_neige.hpp"

#include "erreur.hpp"
#include "projections.hpp"
#include "station_neige_mef.hpp"
#include "util.hpp"

#include <fstream>


using namespace std;


namespace HYDROTEL
{

	STATIONS_NEIGE::STATIONS_NEIGE()
	{
	}

	STATIONS_NEIGE::~STATIONS_NEIGE()
	{
	}

	void STATIONS_NEIGE::Lecture(const PROJECTION& projection)
	{
		Detruire();

		string extension = PrendreExtension(PrendreNomFichier());

		if (extension == ".stn")
		{
			LectureFormatSTN(projection);
		}
		else
		{
			throw ERREUR_LECTURE_FICHIER( PrendreNomFichier() );
		}

		CreeMapRecherche();
	}


	void STATIONS_NEIGE::LectureFormatSTN(const PROJECTION& projection)
	{
		ifstream fichier(_nom_fichier);
		if (!fichier)
		{
			string str = "STATIONS_NEIGE; erreur lecture fichier; ";
			str+= _nom_fichier;
			throw ERREUR(str);
		}
		
		try
		{
			vector<string> sList;
			istringstream iss;
			size_t nb_station, n, nbLu;
			string ligne;
			double x, y;
			int format, type;

			nbLu = 0;

			ligne = "";
			while(ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			iss.clear();
			iss.str(ligne);
			iss >> type;					//type; 1=UTM //2=long/lat wgs84

			ligne = "";
			while(ligne == "")
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			iss.clear();
			iss.str(ligne);
			iss >> nb_station;				//nb_station

			ligne = "";
			while(ligne == "")
			{
				getline_mod(fichier, ligne);	//commentaire
				ligne = TrimString(ligne);
			}

			switch (type)
			{
			case 1: // long/lat wgs84
				_projection = PROJECTIONS::LONGLAT_WGS84();
				break;

			case 2: // meme format de la grille
				_projection = projection;
				break;

			default:
				ligne = "STATIONS_NEIGE; erreur lecture fichier; ";
				ligne+= _nom_fichier;
				ligne+= "; type invalide.";
				throw ERREUR(ligne);
			}

			vector<shared_ptr<STATION>> stations(nb_station);

			for (n = 0; n < nb_station; ++n)
			{
				ligne = "";
				while(ligne == "")	//ignore les lignes vides s'il y en a
				{
					getline_mod(fichier, ligne);
					ligne = TrimString(ligne);
				}

				SplitString(sList, ligne, " \t", true, true);
				if(sList.size() < 5)
				{
					ligne = "STATIONS_NEIGE; erreur lecture fichier; ";
					ligne+= _nom_fichier;
					ligne+= "; le nb de colonne est invalide.";
					throw ERREUR(ligne);
				}

				// NOTE: on ignore l'occupation

				iss.clear();
				iss.str(sList[4]);
				iss >> format;

				//--------------------------------
				//il y a 1 seul format valide donc on ignore le code du format pour l'instant
				//switch (format)
				//{
				//case 7:	// FORMAT MEF

				//	ligne = sList[0];
				//	ligne.append(".nei");
				//	ligne = Combine(PrendreRepertoire(_nom_fichier), ligne);

				//	stations[n] = make_shared<STATION_NEIGE_MEF>(ligne);
				//	break;

				//default:
				//	ligne = "STATIONS_NEIGE; erreur lecture fichier; ";
				//	ligne+= _nom_fichier;
				//	ligne+= "; numero de format invalide.";
				//	throw ERREUR(ligne);
				//}


				ligne = sList[0];
				ligne.append(".nei");
				ligne = Combine(PrendreRepertoire(_nom_fichier), ligne);
				stations[n] = make_shared<STATION_NEIGE_MEF>(ligne);
				//--------------------------------


				if(type == 1)	//long/lat
				{
					x = ParseLatLongCoord(sList[1], true);
					y = ParseLatLongCoord(sList[2], false);
				}
				else
				{
					iss.clear();
					iss.str(sList[1]);
					iss >> x;

					iss.clear();
					iss.str(sList[2]);
					iss >> y;
				}

				stations[n].get()->ChangeIdent(sList[0]);
				stations[n].get()->ChangeNom(sList[0]);
				stations[n].get()->ChangeCoordonnee(COORDONNEE(x, y, 0.0));

				++nbLu;
			}

			fichier.close();

			if(nbLu != nb_station)
            {
                ligne = "STATIONS_NEIGE; erreur lecture fichier; ";
				ligne+= _nom_fichier;
				ligne+= "; nombre de station invalide.";
				throw ERREUR(ligne);
            }

			_stations.swap(stations);
		}

		catch(const ERREUR& err)
		{
			fichier.close();
			throw ERREUR(err.what());
		}

		catch(...)
		{
			fichier.close();
			string str = "STATIONS_NEIGE; erreur lecture fichier; ";
			str+= _nom_fichier;
			throw ERREUR(str);
		}
	}


	void STATIONS_NEIGE::LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin)
	{
		for (size_t index = 0; index < _stations.size(); ++index)
		{
			auto station = static_pointer_cast<STATION_NEIGE>(_stations[index]);
			station->LectureDonnees(debut, fin);
		}		
	}

}
