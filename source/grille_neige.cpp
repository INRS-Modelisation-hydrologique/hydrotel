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

#include "grille_neige.hpp"

#include "gdal_util.hpp"
#include "erreur.hpp"
#include "util.hpp"

#include <algorithm>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	GRILLE_NEIGE::GRILLE_NEIGE()
	{
	}

	GRILLE_NEIGE::~GRILLE_NEIGE()
	{
	}

	void GRILLE_NEIGE::Initialise()
	{
		LectureParametres();

		if (!LecturePonderation())
		{
			CalculePonderation();
			SauvegardePonderation();
		}
	}

	void GRILLE_NEIGE::LectureParametres()
	{
		vector<std::string> sList;
		string sString, sString2;
		size_t x;

		ifstream fichier(_sPathFichierParam);
		if (!fichier)
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; erreur lors de l ouverture du fichier [NOM FICHIER GRILLE NEIGE] (.grn)");

		try{

		getline_mod(fichier, sString);	//2; type de coordonnee	LONGLAT (1) ou UTM (2)
		
		getline_mod(fichier, sString);	//3; unite de mesure des donnees (donnees nivometrique seulement) 1=m, 2=cm, 3=mm
		if(sString == "1")
			_facteurMultiplicatifDonnees = 1.0f;
		else
		{
			if(sString == "2")
				_facteurMultiplicatifDonnees = 0.01f;
			else
			{
				if(sString == "3")
					_facteurMultiplicatifDonnees = 0.001f;
				else
				{
					fichier.close();
					throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; erreur lors de la lecture du fichier [NOM FICHIER GRILLE NEIGE] (.grn); ligne 2; unite invalide (1=m, 2=cm, 3=mm).");
				}
			}
		}
			
		getline_mod(fichier, sString);	//1; indique si les donnees sont obtenues au debut ou a la fin du pas de temps; 1 (début du pas de temps), 2 (fin du pas de temps)
		getline_mod(fichier, sString);	//commentaire

		getline_mod(fichier, sString);	//frequence, type de donnees / emplacement des fichiers de grilles
		SplitString(sList, sString, " ", true, false);

		if(sList[0] != "32")
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; le format [" + sList[0] + "] specifie pour les grilles de donnees est invalide; les grilles de donnees doivent etre de type [32] (fichier local 24h).");

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
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; le dossier specifie pour les grilles de donnees est invalide; " + _sPathFichierGrille);

		getline_mod(fichier, _sPrefixeNomFichier);	//prefixe du nom des fichiers

		if(_sPrefixeNomFichier == "@")
			_sPrefixeNomFichier = "";	//il ny a pas de suffixe

		getline_mod(fichier, sString);		//3; nombre de type de donnees à lire
		if(sString != "2" && sString != "3")
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; le nombre de fichier de donnees disponible est invalide. Celui-ci doit etre au minimum de 2; een, hau.");
		
		//lignes suivantes = extension pour chaque type de donnees
		//.een
		//.hau
		//.occ

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
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; erreur lors de la lecture du fichier [NOM FICHIER GRILLE NEIGE] (.grn)");
		}
	}


	void GRILLE_NEIGE::SauvegardeParametres()
	{
	//	ZONES& zones = _sim_hyd.PrendreZones();

	//	string nom_fichier = PrendreNomFichierParametres();

	//	ofstream fichier(nom_fichier);

	//	if (!fichier)
	//		throw ERREUR_ECRITURE_FICHIER(nom_fichier);

	//	fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
	//	fichier << endl;

	//	fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
	//	fichier << endl;

	//	fichier << "UHRH ID;GRADIENT TEMPERATURE(C/100m);GRADIENT PRECIPITATION(mm/100m);PASSAGE PLUIE NEIGE(C);" << endl;
	//	for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
	//	{
	//		fichier << zones[index].PrendreIdent() << ';';
	//		fichier << PrendreGradientTemperature(index) << ';';
	//		fichier << PrendreGradientPrecipitation(index) << ';';
	//		fichier << PrendrePassagePluieNeige(index) << endl;
	//	}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void GRILLE_NEIGE::SauvegardePonderation()
	{
		ostringstream oss;
		string sString;
		int iCompteur;

		sString = Combine(_sim_hyd->PrendreRepertoireProjet(), "meteo/");
		
		if(_sPrefixeNomFichier != "")
			sString+= _sPrefixeNomFichier + ".pgn";
		else
			sString+= "ponderationgrilleneige.pgn";

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
	void GRILLE_NEIGE::CalculePonderation()
	{
		ostringstream oss;
		string sPathTarget, sString;

		//trouve le path du 1er fichier .een disponible
		sPathTarget = _sPathFichierGrille;
		boost::filesystem::path p(sPathTarget);
		sPathTarget = "";

		for (auto iter = boost::filesystem::directory_iterator(p); iter != boost::filesystem::directory_iterator(); ++iter)
		{
			boost::filesystem::path fp = iter->path().extension();
			sString = fp.string();
			boost::algorithm::to_lower(sString);
			if(sString == ".een")
			{
				sPathTarget = iter->path().string();
				std::replace(sPathTarget.begin(), sPathTarget.end(), '\\', '/');
				break;
			}
		}
		
		//
		const RASTER<int>&	uhrh = _sim_hyd->PrendreZones().PrendreGrille();

		map<int, size_t> ident_pixels;
		map<int, size_t> somme_lig;
		map<int, size_t> somme_col;

		double geotransform[6] = { 0 };

		_mapPonderation.clear();

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(sPathTarget.c_str(), GA_ReadOnly));
		if(dataset == nullptr)
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; CalculePonderation; erreur ouverture fichier grille neige; " + sPathTarget);

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
	bool GRILLE_NEIGE::LecturePonderation()
	{
		vector<string> sList;
		istringstream iss;
		string nom_fichier, sString;
		double dTemp;
		size_t index, compteur;
		int iID, iTemp;
		
		nom_fichier = Combine(_sim_hyd->PrendreRepertoireProjet(), "meteo/");
		if(_sPrefixeNomFichier != "")
			nom_fichier+= _sPrefixeNomFichier + ".pgn";
		else
			nom_fichier+= "ponderationgrilleneige.pgn";

		if (!FichierExiste(nom_fichier))
			return false;

		ifstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; LecturePonderation; erreur ouverture fichier de ponderation .pgn");

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
					iss >> iTemp;	//zero-based index du pixel de la grille

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
				throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; LecturePonderation; erreur lecture fichier de ponderation .pgn");
			}
		}

		fichier.close();
		if(index != _sim_hyd->PrendreZones().PrendreNbZone())
			throw ERREUR("DEGRE JOUR MODIFIE; GRILLE_NEIGE; LecturePonderation; erreur lecture fichier de ponderation .pgn; nb de uhrh invalide");

		return true;
	}


	void GRILLE_NEIGE::FormatePathFichierGrilleCourant(string& sPath)
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

}
