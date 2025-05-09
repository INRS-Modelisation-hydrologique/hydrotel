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

#ifndef SIM_HYD_H_INCLUDED
#define SIM_HYD_H_INCLUDED


#include "date_heure.hpp"
#include "groupe_zone.hpp"
#include "noeuds.hpp"
#include "occupation_sol.hpp"
#include "output.hpp"
#include "propriete_hydroliques.hpp"
#include "stations_hydro.hpp"
#include "stations_meteo.hpp"
#include "troncons.hpp"
#include "corrections.hpp"
#include "grille_prevision.hpp"
#include "rayonnement_net.hpp"
#include "raster_double2.hpp"
#include "prelevements.hpp"


namespace HYDROTEL
{

	// forward declaration
	class INTERPOLATION_DONNEES;
	class FONTE_NEIGE;
	class FONTE_GLACIER;
	class TEMPSOL;
	class EVAPOTRANSPIRATION;
	class BILAN_VERTICAL;
	class RUISSELEMENT_SURFACE;
	class ACHEMINEMENT_RIVIERE;
	class PRELEVEMENTS;

	class THIESSEN1;
	class THIESSEN2;
	class MOYENNE_3_STATIONS1;
	class MOYENNE_3_STATIONS2;
	class GRILLE_METEO;

	class TRONCONS;


	class SIM_HYD
	{
	public:
		enum TYPE_INTERPOLATION_DONNEES
		{
			INTERPOLATION_LECTURE,
			INTERPOLATION_THIESSEN1,
			INTERPOLATION_THIESSEN2,
			INTERPOLATION_MOYENNE_3_STATIONS1,
			INTERPOLATION_MOYENNE_3_STATIONS2,
			INTERPOLATION_GRILLE,
		};

		enum TYPE_FONTE_NEIGE
		{
			FONTE_NEIGE_LECTURE,
			FONTE_NEIGE_DEGRE_JOUR_MODIFIE,
			FONTE_NEIGE_DEGRE_JOUR_BANDE,
		};

		enum TYPE_FONTE_GLACIER
		{
			FONTE_GLACIER_LECTURE,
			FONTE_GLACIER_DEGRE_JOUR_GLACIER,
		};

		enum TYPE_TEMPSOL
		{
			TEMPSOL_LECTURE,
			TEMPSOL_RANKINEN,
			TEMPSOL_THORSEN,
		};

		enum TYPE_EVAPOTRANSPIRATION
		{
			ETP_LECTURE,
			ETP_HYDRO_QUEBEC,
			ETP_THORNTHWAITE,
			ETP_LINACRE,
			ETP_PENMAN,
			ETP_PRIESTLAY_TAYLOR,
			ETP_PENMAN_MONTEITH,
			ENUM_ETP_MC_GUINESS,
		};

		enum TYPE_BILAN_VERTICAL
		{
			BILAN_VERTICAL_LECTURE,
			BILAN_VERTICAL_BV3C1,
			BILAN_VERTICAL_BV3C2,
			BILAN_VERTICAL_CEQUEAU,
		};

		enum TYPE_RUISSELEMENT
		{
			RUISSELEMENT_LECTURE,
			RUISSELEMENT_ONDE_CINEMATIQUE,
		};

		enum TYPE_ACHEMINEMENT
		{
			ACHEMINEMENT_LECTURE,
			ACHEMINEMENT_ONDE_CINEMATIQUE_MODIFIEE,
		};

		//enum TYPE_DONNEE
		//{
		//	TMIN = 1,
		//	TMAX = 2,
		//	PLUIE = 4,
		//	NEIGE = 8,
		//	TMIN_JOUR = 16,
		//	TMAX_JOUR = 32,
		//};

		int _versionTHIESSEN;		//models versions to use
		int _versionMOY3STATION;	//
		int _versionBV3C;			//


	public:

		SIM_HYD();
		~SIM_HYD();

		// retourne les options de sorties des resultats
		OUTPUT& PrendreOutput();

