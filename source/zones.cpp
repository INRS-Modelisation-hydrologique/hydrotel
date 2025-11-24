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

#include "zones.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "mise_a_jour.hpp"
#include "projections.hpp"
#include "transforme_coordonnee.hpp"
#include "util.hpp"
#include "version.hpp"

#include <fstream>
#include <regex>
#include <sstream>
#include <set>


using namespace std;


namespace HYDROTEL 
{

	ZONES::ZONES()
	{
		_nom_fichier_zoneTemp = "";
		_pRasterUhrhId = nullptr;
		_bSaveUhrhCsvFile = false;
	}

	ZONES::~ZONES()
	{
		if(_pRasterUhrhId)
		{
			_pRasterUhrhId->Close();
			delete _pRasterUhrhId;
		}
	}

	std::string ZONES::PrendreNomFichierZone() const
	{
		return _nom_fichier_zone;
	}

	std::string ZONES::PrendreNomFichierAltitude() const
	{
		return _nom_fichier_altitude;
	}

	std::string ZONES::PrendreNomFichierPente() const
	{
		return _nom_fichier_pente;
	}

	std::string ZONES::PrendreNomFichierOrientation() const
	{
		return _nom_fichier_orientation;
	}

	size_t ZONES::PrendreNbZone() const
	{
		return _zones.size();
	}

	const PROJECTION& ZONES::PrendreProjection() const
	{
		return _grille.PrendreProjection();
	}

	const COORDONNEE& ZONES::PrendreCoordonnee() const
	{
		return _grille.PrendreCoordonnee();
	}

	float ZONES::PrendreResolution() const
	{
		return _grille.PrendreTailleCelluleX();
	}

	const RASTER<int>& ZONES::PrendreGrille() const
	{
		return _grille;
	}

	void ZONES::ChangeNomFichierZone(const std::string& nom_fichier)
	{
		//DetruireZones();
		_nom_fichier_zone = nom_fichier;
	}

	void ZONES::ChangeNomFichierAltitude(const std::string& nom_fichier)
	{
		//DetruireZones();
		_nom_fichier_altitude = nom_fichier;
	}

	void ZONES::ChangeNomFichierPente(const std::string& nom_fichier)
	{
		//DetruireZones();
		_nom_fichier_pente = nom_fichier;
	}

	void ZONES::ChangeNomFichierOrientation(const std::string& nom_fichier)
	{
		//DetruireZones();
		_nom_fichier_orientation = nom_fichier;
	}

	ZONE& ZONES::operator[] (size_t index)
	{
		BOOST_ASSERT(index < _zones.size());
		return *(_zones[index].get());
	}

	const ZONE& ZONES::operator[] (size_t index) const
	{
		BOOST_ASSERT(index < _zones.size());
		return *(_zones[index].get());
	}

	void ZONES::DetruireZones()
	{
		_zones.clear();
		_map.clear();
	}


	void ZONES::LectureZones()
	{
		size_t index, nbZone;
		int ident, identMax;

		if(_nom_fichier_zoneTemp != "")
		{
			_grille = LectureRaster_int(_nom_fichier_zoneTemp);
			_nom_fichier_zoneTemp = "";
		}
		else
			_grille = LectureRaster_int(_nom_fichier_zone);

		string nom_fichier_resumer_csv = RemplaceExtension(_nom_fichier_zone, "csv");
		string nom_fichier_resumer_rsm = RemplaceExtension(_nom_fichier_zone, "rsm");

		if (FichierExiste(nom_fichier_resumer_csv))
		{
			LectureResumerCsv(nom_fichier_resumer_csv);
		}
		else if (FichierExiste(nom_fichier_resumer_rsm))
		{
			LectureResumerRsm(nom_fichier_resumer_rsm);
		}
		else
		{
			CalculResumer();

			_bSaveUhrhCsvFile = true;						//
			//SauvegardeResumer(nom_fichier_resumer_csv);	//doit etre effectue apres la lecture des troncons (type zone)
		}

		//obtient l'identifiant maximum
		nbZone = _zones.size();

		identMax = 1;
		for(index=0; index!=nbZone; index++)
		{
			ident = _zones[index].get()->_identABS;
			if(ident > identMax)
				identMax = ident;
		}

		//
		_vIdentVersIndex.clear();
		_vIdentVersIndex.resize(identMax+1);

		for(index=0; index!=nbZone; index++)
		{
			ident = _zones[index].get()->_identABS;
			_vIdentVersIndex[ident] = index;
		}
	}

