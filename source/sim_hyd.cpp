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

#include "sim_hyd.hpp"

#include "mise_a_jour.hpp"
#include "gdal_util.hpp"
#include "lecture_interpolation_donnees.hpp"
#include "thiessen1.hpp"
#include "thiessen2.hpp"
#include "moyenne_3_stations1.hpp"
#include "moyenne_3_stations2.hpp"
#include "grille_meteo.hpp"
#include "lecture_fonte_neige.hpp"
#include "degre_jour_modifie.hpp"
#include "degre_jour_bande.hpp"
#include "lecture_fonte_glacier.hpp"
#include "degre_jour_glacier.hpp"
#include "lecture_tempsol.hpp"
#include "rankinen.hpp"
#include "thorsen.hpp"
#include "lecture_evapotranspiration.hpp"
#include "hydro_quebec.hpp"
#include "thornthwaite.hpp"
#include "linacre.hpp"
#include "penman.hpp"
#include "priestlay_taylor.hpp"
#include "penman_monteith.hpp"
#include "etp_mc_guiness.hpp"
#include "lecture_bilan_vertical.hpp"
#include "bv3c1.hpp"
#include "bv3c2.hpp"
#include "cequeau.hpp"
#include "lecture_ruisselement_surface.hpp"
#include "onde_cinematique.hpp"
#include "lecture_acheminement_riviere.hpp"
#include "onde_cinematique_modifiee.hpp"
#include "util.hpp"
#include "erreur.hpp"
#include "version.hpp"
#include "constantes.hpp"

