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

#include "grille_prevision.hpp"

#include "gdal_util.hpp"
#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "sim_hyd.hpp"
#include "version.hpp"

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	GRILLE_PREVISION::GRILLE_PREVISION()
	{
	}

	GRILLE_PREVISION::~GRILLE_PREVISION()
	{
	}

	void GRILLE_PREVISION::Initialise()
	{
		if (!LecturePonderation())
		{
			CalculePonderation();
			SauvegardePonderation();
		}

		if (!LectureAltitude())
		{
			CalculeAltitude();
			SauvegardeAltitude();
		}
	}

	void GRILLE_PREVISION::Calcule()
	{
		RepartieDonnees();
		
		if(_bUseTotalPrecip)
			PassagePluieNeige();
	}

	void GRILLE_PREVISION::LectureParametres()
	{
		vector<std::string> sList;
		string sString, sString2;
		size_t index_zone, x;
		int iIdent;

		_bUseTotalPrecip = false;

		if(_sim_hyd->_bSimulePrevision)
		{
			ifstream fichier(_sPathFichierParam);
			if (!fichier)
				throw ERREUR("GRILLE_PREVISION; erreur ouverture FICHIER GRILLE_PREVISION (.grp)");

			try{

			getline_mod(fichier, sString);	//2; type de coordonnee	LONGLAT (1) ou UTM (2)
			getline_mod(fichier, sString);	//2; unite de mesure des donnees (donnees nivometrique seulement) //les données sont en mm pour les grilles météo
			getline_mod(fichier, sString);	//1; indique si les donnees sont obtenues au debut ou a la fin du pas de temps; 1 (début du pas de temps), 2 (fin du pas de temps)
			getline_mod(fichier, sString);	//commentaire

			getline_mod(fichier, sString);	//22; frequence, type de donnees / emplacement des fichiers de grilles

			SplitString(sList, sString, " ", true, false);

			if(sList[0] != "22")
				throw ERREUR("GRILLE_PREVISION; le format [" + sList[0] + "] specifie pour les grilles de donnees est invalide; les grilles de donnees doivent etre de type [22] (fichier local 24h).");

			_sPathFichierGrille = "";
			for(x=2; x<sList.size(); x++)			//path des fichier de grille
			{
				_sPathFichierGrille+= sList[x];		//s'il y a des espace dans le path, il faut concatener la string
				if(x < sList.size()-1)
					_sPathFichierGrille+= " ";
			}

			if (!Racine(_sPathFichierGrille) )
				_sPathFichierGrille = Combine(_sim_hyd->PrendreRepertoireProjet(), _sPathFichierGrille);

			if(!RepertoireExiste(_sPathFichierGrille))
				throw ERREUR("GRILLE_PREVISION; le dossier specifie pour les grilles de donnees est invalide; " + _sPathFichierGrille);

			getline_mod(fichier, _sPrefixeNomFichier);	//prefixe du nom des fichiers

			if(_sPrefixeNomFichier == "@")
				_sPrefixeNomFichier = "";	//il ny a pas de suffixe

			getline_mod(fichier, sString);		//4; nombre de type de donnees à lire 
			if(sString != "4")
				throw ERREUR("GRILLE_PREVISION; le nombre de fichier de donnees disponible est invalide. Celui-ci doit etre egal a 4; pluie, neige, tmax, tmin.");
		
			//extension pour chaque type de donnees
			//.pluie
			//.neige
			//.tmax
			//.tmin
			getline_mod(fichier, sString);
			getline_mod(fichier, sString);
			getline_mod(fichier, sString);
			getline_mod(fichier, sString);

			try{
				getline_mod(fichier, sString);
				if(sString == "1")
					_bUseTotalPrecip = true;	//utilisation des fichiers de precip totale
			}
			catch(...)
			{
			}
		
			fichier.close();
			}
			catch(const ERREUR& err)
			{
				fichier.close();
				throw err;
			}
			catch(...)
			{
				fichier.close();
				throw ERREUR("GRILLE_PREVISION; erreur lecture FICHIER GRILLE_PREVISION (.grp)");
			}
		}

		//lecture des gradients (fichier 'grilleprevision.csv')
		_gradient_precipitations.resize(_sim_hyd->PrendreZones().PrendreNbZone(), 0.0f);
		_gradient_temperature.resize(_sim_hyd->PrendreZones().PrendreNbZone(), 0.0f);
		_passage_pluie_neige.resize(_sim_hyd->PrendreZones().PrendreNbZone(), 0.0f);

		if(_sim_hyd->_fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		string sPathParamGrille;
		sPathParamGrille = Combine(_sim_hyd->PrendreRepertoireSimulation(), "grilleprevision.csv");

		if(FichierExiste(sPathParamGrille))
		{
			ZONES& zones = _sim_hyd->PrendreZones();

			ifstream fichier(sPathParamGrille);
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES grilleprevision.csv");

			string cle, valeur, ligne;
			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES grilleprevision.csv", 1);

			getline_mod(fichier, ligne);				//ligne vide
			lire_cle_valeur(fichier, cle, valeur);	//nom du sous modele
				
			getline_mod(fichier, ligne);				//ligne vide
			getline_mod(fichier, ligne);				//commentaire

			for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
			{
				getline_mod(fichier, ligne);
				auto vValeur = extrait_fvaleur(ligne, ";");

				if(vValeur.size() != 3 && vValeur.size() != 4)
				{
					fichier.close();
					throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES grilleprevision.csv; nombre de colonne invalide.");
				}

				iIdent = static_cast<int>(vValeur[0]);
				index_zone = zones.IdentVersIndex(iIdent);

				_gradient_temperature[index_zone] = vValeur[1];
				_gradient_precipitations[index_zone] = vValeur[2];

				if(vValeur.size() == 4)
					_passage_pluie_neige[index_zone] = vValeur[3];
				else
					_passage_pluie_neige[index_zone] = 1.0f;
			}
		}
		else
			SauvegardeParametres();
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool GRILLE_PREVISION::LecturePonderation()
	{
		vector<string> sList;
		istringstream iss;
		string nom_fichier, sString;
		double dTemp;
		size_t index, compteur;
		int iID, iTemp;

		_mapPonderation.clear();
		
		nom_fichier = Combine(_sim_hyd->PrendreRepertoireProjet(), "meteo/");
		if(_sPrefixeNomFichier != "")
			nom_fichier+= _sPrefixeNomFichier + ".pgp";
		else
			nom_fichier+= "ponderationgrilleprevision.pgp";

		if (!FichierExiste(nom_fichier))
			return false;

		ifstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR("GRILLE_PREVISION; LecturePonderation; erreur ouverture fichier de ponderation .pgp");

		index = 0;

		try{

		getline_mod(fichier, sString);	//ignore la 1iere ligne
		while (!fichier.eof())
		{
			getline_mod(fichier, sString);
			if(sString != "")
			{
				SplitString(sList, sString, " ", true, true);

				iID = _sim_hyd->PrendreZones()[index].PrendreIdent();	//id uhrh
			
				compteur = 1;
				while(compteur < sList.size() - 1)
				{
					iss.clear();
					iss.str(sList[compteur]);
					iss >> iTemp;	//zero-based index du pixel de la grille meteo

					iss.clear();
					iss.str(sList[compteur+1]);
					iss >> dTemp;	//valeur de ponderation

					_mapPonderation[iID][iTemp] = dTemp;
					compteur+= 2;
				}

				++index;
			}
		}
		}
		catch(...)
		{
			if(!fichier.eof())
			{
				fichier.close();
				throw ERREUR("GRILLE_PREVISION; LecturePonderation; erreur lecture fichier de ponderation .pgp");
			}
		}

		fichier.close();
		if(index != _sim_hyd->PrendreZones().PrendreNbZone())
			throw ERREUR("GRILLE_PREVISION; LecturePonderation; erreur lecture fichier de ponderation .pgp; nb de uhrh invalide");

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void GRILLE_PREVISION::CalculePonderation()
	{
		string sPathTarget, sString;

		_mapPonderation.clear();

		//trouve le path du 1er fichier .tmin disponible
		sPathTarget = _sPathFichierGrille;
		boost::filesystem::path p(sPathTarget);
		sPathTarget = "";

		for (auto iter = boost::filesystem::directory_iterator(p); iter != boost::filesystem::directory_iterator(); ++iter)
		{
			boost::filesystem::path fp = iter->path().extension();
			sString = fp.string();
			boost::algorithm::to_lower(sString);
			if(sString == ".tmin")
			{
				sPathTarget = iter->path().string();
				std::replace(sPathTarget.begin(), sPathTarget.end(), '\\', '/');
				break;
			}
		}

		if(sPathTarget == "")
			throw ERREUR("GRILLE_PREVISION; CalculePonderation; erreur ouverture fichier GRILLE_PREVISION; aucun fichier disponible; " + _sPathFichierGrille);

		const RASTER<int>&	uhrh = _sim_hyd->PrendreZones().PrendreGrille();

		double geotransform[6] = { 0 };

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(sPathTarget.c_str(), GA_ReadOnly));
		if(dataset == nullptr)
			throw ERREUR("GRILLE_PREVISION; CalculePonderation; erreur ouverture fichier GRILLE_PREVISION; " + sPathTarget);

		dataset->GetGeoTransform(geotransform);

		double dXSource, dYSource, dXTarget, dYTarget, dX, dY, dSourceHalfPixelWidth, dSourcePixelWidth, dTargetPixelWidth;
		int row, col, nbRow, nbCol, targetRow, targetCol, index, iVal, iSourceNoData, iTargetNbCol;

		std::map<int, double> mapNbPixel;

		dXSource = uhrh.PrendreCoordonnee().PrendreX();	//upper left corner x
		dYSource = uhrh.PrendreCoordonnee().PrendreY();	//upper left corner y
		dSourcePixelWidth = static_cast<double>(uhrh.PrendreTailleCelluleX());
		dSourceHalfPixelWidth = dSourcePixelWidth / 2.0;
		iSourceNoData = uhrh.PrendreNoData();

		dXTarget = geotransform[0];	//upper left corner x
		dYTarget = geotransform[3];	//upper left corner y
		iTargetNbCol = dataset->GetRasterXSize();
		dTargetPixelWidth = geotransform[1];

		GDALClose((GDALDatasetH)dataset);

		//pixel (0, 0) is upper left pixel

		//lecture des pixel du grid uhrh
		nbRow = static_cast<int>(uhrh.PrendreNbLigne());
		nbCol = static_cast<int>(uhrh.PrendreNbColonne());

		for(row = 0; row < nbRow; row++)
		{
			for (col = 0; col < nbCol; col++)
			{
				iVal = uhrh(row, col);
				if(iVal != iSourceNoData)
				{
					dY = dYSource - row * dSourcePixelWidth - dSourceHalfPixelWidth;
					dX = col * dSourcePixelWidth + dXSource + dSourceHalfPixelWidth;	//get current pixel centroid coordinate					

					targetRow = static_cast<int>((dYTarget - dY) / dTargetPixelWidth);	//zero-based
					targetCol = static_cast<int>((dX - dXTarget) / dTargetPixelWidth);

					index = targetRow * iTargetNbCol + targetCol;	//zero-based

					++_mapPonderation[iVal][index];	//aditionne le nb pixel pour l'index (no pixel couche target) en cours
					++mapNbPixel[iVal];
				}
			}
		}

		//calcule des ponderation
		for (auto iter1 = begin(_mapPonderation); iter1 != end(_mapPonderation); iter1++)
		{
			for (auto iter2 = begin(iter1->second); iter2 != end(iter1->second); iter2++)
				iter2->second/= mapNbPixel[iter1->first];
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void GRILLE_PREVISION::SauvegardePonderation()
	{
		ostringstream oss;
		string sString;
		int iCompteur;

		sString = Combine(_sim_hyd->PrendreRepertoireProjet(), "meteo/");
		
		if(_sPrefixeNomFichier != "")
			sString+= _sPrefixeNomFichier + ".pgp";
		else
			sString+= "ponderationgrilleprevision.pgp";

		ofstream fichier(sString);

		fichier << endl;	//ligne vide

		iCompteur = 0;
		for (auto iter1 = begin(_mapPonderation); iter1 != end(_mapPonderation); iter1++)
		{
			oss.str("");
			oss << setw(6) << iCompteur << " ";

			auto iter2 = begin(iter1->second);
			oss << setw(6) << iter2->first << " " << setw(10) << setprecision(8) << setiosflags(ios::fixed) << iter2->second << " ";
			iter2++;

			while(iter2 != end(iter1->second))
			{
				oss << setw(5) << iter2->first << " " << setw(10) << setprecision(8) << setiosflags(ios::fixed) << iter2->second << " ";
				iter2++;
			}

			oss << "-1 " << endl;
			fichier << oss.str();
			++iCompteur;
		}

		fichier.close();
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool GRILLE_PREVISION::LectureAltitude()
	{
		vector<string> sList;
		istringstream iss;
		string nom_fichier, sString;
		int iTemp;

		_mapAltitudes.clear();
		
		nom_fichier = Combine(_sim_hyd->PrendreRepertoireProjet(), "meteo/");
		if(_sPrefixeNomFichier != "")
			nom_fichier+= _sPrefixeNomFichier + ".agp";
		else
			nom_fichier+= "altitudegrilleprevision.agp";

		if (!FichierExiste(nom_fichier))
			return false;

		ifstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR("GRILLE_PREVISION; LectureAltitude; erreur ouverture fichier des altitudes .agp");

		try{

		getline_mod(fichier, sString);	//ignore la 1ere ligne
		while (!fichier.eof())
		{
			getline_mod(fichier, sString);
			if(sString != "")
			{
				SplitString(sList, sString, " ", true, true);

				if(sList.size() != 2)
				{
					fichier.close();
					throw ERREUR("GRILLE_PREVISION; LectureAltitude; erreur lecture fichier des altitudes .agp; nombre de colonne invalide.");
				}

				iss.clear();
				iss.str(sList[0]);
				iss >> iTemp;	//zero-based index du pixel de la grille meteo

				iss.clear();
				iss.str(sList[1]);
				iss >> _mapAltitudes[iTemp];	//altitude du pixel de la grille meteo
			}
		}
		}
		catch(...)
		{
			if(!fichier.eof())
			{
				fichier.close();
				throw ERREUR("GRILLE_PREVISION; LectureAltitude; erreur lecture fichier des altitudes .agp");
			}
		}

		fichier.close();
		return true;
	}
	
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Calcul l'altitude pour chaque pixel de grille
	void GRILLE_PREVISION::CalculeAltitude()
	{
		string sPathTarget, sString;
		float fVal;

		_mapAltitudes.clear();

		//trouve le path du 1er fichier .tmin disponible
		sPathTarget = _sPathFichierGrille;
		boost::filesystem::path p(sPathTarget);
		sPathTarget = "";

		for (auto iter = boost::filesystem::directory_iterator(p); iter != boost::filesystem::directory_iterator(); ++iter)
		{
			boost::filesystem::path fp = iter->path().extension();
			sString = fp.string();
			boost::algorithm::to_lower(sString);
			if(sString == ".tmin")
			{
				sPathTarget = iter->path().string();
				std::replace(sPathTarget.begin(), sPathTarget.end(), '\\', '/');
				break;
			}
		}

		if(sPathTarget == "")
			throw ERREUR("GRILLE_PREVISION; CalculeAltitudeGrille; erreur ouverture fichier GRILLE_PREVISION; aucun fichier disponible; " + _sPathFichierGrille);	

		RASTER<float> alt = LectureRaster_float(_sim_hyd->PrendreZones().PrendreNomFichierAltitude());

		double geotransform[6] = { 0 };

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(sPathTarget.c_str(), GA_ReadOnly));
		if(dataset == nullptr)
			throw ERREUR("GRILLE_PREVISION; CalculeAltitudeGrille; erreur ouverture fichier GRILLE_PREVISION; " + sPathTarget);

		dataset->GetGeoTransform(geotransform);

		double dXSource, dYSource, dXTarget, dYTarget, dX, dY, dSourceHalfPixelWidth, dSourcePixelWidth, dTargetPixelWidth;
		float fSourceNoData;
		int row, col, nbRow, nbCol, targetRow, targetCol, index, iTargetNbCol;

		std::map<int, float> mapNbPixel;

		dXSource = alt.PrendreCoordonnee().PrendreX();	//upper left corner x
		dYSource = alt.PrendreCoordonnee().PrendreY();	//upper left corner y
		dSourcePixelWidth = static_cast<double>(alt.PrendreTailleCelluleX());
		dSourceHalfPixelWidth = dSourcePixelWidth / 2.0;
		fSourceNoData = alt.PrendreNoData();

		dXTarget = geotransform[0];	//upper left corner x
		dYTarget = geotransform[3];	//upper left corner y
		iTargetNbCol = dataset->GetRasterXSize();
		dTargetPixelWidth = geotransform[1];

		GDALClose((GDALDatasetH)dataset);

		//pixel (0, 0) is upper left pixel

		//lecture des pixel du grid uhrh
		nbRow = static_cast<int>(alt.PrendreNbLigne());
		nbCol = static_cast<int>(alt.PrendreNbColonne());

		for(row = 0; row < nbRow; row++)
		{
			for (col = 0; col < nbCol; col++)
			{
				fVal = alt(row, col);
				if(fVal != fSourceNoData)
				{
					dY = dYSource - row * dSourcePixelWidth - dSourceHalfPixelWidth;
					dX = col * dSourcePixelWidth + dXSource + dSourceHalfPixelWidth;	//get current pixel centroid coordinate					

					targetRow = static_cast<int>((dYTarget - dY) / dTargetPixelWidth);	//zero-based
					targetCol = static_cast<int>((dX - dXTarget) / dTargetPixelWidth);

					index = targetRow * iTargetNbCol + targetCol;	//zero-based

					_mapAltitudes[index]+= fVal;	//aditionne l'altitude pour calculer la moyenne
					++mapNbPixel[index];			//aditionne le nb pixel pour l'index (no pixel couche target) en cours
				}
			}
		}

		//calcule la moyenne (moyenne des pixels altidude pour chaque pixel grille
		for(auto iter1 = begin(_mapAltitudes); iter1 != end(_mapAltitudes); iter1++)
			iter1->second/= mapNbPixel[iter1->first];
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void GRILLE_PREVISION::SauvegardeAltitude()
	{
		ostringstream oss;
		string sString;

		sString = Combine(_sim_hyd->PrendreRepertoireProjet(), "meteo/");
		
		if(_sPrefixeNomFichier != "")
			sString+= _sPrefixeNomFichier + ".agp";
		else
			sString+= "altitudegrilleprevision.agp";

		ofstream fichier(sString);

		fichier << endl;	//ligne vide

		for (auto iter1 = begin(_mapAltitudes); iter1 != end(_mapAltitudes); iter1++)
		{
			oss.str("");
			oss << iter1->first << " " << setprecision(2) << setiosflags(ios::fixed) << iter1->second << endl;

			fichier << oss.str();
		}

		fichier.close();
	}

		
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void GRILLE_PREVISION::RepartieDonnees()
	{
		string sString, sString2;
		int iIdent, indexcell;

		ZONES& zones = _sim_hyd->PrendreZones();

		vector<size_t> index_zones = _sim_hyd->PrendreZonesSimules();

		size_t index_zone, row, col, nbCol;
		float tmin, tmax, pluie, neige, tmin_jour, tmax_jour, fAltitude, fDiffAlt, fDensiteNeige;

		vector<RASTER<float>> grilleTMin;
		vector<RASTER<float>> grilleTMax;

		vector<RASTER<float>> grillePluie;
		vector<RASTER<float>> grilleNeige;

		vector<RASTER<float>> grillePrecipTot;
		
		//if(_sim_hyd.PrendrePasDeTemps() == 24)

		FormatePathFichierGrilleCourant(sString);

		try{

		if(_bUseTotalPrecip)
		{
			sString2 = sString + ".prcp";
			grillePrecipTot.push_back(ReadGeoTIFF_float(sString2));
		}
		else
		{
			sString2 = sString + ".pluie";
			grillePluie.push_back(ReadGeoTIFF_float(sString2));

			sString2 = sString + ".neige";
			grilleNeige.push_back(ReadGeoTIFF_float(sString2));
		}

		sString2 = sString + ".tmin";
		grilleTMin.push_back(ReadGeoTIFF_float(sString2));

		sString2 = sString + ".tmax";
		grilleTMax.push_back(ReadGeoTIFF_float(sString2));
		
		}
		catch(...)
		{
			throw ERREUR("GRILLE_PREVISION; erreur de lecture du fichier; " + sString2);
		}

		nbCol = grilleTMin[0].PrendreNbColonne();

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];
			iIdent = zone.PrendreIdent();
			fAltitude = zone.PrendreAltitude();

			tmin = tmax = pluie = neige = tmin_jour = tmax_jour = 0.0f;

			for (auto iter2 = begin(_mapPonderation[iIdent]); iter2 != end(_mapPonderation[iIdent]); iter2++)
			{
				indexcell = iter2->first;
				row = static_cast<size_t>(indexcell / nbCol);
				col = indexcell % nbCol;

				fDiffAlt = fAltitude - static_cast<float>(_mapAltitudes[indexcell]);

				if(grilleTMin[0](row, col) > VALEUR_MANQUANTE)
					tmin+= (grilleTMin[0](row, col) + _gradient_temperature[index_zone] * fDiffAlt / 100.0f) * static_cast<float>(iter2->second);	//iter2->second -> ponderation

				if(grilleTMax[0](row, col) > VALEUR_MANQUANTE)
					tmax+= (grilleTMax[0](row, col) + _gradient_temperature[index_zone] * fDiffAlt / 100.0f) * static_cast<float>(iter2->second);	//iter2->second -> ponderation

				if(_bUseTotalPrecip)
				{
					if(grillePrecipTot[0](row, col) > VALEUR_MANQUANTE)
						pluie+= (grillePrecipTot[0](row, col) * static_cast<float>(iter2->second) * (1.0f + _gradient_precipitations[index_zone] / 100.0f * fDiffAlt));	//iter2->second -> ponderation
				}
				else
				{
					if(grillePluie[0](row, col) > VALEUR_MANQUANTE)
						pluie+= (grillePluie[0](row, col) * static_cast<float>(iter2->second) * (1.0f + _gradient_precipitations[index_zone] / 100.0f * fDiffAlt));	//iter2->second -> ponderation

					if(grilleNeige[0](row, col) > VALEUR_MANQUANTE)
						neige+= (grilleNeige[0](row, col) * static_cast<float>(iter2->second) * (1.0f + _gradient_precipitations[index_zone] / 100.0f * fDiffAlt));	//iter2->second -> ponderation
				}

				//if (tmin_jour_station > VALEUR_MANQUANTE)
				//	tmin_jour+= (tmin_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;

				//if (tmax_jour_station > VALEUR_MANQUANTE)
				//	tmax_jour+= (tmax_jour_station + PrendreGradientTemperature(index_zone) * diff_alt / 100.0f) * ponderation;
			}

			zone.ChangeTemperature(tmin, tmax);
			
			//zone.ChangeTemperatureJournaliere(tmin_jour, tmax_jour);
			zone.ChangeTemperatureJournaliere(tmin, tmax);

			if(pluie > 0.0f)
				zone.ChangePluie(pluie);
			else
				zone.ChangePluie(0.0f);

			//equivalent en eau de la neige [mm] -> hauteur de precipitation en mm de neige
			//if (pas_de_temps == 1)
			//	densite_neige = CalculDensiteNeige(zones[index].PrendreTMin()) / DENSITE_EAU;
			//else
			//{
				fDensiteNeige = CalculDensiteNeige((tmax + tmin) / 2.0f) / DENSITE_EAU;
			//}

			if(neige > 0.0f)
				zone.ChangeNeige(neige / fDensiteNeige);
			else
				zone.ChangeNeige(0.0f);
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void GRILLE_PREVISION::FormatePathFichierGrilleCourant(string& sPath)
	{
		ostringstream oss;

		sPath = _sPathFichierGrille + "/" + _sPrefixeNomFichier;
		oss << _sim_hyd->PrendreDateCourante().PrendreAnnee() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd->PrendreDateCourante().PrendreMois() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd->PrendreDateCourante().PrendreJour() << "_";

		//if(_sim_hyd.PrendrePasDeTemps() == 24)
			oss << "24h";

		sPath+= oss.str();
	}


	void GRILLE_PREVISION::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd->PrendreZones();

		ifstream fichier( _sim_hyd->_nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( _sim_hyd->_nomFichierParametresGlobal );

		bool bOK = false;

		try{

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( _sim_hyd->_nomFichierParametresGlobal, 1);

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd->PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "GRILLE PREVISION")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 3 && vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd->_nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide GRILLE_PREVISION.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd->_nomFichierParametresGlobal, no_ligne, "ID de groupe invalide GRILLE_PREVISION. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd->PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd->PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_gradient_temperature[index_zone] = vValeur[1];
						_gradient_precipitations[index_zone] = vValeur[2];

						if(vValeur.size() == 4)
							_passage_pluie_neige[index_zone] = vValeur[3];
						else
							_passage_pluie_neige[index_zone] = 1.0f;
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
			throw ERREUR_LECTURE_FICHIER(_sim_hyd->_nomFichierParametresGlobal + "; GRILLE_PREVISION");
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd->_nomFichierParametresGlobal, 0, "Parametres sous-modele GRILLE_PREVISION");
	}


	void GRILLE_PREVISION::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd->PrendreZones();

		string nom_fichier = Combine(_sim_hyd->PrendreRepertoireSimulation(), "grilleprevision.csv");
		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << "GRILLE PREVISION" << endl;
		fichier << endl;

		fichier << "UHRH ID;GRADIENT TEMPERATURE(C/100m);GRADIENT PRECIPITATION(mm/100m);" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << _gradient_temperature[index] << ';';
			fichier << _gradient_precipitations[index] << ';';
			fichier << _passage_pluie_neige[index] << endl;
		}
	}


	void GRILLE_PREVISION::PassagePluieNeige()
	{
		ZONES& zones = _sim_hyd->PrendreZones();
		unsigned short pas_de_temps = _sim_hyd->PrendrePasDeTemps();
		
		vector<size_t> index_zones = _sim_hyd->PrendreZonesSimules();

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			float pluie = zone.PrendrePluie();
			float neige = zone.PrendreNeige();

			const float tmin = zone.PrendreTMin();
			const float tmax = zone.PrendreTMax();

			const float cst_pluie_en_neige = _passage_pluie_neige[index_zone];

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
					neige+= pluie / densite_neige;	//equivalent en eau de la neige [mm] -> hauteur de precipitation en mm de neige
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
				neige = (1.0f - taux_transformation) * precipitation / densite_neige;	//equivalent en eau de la neige [mm] -> hauteur de precipitation en mm de neige
			}

			zone.ChangePluie(pluie);
			zone.ChangeNeige(neige);
		}		
	}

}