	void ZONES::SauvegardeResumer(const string& nom_fichier)
	{
		ofstream fichier(nom_fichier);
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "RESUMER ZONES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "UHRH ID; TYPE; ALTITUDE MOYENNE (m); PENTE MOYENNE (ratio); ORIENTATION MOYENNE; NB PIXEL; SUPERFICIE (km2); LONGITUDE; LATITUDE" << endl;

		for (auto iter = begin(_zones); iter != end(_zones); ++iter)
		{
			ZONE* zone = iter->get();

			fichier << zone->PrendreIdent() << ';';

			switch (zone->PrendreTypeZone())
			{
			case ZONE::SOUS_BASSIN:
				fichier << "SOUS-BASSIN;";
				break;

			case ZONE::LAC:
				fichier << "LAC;";
				break;

			default:
				throw ERREUR("type de zone non pris en compte");
			}

			fichier
				<< setprecision(6) << setiosflags(ios::fixed) << zone->PrendreAltitude() << ';'
				<< zone->PrendrePente() << ';'														//pente moyenne de l'uhrh [ratio]
				<< setprecision(0) << zone->PrendreOrientation() << ';'
				<< zone->PrendreNbPixel() << ';'
				<< setprecision(6) << zone->PrendreSuperficie() << ';'
				<< setprecision(5) << zone->PrendreCentroide().PrendreX() << ';'
				<< zone->PrendreCentroide().PrendreY() << endl;
		}
	}

	
	//création fichier uhrh.csv
	void ZONES::CalculResumer()
	{
		int grilleNoData;
		size_t grilleNbLigne, grilleNbCol;

		grilleNoData = _grille.PrendreNoData();
		grilleNbCol = _grille.PrendreNbColonne();
		grilleNbLigne = _grille.PrendreNbLigne();
		
		RASTER<float> altitudes = LectureRaster_float(_nom_fichier_altitude);

		if(grilleNbCol != altitudes.PrendreNbColonne() || grilleNbLigne != altitudes.PrendreNbLigne() || 
			_grille.PrendreCoordonnee().PrendreX() != altitudes.PrendreCoordonnee().PrendreX() || _grille.PrendreCoordonnee().PrendreY() != altitudes.PrendreCoordonnee().PrendreY())
		{
			throw ERREUR("Erreur 1; CalculResumer; incoherence entre matrice zone et altitude.");
		}
				
		//calcul des coordonnees centroide des uhrh 
		map<int, size_t> ident_pixels;
		map<int, size_t> somme_lig;
		map<int, size_t> somme_col;

		for (size_t lig = 0; lig < grilleNbLigne; ++lig)
		{
			for (size_t col = 0; col < grilleNbCol; ++col)
			{
				int ident = _grille(lig, col);
				if (ident != 0 && ident != grilleNoData)
				{
					++ident_pixels[ident];
					somme_lig[ident] += lig + 1;
					somme_col[ident] += col + 1;
				}
			}
		}

		size_t nb_zone = ident_pixels.size();
		vector<shared_ptr<ZONE>> zones(nb_zone);

		TRANSFORME_COORDONNEE trans(_grille.PrendreProjection(), PROJECTIONS::LONGLAT_WGS84());
		double resolution = static_cast<double>(_grille.PrendreTailleCelluleX());

		size_t index_zone = 0;

		double est = _grille.PrendreCoordonnee().PrendreX();
		double nord = _grille.PrendreCoordonnee().PrendreY();

		size_t resolutionX = static_cast<size_t>(_grille.PrendreTailleCelluleX());
		size_t resolutionY = static_cast<size_t>(_grille.PrendreTailleCelluleY());

		for (auto iter = begin(ident_pixels); iter != end(ident_pixels); ++iter)
		{
			auto zone = make_shared<ZONE>(ZONE());

			zone->ChangeIdent(iter->first);
			zone->ChangeNbPixel(iter->second);
			zone->ChangeSuperficie(resolution * resolution * iter->second / 1000000.0);

			//zone->ChangeTypeZone(ZONE::LAC);	//est fait dans le LectureTroncons

			int x = static_cast<int>(est + (resolutionX * (somme_col[iter->first] - (1.0 * iter->second / 2))) / iter->second); 
			int y = static_cast<int>(nord - (resolutionY * (somme_lig[iter->first] - (1.0 * iter->second / 2))) / iter->second);

			COORDONNEE coord = COORDONNEE(x, y);			
			zone->ChangeCentroide(trans.TransformeXY(coord));

			zones[index_zone++] = zone;
		}

		_zones.swap(zones);
		for (auto zone = begin(_zones); zone != end(_zones); ++zone)
		{
			_map[zone->get()->PrendreIdent()] = zone->get();
		}

		map<int, float> altitudes_moyenne;
		map<int, float> pentes_moyenne;

		int noData;

		// calcul la somme des altitudes par zone
		{
			noData = static_cast<int>(altitudes.PrendreNoData());

			for (size_t lig = 0; lig < grilleNbLigne; ++lig)
			{
				for (size_t col = 0; col < grilleNbCol; ++col)
				{
					int ident = _grille(lig, col);

					if (ident != grilleNoData)
					{
						float altitude = altitudes(lig, col);

						if (altitude != noData)
							altitudes_moyenne[ident] += altitude;
					}
				}
			}
		}

		// calcul la moyenne des pentes par zone [ratio]
		{
			RASTER<float> pentes = LectureRaster_float(_nom_fichier_pente, 1.0f / 1000.0f);	//convertie de pour mille à decimal

			if(grilleNbCol != pentes.PrendreNbColonne() || grilleNbLigne != pentes.PrendreNbLigne() || 
				_grille.PrendreCoordonnee().PrendreX() != pentes.PrendreCoordonnee().PrendreX() || _grille.PrendreCoordonnee().PrendreY() != pentes.PrendreCoordonnee().PrendreY())
			{
				throw ERREUR("Erreur 3; CalculResumer; incoherence entre matrice zone et pente.");
			}

			for (size_t lig = 0; lig < grilleNbLigne; ++lig)
			{
				for (size_t col = 0; col < grilleNbCol; ++col)
				{
					int ident = _grille(lig, col);

					if (ident != grilleNoData)
						pentes_moyenne[ident] += max(pentes(lig, col), 0.0025f);
				}
			}
		}

		// calcul la moyenne des altitudes et des pentes
		for (auto iter = begin(_zones); iter != end(_zones); ++iter)
		{
			ZONE* zone = iter->get();

			int ident = zone->PrendreIdent();
			size_t nb_pixel = zone->PrendreNbPixel();

			float altitude = altitudes_moyenne[ident] / nb_pixel;
			float pente = pentes_moyenne[ident] / nb_pixel;

			zone->ChangeAltitude(altitude);
			zone->ChangePente(pente);
		}

		// calcul de l'orientation moyenne

		//Break the aspect angle into measures of “Northness” and “Eastness.”  
		//You calculate these as the cosine and sine of the angles, respectively.
		//To calculate the mean, we need to sum up the cosine and sine values individually, 
		//then take the arctangent of their ratio.

		{
			RASTER<int> orientations = LectureRaster_int(_nom_fichier_orientation);

			if(grilleNbCol != orientations.PrendreNbColonne() || grilleNbLigne != orientations.PrendreNbLigne() || 
				_grille.PrendreCoordonnee().PrendreX() != orientations.PrendreCoordonnee().PrendreX() || _grille.PrendreCoordonnee().PrendreY() != orientations.PrendreCoordonnee().PrendreY())
			{
				throw ERREUR("Erreur 4; CalculResumer; incoherence entre matrice zone et orientation.");
			}

			const float orientationDefinition[8][2] = { 1.0f, 0.0f, sqrt(0.5f), sqrt(0.5f), 0.0f, 1.0f, -sqrt(0.5f), sqrt(0.5f), 
														-1.0f, 0.0f, -sqrt(0.5f), -sqrt(0.5f), 0.0f, -1.0f, sqrt(0.5f), -sqrt(0.5f) };

			map<int, pair<float, float>> somme_ori;

			for (size_t lig = 0; lig < grilleNbLigne; ++lig)
			{
				for (size_t col = 0; col < grilleNbCol; ++col)
				{
					int ident = _grille(lig, col);

					if (ident != grilleNoData)
					{
						int orientation = orientations(lig, col);

						if (orientation < 1 || orientation > 8)
						{
							ostringstream msg;
							msg << "orientation invalide " << orientation << " (lig " << lig + 1 << ", col " << col + 1
								<< ") dans le fichier \"" << _nom_fichier_orientation << '\"';
							throw ERREUR(msg.str());
						}

						somme_ori[ident].first += orientationDefinition[orientation - 1][0];
						somme_ori[ident].second += orientationDefinition[orientation - 1][1];
					}
				}
			}

			float tanOrientation[9] = { 0.0f, PI/4.0f, PI/2.0f, 3.0f*PI/4.0f, PI, 5.0f*PI/4.0f, 3.0f*PI/2.0f, 7.0f*PI/4.0f, 2.0f*PI };	

			vector<int> zones_nulle;

			for (auto iter = begin(_zones); iter != end(_zones); ++iter)
			{
				ZONE* zone = iter->get();

				int ident = zone->PrendreIdent();

				// calcul des orientations moyennes
				float ori;
				float valeurPlusProche;

				// cas particulier
				if (somme_ori[ident].first == 0 && somme_ori[ident].second > 0)
				{
					zone->ChangeOrientation(3);
				}
				else if (somme_ori[ident].first == 0 && somme_ori[ident].second < 0)
				{
					zone->ChangeOrientation(7);
				}
				else if (somme_ori[ident].first > 0 && somme_ori[ident].second == 0)
				{
					zone->ChangeOrientation(1);
				}
				else if (somme_ori[ident].first < 0 && somme_ori[ident].second == 0)
				{
					zone->ChangeOrientation(5);
				}
				else 
				{
					// cadrant
					if (somme_ori[ident].first > 0 && somme_ori[ident].second > 0)
					{
						// cadrant ++
						ori = atan(somme_ori[ident].second / somme_ori[ident].first);
						valeurPlusProche = abs(tanOrientation[0] - ori);
						zone->ChangeOrientation(1);

						if (abs(tanOrientation[1] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[1] - ori);
							zone->ChangeOrientation(2);
						}

						if (abs(tanOrientation[2] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[2] - ori);
							zone->ChangeOrientation(3);
						}
					}
					else if (somme_ori[ident].first < 0 && somme_ori[ident].second > 0)
					{
						// cadrant -+
						ori = PI + atan(somme_ori[ident].second / somme_ori[ident].first);

						valeurPlusProche = abs(tanOrientation[2] - ori);
						zone->ChangeOrientation(3);

						if (abs(tanOrientation[3] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[3] - ori);
							zone->ChangeOrientation(4);
						}

						if (abs(tanOrientation[4] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[4] - ori);
							zone->ChangeOrientation(5);
						}
					}
					else if (somme_ori[ident].first < 0 && somme_ori[ident].second < 0)
					{
						// cadrant --
						ori = PI + atan(somme_ori[ident].second / somme_ori[ident].first);

						valeurPlusProche = abs(tanOrientation[4] - ori);
						zone->ChangeOrientation(5);

						if (abs(tanOrientation[5] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[5] - ori);
							zone->ChangeOrientation(6);
						}

						if (abs(tanOrientation[6] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[6] - ori);
							zone->ChangeOrientation(7);
						}
					}
					else if (somme_ori[ident].first > 0 && somme_ori[ident].second < 0)
					{
						// cadrant +-
						ori = 2.0f * PI + atan(somme_ori[ident].second / somme_ori[ident].first);

						valeurPlusProche = abs(tanOrientation[6] - ori);
						zone->ChangeOrientation(7);

						if (abs(tanOrientation[7] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[7] - ori);
							zone->ChangeOrientation(8);
						}

						if (abs(tanOrientation[8] - ori) < valeurPlusProche)
						{
							valeurPlusProche = abs(tanOrientation[8] - ori);
							zone->ChangeOrientation(1);
						}
					}
				}

				if (zone->PrendreOrientation() == 0)
					zones_nulle.push_back(zone->PrendreIdent());
			}

			if (!zones_nulle.empty())
			{
				map<int, map<int, int>> total_pixels;

				for (size_t lig = 0; lig < grilleNbLigne; ++lig)
				{
					for (size_t col = 0; col < grilleNbCol; ++col)
					{
						int ident = _grille(lig, col);

						if (ident != grilleNoData)
						{
							int orientation = orientations(lig, col);
							total_pixels[ident][orientation]++;
						}
					}
				}

				for (auto iter = begin(zones_nulle); iter != end(zones_nulle); ++iter)
				{
					ZONE* zone = Recherche(*iter);

					int ori = 1;
					int max = total_pixels[zone->PrendreIdent()][ori];

					for (int o = 2; o <= 8; ++o)
					{
						if (total_pixels[zone->PrendreIdent()][o] > max)
						{
							ori = o;
							max = total_pixels[zone->PrendreIdent()][o];
						}
					}

					zone->ChangeOrientation(ori);
				}
			}
		}
	}