#include <fstream>
#include <regex>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	SIM_HYD::SIM_HYD()
	{
		_nbThread = 1;

		_bUpdatingV26Project = false;

		_output._sim_hyd = this;

		_interpolation_donnees = nullptr;
		_fonte_neige = nullptr;
		_fonte_glacier = nullptr;
		_tempsol = nullptr;
		_evapotranspiration = nullptr;
		_bilan_vertical = nullptr;
		_ruisselement_surface = nullptr;
		_acheminement_riviere = nullptr;

		_smThiessen1 = new THIESSEN1(*this);
		_smThiessen2 = new THIESSEN2(*this);
		_smMoy3station1 = new MOYENNE_3_STATIONS1(*this);
		_smMoy3station2 = new MOYENNE_3_STATIONS2(*this);
		_smGrilleMeteo = new GRILLE_METEO(*this);

		_vinterpolation_donnees.resize(6);
		_vinterpolation_donnees[INTERPOLATION_LECTURE] = unique_ptr<INTERPOLATION_DONNEES>(new LECTURE_INTERPOLATION_DONNEES(*this));
		_vinterpolation_donnees[INTERPOLATION_THIESSEN1] = unique_ptr<INTERPOLATION_DONNEES>(_smThiessen1);
		_vinterpolation_donnees[INTERPOLATION_THIESSEN2] = unique_ptr<INTERPOLATION_DONNEES>(_smThiessen2);
		_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS1] = unique_ptr<INTERPOLATION_DONNEES>(_smMoy3station1);
		_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS2] = unique_ptr<INTERPOLATION_DONNEES>(_smMoy3station2);
		_vinterpolation_donnees[INTERPOLATION_GRILLE] = unique_ptr<INTERPOLATION_DONNEES>(_smGrilleMeteo);

		_vfonte_neige.resize(3);
		_vfonte_neige[FONTE_NEIGE_LECTURE] = unique_ptr<FONTE_NEIGE>(new LECTURE_FONTE_NEIGE(*this));
		_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_MODIFIE] = unique_ptr<FONTE_NEIGE>(new DEGRE_JOUR_MODIFIE(*this));
		_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_BANDE] = unique_ptr<FONTE_NEIGE>(new DEGRE_JOUR_BANDE(*this));

		_vfonte_glacier.resize(2);
		_vfonte_glacier[FONTE_GLACIER_LECTURE] = unique_ptr<FONTE_GLACIER>(new LECTURE_FONTE_GLACIER(*this));
		_vfonte_glacier[FONTE_GLACIER_DEGRE_JOUR_GLACIER] = unique_ptr<FONTE_GLACIER>(new DEGRE_JOUR_GLACIER(*this));

		_vtempsol.resize(3);
		_vtempsol[TEMPSOL_LECTURE] = unique_ptr<TEMPSOL>(new LECTURE_TEMPSOL(*this));
		_vtempsol[TEMPSOL_RANKINEN] = unique_ptr<TEMPSOL>(new RANKINEN(*this));
		_vtempsol[TEMPSOL_THORSEN] = unique_ptr<TEMPSOL>(new THORSEN(*this));

		_rayonnementNet._sim_hyd = this;

		_vevapotranspiration.resize(8);
		_vevapotranspiration[ETP_LECTURE] = unique_ptr<EVAPOTRANSPIRATION>(new LECTURE_EVAPOTRANSPIRATION(*this));
		_vevapotranspiration[ETP_HYDRO_QUEBEC] = unique_ptr<EVAPOTRANSPIRATION>(new HYDRO_QUEBEC(*this));
		_vevapotranspiration[ETP_THORNTHWAITE] = unique_ptr<EVAPOTRANSPIRATION>(new THORNTHWAITE(*this));
		_vevapotranspiration[ETP_LINACRE] = unique_ptr<EVAPOTRANSPIRATION>(new LINACRE(*this));
		_vevapotranspiration[ETP_PENMAN] = unique_ptr<EVAPOTRANSPIRATION>(new PENMAN(*this));
		_vevapotranspiration[ETP_PRIESTLAY_TAYLOR] = unique_ptr<EVAPOTRANSPIRATION>(new PRIESTLAY_TAYLOR(*this));
		_vevapotranspiration[ETP_PENMAN_MONTEITH] = unique_ptr<EVAPOTRANSPIRATION>(new PENMAN_MONTEITH(*this));
		_vevapotranspiration[ENUM_ETP_MC_GUINESS] = unique_ptr<EVAPOTRANSPIRATION>(new ETP_MC_GUINESS(*this));

		_vbilan_vertical.resize(4);
		_vbilan_vertical[BILAN_VERTICAL_LECTURE] = unique_ptr<BILAN_VERTICAL>(new LECTURE_BILAN_VERTICAL(*this));
		_vbilan_vertical[BILAN_VERTICAL_BV3C1] = unique_ptr<BILAN_VERTICAL>(new BV3C1(*this));
		_vbilan_vertical[BILAN_VERTICAL_BV3C2] = unique_ptr<BILAN_VERTICAL>(new BV3C2(*this));
		_vbilan_vertical[BILAN_VERTICAL_CEQUEAU] = unique_ptr<BILAN_VERTICAL>(new CEQUEAU(*this));

		_vruisselement.resize(2);
		_vruisselement[RUISSELEMENT_LECTURE] = unique_ptr<RUISSELEMENT_SURFACE>(new LECTURE_RUISSELEMENT_SURFACE(*this));
		_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE] = unique_ptr<RUISSELEMENT_SURFACE>(new ONDE_CINEMATIQUE(*this));

		_vacheminement.resize(2);
		_vacheminement[ACHEMINEMENT_LECTURE] = unique_ptr<ACHEMINEMENT_RIVIERE>(new LECTURE_ACHEMINEMENT_RIVIERE(*this));
		_vacheminement[ACHEMINEMENT_ONDE_CINEMATIQUE_MODIFIEE] = unique_ptr<ACHEMINEMENT_RIVIERE>(new ONDE_CINEMATIQUE_MODIFIEE(*this));

		_versionTHIESSEN = 2;		//models versions to use (initialized with latest available version)
		_versionMOY3STATION = 2;	//
		_versionBV3C = 2;			//

		_pas_de_temps = 24;
		_ident_troncon_exutoire = 1;
		_fichierParametreGlobal = 0;
		_bSimulePrevision = false;
		_bSimuleMHRiverain = false;
		_bSimuleMHIsole = false;

		_bActiveTronconDeconnecte = false;

		_sPathProjetImport = "";
		
		_bAutoInverseTMinTMax = false;
		_bStationInterpolation = true;
		_bSkipCharacterValidation = false;

		_outputCDF = false;

		_outputNbZone = 0;
		_vOutputIndexZone = nullptr;

		_pRasterOri = nullptr;
		_pRasterPente = nullptr;

		_bGenereBdPrelevements = false;		
		_sFolderNamePrelevements = "prelevements";
		_sFolderNamePrelevementsSrc = "SitesPrelevements";
		_pr = new PRELEVEMENTS(*this);
	}


	SIM_HYD::~SIM_HYD()
	{
		if(_vOutputIndexZone != nullptr)
			delete [] _vOutputIndexZone;

		if(_pRasterOri)
		{
			_pRasterOri->Close();
			delete _pRasterOri;
		}

		if(_pRasterPente)
		{
			_pRasterPente->Close();
			delete _pRasterPente;
		}
	}

	void SIM_HYD::ChangeNomFichier(const string& nom_fichier)
	{
		_nom_fichier = nom_fichier;
	}


	void SIM_HYD::LectureVersionFichierSim()
	{
		vector<string> sList;
		istringstream iss;
		ifstream file;
		size_t lVal, lVal2;
		string sErr, cle, valeur;

		sErr = "";

		try{
		file.open(_nom_fichier_simulation);
		if(!file.is_open())
			sErr = "error opening simulation file: " + _nom_fichier_simulation;
		else
		{
			lire_cle_valeur(file, cle, valeur);
			file.close();

			if(cle != "SIMULATION HYDROTEL VERSION")
				sErr = "error reading simulation file version: 1st line not valid: " + _nom_fichier_simulation;
			else
			{
				SplitString2(sList, valeur, ".", true);		//ex: 4.1.5.0014

				try{
				iss.str(sList[0]);	//1st number: possible value: 0 a 999
				iss >> lVal2;
				lVal = lVal2 * 1000000;

				iss.clear();
				iss.str(sList[1]);	//2nd number: possible value: 0 a 999
				iss >> lVal2;
				lVal+= lVal2 * 1000;

				iss.clear();
				iss.str(sList[2]);	//3rd number: possible value: 0 a 999
				iss >> lVal2;
				lVal+= lVal2;

				_versionSim = lVal;
				_versionSimStr = valeur;
				}
				catch(...)
				{
					sErr = "error reading simulation file version: error parsing version number: " + _nom_fichier_simulation;
				}
			}
		}
		}
		catch(...)
		{
			if(file && file.is_open())
				file.close();
			sErr = "error opening/reading simulation file: catch exception: " + _nom_fichier_simulation;
		}

		if(sErr != "")
			throw ERREUR(sErr);
	}


	void SIM_HYD::Lecture()
	{
		vector<string> fileList;
		ofstream file;
		string str, str2;
		size_t i;

		_troncons._pSimHyd = this;

		string ext = PrendreExtension(_nom_fichier);		

		if (ext == ".prj") //old version (2.6) project file
		{
			LectureProjetFormatPrj();

			_versionTHIESSEN = 1;
			_versionMOY3STATION = 1;
			_versionBV3C = 1;

			if(!_bUpdatingV26Project)	//dont modify source project folder if run with -u switch
				CreateSubmodelsVersionsFile();

			if(_sPathProjetImport != "")	//workaround pour utiliser les nouvelles carte tif importé plutot que les anciennes (.uh, .mna, etc)
				_zones._nom_fichier_zoneTemp = _sPathProjetImport + "/uhrh.tif";

			_zones.LectureZones();

			LectureGroupeZone();

			_propriete_hydroliques._coefficient_additif.resize(_groupes.size(), 0);
			LectureDonneesPhysiographiques();

			LectureDonneesMeteorologiques();		
			ChangeNbParams();
			LectureGroupeZoneCorrection();
			LectureSimulationFormatSim();

			DisplayInfo();
		}
		else if (ext == ".csv") //version 4 project file
		{
			//input files characters validation
			if(!_bSkipCharacterValidation)
			{
				fileList.push_back(_nom_fichier);		//project file
				str = ValidateInputFilesCharacters(fileList, _listErrMessCharValidation);
				if(str != "")
					throw ERREUR(str);

				if(_listErrMessCharValidation.size() != 0)
				{
					std::cout << endl;
					for(i=0; i!=_listErrMessCharValidation.size(); i++)
						std::cout << _listErrMessCharValidation[i] << endl;

					throw ERREUR("Error reading input files: invalid characters: valid characters are ascii/utf8 code 32 to 126.");
				}
			}
			
			LectureProjetFormatCsv();

			str = Combine(PrendreRepertoireSimulation(), "submodels-versions.txt");
			if(!boost::filesystem::exists(str))
			{
				LectureVersionFichierSim();
				if(_versionSim <= 4001005)	//v 4.1.5
				{
					_versionTHIESSEN = 1;
					_versionMOY3STATION = 1;
					_versionBV3C = 1;
				} //else: the latest versions of models are used (initialization values)

				CreateSubmodelsVersionsFile();
			}
			else
				ReadSubmodelsVersionsFile();

			_zones.LectureZones();
			_noeuds.Lecture();

			//input files characters validation
			if(!_bSkipCharacterValidation)
			{
				fileList.clear();
				fileList.push_back(_troncons.PrendreNomFichier());							//river reach file (troncon.trl)

				str = Combine(PrendreRepertoireSimulation(), _nom_simulation + ".gsb");		//subwatershed group file (.gsb)
				fileList.push_back(str);

				fileList.push_back(_nom_fichier_simulation);								//simulation file

				str = ValidateInputFilesCharacters(fileList, _listErrMessCharValidation);
				if(str != "")
					throw ERREUR(str);

				if(_listErrMessCharValidation.size() != 0)
				{
					std::cout << endl;
					for(i=0; i!=_listErrMessCharValidation.size(); i++)
						std::cout << _listErrMessCharValidation[i] << endl;

					throw ERREUR("Error reading input files: invalid characters: valid characters are ascii/utf8 code 32 to 126.");
				}
			}

			_troncons.LectureTroncons(_zones, _noeuds);

			if(_zones._bSaveUhrhCsvFile)
			{
				str = RemplaceExtension(_zones.PrendreNomFichierZone(), "csv");
				_zones.SauvegardeResumer(str);
			}

			LectureGroupeZone();
			LectureSimulationFormatCsv();

			//input files characters validation
			if(!_bSkipCharacterValidation)
			{
				fileList.clear();

				str = PrendreExtension(_stations_meteo.PrendreNomFichier());
				if(str != ".nc" && str != ".h5")
					fileList.push_back(_stations_meteo.PrendreNomFichier());														//weather stations file

				fileList.push_back(_stations_hydro.PrendreNomFichier());															//hydro stations file

				str = RemplaceExtension(_occupation_sol.PrendreNomFichier(), "csv");												//land use names
				fileList.push_back(str);

				fileList.push_back(_occupation_sol.PrendreNomFichier());															//land use

				fileList.push_back(_propriete_hydroliques.PrendreNomFichier());														//hydraulic properties

				fileList.push_back(_occupation_sol.PrendreNomFichierIndicesFolieres());												//indice foliaire (ind_fol.def)

				fileList.push_back(_occupation_sol.PrendreNomFichierProfondeursRacinaires());										//profondeur racinaire (pro_rac.def)

				str = Combine(PrendreRepertoireSimulation(), _nom_simulation + ".sbc");												//correction group file (.sbc)
				fileList.push_back(str);

				auto degre_jour = static_cast<DEGRE_JOUR_MODIFIE*>(_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_MODIFIE].get());
				auto degre_jour_bande = static_cast<DEGRE_JOUR_BANDE*>(_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_BANDE].get());

				str = degre_jour->_stations_neige_conifers.PrendreNomFichier();														//snow stations file (.stn)
				if(str != "")
					fileList.push_back(str);
				str = degre_jour->_stations_neige_feuillus.PrendreNomFichier();														//
				if(str != "")
					fileList.push_back(str);
				str = degre_jour->_stations_neige_decouver.PrendreNomFichier();														//
				if(str != "")
					fileList.push_back(str);

				str = degre_jour_bande->_stations_neige_conifers.PrendreNomFichier();												//
				if(str != "")
					fileList.push_back(str);
				str = degre_jour_bande->_stations_neige_feuillus.PrendreNomFichier();												//
				if(str != "")
					fileList.push_back(str);
				str = degre_jour_bande->_stations_neige_decouver.PrendreNomFichier();												//
				if(str != "")
					fileList.push_back(str);

				if(_fichierParametreGlobal)
					fileList.push_back(_nomFichierParametresGlobal);																//parameters unique file
				else
				{
					fileList.push_back(degre_jour->PrendreNomFichierParametres());													//degre_jour_modifie.csv
					fileList.push_back(degre_jour_bande->PrendreNomFichierParametres());											//degre_jour_bande.csv

					auto onde_cinematique = static_cast<ONDE_CINEMATIQUE*>(_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE].get());
					fileList.push_back(onde_cinematique->PrendreNomFichierParametres());											//onde_cinematique.csv
				}

				if(!_bGenereBdPrelevements)
				{
					str = PrendreRepertoireSimulation() + "/" + _sFolderNamePrelevements + "/" + "BD_CULTURES.csv";							//prélèvements
					fileList.push_back(str);
					str = PrendreRepertoireSimulation() + "/" + _sFolderNamePrelevements + "/" + "BD_EFFLUENTS.csv";						//
					fileList.push_back(str);
					str = PrendreRepertoireSimulation() + "/" + _sFolderNamePrelevements + "/" + "BD_ELEVAGES.csv";							//
					fileList.push_back(str);
					str = PrendreRepertoireSimulation() + "/" + _sFolderNamePrelevements + "/" + "BD_GPE.csv";								//
					fileList.push_back(str);
					str = PrendreRepertoireSimulation() + "/" + _sFolderNamePrelevements + "/" + "BD_PRELEVEMENTS.csv";						//
					fileList.push_back(str);
					str = PrendreRepertoireSimulation() + "/" + _sFolderNamePrelevements + "/" + "BD_SIH.csv";								//
					fileList.push_back(str);
				}

				str = ValidateInputFilesCharacters(fileList, _listErrMessCharValidation);
				if(str != "")
					throw ERREUR(str);

				if(_listErrMessCharValidation.size() != 0)
				{
					std::cout << endl;
					for(i=0; i!=_listErrMessCharValidation.size(); i++)
						std::cout << _listErrMessCharValidation[i] << endl;

					throw ERREUR("Error reading input files: invalid characters: valid characters are ascii/utf8 code 32 to 126.");
				}
			}

			LectureDonneesMeteorologiques();
			LectureDonneesHydrologiques();

			DisplayInfo();

			_occupation_sol.Lecture(_zones);
			_propriete_hydroliques.Lecture((*this));

			ChangeNbParams();

			LectureInterpolationDonnees();
			LectureFonteNeige();
			
			if(_fonte_glacier)
				LectureFonteGlacier();

			if(_tempsol)
				LectureTempSol();

			_rayonnementNet.LectureParametres();

			LectureEtp();
			LectureBilanVertical();
			LectureRuisselement();
			LectureAcheminementRiviere();

			LectureGroupeZoneCorrection();

			_output._iIDTronconExutoire = _ident_troncon_exutoire;
			_output.Lecture( PrendreRepertoireSimulation() );
			if(_output._iOutputCDF == 1)
				_outputCDF = true;
			else
			{
				if(_output._iOutputCDF == 0)
					_outputCDF = false;
			}
		}
		else
		{
			throw ERREUR_LECTURE_FICHIER("FICHIER PROJET; " + _nom_fichier);
		}

		//lecture et utilisation des largeurs calculées par physitel si disponible
		str = Combine(PrendreRepertoireProjet(), "physio/troncons-largeur-ponderees.csv");
		if(boost::filesystem::exists(str))
		{
			_troncons.LectureFichierLargeur(str);
			std::cout << "Using river reach width from file: " << str << endl << endl;
		}
		else
		{
			str = Combine(PrendreRepertoireProjet(), "physio/troncons-largeur.csv");
			if(boost::filesystem::exists(str))
			{
				_troncons.LectureFichierLargeur(str);
				std::cout << "Using river reach width from file: " << str << endl << endl;
			}
		}

		//calcul et création du fichier ordre de shreve s'il n'existe pas
		if(!_bUpdatingV26Project)	//dont modify source project folder if run with -u switch
		{
			str = Combine(PrendreRepertoireProjet(), "physio/shreve.csv");
			if(!boost::filesystem::exists(str))
			{
				_troncons.CalculeShreve();

				try{
				file.open(str);
				if(!file.is_open())
					throw ERREUR("");

				file << "IdTroncon;NoOrdreShreve" << endl;

				for(i=0; i!=_troncons._troncons.size(); i++)
					file << i+1 << ";" << _troncons._troncons[i]->_iSchreve << endl;

				file.close();
				}
				catch(...)
				{
					if(file && file.is_open())
						file.close();

					throw ERREUR("erreur lors de la creation du fichier: " + str);
				}
			}

			//calcul et création du fichier ordre de strahler s'il n'existe pas
			str = Combine(PrendreRepertoireProjet(), "physio/strahler.csv");
			if(!boost::filesystem::exists(str))
			{
				_troncons.CalculeStrahler();

				try{
				file.clear();
				file.open(str);
				if(!file.is_open())
					throw ERREUR("");

				file << "IdTroncon;NoOrdreStrahler" << endl;

				for(i=0; i!=_troncons._troncons.size(); i++)
					file << i+1 << ";" << _troncons._troncons[i]->_iSchreve << endl;

				file.close();
				}
				catch(...)
				{
					if(file && file.is_open())
						file.close();

					throw ERREUR("erreur lors de la creation du fichier: " + str);
				}
			}
		}

		//
		_lNbPasTempsSim = _date_debut.NbHeureEntre(_date_fin) / _pas_de_temps;

		_sPathProjetImport = "";

		_corrections.LectureFichier(*this);

		// creation du groupe "all"
		vector<int> idents(_zones.PrendreNbZone());
		for (size_t index = 0; index <_zones.PrendreNbZone(); ++index)
		{
			idents[index] = _zones[index].PrendreIdent();
		}
		_groupe_all.ChangeIdentZones(idents);
	}


	void SIM_HYD::ChangeNbParams()
	{		
		for(auto iter = begin(_vinterpolation_donnees); iter != end(_vinterpolation_donnees); ++iter)
			iter->get()->ChangeNbParams(_zones);

		//for(auto iter = begin(_vinterpolation_donnees); iter != end(_vinterpolation_donnees); ++iter)
		//{
		//	if((*iter)->PrendreNomSousModele() == "THIESSEN1")
		//		_smThiessen1->ChangeNbParams(_zones);	//THIESSEN1* ptr = (THIESSEN1*)(iter->get());
		//	else
		//	{
		//		if((*iter)->PrendreNomSousModele() == "THIESSEN2")
		//			_smThiessen2->ChangeNbParams(_zones);
		//		else
		//		{
		//			if((*iter)->PrendreNomSousModele() == "MOYENNE 3 STATIONS1")
		//				_smMoy3station1->ChangeNbParams(_zones);
		//			else
		//			{
		//				if((*iter)->PrendreNomSousModele() == "MOYENNE 3 STATIONS2")
		//					_smMoy3station2->ChangeNbParams(_zones);
		//				else
		//					iter->get()->ChangeNbParams(_zones);
		//			}
		//		}
		//	}
		//}

		for (auto iter = begin(_vfonte_neige); iter != end(_vfonte_neige); ++iter)
			iter->get()->ChangeNbParams(_zones);

		for (auto iter = begin(_vfonte_glacier); iter != end(_vfonte_glacier); ++iter)
			iter->get()->ChangeNbParams(_zones);

		for (auto iter = begin(_vtempsol); iter != end(_vtempsol); ++iter)
			iter->get()->ChangeNbParams(_zones);

		_rayonnementNet.ChangeNbParams(_zones);

		for (auto iter = begin(_vevapotranspiration); iter != end(_vevapotranspiration); ++iter)
			iter->get()->ChangeNbParams(_zones);

		for (auto iter = begin(_vbilan_vertical); iter != end(_vbilan_vertical); ++iter)
			iter->get()->ChangeNbParams(_zones);

		for (auto iter = begin(_vruisselement); iter != end(_vruisselement); ++iter)
			iter->get()->ChangeNbParams(_zones);

		for (auto iter = begin(_vacheminement); iter != end(_vacheminement); ++iter)
			iter->get()->ChangeNbParams(_zones);
	}

	void SIM_HYD::LectureProjetFormatCsv()
	{
		string cle, valeur, ligne, nom_fichier, str;

		if(!FichierExiste(_nom_fichier))
			throw ERREUR_LECTURE_FICHIER("PROJECT FILE: invalid filename: " + _nom_fichier);

		ifstream fichier(_nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("PROJECT FILE: " + _nom_fichier);

		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		if (cle != "PROJET HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("PROJECT FILE: the file `" + _nom_fichier + "` is not a valid project file.");

		string repertoire = PrendreRepertoireProjet();
		
		lire_cle_valeur(fichier, cle, nom_fichier);		
		if (!Racine(nom_fichier) )
			nom_fichier = Combine(repertoire, nom_fichier);
		_zones.ChangeNomFichierAltitude(nom_fichier);

		lire_cle_valeur(fichier, cle, nom_fichier);
		if (!Racine(nom_fichier) )
			nom_fichier = Combine(repertoire, nom_fichier);
		_zones.ChangeNomFichierPente(nom_fichier);

		lire_cle_valeur(fichier, cle, nom_fichier);
		if (!Racine(nom_fichier) )
			nom_fichier = Combine(repertoire, nom_fichier);
		_zones.ChangeNomFichierOrientation(nom_fichier);

		lire_cle_valeur(fichier, cle, nom_fichier);
		if (!Racine(nom_fichier) )
			nom_fichier = Combine(repertoire, nom_fichier);
		_zones.ChangeNomFichierZone(nom_fichier);

		lire_cle_valeur(fichier, cle, nom_fichier);
		if (!Racine(nom_fichier) )
			nom_fichier = Combine(repertoire, nom_fichier);
		_noeuds.ChangeNomFichier(nom_fichier);

		lire_cle_valeur(fichier, cle, nom_fichier);
		if (!Racine(nom_fichier) )
			nom_fichier = Combine(repertoire, nom_fichier);
		_troncons.ChangeNomFichier(nom_fichier);

		lire_cle_valeur(fichier, cle, nom_fichier);
		if (!Racine(nom_fichier) )
			nom_fichier = Combine(repertoire, nom_fichier);
		_troncons.ChangeNomFichierPixels(nom_fichier);

		lire_cle_valeur(fichier, cle, _nom_simulation);
		if(cle == "FICHIER MILIEUX HUMIDES PROFONDEUR TRONCONS")
		{
			if (!Racine(_nom_simulation))
				_nom_simulation = Combine(repertoire, _nom_simulation);
			_nom_fichier_milieu_humide_profondeur_troncon = _nom_simulation;

			getline_mod(fichier, ligne);	//ligne vide
			lire_cle_valeur(fichier, cle, _nom_simulation);
		}

		_nom_fichier_simulation = repertoire + "/simulation/" + _nom_simulation + "/" + _nom_simulation + ".csv";
	}


	void SIM_HYD::LectureProjetFormatPrj()
	{
		ifstream fichier(_nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("PROJECT FILE: " + _nom_fichier);

		fichier.exceptions(ios::failbit | ios::badbit);

		string repertoire = PrendreRepertoire(_nom_fichier);

		string version, temp;
		getline_mod(fichier, version);

		//	if (!regex_match(version, regex("HYDROTEL V46")) && 
		//!regex_match(version, regex("HYDROTEL V47")) && 
		//!regex_match(version, regex("HYDROTEL V49")) &&
		//!regex_match(version, regex("HYDROTEL V50")) )
		//throw ERREUR("Erreur; la version du fichier de projet nest pas supportee.");

		if(version.size() >= 12)
		{
			if(version.substr(0, 10) != "HYDROTEL V")
				throw ERREUR("PROJECT FILE: invalid project file.");
		}
		else
			throw ERREUR("PROJECT FILE: invalid project file.");

		getline_mod(fichier, temp);
		getline_mod(fichier, temp);

		_stations_meteo.ChangeNomFichier(LireNomFichier(repertoire, fichier, 6));

		temp = LireChaine(fichier, 1);
		temp = LireChaine(fichier);
		temp = LireChaine(fichier);

		_stations_hydro.ChangeNomFichier(LireNomFichier(repertoire, fichier, 2));

		temp = LireChaine(fichier, 1);

		//if(regex_match(version, regex("HYDROTEL V46")))
		//	temp = LireChaine(fichier);

		_zones.ChangeNomFichierZone(LireNomFichier(repertoire, fichier, 1));
		_zones.ChangeNomFichierAltitude(LireNomFichier(repertoire, fichier, 1));
		_zones.ChangeNomFichierOrientation(LireNomFichier(repertoire, fichier, 1));
		_zones.ChangeNomFichierPente(LireNomFichier(repertoire, fichier, 1));

		_occupation_sol.ChangeNomFichier(LireNomFichier(repertoire, fichier, 1));
		
		_noeuds.ChangeNomFichier(LireNomFichier(repertoire, fichier, 1));

		_troncons.ChangeNomFichier(LireNomFichier(repertoire, fichier, 1));
		_troncons.ChangeNomFichierPixels(LireNomFichier(repertoire, fichier, 1));

		//---------------------
		//il y a plusieurs version, on s'assure de lire les bonne valeurs
		//dans tous les cas, après, on sera rendu au nb de simulation
		vector<string> sList;
		char ch;

		std::streamoff filepos = fichier.tellg();

		getline_mod(fichier, temp);
		if(temp == "")
			getline_mod(fichier, temp);

		SplitString2(sList, temp, " \t", true);
		if(sList.size() == 1)
		{
			//soit le nb de simulation ou les fichiers pour milieux humide
			fichier.seekg(filepos);

			if(sList[0][0] == '@')
			{
				//fichier pour MH
				temp = LireChaine(fichier);
				if(temp != "absent")
				{
					if (!Racine(temp))
						temp = Combine(PrendreRepertoireProjet(), temp);			
					_nom_fichier_milieu_humide_profondeur_troncon = temp;
				}

				temp = LireChaine(fichier);
				if(temp != "absent")
				{
					if (!Racine(temp))
						temp = Combine(PrendreRepertoireProjet(), temp);			
					_nom_fichier_milieu_humide_riverain = temp;
				}

				temp = LireChaine(fichier);
				if(temp != "absent")
				{
					if (!Racine(temp))
						temp = Combine(PrendreRepertoireProjet(), temp);			
					_nom_fichier_milieu_humide_isole = temp;
				}

				temp = LireChaine(fichier);
			}
		}
		else
		{
			//les valeurs 0 0 0 ont été lu
			//vérifie si la prochaine ligne est les fichiers pour MH
			filepos = fichier.tellg();
			fichier >> ch;	//lit le caractere et remet a la meme position
			fichier.seekg(filepos);

			if(ch == '@')
			{
				//fichier pour MH
				temp = LireChaine(fichier);
				if(temp != "absent")
				{
					if (!Racine(temp))
						temp = Combine(PrendreRepertoireProjet(), temp);			
					_nom_fichier_milieu_humide_profondeur_troncon = temp;
				}

				temp = LireChaine(fichier);
				if(temp != "absent")
				{
					if (!Racine(temp))
						temp = Combine(PrendreRepertoireProjet(), temp);			
					_nom_fichier_milieu_humide_riverain = temp;
				}

				temp = LireChaine(fichier);
				if(temp != "absent")
				{
					if (!Racine(temp))
						temp = Combine(PrendreRepertoireProjet(), temp);			
					_nom_fichier_milieu_humide_isole = temp;
				}

				temp = LireChaine(fichier);
			}
		}
		//----------------------------

		// simulation disponible
		size_t nb_simulation;
		fichier >> nb_simulation;
		
		for (size_t n = 0; n < nb_simulation; ++n)
		{
			string nom_simulation = LireChaine(fichier);
		}

		// simulation courante
		size_t index_simulation;
		fichier >> index_simulation;

		_nom_simulation = LireChaine(fichier);

		_nom_fichier_simulation = repertoire + "/simulation/" + _nom_simulation + "/" + _nom_simulation + ".sim";
	}

	void SIM_HYD::LectureDonneesPhysiographiques() //appelé lors lecture format 2.6
	{
		_noeuds.Lecture();
		_troncons.LectureTroncons(_zones, _noeuds, false);
		_occupation_sol.Lecture(_zones);

		//lecture des propriete hydrolique
		string tmp;

		ifstream fichier(_nom_fichier_simulation);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION; " + _nom_fichier_simulation);

		fichier.exceptions(ios::failbit | ios::badbit);
		try
		{

		string version;
		getline_mod(fichier, version);

		while (tmp.find("BV3C") == string::npos)
			getline_mod(fichier, tmp);

		for (int n = 0; n < 4; ++n)
			getline_mod(fichier, tmp);

		tmp = LireChaine(fichier);
		
		tmp = LireNomFichier(PrendreRepertoireProjet(), fichier);
		_propriete_hydroliques.ChangeNomFichier(tmp);

		tmp = LireChaine(fichier);

		tmp = LireNomFichier(PrendreRepertoireProjet(), fichier);
		_propriete_hydroliques.ChangeNomFichierCouche1(tmp);

		char ch;
		std::streamoff filepos = fichier.tellg();

		fichier >> ch;	//lit le caractere et remet a la meme position
		fichier.seekg(filepos);

		if(ch == '@')
		{
			tmp = LireNomFichier(PrendreRepertoireProjet(), fichier);
			_propriete_hydroliques.ChangeNomFichierCouche2(tmp);

			tmp = LireNomFichier(PrendreRepertoireProjet(), fichier);
			_propriete_hydroliques.ChangeNomFichierCouche3(tmp);
		}
		else
		{
			_propriete_hydroliques.ChangeNomFichierCouche2(_propriete_hydroliques.PrendreNomFichierCouche1());
			_propriete_hydroliques.ChangeNomFichierCouche3(_propriete_hydroliques.PrendreNomFichierCouche1());
		}

		fichier.close();

		}
		catch (...)
		{
			throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION; " + _nom_fichier_simulation);
		}

		_propriete_hydroliques.Lecture((*this));
	}


	void SIM_HYD::LectureDonneesMeteorologiques()
	{
		_stations_meteo._bAutoInverseTMinTMax = _bAutoInverseTMinTMax;
		_stations_meteo._bStationInterpolation = _bStationInterpolation;
		_stations_meteo._pSimHyd = this;

		_stations_meteo.Lecture(_zones.PrendreProjection());
	}

	void SIM_HYD::LectureDonneesHydrologiques()
	{
		_stations_hydro.Lecture(_zones.PrendreProjection());
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void SIM_HYD::LectureGroupeZone()
	{
		string nom_fichier_groupe = Combine(PrendreRepertoireSimulation(), _nom_simulation + ".gsb");

		ifstream fichier(nom_fichier_groupe);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER GSB (groupe sous-bassin); " + nom_fichier_groupe);

		string str;
		size_t nb_groupe;

		istringstream iss;

		getline_mod(fichier, str);
		iss.clear();
		iss.str(str);
		iss >> nb_groupe;

		vector<GROUPE_ZONE> groupes(nb_groupe);
		
		for (size_t index = 0; index < nb_groupe; ++index)
		{
			string nom_groupe;
			getline_mod(fichier, nom_groupe);
			nom_groupe.erase(std::remove(nom_groupe.begin(), nom_groupe.end(), '\r'), nom_groupe.end());
			groupes[index].ChangeNom(nom_groupe);
		}

		map<int, vector<int>> groupe_zones;
		for (size_t n = 0; n < _zones.PrendreNbZone(); ++n)
		{
			int id_zone, index_groupe;
			fichier >> id_zone >> index_groupe;
			groupe_zones[index_groupe].push_back(id_zone);
		}

		for (auto iter = begin(groupe_zones); iter != end(groupe_zones); ++iter)
			groupes[iter->first].ChangeIdentZones(iter->second);

		_groupes.swap(groupes);

		// creation de la map de recherche
		for (auto iter = begin(_groupes); iter != end(_groupes); ++iter)
			_map_groupes_zone[iter->PrendreNom()] = &(*iter);
	}

	void SIM_HYD::LectureGroupeZoneCorrection()
	{
		string nom_fichier_groupe = Combine(PrendreRepertoireSimulation(), _nom_simulation + ".sbc");

		ifstream fichier(nom_fichier_groupe);
		if (!fichier)
			return;	// throw ERREUR_LECTURE_FICHIER(nom_fichier_groupe);

		size_t nb_groupe;
		fichier >> nb_groupe;

		if(nb_groupe > 0)
		{
			fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			vector<GROUPE_ZONE> groupes(nb_groupe);

			for (size_t index = 0; index < nb_groupe; ++index)
			{
				string nom_groupe;
				getline_mod(fichier, nom_groupe);
				groupes[index].ChangeNom(nom_groupe);
			}

			map<int, vector<int>> groupe_zones;
			for (size_t n = 0; n < _zones.PrendreNbZone(); ++n)
			{
				int id_zone, index_groupe;
				fichier >> id_zone >> index_groupe;

				if(index_groupe != 0)
					groupe_zones[index_groupe-1].push_back(id_zone);
			}

			for (auto iter = begin(groupe_zones); iter != end(groupe_zones); ++iter)
				groupes[iter->first].ChangeIdentZones(iter->second);

			_groupes_correction.swap(groupes);

			// creation de la map de recherche
			for (auto iter = begin(_groupes_correction); iter != end(_groupes_correction); ++iter)
				_map_groupes_correction[iter->PrendreNom()] = &(*iter);
		}
	}

	NOEUDS& SIM_HYD::PrendreNoeuds()
	{
		return _noeuds;
	}

	TRONCONS& SIM_HYD::PrendreTroncons()
	{
		return _troncons;
	}

	ZONES& SIM_HYD::PrendreZones()
	{
		return _zones;
	}

	OCCUPATION_SOL& SIM_HYD::PrendreOccupationSol()
	{
		return _occupation_sol;
	}

	PROPRIETE_HYDROLIQUES& SIM_HYD::PrendreProprieteHydrotliques()
	{
		return _propriete_hydroliques;
	}

	DATE_HEURE SIM_HYD::PrendreDateDebut() const
	{
		return _date_debut;
	}

	DATE_HEURE SIM_HYD::PrendreDateFin() const
	{
		return _date_fin;
	}

	unsigned short SIM_HYD::PrendrePasDeTemps() const
	{
		return _pas_de_temps; 
	}

	DATE_HEURE SIM_HYD::PrendreDateCourante() const
	{
		return _date_courante;
	}

	STATIONS_HYDRO& SIM_HYD::PrendreStationsHydro()
	{
		return _stations_hydro;
	}

	STATIONS_METEO& SIM_HYD::PrendreStationsMeteo()
	{
		return _stations_meteo;
	}

	size_t SIM_HYD::PrendreNbGroupe() const
	{
		return _groupes.size();
	}

	const GROUPE_ZONE& SIM_HYD::PrendreGroupeZone(size_t index) const
	{
		BOOST_ASSERT(index < _groupes.size());
		return _groupes[index];
	}

	string SIM_HYD::PrendreNomProjet() const
	{
		return ExtraitNomFichier(_nom_fichier);
	}

	string SIM_HYD::PrendreNomSimulation() const
	{
		return _nom_simulation;
	}

	string SIM_HYD::PrendreNomInterpolationDonnees() const
	{
		return _interpolation_donnees->PrendreNomSousModele();
	}

	string SIM_HYD::PrendreNomFonteNeige() const
	{
		return _fonte_neige->PrendreNomSousModele();
	}

	string SIM_HYD::PrendreNomFonteGlacier() const
	{
		string nom;

		if(_fonte_glacier)
			nom = _fonte_glacier->PrendreNomSousModele();

		return nom;
	}

	string SIM_HYD::PrendreNomTempSol() const
	{
		string nom;
		
		if(_tempsol)
			nom = _tempsol->PrendreNomSousModele();

		return nom;
	}

	string SIM_HYD::PrendreNomEvapotranspiration() const
	{
		return _evapotranspiration->PrendreNomSousModele();
	}

	string SIM_HYD::PrendreNomBilanVertical() const
	{
		return _bilan_vertical->PrendreNomSousModele();
	}

	string SIM_HYD::PrendreNomRuisselement() const
	{
		return _ruisselement_surface->PrendreNomSousModele();
	}

	string SIM_HYD::PrendreNomAcheminement() const
	{
		return _acheminement_riviere->PrendreNomSousModele();
	}

	int SIM_HYD::PrendreIdentTronconExutoire() const
	{
		return _ident_troncon_exutoire;
	}

	void SIM_HYD::ChangeIdentTronconExutoire(int ident)
	{
		// NOTE: il est valider lors de l'initialisation
		_ident_troncon_exutoire = ident;
	}

	void SIM_HYD::ChangeInterpolationDonnees(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_interpolation_donnees = nullptr;
		}
		else
		{
			auto iter = begin(_vinterpolation_donnees);

			while (iter != end(_vinterpolation_donnees))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_interpolation_donnees = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vinterpolation_donnees))
				throw ERREUR("nom sous modele interpolation introuvable");
		}
	}

	void SIM_HYD::ChangeFonteNeige(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_fonte_neige = nullptr;
		}
		else
		{
			auto iter = begin(_vfonte_neige);

			while (iter != end(_vfonte_neige))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_fonte_neige = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vfonte_neige))
				throw ERREUR("nom sous modele fonte neige introuvable");
		}
	}

	void SIM_HYD::ChangeFonteGlacier(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_fonte_glacier = nullptr;
		}
		else
		{
			auto iter = begin(_vfonte_glacier);

			while (iter != end(_vfonte_glacier))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_fonte_glacier = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vfonte_glacier))
				throw ERREUR("nom sous modele fonte glacier introuvable");
		}
	}

	void SIM_HYD::ChangeTempSol(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_tempsol = nullptr;
		}
		else
		{
			auto iter = begin(_vtempsol);

			while (iter != end(_vtempsol))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_tempsol = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vtempsol))
				throw ERREUR("nom du sous modele de temperature du sol introuvable");
		}
	}

	void SIM_HYD::ChangeEvapotranspiration(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_evapotranspiration = nullptr;
		}
		else
		{
			auto iter = begin(_vevapotranspiration);

			while (iter != end(_vevapotranspiration))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_evapotranspiration = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vevapotranspiration))
				throw ERREUR("nom sous modele evapotranspiration introuvable");
		}
	}

	void SIM_HYD::ChangeBilanVertical(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_bilan_vertical = nullptr;
		}
		else
		{
			auto iter = begin(_vbilan_vertical);

			while (iter != end(_vbilan_vertical))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_bilan_vertical = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vbilan_vertical))
				throw ERREUR("nom sous modele bilan vertical introuvable");
		}
	}

	void SIM_HYD::ChangeRuisselementSurface(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_ruisselement_surface = nullptr;
		}
		else
		{
			auto iter = begin(_vruisselement);

			while (iter != end(_vruisselement))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_ruisselement_surface = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vruisselement))
				throw ERREUR("nom sous modele ruisselement surface introuvable");
		}
	}

	void SIM_HYD::ChangeAcheminementRiviere(const string& nom_sous_modele)
	{
		if (nom_sous_modele.empty())
		{
			_acheminement_riviere = nullptr;
		}
		else
		{
			auto iter = begin(_vacheminement);

			while (iter != end(_vacheminement))
			{
				if (nom_sous_modele == iter->get()->PrendreNomSousModele())
				{
					_acheminement_riviere = iter->get();
					break;
				}

				++iter;
			}

			if (iter == end(_vacheminement))
				throw ERREUR("nom sous modele acheminement riviere introuvable");
		}
	}

	void SIM_HYD::LectureSimulationFormatCsv()
	{
		std::vector<int> coeffsol;

		_bActiveTronconDeconnecte = false;
		_mapIndexTronconDeconnecte.clear();

		ifstream fichier(_nom_fichier_simulation);
		if(!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION; " + _nom_fichier_simulation);

		string cle, valeur, ligne, nom_fichier, sPathFichier, sTemp;
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		if(cle != "SIMULATION HYDROTEL VERSION")
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation, 1);
		}

		std::cout << "Reading simulation file...   " << flush;

		string repertoire = PrendreRepertoireProjet();
		string repertoire_simulation = PrendreRepertoireSimulation();

		coeffsol.resize(_groupes.size(), 0);

		_rayonnementNet._nom_fichier_parametres = Combine(repertoire_simulation, "rayonnement_net.csv");

		DATE_HEURE dtDebutSim, dtFinSim;
		string pas_de_temps;

		istringstream iss;
		vector<std::string> sList;
		size_t x, index;
		int id, iTemp, iCompteur;

		auto degre_jour = static_cast<DEGRE_JOUR_MODIFIE*>(_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_MODIFIE].get());
		auto degre_jour_bande = static_cast<DEGRE_JOUR_BANDE*>(_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_BANDE].get());
		auto rankinen = static_cast<RANKINEN*>(_vtempsol[TEMPSOL_RANKINEN].get());
		auto thorsen = static_cast<THORSEN*>(_vtempsol[TEMPSOL_THORSEN].get());
		auto bv3c1 = static_cast<BV3C1*>(_vbilan_vertical[BILAN_VERTICAL_BV3C1].get());
		auto bv3c2 = static_cast<BV3C2*>(_vbilan_vertical[BILAN_VERTICAL_BV3C2].get());
		auto cequeau = static_cast<CEQUEAU*>(_vbilan_vertical[BILAN_VERTICAL_CEQUEAU].get());
		auto onde_cinematique = static_cast<ONDE_CINEMATIQUE*>(_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE].get());
		auto onde_cinematique_modifiee = static_cast<ONDE_CINEMATIQUE_MODIFIEE*>(_vacheminement[ACHEMINEMENT_ONDE_CINEMATIQUE_MODIFIEE].get());

		iCompteur = 3;

		try{

		while(!fichier.eof() || fichier.bad())
		{
			lire_cle_valeur(fichier, cle, nom_fichier);

			cle = TrimString(cle);
			nom_fichier = TrimString(nom_fichier);
				
			if(cle != "" && nom_fichier != "")
			{
				if(cle == "FICHIER OCCUPATION SOL")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_occupation_sol.ChangeNomFichier(nom_fichier);
				}
				else if (cle == "FICHIER PROPRIETE HYDROLIQUE")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_propriete_hydroliques.ChangeNomFichier(nom_fichier);
				}
				else if (cle == "FICHIER TYPE SOL COUCHE1")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_propriete_hydroliques.ChangeNomFichierCouche1(nom_fichier);
				}
				else if (cle == "FICHIER TYPE SOL COUCHE2")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_propriete_hydroliques.ChangeNomFichierCouche2(nom_fichier);
				}
				else if (cle == "FICHIER TYPE SOL COUCHE3")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_propriete_hydroliques.ChangeNomFichierCouche3(nom_fichier);
				}
				else if (cle == "COEFFICIENT ADDITIF PROPRIETE HYDROLIQUE")
				{
					SplitString(sList, nom_fichier, ";", true, false);

					for(x=0; x<sList.size() && x<coeffsol.size(); x++)
					{
						iss.clear();
						iss.str(sList[x]);
						iss >> id;
						
						coeffsol[x] = id;
					}
					_propriete_hydroliques._coefficient_additif.swap(coeffsol);
				}
				else if (cle == "FICHIER INDICE FOLIERE")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_occupation_sol.ChangeNomFichierIndicesFolieres(nom_fichier);
				}
				else if (cle == "FICHIER PROFONDEUR RACINAIRE")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_occupation_sol.ChangeNomFichierProfondeursRacinaires(nom_fichier);
				}
				//else if (cle == "FICHIER ALBEDO")
				//{
				//	if (!Racine(nom_fichier) )
				//		nom_fichier = Combine(repertoire, nom_fichier);
				//	_occupation_sol.ChangeNomFichierAlbedo(nom_fichier);
				//}
				//else if (cle == "FICHIER HAUTEUR VEGETATION")
				//{
				//	if (!Racine(nom_fichier) )
				//		nom_fichier = Combine(repertoire, nom_fichier);
				//	_occupation_sol.ChangeNomFichierHauteurVegetation(nom_fichier);
				//}
				else if (cle == "FICHIER GRILLE METEO")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_nom_fichier_grille_meteo = nom_fichier;
				}
				else if (cle == "FICHIER STATIONS METEO")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_stations_meteo.ChangeNomFichier(nom_fichier);
				}
				else if (cle == "FICHIER STATIONS HYDRO")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_stations_hydro.ChangeNomFichier(nom_fichier);
				}
				else if (cle == "PREVISION METEO")
				{
					iTemp = string_to_int(nom_fichier);
					if(iTemp == 1)
						_bSimulePrevision = true;
				}
				else if (cle == "FICHIER GRILLE PREVISION")
				{
					if (!Racine(nom_fichier) )
						nom_fichier = Combine(repertoire, nom_fichier);
					_grille_prevision._sPathFichierParam = nom_fichier;
				}
				else if (cle == "DATE DEBUT PREVISION")
				{
					try{ _grille_prevision._date_debut_prevision = DATE_HEURE::Convertie(nom_fichier); }
					catch(const exception& ex)
					{
						throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
					}
				}
				else if (cle == "DATE DEBUT")
				{
					try{ dtDebutSim = DATE_HEURE::Convertie(nom_fichier); }
					catch(const exception& ex)
					{
						throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
					}
				}
				else if (cle == "DATE FIN")
				{
					try{ dtFinSim = DATE_HEURE::Convertie(nom_fichier); }
					catch(const exception& ex)
					{
						throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
					}
				}
				else if (cle == "PAS DE TEMPS")
				{
					pas_de_temps = nom_fichier;
				}
				else if (cle == "TRONCON EXUTOIRE")
				{
					iTemp = string_to_int(nom_fichier);
					ChangeIdentTronconExutoire(iTemp);
				}
				else if (cle == "TRONCONS DECONNECTER")
				{
					SplitString(sList, nom_fichier, ";", true, false);
						
					boost::algorithm::to_lower(sList[0]);
					if(sList[0] == "on")
						_bActiveTronconDeconnecte = true;

					for(x=1; x<sList.size(); x++)
					{
						iss.clear();
						iss.str(sList[x]);
						iss >> id;
						
						try{
							index = _troncons.IdentVersIndex(id);}
						catch(...)
						{
							id = -1;
							std::cout << "Reading disconnected river reach: invalid id " << sList[x] << "." << endl;
						}

						if(id != -1)
							_mapIndexTronconDeconnecte[index] = index;
					}
				}
				else if (cle == "UHRH_TOUTE_PRECIP_EN_PLUIE")
				{
				}
				else if (cle == "NOM FICHIER CORRECTIONS")
				{
					SplitString(sList, nom_fichier, ";", true, false);
					if(sList.size() == 2)
					{
						if(sList[0] != "1")
							_corrections._bActiver = false;
							
						nom_fichier = sList[1];
					}
					else
					{
						if(sList[0].size() > 1)
							nom_fichier = sList[0];
						else
							nom_fichier = "";
					}

					if(nom_fichier != "")
					{
						if (!Racine(nom_fichier))
							nom_fichier = Combine(repertoire_simulation, nom_fichier);
						_corrections.ChangeNomFichier(nom_fichier);
					}
				}

				// fichiers d'etats (lecture)
				else if (cle == "LECTURE ETAT FONTE NEIGE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					degre_jour->ChangeNomFichierLectureEtat(nom_fichier);
					degre_jour_bande->ChangeNomFichierLectureEtat(nom_fichier);
				}
				else if (cle == "LECTURE ETAT TEMPERATURE DU SOL")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);

					rankinen->ChangeNomFichierLectureEtat(nom_fichier);
					thorsen->ChangeNomFichierLectureEtat(nom_fichier);
				}
				else if (cle == "LECTURE ETAT BILAN VERTICAL")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);

					bv3c1->ChangeNomFichierLectureEtat(nom_fichier);
					bv3c2->ChangeNomFichierLectureEtat(nom_fichier);
					cequeau->ChangeNomFichierLectureEtat(nom_fichier);
				}
				else if (cle == "LECTURE ETAT RUISSELEMENT SURFACE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					onde_cinematique->ChangeNomFichierLectureEtat(nom_fichier);
				}
				else if (cle == "LECTURE ETAT ACHEMINEMENT RIVIERE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					onde_cinematique_modifiee->ChangeNomFichierLectureEtat(nom_fichier);
				}
				else if (cle == "ECRITURE ETAT FONTE NEIGE")
				{
					boost::algorithm::to_lower(nom_fichier);
					if (nom_fichier != "tous")
					{
						if(nom_fichier == "fin")
						{
							degre_jour->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
							degre_jour_bande->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
						}
						else
						{
							try{ degre_jour->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier)); }
							catch(const exception& ex)
							{
								throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
							}

							degre_jour_bande->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier));
						}
					}
					else
					{
						degre_jour->ChangeSauvegardeTousEtat(true);
						degre_jour->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
						degre_jour_bande->ChangeSauvegardeTousEtat(true);
						degre_jour_bande->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
					}
				}
				else if (cle == "ECRITURE ETAT TEMPERATURE DU SOL")
				{
					boost::algorithm::to_lower(nom_fichier);
					if (nom_fichier != "tous")
					{
						if(nom_fichier == "fin")
						{
							rankinen->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
							thorsen->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
						}
						else
						{
							try{ rankinen->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier)); }
							catch(const exception& ex)
							{
								throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
							}

							thorsen->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier));
						}
					}
					else
					{
						rankinen->ChangeSauvegardeTousEtat(true);
						rankinen->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
						thorsen->ChangeSauvegardeTousEtat(true);
						thorsen->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
					}
				}
				else if (cle == "ECRITURE ETAT BILAN VERTICAL")
				{
					boost::algorithm::to_lower(nom_fichier);
					if (nom_fichier != "tous")
					{
						if(nom_fichier == "fin")
						{
							bv3c1->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
							bv3c2->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
							cequeau->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
						}
						else
						{
							try{ bv3c1->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier)); }
							catch(const exception& ex)
							{
								throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
							}

							bv3c2->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier));
							cequeau->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier));
						}
					}
					else
					{
						bv3c1->ChangeSauvegardeTousEtat(true);
						bv3c2->ChangeSauvegardeTousEtat(true);
						bv3c1->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
						bv3c2->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
						cequeau->ChangeSauvegardeTousEtat(true);
						cequeau->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
					}
				}
				else if (cle == "ECRITURE ETAT RUISSELEMENT SURFACE")
				{
					boost::algorithm::to_lower(nom_fichier);
					if (nom_fichier != "tous")
					{
						if(nom_fichier == "fin")
							onde_cinematique->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
						else
						{
							try{ onde_cinematique->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier)); }
							catch(const exception& ex)
							{
								throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
							}
						}
					}
					else
					{
						onde_cinematique->ChangeSauvegardeTousEtat(true);
						onde_cinematique->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
					}
				}
				else if (cle == "ECRITURE ETAT ACHEMINEMENT RIVIERE")
				{
					boost::algorithm::to_lower(nom_fichier);
					if (nom_fichier != "tous")
					{
						if(nom_fichier == "fin")
							onde_cinematique_modifiee->ChangeDateHeureSauvegardeEtat(true, dtFinSim);
						else
						{
							try{ onde_cinematique_modifiee->ChangeDateHeureSauvegardeEtat(true, DATE_HEURE::Convertie(nom_fichier)); }
							catch(const exception& ex)
							{
								throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation + ": " + cle + ": " + ex.what());
							}
						}
					}
					else
					{
						onde_cinematique_modifiee->ChangeSauvegardeTousEtat(true);
						onde_cinematique_modifiee->ChangeDateHeureSauvegardeEtat(false, DATE_HEURE());
					}
				}
				else if (cle == "REPERTOIRE ECRITURE ETAT FONTE NEIGE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					degre_jour->ChangeRepertoireEcritureEtat(nom_fichier);
					degre_jour_bande->ChangeRepertoireEcritureEtat(nom_fichier);
				}
				else if (cle == "REPERTOIRE ECRITURE ETAT TEMPERATURE DU SOL")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					rankinen->ChangeRepertoireEcritureEtat(nom_fichier);
					thorsen->ChangeRepertoireEcritureEtat(nom_fichier);
				}
				else if (cle == "REPERTOIRE ECRITURE ETAT BILAN VERTICAL")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					bv3c1->ChangeRepertoireEcritureEtat(nom_fichier);
					bv3c2->ChangeRepertoireEcritureEtat(nom_fichier);
					cequeau->ChangeRepertoireEcritureEtat(nom_fichier);
				}
				else if (cle == "REPERTOIRE ECRITURE ETAT RUISSELEMENT SURFACE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					onde_cinematique->ChangeRepertoireEcritureEtat(nom_fichier);
				}
				else if (cle == "REPERTOIRE ECRITURE ETAT ACHEMINEMENT RIVIERE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					onde_cinematique_modifiee->ChangeRepertoireEcritureEtat(nom_fichier);
				}
				else if (cle == "INTERPOLATION DONNEES")
				{
					sTemp = nom_fichier;
					boost::algorithm::to_lower(sTemp);
					if(sTemp == "thiessen")
					{
						if(_versionTHIESSEN == 1)
							nom_fichier = "THIESSEN1";
						else
							nom_fichier = "THIESSEN2";	//_versionTHIESSEN == 2
					}
					else
					{
						if(sTemp == "moyenne 3 stations")
						{
							if(_versionMOY3STATION == 1)
								nom_fichier = "MOYENNE 3 STATIONS1";
							else
								nom_fichier = "MOYENNE 3 STATIONS2";	//_versionMOY3STATION == 2
						}
					}

					ChangeInterpolationDonnees(nom_fichier);
				}
				else if (cle == "FONTE DE NEIGE")
				{
					ChangeFonteNeige(nom_fichier);
				}
				else if (cle == "FONTE GLACIER")
				{
					ChangeFonteGlacier(nom_fichier);
				}
				else if (cle == "TEMPERATURE DU SOL")
				{
					ChangeTempSol(nom_fichier);
				}
				else if (cle == "EVAPOTRANSPIRATION")
				{
					ChangeEvapotranspiration(nom_fichier);
				}
				else if (cle == "BILAN VERTICAL")
				{
					sTemp = nom_fichier;
					boost::algorithm::to_lower(sTemp);
					if(sTemp == "bv3c")
					{
						if(_versionBV3C == 1)
							nom_fichier = "BV3C1";
						else
							nom_fichier = "BV3C2";	//_versionBV3C == 2
					}
					ChangeBilanVertical(nom_fichier);
				}
				else if (cle == "RUISSELEMENT")
				{
					ChangeRuisselementSurface(nom_fichier);
				}
				else if (cle == "ACHEMINEMENT RIVIERE")
				{
					ChangeAcheminementRiviere(nom_fichier);
				}
				else if (cle == "FICHIER DE PARAMETRE GLOBAL")
				{
					iss.clear();
					iss.str(nom_fichier);
					iss >> _fichierParametreGlobal;
					_nomFichierParametresGlobal = Combine(repertoire_simulation, "parametres_sous_modeles.csv");
				}
				else if (cle == "LECTURE INTERPOLATION DONNEES")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vinterpolation_donnees[INTERPOLATION_LECTURE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "THIESSEN")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);

					_vinterpolation_donnees[INTERPOLATION_THIESSEN1]->ChangeNomFichierParametres(nom_fichier);
					_vinterpolation_donnees[INTERPOLATION_THIESSEN2]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "MOYENNE 3 STATIONS")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
						
					_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS1]->ChangeNomFichierParametres(nom_fichier);
					_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS2]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "LECTURE FONTE NEIGE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vfonte_neige[FONTE_NEIGE_LECTURE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "DEGRE JOUR MODIFIE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_MODIFIE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "LECTURE TEMPERATURE DU SOL")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vtempsol[TEMPSOL_LECTURE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "RANKINEN")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vtempsol[TEMPSOL_RANKINEN]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "THORSEN")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vtempsol[TEMPSOL_THORSEN]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "LECTURE EVAPOTRANSPIRATION")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ETP_LECTURE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "HYDRO-QUEBEC")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ETP_HYDRO_QUEBEC]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "THORNTHWAITE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ETP_THORNTHWAITE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "LINACRE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ETP_LINACRE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "PENMAN")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ETP_PENMAN]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "PRIESTLAY-TAYLOR" || cle == "PRIESTLAY TAYLOR")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ETP_PRIESTLAY_TAYLOR]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "PENMAN-MONTEITH")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ETP_PENMAN_MONTEITH]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "ETP-MC-GUINESS")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vevapotranspiration[ENUM_ETP_MC_GUINESS]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "LECTURE BILAN VERTICAL")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vbilan_vertical[BILAN_VERTICAL_LECTURE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "BV3C")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vbilan_vertical[BILAN_VERTICAL_BV3C1]->ChangeNomFichierParametres(nom_fichier);
					_vbilan_vertical[BILAN_VERTICAL_BV3C2]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "CEQUEAU")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vbilan_vertical[BILAN_VERTICAL_CEQUEAU]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "LECTURE RUISSELEMENT SURFACE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vruisselement[RUISSELEMENT_LECTURE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "ONDE CINEMATIQUE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "LECTURE ACHEMINEMENT RIVIERE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vacheminement[ACHEMINEMENT_LECTURE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "ONDE CINEMATIQUE MODIFIEE")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_vacheminement[ACHEMINEMENT_ONDE_CINEMATIQUE_MODIFIEE]->ChangeNomFichierParametres(nom_fichier);
				}
				else if (cle == "FICHIER MILIEUX HUMIDES ISOLES")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_nom_fichier_milieu_humide_isole = nom_fichier;
				}
				else if (cle == "MILIEUX HUMIDES ISOLES")
				{
					if(nom_fichier.size() > 0 && (nom_fichier[0] == '0' || nom_fichier[0] == '1'))
					{
						if(nom_fichier[0] == '1')
							_bSimuleMHIsole = true;
					}
					else
					{
						//ancien format
						if (!Racine(nom_fichier))
							nom_fichier = Combine(repertoire_simulation, nom_fichier);
						_nom_fichier_milieu_humide_isole = nom_fichier;

						if(_nom_fichier_milieu_humide_isole != "")
							_bSimuleMHIsole = true;
					}
				}
				else if (cle == "FICHIER MILIEUX HUMIDES RIVERAINS")
				{
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire_simulation, nom_fichier);
					_nom_fichier_milieu_humide_riverain = nom_fichier;
				}
				else if (cle == "MILIEUX HUMIDES RIVERAINS")
				{
					if(nom_fichier.size() > 0 && (nom_fichier[0] == '0' || nom_fichier[0] == '1'))
					{
						if(nom_fichier[0] == '1')
							_bSimuleMHRiverain = true;
					}
					else
					{
						//ancien format
						if (!Racine(nom_fichier))
							nom_fichier = Combine(repertoire_simulation, nom_fichier);
						_nom_fichier_milieu_humide_riverain = nom_fichier;

						if(_nom_fichier_milieu_humide_riverain != "")
							_bSimuleMHRiverain = true;
					}
				}
				else if (cle == "MILIEUX HUMIDES PROFONDEUR TRONCONS")
				{
					//ancien format
					if (!Racine(nom_fichier))
						nom_fichier = Combine(repertoire, nom_fichier);
					_nom_fichier_milieu_humide_profondeur_troncon = nom_fichier;
				}
			}

			++iCompteur;
		}

		}
		catch(const ERREUR_LECTURE_FICHIER& elf)
		{
			fichier.close();
			throw elf;
		}		
		catch(...)
		{
			if(!fichier.eof())
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: " + _nom_fichier_simulation);
			}
		}		

		fichier.close();

		if(dtDebutSim >= dtFinSim)
			throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION: la date de fin de la simulation doit etre superieur a la date de debut.");
		else
			ChangeParametresTemporels(dtDebutSim, dtFinSim, string_to_ushort(pas_de_temps));

		//nom fichier de parametre fixé par defaut
		nom_fichier = Combine(repertoire_simulation, "etp-mc-guiness.csv");
		_vevapotranspiration[ENUM_ETP_MC_GUINESS]->ChangeNomFichierParametres(nom_fichier);

		nom_fichier = Combine(repertoire_simulation, "degre-jour-glacier.csv");
		_vfonte_glacier[FONTE_GLACIER_DEGRE_JOUR_GLACIER]->ChangeNomFichierParametres(nom_fichier);

		nom_fichier = Combine(repertoire_simulation, "degre-jour-bande.csv");
		_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_BANDE]->ChangeNomFichierParametres(nom_fichier);

		//repertoire par defaut pour ecriture fichiers etats (repertoire du projet)
		if(degre_jour->PrendreRepertoireEcritureEtat() == "")
		{
			degre_jour->ChangeRepertoireEcritureEtat(repertoire);
			degre_jour_bande->ChangeRepertoireEcritureEtat(repertoire);
		}

		if(rankinen->PrendreRepertoireEcritureEtat() == "")
		{
			rankinen->ChangeRepertoireEcritureEtat(repertoire);
			thorsen->ChangeRepertoireEcritureEtat(repertoire);
		}

		if(bv3c1->PrendreRepertoireEcritureEtat() == "")
		{
			bv3c1->ChangeRepertoireEcritureEtat(repertoire);
			bv3c2->ChangeRepertoireEcritureEtat(repertoire);
			cequeau->ChangeRepertoireEcritureEtat(repertoire);
		}

		if(onde_cinematique->PrendreRepertoireEcritureEtat() == "")
			onde_cinematique->ChangeRepertoireEcritureEtat(repertoire);

		if(onde_cinematique_modifiee->PrendreRepertoireEcritureEtat() == "")
			onde_cinematique_modifiee->ChangeRepertoireEcritureEtat(repertoire);

		std::cout << "ok" << endl;
	}


	void SIM_HYD::LectureSimulationFormatSim()
	{
		istringstream iss;
		size_t index;
		int iVal;

		ifstream fichier(_nom_fichier_simulation);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER SIMULATION; " + _nom_fichier_simulation);

		fichier.exceptions(ios::failbit | ios::badbit);

		try
		{
			vector<size_t> index_eaux;
			string version, tmp;
			float temp;

			getline_mod(fichier, version);

			//if (!regex_match(version, regex("HYDROTEL V46")) && 
			//	!regex_match(version, regex("HYDROTEL V47")) && 
			//	!regex_match(version, regex("HYDROTEL V49")) &&
			//	!regex_match(version, regex("HYDROTEL V50")) )
			//	throw ERREUR_LECTURE_FICHIER(_nom_fichier);

			if(version.size() >= 12)
			{
				if(version.substr(0, 10) != "HYDROTEL V")
					throw ERREUR_LECTURE_FICHIER(_nom_fichier);
			}
			else
				throw ERREUR_LECTURE_FICHIER(_nom_fichier);

			getline_mod(fichier, tmp);

			fichier >> temp >> temp;

			unsigned short annee, jour_julien, heure;
			fichier >> annee >> jour_julien >> heure;

			DATE_HEURE date_debut(annee, jour_julien, heure);

			fichier >> annee >> jour_julien >> heure;

			DATE_HEURE date_fin(annee, jour_julien, heure);

			unsigned short pas_de_temps;
			fichier >> pas_de_temps;

			ChangeParametresTemporels(date_debut, date_fin, pas_de_temps);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			while (tmp.find("PRECIPITATION") == string::npos)
				getline_mod(fichier, tmp);
			iss.clear();
			iss.str(tmp.substr(tmp.length()-1, 1));	//obtient l'index du modele selectionné
			iss >> iVal;
			switch(iVal)
			{
			case 0:
				ChangeInterpolationDonnees("LECTURE INTERPOLATION DONNEES");
				break;
			case 1:
				ChangeInterpolationDonnees("THIESSEN1");
				break;
			case 2:
				ChangeInterpolationDonnees("MOYENNE 3 STATIONS1");
				break;
			default:
				throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation);
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			while (tmp.find("THIESSEN") == string::npos)
				getline_mod(fichier, tmp);

			for (int n = 0; n < 11; ++n)
				getline_mod(fichier, tmp);

			for (size_t index_groupe = 0; index_groupe < _groupes.size(); ++index_groupe)
			{
				fichier >> index;

				float passage_pluie_neige, gradient_precipitation, gradient_temperature;
				fichier >> passage_pluie_neige >> temp >> temp >> temp;
				fichier >> gradient_precipitation >> temp >> temp >> temp;
				fichier >> gradient_temperature >> temp >> temp >> temp;

				for (index = 0; index < _groupes[index_groupe].PrendreNbZone(); ++index)
				{
					int ident = _groupes[index_groupe].PrendreIdent(index);
					size_t index_zone = _zones.IdentVersIndex(ident);

					_smThiessen1->ChangePassagePluieNeige(index_zone, passage_pluie_neige);
					_smThiessen1->ChangeGradientPrecipitation(index_zone, gradient_precipitation);
					_smThiessen1->ChangeGradientTemperature(index_zone, gradient_temperature);
				}
			}

			//----------------------------------------------------------------------
			while (tmp.find("MOYENNE_3_STATIONS") == string::npos)
				getline_mod(fichier, tmp);

			for (int n = 0; n < 11; ++n)
				getline_mod(fichier, tmp);

			for (size_t index_groupe = 0; index_groupe < _groupes.size(); ++index_groupe)
			{
				fichier >> index;

				float passage_pluie_neige, gradient_precipitation, gradient_temperature;
				fichier >> passage_pluie_neige >> temp >> temp >> temp;
				fichier >> gradient_precipitation >> temp >> temp >> temp;
				fichier >> gradient_temperature >> temp >> temp >> temp;

				for (index = 0; index < _groupes[index_groupe].PrendreNbZone(); ++index)
				{
					int ident = _groupes[index_groupe].PrendreIdent(index);
					size_t index_zone = _zones.IdentVersIndex(ident);

					_smMoy3station1->ChangePassagePluieNeige(index_zone, passage_pluie_neige);
					_smMoy3station1->ChangeGradientPrecipitation(index_zone, gradient_precipitation);
					_smMoy3station1->ChangeGradientTemperature(index_zone, gradient_temperature);
				}
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			while (tmp.find("FONTE_DE_NEIGE") == string::npos)
				getline_mod(fichier, tmp);
			iss.clear();
			iss.str(tmp.substr(tmp.length()-1, 1));	//obtient l'index du modele selectionné
			iss >> iVal;
			switch(iVal)
			{
			case 0:
				ChangeFonteNeige("LECTURE FONTE NEIGE");
				break;
			case 1:
				ChangeFonteNeige("DEGRE JOUR MODIFIE");
				break;
			default:
				throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation);
			}
			
			//----------------------------------------------------------------------
			while (tmp.find("DEGRE_JOUR_MODIFIEE") == string::npos)
				getline_mod(fichier, tmp);

			for (int n = 0; n < 4; ++n)
				getline_mod(fichier, tmp);

			DEGRE_JOUR_MODIFIE* degre_jour = static_cast<DEGRE_JOUR_MODIFIE*>(_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_MODIFIE].get());

			for (size_t index_groupe = 0; index_groupe < _groupes.size(); ++index_groupe)
			{
				fichier >> index;

				float taux_fonte_conifers, taux_fonte_feuillus, taux_fonte_decouver;
				float seuil_conifers, seuil_feuillus, seuil_decouver;
				float taux_fonte, densite, constante_tassement;

				fichier >> temp >> taux_fonte_conifers >> taux_fonte_feuillus >> taux_fonte_decouver;
				fichier >> seuil_conifers >> seuil_feuillus >> seuil_decouver;
				fichier >> taux_fonte >> densite >> constante_tassement;
				fichier >> temp >> temp;

				for (index = 0; index < _groupes[index_groupe].PrendreNbZone(); ++index)
				{
					int ident = _groupes[index_groupe].PrendreIdent(index);
					size_t index_zone = _zones.IdentVersIndex(ident);

					degre_jour->ChangeTauxFonteConifers(index_zone, taux_fonte_conifers);
					degre_jour->ChangeTauxFonteFeuillus(index_zone, taux_fonte_feuillus);
					degre_jour->ChangeTauxFonteDecouver(index_zone, taux_fonte_decouver);

					degre_jour->ChangeSeuilFonteConifers(index_zone, seuil_conifers);
					degre_jour->ChangeSeuilFonteFeuillus(index_zone, seuil_feuillus);
					degre_jour->ChangeSeuilFonteDecouver(index_zone, seuil_decouver);

					degre_jour->ChangeTauxFonte(index_zone, taux_fonte);
					degre_jour->ChangeDesiteMaximale(index_zone, densite);
					degre_jour->ChangeConstanteTassement(index_zone, constante_tassement);
				}
			}

			int cot_conifer, cot_feuillus;
			fichier >> cot_conifer >> cot_feuillus;

			vector<size_t> index_feuillus, index_conifers;
			int cot = 1;
			for (index = 0; index < _occupation_sol.PrendreNbClasse(); ++index)
			{
				if ((cot_conifer & cot) == cot)
					index_conifers.push_back(index);

				if ((cot_feuillus & cot) == cot)
					index_feuillus.push_back(index);

				cot = cot * 2;
			}

			degre_jour->ChangeIndexOccupationConifers(index_conifers);
			degre_jour->ChangeIndexOccupationFeuillus(index_feuillus);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			while (tmp.find("EVAPOTRANSPIRATION_POTENTIEL") == string::npos)
				getline_mod(fichier, tmp);
			iss.clear();
			iss.str(tmp.substr(tmp.length()-1, 1));	//obtient l'index du modele selectionné
			iss >> iVal;
			switch(iVal)
			{
			case 0:
				ChangeEvapotranspiration("LECTURE EVAPOTRANSPIRATION");
				break;
			case 1:
				ChangeEvapotranspiration("THORNTHWAITE");
				break;
			case 2:
				ChangeEvapotranspiration("LINACRE");
				break;
			case 3:
				ChangeEvapotranspiration("PENMAN");
				break;
			case 4:
				ChangeEvapotranspiration("PRIESTLAY-TAYLOR");
				break;
			case 5:
				ChangeEvapotranspiration("HYDRO-QUEBEC");
				break;
			default:
				throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation);
			}
			
			//----------------------------------------------------------------------
			while (tmp.find("THORNTHWAITE") == string::npos)
				getline_mod(fichier, tmp);

			THORNTHWAITE* thornthwaite = static_cast<THORNTHWAITE*>(_vevapotranspiration[ETP_THORNTHWAITE].get());

			getline_mod(fichier, tmp);

			float indice_thermique, latitude;
			int facteur_dephasage;

			fichier >> indice_thermique >> latitude >> facteur_dephasage;

			for (size_t index_groupe = 0; index_groupe < _groupes.size(); ++index_groupe)
			{
				for (index = 0; index < _groupes[index_groupe].PrendreNbZone(); ++index)
				{
					int ident = _groupes[index_groupe].PrendreIdent(index);
					size_t index_zone = _zones.IdentVersIndex(ident);

					thornthwaite->ChangeIndiceThermique(index_zone, indice_thermique);
					thornthwaite->ChangeFacteurDephasage(index_zone, facteur_dephasage);
				}
			}

			//----------------------------------------------------------------------
			while (tmp.find("HYDRO_QUEBEC") == string::npos)
				getline_mod(fichier, tmp);

			HYDRO_QUEBEC* hydro_quebec = static_cast<HYDRO_QUEBEC*>(_vevapotranspiration[ETP_HYDRO_QUEBEC].get());

			for (int n = 0; n < 1; ++n)
				getline_mod(fichier, tmp);

			for (size_t index_groupe = 0; index_groupe < _groupes.size(); ++index_groupe)
			{
				for (int n = 0; n <= 4; ++n)
					fichier >> temp;

				float coefficient;
				fichier >> temp >> coefficient;

				for (index = 0; index < _groupes[index_groupe].PrendreNbZone(); ++index)
				{
					int ident = _groupes[index_groupe].PrendreIdent(index);
					size_t index_zone = _zones.IdentVersIndex(ident);

					hydro_quebec->ChangeCoefficientMultiplicatif(index_zone, coefficient);
				}
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			while (tmp.find("BILAN_VERTICAL") == string::npos)
				getline_mod(fichier, tmp);
			iss.clear();
			iss.str(tmp.substr(tmp.length()-1, 1));	//obtient l'index du modele selectionné
			iss >> iVal;
			switch(iVal)
			{
			case 0:
				ChangeBilanVertical("LECTURE BILAN VERTICAL");
				break;
			case 2:
				ChangeBilanVertical("BV3C1");
				break;
			default:
				throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation);
			}

			//----------------------------------------------------------------------
			while (tmp.find("BV3C") == string::npos)
				getline_mod(fichier, tmp);

			BV3C1* bv3c1 = static_cast<BV3C1*>(_vbilan_vertical[BILAN_VERTICAL_BV3C1].get());

			for (int n = 0; n < 4; ++n)
				getline_mod(fichier, tmp);

			_occupation_sol.ChangeNomFichierIndicesFolieres(LireNomFichier(PrendreRepertoireProjet(), fichier));

			tmp = LireChaine(fichier);

			_occupation_sol.ChangeNomFichierProfondeursRacinaires(LireNomFichier(PrendreRepertoireProjet(), fichier));

			tmp = LireChaine(fichier);	//1ere ligne typesol

			//certaine version plus recente ont 3 lignes pour fichier typesol...
			char ch;
			std::streamoff filepos = fichier.tellg();

			fichier >> ch;	//lit le caractere et remet a la meme position
			fichier.seekg(filepos);

			if(ch == '@')	//sinon; ont est deja a la bonne position
			{
				//lit les 2 lignes suivantes contenant les fichiers typesol
				tmp = LireChaine(fichier);	//2e ligne typesol
				tmp = LireChaine(fichier);	//3e ligne typesol
			}

			for (size_t index_groupe = 0; index_groupe < _groupes.size(); ++index_groupe)
			{
				fichier >> index;

				float theta1, theta2, theta3, z1, z2, z3, krec, des, fCin;

				fichier >> theta1 >> theta2 >> theta3 >> z1 >> z2 >> z3 >> krec >> des >> fCin;

				for (index = 0; index < _groupes[index_groupe].PrendreNbZone(); ++index)
				{
					int ident = _groupes[index_groupe].PrendreIdent(index);
					size_t index_zone = _zones.IdentVersIndex(ident);

					bv3c1->ChangeTheta1Initial(index_zone, theta1);
					bv3c1->ChangeTheta2Initial(index_zone, theta2);
					bv3c1->ChangeTheta3Initial(index_zone, theta3);

					_zones[index_zone].ChangeZ1Z2Z3(z1, z2, z3);

					bv3c1->ChangeKrec(index_zone, krec);
					bv3c1->ChangeDes(index_zone, des);
					bv3c1->ChangeCin(index_zone, fCin);
				}
			}

			int cot_eau, cot_impermeable;
			fichier >> cot_eau >> cot_impermeable;

			vector<size_t> index_impermeables;
			cot = 1;
			for (index = 0; index < _occupation_sol.PrendreNbClasse(); ++index)
			{
				if ((cot_eau & cot) == cot)
					index_eaux.push_back(index);

				if ((cot_impermeable & cot) == cot)
					index_impermeables.push_back(index);

				cot = cot * 2;
			}

			bv3c1->ChangeIndexEaux(index_eaux);
			bv3c1->ChangeIndexImpermeables(index_impermeables);

			for (int n = 0; n <= 17; ++n)
				getline_mod(fichier, tmp);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			while (tmp.find("RUISSELLEMENT_SURFACE") == string::npos)
				getline_mod(fichier, tmp);
			iss.clear();
			iss.str(tmp.substr(tmp.length()-1, 1));	//obtient l'index du modele selectionné
			iss >> iVal;
			switch(iVal)
			{
			case 0:
				ChangeRuisselementSurface("LECTURE RUISSELEMENT SURFACE");
				break;
			case 2:
				ChangeRuisselementSurface("ONDE CINEMATIQUE");
				break;
			default:
				throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation);
			}
			
			//----------------------------------------------------------------------
			while (tmp.find("RU_OC_ZONE") == string::npos)
				getline_mod(fichier, tmp);

			ONDE_CINEMATIQUE* onde_cinematique = static_cast<ONDE_CINEMATIQUE*>(_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE].get());

			onde_cinematique->ChangeNomFichierHGM( LireNomFichier(PrendreRepertoireProjet(), fichier, 1) );

			for (int n = 0; n < 5; ++n)
				getline_mod(fichier, tmp);

			float lame;
			fichier >> lame; // NOTE: la lame est identique pour toute les zones

			onde_cinematique->ChangeLame(lame);

			for (size_t index_groupe = 0; index_groupe < _groupes.size(); ++index_groupe)
			{
				float manning_forets, manning_eaux, manning_autres;
				fichier >> manning_forets >> manning_eaux >> manning_autres;

				for (index = 0; index < _groupes[index_groupe].PrendreNbZone(); ++index)
				{
					int ident = _groupes[index_groupe].PrendreIdent(index);
					size_t index_zone = _zones.IdentVersIndex(ident);

					onde_cinematique->ChangeManningForet(index_zone, manning_forets);
					onde_cinematique->ChangeManningEau(index_zone, manning_eaux);
					onde_cinematique->ChangeManningAutre(index_zone, manning_autres);
				}
			}

			int cot_forets, cot_eaux;
			fichier >> cot_forets >> cot_eaux;

			{
				vector<size_t> index_forets;
				index_eaux.clear();
				cot = 1;
				for (index = 0; index < _occupation_sol.PrendreNbClasse(); ++index)
				{
					if ((cot_eau & cot) == cot)
						index_eaux.push_back(index);

					if ((cot_forets & cot) == cot)
						index_forets.push_back(index);

					cot = cot * 2;
				}

				onde_cinematique->ChangeIndexForets(index_forets);
				onde_cinematique->ChangeIndexEaux(index_eaux);
			}

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			while (tmp.find("TRANSFERT_RIVIERE") == string::npos)
				getline_mod(fichier, tmp);
			iss.clear();
			iss.str(tmp.substr(tmp.length()-1, 1));	//obtient l'index du modele selectionné
			iss >> iVal;
			switch(iVal)
			{
			case 0:
				ChangeAcheminementRiviere("LECTURE ACHEMINEMENT RIVIERE");
				break;
			case 1:
				ChangeAcheminementRiviere("ONDE CINEMATIQUE MODIFIEE");
				break;
			default:
				throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation);
			}

		}
		catch (...)
		{
			throw ERREUR_LECTURE_FICHIER(_nom_fichier_simulation);
		}
	}


	string SIM_HYD::PrendreRepertoireProjet() const
	{
		return PrendreRepertoire(_nom_fichier);
	}

	string SIM_HYD::PrendreRepertoireSimulation() const
	{
		return Combine(PrendreRepertoireProjet(), Combine("simulation", _nom_simulation));
	}

	string SIM_HYD::PrendreRepertoireResultat() const
	{
		return Combine(PrendreRepertoireSimulation(), "resultat");
	}


	void SIM_HYD::Initialise()	
	{
		std::vector<size_t> vect;
		size_t i, j, index;
		string str;
		double dTotalArea;

		_bHGMCalculer = false;

		_occupation_sol.clear();

		//supprime le contenu du repertoire qui contiendra les resultats
		if(boost::filesystem::exists(PrendreRepertoireResultat()))
		{
			if(!DeleteFolderContent(PrendreRepertoireResultat()))
				throw ERREUR("Error deleting result folder: " + PrendreRepertoireResultat());
		}
		else
			CreeRepertoire(PrendreRepertoireResultat());

		//
		InitListeTronconsZonesSimules();

		//open weighted avg output file
		if(_output._weighted_avg)
		{
			_wavg_idtroncon.clear();
			_wavg_zones.clear();
			_wavg_zones_weighting.clear();
			_wavg_fichier.clear();

			for (i=0; i<_output._wavg_IdTronconMoyPond.size(); i++)
			{
				//obtient la liste des uhrhs amont au troncon
				if (find(_troncons_simules_ident.begin(), _troncons_simules_ident.end(), _output._wavg_IdTronconMoyPond[i]) != _troncons_simules_ident.end())
				{
					//si le troncon est simulé
					TRONCON* troncon_exutoire = _troncons.RechercheTroncon(_output._wavg_IdTronconMoyPond[i]);

					vector<TRONCON*> troncons;
					troncons.push_back(troncon_exutoire);

					std::vector<size_t> zonesIndex;
					std::vector<size_t> zonesIndex2;

					while (!troncons.empty())
					{
						TRONCON* troncon = troncons.back();
						troncons.pop_back();

						vector<ZONE*> zones_amont = troncon->PrendreZonesAmont();
						for(j=0; j<zones_amont.size(); j++)
							zonesIndex.push_back(_zones.IdentVersIndex(zones_amont[j]->PrendreIdent()));

						vector<TRONCON*> troncons_amont = troncon->PrendreTronconsAmont();
						if (!troncons_amont.empty())
							troncons.insert(troncons.end(), troncons_amont.begin(), troncons_amont.end());
					}

					//enleve les zones s'il ne sont pas dans la liste des zones simulé
					//pour developement futur; sera util dans le cas ou on exclue un/des sous-bassin en amont de l'exutoire
					for(j=0; j<zonesIndex.size(); j++)
					{
						if(find(_zones_simules.begin(), _zones_simules.end(), zonesIndex[j]) != _zones_simules.end())
							zonesIndex2.push_back(zonesIndex[j]);
					}

					if(zonesIndex2.size() != 0)
					{
						_wavg_idtroncon.push_back(_output._wavg_IdTronconMoyPond[i]);
						_wavg_zones.push_back(zonesIndex2);
					}
				}
			}

			if(_wavg_idtroncon.size() == 0)
				_output._weighted_avg = false;
			else
			{
				for (i=0; i<_wavg_idtroncon.size(); i++)
				{
					//init ponderation
					std::vector<float> vWeight;

					dTotalArea = 0.0;
					for (j=0; j<_wavg_zones[i].size(); j++)
						dTotalArea+= _zones[_wavg_zones[i][j]].PrendreSuperficie();

					for (j=0; j<_wavg_zones[i].size(); j++)
						vWeight.push_back(static_cast<float>(_zones[_wavg_zones[i][j]].PrendreSuperficie() / dTotalArea));

					_wavg_zones_weighting.push_back(vWeight);

					//init fichier output
					ostringstream oss;

					oss.str("");
					oss << "moyennes-ponderees-troncon" << _wavg_idtroncon[i] << ".csv";

					string nom_fichier( Combine(PrendreRepertoireResultat(), oss.str()) );

					std::ofstream file;
					file.open(nom_fichier.c_str());

					_wavg_fichier.push_back(std::move(file));

					_wavg_fichier[i] << "Moyennes pondérées" << PrendreOutput().Separator() << "( VERSION " << HYDROTEL_VERSION << " )" << endl;
					_wavg_fichier[i] << "Troncon" << PrendreOutput().Separator() << _wavg_idtroncon[i] << endl;
					
					//uhrh amont list
					_wavg_fichier[i] << "UHRH amont" << PrendreOutput().Separator();

					std::vector<int> zonesIdents;
					for(j=0; j<_wavg_zones[i].size(); j++)
						zonesIdents.push_back(_zones[_wavg_zones[i][j]].PrendreIdent());

					std::sort(zonesIdents.begin(), zonesIdents.end());

					oss.str("");
					for (j=0; j<zonesIdents.size(); j++)
						oss << zonesIdents[j] << PrendreOutput().Separator();

					str = oss.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_wavg_fichier[i] << str << endl;

					//
					_wavg_fichier[i] << "Date heure" << PrendreOutput().Separator() << "TMin (C)" << PrendreOutput().Separator() << "TMax (C)" << PrendreOutput().Separator() << "TMoy (C)" << 
						PrendreOutput().Separator() << "Précip pluie (mm)" << PrendreOutput().Separator() << "Précip neige (EEN) (mm)" << 
						PrendreOutput().Separator() << "Couvert nival (EEN) (mm)" << PrendreOutput().Separator() << "ETP (mm)" << PrendreOutput().Separator() << "ETR (mm)";
					
					if(_fonte_glacier)
					{
						_wavg_fichier[i] << PrendreOutput().Separator() << "Apport glacier total (mm)";
						_wavg_fichier[i] << PrendreOutput().Separator() << "Équivalent eau glacier total (m)";
					}
						
					_wavg_fichier[i] << endl;
				}
			}
		}

		//Mode lecture

		_bLectInterpolation = _bLectNeige = _bLectTempsol = _bLectEvap = _bLectBilan = _bLectRuissellement = _bLectAcheminement = false;

		if (_acheminement_riviere && _acheminement_riviere->PrendreNomSousModele() == "LECTURE ACHEMINEMENT RIVIERE")
			_bLectAcheminement = true;

		if (_ruisselement_surface && _ruisselement_surface->PrendreNomSousModele() == "LECTURE RUISSELEMENT SURFACE")
			_bLectRuissellement = true;

		if (_bilan_vertical && _bilan_vertical->PrendreNomSousModele() == "LECTURE BILAN VERTICAL")
			_bLectBilan = true;

		if (_evapotranspiration && _evapotranspiration->PrendreNomSousModele() == "LECTURE EVAPOTRANSPIRATION")
			_bLectEvap = true;

		if (_tempsol && _tempsol->PrendreNomSousModele() == "LECTURE TEMPERATURE DU SOL")
			_bLectTempsol = true;

		if (_fonte_neige && _fonte_neige->PrendreNomSousModele() == "LECTURE FONTE NEIGE")
			_bLectNeige = true;

		if (_interpolation_donnees && _interpolation_donnees->PrendreNomSousModele() == "LECTURE INTERPOLATION DONNEES")
			_bLectInterpolation = true;

		if(_evapotranspiration && !_bLectEvap && (PrendreNomEvapotranspiration() == "PENMAN" || PrendreNomEvapotranspiration() == "PENMAN-MONTEITH" || PrendreNomEvapotranspiration() == "PRIESTLAY-TAYLOR"))
			_bRayonnementNet = true;
		else
			_bRayonnementNet = false;

		//initialization for NetCDF output
		if (_outputCDF)
		{
			//uhrh
			if(_output._bOutputUhrhVar)
			{
				vect.clear();

				i = 0;
				for (index=0; index<_zones.PrendreNbZone(); index++)
				{
					if (find(begin(_zones_simules), end(_zones_simules), index) != end(_zones_simules))
					{
						if (_output._bSauvegardeTous || find(begin(_output._vIdTronconSelect), end(_output._vIdTronconSelect), _zones[index].PrendreTronconAval()->PrendreIdent()) != end(_output._vIdTronconSelect))
						{
							vect.push_back(index);
							++i;
						}
					}
				}

				_output._uhrhOutputNb = vect.size();
				_output._uhrhOutputIndex = new size_t[_output._uhrhOutputNb];
				_output._uhrhOutputIDs = new int[_output._uhrhOutputNb];

				for (i=0; i<_output._uhrhOutputNb; i++)
				{
					_output._uhrhOutputIndex[i] = vect[i];
					_output._uhrhOutputIDs[i] = _zones[_output._uhrhOutputIndex[i]].PrendreIdent();
				}
			}

			//troncon
			if (_output._bOutputTronconVar)
			{
				vect.clear();

				i = 0;			
				for (index=0; index<_troncons.PrendreNbTroncon(); index++)
				{
					if (find(begin(_troncons_simules), end(_troncons_simules), index) != end(_troncons_simules))
					{
						if (_output._bSauvegardeTous || find(begin(_output._vIdTronconSelect), end(_output._vIdTronconSelect), _troncons[index]->PrendreIdent()) != end(_output._vIdTronconSelect))
						{
							vect.push_back(index);
							++i;
						}
					}
				}

				_output._tronconOutputNb = vect.size();
				_output._tronconOutputIndex = new size_t[_output._tronconOutputNb];
				_output._tronconOutputIDs = new int[_output._tronconOutputNb];

				for(i=0; i<_output._tronconOutputNb; i++)
				{
					_output._tronconOutputIndex[i] = vect[i];
					_output._tronconOutputIDs[i] = _troncons[_output._tronconOutputIndex[i]]->PrendreIdent();
				}
			}
		}
		
		//sub model initialisation
		if (_interpolation_donnees)
		{
			if(_bSimulePrevision && !_bLectInterpolation)
			{
				if(_grille_prevision._date_debut_prevision < _date_debut || _grille_prevision._date_debut_prevision > _date_fin)
					throw ERREUR("GRILLE_PREVISION; la date de debut des previsions meteorologique est invalide.");

				_grille_prevision._sim_hyd = this;
				_grille_prevision.Initialise();
			}

			if(! (_bSimulePrevision && _grille_prevision._date_debut_prevision == _date_debut && !_bLectInterpolation) )
			{
				if(_interpolation_donnees->PrendreNomSousModele() == "THIESSEN2")
				{
					_stations_meteo._fGradientStationTemp = _smThiessen2->_gradient_station_temp;
					_stations_meteo._fGradientStationPrecip = _smThiessen2->_gradient_station_precip;
				}
				else
				{
					if(_interpolation_donnees->PrendreNomSousModele() == "MOYENNE 3 STATIONS2")
					{
						_stations_meteo._fGradientStationTemp = _smMoy3station2->_gradient_station_temp;
						_stations_meteo._fGradientStationPrecip = _smMoy3station2->_gradient_station_precip;
					}
				}

				_interpolation_donnees->Initialise();
			}
		}

		if (_fonte_neige)
			_fonte_neige->Initialise();

		if (_fonte_glacier)
			_fonte_glacier->Initialise();

		if (_tempsol)
			_tempsol->Initialise();

		if (_evapotranspiration)
			_evapotranspiration->Initialise();

		if (_bilan_vertical)
			_bilan_vertical->Initialise();

		if (_ruisselement_surface)
			_ruisselement_surface->Initialise();

		if (_acheminement_riviere)
			_acheminement_riviere->Initialise();

		_date_courante = _date_debut;
		_lPasTempsCourantIndex = 0;
	}


	void SIM_HYD::Calcule()
	{
		BOOST_ASSERT(_date_courante < _date_fin);

		double dVol, dSuperficieGlaceM1Total;
		size_t index_troncon, nbTroncon, nbZone, i, j;
		float fWeight, fTMin, fTMax, fTMoy, densite, neige;

		// initialise les variables cumulant les resultats
		for (index_troncon = 0; index_troncon < _troncons.PrendreNbTroncon(); ++index_troncon)
		{
			_troncons[index_troncon]->ChangeApportLateral(0.0f);
			
			_troncons[index_troncon]->_surf = 0.0f;
			_troncons[index_troncon]->_hypo = 0.0f;
			_troncons[index_troncon]->_base = 0.0f;
		}

		//
		if (_interpolation_donnees)
		{
			if(!_bLectInterpolation && _bSimulePrevision && _date_courante >= _grille_prevision._date_debut_prevision)
			{
				_grille_prevision.Calcule();
				_interpolation_donnees->INTERPOLATION_DONNEES::Calcule();
			}
			else
				_interpolation_donnees->Calcule();
		}

		if (_fonte_neige)
			_fonte_neige->Calcule();

		if (_fonte_glacier)
			_fonte_glacier->Calcule();

		if (_tempsol)
			_tempsol->Calcule();

		if (_evapotranspiration)
			_evapotranspiration->Calcule();

		if (_bilan_vertical)
			_bilan_vertical->Calcule();

		if (_ruisselement_surface)
			_ruisselement_surface->Calcule();

		if (_acheminement_riviere)
			_acheminement_riviere->Calcule();

		if(_output._weighted_avg)
		{
			DEGRE_JOUR_GLACIER* ptr = NULL;
			if(_fonte_glacier)
				ptr = dynamic_cast<DEGRE_JOUR_GLACIER*>(_fonte_glacier);

			nbTroncon = _wavg_idtroncon.size();
			for(i=0; i<nbTroncon; i++)
			{
				_output._wavg_TMin = 0.0f;
				_output._wavg_TMax = 0.0f;
				_output._wavg_TMoy = 0.0f;
				_output._wavg_Pluie = 0.0f;
				_output._wavg_Neige = 0.0f;
				_output._wavg_CouvertNival = 0.0f;
				_output._wavg_ETP = 0.0f;
				_output._wavg_ETR = 0.0f;
				
				_output._wavg_TotalApportGlace = 0.0f;
				_output._wavg_TotalEquiEauGlace = 0.0f;

				dVol = 0.0;
				dSuperficieGlaceM1Total = 0.0;
				nbZone = _wavg_zones[i].size();

				for(j=0; j<nbZone; j++)
				{
					fTMin = _zones[_wavg_zones[i][j]].PrendreTMin();

					if (_pas_de_temps == 1)
						fTMoy = fTMax = fTMin;
					else
					{
						fTMax = _zones[_wavg_zones[i][j]].PrendreTMax();
						fTMoy = (fTMin + fTMax) / 2.0f;
					}

					densite = CalculDensiteNeige(fTMoy) / DENSITE_EAU;
					neige = _zones[_wavg_zones[i][j]].PrendreNeige() * densite;		//transforme la neige en equivalent en eau

					fWeight = _wavg_zones_weighting[i][j];

					_output._wavg_TMin+= fTMin * fWeight;
					_output._wavg_TMax+= fTMax * fWeight;
					_output._wavg_TMoy+= fTMoy * fWeight;
					_output._wavg_Pluie+= _zones[_wavg_zones[i][j]].PrendrePluie() * fWeight;
					_output._wavg_Neige+= neige * fWeight;
					_output._wavg_CouvertNival+= _zones[_wavg_zones[i][j]].PrendreCouvertNival() * fWeight;
					_output._wavg_ETP+= _zones[_wavg_zones[i][j]].PrendreEtpTotal() * fWeight;
					_output._wavg_ETR+= _zones[_wavg_zones[i][j]].PrendreEtrTotal() * fWeight;

					if(_fonte_glacier)
					{
						_output._wavg_TotalApportGlace+= _zones[_wavg_zones[i][j]].PrendreApportGlacier() * fWeight;
						
						if(ptr != NULL)
						{
							dVol = _zones[_wavg_zones[i][j]].PrendreEauGlacier() * ptr->_superficieUhrhM1[_wavg_zones[i][j]] * 1000.0 / ptr->_densite_glace / 1000000.0;	//[hm3]
							_output._wavg_TotalEquiEauGlace+= dVol;

							dSuperficieGlaceM1Total+= ptr->_superficieUhrhM1[_wavg_zones[i][j]];
						}
					}
				}

				ostringstream oss;
				string str;

				oss.str("");
			
				oss << PrendreDateCourante() << PrendreOutput().Separator() << setprecision(PrendreOutput()._nbDigit_dC) << setiosflags(ios::fixed) << 
					_output._wavg_TMin << PrendreOutput().Separator() << _output._wavg_TMax << PrendreOutput().Separator() << _output._wavg_TMoy << PrendreOutput().Separator() << 
					setprecision(PrendreOutput()._nbDigit_mm) << setiosflags(ios::fixed) << _output._wavg_Pluie << PrendreOutput().Separator() << _output._wavg_Neige << PrendreOutput().Separator() << 
					_output._wavg_CouvertNival << PrendreOutput().Separator() << _output._wavg_ETP << PrendreOutput().Separator() << _output._wavg_ETR;

				if(_fonte_glacier)
				{
					oss << PrendreOutput().Separator() << _output._wavg_TotalApportGlace;
					
					if(dSuperficieGlaceM1Total != 0.0)
						_output._wavg_TotalEquiEauGlace = ((_output._wavg_TotalEquiEauGlace * 1000000.0) * ptr->_densite_glace) / 1000.0 / dSuperficieGlaceM1Total;

					oss << PrendreOutput().Separator() << setprecision(_output._nbDigit_m) << setiosflags(ios::fixed) << _output._wavg_TotalEquiEauGlace;	//[m]
				}
			
				str = oss.str();
				_wavg_fichier[i] << str << endl;
			}
		}

		_date_courante+= _pas_de_temps;
		++_lPasTempsCourantIndex;
	}


	void SIM_HYD::Termine()
	{
		if(PrendreOutput()._weighted_avg)
		{
			for(size_t i=0; i<_wavg_idtroncon.size(); i++)
				_wavg_fichier[i].close();
		}

		if (_interpolation_donnees)
			_interpolation_donnees->Termine();

		if (_fonte_neige)
			_fonte_neige->Termine();

		if (_fonte_glacier)
			_fonte_glacier->Termine();

		if (_tempsol)
			_tempsol->Termine();

		if (_evapotranspiration)
			_evapotranspiration->Termine();

		if (_bilan_vertical)
			_bilan_vertical->Termine();

		if (_ruisselement_surface)
			_ruisselement_surface->Termine();

		if (_acheminement_riviere)
			_acheminement_riviere->Termine();
	}


	void SIM_HYD::ChangeParametresTemporels(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps)
	{
		BOOST_ASSERT(debut < fin);

		_date_debut = debut;
		_date_fin = fin;

		_pas_de_temps = pas_de_temps;
	}

	void SIM_HYD::SauvegardeFichierProjet()
	{
		ofstream fichier(_nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(_nom_fichier);

		fichier << "PROJET HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		string repertoire_projet = PrendreRepertoireProjet();

		// DONNEES PHYSIOGRAPHIQUES

		fichier << "FICHIER ALTITUDE;"			<< PrendreRepertoireRelatif(repertoire_projet, _zones.PrendreNomFichierAltitude()) << endl;
		fichier << "FICHIER PENTE;"				<< PrendreRepertoireRelatif(repertoire_projet, _zones.PrendreNomFichierPente()) << endl;
		fichier << "FICHIER ORIENTATION;"		<< PrendreRepertoireRelatif(repertoire_projet, _zones.PrendreNomFichierOrientation()) << endl;
		fichier << "FICHIER ZONE;"				<< PrendreRepertoireRelatif(repertoire_projet, _zones.PrendreNomFichierZone()) << endl;
		fichier << "FICHIER NOEUD;"				<< PrendreRepertoireRelatif(repertoire_projet, _noeuds.PrendreNomFichier()) << endl;
		fichier << "FICHIER TRONCON;"			<< PrendreRepertoireRelatif(repertoire_projet, _troncons.PrendreNomFichier()) << endl;
		fichier << "FICHIER PIXELS;"			<< PrendreRepertoireRelatif(repertoire_projet, _troncons.PrendreNomFichierPixels()) << endl;
		fichier << "FICHIER MILIEUX HUMIDES PROFONDEUR TRONCONS;"			<< PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_milieu_humide_profondeur_troncon) << endl;
		fichier << endl;

		// SIMULATION COURANTE

		fichier << "SIMULATION COURANTE;" << PrendreNomSimulation() << endl;
	}

	void SIM_HYD::SauvegardeFichierSimulation()
	{
		ostringstream oss;
		size_t x;
		int iID;

		string repertoire_projet = PrendreRepertoireProjet();
		string repertoire_simulation = PrendreRepertoireSimulation();
		string nom_fichier = Combine(PrendreRepertoireSimulation(), PrendreNomSimulation() + ".csv");
		string sString;

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "SIMULATION HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;	

		// DONNEES PHYSIOGRAPHIQUES

		fichier << "FICHIER OCCUPATION SOL;"	    << PrendreRepertoireRelatif(repertoire_projet, _occupation_sol.PrendreNomFichier()) << endl;
		fichier << "FICHIER PROPRIETE HYDROLIQUE;"	<< PrendreRepertoireRelatif(repertoire_projet, _propriete_hydroliques.PrendreNomFichier()) << endl;
		fichier << "FICHIER TYPE SOL COUCHE1;"		<< PrendreRepertoireRelatif(repertoire_projet, _propriete_hydroliques.PrendreNomFichierCouche1()) << endl;
		fichier << "FICHIER TYPE SOL COUCHE2;"		<< PrendreRepertoireRelatif(repertoire_projet, _propriete_hydroliques.PrendreNomFichierCouche2()) << endl;
		fichier << "FICHIER TYPE SOL COUCHE3;"		<< PrendreRepertoireRelatif(repertoire_projet, _propriete_hydroliques.PrendreNomFichierCouche3()) << endl;
		fichier << endl;

		oss << "COEFFICIENT ADDITIF PROPRIETE HYDROLIQUE;";
		for(x=0; x<_groupes.size(); x++)
		{
			oss << _propriete_hydroliques._coefficient_additif[x];
			oss << ";";
		}

		fichier << oss.str() << endl;
		fichier << endl;

		fichier << "FICHIER INDICE FOLIERE;"        << PrendreRepertoireRelatif(repertoire_projet, _occupation_sol.PrendreNomFichierIndicesFolieres()) << endl;
		fichier << "FICHIER PROFONDEUR RACINAIRE;"  << PrendreRepertoireRelatif(repertoire_projet, _occupation_sol.PrendreNomFichierProfondeursRacinaires()) << endl;
		//fichier << "FICHIER ALBEDO;"				<< PrendreRepertoireRelatif(repertoire_projet, _occupation_sol.PrendreNomFichierAlbedo()) << endl;
		//fichier << "FICHIER HAUTEUR VEGETATION;"	<< PrendreRepertoireRelatif(repertoire_projet, _occupation_sol.PrendreNomFichierHauteurVegetation()) << endl;
		fichier << endl;

		// DONNEES HYDROMETEO

		fichier << "FICHIER GRILLE METEO;"	<< PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_grille_meteo) << endl;
		fichier << "FICHIER STATIONS METEO;"	<< PrendreRepertoireRelatif(repertoire_projet, _stations_meteo.PrendreNomFichier()) << endl;
		fichier << "FICHIER STATIONS HYDRO;"	<< PrendreRepertoireRelatif(repertoire_projet, _stations_hydro.PrendreNomFichier()) << endl;
		fichier << endl;

		if(_bSimulePrevision)
			fichier << "PREVISION METEO;1" << endl;
		else
			fichier << "PREVISION METEO;0" << endl;

		fichier << "FICHIER GRILLE PREVISION;"	<< PrendreRepertoireRelatif(repertoire_projet, _grille_prevision._sPathFichierParam) << endl;

		fichier << "DATE DEBUT PREVISION;"	<< _grille_prevision._date_debut_prevision << endl;
		
		fichier << endl;		

		// DONNEES TEMPORELS

		fichier << "DATE DEBUT;" << _date_debut << endl;
		fichier << "DATE FIN;" << _date_fin << endl;
		fichier << "PAS DE TEMPS;" << _pas_de_temps << endl;
		fichier << endl;

		// TRONCON EXUTOIRE

		fichier << "TRONCON EXUTOIRE;" << PrendreIdentTronconExutoire() << endl;
		fichier << endl;

		// TRONCONS DECONNECTER

		oss.str("");
		if(_bActiveTronconDeconnecte)
			oss << "TRONCONS DECONNECTER;on;";
		else
			oss << "TRONCONS DECONNECTER;off;";

		for(x=0; x<_mapIndexTronconDeconnecte.size(); x++)
		{
			iID = _zones[_mapIndexTronconDeconnecte.at(x)].PrendreIdent();
			oss << iID << ";";
		}

		fichier << oss.str() << endl;
		fichier << endl;

		// FICHIER DE CORRECTION

		if(_corrections._bActiver)
			sString = "1;";
		else
			sString = "0;";

		fichier << "NOM FICHIER CORRECTIONS;" << sString << PrendreRepertoireRelatif(repertoire_simulation, _corrections.PrendreNomFichier()) << endl;
		fichier << endl;

		// VARIABLES ETAT

		fichier << "LECTURE ETAT FONTE NEIGE;" << endl;				// [NOM FICHIER/vide pour ne pas lire]
		fichier << "LECTURE ETAT TEMPERATURE DU SOL;" << endl;		// [NOM FICHIER/vide pour ne pas lire]
		fichier << "LECTURE ETAT BILAN VERTICAL;" << endl;			// [NOM FICHIER/vide pour ne pas lire]
		fichier << "LECTURE ETAT RUISSELEMENT SURFACE;" << endl;	// [NOM FICHIER/vide pour ne pas lire]
		fichier << "LECTURE ETAT ACHEMINEMENT RIVIERE;" << endl;	// [NOM FICHIER/vide pour ne pas lire]
		fichier << endl;

		fichier << "ECRITURE ETAT FONTE NEIGE;" << endl;			// [DATE D'ECRITURE/TOUS/vide pour ne pas ecrire]
		fichier << "ECRITURE ETAT TEMPERATURE DU SOL;" << endl;		// [DATE D'ECRITURE/TOUS/vide pour ne pas ecrire]
		fichier << "ECRITURE ETAT BILAN VERTICAL;" << endl;			// [DATE D'ECRITURE / TOUS/vide pour ne pas ecrire]
		fichier << "ECRITURE ETAT RUISSELEMENT SURFACE;" << endl;	// [DATE D'ECRITURE / TOUS/vide pour ne pas ecrire]
		fichier << "ECRITURE ETAT ACHEMINEMENT RIVIERE;" << endl;	// [DATE D'ECRITURE / TOUS/vide pour ne pas ecrire]
		fichier << endl;

		fichier << "REPERTOIRE ECRITURE ETAT FONTE NEIGE;" << endl;				// [repertoire relatif]
		fichier << "REPERTOIRE ECRITURE ETAT TEMPERATURE DU SOL;" << endl;		// [repertoire relatif]
		fichier << "REPERTOIRE ECRITURE ETAT BILAN VERTICAL;" << endl;			// [repertoire relatif]
		fichier << "REPERTOIRE ECRITURE ETAT RUISSELEMENT SURFACE;" << endl;	// [repertoire relatif]
		fichier << "REPERTOIRE ECRITURE ETAT ACHEMINEMENT RIVIERE;" << endl;	// [repertoire relatif]
		fichier << endl;

		//SOUS MODELES SELECTIONNES

		if(_interpolation_donnees == nullptr)	//valeur par defaut si null
			ChangeInterpolationDonnees("MOYENNE 3 STATIONS2");
		
		if(PrendreNomInterpolationDonnees() == "THIESSEN1" || PrendreNomInterpolationDonnees() == "THIESSEN2")
			fichier << "INTERPOLATION DONNEES;" << "THIESSEN" << endl;
		else
		{
			if(PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS1" || PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS2")
				fichier << "INTERPOLATION DONNEES;" << "MOYENNE 3 STATIONS" << endl;
			else
				fichier << "INTERPOLATION DONNEES;" << PrendreNomInterpolationDonnees() << endl;
		}

		if(_fonte_neige == nullptr)
			ChangeFonteNeige("DEGRE JOUR MODIFIE");
		fichier << "FONTE DE NEIGE;" << PrendreNomFonteNeige() << endl;

		if(_fonte_glacier)
			fichier << "FONTE DE GLACIER;" << PrendreNomFonteGlacier() << endl;
		else
			fichier << "FONTE DE GLACIER;" << endl;
		
		if(_tempsol)
			fichier << "TEMPERATURE DU SOL;" << PrendreNomTempSol() << endl;
		else
			fichier << "TEMPERATURE DU SOL;" << endl;

		if(_evapotranspiration == nullptr)
			ChangeEvapotranspiration("HYDRO-QUEBEC");
		fichier << "EVAPOTRANSPIRATION;" << PrendreNomEvapotranspiration() << endl;

		if(_bilan_vertical == nullptr)
			ChangeBilanVertical("BV3C2");	//valeur par defaut si null

		if(PrendreNomBilanVertical() == "BV3C1" || PrendreNomBilanVertical() == "BV3C2")
			fichier << "BILAN VERTICAL;" << "BV3C" << endl;
		else
			fichier << "BILAN VERTICAL;" << PrendreNomBilanVertical() << endl;

		if(_ruisselement_surface == nullptr)
			ChangeRuisselementSurface("ONDE CINEMATIQUE");
		fichier << "RUISSELEMENT;" << PrendreNomRuisselement() << endl;
		
		if(_acheminement_riviere == nullptr)
			ChangeAcheminementRiviere("ONDE CINEMATIQUE MODIFIEE");
		fichier << "ACHEMINEMENT RIVIERE;" << PrendreNomAcheminement() << endl;
		fichier << endl;

		if(_bSimuleMHIsole)
			fichier << "MILIEUX HUMIDES ISOLES;1" << endl;
		else
			fichier << "MILIEUX HUMIDES ISOLES;0" << endl;

		if(_bSimuleMHRiverain)
			fichier << "MILIEUX HUMIDES RIVERAINS;1" << endl;
		else
			fichier << "MILIEUX HUMIDES RIVERAINS;0" << endl;
		fichier << endl;

		// FICHIERS PARAMETRES DES SOUS MODELES

		fichier << "FICHIER DE PARAMETRE GLOBAL;" << _fichierParametreGlobal << endl << endl;

		for (auto iter = begin(_vinterpolation_donnees); iter != end(_vinterpolation_donnees); ++iter)
		{
			if((*iter)->PrendreNomSousModele() != "GRILLE")
			{
				if((*iter)->PrendreNomSousModele() == "THIESSEN1")
					fichier << "THIESSEN" << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;
				else
				{
					if((*iter)->PrendreNomSousModele() == "MOYENNE 3 STATIONS1")
						fichier << "MOYENNE 3 STATIONS" << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;
					else
					{
						if((*iter)->PrendreNomSousModele() != "THIESSEN2" && (*iter)->PrendreNomSousModele() != "MOYENNE 3 STATIONS2")	//parameters file name must be writen only one time... (so we skip version 2 in the _vinterpolation_donnees vector)
							fichier << (*iter)->PrendreNomSousModele() << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;
					}
				}
			}
		}

		for (auto iter = begin(_vfonte_neige); iter != end(_vfonte_neige); ++iter)
			fichier << (*iter)->PrendreNomSousModele() << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;

		for (auto iter = begin(_vtempsol); iter != end(_vtempsol); ++iter)
			fichier << (*iter)->PrendreNomSousModele() << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;

		for (auto iter = begin(_vevapotranspiration); iter != end(_vevapotranspiration); ++iter)
			fichier << (*iter)->PrendreNomSousModele() << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;

		for (auto iter = begin(_vbilan_vertical); iter != end(_vbilan_vertical); ++iter)
		{
			if((*iter)->PrendreNomSousModele() == "BV3C1")
				fichier << "BV3C" << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;
			else
			{
				if((*iter)->PrendreNomSousModele() != "BV3C2")	//parameters file name must be writen only one time... (so we skip version 2 in the _vbilan_vertical vector)
					fichier << (*iter)->PrendreNomSousModele() << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;
			}
		}

		for (auto iter = begin(_vruisselement); iter != end(_vruisselement); ++iter)
			fichier << (*iter)->PrendreNomSousModele() << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;

		for (auto iter = begin(_vacheminement); iter != end(_vacheminement); ++iter)
			fichier << (*iter)->PrendreNomSousModele() << ';' << PrendreRepertoireRelatif(repertoire_simulation, (*iter)->PrendreNomFichierParametres()) << endl;

		// FICHIERS PARAMETRES POUR LES MILIEUX HUMIDE

		string tmp, tmp2, tmp3, nomProjet;
		nomProjet = PrendreNomProjet();
		tmp2 = nomProjet + "/physio/";
		boost::algorithm::to_lower(tmp2);

		tmp = tmp3 = PrendreRepertoireRelatif(repertoire_simulation, _nom_fichier_milieu_humide_isole);
		boost::algorithm::to_lower(tmp3);
		if(tmp3.substr(0, 7 + nomProjet.size() + 1) == tmp2)
			tmp.erase(0, 7 + + nomProjet.size() + 1);
		fichier << "FICHIER MILIEUX HUMIDES ISOLES;" << tmp << endl;

		tmp = tmp3 = PrendreRepertoireRelatif(repertoire_simulation, _nom_fichier_milieu_humide_riverain);
		boost::algorithm::to_lower(tmp3);
		if(tmp3.substr(0, 7 + nomProjet.size() + 1) == tmp2)
			tmp.erase(0, 7 + + nomProjet.size() + 1);
		fichier << "FICHIER MILIEUX HUMIDES RIVERAINS;" << tmp << endl;
	}

	void SIM_HYD::SauvegardeInterpolationDonnees()
	{
		for (auto iter = begin(_vinterpolation_donnees); iter != end(_vinterpolation_donnees); ++iter)
		{
			if((*iter)->PrendreNomSousModele() != "THIESSEN1" && (*iter)->PrendreNomSousModele() != "THIESSEN2" && (*iter)->PrendreNomSousModele() != "MOYENNE 3 STATIONS1" && (*iter)->PrendreNomSousModele() != "MOYENNE 3 STATIONS2")
				(*iter)->SauvegardeParametres();
		}

		if(_versionTHIESSEN == 1)
			_vinterpolation_donnees[INTERPOLATION_THIESSEN1]->SauvegardeParametres();
		else
			_vinterpolation_donnees[INTERPOLATION_THIESSEN2]->SauvegardeParametres();

		if(_versionMOY3STATION == 1)
			_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS1]->SauvegardeParametres();
		else
			_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS2]->SauvegardeParametres();
	}

	void SIM_HYD::SauvegardeFonteNeige()
	{
		for (auto iter = begin(_vfonte_neige); iter != end(_vfonte_neige); ++iter)
			(*iter)->SauvegardeParametres();
	}

	void SIM_HYD::SauvegardeFonteGlacier()
	{
		for (auto iter = begin(_vfonte_glacier); iter != end(_vfonte_glacier); ++iter)
			(*iter)->SauvegardeParametres();
	}

	void SIM_HYD::SauvegardeTempSol()
	{
		for (auto iter = begin(_vtempsol); iter != end(_vtempsol); ++iter)
			(*iter)->SauvegardeParametres();
	}

	void SIM_HYD::SauvegardeEtp()
	{
		for (auto iter = begin(_vevapotranspiration); iter != end(_vevapotranspiration); ++iter)
			(*iter)->SauvegardeParametres();
	}

	void SIM_HYD::SauvegardeBilanVertical()
	{
		for (auto iter = begin(_vbilan_vertical); iter != end(_vbilan_vertical); ++iter)
		{
			if((*iter)->PrendreNomSousModele() != "BV3C1" && (*iter)->PrendreNomSousModele() != "BV3C2")
				(*iter)->SauvegardeParametres();
		}

		if(_versionBV3C == 1)
			_vbilan_vertical[BILAN_VERTICAL_BV3C1]->SauvegardeParametres();
		else
			_vbilan_vertical[BILAN_VERTICAL_BV3C2]->SauvegardeParametres();
	}

	void SIM_HYD::SauvegardeRuisselement()
	{
		for (auto iter = begin(_vruisselement); iter != end(_vruisselement); ++iter)
			(*iter)->SauvegardeParametres();
	}

	void SIM_HYD::SauvegradeAcheminementRiviere()
	{
		for (auto iter = begin(_vacheminement); iter != end(_vacheminement); ++iter)
			(*iter)->SauvegardeParametres();
	}

	void SIM_HYD::SauvegardeSous(string repertoire)
	{
		_nom_fichier = Combine(repertoire, PrendreNomProjet() + ".csv");

		string str;
		
		string repertoire_physitel = Combine(repertoire, "physitel");

		_zones.ChangeNomFichierAltitude( RemplaceRepertoire(_zones.PrendreNomFichierAltitude(), repertoire_physitel) );
		_zones.ChangeNomFichierOrientation( RemplaceRepertoire(_zones.PrendreNomFichierOrientation(), repertoire_physitel) );
		_zones.ChangeNomFichierPente( RemplaceRepertoire(_zones.PrendreNomFichierPente(), repertoire_physitel) );
		_zones.ChangeNomFichierZone( RemplaceRepertoire(_zones.PrendreNomFichierZone(), repertoire_physitel) );

		_troncons.ChangeNomFichier( RemplaceRepertoire(_troncons.PrendreNomFichier(), repertoire_physitel) );
		_troncons.ChangeNomFichierPixels( RemplaceRepertoire(_troncons.PrendreNomFichierPixels(), repertoire_physitel) );

		_noeuds.ChangeNomFichier( RemplaceRepertoire(_noeuds.PrendreNomFichier(), repertoire_physitel) );

		_occupation_sol.ChangeNomFichier( RemplaceRepertoire(_occupation_sol.PrendreNomFichier(), repertoire_physitel) );

		str = _occupation_sol.PrendreNomFichierIndicesFolieres();
		if(str == "" || (str.length() >= 8 && str.substr(str.length()-1-6) == "/absent"))
			_occupation_sol.ChangeNomFichierIndicesFolieres(repertoire + "/physio/ind_fol.def");
		else
			_occupation_sol.ChangeNomFichierIndicesFolieres( RemplaceRepertoire(_occupation_sol.PrendreNomFichierIndicesFolieres(), repertoire_physitel) );

		str = _occupation_sol.PrendreNomFichierProfondeursRacinaires();
		if(str == "" || (str.length() >= 8 && str.substr(str.length()-1-6) == "/absent"))
			_occupation_sol.ChangeNomFichierProfondeursRacinaires(repertoire + "/physio/pro_rac.def");
		else
			_occupation_sol.ChangeNomFichierProfondeursRacinaires( RemplaceRepertoire(_occupation_sol.PrendreNomFichierProfondeursRacinaires(), repertoire_physitel) );

		_propriete_hydroliques.ChangeNomFichier( RemplaceRepertoire(_propriete_hydroliques.PrendreNomFichier(), repertoire_physitel) );	
		_propriete_hydroliques.ChangeNomFichierCouche1( RemplaceRepertoire(_propriete_hydroliques.PrendreNomFichierCouche1(), repertoire_physitel) );
		_propriete_hydroliques.ChangeNomFichierCouche2( RemplaceRepertoire(_propriete_hydroliques.PrendreNomFichierCouche2(), repertoire_physitel) );
		_propriete_hydroliques.ChangeNomFichierCouche3( RemplaceRepertoire(_propriete_hydroliques.PrendreNomFichierCouche3(), repertoire_physitel) );

		string repertoire_meteo = Combine(repertoire, "meteo");
		
		_stations_meteo.ChangeNomFichier( RemplaceRepertoire(_stations_meteo.PrendreNomFichier(), repertoire_meteo) );
		for (size_t index = 0; index < _stations_meteo.PrendreNbStation(); ++index)
			_stations_meteo[index]->ChangeNomFichier( RemplaceRepertoire(_stations_meteo[index]->PrendreNomFichier(), repertoire_meteo) );

		string repertoire_hydro = Combine(repertoire, "hydro");
		
		_stations_hydro.ChangeNomFichier( RemplaceRepertoire(_stations_hydro.PrendreNomFichier(), repertoire_hydro) );
		for (size_t index = 0; index < _stations_hydro.PrendreNbStation(); ++index)
			_stations_hydro[index]->ChangeNomFichier( RemplaceRepertoire(_stations_hydro[index]->PrendreNomFichier(), repertoire_hydro) );

		string repertoire_physio = Combine(repertoire, "physio");

		_occupation_sol.ChangeNomFichierIndicesFolieres( RemplaceRepertoire(_occupation_sol.PrendreNomFichierIndicesFolieres(), repertoire_physio) );
		_occupation_sol.ChangeNomFichierProfondeursRacinaires( RemplaceRepertoire(_occupation_sol.PrendreNomFichierProfondeursRacinaires(), repertoire_physio) );

		string repertoire_hgm = Combine(repertoire, "hgm");

		ONDE_CINEMATIQUE* onde_cinematique = static_cast<ONDE_CINEMATIQUE*>(_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE].get());
		onde_cinematique->ChangeNomFichierHGM( RemplaceRepertoire(onde_cinematique->PrendreNomFichierHgm(), repertoire_hgm) );

		string repertoire_simulation = PrendreRepertoireSimulation();

		// cree le repertoire pour la simulation courante
		CreeRepertoire( repertoire_simulation );

		_vinterpolation_donnees[INTERPOLATION_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_interpolation.csv") );
		_vinterpolation_donnees[INTERPOLATION_THIESSEN1]->ChangeNomFichierParametres( Combine(repertoire_simulation, "thiessen.csv") );
		_vinterpolation_donnees[INTERPOLATION_THIESSEN2]->ChangeNomFichierParametres( Combine(repertoire_simulation, "thiessen.csv") );
		_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS1]->ChangeNomFichierParametres( Combine(repertoire_simulation, "moyenne_3_stations.csv") );
		_vinterpolation_donnees[INTERPOLATION_MOYENNE_3_STATIONS2]->ChangeNomFichierParametres( Combine(repertoire_simulation, "moyenne_3_stations.csv") );

		_vfonte_neige[FONTE_NEIGE_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_fonte_neige.csv") );
		_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_MODIFIE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "degre_jour_modifie.csv") );
		_vfonte_neige[FONTE_NEIGE_DEGRE_JOUR_BANDE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "degre-jour-bande.csv") );

		_vfonte_glacier[FONTE_GLACIER_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_fonte_glacier.csv") );
		_vfonte_glacier[FONTE_GLACIER_DEGRE_JOUR_GLACIER]->ChangeNomFichierParametres( Combine(repertoire_simulation, "degre-jour-glacier.csv") );

		_vtempsol[TEMPSOL_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_tempsol.csv") );
		_vtempsol[TEMPSOL_RANKINEN]->ChangeNomFichierParametres( Combine(repertoire_simulation, "rankinen.csv") );
		_vtempsol[TEMPSOL_THORSEN]->ChangeNomFichierParametres( Combine(repertoire_simulation, "thorsen.csv") );

		_rayonnementNet._nom_fichier_parametres = Combine(repertoire_simulation, "rayonnement_net.csv");

		_vevapotranspiration[ETP_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_etp.csv") );
		_vevapotranspiration[ETP_HYDRO_QUEBEC]->ChangeNomFichierParametres( Combine(repertoire_simulation, "hydro_quebec.csv") );
		_vevapotranspiration[ETP_THORNTHWAITE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "thornthwaite.csv") );
		_vevapotranspiration[ETP_LINACRE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "linacre.csv") );
		_vevapotranspiration[ETP_PENMAN]->ChangeNomFichierParametres( Combine(repertoire_simulation, "penman.csv") );
		_vevapotranspiration[ETP_PRIESTLAY_TAYLOR]->ChangeNomFichierParametres( Combine(repertoire_simulation, "priestlay_taylor.csv") );
		_vevapotranspiration[ETP_PENMAN_MONTEITH]->ChangeNomFichierParametres( Combine(repertoire_simulation, "penman_monteith.csv") );
		_vevapotranspiration[ENUM_ETP_MC_GUINESS]->ChangeNomFichierParametres( Combine(repertoire_simulation, "etp-mc-guiness.csv") );

		_vbilan_vertical[BILAN_VERTICAL_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_bilan_vertical.csv") );
		_vbilan_vertical[BILAN_VERTICAL_BV3C1]->ChangeNomFichierParametres( Combine(repertoire_simulation, "bv3c.csv") );
		_vbilan_vertical[BILAN_VERTICAL_BV3C2]->ChangeNomFichierParametres( Combine(repertoire_simulation, "bv3c.csv") );
		_vbilan_vertical[BILAN_VERTICAL_CEQUEAU]->ChangeNomFichierParametres( Combine(repertoire_simulation, "cequeau.csv") );

		_vruisselement[RUISSELEMENT_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_ruisselement.csv") );
		_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "onde_cinematique.csv") );

		_vacheminement[ACHEMINEMENT_LECTURE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "lecture_acheminement.csv") );
		_vacheminement[ACHEMINEMENT_ONDE_CINEMATIQUE_MODIFIEE]->ChangeNomFichierParametres( Combine(repertoire_simulation, "onde_cinematique_modifiee.csv") );

		Sauvegarde();
	}


	void SIM_HYD::Sauvegarde()
	{
		SauvegardeFichierProjet();

		//parametres des sous modeles
		SauvegardeInterpolationDonnees();
		SauvegardeFonteNeige();
		SauvegardeFonteGlacier();
		SauvegardeTempSol();
		_rayonnementNet.SauvegardeParametres();
		SauvegardeEtp();
		SauvegardeBilanVertical();
		SauvegardeRuisselement();
		SauvegradeAcheminementRiviere();

		SauvegardeFichierSimulation();
	}


	void SIM_HYD::LectureInterpolationDonnees()
	{
		if(! (_bSimulePrevision && _grille_prevision._date_debut_prevision == _date_debut) )
		{
			for (auto iter = begin(_vinterpolation_donnees); iter != end(_vinterpolation_donnees); ++iter)
			{
				if(PrendreNomInterpolationDonnees() == (*iter)->PrendreNomSousModele())
					(*iter)->LectureParametres();
			}
		}

		_grille_prevision._sim_hyd = this;
		_grille_prevision.LectureParametres();
	}

	void SIM_HYD::LectureFonteNeige()
	{
		for (auto iter = begin(_vfonte_neige); iter != end(_vfonte_neige); ++iter)
		{
			if(PrendreNomFonteNeige() == (*iter)->PrendreNomSousModele())
				(*iter)->LectureParametres();
		}
	}

	void SIM_HYD::LectureFonteGlacier()
	{
		for (auto iter = begin(_vfonte_glacier); iter != end(_vfonte_glacier); ++iter)
		{
			if(PrendreNomFonteGlacier() == (*iter)->PrendreNomSousModele())
				(*iter)->LectureParametres();
		}
	}

	void SIM_HYD::LectureTempSol()
	{
		for (auto iter = begin(_vtempsol); iter != end(_vtempsol); ++iter)
		{
			if(PrendreNomTempSol() == (*iter)->PrendreNomSousModele())
				(*iter)->LectureParametres();
		}
	}

	void SIM_HYD::LectureEtp()
	{
		for (auto iter = begin(_vevapotranspiration); iter != end(_vevapotranspiration); ++iter)
		{
			if(PrendreNomEvapotranspiration() == (*iter)->PrendreNomSousModele())
				(*iter)->LectureParametres();
		}
	}

	void SIM_HYD::LectureBilanVertical()
	{
		for (auto iter = begin(_vbilan_vertical); iter != end(_vbilan_vertical); ++iter)
		{
			if(PrendreNomBilanVertical() == (*iter)->PrendreNomSousModele())
				(*iter)->LectureParametres();
		}
	}

	void SIM_HYD::LectureRuisselement()
	{
		for (auto iter = begin(_vruisselement); iter != end(_vruisselement); ++iter)
		{
			if(PrendreNomRuisselement() == (*iter)->PrendreNomSousModele())
				(*iter)->LectureParametres();
		}
	}

	void SIM_HYD::LectureAcheminementRiviere()
	{
		for (auto iter = begin(_vacheminement); iter != end(_vacheminement); ++iter)
		{
			if(PrendreNomAcheminement() == (*iter)->PrendreNomSousModele())
				(*iter)->LectureParametres();
		}
	}


	void SIM_HYD::CalculeHgm(float lame, string nom_fichier)
	{
		ONDE_CINEMATIQUE* onde_cinematique = static_cast<ONDE_CINEMATIQUE*>(_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE].get());
		onde_cinematique->CalculeHgm(lame, nom_fichier);
	}

	OUTPUT& SIM_HYD::PrendreOutput()
	{
		return _output;
	}


	void SIM_HYD::InitListeTronconsZonesSimules()
	{
		size_t i;

		TRONCON* troncon_exutoire = _troncons.RechercheTroncon(_ident_troncon_exutoire);

		if (troncon_exutoire == nullptr)
			throw ERREUR("l'identificateur de troncon exutoire est invalide");

		vector<TRONCON*> troncons;
		troncons.push_back(troncon_exutoire);

		vector<TRONCON*> troncons_simules;
		vector<ZONE*> zones_simules;

		while (!troncons.empty())
		{
			TRONCON* troncon = troncons.back();
			troncons.pop_back();

			troncons_simules.push_back(troncon);

			vector<ZONE*> zones_amont = troncon->PrendreZonesAmont();
			zones_simules.insert(zones_simules.end(), zones_amont.begin(), zones_amont.end());

			vector<TRONCON*> troncons_amont = troncon->PrendreTronconsAmont();
			if (!troncons_amont.empty())
				troncons.insert(troncons.end(), troncons_amont.begin(), troncons_amont.end());
		}

		_troncons_simules.clear();
		_troncons_simules_ident.clear();

		for (auto iter = begin(troncons_simules); iter != end(troncons_simules); ++iter)
		{
			_troncons_simules.push_back(_troncons.IdentVersIndex( (*iter)->PrendreIdent() ));
			_troncons_simules_ident.push_back((*iter)->PrendreIdent());
		}
		_troncons_simules.shrink_to_fit();

		_troncons_simules_ident.shrink_to_fit();
		std::sort(_troncons_simules_ident.begin(), _troncons_simules_ident.end());

		//uhrhs
		_zones_simules.clear();
		_zones_simules_ident.clear();

		for (auto iter=begin(zones_simules); iter!=end(zones_simules); iter++)
		{
			_zones_simules.push_back(_zones.IdentVersIndex((*iter)->PrendreIdent()));	//uhrh index
			_zones_simules_ident.push_back((*iter)->PrendreIdent());	//uhrh id
		}

		std::sort(_zones_simules.begin(), _zones_simules.end());

		if(_vOutputIndexZone != nullptr)
			delete [] _vOutputIndexZone;
		
		vector<size_t> indexOutput;
		for (auto iter=begin(_zones_simules); iter!=end(_zones_simules); iter++)
		{
			if(_output._bSauvegardeTous || 
				find(begin(_output._vIdTronconSelect), end(_output._vIdTronconSelect), _zones[*iter].PrendreTronconAval()->PrendreIdent()) != end(_output._vIdTronconSelect))
			{
				indexOutput.push_back(*iter);
			}
		}

		_outputNbZone = indexOutput.size();
		_vOutputIndexZone = new size_t[_outputNbZone];

		for(i=0; i<indexOutput.size(); i++)
			_vOutputIndexZone[i] = indexOutput[i];

		_zones_simules.shrink_to_fit();		
		_zones_simules_ident.shrink_to_fit();
		std::sort(_zones_simules_ident.begin(), _zones_simules_ident.end());
	}


	vector<size_t>& SIM_HYD::PrendreTronconsSimules()
	{
		return _troncons_simules;
	}

	vector<int>& SIM_HYD::PrendreTronconsSimulesIdent()
	{
		return _troncons_simules_ident;
	}

	vector<size_t>& SIM_HYD::PrendreZonesSimules()	//index zone (0 à x)
	{
		return _zones_simules;
	}

	vector<int>& SIM_HYD::PrendreZonesSimulesIdent()
	{
		return _zones_simules_ident;
	}

	CORRECTIONS& SIM_HYD::PrendreCorrections()
	{
		return _corrections;
	}

	GROUPE_ZONE* SIM_HYD::RechercheGroupeZone(const string& nom)
	{
		return _map_groupes_zone.find(nom) != end(_map_groupes_zone) ? _map_groupes_zone[nom] : nullptr;
	}

	int SIM_HYD::RechercheZoneIndexGroupe(int ident)
	{
		size_t i;
		int ret;

		for (i=0; i< _groupes.size(); i++)
		{
			if(std::find(_groupes[i]._ident_zones.begin(), _groupes[i]._ident_zones.end(), ident) != _groupes[i]._ident_zones.end())
				break;
		}

		if(i == _groupes.size())
			ret = -1;
		else
			ret = static_cast<int>(i);

		return ret;
	}

	GROUPE_ZONE* SIM_HYD::RechercheGroupeCorrection(const string& nom)
	{
		return _map_groupes_correction.find(nom) != end(_map_groupes_correction) ? _map_groupes_correction[nom] : nullptr;
	}

	GROUPE_ZONE* SIM_HYD::PrendreToutBassin()
	{
		return &_groupe_all;
	}


	void SIM_HYD::CreerNouveauProjet(std::string repertoire)
	{
		string repertoire_simulation = Combine(repertoire, "simulation");
		string repertoire_physitel = Combine(repertoire, "physitel");
		string repertoire_physio = Combine(repertoire, "physio");
		string repertoire_meteo = Combine(repertoire, "meteo");
		string repertoire_hydro = Combine(repertoire, "hydro");
		string repertoire_hgm = Combine(repertoire, "hgm");

		string sTemp;
		size_t index;

		CreeRepertoire(repertoire);
		CreeRepertoire(repertoire_simulation);
		CreeRepertoire(repertoire_physitel);
		CreeRepertoire(repertoire_physio);
		CreeRepertoire(repertoire_meteo);
		CreeRepertoire(repertoire_hydro);
		CreeRepertoire(repertoire_hgm);

		_zones.ChangeNomFichierAltitude( Combine(repertoire_physitel, "altitude.tif") );
		_zones.ChangeNomFichierPente( Combine(repertoire_physitel, "pente.tif") );
		_zones.ChangeNomFichierOrientation( Combine(repertoire_physitel, "orientation.tif") );
		_zones.ChangeNomFichierZone( Combine(repertoire_physitel, "uhrh.tif") );
		_zones.LectureZones();

		_noeuds.ChangeNomFichier( Combine(repertoire_physitel, "noeuds.nds") );
		_noeuds.Lecture();

		_troncons.ChangeNomFichier( Combine(repertoire_physitel, "troncon.trl") );
		_troncons.ChangeNomFichierPixels( Combine(repertoire_physitel, "point.rdx") );
		_troncons.LectureTroncons(_zones, _noeuds);

		sTemp = RemplaceExtension(_zones.PrendreNomFichierZone(), "csv");
		_zones.SauvegardeResumer(sTemp);

		//
		RASTER<int> zones = ReadGeoTIFF_int(_zones.PrendreNomFichierZone());
		
		RASTER<int> occsol = ReadGeoTIFF_int(Combine(repertoire_physitel, "occupation_sol.tif"));
		if(occsol.PrendreNbLigne() != zones.PrendreNbLigne() || occsol.PrendreNbColonne() != zones.PrendreNbColonne())
			CropRaster(Combine(repertoire_physitel, "occupation_sol.tif"), _zones.PrendreNomFichierZone());

		_occupation_sol.ChangeNomFichier( Combine(repertoire_physitel, "occupation_sol.cla") );
		_occupation_sol.Lecture(_zones);

		// NOTE: 1 fichier par couche de sol (3) pourrais etre supporté

		sTemp = Combine(repertoire_physitel, "type_sol.tif");
		if(FichierExiste(sTemp))
		{		
			_propriete_hydroliques.ChangeNomFichierCouche1( Combine(repertoire_physitel, "type_sol.cla") );
			_propriete_hydroliques.ChangeNomFichierCouche2( Combine(repertoire_physitel, "type_sol.cla") );
			_propriete_hydroliques.ChangeNomFichierCouche3( Combine(repertoire_physitel, "type_sol.cla") );
			_propriete_hydroliques.ChangeNomFichier( Combine(repertoire_physitel, "proprietehydrolique.sol") );

			RASTER<int> typesol = ReadGeoTIFF_int(Combine(repertoire_physitel, "type_sol.tif"));
			if(typesol.PrendreNbLigne() != zones.PrendreNbLigne() || typesol.PrendreNbColonne() != zones.PrendreNbColonne())
				CropRaster(Combine(repertoire_physitel, "type_sol.tif"), _zones.PrendreNomFichierZone());
		}

		//_propriete_hydroliques._coefficient_additif.resize(_groupes.size(), 0);
		_propriete_hydroliques.Lecture((*this));

		ChangeNbParams();

		_nom_simulation = "simulation";		

		// creation des shapefiles

		string nom_fichier_masque = Combine(repertoire_physitel, "masque.tif");

		RASTER<int> masque(zones.PrendreCoordonnee(), zones.PrendreProjection(), zones.PrendreNbLigne(), zones.PrendreNbColonne(), zones.PrendreTailleCelluleX(), zones.PrendreTailleCelluleY());

		for (size_t lig = 0; lig < zones.PrendreNbLigne(); ++lig)
		{
			for (size_t col = 0; col < zones.PrendreNbColonne(); ++col)
			{
				if (zones(lig, col) != 0 && zones(lig, col) != zones.PrendreNoData())
					masque(lig, col) = 1;
			}
		}

		WriteGeoTIFF(masque, nom_fichier_masque, 0);

		string src = _zones.PrendreNomFichierZone();
		string dst = RemplaceExtension(src, "shp");
		Polygonize(src, dst, nom_fichier_masque);

		string nom_fichier_reseau = Combine(repertoire_physitel, "reseau.tif");

		PhysitelPoint2GeoTIFF(
			_troncons.PrendreNomFichierPixels(),
			_troncons.PrendreNomFichier(),
			nom_fichier_reseau,
			_zones.PrendreNomFichierZone());

		ReseauGeoTIFF2Shapefile(
			_zones.PrendreNomFichierOrientation(),
			nom_fichier_reseau,
			Combine(repertoire_physitel, "rivieres.shp"),
			Combine(repertoire_physitel, "lacs.shp"));

		//fichiers hydro et météo
		if(FichierExiste(repertoire_physitel+"/station.stm"))
		{
			Copie(repertoire_physitel+"/station.stm", repertoire_meteo+"/station.stm");
			SupprimerFichier(repertoire_physitel+"/station.stm");
		}

		if(FichierExiste(repertoire_physitel+"/station.sth"))
		{
			Copie(repertoire_physitel+"/station.sth", repertoire_hydro+"/station.sth");		
			SupprimerFichier(repertoire_physitel+"/station.sth");		
		}

		//
		DATE_HEURE dtDebut(2000, 6, 1, 0);
		DATE_HEURE dtFin(2001, 5, 31, 0);
		ChangeParametresTemporels(dtDebut, dtFin, 24);	//parametres temporel par defaut

		_stations_meteo.ChangeNomFichier(repertoire_meteo+"/station.stm");
		_stations_hydro.ChangeNomFichier(repertoire_hydro+"/station.sth");

		auto ondeCinematiquePtr = static_cast<ONDE_CINEMATIQUE*>(_vruisselement[RUISSELEMENT_ONDE_CINEMATIQUE].get());
		ondeCinematiquePtr->ChangeNomFichierHGM(repertoire_hgm + "/hydrogramme.hgm");

		//fichier milieux humides
		Copie(repertoire_physitel+"/troncon_width_depth.csv", repertoire_physio+"/troncon_width_depth.csv");		
		SupprimerFichier(repertoire_physitel+"/troncon_width_depth.csv");
		_nom_fichier_milieu_humide_profondeur_troncon = repertoire_physio+"/troncon_width_depth.csv";

		_corrections._bActiver = false;
	
		//sauvegarde une simulation par defaut
		SauvegardeSous(repertoire);

		//fichier milieux humides
		if(FichierExiste(repertoire_physitel+"/milieux_humides_riverains.csv"))
		{
			Copie(repertoire_physitel+"/milieux_humides_riverains.csv", repertoire_simulation+"/simulation/milieux_humides_riverains.csv");		
			SupprimerFichier(repertoire_physitel+"/milieux_humides_riverains.csv");
			_nom_fichier_milieu_humide_riverain = repertoire_simulation+"/simulation/milieux_humides_riverains.csv";
		}

		if(FichierExiste(repertoire_physitel+"/milieux_humides_isoles.csv"))
		{
			Copie(repertoire_physitel+"/milieux_humides_isoles.csv", repertoire_simulation+"/simulation/milieux_humides_isoles.csv");		
			SupprimerFichier(repertoire_physitel+"/milieux_humides_isoles.csv");
			_nom_fichier_milieu_humide_isole = repertoire_simulation+"/simulation/milieux_humides_isoles.csv";
		}

		Sauvegarde();

		PrendreOutput().Sauvegarde(PrendreRepertoireSimulation());	//create output.csv

		//creation fichier groupe par defaut
		string nom_fichier_groupe = Combine(PrendreRepertoireSimulation(), PrendreNomSimulation() + ".gsb");
		ofstream fichier(nom_fichier_groupe);

		fichier << '1' << endl;
		fichier << PrendreNomSimulation() << endl;

		for (index = 0; index < _zones.PrendreNbZone(); index++)
			fichier << _zones[index].PrendreIdent() << " 0" << endl;

		fichier.close();
		fichier.clear();

		//creation fichier ind_fol par defaut
		sTemp = _occupation_sol.PrendreNomFichierIndicesFolieres();
		fichier.open(sTemp);

		fichier << '2' << endl;	//type
		fichier << _occupation_sol.PrendreNbClasse() << " " << "2" << endl;	//nbocc nbjour
		fichier << "Indices foliere au jour J / Leaf area index on day D" << endl;
		
		fichier << "Jour";
		for (index = 0; index < _occupation_sol.PrendreNbClasse(); index++)
			fichier << " \"" << _occupation_sol._classes_occupation_sol[index].nom << "\"";	//nom des classes d'occupation du sol
		fichier << endl;

		fichier << "1";
		for (index = 0; index < _occupation_sol.PrendreNbClasse(); index++)
			fichier << " 0";	//jour 1
		fichier << endl;

		fichier << "365";
		for (index = 0; index < _occupation_sol.PrendreNbClasse(); index++)
			fichier << " 0";	//jour 365
		fichier << endl;

		fichier.close();
		fichier.clear();

		//creation fichier pro_rac par defaut
		sTemp = _occupation_sol.PrendreNomFichierProfondeursRacinaires();
		fichier.open(sTemp);

		fichier << '2' << endl;	//type
		fichier << _occupation_sol.PrendreNbClasse() << " " << "2" << endl;	//nbocc nbjour
		fichier << "Profondeur racinaire [m]" << endl;
		
		fichier << "Jour";
		for (index = 0; index < _occupation_sol.PrendreNbClasse(); index++)
			fichier << " \"" << _occupation_sol._classes_occupation_sol[index].nom << "\"";	//nom des classes d'occupation du sol
		fichier << endl;

		fichier << "1";
		for (index = 0; index < _occupation_sol.PrendreNbClasse(); index++)
			fichier << " 0";	//jour 1	//met la valeur 0 pour chaque classe
		fichier << endl;

		fichier << "365";
		for (index = 0; index < _occupation_sol.PrendreNbClasse(); index++)
			fichier << " 0";	//jour 365	//met la valeur 0 pour chaque classe
		fichier << endl;

		fichier.close();
		fichier.clear();
		
		//suppression fichiers temporaires
		SupprimerFichier(nom_fichier_masque);
		SupprimerFichier(nom_fichier_reseau);
	}


	void SIM_HYD::DisplayInfo()
	{
		NOEUDS&		noeuds = PrendreNoeuds();
		ZONES&		zones = PrendreZones();
		TRONCONS&	troncons = PrendreTroncons();

		STATIONS_METEO& stations_meteo = PrendreStationsMeteo();

		std::cout << endl;
		std::cout << "Simulation                                       " << PrendreNomSimulation() << endl;
		std::cout << endl;

		std::cout << "Nb nodes                                         " << noeuds.PrendreNbNoeud() << endl;
		std::cout << "Nb RHHUs                                         " << zones.PrendreNbZone() << endl;
		std::cout << "Nb rivers reach                                  " << troncons.PrendreNbTroncon() << endl;
		std::cout << "Nb groups                                        " << PrendreNbGroupe() << endl;
		std::cout << "Nb weather stations                              " << stations_meteo.PrendreNbStation() << endl;
		std::cout << "Nb hydro stations                                " << _stations_hydro.PrendreNbStation() << endl;
		std::cout << endl;

		std::cout << "Weather data interpolation                       " << PrendreNomInterpolationDonnees() << endl;
		std::cout << "Snow cover evolution                             " << PrendreNomFonteNeige() << endl;

		if(PrendreNomFonteGlacier() != "")
			std::cout << "Glacier melt                                     " << PrendreNomFonteGlacier() << endl;
		else
			std::cout << "Glacier melt                                     " << "(non-simulated)" << endl;

		if(PrendreNomTempSol() != "")
			std::cout << "Soil temperature / Frost depth                   " << PrendreNomTempSol() << endl;
		else
			std::cout << "Soil temperature / Frost depth                   " << "(non-simulated)" << endl;

		std::cout << "Potential evapotranspiration                     " << PrendreNomEvapotranspiration() << endl;
		std::cout << "Vertical water balance                           " << PrendreNomBilanVertical() << endl;

		if(_ruisselement_surface)
			std::cout << "Flow towards the hydrographic network            " << PrendreNomRuisselement() << endl;
		else
			std::cout << "Flow towards the hydrographic network            " << "(non-simulated)" << endl;

		if(_acheminement_riviere)
			std::cout << "Flow into the hydrographic network               " << PrendreNomAcheminement() << endl;
		else
			std::cout << "Flow into the hydrographic network               " << "(non-simulated)" << endl;

		std::cout << endl;

		std::cout << "Start date                                       " << PrendreDateDebut() << endl;
		std::cout << "End date                                         " << PrendreDateFin() << endl;
		std::cout << "Timestep                                         " << PrendrePasDeTemps() << endl;
		std::cout << endl;

		std::cout << "Outlet river reach id                            " << PrendreIdentTronconExutoire() << endl;
		std::cout << endl;
	}


	void SIM_HYD::CreateSubmodelsVersionsFile()
	{
		ofstream file;
		string str, sErr;

		sErr = "";
		str = Combine(PrendreRepertoireSimulation(), "submodels-versions.txt");

		try{
		file.open(str, ios_base::trunc);
		if(!file.is_open())
			sErr = "error creating file: " + str;
		else
		{
			file << "//Model;Version number to use for the simulation" << endl;
			file << endl;
			file << "//If model is not specified, the version number 1 is used by default." << endl;
			file << endl;
			file << "THIESSEN;" << _versionTHIESSEN << "              //weather data interpolation" << endl;
			file << "MOY3STATION;" << _versionMOY3STATION << "		//weather data interpolation" << endl;
			file << endl;
			file << "BV3C;" << _versionBV3C << "                  //vertical water balance" << endl;
			file << endl;
			file.close();
		}

		}
		catch(...)
		{
			if(file && file.is_open())
				file.close();
			sErr = "error writing to file: " + str;
		}

		if(sErr != "")
			throw ERREUR(sErr);
	}


	void SIM_HYD::ReadSubmodelsVersionsFile()
	{
		vector<string> sList;
		istringstream iss;
		ifstream file;
		size_t pos;
		string str, str2, sFile, sErr;
		bool valid;
		int ival;

		sErr = "";
		sFile = Combine(PrendreRepertoireSimulation(), "submodels-versions.txt");

		try{
		file.open(sFile, ios_base::in);
		if(!file.is_open())
			sErr = "error opening file: " + sFile;
		else
		{
			while(!file.eof())
			{
				getline_mod(file, str);
				str2 = TrimString(str);

				if(str2.length() > 2 && !(str2[0] == '/' && str2[1] == '/'))	//if string dont begin with "//"
				{
					pos = str2.find("//");
					if(pos != string::npos)
						str2 = str2.substr(0, pos);	//exclude comment in the string ("//")		//ex: THIESSEN;2              //weather data interpolation

					SplitString2(sList, str2, ";", true);
					if(sList.size() != 2)
					{
						sErr = "error reading file: " + sFile + ": invalid line: " + str2;
						break;
					}
					else
					{
						try{
						iss.clear();
						iss.str(sList[1]);
						iss >> ival;
						}
						catch(...)
						{
							sErr = "error reading file: " + sFile + ": invalid line: " + str2;
							break;
						}

						valid = true;
						str = sList[0];
						boost::algorithm::to_lower(str);

						if(str == "thiessen")
						{
							if(ival < 1 || ival > 2)
								valid = false;
							else
								_versionTHIESSEN = ival;
						}
						else
						{
							if(str == "moy3station")
							{
								if(ival < 1 || ival > 2)
									valid = false;
								else
									_versionMOY3STATION = ival;
							}
							else
							{
								if(str == "bv3c")
								{
									if(ival < 1 || ival > 2)
										valid = false;
									else
										_versionBV3C = ival;
								}
								else
								{
									sErr = "error reading file: " + sFile + ": unknown model: " + str;
									break;
								}
							}
						}

						if(!valid)
						{
							sErr = "error reading file: " + sFile + ": invalid model version: " + str2;
							break;
						}
					}
				}
			}

			file.close();
		}

		}
		catch(...)
		{
			if(file && file.is_open())
				file.close();
			sErr = "error opening/reading file: catch exception: " + sFile;
		}

		if(sErr != "")
			throw ERREUR(sErr);
	}

}
