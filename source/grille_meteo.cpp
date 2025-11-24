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

#include "grille_meteo.hpp"

#include "gdal_util.hpp"
#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"


using namespace std;


namespace HYDROTEL
{

	GRILLE_METEO::GRILLE_METEO(SIM_HYD& sim_hyd)
		: INTERPOLATION_DONNEES(sim_hyd, "GRILLE")
	{
	}

	GRILLE_METEO::~GRILLE_METEO()
	{
	}

	void GRILLE_METEO::ChangeNbParams(const ZONES& zones)
	{
		_gradient_precipitations.resize(zones.PrendreNbZone(), 0.0f);
		_gradient_temperature.resize(zones.PrendreNbZone(), 0.0f);
		_passage_pluie_neige.resize(zones.PrendreNbZone(), 1.0f);
	}

	void GRILLE_METEO::ChangeGradientPrecipitation(size_t index, float gradient_precipitation)
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		_gradient_precipitations[index] = gradient_precipitation;
	}

	void GRILLE_METEO::ChangeGradientTemperature(size_t index, float gradient_temperature)
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		_gradient_temperature[index] = gradient_temperature;
	}

	void GRILLE_METEO::ChangePassagePluieNeige(size_t index, float passage_pluie_neige)
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		_passage_pluie_neige[index] = passage_pluie_neige;
	}


	float GRILLE_METEO::PrendreGradientPrecipitation(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_precipitations.size());
		return _gradient_precipitations[index];
	}

	float GRILLE_METEO::PrendreGradientTemperature(size_t index) const
	{
		BOOST_ASSERT(index < _gradient_temperature.size());
		return _gradient_temperature[index];
	}

	float GRILLE_METEO::PrendrePassagePluieNeige(size_t index) const
	{
		BOOST_ASSERT(index < _passage_pluie_neige.size());
		return _passage_pluie_neige[index];
	}

	void GRILLE_METEO::Initialise()
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

		INTERPOLATION_DONNEES::Initialise();
	}

	void GRILLE_METEO::Calcule()
	{
		RepartieDonnees();

		if(_bUseTotalPrecip)
			PassagePluieNeige();

		INTERPOLATION_DONNEES::Calcule();
	}

	void GRILLE_METEO::Termine()
	{
		INTERPOLATION_DONNEES::Termine();
	}

	void GRILLE_METEO::LectureParametres()
	{
		vector<std::string> sList;
		string sString, sString2;
		size_t index_zone, x;
		int iIdent;

		_bUseTotalPrecip = false;

		if(_sim_hyd.PrendreNomInterpolationDonnees() == PrendreNomSousModele())	//si le modele grille est simulé
		{
			ifstream fichier(_sim_hyd._nom_fichier_grille_meteo);
			if (!fichier)
				throw ERREUR("INTERPOLATION DONNEES; GRILLE; erreur ouverture FICHIER GRILLE METEO (.grm)");

			try{

			getline_mod(fichier, sString);	//2; type de coordonnee	LONGLAT (1) ou UTM (2)
			getline_mod(fichier, sString);	//2; unite de mesure des donnees (donnees nivometrique seulement) //les données sont en mm pour les grilles météo
			getline_mod(fichier, sString);	//1; indique si les donnees sont obtenues au debut ou a la fin du pas de temps; 1 (début du pas de temps), 2 (fin du pas de temps)
			getline_mod(fichier, sString);	//commentaire

			getline_mod(fichier, sString);	//22; frequence, type de donnees / emplacement des fichiers de grilles

			//21;	ARC_GRILLE_METEO_1H_LOCAL
			//22;	ARC_GRILLE__METEO_24H_LOCAL
			//23;	ARC_GRILLE_1H__METEO_BD
			//24;	ARC_GRILLE_24H__METEO_BD
			//31;	ARC_GRILLE_1H__NEIGE_LOCAL
			//32;	ARC_GRILLE_24H__NEIGE_LOCAL
			//33;	ARC_GRILLE_1H__NEIGE_BD
			//34;	ARC_GRILLE_24H__NEIGE_BD

			SplitString(sList, sString, " ", true, false);

			if(sList[0] != "22")
				throw ERREUR("INTERPOLATION DONNEES; GRILLE; le format [" + sList[0] + "] specifie pour les grilles de donnees est invalide; les grilles de donnees doivent etre de type [22] (fichier local 24h).");

			_sPathFichierGrille = "";
			for(x=2; x<sList.size(); x++)
			{
				_sPathFichierGrille+= sList[x];		//s'il y a des espace dans le path, il faut concatener la string
				if(x < sList.size()-1)
					_sPathFichierGrille+= " ";
			}

			if (!Racine(_sPathFichierGrille) )
				_sPathFichierGrille = Combine(_sim_hyd.PrendreRepertoireProjet(), _sPathFichierGrille);

			if(!RepertoireExiste(_sPathFichierGrille))
				throw ERREUR("INTERPOLATION DONNEES; GRILLE; le dossier specifie pour les grilles de donnees est invalide; " + _sPathFichierGrille);

			getline_mod(fichier, _sPrefixeNomFichier);	//prefixe du nom des fichiers

			if(_sPrefixeNomFichier == "@")
				_sPrefixeNomFichier = "";	//il ny a pas de suffixe

			getline_mod(fichier, sString);	//4; nombre de type de donnees à lire 
			if(sString != "4")
				throw ERREUR("INTERPOLATION DONNEES; GRILLE; le nombre de fichier de donnees disponible est invalide. Celui-ci doit etre egal a 4; pluie, neige, tmax, tmin.");
		
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
				throw ERREUR("INTERPOLATION DONNEES; GRILLE; erreur lecture FICHIER STATIONS METEO (.grm)");
			}
		}

		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		//lecture des parametres (fichier 'grillemeteo.csv')
		string sPathParamGrille;
		sPathParamGrille = Combine(_sim_hyd.PrendreRepertoireSimulation(), "grillemeteo.csv");

		if(FichierExiste(sPathParamGrille))
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			ifstream fichier(sPathParamGrille);
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES grillemeteo.csv");

			string cle, valeur, ligne;
			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES grillemeteo.csv", 1);

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
					throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES grillemeteo.csv; nombre de colonne invalide.");
				}

				iIdent = static_cast<int>(vValeur[0]);
				index_zone = zones.IdentVersIndex(iIdent);

				ChangeGradientTemperature(index_zone, vValeur[1]);
				ChangeGradientPrecipitation(index_zone, vValeur[2]);

				if(vValeur.size() == 4)
					ChangePassagePluieNeige(index_zone, vValeur[3]);
				else
					ChangePassagePluieNeige(index_zone, 1.0f);
			}
		}
		else
			SauvegardeParametres();	//crée le fichier avec les valeurs par defaut s'il n'existe pas
	}


	void GRILLE_METEO::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		string nom_fichier = Combine(_sim_hyd.PrendreRepertoireSimulation(), "grillemeteo.csv");
		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "UHRH ID;GRADIENT TEMPERATURE(C/100m);GRADIENT PRECIPITATION(mm/100m);PASSAGE PLUIE NEIGE(C)" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << PrendreGradientTemperature(index) << ';';
			fichier << PrendreGradientPrecipitation(index) << ';';
			fichier << PrendrePassagePluieNeige(index) << endl;
		}
	}


	//------------------------------------------------------------------------------------------------
	void GRILLE_METEO::SauvegardePonderation()
	{
		ostringstream oss;
		string sString;
		int iCompteur;

		sString = Combine(_sim_hyd.PrendreRepertoireProjet(), "meteo/");
		
		if(_sPrefixeNomFichier != "")
			sString+= _sPrefixeNomFichier + ".pgrm";
		else
			sString+= "ponderationgrillemeteo.pgrm";

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


	//------------------------------------------------------------------------------------------------
	void GRILLE_METEO::SauvegardeAltitude()
	{
		ostringstream oss;
		string sString;

		sString = Combine(_sim_hyd.PrendreRepertoireProjet(), "meteo/");
		
		if(_sPrefixeNomFichier != "")
			sString+= _sPrefixeNomFichier + ".grAlt";
		else
			sString+= "altitudegrillemeteo.grAlt";

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


	//------------------------------------------------------------------------------------------------
	void GRILLE_METEO::CalculePonderation()
	{
		ostringstream oss;
		string sPathTarget;

		_mapPonderation.clear();

		sPathTarget = _sPathFichierGrille + "/" + _sPrefixeNomFichier;
		oss << _sim_hyd.PrendreDateDebut().PrendreAnnee() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd.PrendreDateDebut().PrendreMois() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd.PrendreDateDebut().PrendreJour() << "_";

		if(_sim_hyd.PrendrePasDeTemps() == 24)
			oss << "24h";
		//else

		sPathTarget+= oss.str() + ".tmin";		

		const RASTER<int>&	uhrh = _sim_hyd.PrendreZones().PrendreGrille();

		double geotransform[6] = { 0 };

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(sPathTarget.c_str(), GA_ReadOnly));
		if(dataset == nullptr)
			throw ERREUR("INTERPOLATION DONNEES; GRILLE; CalculePonderation; erreur ouverture fichier grille meteo; " + sPathTarget);

		dataset->GetGeoTransform(geotransform);

		std::map<int, double> mapNbPixel;
		double dXSource, dYSource, dXTarget, dYTarget, dX, dY, dSourceHalfPixelWidth, dSourcePixelWidth, dTargetPixelWidth;
		int row, col, nbRow, nbCol, targetRow, targetCol, index, iVal, iSourceNoData, iTargetNbCol;

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


	//------------------------------------------------------------------------------------------------
	//Calcul l'altitude pour chaque pixel de grille
	void GRILLE_METEO::CalculeAltitude()
	{
		ostringstream oss;
		string sPathTarget;
		float fVal;

		_mapAltitudes.clear();

		sPathTarget = _sPathFichierGrille + "/" + _sPrefixeNomFichier;
		oss << _sim_hyd.PrendreDateDebut().PrendreAnnee() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd.PrendreDateDebut().PrendreMois() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd.PrendreDateDebut().PrendreJour() << "_";

		if(_sim_hyd.PrendrePasDeTemps() == 24)
			oss << "24h";
		//else

		sPathTarget+= oss.str() + ".tmin";		

		RASTER<float> alt = LectureRaster_float(_sim_hyd.PrendreZones().PrendreNomFichierAltitude());

		double geotransform[6] = { 0 };

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(sPathTarget.c_str(), GA_ReadOnly));
		if(dataset == nullptr)
			throw ERREUR("INTERPOLATION DONNEES; GRILLE; CalculeAltitudeGrille; erreur ouverture fichier grille meteo; " + sPathTarget);

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

		//calcule la moyenne (moyenne des pixel altidude pour chaque pixel grille
		for (auto iter1 = begin(_mapAltitudes); iter1 != end(_mapAltitudes); iter1++)
			iter1->second/= mapNbPixel[iter1->first];
	}


	//------------------------------------------------------------------------------------------------
	bool GRILLE_METEO::LecturePonderation()
	{
		vector<string> sList;
		istringstream iss;
		string nom_fichier, sString;
		double dTemp;
		size_t index, compteur;
		int iID, iTemp;

		_mapPonderation.clear();
		
		nom_fichier = Combine(_sim_hyd.PrendreRepertoireProjet(), "meteo/");
		if(_sPrefixeNomFichier != "")
			nom_fichier+= _sPrefixeNomFichier + ".pgrm";
		else
			nom_fichier+= "ponderationgrillemeteo.pgrm";

		if (!FichierExiste(nom_fichier))
			return false;

		ifstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR("INTERPOLATION DONNEES; GRILLE; LecturePonderation; erreur ouverture fichier de ponderation .pgrm");

		index = 0;

		try{

		getline_mod(fichier, sString);	//ignore la 1iere ligne
		while (!fichier.eof())
		{
			getline_mod(fichier, sString);
			if(sString != "")
			{
				SplitString(sList, sString, " ", true, true);

				iID = _sim_hyd.PrendreZones()[index].PrendreIdent();	//id uhrh
			
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
				throw ERREUR("INTERPOLATION DONNEES; GRILLE; LecturePonderation; erreur lecture fichier de ponderation .pgrm");
			}
		}

		fichier.close();
		if(index != _sim_hyd.PrendreZones().PrendreNbZone())
			throw ERREUR("INTERPOLATION DONNEES; GRILLE; LecturePonderation; erreur lecture fichier de ponderation .pgrm; nb de uhrh invalide");

		return true;
	}


	//------------------------------------------------------------------------------------------------
	bool GRILLE_METEO::LectureAltitude()
	{
		vector<string> sList;
		istringstream iss;
		string nom_fichier, sString;
		int iTemp;

		_mapAltitudes.clear();
		
		nom_fichier = Combine(_sim_hyd.PrendreRepertoireProjet(), "meteo/");
		if(_sPrefixeNomFichier != "")
			nom_fichier+= _sPrefixeNomFichier + ".grAlt";
		else
			nom_fichier+= "altitudegrillemeteo.grAlt";

		if (!FichierExiste(nom_fichier))
			return false;

		ifstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR("INTERPOLATION DONNEES; GRILLE; LectureAltitude; erreur ouverture fichier des altitudes .grAlt");

		try{

		getline_mod(fichier, sString);	//ignore la 1iere ligne
		while (!fichier.eof())
		{
			getline_mod(fichier, sString);
			if(sString != "")
			{
				SplitString(sList, sString, " ", true, true);

				if(sList.size() != 2)
				{
					fichier.close();
					throw ERREUR("INTERPOLATION DONNEES; GRILLE; LectureAltitude; erreur lecture fichier des altitudes .grAlt; nombre de colonne invalide.");
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
				throw ERREUR("INTERPOLATION DONNEES; GRILLE; LectureAltitude; erreur lecture fichier des altitudes .grAlt");
			}
		}

		fichier.close();
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void GRILLE_METEO::RepartieDonnees()
	{
		string sString, sString2;
		int iIdent, indexcell;

		ZONES& zones = _sim_hyd.PrendreZones();

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		size_t index_zone, row, col, nbCol;
		float tmin, tmax, pluie, neige, tmin_jour, tmax_jour, fAltitude, fDiffAlt, fDensiteNeige, fVal, fValGrille;

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
			throw ERREUR("INTERPOLATION DONNEES; GRILLE; erreur de lecture du fichier; " + sString2);
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
					tmin+= (grilleTMin[0](row, col) + PrendreGradientTemperature(index_zone) * fDiffAlt / 100.0f) * static_cast<float>(iter2->second);	//iter2->second -> ponderation

				if(grilleTMax[0](row, col) > VALEUR_MANQUANTE)
					tmax+= (grilleTMax[0](row, col) + PrendreGradientTemperature(index_zone) * fDiffAlt / 100.0f) * static_cast<float>(iter2->second);	//iter2->second -> ponderation

				if(_bUseTotalPrecip)
				{
					if(grillePrecipTot[0](row, col) > VALEUR_MANQUANTE)
					{
						fValGrille = grillePrecipTot[0](row, col);
						fVal = (fValGrille * static_cast<float>(iter2->second) * (1.0f + PrendreGradientPrecipitation(index_zone) / 100.0f * fDiffAlt));	//iter2->second -> ponderation

						pluie+= fVal;
					}
				}
				else
				{
					if(grillePluie[0](row, col) > VALEUR_MANQUANTE)
						pluie+= (grillePluie[0](row, col) * static_cast<float>(iter2->second) * (1.0f + PrendreGradientPrecipitation(index_zone) / 100.0f * fDiffAlt));	//iter2->second -> ponderation

					if(grilleNeige[0](row, col) > VALEUR_MANQUANTE)
						neige+= (grilleNeige[0](row, col) * static_cast<float>(iter2->second) * (1.0f + PrendreGradientPrecipitation(index_zone) / 100.0f * fDiffAlt));	//iter2->second -> ponderation
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


	void GRILLE_METEO::FormatePathFichierGrilleCourant(string& sPath)
	{
		ostringstream oss;

		sPath = _sPathFichierGrille + "/" + _sPrefixeNomFichier;
		oss << _sim_hyd.PrendreDateCourante().PrendreAnnee() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd.PrendreDateCourante().PrendreMois() << "_";
		oss << setfill('0') << setw(2) << _sim_hyd.PrendreDateCourante().PrendreJour() << "_";

		//if(_sim_hyd.PrendrePasDeTemps() == 24)
			oss << "24h";

		sPath+= oss.str();
	}


	void GRILLE_METEO::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal );

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
			if(ligne == "GRILLE")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 3 && vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide GRILLE");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide GRILLE. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						ChangeGradientTemperature(index_zone, vValeur[1]);
						ChangeGradientPrecipitation(index_zone, vValeur[2]);

						if(vValeur.size() == 4)
							ChangePassagePluieNeige(index_zone, vValeur[3]);
						else
							ChangePassagePluieNeige(index_zone, 1.0f);
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
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal + "; GRILLE");
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele GRILLE");
	}


	void GRILLE_METEO::PassagePluieNeige()
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