	void ZONES::LectureResumerCsv(const std::string& nom_fichier)
	{
		ifstream fichier(nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		fichier.exceptions(ios::failbit | ios::badbit);

		vector<shared_ptr<ZONE>> zones;

		int no_ligne = 1;

		try
		{
			string cle, valeur, ligne;
			lire_cle_valeur(fichier, cle, valeur);

			++no_ligne;

			if (cle != "RESUMER ZONES HYDROTEL VERSION")
				throw ERREUR_LECTURE_FICHIER(nom_fichier);

			getline_mod(fichier, ligne);
			++no_ligne;

			getline_mod(fichier, ligne); // commentaire
			++no_ligne;

			while (!fichier.eof())
			{
				int ident, orientation;
				char c;
				string type;
				double dSuperficie;
				float altitude, pente, longitude, latitude;
				size_t nb_pixel;

				fichier >> ident >> c;

				do
				{
					fichier.get(c);
					type+= c;
				} 
				while(c != ';');

				fichier	>> altitude >> c
					>> pente >> c
					>> orientation >> c
					>> nb_pixel >> c
					>> dSuperficie >> c
					>> longitude >> c
					>> latitude;

				if (pente <= 0.0f || orientation < 1 || orientation > 8 || nb_pixel <= 0 || dSuperficie <= 0.0)
					throw ERREUR_LECTURE_FICHIER(nom_fichier, no_ligne);

				auto zone = make_shared<ZONE>(ZONE());

				zone->ChangeIdent(ident);

				//type de zone originale
				if(type == "LAC;")
					zone->_type_zone_original = ZONE::TYPE_ZONE::LAC;
				//else par defaut a type SOUS_BASSIN dans constructeur ZONE

				//type de zone lu dans le fichier troncon.trl
				//important pour type barrage avec historique: meme si est un lac à l'origine, sera de type sous_bassin pour que les traitements dans onde_cinematique soient correcte

				//if (regex_match(type, regex("(.*)SOUS-BASSIN(.*)")))
				//	zone->ChangeTypeZone(ZONE::SOUS_BASSIN);
				//else if (regex_match(type, regex("(.*)LAC(.*)")))
				//	zone->ChangeTypeZone(ZONE::LAC);
				//else
				//	throw ERREUR_LECTURE_FICHIER(nom_fichier, no_ligne);

				zone->ChangeAltitude(altitude);
				zone->ChangePente(pente);
				zone->ChangeOrientation(orientation);
				zone->ChangeNbPixel(nb_pixel);
				zone->ChangeSuperficie(dSuperficie);
				zone->ChangeCentroide(COORDONNEE(longitude, latitude));	//wgs84 [decimal degree]

				zones.push_back(zone);

				++no_ligne;
			}
		}
		catch (...)
		{
			if (!fichier.eof())
			{
				fichier.close();
				throw ERREUR("Erreur ZONES::LectureResumerCsv.");
			}
		}

		fichier.close();

		zones.shrink_to_fit();
		_zones.swap(zones);

		for (auto zone = begin(_zones); zone != end(_zones); ++zone)
		{
			_map[zone->get()->PrendreIdent()] = zone->get();
		}
	}

	void ZONES::LectureResumerRsm(const std::string& nom_fichier)
	{
		DetruireZones();

		vector<string> vStr;
		istringstream iss;
		string str;
		int type, projection, zone, nlig, ncol, nbcarreaux, estG, nordG, res_x, res_y, nb_zones;

		type = 0;
		projection = 0;
		zone = 0;
		nlig = 0;
		ncol = 0;
		nbcarreaux = 0;
		estG = 0;
		nordG = 0;
		res_x = 0;
		res_y = 0;
		nb_zones = 0;

		ifstream fichier(nom_fichier);
		if(!fichier)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		try{

		fichier >> type;
		fichier >> projection;
		fichier >> zone;
		fichier >> nlig;
		fichier >> ncol;
		fichier >> nbcarreaux;
		fichier >> estG;
		fichier >> nordG;
		fichier >> res_x;
		fichier >> res_y;
		fichier >> nb_zones;

		vector<shared_ptr<ZONE>> zones(nb_zones);

		//obtient la projection du projet hydrotel 2.6
		PROJECTION projectionV26;
		string sString;

		string nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "CoordSys.txt");
		if(FichierExiste(nom_fichier_projection))
		{
			ifstream fich(nom_fichier_projection);

			getline_mod(fich, sString);
			fich.close();
			fich.clear();

			if(sString == "")
				throw ERREUR("Erreur; LectureResumerRsm;  erreur lors de la lecture du fichier CoordSys.txt; le fichier est vide.");
		
			projectionV26 = PROJECTION::ImportFromCoordSys(sString);
		}
		else
		{
			nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "proj4.txt");
			if(FichierExiste(nom_fichier_projection))
				projectionV26 = PROJECTION::ImportFromProj4File(nom_fichier_projection);
			else
			{
				nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "projection.prj");
				if(!FichierExiste(nom_fichier_projection))
					throw ERREUR("Erreur; LectureResumerRsm;  erreur lors de la lecture du fichier de projection (/physitel/ CoordSys.txt ou projection.prj).");
				else
					projectionV26 = PROJECTION::ImportFromPRJ(nom_fichier_projection);
			}
		}