		std::string PrendreNomProjet() const;

		NOEUDS& PrendreNoeuds();

		TRONCONS& PrendreTroncons();

		ZONES& PrendreZones();

		OCCUPATION_SOL& PrendreOccupationSol();

		PROPRIETE_HYDROLIQUES& PrendreProprieteHydrotliques();

		STATIONS_HYDRO& PrendreStationsHydro();

		STATIONS_METEO& PrendreStationsMeteo();

		std::string PrendreRepertoireProjet() const;

		std::string PrendreRepertoireSimulation() const;

		std::string PrendreRepertoireResultat() const;

		std::string PrendreNomSimulation() const;

		size_t PrendreNbGroupe() const;

		const GROUPE_ZONE& PrendreGroupeZone(size_t index) const;

		// recherche le groupe de zones par nom
		GROUPE_ZONE*	RechercheGroupeZone(const std::string& nom);

		//recherche l'index du groupe pour la zone specifie
		int				RechercheZoneIndexGroupe(int ident);

		// recherche le groupe de correction par nom
		GROUPE_ZONE* RechercheGroupeCorrection(const std::string& nom);

		// retourne tout le bassin
		GROUPE_ZONE* PrendreToutBassin();

		// retourne la date de debut de simulation
		DATE_HEURE PrendreDateDebut() const;

		// retourne la date de fin de simulation
		DATE_HEURE PrendreDateFin() const;

		// retourne le pas de temps de simulation (h)
		unsigned short PrendrePasDeTemps() const;

		// retourne la date du pas courant de simulation
		DATE_HEURE PrendreDateCourante() const;

		// change le nom de fichier de projet
		void ChangeNomFichier(const std::string& nom_fichier);

		// change les parametres temporels de simulation
		void ChangeParametresTemporels(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);

		// change le sous modele d'interpolation des donnees
		void ChangeInterpolationDonnees(const std::string& nom_sous_modele);

		// change le sous modele de fonte de neige
		void ChangeFonteNeige(const std::string& nom_sous_modele);

		// change le sous modele de fonte de glacier
		void ChangeFonteGlacier(const std::string& nom_sous_modele);

		// change le sous modele de temperature du sol
		void ChangeTempSol(const std::string& nom_sous_modele);

		// change le sous modele d'evapotranspiration
		void ChangeEvapotranspiration(const std::string& nom_sous_modele);

		// change le sous modele de bilan vertical
		void ChangeBilanVertical(const std::string& nom_sous_modele);

		// change le sous modele de ruisselement
		void ChangeRuisselementSurface(const std::string& nom_sous_modele);

		// change le sous modele d'acheminement riviere
		void ChangeAcheminementRiviere(const std::string& nom_sous_modele);

		void ChangeIdentTronconExutoire(int ident);

		// retourne le nom du sous modele d'interpolation
		std::string PrendreNomInterpolationDonnees() const;

		// retourne le nom du sous modele de fonte de neige
		std::string PrendreNomFonteNeige() const;

		// retourne le nom du sous modele de fonte de glacier
		std::string PrendreNomFonteGlacier() const;

		// retourne le nom du sous modele de temperature du sol
		std::string PrendreNomTempSol() const;

		// retourne le nom du sous modele d'evapotranspiration
		std::string PrendreNomEvapotranspiration() const;

		// retourne le nom du sous modele de bilan vertical
		std::string PrendreNomBilanVertical() const;

		// retourne le nom du sous modele de ruisselement
		std::string PrendreNomRuisselement() const;

		// retourne le nom du sous modele d'acheminement
		std::string PrendreNomAcheminement() const;

		int PrendreIdentTronconExutoire() const;

		// retourne la liste des troncons simules
		std::vector<size_t>& PrendreTronconsSimules();
		std::vector<int>& PrendreTronconsSimulesIdent();

		// retourne la liste des index des zones simules
		std::vector<size_t>& PrendreZonesSimules();

		// retourne la liste des id des zones simules
		std::vector<int>& PrendreZonesSimulesIdent();

