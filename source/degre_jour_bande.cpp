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

#include "degre_jour_bande.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "moyenne_3_stations1.hpp"
#include "moyenne_3_stations2.hpp"
#include "station_neige.hpp"
#include "thiessen1.hpp"
#include "thiessen2.hpp"
#include "grille_meteo.hpp"
#include "util.hpp"
#include "version.hpp"
#include "gdal_util.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	DEGRE_JOUR_BANDE::DEGRE_JOUR_BANDE(SIM_HYD& sim_hyd)
		: FONTE_NEIGE(sim_hyd, "DEGRE JOUR BANDE")
	{
		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;
		//_bMAJGrilleNeige = false;

		_nbr_jour_delai_mise_a_jour = 5;
		//_grilleneige._sim_hyd = &sim_hyd;

		_pThiessen1 = nullptr;
		_pThiessen2 = nullptr;
		_pMoy3station1 = nullptr;
		_pMoy3station2 = nullptr;
	}


	DEGRE_JOUR_BANDE::~DEGRE_JOUR_BANDE()
	{
	}


	void DEGRE_JOUR_BANDE::ChangeNbParams(const ZONES& zones)
	{
		_seuil_fonte_conifers.resize(zones.PrendreNbZone(), 0.0);
		_seuil_fonte_feuillus.resize(zones.PrendreNbZone(), 0.0);
		_seuil_fonte_decouver.resize(zones.PrendreNbZone(), 0.0);

		_taux_fonte_conifers.resize(zones.PrendreNbZone(), 12.0);
		_taux_fonte_feuillus.resize(zones.PrendreNbZone(), 14.0);
		_taux_fonte_decouver.resize(zones.PrendreNbZone(), 16.0);

		_taux_fonte.resize(zones.PrendreNbZone(), 0.5);
		_densite_maximale.resize(zones.PrendreNbZone(), 550.0);
		_constante_tassement.resize(zones.PrendreNbZone(), 0.1);

		_seuil_albedo.resize(zones.PrendreNbZone(), 1.0);
		_methode_albedo.resize(zones.PrendreNbZone(), 1);

		_dHauteurBande = 100.0;			//m
	}


	void DEGRE_JOUR_BANDE::Initialise()
	{
		ostringstream oss;
		double dNoDataDem, dPixelArea, dAlt, dNbBande, dArea, superficieUhrh;
		double theta, k, h;
		//double albedo;
		size_t nb_zone, nb_zoneSim, nbColZone, nbRowZone, nbColDem, nbRowDem, nbColOcc, nbRowOcc, col, row, indexUhrh, i, j, nbPixel, idx;
		string str, sFile;
		int iNoDataZone, iUhrh, iOcc, iNoDataOcc, iNbBande, x;

		//_corrections_neige_au_sol = _sim_hyd.PrendreCorrections().PrendreCorrectionsNeigeAuSol();

		ZONES& zones = _sim_hyd.PrendreZones();
		vector<size_t> zoneSimIndex = _sim_hyd.PrendreZonesSimules();

		const RASTER<int>& grilleZone = zones.PrendreGrille();
		const RASTER<float> grilleDem = LectureRaster_float(_sim_hyd.PrendreZones().PrendreNomFichierAltitude());
		const RASTER<int>	grilleOcc = LectureRaster_int(RemplaceExtension(_sim_hyd.PrendreOccupationSol().PrendreNomFichier(), "tif"));

		//valide que les coordonnees superieur gauche des matrices sont identique
		//trunc les valeur à 7 décimale pour eviter les problemes de resolution numerique lors de la comparaison des valeurs double
		long long int lx, ly, lxZone, lyZone;

		lxZone = static_cast<long long int>(grilleZone.PrendreCoordonnee().PrendreX() * 10000000.0);
		lyZone = static_cast<long long int>(grilleZone.PrendreCoordonnee().PrendreY() * 10000000.0);

		lx = static_cast<long long int>(grilleDem.PrendreCoordonnee().PrendreX() * 10000000.0);
		ly = static_cast<long long int>(grilleDem.PrendreCoordonnee().PrendreY() * 10000000.0);

		if(lx != lxZone || ly != lyZone)
			throw ERREUR("DEGRE_JOUR_BANDE error: the coordinates of the elevation and land cover matrix do not correspond with the coordinates of the rhhu matrix.");

		lx = static_cast<long long int>(grilleOcc.PrendreCoordonnee().PrendreX() * 10000000.0);
		ly = static_cast<long long int>(grilleOcc.PrendreCoordonnee().PrendreY() * 10000000.0);

		if(lx != lxZone || ly != lyZone)
			throw ERREUR("DEGRE_JOUR_BANDE error: the coordinates of the elevation and land cover matrix do not correspond with the coordinates of the rhhu matrix.");

		nbColZone = grilleZone.PrendreNbColonne();
		nbRowZone = grilleZone.PrendreNbLigne();
		nbColDem = grilleDem.PrendreNbColonne();
		nbRowDem = grilleDem.PrendreNbLigne();
		nbColOcc = grilleOcc.PrendreNbColonne();
		nbRowOcc = grilleOcc.PrendreNbLigne();

		if(nbColDem < nbColZone || nbRowDem < nbRowZone || nbColOcc < nbColZone || nbRowOcc < nbRowZone)
			throw ERREUR("DEGRE_JOUR_BANDE error: the elevation and land cover matrix must be of the same size (NbColumn and NbLine) or larger than the rhhu matrix.");

		//initialisation pour les valeurs des gradients
		if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "LECTURE INTERPOLATION DONNEES")
		{
			if(_sim_hyd._versionTHIESSEN == 1)
				_sim_hyd._smThiessen1->LectureParametres();
			else
				_sim_hyd._smThiessen2->LectureParametres();	//_sim_hyd._versionTHIESSEN == 2

			std::cout << std::endl << "warning: degre_jour_bande submodel will use parameters from the thiessen submodel file because the interpolation model is in read mode" << std::endl;
		}
		
		//ponderations et lecture des donnees des stations de neige
		const PROJECTION& projection = zones.PrendreProjection();

		DATE_HEURE date_debut = _sim_hyd.PrendreDateDebut();
		DATE_HEURE date_fin = _sim_hyd.PrendreDateFin();

		switch(_interpolation_conifers)
		{
		case INTERPOLATION_AUCUNE:
			break;

		case INTERPOLATION_THIESSEN:			
			_stations_neige_conifers.Lecture(projection);
			
			if(_sim_hyd._versionTHIESSEN == 1)
			{
				_pThiessen1 = new THIESSEN1(_sim_hyd);
				_pThiessen1->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_THIESSEN1]->PrendreNomFichierParametres());
				_pThiessen1->ChangeNbParams(_sim_hyd.PrendreZones());
				_pThiessen1->LectureParametres();

				if(!_pThiessen1->LecturePonderation(_stations_neige_conifers, zones, _ponderation_conifersF))
				{
					_pThiessen1->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifersF);
					_pThiessen1->SauvegardePonderation(_stations_neige_conifers, zones, _ponderation_conifersF);
				}
			}
			else
			{
				//_versionTHIESSEN == 2
				_pThiessen2 = new THIESSEN2(_sim_hyd);
				_pThiessen2->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_THIESSEN1]->PrendreNomFichierParametres());
				_pThiessen2->ChangeNbParams(_sim_hyd.PrendreZones());
				_pThiessen2->LectureParametres();

				if(!_pThiessen2->LecturePonderation(_stations_neige_conifers, zones, _ponderation_conifers))
				{
					_pThiessen2->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifers, false);
					_pThiessen2->SauvegardePonderation(_stations_neige_conifers, zones, _ponderation_conifers);
				}
			}

			_stations_neige_conifers.LectureDonnees(date_debut, date_fin);
			break;

		case INTERPOLATION_MOYENNE_3_STATIONS:
			_stations_neige_conifers.Lecture(projection);

			if(_sim_hyd._versionMOY3STATION == 1)
			{
				_pMoy3station1 = new MOYENNE_3_STATIONS1(_sim_hyd);
				_pMoy3station1->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_MOYENNE_3_STATIONS1]->PrendreNomFichierParametres());
				_pMoy3station1->ChangeNbParams(_sim_hyd.PrendreZones());
				_pMoy3station1->LectureParametres();

				if(!_pMoy3station1->LecturePonderation(_stations_neige_conifers, zones, _ponderation_conifersF))
				{
					_pMoy3station1->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifersF);
					_pMoy3station1->SauvegardePonderation(_stations_neige_conifers, zones, _ponderation_conifersF);
				}
			}
			else
			{
				//_versionMOY3STATION == 2
				_pMoy3station2 = new MOYENNE_3_STATIONS2(_sim_hyd);
				_pMoy3station2->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_MOYENNE_3_STATIONS1]->PrendreNomFichierParametres());
				_pMoy3station2->ChangeNbParams(_sim_hyd.PrendreZones());
				_pMoy3station2->LectureParametres();

				if(!_pMoy3station2->LecturePonderation(_stations_neige_conifers, zones, _ponderation_conifers))
				{
					_pMoy3station2->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifers);
					_pMoy3station2->SauvegardePonderation(_stations_neige_conifers, zones, _ponderation_conifers);
				}
			}

			_stations_neige_conifers.LectureDonnees(date_debut, date_fin);
			break;

		default:
			throw ERREUR("interpolation snow stations conifers");
		}

		switch(_interpolation_feuillus)
		{
		case INTERPOLATION_AUCUNE:
			break;

		case INTERPOLATION_THIESSEN:
			_stations_neige_feuillus.Lecture(projection);
			
			if(_sim_hyd._versionTHIESSEN == 1)
			{
				_pThiessen1 = new THIESSEN1(_sim_hyd);
				_pThiessen1->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_THIESSEN1]->PrendreNomFichierParametres());
				_pThiessen1->ChangeNbParams(_sim_hyd.PrendreZones());
				_pThiessen1->LectureParametres();

				if(!_pThiessen1->LecturePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF))
				{
					_pThiessen1->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF);
					_pThiessen1->SauvegardePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF);
				}
			}
			else
			{
				//_versionTHIESSEN == 2
				_pThiessen2 = new THIESSEN2(_sim_hyd);
				_pThiessen2->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_THIESSEN1]->PrendreNomFichierParametres());
				_pThiessen2->ChangeNbParams(_sim_hyd.PrendreZones());
				_pThiessen2->LectureParametres();

				if(!_pThiessen2->LecturePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus))
				{
					_pThiessen2->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus, false);
					_pThiessen2->SauvegardePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus);
				}
			}

			_stations_neige_feuillus.LectureDonnees(date_debut, date_fin);
			break;

		case INTERPOLATION_MOYENNE_3_STATIONS:
			_stations_neige_feuillus.Lecture(projection);

			if(_sim_hyd._versionMOY3STATION == 1)
			{
				_pMoy3station1 = new MOYENNE_3_STATIONS1(_sim_hyd);
				_pMoy3station1->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_MOYENNE_3_STATIONS1]->PrendreNomFichierParametres());
				_pMoy3station1->ChangeNbParams(_sim_hyd.PrendreZones());
				_pMoy3station1->LectureParametres();

				if(!_pMoy3station1->LecturePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF))
				{
					_pMoy3station1->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF);
					_pMoy3station1->SauvegardePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF);
				}
			}
			else
			{
				//_versionMOY3STATION == 2
				_pMoy3station2 = new MOYENNE_3_STATIONS2(_sim_hyd);
				_pMoy3station2->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_MOYENNE_3_STATIONS1]->PrendreNomFichierParametres());
				_pMoy3station2->ChangeNbParams(_sim_hyd.PrendreZones());
				_pMoy3station2->LectureParametres();

				if(!_pMoy3station2->LecturePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus))
				{
					_pMoy3station2->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus);
					_pMoy3station2->SauvegardePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus);
				}
			}

			_stations_neige_feuillus.LectureDonnees(date_debut, date_fin);
			break;

		default:
			throw ERREUR("interpolation snow stations deciduous");
		}

		switch(_interpolation_decouver)
		{
		case INTERPOLATION_AUCUNE:
			break;

		case INTERPOLATION_THIESSEN:
			_stations_neige_decouver.Lecture(projection);

			if(_sim_hyd._versionTHIESSEN == 1)
			{
				_pThiessen1 = new THIESSEN1(_sim_hyd);
				_pThiessen1->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_THIESSEN1]->PrendreNomFichierParametres());
				_pThiessen1->ChangeNbParams(_sim_hyd.PrendreZones());
				_pThiessen1->LectureParametres();

				if(!_pThiessen1->LecturePonderation(_stations_neige_decouver, zones, _ponderation_decouverF))
				{
					_pThiessen1->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouverF);
					_pThiessen1->SauvegardePonderation(_stations_neige_decouver, zones, _ponderation_decouverF);
				}
			}
			else
			{
				//_versionTHIESSEN == 2
				_pThiessen2 = new THIESSEN2(_sim_hyd);
				_pThiessen2->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_THIESSEN1]->PrendreNomFichierParametres());
				_pThiessen2->ChangeNbParams(_sim_hyd.PrendreZones());
				_pThiessen2->LectureParametres();

				if(!_pThiessen2->LecturePonderation(_stations_neige_decouver, zones, _ponderation_decouver))
				{
					_pThiessen2->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouver, false);
					_pThiessen2->SauvegardePonderation(_stations_neige_decouver, zones, _ponderation_decouver);
				}
			}

			_stations_neige_decouver.LectureDonnees(date_debut, date_fin);
			break;

		case INTERPOLATION_MOYENNE_3_STATIONS:
			_stations_neige_decouver.Lecture(projection);

			if(_sim_hyd._versionMOY3STATION == 1)
			{
				_pMoy3station1 = new MOYENNE_3_STATIONS1(_sim_hyd);
				_pMoy3station1->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_MOYENNE_3_STATIONS1]->PrendreNomFichierParametres());
				_pMoy3station1->ChangeNbParams(_sim_hyd.PrendreZones());
				_pMoy3station1->LectureParametres();

				if(!_pMoy3station1->LecturePonderation(_stations_neige_decouver, zones, _ponderation_decouverF))
				{
					_pMoy3station1->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouverF);
					_pMoy3station1->SauvegardePonderation(_stations_neige_decouver, zones, _ponderation_decouverF);
				}
			}
			else
			{
				//_versionMOY3STATION == 2
				_pMoy3station2 = new MOYENNE_3_STATIONS2(_sim_hyd);
				_pMoy3station2->ChangeNomFichierParametres(_sim_hyd._vinterpolation_donnees[_sim_hyd.INTERPOLATION_MOYENNE_3_STATIONS1]->PrendreNomFichierParametres());
				_pMoy3station2->ChangeNbParams(_sim_hyd.PrendreZones());
				_pMoy3station2->LectureParametres();

				if(!_pMoy3station2->LecturePonderation(_stations_neige_decouver, zones, _ponderation_decouver))
				{
					_pMoy3station2->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouver);
					_pMoy3station2->SauvegardePonderation(_stations_neige_decouver, zones, _ponderation_decouver);
				}
			}

			_stations_neige_decouver.LectureDonnees(date_debut, date_fin);
			break;

		default:
			throw ERREUR("interpolation snow stations open areas");
		}

		//
		iNoDataZone = grilleZone.PrendreNoData();
		dNoDataDem = static_cast<double>(grilleDem.PrendreNoData());
		iNoDataOcc = grilleOcc.PrendreNoData();

		dPixelArea = static_cast<double>(grilleZone.PrendreTailleCelluleX());
		dPixelArea = dPixelArea * dPixelArea;	//m2

		nb_zone = zones.PrendreNbZone();

		_ce1.resize(nb_zone, 0.0);
		_ce0.resize(nb_zone, 0.0);
		//_tsn.resize(nb_zone, 0.0);

		_bUhrhSimule.clear();
		_bUhrhSimule.resize(nb_zone, false);
		_bUhrhIndexSimule.clear();
		_bUhrhIndexSimule.resize(nb_zone, false);

		for (i=0; i!=nb_zone; i++)
		{
			if(find(begin(zoneSimIndex), end(zoneSimIndex), i) != end(zoneSimIndex))
			{
				_bUhrhSimule[zones[i]._identABS-1] = true;
				_bUhrhIndexSimule[i] = true;
			}
		}

		//determination des bandes pour chaque uhrh
		//parcours les matrices uhrh, occupation et altitude
		//conserve les valeurs d'altitude de chaque pixel pour chaque milieu pour chaque uhrh
		//conserve altitude min et max pour chaque milieu pour chaque uhrh

		_altMin.clear();
		_altMin.resize(nb_zone, 100000.0);
		_altMax.clear();
		_altMax.resize(nb_zone, -100000.0);

		_altPixelM1.clear();			//conniferes
		_altPixelM1.resize(nb_zone);
		_altPixelM2.clear();			//feuillus
		_altPixelM2.resize(nb_zone);
		_altPixelM3.clear();			//decouvert
		_altPixelM3.resize(nb_zone);

		for(col=0; col!=nbColZone; col++)
		{
			for(row=0; row!=nbRowZone; row++)
			{
				iUhrh = grilleZone(row, col);

				if(iUhrh != iNoDataZone && _bUhrhSimule[abs(iUhrh)-1])	//si l'uhrh est simulé
				{
					dAlt = static_cast<double>(grilleDem(row, col));
					if(dAlt != dNoDataDem)
					{
						iOcc = grilleOcc(row, col);
						indexUhrh = zones.IdentVersIndex(iUhrh);

						if(dAlt < _altMin[indexUhrh])
							_altMin[indexUhrh] = dAlt;
						if(dAlt > _altMax[indexUhrh])
							_altMax[indexUhrh] = dAlt;

						if(iOcc != iNoDataOcc)
						{
							if(find(begin(_index_occupation_conifers), end(_index_occupation_conifers), iOcc-1) != end(_index_occupation_conifers))
								_altPixelM1[indexUhrh].push_back(dAlt);
							else
							{
								if(find(begin(_index_occupation_feuillus), end(_index_occupation_feuillus), iOcc-1) != end(_index_occupation_feuillus))
									_altPixelM2[indexUhrh].push_back(dAlt);
								else
									_altPixelM3[indexUhrh].push_back(dAlt);
							}
						}
					}
				}
			}
		}

		_bandeSuperficieM1.clear();
		_bandeSuperficieM1.resize(nb_zone);
		_bandeSuperficieM2.clear();
		_bandeSuperficieM2.resize(nb_zone);
		_bandeSuperficieM3.clear();
		_bandeSuperficieM3.resize(nb_zone);

		_bandePourcentageM1.clear();
		_bandePourcentageM1.resize(nb_zone);		
		_bandePourcentageM2.clear();
		_bandePourcentageM2.resize(nb_zone);
		_bandePourcentageM3.clear();
		_bandePourcentageM3.resize(nb_zone);

		_bandeAltMoyM1.clear();
		_bandeAltMoyM1.resize(nb_zone);
		_bandeAltMoyM2.clear();
		_bandeAltMoyM2.resize(nb_zone);
		_bandeAltMoyM3.clear();
		_bandeAltMoyM3.resize(nb_zone);

		_stock_conifers.clear();
		_stock_conifers.resize(nb_zone);
		_stock_feuillus.clear();
		_stock_feuillus.resize(nb_zone);
		_stock_decouver.clear();
		_stock_decouver.resize(nb_zone);

		_hauteur_conifers.clear();
		_hauteur_conifers.resize(nb_zone);
		_hauteur_feuillus.clear();
		_hauteur_feuillus.resize(nb_zone);
		_hauteur_decouver.clear();
		_hauteur_decouver.resize(nb_zone);

		_chaleur_conifers.clear();
		_chaleur_conifers.resize(nb_zone);
		_chaleur_feuillus.clear();
		_chaleur_feuillus.resize(nb_zone);
		_chaleur_decouver.clear();
		_chaleur_decouver.resize(nb_zone);

		_eau_retenu_conifers.clear();
		_eau_retenu_conifers.resize(nb_zone);
		_eau_retenu_feuillus.clear();
		_eau_retenu_feuillus.resize(nb_zone);
		_eau_retenu_decouver.clear();
		_eau_retenu_decouver.resize(nb_zone);

		//_superficieUhrhM1.clear();
		//_superficieUhrhM1.resize(nb_zone, 0.0);

		_albedo_conifers.clear();
		_albedo_conifers.resize(nb_zone);
		_albedo_feuillus.clear();
		_albedo_feuillus.resize(nb_zone);
		_albedo_decouver.clear();
		_albedo_decouver.resize(nb_zone);

		//_methode_albedo.resize(nb_zone, 0);

		// mise a jour de la neige avec stations de neige
		if (_stations_neige_conifers.PrendreNbStation() != 0 || _stations_neige_feuillus.PrendreNbStation() != 0 || _stations_neige_decouver.PrendreNbStation() != 0)
		{
			_mise_a_jour_neige = true;

			_maj_conifers.resize(nb_zone);
			_maj_feuillus.resize(nb_zone);
			_maj_decouver.resize(nb_zone);
		}
		else
			_mise_a_jour_neige = false;

		nb_zoneSim = zoneSimIndex.size();	//nb de zone simulé

		for(i=0; i!=nb_zoneSim; i++)
		{
			indexUhrh = zoneSimIndex[i];

			ZONE& zone = zones[indexUhrh];

			//determine le nb de bandes
			dNbBande = (_altMax[indexUhrh] - _altMin[indexUhrh]) / _dHauteurBande;
			iNbBande = static_cast<int>(dNbBande);

			if(iNbBande == 0)	//denivellation inferieur a hauteur bande
				iNbBande = 1;
			else
				++iNbBande;	//bande supplementaire pour l'exedent (altMax - altMin) / HauteurBande ne donne pas un resultat entier
							//si le resultat est entier la bande supplementaire est ajouté pour la valeur maximum (_altMax[indexUhrh]), qui theoriquement est l'altitude de depart de la bande suivante

			//calcule la superficie de chaque milieu pour chaque bande
			_bandeSuperficieM1[indexUhrh].resize(iNbBande, 0.0);
			_bandeSuperficieM2[indexUhrh].resize(iNbBande, 0.0);
			_bandeSuperficieM3[indexUhrh].resize(iNbBande, 0.0);

			_bandePourcentageM1[indexUhrh].resize(iNbBande, 0.0);
			_bandePourcentageM2[indexUhrh].resize(iNbBande, 0.0);
			_bandePourcentageM3[indexUhrh].resize(iNbBande, 0.0);

			_bandeAltMoyM1[indexUhrh].resize(iNbBande, 0.0);
			_bandeAltMoyM2[indexUhrh].resize(iNbBande, 0.0);
			_bandeAltMoyM3[indexUhrh].resize(iNbBande, 0.0);

			//M1
			nbPixel = _altPixelM1[indexUhrh].size();
			for(j=0; j!=nbPixel; j++)
			{
				idx = static_cast<size_t>( (_altPixelM1[indexUhrh][j] - _altMin[indexUhrh]) / _dHauteurBande );	//obtient l'index de la bande d'altitude pour le pixel

				_bandeSuperficieM1[indexUhrh][idx]+= dPixelArea;	//m2
				_bandeAltMoyM1[indexUhrh][idx]+= _altPixelM1[indexUhrh][j];	//altitude moy des bandes calculé selon les pixels correspondant au milieu

				//_superficieUhrhM1[indexUhrh]+= dPixelArea;	//m2
			}

			//M2
			nbPixel = _altPixelM2[indexUhrh].size();
			for(j=0; j!=nbPixel; j++)
			{
				idx = static_cast<size_t>( (_altPixelM2[indexUhrh][j] - _altMin[indexUhrh]) / _dHauteurBande );	//obtient l'index de la bande d'altitude pour le pixel

				_bandeSuperficieM2[indexUhrh][idx]+= dPixelArea;	//m2
				_bandeAltMoyM2[indexUhrh][idx]+= _altPixelM2[indexUhrh][j];	//altitude moy des bandes calculé selon les pixels correspondant au milieu

				//_superficieUhrhM2[indexUhrh]+= dPixelArea;	//m2
			}

			//M3
			nbPixel = _altPixelM3[indexUhrh].size();
			for(j=0; j!=nbPixel; j++)
			{
				idx = static_cast<size_t>( (_altPixelM3[indexUhrh][j] - _altMin[indexUhrh]) / _dHauteurBande );	//obtient l'index de la bande d'altitude pour le pixel

				_bandeSuperficieM3[indexUhrh][idx]+= dPixelArea;	//m2
				_bandeAltMoyM3[indexUhrh][idx]+= _altPixelM3[indexUhrh][j];	//altitude moy des bandes calculé selon les pixels correspondant au milieu

				//_superficieUhrhM3[indexUhrh]+= dPixelArea;	//m2
			}

			//calcul des donnees par bande
			superficieUhrh = zone.PrendreSuperficie() * 1000000.0;	//km2 -> m2

			_stock_conifers[indexUhrh].resize(iNbBande, 0.0);
			_stock_feuillus[indexUhrh].resize(iNbBande, 0.0);
			_stock_decouver[indexUhrh].resize(iNbBande, 0.0);

			_hauteur_conifers[indexUhrh].resize(iNbBande, 0.0);
			_hauteur_feuillus[indexUhrh].resize(iNbBande, 0.0);
			_hauteur_decouver[indexUhrh].resize(iNbBande, 0.0);

			_chaleur_conifers[indexUhrh].resize(iNbBande, 0.0);
			_chaleur_feuillus[indexUhrh].resize(iNbBande, 0.0);
			_chaleur_decouver[indexUhrh].resize(iNbBande, 0.0);

			_eau_retenu_conifers[indexUhrh].resize(iNbBande, 0.0);
			_eau_retenu_feuillus[indexUhrh].resize(iNbBande, 0.0);
			_eau_retenu_decouver[indexUhrh].resize(iNbBande, 0.0);

			_albedo_conifers[indexUhrh].resize(iNbBande, 0.8);
			_albedo_feuillus[indexUhrh].resize(iNbBande, 0.8);
			_albedo_decouver[indexUhrh].resize(iNbBande, 0.8);

			if(_mise_a_jour_neige)
			{
				_maj_conifers[indexUhrh].resize(iNbBande);
				_maj_feuillus[indexUhrh].resize(iNbBande);
				_maj_decouver[indexUhrh].resize(iNbBande);
			}

			for(x=0; x!=iNbBande; x++)
			{
				if(_mise_a_jour_neige)
				{
					_maj_conifers[indexUhrh][x].nb_pas_derniere_correction = 0;
					_maj_conifers[indexUhrh][x].pourcentage_corrige = 0;
					_maj_conifers[indexUhrh][x].pourcentage_sim_eq = 1;
					_maj_conifers[indexUhrh][x].pourcentage_sim_ha = 1;
					_maj_conifers[indexUhrh][x].stations_utilisees.resize( _stations_neige_conifers.PrendreNbStation() );
					
					_maj_feuillus[indexUhrh][x].nb_pas_derniere_correction = 0;
					_maj_feuillus[indexUhrh][x].pourcentage_corrige = 0;
					_maj_feuillus[indexUhrh][x].pourcentage_sim_eq = 1;
					_maj_feuillus[indexUhrh][x].pourcentage_sim_ha = 1;
					_maj_feuillus[indexUhrh][x].stations_utilisees.resize( _stations_neige_feuillus.PrendreNbStation() );

					_maj_decouver[indexUhrh][x].nb_pas_derniere_correction = 0;
					_maj_decouver[indexUhrh][x].pourcentage_corrige = 0;
					_maj_decouver[indexUhrh][x].pourcentage_sim_eq = 1;
					_maj_decouver[indexUhrh][x].pourcentage_sim_ha = 1;
					_maj_decouver[indexUhrh][x].stations_utilisees.resize( _stations_neige_decouver.PrendreNbStation() );
				}

				//calcul moyenne des altitudes des bandes et superficie (pourcentage) des bandes
				dArea = _bandeSuperficieM1[indexUhrh][x];
				if(dArea != 0.0)
				{
					_bandeAltMoyM1[indexUhrh][x]/= (dArea / dPixelArea);			//calcule moyenne des altitude	//divise la somme par le nb de pixel
					_bandePourcentageM1[indexUhrh][x] = dArea / superficieUhrh;
				}

				dArea = _bandeSuperficieM2[indexUhrh][x];
				if(dArea != 0.0)
				{
					_bandeAltMoyM2[indexUhrh][x]/= (dArea / dPixelArea);			//
					_bandePourcentageM2[indexUhrh][x] = dArea / superficieUhrh;
				}

				dArea = _bandeSuperficieM3[indexUhrh][x];
				if(dArea != 0.0)
				{
					_bandeAltMoyM3[indexUhrh][x]/= (dArea / dPixelArea);			//
					_bandePourcentageM3[indexUhrh][x] = dArea / superficieUhrh;
				}
			}

			//parametres pour calcul indice radiation
			theta = zone.PrendreCentroide().PrendreY() / dRAD1;

			k = atan(static_cast<double>(zone.PrendrePente()));
			h = ((495 - zone.PrendreOrientation() * 45) % 360) / dRAD1;

			_ce0[indexUhrh] = atan(sin(h) * sin(k) / (cos(k) * cos(theta) - cos(h) * sin(k) * sin(theta))) * dRAD1;
			_ce1[indexUhrh] = asin(sin(k) * cos(h) * cos(theta) + cos(k) * sin(theta)) * dRAD1;

			//if (_methode_albedo[indexUhrh] == 0)
			//{
			//	albedo = (_albedo_conifers[indexUhrh] + _albedo_feuillus[indexUhrh] + _albedo_decouver[indexUhrh]) / 3.0;
			//	_tsn[indexUhrh] = log( (albedo - 0.8) / 0.4 + 1.0 ) / -0.2;
			//}
		}

		//// mise a jour de la neige avec grille
		//if(_bMAJGrilleNeige)
		//{
		//	_maj_feuillus.resize(nb_zone);
		//	_maj_conifers.resize(nb_zone);
		//	_maj_decouver.resize(nb_zone);

		//	for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		//	{
		//		_maj_feuillus[index_zone].nb_pas_derniere_correction = 0;
		//		_maj_feuillus[index_zone].pourcentage_corrige = 0;
		//		_maj_feuillus[index_zone].pourcentage_sim_eq = 1;
		//		_maj_feuillus[index_zone].pourcentage_sim_ha = 1;
		//		_maj_feuillus[index_zone].stations_utilisees.resize( _stations_neige_feuillus.PrendreNbStation() );

		//		_maj_conifers[index_zone].nb_pas_derniere_correction = 0;
		//		_maj_conifers[index_zone].pourcentage_corrige = 0;
		//		_maj_conifers[index_zone].pourcentage_sim_eq = 1;
		//		_maj_conifers[index_zone].pourcentage_sim_ha = 1;
		//		_maj_conifers[index_zone].stations_utilisees.resize( _stations_neige_conifers.PrendreNbStation() );

		//		_maj_decouver[index_zone].nb_pas_derniere_correction = 0;
		//		_maj_decouver[index_zone].pourcentage_corrige = 0;
		//		_maj_decouver[index_zone].pourcentage_sim_eq = 1;
		//		_maj_decouver[index_zone].pourcentage_sim_ha = 1;
		//		_maj_decouver[index_zone].stations_utilisees.resize( _stations_neige_decouver.PrendreNbStation() );
		//	}

		//	_grilleneige.Initialise();
		//}
		//else
		//{
		//}

		// ouverture des fichiers output
		OUTPUT& output = _sim_hyd.PrendreOutput();
		if (output.SauvegardeCouvertNival())
		{
			//fichier couvert_nival.csv, par uhrh
			sFile = Combine(_sim_hyd.PrendreRepertoireResultat(), "couvert_nival.csv");
			_fichier_couvert_nival.open(sFile);
			_fichier_couvert_nival << "Couvert nival (EEN) (mm)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

			oss.str("");

			for (idx=0; idx!=zones.PrendreNbZone(); idx++)
			{
				if(_bUhrhIndexSimule[idx])
				{
					if (output._bSauvegardeTous || 
						find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[idx].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
					{
						oss << zones[idx].PrendreIdent() << output.Separator();
					}
				}
			}
			
			str = oss.str();
			str = str.substr(0, str.length()-1); //enleve le dernier separateur
			_fichier_couvert_nival << str << endl;

			//fichier couvert_nival_bande.csv, par uhrh/bande
			sFile = Combine(_sim_hyd.PrendreRepertoireResultat(), "couvert_nival_bande.csv");
			_fichier_couvert_nival_bande.open(sFile);
			_fichier_couvert_nival_bande << "Couvert nival bandes altitude (EEN) (mm)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh-bande" << output.Separator();

			oss.str("");

			for (idx=0; idx!=zones.PrendreNbZone(); idx++)
			{
				if(_bUhrhIndexSimule[idx])
				{
					if (output._bSauvegardeTous || 
						find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[idx].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
					{
						j = _bandePourcentageM1[idx].size();	//nb bande
						for(i=0; i!=j; i++)
							oss << zones[idx].PrendreIdent() << "-" << i+1 << output.Separator();
					}
				}
			}

			str = oss.str();
			str = str.substr(0, str.length()-1); //enleve le dernier separateur
			_fichier_couvert_nival_bande << str << endl;
		}

		//fichier hauteur_neige
		if (output.SauvegardeHauteurNeige())
		{
			sFile = Combine(_sim_hyd.PrendreRepertoireResultat(), "hauteur_neige.csv");
			_fichier_hauteur_neige.open(sFile);
			_fichier_hauteur_neige << "Hauteur couvert nival (m)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

			oss.str("");

			for (idx=0; idx!=zones.PrendreNbZone(); idx++)
			{
				if(_bUhrhIndexSimule[idx])
				{
					if (output._bSauvegardeTous || 
						find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[idx].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
					{
						oss << zones[idx].PrendreIdent() << output.Separator();
					}
				}
			}
			
			str = oss.str();
			str = str.substr(0, str.length()-1); //enleve le dernier separateur
			_fichier_hauteur_neige << str << endl;
		}

		//fichier albedo_neige
		if (output.SauvegardeAlbedoNeige())
		{
			sFile = Combine(_sim_hyd.PrendreRepertoireResultat(), "albedo_neige.csv");
			_fichier_albedo_neige.open(sFile);
			_fichier_albedo_neige << "Albedo neige (0-1)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

			oss.str("");

			for (idx=0; idx!=zones.PrendreNbZone(); idx++)
			{
				if(_bUhrhIndexSimule[idx])
				{
					if (output._bSauvegardeTous || 
						find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[idx].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
					{
						oss << zones[idx].PrendreIdent() << output.Separator();
					}
				}
			}
			
			str = oss.str();
			str = str.substr(0, str.length()-1); //enleve le dernier separateur
			_fichier_albedo_neige << str << endl;
		}

		//fichier fonteneige-meteo.csv
		sFile = Combine(_sim_hyd.PrendreRepertoireResultat(), "fonteneige-meteo.csv");
		_outMeteoBandes.open(sFile);
		_outMeteoBandes << "Meteo bandes altitude" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl;
		_outMeteoBandes << "DateHeure" << output.Separator() << "Uhrh" << output.Separator() << "Bande" << output.Separator() << "Milieu" << output.Separator() << "TempMin (dC)" << output.Separator() << "TempMax (dC)" << output.Separator() << "Pluie (mm)" << output.Separator() << "Neige (EEN) (mm)" << endl;

		//Fichier d'état
		if (!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		FONTE_NEIGE::Initialise();
	}


	void DEGRE_JOUR_BANDE::PassagePluieNeige(double dTempPassagePluieNeige, double dTMin, double dTMax, double dPrecip, double* dPluie, double* dEEN)
	{
		unsigned short pas_de_temps;
		double taux_transformation;

		pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		*dPluie = 0.0;
		*dEEN = 0.0;

		if(pas_de_temps == 1)
		{
			if(dTMin > dTempPassagePluieNeige)
				*dPluie = dPrecip;	//tout en pluie
			else
				*dEEN = dPrecip;	//tout en neige
		}
		else
		{
			if(dTMax < dTempPassagePluieNeige)
				taux_transformation = 0.0;
			else
			{
				if(dTMin >= dTempPassagePluieNeige)
					taux_transformation = 1.0;
				else
				{
					taux_transformation = (dTMax - dTempPassagePluieNeige) / (dTMax - dTMin);
					if(taux_transformation > 1.0)
						taux_transformation = 1.0;
				}
			}

			*dPluie = taux_transformation * dPrecip;
			*dEEN = (1.0 - taux_transformation) * dPrecip;
		}
	}


	void DEGRE_JOUR_BANDE::Calcule()
	{
		//STATION_NEIGE::typeOccupationStation occupation;	//pour maj grille neige
		ostringstream oss;
		string str, sString, sString2, sString3;
		size_t index, nbZone, index_zone, nbBande, idxBande;
		double apport, apport_conifers, apport_feuillus, apport_decouver, dAltMoyUhrh, dGradientVerticalTemp, dGradientVerticalPrecip, dTempPassagePluieNeige, dVal;
		double diff_alt, dPrecipNeigeHauteur, stock_moyen, albedo_moyen, dHauteurCouvertNival;
		double dPourcentM1, dPourcentM2, dPourcentM3, dSuperficieTotalBande;
		double apportMilieu, dDensite, precipMilieu, pluieMilieu, neigeMilieu, TMinMilieu, TMaxMilieu;
		double TMoyZone, TMinZone, TMaxZone, precipZone;
		//double dIndiceRadiationUhrh, dCoeffAdditif, albedo, neige;
		bool bMajEffectuer;

		ZONES& zones = _sim_hyd.PrendreZones();

		OUTPUT& output = _sim_hyd.PrendreOutput();

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		//if(_bMAJGrilleNeige)
		//{
		//	//lecture des grilles equivalent en eau & hauteur de neige pour le pas de temps courant
		//	_grilleneige._grilleEquivalentEau.clear();
		//	_grilleneige._grilleHauteurNeige.clear();

		//	_grilleneige.FormatePathFichierGrilleCourant(sString);
		//	sString2 = sString + ".een";
		//	sString3 = sString + ".hau";
		//	if(FichierExiste(sString2) && FichierExiste(sString3))
		//	{
		//		_grilleneige._grilleEquivalentEau.push_back(ReadGeoTIFF_float(sString2));
		//		_grilleneige._grilleHauteurNeige.push_back(ReadGeoTIFF_float(sString3));
		//	}
		//}

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();
		nbZone = index_zones.size();

		for(index=0; index!=nbZone; index++)
		{
			index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			zone._precip_m1.clear();
			zone._precip_m2.clear();
			zone._precip_m3.clear();
			
			zone._apport_m1.clear();
			zone._apport_m2.clear();
			zone._apport_m3.clear();

			CalculIndiceRadiation(date_courante, pas_de_temps, zone, index_zone);

			//// calcul albedo de la neige
			//if (_methode_albedo[index_zone] == 0)	//methode albedo fixé pour l'instant à 1 (sol-neige)
			//{
			//	neige = static_cast<double>(zone.PrendreNeige()) / 10.0;	//mm -> cm
			//
			//	_tsn[index_zone] = neige < _seuil_albedo[index_zone] ? _tsn[index_zone] + static_cast<double>(pas_de_temps) / 24.0 : 0.0;
			//
			//	albedo = min(0.8 - 0.4 * (1.0 - exp(-0.2 * _tsn[index_zone])), 1.0);
			//
			//	_albedo_conifers[index_zone] = albedo;
			//	_albedo_feuillus[index_zone] = albedo;
			//	_albedo_decouver[index_zone] = albedo;
			//}
			
			//TODO lorsque le mode lecture est activé pour interpolation, le fichier de parametres du mode lecture ne contient pas les parametres gradients et passage pluie/neige.
			//     ces parametres devrait etre dans le modele degre_jour_bande.
			//     pour l'instant on prend les meme params que le modele interpolation (ou thiessen si mode lecture)

			if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "THIESSEN2")
			{
				dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen2->PrendreGradientTemperature(index_zone));
				dGradientVerticalPrecip = static_cast<double>(_sim_hyd._smThiessen2->PrendreGradientPrecipitation(index_zone));
				dTempPassagePluieNeige = static_cast<double>(_sim_hyd._smThiessen2->PrendrePassagePluieNeige(index_zone));
			}
			else
			{
				if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "MOYENNE 3 STATIONS2")
				{
					dGradientVerticalTemp = static_cast<double>(_sim_hyd._smMoy3station2->PrendreGradientTemperature(index_zone));
					dGradientVerticalPrecip = static_cast<double>(_sim_hyd._smMoy3station2->PrendreGradientPrecipitation(index_zone));
					dTempPassagePluieNeige = static_cast<double>(_sim_hyd._smMoy3station2->PrendrePassagePluieNeige(index_zone));
				}
				else
				{
					if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "THIESSEN1")
					{
						dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen1->PrendreGradientTemperature(index_zone));
						dGradientVerticalPrecip = static_cast<double>(_sim_hyd._smThiessen1->PrendreGradientPrecipitation(index_zone));
						dTempPassagePluieNeige = static_cast<double>(_sim_hyd._smThiessen1->PrendrePassagePluieNeige(index_zone));
					}
					else
					{
						if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "MOYENNE 3 STATIONS1")
						{
							dGradientVerticalTemp = static_cast<double>(_sim_hyd._smMoy3station1->PrendreGradientTemperature(index_zone));
							dGradientVerticalPrecip = static_cast<double>(_sim_hyd._smMoy3station1->PrendreGradientPrecipitation(index_zone));
							dTempPassagePluieNeige = static_cast<double>(_sim_hyd._smMoy3station1->PrendrePassagePluieNeige(index_zone));
						}
						else
						{
							if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "GRILLE")
							{
								dGradientVerticalTemp = static_cast<double>(_sim_hyd._smGrilleMeteo->PrendreGradientTemperature(index_zone));
								dGradientVerticalPrecip = static_cast<double>(_sim_hyd._smGrilleMeteo->PrendreGradientPrecipitation(index_zone));
								dTempPassagePluieNeige = static_cast<double>(_sim_hyd._smGrilleMeteo->PrendrePassagePluieNeige(index_zone));
							}
							else
							{
								//LECTURE_INTERPOLATION_DONNEES	//use params of thiessen model
								if(_sim_hyd._versionTHIESSEN == 1)
								{
									dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen1->PrendreGradientTemperature(index_zone));
									dGradientVerticalPrecip = static_cast<double>(_sim_hyd._smThiessen1->PrendreGradientPrecipitation(index_zone));
									dTempPassagePluieNeige = static_cast<double>(_sim_hyd._smThiessen1->PrendrePassagePluieNeige(index_zone));
								}
								else
								{
									//_sim_hyd._versionTHIESSEN == 2
									dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen2->PrendreGradientTemperature(index_zone));
									dGradientVerticalPrecip = static_cast<double>(_sim_hyd._smThiessen2->PrendreGradientPrecipitation(index_zone));
									dTempPassagePluieNeige = static_cast<double>(_sim_hyd._smThiessen2->PrendrePassagePluieNeige(index_zone));
								}
							}
						}
					}
				}
			}

			//
			dAltMoyUhrh = static_cast<double>(zone.PrendreAltitude());

			TMoyZone = (static_cast<double>(zone.PrendreTMin()) + static_cast<double>(zone.PrendreTMax())) / 2.0;
			TMinZone = static_cast<double>(zone.PrendreTMin());
			TMaxZone = static_cast<double>(zone.PrendreTMax());
			
			//transforme la neige en equivalent en eau (EEN) avant l'application du gradient vertical (dans le modele d'interpolation, le gradient est appliqué sur la precip totale en eau avant la répartition pluie/neige)
			if(pas_de_temps == 1)
				dDensite = CalculDensiteNeige(TMinZone) / DENSITE_EAU;
			else
				dDensite = CalculDensiteNeige(TMoyZone) / DENSITE_EAU;

			precipZone = static_cast<double>(zone.PrendrePluie()) + ( static_cast<double>(zone.PrendreNeige()) * dDensite );	//mm	//transforme la neige en equivalent en eau

			nbBande = _bandeSuperficieM1[index_zone].size();
			
			dHauteurCouvertNival = 0.0;
			albedo_moyen = 0.0;
			stock_moyen = 0.0;
			apport = 0.0;

			for(idxBande=0; idxBande!=nbBande; idxBande++)
			{
				//M1
				precipMilieu = 0.0;	//mm
				apportMilieu = 0.0;	//mm

				if(_bandePourcentageM1[index_zone][idxBande] != 0.0)
				{
					diff_alt = _bandeAltMoyM1[index_zone][idxBande] - dAltMoyUhrh;
					
					TMinMilieu = TMinZone + dGradientVerticalTemp * diff_alt / 100.0;
					TMaxMilieu = TMaxZone + dGradientVerticalTemp * diff_alt / 100.0;

					precipMilieu = precipZone * (1.0 + (dGradientVerticalPrecip / 1000.0) / 100.0 * diff_alt);	//mm
					if(precipMilieu < 0.0)
						precipMilieu = 0.0;

					PassagePluieNeige(dTempPassagePluieNeige, TMinMilieu, TMaxMilieu, precipMilieu, &pluieMilieu, &neigeMilieu);

					if(_sim_hyd.PrendreDateCourante().PrendreAnnee() == _sim_hyd.PrendreDateDebut().PrendreAnnee())		//seulement pour la 1ere annee sinon le fichier output est trop volumineux
					{
						if(output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zone.PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss.str("");
							oss << _sim_hyd.PrendreDateCourante() << output.Separator();
							oss << zone.PrendreIdent() << output.Separator() << idxBande+1 << output.Separator() << "1" << output.Separator();
							oss << setprecision(4) << setiosflags(ios::fixed) << TMinMilieu << output.Separator() << TMaxMilieu << output.Separator() << pluieMilieu << output.Separator() << neigeMilieu;
							_outMeteoBandes << oss.str() << endl;
						}
					}

					//ramene la neige en hauteur de neige (EEN -> HauteurNeige)
					dDensite = CalculDensiteNeige( (TMinMilieu+TMaxMilieu) / 2.0 ) / DENSITE_EAU;
					dPrecipNeigeHauteur = neigeMilieu / dDensite;	//mm

					CalculeFonte(zone, index_zone, pas_de_temps, ((TMinMilieu+TMaxMilieu)/2.0), pluieMilieu, dPrecipNeigeHauteur, _bandePourcentageM1[index_zone][idxBande], 
									_taux_fonte_conifers[index_zone] / 1000.0, _seuil_fonte_conifers[index_zone], _albedo_conifers[index_zone][idxBande], 
									_stock_conifers[index_zone][idxBande], _hauteur_conifers[index_zone][idxBande], _chaleur_conifers[index_zone][idxBande], apport_conifers, _eau_retenu_conifers[index_zone][idxBande]);

					apportMilieu = apport_conifers * 1000.0;	//m -> mm

					albedo_moyen+= _bandePourcentageM1[index_zone][idxBande] * _albedo_conifers[index_zone][idxBande];
					dHauteurCouvertNival+= _bandePourcentageM1[index_zone][idxBande] * _hauteur_conifers[index_zone][idxBande];
					stock_moyen+= _bandePourcentageM1[index_zone][idxBande] * _stock_conifers[index_zone][idxBande];
					apport+= apport_conifers;
				}
				zone._precip_m1.push_back(precipMilieu);
				zone._apport_m1.push_back(apportMilieu);

				//M2
				precipMilieu = 0.0;	//mm
				apportMilieu = 0.0;	//mm

				if(_bandePourcentageM2[index_zone][idxBande] != 0.0)
				{
					diff_alt = _bandeAltMoyM2[index_zone][idxBande] - dAltMoyUhrh;
					
					TMinMilieu = TMinZone + dGradientVerticalTemp * diff_alt / 100.0;
					TMaxMilieu = TMaxZone + dGradientVerticalTemp * diff_alt / 100.0;

					precipMilieu = precipZone * (1.0 + (dGradientVerticalPrecip / 1000.0) / 100.0 * diff_alt);	//mm
					if(precipMilieu < 0.0)
						precipMilieu = 0.0;

					PassagePluieNeige(dTempPassagePluieNeige, TMinMilieu, TMaxMilieu, precipMilieu, &pluieMilieu, &neigeMilieu);

					if(_sim_hyd.PrendreDateCourante().PrendreAnnee() == _sim_hyd.PrendreDateDebut().PrendreAnnee())		//seulement pour la 1ere annee sinon le fichier output est trop volumineux
					{
						if(output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zone.PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss.str("");
							oss << _sim_hyd.PrendreDateCourante() << output.Separator();
							oss << zone.PrendreIdent() << output.Separator() << idxBande+1 << output.Separator() << "2" << output.Separator();
							oss << setprecision(4) << setiosflags(ios::fixed) << TMinMilieu << output.Separator() << TMaxMilieu << output.Separator() << pluieMilieu << output.Separator() << neigeMilieu;
							_outMeteoBandes << oss.str() << endl;
						}
					}

					//ramene la neige en hauteur de neige (EEN -> HauteurNeige)
					dDensite = CalculDensiteNeige( (TMinMilieu+TMaxMilieu) / 2.0 ) / DENSITE_EAU;
					dPrecipNeigeHauteur = neigeMilieu / dDensite;	//mm

					CalculeFonte(zone, index_zone, pas_de_temps, ((TMinMilieu+TMaxMilieu)/2.0), pluieMilieu, dPrecipNeigeHauteur, _bandePourcentageM2[index_zone][idxBande], 
									_taux_fonte_feuillus[index_zone] / 1000.0, _seuil_fonte_feuillus[index_zone], _albedo_feuillus[index_zone][idxBande], 
									_stock_feuillus[index_zone][idxBande], _hauteur_feuillus[index_zone][idxBande], _chaleur_feuillus[index_zone][idxBande], apport_feuillus, _eau_retenu_feuillus[index_zone][idxBande]);

					apportMilieu = apport_feuillus * 1000.0;	//m -> mm

					albedo_moyen+= _bandePourcentageM2[index_zone][idxBande] * _albedo_feuillus[index_zone][idxBande];
					dHauteurCouvertNival+= _bandePourcentageM2[index_zone][idxBande] * _hauteur_feuillus[index_zone][idxBande];
					stock_moyen+= _bandePourcentageM2[index_zone][idxBande] * _stock_feuillus[index_zone][idxBande];
					apport+= apport_feuillus;
				}
				zone._precip_m2.push_back(precipMilieu);
				zone._apport_m2.push_back(apportMilieu);

				//M3
				precipMilieu = 0.0;	//mm
				apportMilieu = 0.0;	//mm

				if(_bandePourcentageM3[index_zone][idxBande] != 0.0)
				{
					diff_alt = _bandeAltMoyM3[index_zone][idxBande] - dAltMoyUhrh;
					
					TMinMilieu = TMinZone + dGradientVerticalTemp * diff_alt / 100.0;
					TMaxMilieu = TMaxZone + dGradientVerticalTemp * diff_alt / 100.0;

					precipMilieu = precipZone * (1.0 + (dGradientVerticalPrecip / 1000.0) / 100.0 * diff_alt);	//mm
					if(precipMilieu < 0.0)
						precipMilieu = 0.0;

					PassagePluieNeige(dTempPassagePluieNeige, TMinMilieu, TMaxMilieu, precipMilieu, &pluieMilieu, &neigeMilieu);

					if(_sim_hyd.PrendreDateCourante().PrendreAnnee() == _sim_hyd.PrendreDateDebut().PrendreAnnee())		//seulement pour la 1ere annee sinon le fichier output est trop volumineux
					{
						if(output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zone.PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss.str("");
							oss << _sim_hyd.PrendreDateCourante() << output.Separator();
							oss << zone.PrendreIdent() << output.Separator() << idxBande+1 << output.Separator() << "3" << output.Separator();
							oss << setprecision(4) << setiosflags(ios::fixed) << TMinMilieu << output.Separator() << TMaxMilieu << output.Separator() << pluieMilieu << output.Separator() << neigeMilieu;
							_outMeteoBandes << oss.str() << endl;
						}
					}

					//ramene la neige en hauteur de neige (EEN -> HauteurNeige)
					dDensite = CalculDensiteNeige( (TMinMilieu+TMaxMilieu) / 2.0 ) / DENSITE_EAU;
					dPrecipNeigeHauteur = neigeMilieu / dDensite;	//mm

					CalculeFonte(zone, index_zone, pas_de_temps, ((TMinMilieu+TMaxMilieu)/2.0), pluieMilieu, dPrecipNeigeHauteur, _bandePourcentageM3[index_zone][idxBande], 
									_taux_fonte_decouver[index_zone] / 1000.0, _seuil_fonte_decouver[index_zone], _albedo_decouver[index_zone][idxBande], 
									_stock_decouver[index_zone][idxBande], _hauteur_decouver[index_zone][idxBande], _chaleur_decouver[index_zone][idxBande], apport_decouver, _eau_retenu_decouver[index_zone][idxBande]);

					apportMilieu = apport_decouver * 1000.0;	//m -> mm

					albedo_moyen+= _bandePourcentageM3[index_zone][idxBande] * _albedo_decouver[index_zone][idxBande];
					dHauteurCouvertNival+= _bandePourcentageM3[index_zone][idxBande] * _hauteur_decouver[index_zone][idxBande];
					stock_moyen+= _bandePourcentageM3[index_zone][idxBande] * _stock_decouver[index_zone][idxBande];
					apport+= apport_decouver;
				}
				zone._precip_m3.push_back(precipMilieu);
				zone._apport_m3.push_back(apportMilieu);
			}


			//if(_bMAJGrilleNeige)
			//{
			//	if(_grilleneige._grilleEquivalentEau.size() != 0)	//s'il y a des données pour le jour courant
			//	{
			//		// On prend l occupation du sol dominant de lUHRH
			//		occupation = STATION_NEIGE::RESINEUX;
			//					
			//		if(_pourcentage_conifers[index_zone] < _pourcentage_feuillus[index_zone])
			//		{
			//			occupation = STATION_NEIGE::FEUILLUS;
			//			if ( _pourcentage_feuillus[index_zone] < _pourcentage_autres[index_zone])
			//				occupation = STATION_NEIGE::DECOUVERTE;
			//		}
			//		else
			//		{
			//			if(_pourcentage_conifers[index_zone] < _pourcentage_autres[index_zone])
			//				occupation = STATION_NEIGE::DECOUVERTE;
			//		}

			//		MiseAJourGrille(index_zone, occupation);
			//	}
			//}
			//else
			//{

				if (_mise_a_jour_neige)
				{
					MiseAJour(date_courante, index_zone, bMajEffectuer);

					if(bMajEffectuer)
					{
						stock_moyen = 0.0;
						dHauteurCouvertNival = 0.0;

						for(idxBande=0; idxBande!=nbBande; idxBande++)
						{
							dHauteurCouvertNival+= (_bandePourcentageM1[index_zone][idxBande] * _hauteur_conifers[index_zone][idxBande] + 
								_bandePourcentageM2[index_zone][idxBande] * _hauteur_feuillus[index_zone][idxBande] + _bandePourcentageM3[index_zone][idxBande] * _hauteur_decouver[index_zone][idxBande]);

							stock_moyen+= (_bandePourcentageM1[index_zone][idxBande] * _stock_conifers[index_zone][idxBande] + 
								_bandePourcentageM2[index_zone][idxBande] * _stock_feuillus[index_zone][idxBande] + _bandePourcentageM3[index_zone][idxBande] * _stock_decouver[index_zone][idxBande]);
						}
					}
				}

			//}

			apport*= 1000.0;		//[m] -> [mm]
			zone.ChangeApport(static_cast<float>(apport));

			stock_moyen*= 1000.0;	//[m] -> [mm]
			if(stock_moyen >= 0.001)
			{
				zone.ChangeAlbedoNeige(static_cast<float>(albedo_moyen));
				zone.ChangeCouvertNival(static_cast<float>(stock_moyen));   //equivalent en eau de la neige [mm]
			}
			else
			{
				zone.ChangeAlbedoNeige(0.0f);
				zone.ChangeCouvertNival(0.0f);
			}

			zone.ChangeHauteurCouvertNival(static_cast<float>(dHauteurCouvertNival));	//m

			//conserve les stocks de l'uhrh par milieu et par bande pour le pdt courant
			zone._couvert_nival_m1.clear();
			zone._couvert_nival_m2.clear();
			zone._couvert_nival_m3.clear();

			for(idxBande=0; idxBande!=nbBande; idxBande++)
			{
				if(_stock_conifers[index_zone][idxBande] * 1000.0 >= 0.001)
					zone._couvert_nival_m1.push_back(_stock_conifers[index_zone][idxBande] * 1000.0);	// * 1000: [m] -> [mm]
				else
					zone._couvert_nival_m1.push_back(0.0);

				if(_stock_feuillus[index_zone][idxBande] * 1000.0 >= 0.001)
					zone._couvert_nival_m2.push_back(_stock_feuillus[index_zone][idxBande] * 1000.0);	// * 1000: [m] -> [mm]
				else
					zone._couvert_nival_m2.push_back(0.0);

				if(_stock_decouver[index_zone][idxBande] * 1000.0 >= 0.001)
					zone._couvert_nival_m3.push_back(_stock_decouver[index_zone][idxBande] * 1000.0);	// * 1000: [m] -> [mm]
				else
					zone._couvert_nival_m3.push_back(0.0);
			}
		}

		// correction de la neige au sol

		//for(auto iter = begin(_corrections_neige_au_sol); iter != end(_corrections_neige_au_sol); ++iter)
		//{
		//	CORRECTION* correction = *iter;

		//	if (correction->Applicable(date_courante))
		//	{
		//		GROUPE_ZONE* groupe_zone = nullptr;

		//		switch (correction->PrendreTypeGroupe())
		//		{
		//		case TYPE_GROUPE_ALL:
		//			groupe_zone = _sim_hyd.PrendreToutBassin();
		//			break;

		//		case TYPE_GROUPE_HYDRO:
		//			groupe_zone = _sim_hyd.RechercheGroupeZone(correction->PrendreNomGroupe());
		//			break;

		//		case TYPE_GROUPE_CORRECTION:
		//			groupe_zone = _sim_hyd.RechercheGroupeCorrection(correction->PrendreNomGroupe());
		//			break;
		//		}

		//		fCoeffAdditif = correction->PrendreCoefficientAdditif() / 1000.0f;

		//		for (size_t index = 0; index < groupe_zone->PrendreNbZone(); ++index)
		//		{
		//			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
		//			{
		//				int ident = groupe_zone->PrendreIdent(index);
		//				size_t index_zone = zones.IdentVersIndex(ident);

		//				float stock_avant;
		//			
		//				stock_avant = _stock_conifers[index_zone];
		//				_stock_conifers[index_zone] = (stock_avant + fCoeffAdditif) * correction->PrendreCoefficientMultiplicatif();
		//				if(stock_avant != 0.0)
		//				{
		//					_chaleur_conifers[index_zone] = _chaleur_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
		//					_eau_retenu_conifers[index_zone] = _eau_retenu_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
		//				}

		//				stock_avant = _stock_feuillus[index_zone];
		//				_stock_feuillus[index_zone] = (stock_avant + fCoeffAdditif) * correction->PrendreCoefficientMultiplicatif();
		//				if(stock_avant != 0.0)
		//				{
		//					_chaleur_feuillus[index_zone] = _chaleur_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
		//					_eau_retenu_feuillus[index_zone] = _eau_retenu_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
		//				}

		//				stock_avant = _stock_decouver[index_zone];
		//				_stock_decouver[index_zone] = (stock_avant + fCoeffAdditif) * correction->PrendreCoefficientMultiplicatif();
		//				if(stock_avant != 0.0)
		//				{
		//					_chaleur_decouver[index_zone] = _chaleur_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
		//					_eau_retenu_decouver[index_zone] = _eau_retenu_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
		//				}
		//			}
		//		}
		//	}
		//}

		if(output.SauvegardeCouvertNival() || output.SauvegardeHauteurNeige() || output.SauvegardeAlbedoNeige())
		{
			ostringstream ossCN, ossCNB, ossHN, ossAN;

			if(output.SauvegardeCouvertNival())	//fichier couvert_nival.csv & couvert_nival_bande.csv
			{
				ossCN << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);
				ossCNB << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);
			}

			if(output.SauvegardeHauteurNeige())	//fichier hauteur_neige.csv
				ossHN << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

			if(output.SauvegardeAlbedoNeige())	//fichier albedo_neige.csv
				ossAN << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

			for(index=0; index!=zones.PrendreNbZone(); index++)
			{
				if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
				{
					if(output._bSauvegardeTous || 
						find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
					{
						if(output.SauvegardeCouvertNival())
						{
							ossCN << zones[index].PrendreCouvertNival() << output.Separator();	//equivalent en eau du couvert nival	//mm

							nbBande = _bandePourcentageM1[index].size();
							for(idxBande=0; idxBande!=nbBande; idxBande++)
							{
								dSuperficieTotalBande = _bandeSuperficieM1[index][idxBande] + _bandeSuperficieM2[index][idxBande] + _bandeSuperficieM3[index][idxBande];
								dPourcentM1 = _bandeSuperficieM1[index][idxBande] / dSuperficieTotalBande;
								dPourcentM2 = _bandeSuperficieM2[index][idxBande] / dSuperficieTotalBande;
								dPourcentM3 = _bandeSuperficieM3[index][idxBande] / dSuperficieTotalBande;

								dVal = dPourcentM1 * _stock_conifers[index][idxBande] + dPourcentM2 * _stock_feuillus[index][idxBande] + dPourcentM3 * _stock_decouver[index][idxBande];
								dVal*= 1000.0;	//[m] -> [mm]
								ossCNB << dVal << output.Separator();	//equivalent en eau du couvert nival de la bande	//mm
							}
						}

						if(output.SauvegardeHauteurNeige())
							ossHN << zones[index].PrendreHauteurCouvertNival() << output.Separator();	//m

						if(output.SauvegardeAlbedoNeige())
							ossAN << zones[index].PrendreAlbedoNeige() << output.Separator();
					}
				}
			}

			if(output.SauvegardeCouvertNival())
			{
				str = ossCN.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_couvert_nival << str << endl;

				str = ossCNB.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_couvert_nival_bande << str << endl;
			}

			if(output.SauvegardeHauteurNeige())
			{
				str = ossHN.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_hauteur_neige << str << endl;
			}

			if(output.SauvegardeAlbedoNeige())
			{
				str = ossAN.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_albedo_neige << str << endl;
			}
		}

		//Fichier d'état
		if (_sauvegarde_tous_etat || (_sauvegarde_etat && (_date_sauvegarde_etat - pas_de_temps == date_courante)))
			SauvegardeEtat(date_courante);

		FONTE_NEIGE::Calcule();
	}


	void DEGRE_JOUR_BANDE::Termine()
	{
		_ce1.clear();
		_ce0.clear();
		//_tsn.clear();

		_stock_conifers.clear();
		_stock_feuillus.clear();
		_stock_decouver.clear();

		_hauteur_conifers.clear();
		_hauteur_feuillus.clear();
		_hauteur_decouver.clear();

		_chaleur_conifers.clear();
		_chaleur_feuillus.clear();
		_chaleur_decouver.clear();

		_eau_retenu_conifers.clear();
		_eau_retenu_feuillus.clear();
		_eau_retenu_decouver.clear();

		_albedo_conifers.clear();
		_albedo_feuillus.clear();
		_albedo_decouver.clear();

		_methode_albedo.clear();

		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeCouvertNival())
		{
			_fichier_couvert_nival.close();
			_fichier_couvert_nival_bande.close();
		}
		if (output.SauvegardeHauteurNeige())
			_fichier_hauteur_neige.close();
		if (output.SauvegardeAlbedoNeige())
			_fichier_albedo_neige.close();

		_outMeteoBandes.close();

		_maj_conifers.clear();
		_maj_feuillus.clear();
		_maj_decouver.clear();

		FONTE_NEIGE::Termine();
	}


	void DEGRE_JOUR_BANDE::CalculIndiceRadiation(DATE_HEURE date_heure, unsigned short pas_de_temps, ZONE& zone, size_t index_zone)
	{
		const double i0 = dCONSTANTE_SOLAIRE;	// constante solaire en Watt/m2.
		const double w = 15.0 / dRAD1;	// vitesse angulaire de la rotation de la terre

		double theta, theta1, alpha, i_j1, i_j2, jour, heure, e2, i_e2, decli, duree_hor, duree_pte, tampon;
		double t1_pte, t2_pte, t1_pte_sim, t2_pte_sim, t1_hor_sim, t2_hor_sim, t1, t2, dIndiceRadiation;

		theta = zone.PrendreCentroide().PrendreY() / dRAD1;
		theta1 = _ce1[index_zone] / dRAD1;
		alpha = _ce0[index_zone] / dRAD1;

		jour = static_cast<double>(date_heure.PrendreJourJulien());
		heure = static_cast<double>(date_heure.PrendreHeure());

		// calcul du vecteur radian
		e2 = pow(1.0 - dEXENTRICITE_ORBITE_TERRESTRE * cos((jour - 4.0) / dDEG1), 2.0);
		i_e2 = i0 / e2;

		// calcul de la declinaison
		decli = 0.410152374218 * sin((jour - 80.25) / dDEG1);

		duree_hor = 0.0;
		duree_pte = 0.0;

		// demi-duree du jour sur une surface horizontale
		tampon = -tan(theta) * tan(decli);
		if (tampon > 1.0)
			duree_hor = 0.0;
		else if (tampon < -1.0)
			duree_hor = 12.0;
		else
			duree_hor = acos(tampon) / w;

		// duree du jour sur une surface en pente
		tampon = -tan(theta1) * tan(decli);
		if (tampon > 1.0)
			duree_pte = 0.0;
		else if (tampon < -1.0)
			duree_pte = 12.0;
		else
			duree_pte = acos(tampon) / w;

		// leve et couche du soleil pour une surface en pente
		t1_pte = -duree_pte - alpha / w;
		t2_pte = duree_pte - alpha / w;

		if (t1_pte < -duree_hor)
			t1_pte = -duree_hor;
		if (t2_pte > duree_hor)
			t2_pte = duree_hor;

		// Si le pas de temps de la simulation (en heure) est inferieur a 24h
		// alors il ne suffit pas de calculer pour une surface en pente la duree du
		// jour, le leve et le couche du soleil. Mais il faut inclure seulement les
		// heures qu'on simule.

		//calculs pour un pas de temps de 24h

		t1_pte_sim = t1_pte;
		t2_pte_sim = t2_pte;
		t1_hor_sim = -duree_hor;
		t2_hor_sim = duree_hor;

		i_j1 = 0.0;
		i_j2 = 0.0;

		// calcul de l'ensoleillement d'une surface horizontale
		if (t1_hor_sim > t2_hor_sim) 
			i_j1 = 0.0;
		else
			i_j1 = 3600.0 * i_e2 * ((t2_hor_sim - t1_hor_sim) * sin(theta) * sin(decli) + 1.0 / w * cos(theta) * cos(decli) * (sin(w * t2_hor_sim) - sin(w * t1_hor_sim)));

		// calcul de l'ensoleillement d'une surface en pente
		if (t1_pte_sim > t2_pte_sim) 
			i_j2 = 0.0;
		else
			i_j2 = 3600.0 * i_e2 * ((t2_pte_sim - t1_pte_sim) * sin(theta1) * sin(decli) + 1.0 / w * cos(theta1)*cos(decli) * (sin(w * t2_pte_sim + alpha) - sin(w * t1_pte_sim + alpha)));

		dIndiceRadiation = i_j1 != 0.0 ? fabs(i_j2 / i_j1) : 1.0;

		if (pas_de_temps != 24)
		{
			//calculs pour un pas de temps inférieur à 24h

			double dIndiceRadiation24H = dIndiceRadiation; 

			t1 = heure - 12.0;
			t2 = heure + static_cast<double>(pas_de_temps) - 12.0;

			t1_pte_sim = max(t1, t1_pte);
			t2_pte_sim = min(t2, t2_pte);

			t1_hor_sim = max(t1, -duree_hor);
			t2_hor_sim = min(t2, duree_hor);

			i_j1 = 0.0;
			i_j2 = 0.0;

			// calcul de l'ensoleillement d'une surface horizontale
			if (t1_hor_sim > t2_hor_sim) 
				i_j1 = 0.0;
			else
				i_j1 = 3600.0 * i_e2 * ((t2_hor_sim - t1_hor_sim) * sin(theta) * sin(decli) + 1.0 / w * cos(theta) * cos(decli) * (sin(w * t2_hor_sim) - sin(w * t1_hor_sim)));

			// calcul de l'ensoleillement d'une surface en pente
			if (t1_pte_sim > t2_pte_sim) 
				i_j2 = 0.0;
			else
				i_j2 = 3600.0 * i_e2 * ((t2_pte_sim - t1_pte_sim) * sin(theta1) * sin(decli) + 1.0 / w * cos(theta1)*cos(decli) * (sin(w * t2_pte_sim + alpha) - sin(w * t1_pte_sim + alpha)));

			dIndiceRadiation = i_j1 != 0.0 ? fabs(i_j2 / i_j1) : 1.0;

			if(dIndiceRadiation24H < dIndiceRadiation)
				dIndiceRadiation = dIndiceRadiation24H;
		}

		zone._dRayonnementSolaire = max(0.0, i_j2);
		zone._dDureeJour = max(0.0, t2_hor_sim - t1_hor_sim);
		zone._dIndiceRadiation = dIndiceRadiation;

		zone.ChangeRayonnementSolaire(max(0.0f, static_cast<float>(i_j2)));
		zone.ChangeDureeJour(max(0.0f, static_cast<float>(t2_hor_sim - t1_hor_sim)));
		zone.ChangeIndiceRadiation(static_cast<float>(dIndiceRadiation));
	}


	void DEGRE_JOUR_BANDE::ChangeSeuilFonteFeuillus(size_t index_zone, double seuil_fonte)
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_feuillus.size());
		_seuil_fonte_feuillus[index_zone] = seuil_fonte;
	}

	void DEGRE_JOUR_BANDE::ChangeSeuilFonteConifers(size_t index_zone, double seuil_fonte)
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_conifers.size());
		_seuil_fonte_conifers[index_zone] = seuil_fonte;
	}

	void DEGRE_JOUR_BANDE::ChangeSeuilFonteDecouver(size_t index_zone, double seuil_fonte)
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_decouver.size());
		_seuil_fonte_decouver[index_zone] = seuil_fonte;
	}

	void DEGRE_JOUR_BANDE::ChangeTauxFonteFeuillus(size_t index_zone, double taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte_feuillus.size() && taux_fonte >= 0.0);
		_taux_fonte_feuillus[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_BANDE::ChangeTauxFonteConifers(size_t index_zone, double taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte_conifers.size() && taux_fonte >= 0.0);
		_taux_fonte_conifers[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_BANDE::ChangeTauxFonteDecouver(size_t index_zone, double taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte_decouver.size() && taux_fonte >= 0.0);
		_taux_fonte_decouver[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_BANDE::ChangeTauxFonte(size_t index_zone, double taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte.size());
		_taux_fonte[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_BANDE::ChangeDesiteMaximale(size_t index_zone, double densite)
	{
		BOOST_ASSERT(index_zone < _densite_maximale.size());
		_densite_maximale[index_zone] = densite;
	}

	void DEGRE_JOUR_BANDE::ChangeConstanteTassement(size_t index_zone, double constante_tassement)
	{
		BOOST_ASSERT(index_zone < _constante_tassement.size());
		_constante_tassement[index_zone] = constante_tassement;
	}

	void DEGRE_JOUR_BANDE::ChangeIndexOccupationFeuillus(const std::vector<size_t>& index)
	{
		_index_occupation_feuillus = index;
		_index_occupation_feuillus.shrink_to_fit();
	}

	void DEGRE_JOUR_BANDE::ChangeIndexOccupationConifers(const std::vector<size_t>& index)
	{
		_index_occupation_conifers = index;
		_index_occupation_conifers.shrink_to_fit();
	}


	void DEGRE_JOUR_BANDE::CalculeFonte(ZONE& zone, size_t index_zone, unsigned short pas_de_temps, double temperature_moyenne, double dPrecipPluieMM, double dPrecipNeigeMM, double proportion_terrain, 
										double coeff_fonte, double temperaure_de_fonte, double& albedo, double& stock_neige, double& hauteur_neige, double& chaleur_stock, double& apport, double& eau_retenue)
	{
		double dennei, hneige, densto, rmax, beta2, eq_neige, st_neige, tneige, drel, alpha, erf, alb_t_plus_1, liquide;
		double pluie, neige, indice_radiation, pdts, fonte, dpas_de_temps, compaction;

		if(stock_neige > 0.0 && hauteur_neige == 0.0)
			hauteur_neige = 0.00000000001;

		pluie = dPrecipPluieMM / 1000.0;	//mm -> m
		neige = dPrecipNeigeMM / 1000.0;	//mm -> m

		if (stock_neige > 0.0 || neige > 0.0)
		{
			dpas_de_temps = static_cast<double>(pas_de_temps);
			pdts = dpas_de_temps * 60.0 * 60.0;

			indice_radiation = zone._dIndiceRadiation;

			tneige = temperature_moyenne;

			// calcul de la densite relative de la neige fraiche
			drel = CalculDensiteNeige(temperature_moyenne) / dDENSITE_EAU;

			// ajout de la nouvelle neige
			stock_neige+= neige * drel;
			hauteur_neige+= neige;
			chaleur_stock+= neige * drel * dDENSITE_EAU * dCHALEUR_NEIGE * temperature_moyenne;

			// calcul la densite de la neige stockee
			dennei = stock_neige / hauteur_neige;

			// pertes de chaleur par convection
			if (temperature_moyenne < temperaure_de_fonte)
			{
				tneige = chaleur_stock / (stock_neige * dCHALEUR_NEIGE * dDENSITE_EAU);

				hneige = (hauteur_neige < 0.4) ? (0.5 * hauteur_neige) : (0.2 + 0.25 * (hauteur_neige - 0.4));

				alpha = ConductiviteNeige(dennei * dDENSITE_EAU) / (dennei * dDENSITE_EAU * dCHALEUR_NEIGE);
				erf = Erf(hneige / (2.0 * sqrt(alpha * pdts)));

				tneige = temperature_moyenne + (tneige - temperature_moyenne) * erf;
				chaleur_stock = tneige * stock_neige * dDENSITE_EAU * dCHALEUR_NEIGE;
			}

			// ajustement du bilan calorifique en fonction de l'eau retenue au pas precedent
			chaleur_stock+= (eau_retenue * dDENSITE_EAU * dCHALEUR_FONTE);

			// ajout de la pluie
			stock_neige+= pluie;
			chaleur_stock+= pluie * dDENSITE_EAU * (dCHALEUR_FONTE + dCHALEUR_EAU * temperature_moyenne);

			// ajout de la chaleur due au gradient geothermique
			chaleur_stock+= (_taux_fonte[index_zone] * dpas_de_temps / 24.0) / 1000.0 * dDENSITE_EAU * dCHALEUR_FONTE;

			// calcul de l'albedo
			// les equations suivantes supposent:
			// albedo representatif sol et vegetation = 0.15
			// albedo max. de la neige = 0.8
			// albedo min de la neige = 0.5
			// valeur de beta1 = 0.5  
			// transparence de la neige
			// on utilise aussi le stock_neige du pas precedent

			if (_methode_albedo[index_zone] == 1)
			{
				eq_neige = neige * drel * 1000.0;
				st_neige = (stock_neige - neige * drel) * 1000.0;

				if (pluie > 0.0 || tneige >= 0.0)
					liquide = 1.0;
				else
					liquide = 0.0;

				if (st_neige > 0.0)  // s'il y a deja de la neige au sol
				{
					alb_t_plus_1 = (1.0 - exp(-0.5 * eq_neige)) * 0.8 + (1.0 - (1.0 - exp(-0.5 * eq_neige))) * (0.5 + (albedo - 0.5) * exp(-0.2 * dpas_de_temps / 24.0 * (1.0 + liquide)));

					// il y a probablement une erreur - albedo de la line precedente doit etre excluvement l'albedo de la partie specifique au stock de neige du pas precedent
					// il ne serait donc plus nécessaire de faire la conditionelle suivant mais l'Albedo stocke d'un pas de temps a l'autre devra differer

					if (albedo < 0.5)
						beta2 = 0.2;
					else
						beta2 = 0.2 + (albedo - 0.5);

					albedo = (1.0 - exp(-beta2 * st_neige)) * alb_t_plus_1 + (1.0 - (1.0 - exp(-beta2 * st_neige))) * 0.15;
				}
				else // s'il n'y a pas de neige au sol
				{
					albedo = (1.0 - exp(-0.5 * eq_neige)) * 0.8 + (1.0 - (1.0 - exp(-0.5 * eq_neige))) * 0.15;
				}
			}

			// ajout de la chaleur de fonte par rayonnement
			fonte = (temperature_moyenne > temperaure_de_fonte) ? (coeff_fonte * (temperature_moyenne - temperaure_de_fonte) * indice_radiation * (1.0-albedo)) : 0.0;
			fonte = fonte * (dpas_de_temps / 24.0);

			chaleur_stock+= (fonte * dDENSITE_EAU *	dCHALEUR_FONTE);

			// calcul de la hauteur du stock et de sa densite apres compaction et ajout de la pluie
			compaction = hauteur_neige * (_constante_tassement[index_zone] * (dpas_de_temps / 24.0)) * (1.0 - dennei / _densite_maximale[index_zone] * 1000.0);
			if (compaction < 0.0) 
				compaction = 0.0;

			hauteur_neige-= compaction;
			densto = stock_neige / hauteur_neige;

			// si la densite depasse le maximum permis
			if (densto * 1000.0 > _densite_maximale[index_zone])
			{
				densto = _densite_maximale[index_zone] / 1000.0;
				hauteur_neige = stock_neige / densto;
			}

			// calcul du surplus calorifique et de la fonte
			if (chaleur_stock > 0.0)
			{
				fonte = chaleur_stock / dCHALEUR_FONTE / dDENSITE_EAU;
				if (fonte > stock_neige) 
					fonte = stock_neige;

				stock_neige-= fonte;
				if ((fonte - pluie) > 0.0) 
					hauteur_neige-= ((fonte - pluie) / densto);

				if (hauteur_neige <= 0.0) 
					hauteur_neige = stock_neige / densto;

				chaleur_stock-= (fonte * dDENSITE_EAU * dCHALEUR_FONTE);
			}
			else
				fonte = 0.0;

			// reinitialisation des stocks si l'equivalent en eau est nul
			if (stock_neige < 0.0001) 
			{
				stock_neige = 0.0;
				hauteur_neige = 0.0;
				chaleur_stock = 0.0;
				eau_retenue = 0.0;
			}

			// calcul de l'eau retenue dans le stock_neige
			rmax = (0.1 * dennei) * stock_neige;
			if (rmax > fonte)
			{
				stock_neige+= fonte;
				eau_retenue = fonte;
				fonte = 0.0;
			}
			else
			{
				stock_neige+= rmax;
				eau_retenue = rmax;
				fonte-= rmax;
			}

			// proportion du type de couverture
			apport = proportion_terrain * fonte;
		}
		else
			apport = proportion_terrain * pluie;
	}


	void DEGRE_JOUR_BANDE::MiseAJour(const DATE_HEURE& date_courante, size_t index_zone, bool& bMajEffectuer)
	{
		STATION_NEIGE* station_neige;
		double stock_ref, hauteur_ref, stock, ponderation, hauteur;
		size_t idxBande, nbBande, n, index_station;

		int nb_pas_par_jour = 24 / _sim_hyd.PrendrePasDeTemps();

		bMajEffectuer = false;

		nbBande = _bandePourcentageM1[index_zone].size();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//M1
		for(idxBande=0; idxBande!=nbBande; idxBande++)
			_maj_conifers[index_zone][idxBande].nb_pas_derniere_correction++;

		for (index_station = 0; index_station < _stations_neige_conifers.PrendreNbStation(); ++index_station)
		{
			station_neige = static_cast<STATION_NEIGE*>(_stations_neige_conifers[index_station]);
			
			if((_interpolation_conifers == INTERPOLATION_THIESSEN && _sim_hyd._versionTHIESSEN == 1) || (_interpolation_conifers == INTERPOLATION_MOYENNE_3_STATIONS && _sim_hyd._versionMOY3STATION == 1))
				ponderation = static_cast<double>(_ponderation_conifersF(index_zone, index_station));
			else
				ponderation = _ponderation_conifers(index_zone, index_station);

			if (station_neige->DonneeDisponible(date_courante) && ponderation > 0.0)
			{
				auto donnees = station_neige->PrendreDonnee(date_courante);

				for(idxBande=0; idxBande!=nbBande; idxBande++)
				{
					if (_maj_conifers[index_zone][idxBande].stations_utilisees[index_station] || 
						_maj_conifers[index_zone][idxBande].pourcentage_corrige >= 1.0 ||
						_maj_conifers[index_zone][idxBande].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
					{
						_maj_conifers[index_zone][idxBande].nb_pas_derniere_correction = 0;
					
						_maj_conifers[index_zone][idxBande].pourcentage_corrige = 0.0;
						_maj_conifers[index_zone][idxBande].pourcentage_sim_eq = 1.0;
						_maj_conifers[index_zone][idxBande].pourcentage_sim_ha = 1.0;

						for (n = 0; n < _maj_conifers[index_zone][idxBande].stations_utilisees.size(); ++n)
							_maj_conifers[index_zone][idxBande].stations_utilisees[n] = false;
					}

					_maj_conifers[index_zone][idxBande].stations_utilisees[index_station] = true;
					_maj_conifers[index_zone][idxBande].pourcentage_corrige+= ponderation;
				}

				if (donnees.equi_eau > 0.0f)
				{
					bMajEffectuer = true;

					for(idxBande=0; idxBande!=nbBande; idxBande++)
					{
						stock_ref = _stock_conifers[index_zone][idxBande];
						hauteur_ref = _hauteur_conifers[index_zone][idxBande];

						stock = stock_ref;
					
						UneMiseAJourOccupation(
							static_cast<double>(donnees.equi_eau), 
							stock,
							stock_ref,
							_maj_conifers[index_zone][idxBande].pourcentage_corrige,
							ponderation,
							_maj_conifers[index_zone][idxBande].pourcentage_sim_eq);

						_stock_conifers[index_zone][idxBande] = stock;

						if (stock_ref == 0.0)
							_hauteur_conifers[index_zone][idxBande] = 0.0;
						else
						{
							_chaleur_conifers[index_zone][idxBande] = _chaleur_conifers[index_zone][idxBande] * _stock_conifers[index_zone][idxBande] / stock_ref;
							_eau_retenu_conifers[index_zone][idxBande] = _eau_retenu_conifers[index_zone][idxBande] * _stock_conifers[index_zone][idxBande] / stock_ref;
						}

						// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
						if (donnees.hauteur <= 0.0f)
						{
							if (stock_ref == 0.0)
								donnees.hauteur = donnees.equi_eau * 2.5f;
							else
								donnees.hauteur = donnees.equi_eau * static_cast<float>(hauteur_ref) / static_cast<float>(stock_ref);
						}

						if (_hauteur_conifers[index_zone][idxBande] > 0.0)
						{
							hauteur = _hauteur_conifers[index_zone][idxBande];

							UneMiseAJourOccupation(static_cast<double>(donnees.hauteur),
								hauteur,
								hauteur_ref,
								_maj_conifers[index_zone][idxBande].pourcentage_corrige,
								ponderation,
								_maj_conifers[index_zone][idxBande].pourcentage_sim_ha);

							_hauteur_conifers[index_zone][idxBande] = hauteur;
						}
						else
						{
							if (_stock_conifers[index_zone][idxBande] > 0.0)
							{
								if (stock_ref == 0.0)
									_hauteur_conifers[index_zone][idxBande] = _stock_conifers[index_zone][idxBande] * 2.5;
								else
									_hauteur_conifers[index_zone][idxBande] = hauteur_ref * _stock_conifers[index_zone][idxBande] / stock_ref;
							}
						}
					}
				}
			}
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//M2
		for(idxBande=0; idxBande!=nbBande; idxBande++)
			_maj_feuillus[index_zone][idxBande].nb_pas_derniere_correction++;

		for (index_station = 0; index_station < _stations_neige_feuillus.PrendreNbStation(); ++index_station)
		{
			station_neige = static_cast<STATION_NEIGE*>(_stations_neige_feuillus[index_station]);
			
			if((_interpolation_feuillus == INTERPOLATION_THIESSEN && _sim_hyd._versionTHIESSEN == 1)  || (_interpolation_feuillus == INTERPOLATION_MOYENNE_3_STATIONS && _sim_hyd._versionMOY3STATION == 1))
				ponderation = static_cast<double>(_ponderation_feuillusF(index_zone, index_station));
			else
				ponderation = _ponderation_feuillus(index_zone, index_station);

			if (station_neige->DonneeDisponible(date_courante) && ponderation > 0.0)
			{				
				auto donnees = station_neige->PrendreDonnee(date_courante);

				for(idxBande=0; idxBande!=nbBande; idxBande++)
				{
					if (_maj_feuillus[index_zone][idxBande].stations_utilisees[index_station] || 
						_maj_feuillus[index_zone][idxBande].pourcentage_corrige >= 1.0 ||
						_maj_feuillus[index_zone][idxBande].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
					{
						_maj_feuillus[index_zone][idxBande].nb_pas_derniere_correction = 0;
					
						_maj_feuillus[index_zone][idxBande].pourcentage_corrige = 0.0;
						_maj_feuillus[index_zone][idxBande].pourcentage_sim_eq = 1.0;
						_maj_feuillus[index_zone][idxBande].pourcentage_sim_ha = 1.0;

						for (n = 0; n < _maj_feuillus[index_zone][idxBande].stations_utilisees.size(); ++n)
							_maj_feuillus[index_zone][idxBande].stations_utilisees[n] = false;
					}

					_maj_feuillus[index_zone][idxBande].stations_utilisees[index_station] = true;
					_maj_feuillus[index_zone][idxBande].pourcentage_corrige += ponderation;
				}

				if (donnees.equi_eau > 0.0f)
				{
					bMajEffectuer = true;

					for(idxBande=0; idxBande!=nbBande; idxBande++)
					{
						stock_ref = _stock_feuillus[index_zone][idxBande];
						hauteur_ref = _hauteur_feuillus[index_zone][idxBande];

						stock = stock_ref;
					
						UneMiseAJourOccupation(
							static_cast<double>(donnees.equi_eau), 
							stock,
							stock_ref,
							_maj_feuillus[index_zone][idxBande].pourcentage_corrige,
							ponderation,
							_maj_feuillus[index_zone][idxBande].pourcentage_sim_eq);

						_stock_feuillus[index_zone][idxBande] = stock;

						if (stock_ref == 0.0)
							_hauteur_feuillus[index_zone][idxBande] = 0.0;
						else
						{
							_chaleur_feuillus[index_zone][idxBande] = _chaleur_feuillus[index_zone][idxBande] * _stock_feuillus[index_zone][idxBande] / stock_ref;
							_eau_retenu_feuillus[index_zone][idxBande] = _eau_retenu_feuillus[index_zone][idxBande] * _stock_feuillus[index_zone][idxBande] / stock_ref;
						}

						// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
						if (donnees.hauteur <= 0.0f)
						{
							if (stock_ref == 0.0)
								donnees.hauteur = donnees.equi_eau * 2.5f;
							else
								donnees.hauteur = donnees.equi_eau * static_cast<float>(hauteur_ref) / static_cast<float>(stock_ref);
						}

						if (_hauteur_feuillus[index_zone][idxBande] > 0.0)
						{
							hauteur = _hauteur_feuillus[index_zone][idxBande];

							UneMiseAJourOccupation(static_cast<double>(donnees.hauteur),
								hauteur,
								hauteur_ref,
								_maj_feuillus[index_zone][idxBande].pourcentage_corrige,
								ponderation,
								_maj_feuillus[index_zone][idxBande].pourcentage_sim_ha);

							_hauteur_feuillus[index_zone][idxBande] = hauteur;
						}
						else
						{
							if (_stock_feuillus[index_zone][idxBande] > 0.0)
							{
								if (stock_ref == 0.0)
									_hauteur_feuillus[index_zone][idxBande] = _stock_feuillus[index_zone][idxBande] * 2.5;
								else
									_hauteur_feuillus[index_zone][idxBande] = hauteur_ref * _stock_feuillus[index_zone][idxBande] / stock_ref;
							}
						}
					}
				}
			}
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//M3
		for(idxBande=0; idxBande!=nbBande; idxBande++)
			_maj_decouver[index_zone][idxBande].nb_pas_derniere_correction++;

		for (index_station = 0; index_station < _stations_neige_decouver.PrendreNbStation(); ++index_station)
		{
			station_neige = static_cast<STATION_NEIGE*>(_stations_neige_decouver[index_station]);
			
			if((_interpolation_decouver == INTERPOLATION_THIESSEN && _sim_hyd._versionTHIESSEN == 1) || (_interpolation_decouver == INTERPOLATION_MOYENNE_3_STATIONS && _sim_hyd._versionMOY3STATION == 1))
				ponderation = static_cast<double>(_ponderation_decouverF(index_zone, index_station));
			else
				ponderation = _ponderation_decouver(index_zone, index_station);

			if (station_neige->DonneeDisponible(date_courante) && ponderation > 0.0)
			{				
				auto donnees = station_neige->PrendreDonnee(date_courante);

				for(idxBande=0; idxBande!=nbBande; idxBande++)
				{
					if (_maj_decouver[index_zone][idxBande].stations_utilisees[index_station] || 
						_maj_decouver[index_zone][idxBande].pourcentage_corrige >= 1.0 ||
						_maj_decouver[index_zone][idxBande].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
					{
						_maj_decouver[index_zone][idxBande].nb_pas_derniere_correction = 0;
					
						_maj_decouver[index_zone][idxBande].pourcentage_corrige = 0.0;
						_maj_decouver[index_zone][idxBande].pourcentage_sim_eq = 1.0;
						_maj_decouver[index_zone][idxBande].pourcentage_sim_ha = 1.0;

						for (n = 0; n < _maj_decouver[index_zone][idxBande].stations_utilisees.size(); ++n)
							_maj_decouver[index_zone][idxBande].stations_utilisees[n] = false;
					}

					_maj_decouver[index_zone][idxBande].stations_utilisees[index_station] = true;
					_maj_decouver[index_zone][idxBande].pourcentage_corrige += ponderation;
				}

				if (donnees.equi_eau > 0.0f)
				{
					bMajEffectuer = true;

					for(idxBande=0; idxBande!=nbBande; idxBande++)
					{
						stock_ref = _stock_decouver[index_zone][idxBande];
						hauteur_ref = _hauteur_decouver[index_zone][idxBande];

						stock = stock_ref;
					
						UneMiseAJourOccupation(
							static_cast<double>(donnees.equi_eau), 
							stock,
							stock_ref,
							_maj_decouver[index_zone][idxBande].pourcentage_corrige,
							ponderation,
							_maj_decouver[index_zone][idxBande].pourcentage_sim_eq);

						_stock_decouver[index_zone][idxBande] = stock;

						if (stock_ref == 0.0)
							_hauteur_decouver[index_zone][idxBande] = 0.0;
						else
						{
							_chaleur_decouver[index_zone][idxBande] = _chaleur_decouver[index_zone][idxBande] * _stock_decouver[index_zone][idxBande] / stock_ref;
							_eau_retenu_decouver[index_zone][idxBande] = _eau_retenu_decouver[index_zone][idxBande] * _stock_decouver[index_zone][idxBande] / stock_ref;
						}

						// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
						if (donnees.hauteur <= 0.0f)
						{
							if (stock_ref == 0.0)
								donnees.hauteur = donnees.equi_eau * 2.5f;
							else
								donnees.hauteur = donnees.equi_eau * static_cast<float>(hauteur_ref) / static_cast<float>(stock_ref);
						}

						if (_hauteur_decouver[index_zone][idxBande] > 0.0)
						{
							hauteur = _hauteur_decouver[index_zone][idxBande];

							UneMiseAJourOccupation(static_cast<double>(donnees.hauteur),
								hauteur,
								hauteur_ref,
								_maj_decouver[index_zone][idxBande].pourcentage_corrige,
								ponderation,
								_maj_decouver[index_zone][idxBande].pourcentage_sim_ha);

							_hauteur_decouver[index_zone][idxBande] = hauteur;
						}
						else
						{
							if (_stock_decouver[index_zone][idxBande] > 0.0)
							{
								if (stock_ref == 0.0)
									_hauteur_decouver[index_zone][idxBande] = _stock_decouver[index_zone][idxBande] * 2.5;
								else
									_hauteur_decouver[index_zone][idxBande] = hauteur_ref * _stock_decouver[index_zone][idxBande] / stock_ref;
							}
						}
					}
				}
			}
		}
	}


	////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//void DEGRE_JOUR_BANDE::MiseAJourGrille(size_t index_zone, STATION_NEIGE::typeOccupationStation occupation)
	//{
	//	size_t row, col, nbCol;
	//	float stock_ref, stock_avant, hauteur_ref, stock;
	//	float hauteur, hauteur2;	// m
	//	float equi_eau;				// m
	//	//float densite;				// pourcentage
	//	int iIdent, milieuRef, indexcell;
	//	
	//	int nb_pas_par_jour = 24 / _sim_hyd.PrendrePasDeTemps();
	//	
	//	// conversion d'occupation de sol dominante dans la zone
	//	stock_ref = 0.0f;
	//	if (occupation==STATION_NEIGE::RESINEUX)
	//	{
	//		stock_ref=_stock_conifers[index_zone];
	//		hauteur_ref= _hauteur_conifers[index_zone];
	//		milieuRef = 0;
	//	}
	//	if (occupation==STATION_NEIGE::DECOUVERTE)
	//	{
	//		stock_ref=_stock_decouver[index_zone];
	//		hauteur_ref= _hauteur_decouver[index_zone];
	//		milieuRef = 2;
	//	}
	//	if (occupation==STATION_NEIGE::FEUILLUS)
	//	{
	//		stock_ref=_stock_feuillus[index_zone];
	//		hauteur_ref= _hauteur_feuillus[index_zone];
	//		milieuRef = 1;
	//	}

	//	//
	//	ZONE& zone = _sim_hyd.PrendreZones()[index_zone];
	//	iIdent = zone.PrendreIdent();

	//	nbCol = _grilleneige._grilleEquivalentEau[0].PrendreNbColonne();
	//	
	//	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//	_maj_conifers[index_zone].nb_pas_derniere_correction++;

	//	for (auto iter2 = begin(_grilleneige._mapPonderation[iIdent]); iter2 != end(_grilleneige._mapPonderation[iIdent]); iter2++)
	//	{
	//		indexcell = iter2->first;
	//		row = static_cast<size_t>(indexcell / nbCol);
	//		col = indexcell % nbCol;

	//		equi_eau = _grilleneige._grilleEquivalentEau[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;	//conversion -> [m]
	//		hauteur = _grilleneige._grilleHauteurNeige[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;	//conversion -> [m]

	//		if(equi_eau > VALEUR_MANQUANTE && hauteur > VALEUR_MANQUANTE && hauteur != 0.0f)
	//		{
	//			_maj_conifers[index_zone].pourcentage_corrige+= static_cast<float>(iter2->second);	//iter2->second -> ponderation

	//			if (_maj_conifers[index_zone].pourcentage_corrige >= 1.0f ||
	//				_maj_conifers[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
	//			{
	//				_maj_conifers[index_zone].nb_pas_derniere_correction = 0;
	//				
	//				_maj_conifers[index_zone].pourcentage_corrige = 0.0f;
	//				_maj_conifers[index_zone].pourcentage_sim_eq = 1.0f;
	//				_maj_conifers[index_zone].pourcentage_sim_ha = 1.0f;
	//			}

	//			if(equi_eau > 0.0f)
	//			{
	//				stock = stock_avant = _stock_conifers[index_zone];
	//				hauteur_ref = _hauteur_conifers[index_zone];
	//				
	//				UneMiseAJourOccupation(
	//					equi_eau, 
	//					stock,
	//					stock_ref,
	//					_maj_conifers[index_zone].pourcentage_corrige,
	//					static_cast<float>(iter2->second),	//iter2->second -> ponderation
	//					_maj_conifers[index_zone].pourcentage_sim_eq);

	//				_stock_conifers[index_zone] = stock;

	//				if (stock_avant == 0.0f)
	//					_hauteur_conifers[index_zone] = 0.0f;
	//				else
	//				{
	//					_chaleur_conifers[index_zone] = _chaleur_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
	//					_eau_retenu_conifers[index_zone] = _eau_retenu_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
	//				}

	//				// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
	//				if (hauteur <= 0.0f)
	//				{
	//					if (stock_avant == 0.0f)
	//						hauteur = equi_eau * 2.5f;
	//					else
	//						hauteur = equi_eau * hauteur_ref / stock_avant;
	//				}

	//				if (_hauteur_conifers[index_zone] > 0.0f)
	//				{
	//					hauteur2 = _hauteur_conifers[index_zone];

	//					UneMiseAJourOccupation(hauteur,
	//						hauteur2,
	//						hauteur_ref,
	//						_maj_conifers[index_zone].pourcentage_corrige,
	//						static_cast<float>(iter2->second),	//iter2->second -> ponderation
	//						_maj_conifers[index_zone].pourcentage_sim_ha);

	//					_hauteur_conifers[index_zone] = hauteur2;
	//				}
	//				else
	//				{
	//					if (_stock_conifers[index_zone] > 0.0f)
	//					{
	//						if (stock_avant == 0.0f)
	//							_hauteur_conifers[index_zone] = _stock_conifers[index_zone] * 2.5f;
	//						else
	//							_hauteur_conifers[index_zone] = hauteur_ref * _stock_conifers[index_zone] / stock_avant;
	//					}
	//				}
	//			}
	//		}
	//	}

	//	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//	_maj_feuillus[index_zone].nb_pas_derniere_correction++;

	//	for (auto iter2 = begin(_grilleneige._mapPonderation[iIdent]); iter2 != end(_grilleneige._mapPonderation[iIdent]); iter2++)
	//	{
	//		indexcell = iter2->first;
	//		row = static_cast<size_t>(indexcell / nbCol);
	//		col = indexcell % nbCol;

	//		equi_eau = _grilleneige._grilleEquivalentEau[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;
	//		hauteur = _grilleneige._grilleHauteurNeige[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;

	//		if(equi_eau > VALEUR_MANQUANTE && hauteur > VALEUR_MANQUANTE && hauteur != 0.0f)
	//		{
	//			_maj_feuillus[index_zone].pourcentage_corrige+= static_cast<float>(iter2->second);	//iter2->second -> ponderation

	//			if (_maj_feuillus[index_zone].pourcentage_corrige >= 1.0f ||
	//				_maj_feuillus[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
	//			{
	//				_maj_feuillus[index_zone].nb_pas_derniere_correction = 0;
	//				
	//				_maj_feuillus[index_zone].pourcentage_corrige = 0.0f;
	//				_maj_feuillus[index_zone].pourcentage_sim_eq = 1.0f;
	//				_maj_feuillus[index_zone].pourcentage_sim_ha = 1.0f;
	//			}

	//			if(equi_eau > 0.0f)
	//			{
	//				stock = stock_avant = _stock_feuillus[index_zone];
	//				hauteur_ref = _hauteur_feuillus[index_zone];
	//				
	//				UneMiseAJourOccupation(
	//					equi_eau, 
	//					stock,
	//					stock_ref,
	//					_maj_feuillus[index_zone].pourcentage_corrige,
	//					static_cast<float>(iter2->second),	//iter2->second -> ponderation
	//					_maj_feuillus[index_zone].pourcentage_sim_eq);

	//				_stock_feuillus[index_zone] = stock;

	//				if (stock_avant == 0.0f)
	//				{
	//					_hauteur_feuillus[index_zone] = 0.0f;
	//				}
	//				else
	//				{
	//					_chaleur_feuillus[index_zone] = _chaleur_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
	//					_eau_retenu_feuillus[index_zone] = _eau_retenu_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
	//				}

	//				// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
	//				if (hauteur <= 0.0f)
	//				{
	//					if (stock_avant == 0.0f)
	//						hauteur = equi_eau * 2.5f;
	//					else
	//						hauteur = equi_eau * hauteur_ref / stock_avant;
	//				}

	//				if (_hauteur_feuillus[index_zone] > 0.0f)
	//				{
	//					hauteur2 = _hauteur_feuillus[index_zone];

	//					UneMiseAJourOccupation(hauteur,
	//						hauteur2,
	//						hauteur_ref,
	//						_maj_feuillus[index_zone].pourcentage_corrige,
	//						static_cast<float>(iter2->second),	//iter2->second -> ponderation
	//						_maj_feuillus[index_zone].pourcentage_sim_ha);

	//					_hauteur_feuillus[index_zone] = hauteur2;
	//				}
	//				else
	//				{
	//					if (_stock_feuillus[index_zone] > 0.0f)
	//					{
	//						if (stock_avant == 0.0f)
	//							_hauteur_feuillus[index_zone] = _stock_feuillus[index_zone] * 2.5f;
	//						else
	//							_hauteur_feuillus[index_zone] = hauteur_ref * _stock_feuillus[index_zone] / stock_avant;
	//					}
	//				}
	//			}
	//		}
	//	}

	//	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//	_maj_decouver[index_zone].nb_pas_derniere_correction++;

	//	for (auto iter2 = begin(_grilleneige._mapPonderation[iIdent]); iter2 != end(_grilleneige._mapPonderation[iIdent]); iter2++)
	//	{
	//		indexcell = iter2->first;
	//		row = static_cast<size_t>(indexcell / nbCol);
	//		col = indexcell % nbCol;

	//		equi_eau = _grilleneige._grilleEquivalentEau[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;
	//		hauteur = _grilleneige._grilleHauteurNeige[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;

	//		if(equi_eau > VALEUR_MANQUANTE && hauteur > VALEUR_MANQUANTE && hauteur != 0.0f)
	//		{
	//			_maj_decouver[index_zone].pourcentage_corrige+= static_cast<float>(iter2->second);	//iter2->second -> ponderation

	//			if (_maj_decouver[index_zone].pourcentage_corrige >= 1.0f ||
	//				_maj_decouver[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
	//			{
	//				_maj_decouver[index_zone].nb_pas_derniere_correction = 0;
	//				
	//				_maj_decouver[index_zone].pourcentage_corrige = 0.0f;
	//				_maj_decouver[index_zone].pourcentage_sim_eq = 1.0f;
	//				_maj_decouver[index_zone].pourcentage_sim_ha = 1.0f;
	//			}

	//			if(equi_eau > 0.0f)
	//			{
	//				stock = stock_avant = _stock_decouver[index_zone];
	//				hauteur_ref = _hauteur_decouver[index_zone];
	//				
	//				UneMiseAJourOccupation(
	//					equi_eau, 
	//					stock,
	//					stock_ref,
	//					_maj_decouver[index_zone].pourcentage_corrige,
	//					static_cast<float>(iter2->second),	//iter2->second -> ponderation
	//					_maj_decouver[index_zone].pourcentage_sim_eq);

	//				_stock_decouver[index_zone] = stock;

	//				if (stock_avant == 0.0f)
	//					_hauteur_decouver[index_zone] = 0.0f;
	//				else
	//				{
	//					_chaleur_decouver[index_zone] = _chaleur_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
	//					_eau_retenu_decouver[index_zone] = _eau_retenu_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
	//				}

	//				// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
	//				if (hauteur <= 0.0f)
	//				{
	//					if (stock_avant == 0.0f)
	//						hauteur = equi_eau * 2.5f;
	//					else
	//						hauteur = equi_eau * hauteur_ref / stock_avant;
	//				}

	//				if (_hauteur_decouver[index_zone] > 0.0f)
	//				{
	//					hauteur2 = _hauteur_decouver[index_zone];

	//					UneMiseAJourOccupation(hauteur,
	//						hauteur2,
	//						hauteur_ref,
	//						_maj_decouver[index_zone].pourcentage_corrige,
	//						static_cast<float>(iter2->second),	//iter2->second -> ponderation
	//						_maj_decouver[index_zone].pourcentage_sim_ha);

	//					_hauteur_decouver[index_zone] = hauteur2;
	//				}
	//				else
	//				{
	//					if (_stock_decouver[index_zone] > 0.0f)
	//					{
	//						if (stock_avant == 0.0f)
	//							_hauteur_decouver[index_zone] = _stock_decouver[index_zone] * 2.5f;
	//						else
	//							_hauteur_decouver[index_zone] = hauteur_ref * _stock_decouver[index_zone] / stock_avant;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	void DEGRE_JOUR_BANDE::UneMiseAJourOccupation(double mesure, double& valeur_actuelle, double valeur_ref, double fraction_corrigee, double ponderation, double &fraction_simulee)
	{
		double frac = min(fraction_corrigee, 1.0);
		
		// ajustement de la mesure en fonction de l'occupation du sol
		double mesure_ajustee = max(mesure - (valeur_ref - valeur_actuelle), 0.0);

		// contibution relative des observations et des simulations
		double contr_obs = (1.0 - fraction_simulee) * valeur_actuelle + ponderation * mesure_ajustee;
		double contr_sim = (fraction_simulee * valeur_actuelle * (1.0 - frac)) / ((1.0 - frac) + ponderation);

		// mise a jour 
		valeur_actuelle = contr_obs + contr_sim;

		//calcul de la fraction simulee
		if (valeur_actuelle != 0.0)
			fraction_simulee = contr_sim / valeur_actuelle;
		else
			fraction_simulee = 1.0;
	}


	double DEGRE_JOUR_BANDE::ConductiviteNeige(double densite)
	{
		double d0 = 0.36969;
		double d1 = 0.00158688;
		double d2 = 0.00000302462;
		double d3 = 0.00000000519756;
		double d4 = 0.0000000000156984;

		double p0 = 1.0;

		double p1 = densite - 329.6;
		double p2 = ((densite - 260.378) * p1) - (21166.4 * p0);
		double p3 = ((densite - 320.69) * p2) - (24555.8 * p1);
		double p4 = ((densite - 263.363) * p3) - (11739.3 * p2);

		return d0 * p0 + d1 * p1 + d2 * p2 + d3 * p3 + d4 * p4;
	}


	double DEGRE_JOUR_BANDE::Erf(double x)
	{
		BOOST_ASSERT(x >= 0.0);

		//approximation rationnelle
		double t = 1.0 / (1.0 + 0.47047 * x);
		return 1.0 - (0.3480242 * t - 0.0958798 * t * t + 0.7478556 * t * t * t) * exp(-x * x);
	}


	double DEGRE_JOUR_BANDE::PrendreTauxFonte(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte.size());
		return _taux_fonte[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreDensiteMaximal(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _densite_maximale.size());
		return _densite_maximale[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreConstanteTassement(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _constante_tassement.size());
		return _constante_tassement[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreSeuilFonteFeuillus(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_feuillus.size());
		return _seuil_fonte_feuillus[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreSeuilFonteConifers(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_conifers.size());
		return _seuil_fonte_conifers[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreSeuilFonteDecouver(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_decouver.size());
		return _seuil_fonte_decouver[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreTauxFonteFeuillus(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte_feuillus.size());
		return _taux_fonte_feuillus[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreTauxFonteConifers(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte_conifers.size());
		return _taux_fonte_conifers[index_zone];
	}

	double DEGRE_JOUR_BANDE::PrendreTauxFonteDecouver(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte_decouver.size());
		return _taux_fonte_decouver[index_zone];
	}

	vector<size_t> DEGRE_JOUR_BANDE::PrendreIndexOccupationFeuillus() const
	{
		return _index_occupation_feuillus;
	}

	vector<size_t> DEGRE_JOUR_BANDE::PrendreIndexOccupationConifers() const
	{
		return _index_occupation_conifers;
	}


	void DEGRE_JOUR_BANDE::LectureEtat(DATE_HEURE date_courante)
	{
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; fichier etat DEGRE_JOUR_BANDE; " + _nom_fichier_lecture_etat);

		vector<string> svaleurs;
		vector<float> valeurs;
		vector<int> vValidation;
		string ligne, str;
		size_t index_zone, idxBande;
		int iIdent, iIdentOld;

		iIdentOld = -999999;

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);

		while(!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne != "")
			{
				svaleurs = extrait_stringValeur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);
				str = svaleurs[0];	//IDENT-BANDE

				valeurs = extrait_fvaleur(str, "-");
				if(valeurs.size() != 2)
				{
					fichier.close();
					throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; fichier etat DEGRE_JOUR_BANDE; " + _nom_fichier_lecture_etat);
				}

				iIdent = static_cast<int>(valeurs[0]);
				idxBande = static_cast<size_t>(valeurs[1]);
				--idxBande;	//one-based -> zero-based index

				valeurs = extrait_fvaleur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					try{

					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

					_stock_conifers[index_zone][idxBande] = valeurs[1];
					_stock_feuillus[index_zone][idxBande] = valeurs[2];
					_stock_decouver[index_zone][idxBande] = valeurs[3];

					_hauteur_conifers[index_zone][idxBande] = valeurs[4];
					_hauteur_feuillus[index_zone][idxBande] = valeurs[5];
					_hauteur_decouver[index_zone][idxBande] = valeurs[6];

					_chaleur_conifers[index_zone][idxBande] = valeurs[7];
					_chaleur_feuillus[index_zone][idxBande] = valeurs[8];
					_chaleur_decouver[index_zone][idxBande] = valeurs[9];

					_eau_retenu_conifers[index_zone][idxBande] = valeurs[10];
					_eau_retenu_feuillus[index_zone][idxBande] = valeurs[11];
					_eau_retenu_decouver[index_zone][idxBande] = valeurs[12];

					_albedo_conifers[index_zone][idxBande] = valeurs[13];
					_albedo_feuillus[index_zone][idxBande] = valeurs[14];
					_albedo_decouver[index_zone][idxBande] = valeurs[15];
					
					}
					catch(...)
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; fichier etat DEGRE_JOUR_BANDE; " + _nom_fichier_lecture_etat);
					}

					if(iIdentOld == -999999 || iIdent != iIdentOld)
					{
						vValidation.push_back(iIdent);
						iIdentOld = iIdent;
					}
				}
			}
		}

		fichier.close();

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; fichier etat DEGRE_JOUR_BANDE; id mismatch; " + _nom_fichier_lecture_etat);
	}


	void DEGRE_JOUR_BANDE::SauvegardeEtat(DATE_HEURE date_courante) const
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier, oss;
		string sSep = _sim_hyd._output._sFichiersEtatsSeparator;
		size_t x, nbSimuler, index_zone, nbBande, idxBande;

		if(!RepertoireExiste(_repertoire_ecriture_etat))
			CreeRepertoire(_repertoire_ecriture_etat);

		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			        << "fonte_neige_" 
			        << setw(4) << date_courante.PrendreAnnee() 
			        << setw(2) << date_courante.PrendreMois() 
			        << setw(2) << date_courante.PrendreJour() 
			        << setw(2) << date_courante.PrendreHeure() 
					<< ".csv";

		ofstream fichier(nom_fichier.str());
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier.exceptions(ios::failbit | ios::badbit);

		fichier << "ETATS FONTE NEIGE" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier << "DATE_HEURE" << sSep << date_courante << endl;
		fichier << endl;

		fichier << "UHRH-BANDE" << sSep 
				<< "STOCK CONIFERS" << sSep << "STOCK FEUILLUS" << sSep << "STOCK DECOUVERT" << sSep 
				<< "HAUTEUR CONIFERS" << sSep << "HAUTEUR FEUILLUS" << sSep << "HAUTEUR DECOUVERT" <<sSep
				<< "CHALEUR CONIFERS" << sSep << "CHALEUR FEUILLUS" << sSep << "CHALEUR DECOUVERT" <<sSep
				<< "EAU RETENUE CONIFERS" << sSep << "EAU RETENUE FEUILLUS" << sSep << "EAU RETENUE DECOUVERT" <<sSep
				<< "ALBEDO CONIFERS" << sSep << "ALBEDO FEUILLUS" << sSep << "ALBEDO DECOUVERT" << endl;

		ZONES& zones = _sim_hyd.PrendreZones();

		nbSimuler = _sim_hyd.PrendreZonesSimules().size();
		x = 0;

		for (index_zone=0; x<nbSimuler; index_zone++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				nbBande = _bandePourcentageM1[index_zone].size();
				for(idxBande=0; idxBande!=nbBande; idxBande++)
				{
					oss.str("");
					oss << zone.PrendreIdent() << "-" << idxBande+1 << sSep;

					oss << setprecision(21) << setiosflags(ios::fixed);
			
					oss << _stock_conifers[index_zone][idxBande] << sSep;
					oss << _stock_feuillus[index_zone][idxBande] << sSep;
					oss << _stock_decouver[index_zone][idxBande] << sSep;

					oss << _hauteur_conifers[index_zone][idxBande] << sSep;
					oss << _hauteur_feuillus[index_zone][idxBande] << sSep;
					oss << _hauteur_decouver[index_zone][idxBande] << sSep;

					oss << _chaleur_conifers[index_zone][idxBande] << sSep;
					oss << _chaleur_feuillus[index_zone][idxBande] << sSep;
					oss << _chaleur_decouver[index_zone][idxBande] << sSep;

					oss << _eau_retenu_conifers[index_zone][idxBande] << sSep;
					oss << _eau_retenu_feuillus[index_zone][idxBande] << sSep;
					oss << _eau_retenu_decouver[index_zone][idxBande] << sSep;

					oss << _albedo_conifers[index_zone][idxBande] << sSep;
					oss << _albedo_feuillus[index_zone][idxBande] << sSep;
					oss << _albedo_decouver[index_zone][idxBande];

					fichier << oss.str() << endl;
				}
				++x;
			}
		}

		fichier.close();
	}


	void DEGRE_JOUR_BANDE::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_BANDE");

		try
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			istringstream iss;
			string cle, valeur, ligne;
			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_BANDE", 1);

			getline_mod(fichier, ligne);
			lire_cle_valeur(fichier, cle, valeur);
			getline_mod(fichier, ligne);

			// lecture des classes integrees

			lire_cle_valeur(fichier, cle, valeur);
			ChangeIndexOccupationConifers( extrait_valeur(valeur) );	//M1

			lire_cle_valeur(fichier, cle, valeur);
			ChangeIndexOccupationFeuillus( extrait_valeur(valeur) );	//M2

			getline_mod(fichier, ligne);

			// stations neige
			string repertoire = _sim_hyd.PrendreRepertoireProjet();
			string nom_fichier;

			lire_cle_valeur(fichier, cle, nom_fichier);
			if (!nom_fichier.empty() && !Racine(nom_fichier))
				nom_fichier = Combine(repertoire, nom_fichier);
			_stations_neige_conifers.ChangeNomFichier(nom_fichier);

			lire_cle_valeur(fichier, cle, nom_fichier);
			if (!nom_fichier.empty() && !Racine(nom_fichier))
				nom_fichier = Combine(repertoire, nom_fichier);
			_stations_neige_feuillus.ChangeNomFichier(nom_fichier);

			lire_cle_valeur(fichier, cle, nom_fichier);
			if (!nom_fichier.empty() && !Racine(nom_fichier))
				nom_fichier = Combine(repertoire, nom_fichier);
			_stations_neige_decouver.ChangeNomFichier(nom_fichier);

			getline_mod(fichier, ligne);

			// modele interpolation stations neige
			lire_cle_valeur(fichier, cle, ligne);
			if (ligne.empty())
				_interpolation_conifers = INTERPOLATION_AUCUNE;
			else if (ligne == "THIESSEN")
				_interpolation_conifers = INTERPOLATION_THIESSEN;
			else if (ligne == "MOYENNE 3 STATIONS")
				_interpolation_conifers = INTERPOLATION_MOYENNE_3_STATIONS;
			else
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_BANDE");

			lire_cle_valeur(fichier, cle, ligne);
			if (ligne.empty())
				_interpolation_feuillus = INTERPOLATION_AUCUNE;
			else if (ligne == "THIESSEN")
				_interpolation_feuillus = INTERPOLATION_THIESSEN;
			else if (ligne == "MOYENNE 3 STATIONS")
				_interpolation_feuillus = INTERPOLATION_MOYENNE_3_STATIONS;
			else
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_BANDE");

			lire_cle_valeur(fichier, cle, ligne);
			if (ligne.empty())
				_interpolation_decouver = INTERPOLATION_AUCUNE;
			else if (ligne == "THIESSEN")
				_interpolation_decouver = INTERPOLATION_THIESSEN;
			else if (ligne == "MOYENNE 3 STATIONS")
				_interpolation_decouver = INTERPOLATION_MOYENNE_3_STATIONS;
			else
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_BANDE");

			getline_mod(fichier, ligne); //vide

			//hauteur bande
			getline_mod(fichier, ligne);
			ligne = ligne.substr(17, string::npos);	//HAUTEUR BANDE(m);
			iss.clear();
			iss.str(ligne);
			iss >> _dHauteurBande;

			getline_mod(fichier, ligne); //vide

			//
			//_bMAJGrilleNeige = false;	//valeur par defaut

			lire_cle_valeur(fichier, cle, ligne);
			//if(cle == "MISE A JOUR GRILLE NEIGE")
			//{
			//	if(ligne == "1")
			//		_bMAJGrilleNeige = true;
			//	
			//	//
			//	lire_cle_valeur(fichier, cle, ligne);
			//	if(cle == "NOM FICHIER GRILLE NEIGE")
			//	{
			//		if (!ligne.empty() && !Racine(ligne))
			//			ligne = Combine(_sim_hyd.PrendreRepertoireProjet(), ligne);
			//		_grilleneige._sPathFichierParam = ligne;

			//		getline_mod(fichier, ligne); //vide
			//		getline_mod(fichier, ligne); // commentaire
			//	}
			//	else
			//		throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_BANDE");
			//}
			//else
			//{			
			//	if(cle == "NOM FICHIER GRILLE NEIGE")
			//	{
			//		if (!ligne.empty() && !Racine(ligne))
			//		{
			//			ligne = Combine(_sim_hyd.PrendreRepertoireProjet(), ligne);
			//			_bMAJGrilleNeige = true;
			//		}
			//		_grilleneige._sPathFichierParam = ligne;

			//		getline_mod(fichier, ligne); //vide
			//		getline_mod(fichier, ligne); // commentaire
			//	}
			//}

			// lecture des parametres
			for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
			{
				int ident;
				double taux_fonte, densite_max, tassement, 
					seuil_conif, seuil_feuillu, seuil_decou,
					taux_conif, taux_feuillu, taux_decou,
					seuil_albedo;
				char c;

				fichier >> ident >> c;
				fichier >> taux_fonte >> c;
				fichier >> densite_max >> c;
				fichier >> tassement >> c;
				fichier >> seuil_conif >> c;
				fichier >> seuil_feuillu >> c;
				fichier >> seuil_decou >> c;
				fichier >> taux_conif >> c;
				fichier >> taux_feuillu >> c;
				fichier >> taux_decou >> c;
				fichier >> seuil_albedo;

				size_t index_zone = zones.IdentVersIndex(ident);

				ChangeTauxFonte(index_zone, taux_fonte);
				ChangeDesiteMaximale(index_zone, densite_max);
				ChangeConstanteTassement(index_zone, tassement);

				ChangeSeuilFonteConifers(index_zone, seuil_conif);
				ChangeSeuilFonteFeuillus(index_zone, seuil_feuillu);
				ChangeSeuilFonteDecouver(index_zone, seuil_decou);

				ChangeTauxFonteConifers(index_zone, taux_conif);
				ChangeTauxFonteFeuillus(index_zone, taux_feuillu);
				ChangeTauxFonteDecouver(index_zone, taux_decou);

				_seuil_albedo[index_zone] = seuil_albedo;

				//_methode_albedo[index_zone] = 0;	//FIXÉ à 1 dans DEGRE_JOUR
			}

			fichier.close();

		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_BANDE");
		}
	}


	void DEGRE_JOUR_BANDE::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		string repertoire = _sim_hyd.PrendreRepertoireProjet();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; " + _sim_hyd._nomFichierParametresGlobal );

		bool bOK = false;

		try{

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, 1);

		istringstream iss;
		size_t nbGroupe, x, y, index_zone;
		double dVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "DEGRE JOUR BANDE")
			{
				++no_ligne;
				getline_mod(fichier, ligne);				
				ChangeIndexOccupationConifers(extrait_valeur(ligne));

				++no_ligne;
				getline_mod(fichier, ligne);				
				ChangeIndexOccupationFeuillus(extrait_valeur(ligne));

				++no_ligne;
				getline_mod(fichier, ligne);
				if (!ligne.empty() && !Racine(ligne))
					ligne = Combine(repertoire, ligne);
				_stations_neige_conifers.ChangeNomFichier(ligne);

				++no_ligne;
				getline_mod(fichier, ligne);
				if (!ligne.empty() && !Racine(ligne))
					ligne = Combine(repertoire, ligne);
				_stations_neige_feuillus.ChangeNomFichier(ligne);

				++no_ligne;
				getline_mod(fichier, ligne);
				if (!ligne.empty() && !Racine(ligne))
					ligne = Combine(repertoire, ligne);
				_stations_neige_decouver.ChangeNomFichier(ligne);

				++no_ligne;
				getline_mod(fichier, ligne);
				_interpolation_conifers = INTERPOLATION_AUCUNE;
				if (ligne == "THIESSEN")
					_interpolation_conifers = INTERPOLATION_THIESSEN;
				else if (ligne == "MOYENNE 3 STATIONS")
					_interpolation_conifers = INTERPOLATION_MOYENNE_3_STATIONS;

				++no_ligne;
				getline_mod(fichier, ligne);
				_interpolation_feuillus = INTERPOLATION_AUCUNE;
				if (ligne == "THIESSEN")
					_interpolation_feuillus = INTERPOLATION_THIESSEN;
				else if (ligne == "MOYENNE 3 STATIONS")
					_interpolation_feuillus = INTERPOLATION_MOYENNE_3_STATIONS;

				++no_ligne;
				getline_mod(fichier, ligne);
				_interpolation_decouver = INTERPOLATION_AUCUNE;
				if (ligne == "THIESSEN")
					_interpolation_decouver = INTERPOLATION_THIESSEN;
				else if (ligne == "MOYENNE 3 STATIONS")
					_interpolation_decouver = INTERPOLATION_MOYENNE_3_STATIONS;

				++no_ligne;
				getline_mod(fichier, ligne);
				iss.clear();
				iss.str(ligne);
				iss >> _dHauteurBande;

				//++no_ligne;
				//getline_mod(fichier, ligne);
				//if(ligne == "1")
				//	_bMAJGrilleNeige = true;
				//else
				//	_bMAJGrilleNeige = false;

				//++no_ligne;
				//getline_mod(fichier, ligne);
				//if (!ligne.empty() && !Racine(ligne))
				//	ligne = Combine(_sim_hyd.PrendreRepertoireProjet(), ligne);
				//_grilleneige._sPathFichierParam = ligne;

				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_dvaleur(ligne, ";");

					if(vValeur.size() != 11)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide. DEGRE JOUR BANDE");

					dVal = static_cast<double>(x);
					if(dVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. DEGRE JOUR BANDE. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						ChangeTauxFonte(index_zone, vValeur[1]);
						ChangeDesiteMaximale(index_zone, vValeur[2]);
						ChangeConstanteTassement(index_zone, vValeur[3]);

						ChangeSeuilFonteConifers(index_zone, vValeur[4]);
						ChangeSeuilFonteFeuillus(index_zone, vValeur[5]);
						ChangeSeuilFonteDecouver(index_zone, vValeur[6]);

						ChangeTauxFonteConifers(index_zone, vValeur[7]);
						ChangeTauxFonteFeuillus(index_zone, vValeur[8]);
						ChangeTauxFonteDecouver(index_zone, vValeur[9]);

						_seuil_albedo[index_zone] = vValeur[10];

						//_methode_albedo[index_zone] = 0;
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; DEGRE JOUR BANDE; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele DEGRE JOUR BANDE");
	}


	void DEGRE_JOUR_BANDE::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "CLASSE INTEGRE CONIFERES;";
		auto index_conifers = PrendreIndexOccupationConifers();
		for (auto iter = begin(index_conifers); iter != end(index_conifers); ++iter)
			fichier << *iter + 1 << ';';
		fichier << endl;

		fichier << "CLASSE INTEGRE FEUILLUS;";
		auto index_feuillus = PrendreIndexOccupationFeuillus();
		for (auto iter = begin(index_feuillus); iter != end(index_feuillus); ++iter)
			fichier << *iter + 1 << ';';
		fichier << endl << endl;

		fichier << "NOM FICHIER STATION NEIGE CONIFERS;" << endl;
		fichier << "NOM FICHIER STATION NEIGE FEUILLUS;" << endl;
		fichier << "NOM FICHIER STATION NEIGE DECOUVERTS;" << endl;
		fichier << endl;

		fichier << "INTERPOLATION STATION NEIGE CONIFERS;" << endl;
		fichier << "INTERPOLATION STATION NEIGE FEUILLUS;" << endl;
		fichier << "INTERPOLATION STATION NEIGE DECOUVERTS;" << endl;
		fichier << endl;

		fichier << "HAUTEUR BANDE(m);" << _dHauteurBande << endl;
		fichier << endl;

		fichier << "UHRH ID;TAUX DE FONTE (mm/jour);DENSITE MAXIMALE (Kg/m3);CONSTANTE TASSEMENT;SEUIL FONTE CONIFERS (C);SEUIL FONTE FEUILLUS (C);SEUIL FONTE DECOUVERTS (C);TAUX FONTE CONIFERS (mm/jour.C);TAUX FONTE FEUILLUS (mm/jour.C);TAUX FONTE DECOUVERTS (mm/jour.C);SEUIL ALBEDO" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << PrendreTauxFonte(index) << ';';
			fichier << PrendreDensiteMaximal(index) << ';';
			fichier << PrendreConstanteTassement(index) << ';';
			fichier << PrendreSeuilFonteConifers(index) << ';';
			fichier << PrendreSeuilFonteFeuillus(index) << ';';
			fichier << PrendreSeuilFonteDecouver(index) << ';';
			fichier << PrendreTauxFonteConifers(index) << ';';
			fichier << PrendreTauxFonteFeuillus(index) << ';';
			fichier << PrendreTauxFonteDecouver(index) << ';';
			fichier << _seuil_albedo[index] << endl;
		}

		fichier.close();
	}


	void DEGRE_JOUR_BANDE::ChangeNomFichierLectureEtat(std::string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}

	void DEGRE_JOUR_BANDE::ChangeRepertoireEcritureEtat(std::string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}

	void DEGRE_JOUR_BANDE::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}

	void DEGRE_JOUR_BANDE::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}

	string DEGRE_JOUR_BANDE::PrendreNomFichierLectureEtat() const
	{
		return _nom_fichier_lecture_etat;
	}

	string DEGRE_JOUR_BANDE::PrendreRepertoireEcritureEtat() const
	{
		return _repertoire_ecriture_etat;
	}

	bool DEGRE_JOUR_BANDE::PrendreSauvegardeTousEtat() const
	{
		return _sauvegarde_tous_etat;
	}

	DATE_HEURE DEGRE_JOUR_BANDE::PrendreDateHeureSauvegardeEtat() const
	{
		return _date_sauvegarde_etat;
	}

}