		//si projection UTM; set la zone utm du projet, important si le bassin chevauche sur 2 zones utm
		sString = projectionV26.ExportWkt();
		if(sString.find("PROJECTION[\"Transverse_Mercator\"]") != string::npos)
		{
			if(projectionV26._spatial_reference.SetUTM(zone) != OGRERR_NONE)
				throw ERREUR("Erreur; LectureResumerRsm;  erreur lors de la lecture du fichier CoordSys.txt; zone utm.");
		}

		TRANSFORME_COORDONNEE trans(projectionV26, PROJECTIONS::LONGLAT_WGS84());

		int no_ligne = 6;

		for (int index = 0; index < nb_zones; ++index)
		{
			int nb_pixel;
			int id, ori, somcol, somlig;
			float alt, pte;

			id = 0;
			ori = 0;
			nb_pixel = 0;
			somcol = 0;
			somlig = 0;
			alt = 0.0f;
			pte = 0.0f;

			fichier >> id;
			fichier >> alt;
			fichier >> ori;
			fichier >> pte;
			fichier >> nb_pixel;
			fichier >> somcol;
			fichier >> somlig;

			// valider les donnees lues dans le fichier

			if (alt < 0.0f || pte <= 0.0f || ori < 1 || ori > 8 || nb_pixel <= 0)
			{
				ostringstream oss;
				oss.str("");
				oss << "Erreur de lecture fichier uhrh.rsm ligne " << no_ligne;
				if(alt < 0)
					oss << ". La valeur pour l'altitude (colonne 2) doit etre superieur ou egal a 0.";
				else
				{
					if(pte <= 0)
						oss << ". La valeur pour la pente (colonne 4) doit etre superieur a 0.";
					else
					{
						if(ori < 1)
							oss << ". La valeur pour l'orientation (colonne 3) doit etre superieur ou egal a 1 et inferieur ou egal a 8.";
						else
							oss << ". La valeur pour le nb de pixel (colonne 5) doit être superieur a 0.";
					}
				}

				throw ERREUR(oss.str());
			}

			auto zone2 = make_shared<ZONE>(ZONE());

			zone2->ChangeIdent(id);
			zone2->ChangeAltitude(alt);
			zone2->ChangeOrientation(ori);
			zone2->ChangePente(pte);
			zone2->ChangeNbPixel(nb_pixel);

			int est = static_cast<int>(estG + (res_x * (somcol - (1.0 * nb_pixel / 2))) / nb_pixel); 
			int nord = static_cast<int>(nordG - (res_y * (somlig - (1.0 * nb_pixel / 2))) / nb_pixel);

			zone2->ChangeCentroide(trans.TransformeXY(COORDONNEE(est, nord)));

			//les types de zone sont lu dans le fichier troncon.trl			
			//if (id < 0)
			//	zone2->ChangeTypeZone(ZONE::LAC);

			zones[index] = zone2;

			++no_ligne;
		}

		fichier.close();

		_zones.swap(zones);

		for(auto zone3 = begin(_zones); zone3 != end(_zones); ++zone3)
		{
			_map[zone3->get()->PrendreIdent()] = zone3->get();
		}

		}
		catch(const ERREUR& ex)
		{
			if(fichier && fichier.is_open())
				fichier.close();
			throw ex;
		}
		catch(...)
		{
			if(fichier && fichier.is_open())
				fichier.close();
			throw ERREUR("Error LectureResumerRsm: " + nom_fichier);
		}
	}


	ZONE* ZONES::Recherche(int ident)
	{
		return _map.find(ident) == _map.end() ? nullptr : _map[ident];
	}

	//size_t ZONES::IdentVersIndex(int ident) const
	//{
	//	for (size_t index = 0; index != _zones.size(); ++index)
	//	{
	//		if (_zones[index].get()->PrendreIdent() == ident)
	//			return index;
	//	}

	//	ostringstream msg;
	//	msg << "UHRH ID " << ident << " introuvable.";

	//	throw ERREUR( msg.str() );
	//}

	size_t ZONES::IdentVersIndex(int ident) const
	{
		return _vIdentVersIndex[abs(ident)];
	}

}