		// retourne la liste des corrections
		CORRECTIONS& PrendreCorrections();

		void Lecture();

		void SauvegardeSous(std::string repertoire);

		void Sauvegarde();

		void CalculeHgm(float lame, std::string nom_fichier);

		void Initialise();

		void Calcule();

		void Termine();

		void CreerNouveauProjet(std::string repertoire);

		void DisplayInfo();

		void LectureVersionFichierSim();
		void CreateSubmodelsVersionsFile();
		void ReadSubmodelsVersionsFile();


	public:

		int									_nbThread;	//nb thread to use for hgm computation	//0=max available threads

		std::string							_versionSimStr;
		size_t								_versionSim;

		bool								_bUpdatingV26Project;	//true if hydrotel is run with -u switch

		//simulation
		DATE_HEURE							_date_debut;
		DATE_HEURE							_date_fin;

		unsigned short						_pas_de_temps;

		size_t								_lNbPasTempsSim;

		size_t								_lPasTempsCourantIndex;		//used for saving NetCDF files

        //nom du fichier des parametres pour les milieux humides isoles
        bool								_bSimuleMHIsole;
		std::string							_nom_fichier_milieu_humide_isole;

        //nom des fichiers des parametres pour les milieux humides riverains
		bool								_bSimuleMHRiverain;
        std::string							_nom_fichier_milieu_humide_riverain;

        std::string							_nom_fichier_milieu_humide_profondeur_troncon;

		int									_fichierParametreGlobal;				//0 ou 1; indique si on utilise le fichier parametres_sous_modeles.csv
		std::string							_nomFichierParametresGlobal;

		bool								_bActiveTronconDeconnecte;
		std::map<size_t, size_t>			_mapIndexTronconDeconnecte;

		STATIONS_HYDRO						_stations_hydro;
		STATIONS_METEO						_stations_meteo;

		std::string							_nom_fichier_grille_meteo;

		GRILLE_PREVISION					_grille_prevision;
		bool								_bSimulePrevision;

		bool								_bHGMCalculer;

		RAYONNEMENT_NET						_rayonnementNet;

		OUTPUT								_output;
		bool								_outputCDF;

		std::string							_sPathProjetImport;				//utilisé lors de la conversion d'un projet afin d'utiliser les cartes .tif nouvellement importé

		bool								_bAutoInverseTMinTMax;
		bool								_bStationInterpolation;
		
		bool								_bSkipCharacterValidation;		//tell to skip validation of input files characters (only ASCII/UTF8 char code 32 to 126 are valid in input files)
		std::vector<std::string>			_listErrMessCharValidation;

		bool								_bGenereBdPrelevements;
		std::string							_sFolderNamePrelevements;		//folder name for prelevements input files	//prelevements
		std::string							_sFolderNamePrelevementsSrc;	//folder name for source input file for prelevements db generation	//SitesPrelevements

		//sous modeles courant
		INTERPOLATION_DONNEES*	_interpolation_donnees;
		FONTE_NEIGE*			_fonte_neige;
		FONTE_GLACIER*			_fonte_glacier;
		TEMPSOL*				_tempsol;
		EVAPOTRANSPIRATION*		_evapotranspiration;
		BILAN_VERTICAL*			_bilan_vertical;
		RUISSELEMENT_SURFACE*	_ruisselement_surface;
		ACHEMINEMENT_RIVIERE*   _acheminement_riviere;

		bool					_bRayonnementNet;

		//mode lecture
		bool					_bLectInterpolation;
		bool					_bLectNeige;
		bool					_bLectFonteGlacier;
		bool					_bLectTempsol;
		bool					_bLectEvap;
		bool					_bLectBilan;
		bool					_bLectRuissellement;
		bool					_bLectAcheminement;

		size_t					_outputNbZone;
		size_t*					_vOutputIndexZone;

