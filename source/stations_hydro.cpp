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

#include "stations_hydro.hpp"

#include "erreur.hpp"
#include "projections.hpp"
#include "station_hydro_gibsi.hpp"
#include "util.hpp"

#include <fstream>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	STATIONS_HYDRO::STATIONS_HYDRO()
	{
	}


	STATIONS_HYDRO::~STATIONS_HYDRO()
	{
	}


	void STATIONS_HYDRO::Lecture(const PROJECTION& projection)
	{
		Detruire();

		if(PrendreNomFichier() != "")
		{
			string extension = PrendreExtension(PrendreNomFichier());

			if(extension == ".sth")
				LectureFormatSTH(projection);
			else
				throw ERREUR_LECTURE_FICHIER("Hydrological stations file (.sth): " + PrendreNomFichier());

			CreeMapRecherche();
		}
	}


	void STATIONS_HYDRO::LectureFormatSTH(const PROJECTION& projection)
	{
		if(_nom_fichier == "")
			return;

		string str;

		if(!FichierExiste(_nom_fichier))
		{
			str = "Error reading hydrological stations data: file not found: " + _nom_fichier;
			throw ERREUR(str);
		}

		ifstream fichier(_nom_fichier);
		if (!fichier)
		{
			str = "STATIONS_HYDRO; erreur lecture fichier; ";
			str+= _nom_fichier;
			throw ERREUR(str);
		}

		try
		{
			vector<string> sList;
			istringstream iss;
			string id, repertoire, ligne;
			size_t nb_station, n, nbLu;
			double x, y, z;
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
			iss >> type;					//type; 1=long/lat wgs84 //2=meme sys. coord. que le dem (ex: UTM)

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
				ligne = "STATIONS_HYDRO; erreur lecture fichier; ";
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
					ligne = "STATIONS_HYDRO; erreur lecture fichier; ";
					ligne+= _nom_fichier;
					ligne+= "; le nb de colonne est invalide.";
					throw ERREUR(ligne);
				}

				id = sList[0];

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

				iss.clear();
				iss.str(sList[3]);
				iss >> z;

				iss.clear();
				iss.str(sList[4]);
				iss >> format;

				switch (format)
				{
				case 3:	// FORMAT GIBSI

					str = id;
					ligne = id;
					boost::algorithm::to_lower(str);
					if(str.size() < 4 || str.substr(str.size()-4) != ".hyd")
						ligne.append(".hyd");

					ligne = Combine(PrendreRepertoire(_nom_fichier), ligne);
					stations[n] = make_shared<STATION_HYDRO_GIBSI>(ligne);
					break;

				default:
					ligne = "STATIONS_HYDRO; erreur lecture fichier; ";
					ligne+= _nom_fichier;
					ligne+= "; numero de format invalide.";
					throw ERREUR(ligne);
				}

				stations[n].get()->ChangeIdent(id);
				stations[n].get()->ChangeNom(id);
				stations[n].get()->ChangeCoordonnee(COORDONNEE(x, y, z));

				++nbLu;
			}

			fichier.close();

			if(nbLu != nb_station)
            {
                ligne = "STATIONS_HYDRO; erreur lecture fichier; ";
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
			str = "STATIONS_HYDRO; erreur lecture fichier; ";
			str+= _nom_fichier;
			throw ERREUR(str);
		}
	}


}