		RasterInt2*				_pRasterOri;			//raster orientation	//(1=East; 2=Northeast; 3=North; 4=Northwest; 5=West; 6=Southwest; 7=South; 8=Southeast)
		RasterDouble2*			_pRasterPente;			//raster pente [pour mille]

		PRELEVEMENTS*			_pr;

		// sous modeles
		THIESSEN1*				_smThiessen1;
		THIESSEN2*				_smThiessen2;
		MOYENNE_3_STATIONS1*	_smMoy3station1;
		MOYENNE_3_STATIONS2*	_smMoy3station2;
		GRILLE_METEO*			_smGrilleMeteo;

		std::vector<std::unique_ptr<INTERPOLATION_DONNEES>> _vinterpolation_donnees;
		std::vector<std::unique_ptr<FONTE_NEIGE>>			_vfonte_neige;
		std::vector<std::unique_ptr<FONTE_GLACIER>>			_vfonte_glacier;
		std::vector<std::unique_ptr<TEMPSOL>>				_vtempsol;
		std::vector<std::unique_ptr<EVAPOTRANSPIRATION>>	_vevapotranspiration;
		std::vector<std::unique_ptr<BILAN_VERTICAL>>		_vbilan_vertical;
		std::vector<std::unique_ptr<RUISSELEMENT_SURFACE>>	_vruisselement;
		std::vector<std::unique_ptr<ACHEMINEMENT_RIVIERE>>	_vacheminement;


	private:

		void LectureProjetFormatPrj();
		void LectureProjetFormatCsv();
		
		void LectureDonneesPhysiographiques();
		void LectureDonneesMeteorologiques();
		void LectureDonneesHydrologiques();

		void LectureGroupeZone();
		void LectureGroupeZoneCorrection();
		
		void LectureSimulationFormatSim();
		void LectureSimulationFormatCsv();

		void LectureInterpolationDonnees();
		void LectureFonteNeige();
		void LectureFonteGlacier();
		void LectureTempSol();
		void LectureEtp();
		void LectureBilanVertical();
		void LectureRuisselement();
		void LectureAcheminementRiviere();

		void ChangeNbParams();

		void SauvegardeFichierProjet();
		void SauvegardeFichierSimulation();

		void SauvegardeInterpolationDonnees();
		void SauvegardeFonteNeige();
		void SauvegardeFonteGlacier();
		void SauvegardeTempSol();
		void SauvegardeEtp();
		void SauvegardeBilanVertical();
		void SauvegardeRuisselement();
		void SauvegradeAcheminementRiviere();

		void InitListeTronconsZonesSimules();

		std::string _nom_fichier;

		// donnees physiographiques
		NOEUDS		_noeuds;
		TRONCONS	_troncons;
		ZONES		_zones;

		OCCUPATION_SOL			_occupation_sol;
		PROPRIETE_HYDROLIQUES	_propriete_hydroliques;

		std::string _nom_simulation;
		std::string _nom_fichier_simulation;

		std::vector<GROUPE_ZONE> _groupes; //pour supporter les anciennes version
		std::vector<GROUPE_ZONE> _groupes_correction;
		GROUPE_ZONE _groupe_all;

		int _ident_troncon_exutoire;

		// simulation en cours
		DATE_HEURE _date_courante;

		// index des troncons et des zones a simules
		std::vector<size_t> _troncons_simules;
		std::vector<int>	_troncons_simules_ident;
		
		std::vector<size_t> _zones_simules;
		std::vector<int>	_zones_simules_ident;	//l'identifiant est negatif pour les lacs

		//
		std::vector<size_t>					_wavg_idtroncon;
		std::vector<std::vector<size_t>>	_wavg_zones;	//uhrh index
		std::vector<std::vector<float>>		_wavg_zones_weighting;

		std::vector<std::ofstream>			_wavg_fichier;

		//
		CORRECTIONS _corrections;

		// accelerer la recherche par nom
		std::map<std::string, GROUPE_ZONE*> _map_groupes_zone;
		std::map<std::string, GROUPE_ZONE*> _map_groupes_correction;
	};

}

#endif

