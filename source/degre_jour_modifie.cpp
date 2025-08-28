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

#include "degre_jour_modifie.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "moyenne_3_stations1.hpp"
#include "moyenne_3_stations2.hpp"
#include "station_neige.hpp"
#include "thiessen1.hpp"
#include "thiessen2.hpp"
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

	DEGRE_JOUR_MODIFIE::DEGRE_JOUR_MODIFIE(SIM_HYD& sim_hyd)
		: FONTE_NEIGE(sim_hyd, "DEGRE JOUR MODIFIE")
	{
		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;
		_bMAJGrilleNeige = false;

		_nbr_jour_delai_mise_a_jour = 5;
		_grilleneige._sim_hyd = &sim_hyd;

		_pOutput = &sim_hyd.PrendreOutput();
		
		_netCdf_couvertnival = NULL;
		_netCdf_hauteurneige = NULL;
		_netCdf_albedoneige = NULL;

		_pThiessen1 = nullptr;
		_pThiessen2 = nullptr;
		_pMoy3station1 = nullptr;
		_pMoy3station2 = nullptr;
	}

	DEGRE_JOUR_MODIFIE::~DEGRE_JOUR_MODIFIE()
	{
	}

	void DEGRE_JOUR_MODIFIE::Initialise()
	{
		_corrections_neige_au_sol = _sim_hyd.PrendreCorrections().PrendreCorrectionsNeigeAuSol();

		ZONES& zones = _sim_hyd.PrendreZones();

		const size_t nb_zone = zones.PrendreNbZone();

		_ce1.resize(nb_zone, 0);
		_ce0.resize(nb_zone, 0);
		_tsn.resize(nb_zone, 0);

		_stock_feuillus.resize(nb_zone, 0);
		_stock_conifers.resize(nb_zone, 0);
		_stock_decouver.resize(nb_zone, 0);

		_hauteur_feuillus.resize(nb_zone, 0);
		_hauteur_conifers.resize(nb_zone, 0);
		_hauteur_decouver.resize(nb_zone, 0);

		_chaleur_feuillus.resize(nb_zone, 0);
		_chaleur_conifers.resize(nb_zone, 0);
		_chaleur_decouver.resize(nb_zone, 0);

		_eau_retenu_feuillus.resize(nb_zone, 0);
		_eau_retenu_conifers.resize(nb_zone, 0);
		_eau_retenu_decouver.resize(nb_zone, 0);

		_albedo_feuillus.resize(nb_zone, 0.8f);
		_albedo_conifers.resize(nb_zone, 0.8f);
		_albedo_decouver.resize(nb_zone, 0.8f);

		//_methode_albedo.resize(nb_zone, 0);

		auto occupation_sol = _sim_hyd.PrendreOccupationSol();

		_pourcentage_feuillus.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				float pourcentage = 0;

				for (auto index = begin(_index_occupation_feuillus); index != end(_index_occupation_feuillus); ++index)
					pourcentage += occupation_sol.PrendrePourcentage(index_zone, *index);

				_pourcentage_feuillus[index_zone] = pourcentage;
			}
		}

		_pourcentage_conifers.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				float pourcentage = 0;

				for (auto index = begin(_index_occupation_conifers); index != end(_index_occupation_conifers); ++index)
					pourcentage += occupation_sol.PrendrePourcentage(index_zone, *index);

				_pourcentage_conifers[index_zone] = pourcentage;
			}
		}

		_pourcentage_autres.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
				_pourcentage_autres[index_zone] = max(1.0f - _pourcentage_feuillus[index_zone] - _pourcentage_conifers[index_zone], 0.0f);
		}

		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				float theta = static_cast<float>(zone.PrendreCentroide().PrendreY()) / RAD1;

				float k = atan(zone.PrendrePente());
				float h = ((495 - zone.PrendreOrientation() * 45) % 360) / RAD1;

				_ce1[index_zone] = asin(sin(k) * cos(h) * cos(theta) + cos(k) * sin(theta)) * RAD1;
				_ce0[index_zone] = atan(sin(h) * sin(k) / (cos(k) * cos(theta) - cos(h) * sin(k) * sin(theta))) * RAD1;

				if (_methode_albedo[index_zone] == 0)
				{
					float albedo = (_albedo_conifers[index_zone] + _albedo_feuillus[index_zone] + _albedo_decouver[index_zone]) / 3;
					_tsn[index_zone] = static_cast<float>( log((albedo - 0.8) / 0.4 + 1) / -0.2 );
				}
			}
		}

		const PROJECTION& projection = zones.PrendreProjection();

		// ponderations et lecture des donnees des stations de neige

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
					_pThiessen1->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifersF, "DEGRE_JOUR_MODIFIE");
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
					_pThiessen2->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifers, "DEGRE_JOUR_MODIFIE");
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
					_pMoy3station1->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifersF, "DEGRE_JOUR_MODIFIE");
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
					_pMoy3station2->CalculePonderation(_stations_neige_conifers, zones, _ponderation_conifers, "DEGRE_JOUR_MODIFIE");
					_pMoy3station2->SauvegardePonderation(_stations_neige_conifers, zones, _ponderation_conifers);
				}
			}

			_stations_neige_conifers.LectureDonnees(date_debut, date_fin);
			break;

		default:
			throw ERREUR("interpolation stations neige conifers");
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
					_pThiessen1->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF, "DEGRE_JOUR_MODIFIE");
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
					_pThiessen2->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus, "DEGRE_JOUR_MODIFIE");
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
					_pMoy3station1->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillusF, "DEGRE_JOUR_MODIFIE");
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
					_pMoy3station2->CalculePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus, "DEGRE_JOUR_MODIFIE");
					_pMoy3station2->SauvegardePonderation(_stations_neige_feuillus, zones, _ponderation_feuillus);
				}
			}

			_stations_neige_feuillus.LectureDonnees(date_debut, date_fin);
			break;

		default:
			throw ERREUR("interpolation stations neige feuillus");
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
					_pThiessen1->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouverF, "DEGRE_JOUR_MODIFIE");
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
					_pThiessen2->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouver, "DEGRE_JOUR_MODIFIE");
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
					_pMoy3station1->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouverF, "DEGRE_JOUR_MODIFIE");
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
					_pMoy3station2->CalculePonderation(_stations_neige_decouver, zones, _ponderation_decouver, "DEGRE_JOUR_MODIFIE");
					_pMoy3station2->SauvegardePonderation(_stations_neige_decouver, zones, _ponderation_decouver);
				}
			}

			_stations_neige_decouver.LectureDonnees(date_debut, date_fin);
			break;

		default:
			throw ERREUR("interpolation stations neige decouver");
		}

		// mise a jour de la neige avec grille
		if(_bMAJGrilleNeige)
		{
			_maj_feuillus.resize(nb_zone);
			_maj_conifers.resize(nb_zone);
			_maj_decouver.resize(nb_zone);

			for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
			{
				_maj_feuillus[index_zone].nb_pas_derniere_correction = 0;
				_maj_feuillus[index_zone].pourcentage_corrige = 0;
				_maj_feuillus[index_zone].pourcentage_sim_eq = 1;
				_maj_feuillus[index_zone].pourcentage_sim_ha = 1;
				_maj_feuillus[index_zone].stations_utilisees.resize( _stations_neige_feuillus.PrendreNbStation() );

				_maj_conifers[index_zone].nb_pas_derniere_correction = 0;
				_maj_conifers[index_zone].pourcentage_corrige = 0;
				_maj_conifers[index_zone].pourcentage_sim_eq = 1;
				_maj_conifers[index_zone].pourcentage_sim_ha = 1;
				_maj_conifers[index_zone].stations_utilisees.resize( _stations_neige_conifers.PrendreNbStation() );

				_maj_decouver[index_zone].nb_pas_derniere_correction = 0;
				_maj_decouver[index_zone].pourcentage_corrige = 0;
				_maj_decouver[index_zone].pourcentage_sim_eq = 1;
				_maj_decouver[index_zone].pourcentage_sim_ha = 1;
				_maj_decouver[index_zone].stations_utilisees.resize( _stations_neige_decouver.PrendreNbStation() );
			}

			_grilleneige.Initialise();
		}
		else
		{
			// mise a jour de la neige avec stations de neige
			if (_stations_neige_conifers.PrendreNbStation() > 0 || 
				_stations_neige_decouver.PrendreNbStation() > 0 ||
				_stations_neige_feuillus.PrendreNbStation() > 0)
			{
				_mise_a_jour_neige = true;

				_maj_feuillus.resize(nb_zone);
				_maj_conifers.resize(nb_zone);
				_maj_decouver.resize(nb_zone);

				for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
				{
					_maj_feuillus[index_zone].nb_pas_derniere_correction = 0;
					_maj_feuillus[index_zone].pourcentage_corrige = 0;
					_maj_feuillus[index_zone].pourcentage_sim_eq = 1;
					_maj_feuillus[index_zone].pourcentage_sim_ha = 1;
					_maj_feuillus[index_zone].stations_utilisees.resize( _stations_neige_feuillus.PrendreNbStation() );

					_maj_conifers[index_zone].nb_pas_derniere_correction = 0;
					_maj_conifers[index_zone].pourcentage_corrige = 0;
					_maj_conifers[index_zone].pourcentage_sim_eq = 1;
					_maj_conifers[index_zone].pourcentage_sim_ha = 1;
					_maj_conifers[index_zone].stations_utilisees.resize( _stations_neige_conifers.PrendreNbStation() );

					_maj_decouver[index_zone].nb_pas_derniere_correction = 0;
					_maj_decouver[index_zone].pourcentage_corrige = 0;
					_maj_decouver[index_zone].pourcentage_sim_eq = 1;
					_maj_decouver[index_zone].pourcentage_sim_ha = 1;
					_maj_decouver[index_zone].stations_utilisees.resize( _stations_neige_decouver.PrendreNbStation() );
				}
			}
			else
				_mise_a_jour_neige = false;
		}

		//initialisation sauvegarde des resultats

		if (_pOutput->SauvegardeCouvertNival())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_couvertnival = new float[_sim_hyd._lNbPasTempsSim*_pOutput->_uhrhOutputNb];
			else
			{
				string sFile( Combine(_sim_hyd.PrendreRepertoireResultat(), "couvert_nival.csv") );
				_fichier_couvert_nival.open(sFile);
				_fichier_couvert_nival << "Couvert nival (EEN) (mm)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _pOutput->Separator();

				string str;
				ostringstream oss;
				oss.str("");

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_couvert_nival << str << endl;
			}
		}

		//fichier hauteur_neige
		if (_pOutput->SauvegardeHauteurNeige())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_hauteurneige = new float[_sim_hyd._lNbPasTempsSim*_pOutput->_uhrhOutputNb];
			else
			{
				string sFile( Combine(_sim_hyd.PrendreRepertoireResultat(), "hauteur_neige.csv") );
				_fichier_hauteur_neige.open(sFile);
				_fichier_hauteur_neige << "Hauteur couvert nival (m)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _pOutput->Separator();

				string str;
				ostringstream oss;
				oss.str("");

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_hauteur_neige << str << endl;
			}
		}

		//fichier albedo_neige
		if (_pOutput->SauvegardeAlbedoNeige())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_albedoneige = new float[_sim_hyd._lNbPasTempsSim*_pOutput->_uhrhOutputNb];
			else
			{
				string sFile( Combine(_sim_hyd.PrendreRepertoireResultat(), "albedo_neige.csv") );
				_fichier_albedo_neige.open(sFile);
				_fichier_albedo_neige << "Albedo neige (0-1)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _pOutput->Separator();

				string str;
				ostringstream oss;
				oss.str("");

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_albedo_neige << str << endl;
			}
		}

		//Fichier d'état
		if (!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		FONTE_NEIGE::Initialise();
	}


	void DEGRE_JOUR_MODIFIE::Calcule()
	{
		STATION_NEIGE::typeOccupationStation occupation;	//pour maj grille neige
		string sString, sString2, sString3;
		float fCoeffAdditif;

		ZONES& zones = _sim_hyd.PrendreZones();

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		if(_bMAJGrilleNeige)
		{
			//lecture des grilles equivalent en eau & hauteur de neige pour le pas de temps courant
			_grilleneige._grilleEquivalentEau.clear();
			_grilleneige._grilleHauteurNeige.clear();

			_grilleneige.FormatePathFichierGrilleCourant(sString);
			sString2 = sString + ".een";
			sString3 = sString + ".hau";
			if(FichierExiste(sString2) && FichierExiste(sString3))
			{
				_grilleneige._grilleEquivalentEau.push_back(ReadGeoTIFF_float(sString2));
				_grilleneige._grilleHauteurNeige.push_back(ReadGeoTIFF_float(sString3));
			}
		}

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			float apport_conifers = 0;
			float apport_feuillus = 0;
			float apport_decouver = 0;
			
			CalculIndiceRadiation(date_courante, pas_de_temps, zone, index_zone);

			// calcul albedo de la neige
			if (_methode_albedo[index_zone] == 0)	//methode albedo fixé pour l'instant à sol-neige
			{
				float neige = zone.PrendreNeige() / 10.0f;	//mm -> cm

				_tsn[index_zone] = neige < _seuil_albedo[index_zone] ? _tsn[index_zone] + static_cast<float>(pas_de_temps) / 24 : 0;

				float albedo = min(0.8f - 0.4f * (1.0f - exp(-0.2f * _tsn[index_zone])), 1.0f);

				_albedo_conifers[index_zone] = albedo;
				_albedo_feuillus[index_zone] = albedo;
				_albedo_decouver[index_zone] = albedo;
			}

			CalculeFonte(zone, 
				index_zone, 
				pas_de_temps, 
				_pourcentage_conifers[index_zone], 
				_taux_fonte_conifers[index_zone] / 1000, 
				_seuil_fonte_conifers[index_zone], 
				_albedo_conifers[index_zone], 
				_stock_conifers[index_zone], 
				_hauteur_conifers[index_zone], 
				_chaleur_conifers[index_zone], 
				apport_conifers, 
				_eau_retenu_conifers[index_zone]);


			CalculeFonte(zone, 
				index_zone, 
				pas_de_temps, 
				_pourcentage_feuillus[index_zone], 
				_taux_fonte_feuillus[index_zone] / 1000, 
				_seuil_fonte_feuillus[index_zone], 
				_albedo_feuillus[index_zone], 
				_stock_feuillus[index_zone], 
				_hauteur_feuillus[index_zone], 
				_chaleur_feuillus[index_zone], 
				apport_feuillus, 
				_eau_retenu_feuillus[index_zone]);


			CalculeFonte(zone, 
				index_zone, 
				pas_de_temps, 
				_pourcentage_autres[index_zone],
				_taux_fonte_decouver[index_zone] / 1000, 
				_seuil_fonte_decouver[index_zone], 
				_albedo_decouver[index_zone], 
				_stock_decouver[index_zone], 
				_hauteur_decouver[index_zone], 
				_chaleur_decouver[index_zone], 
				apport_decouver, 
				_eau_retenu_decouver[index_zone]);


			if(_bMAJGrilleNeige)
			{
				if(_grilleneige._grilleEquivalentEau.size() != 0)	//s'il y a des données pour le jour courant
				{
					// On prend l occupation du sol dominant de lUHRH
					occupation = STATION_NEIGE::RESINEUX;
								
					if(_pourcentage_conifers[index_zone] < _pourcentage_feuillus[index_zone])
					{
						occupation = STATION_NEIGE::FEUILLUS;
						if ( _pourcentage_feuillus[index_zone] < _pourcentage_autres[index_zone])
							occupation = STATION_NEIGE::DECOUVERTE;
					}
					else
					{
						if(_pourcentage_conifers[index_zone] < _pourcentage_autres[index_zone])
							occupation = STATION_NEIGE::DECOUVERTE;
					}

					MiseAJourGrille(index_zone, occupation);
				}
			}
			else
			{
				if (_mise_a_jour_neige)
					MiseAJour(date_courante, index_zone);
			}


			float stock_moyen = ( _pourcentage_conifers[index_zone] * _stock_conifers[index_zone] + 
				_pourcentage_autres[index_zone] * _stock_decouver[index_zone] + 
				_pourcentage_feuillus[index_zone] * _stock_feuillus[index_zone] ) * 1000.0f;	// * 1000: [m] -> [mm]

			zone.ChangeApport(max(0.0f, (apport_conifers + apport_feuillus + apport_decouver) * 1000.0f));	// * 1000: [m] -> [mm]

			if (stock_moyen >= 0.001f)
			{
				float albedo_moyen = ( _pourcentage_conifers[index_zone] * _albedo_conifers[index_zone] + 
				_pourcentage_autres[index_zone] * _albedo_decouver[index_zone] + 
				_pourcentage_feuillus[index_zone] * _albedo_feuillus[index_zone] );

				zone.ChangeAlbedoNeige(albedo_moyen);
				zone.ChangeCouvertNival(stock_moyen);   //equivalent en eau de la neige [mm]
			}
			else
			{
				zone.ChangeAlbedoNeige(0.0f);
				zone.ChangeCouvertNival(0.0f);   //equivalent en eau de la neige [mm]
			}

			float fHauteurCouvertNival = _pourcentage_conifers[index_zone] * _hauteur_conifers[index_zone] + 
								_pourcentage_autres[index_zone] * _hauteur_decouver[index_zone] + 
								_pourcentage_feuillus[index_zone] * _hauteur_feuillus[index_zone];	//m

			zone.ChangeHauteurCouvertNival(fHauteurCouvertNival);	//m
		}

		// correction de la neige au sol

		for(auto iter = begin(_corrections_neige_au_sol); iter != end(_corrections_neige_au_sol); ++iter)
		{
			CORRECTION* correction = *iter;

			if (correction->Applicable(date_courante))
			{
				GROUPE_ZONE* groupe_zone = nullptr;

				switch (correction->PrendreTypeGroupe())
				{
				case TYPE_GROUPE_ALL:
					groupe_zone = _sim_hyd.PrendreToutBassin();
					break;

				case TYPE_GROUPE_HYDRO:
					groupe_zone = _sim_hyd.RechercheGroupeZone(correction->PrendreNomGroupe());
					break;

				case TYPE_GROUPE_CORRECTION:
					groupe_zone = _sim_hyd.RechercheGroupeCorrection(correction->PrendreNomGroupe());
					break;
				}

				fCoeffAdditif = correction->PrendreCoefficientAdditif() / 1000.0f;

				for (size_t index = 0; index < groupe_zone->PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						int ident = groupe_zone->PrendreIdent(index);
						size_t index_zone = zones.IdentVersIndex(ident);

						float stock_avant;
					
						//
						stock_avant = _stock_conifers[index_zone];
						_stock_conifers[index_zone] = (stock_avant + fCoeffAdditif) * correction->PrendreCoefficientMultiplicatif();
						
						if(_stock_conifers[index_zone] < 0.0f)
							_stock_conifers[index_zone] = 0.0f;

						if(stock_avant != 0.0)
						{
							_chaleur_conifers[index_zone] = _chaleur_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
							_eau_retenu_conifers[index_zone] = _eau_retenu_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
						}

						//
						stock_avant = _stock_feuillus[index_zone];
						_stock_feuillus[index_zone] = (stock_avant + fCoeffAdditif) * correction->PrendreCoefficientMultiplicatif();

						if(_stock_feuillus[index_zone] < 0.0f)
							_stock_feuillus[index_zone] = 0.0f;

						if(stock_avant != 0.0)
						{
							_chaleur_feuillus[index_zone] = _chaleur_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
							_eau_retenu_feuillus[index_zone] = _eau_retenu_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
						}

						//
						stock_avant = _stock_decouver[index_zone];
						_stock_decouver[index_zone] = (stock_avant + fCoeffAdditif) * correction->PrendreCoefficientMultiplicatif();

						if(_stock_decouver[index_zone] < 0.0f)
							_stock_decouver[index_zone] = 0.0f;

						if(stock_avant != 0.0)
						{
							_chaleur_decouver[index_zone] = _chaleur_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
							_eau_retenu_decouver[index_zone] = _eau_retenu_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
						}
					}
				}
			}
		}

		size_t i, idx;
		string str;
		float stock_moyen;

		//fichier couvert_nival.csv
		if (_pOutput->SauvegardeCouvertNival())
		{
			if (_netCdf_couvertnival != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _pOutput->_uhrhOutputNb;

				for (i=0; i<_pOutput->_uhrhOutputNb; i++)
				{
					stock_moyen = (_pourcentage_conifers[_pOutput->_uhrhOutputIndex[i]] * _stock_conifers[_pOutput->_uhrhOutputIndex[i]] +
						_pourcentage_autres[_pOutput->_uhrhOutputIndex[i]] * _stock_decouver[_pOutput->_uhrhOutputIndex[i]] +
						_pourcentage_feuillus[_pOutput->_uhrhOutputIndex[i]] * _stock_feuillus[_pOutput->_uhrhOutputIndex[i]]) * 1000.0f;	//equivalent en eau du couvert nival	//m -> mm

					_netCdf_couvertnival[idx+i] = stock_moyen;
				}
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_mm) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_pOutput->_bSauvegardeTous || 
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							stock_moyen = ( _pourcentage_conifers[index] * _stock_conifers[index] + 
								_pourcentage_autres[index] * _stock_decouver[index] + 
								_pourcentage_feuillus[index] * _stock_feuillus[index] ) * 1000.0f;	//equivalent en eau du couvert nival	//m -> mm

							oss << stock_moyen << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_couvert_nival << str << endl;
			}
		}

		//fichier hauteur_neige.csv
		if (_pOutput->SauvegardeHauteurNeige())
		{
			if (_netCdf_hauteurneige != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _pOutput->_uhrhOutputNb;

				for (i=0; i<_pOutput->_uhrhOutputNb; i++)
					_netCdf_hauteurneige[idx+i] = zones[_pOutput->_uhrhOutputIndex[i]].PrendreHauteurCouvertNival();	//m;
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_m) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_pOutput->_bSauvegardeTous || 
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							float hauteurNeige = zones[index].PrendreHauteurCouvertNival();	//m
							oss << hauteurNeige << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_hauteur_neige << str << endl;
			}
		}

		//fichier albedo_neige.csv
		if (_pOutput->SauvegardeAlbedoNeige())
		{
			if (_netCdf_albedoneige != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _pOutput->_uhrhOutputNb;

				for (i=0; i<_pOutput->_uhrhOutputNb; i++)
					_netCdf_albedoneige[idx+i] = zones[_pOutput->_uhrhOutputIndex[i]].PrendreAlbedoNeige();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_ratio) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_pOutput->_bSauvegardeTous || 
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							float fVal = zones[index].PrendreAlbedoNeige();		//0-1
							oss << fVal << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_albedo_neige << str << endl;
			}
		}

		//Fichier d'état
		if (_sauvegarde_tous_etat || (_sauvegarde_etat && (_date_sauvegarde_etat - pas_de_temps == date_courante)))
			SauvegardeEtat(date_courante);

		FONTE_NEIGE::Calcule();
	}


	void DEGRE_JOUR_MODIFIE::Termine()
	{
		string str1, str2;

		_ce1.clear();
		_ce0.clear();
		_tsn.clear();

		_stock_feuillus.clear();
		_stock_conifers.clear();
		_stock_decouver.clear();

		_hauteur_feuillus.clear();
		_hauteur_conifers.clear();
		_hauteur_decouver.clear();

		_chaleur_feuillus.clear();
		_chaleur_conifers.clear();
		_chaleur_decouver.clear();

		_eau_retenu_feuillus.clear();
		_eau_retenu_conifers.clear();
		_eau_retenu_decouver.clear();

		_albedo_feuillus.clear();
		_albedo_conifers.clear();
		_albedo_decouver.clear();

		_methode_albedo.clear();

		if (_pOutput->SauvegardeCouvertNival())
		{
			if (_netCdf_couvertnival != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "fonte_neige-degre_jour_mod-couvert_nival.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "couvert_nival.nc");
				//str2 = _pOutput->SauvegardeOutputNetCDF(str1, true, "fonte_neige-degre_jour_mod-couvert_nival", _netCdf_couvertnival, "mm", "Equivalent en eau du couvert nival");
				str2 = _pOutput->SauvegardeOutputNetCDF(str1, true, "couvert_nival", _netCdf_couvertnival, "mm", "Equivalent en eau du couvert nival");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_couvertnival;
			}
			else
				_fichier_couvert_nival.close();
		}

		if (_pOutput->SauvegardeHauteurNeige())
		{
			if (_netCdf_hauteurneige != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "fonte_neige-degre_jour_mod-hauteur_neige.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "hauteur_neige.nc");
				//str2 = _pOutput->SauvegardeOutputNetCDF(str1, true, "fonte_neige-degre_jour_mod-hauteur_neige", _netCdf_hauteurneige, "m", "Hauteur du couvert nival");
				str2 = _pOutput->SauvegardeOutputNetCDF(str1, true, "hauteur_neige", _netCdf_hauteurneige, "m", "Hauteur du couvert nival");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_hauteurneige;
			}
			else
				_fichier_hauteur_neige.close();
		}

		if (_pOutput->SauvegardeAlbedoNeige())
		{
			if (_netCdf_albedoneige != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "fonte_neige-degre_jour_mod-albedo_neige.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "albedo_neige.nc");
				//str2 = _pOutput->SauvegardeOutputNetCDF(str1, true, "fonte_neige-degre_jour_mod-albedo_neige", _netCdf_albedoneige, "[0-1]", "Albedo de la neige");
				str2 = _pOutput->SauvegardeOutputNetCDF(str1, true, "albedo_neige", _netCdf_albedoneige, "[0-1]", "Albedo de la neige");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_albedoneige;
			}
			else
				_fichier_albedo_neige.close();
		}

		_maj_feuillus.clear();
		_maj_conifers.clear();
		_maj_decouver.clear();

		FONTE_NEIGE::Termine();
	}


	void DEGRE_JOUR_MODIFIE::CalculIndiceRadiation(DATE_HEURE date_heure, unsigned short pas_de_temps, ZONE& zone, size_t index_zone)
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


	void DEGRE_JOUR_MODIFIE::ChangeNbParams(const ZONES& zones)
	{
		_seuil_fonte_feuillus.resize(zones.PrendreNbZone(), 0.0f);
		_seuil_fonte_conifers.resize(zones.PrendreNbZone(), 0.0f);
		_seuil_fonte_decouver.resize(zones.PrendreNbZone(), 0.0f);

		_taux_fonte_conifers.resize(zones.PrendreNbZone(), 12.0f);
		_taux_fonte_feuillus.resize(zones.PrendreNbZone(), 14.0f);
		_taux_fonte_decouver.resize(zones.PrendreNbZone(), 16.0f);

		_taux_fonte.resize(zones.PrendreNbZone(), 0.5f);
		_densite_maximale.resize(zones.PrendreNbZone(), 550.0f);
		_constante_tassement.resize(zones.PrendreNbZone(), 0.1f);

		_seuil_albedo.resize(zones.PrendreNbZone(), 1.0f);
		_methode_albedo.resize(zones.PrendreNbZone(), 1);
	}

	void DEGRE_JOUR_MODIFIE::ChangeSeuilFonteFeuillus(size_t index_zone, float seuil_fonte)
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_feuillus.size());
		_seuil_fonte_feuillus[index_zone] = seuil_fonte;
	}

	void DEGRE_JOUR_MODIFIE::ChangeSeuilFonteConifers(size_t index_zone, float seuil_fonte)
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_conifers.size());
		_seuil_fonte_conifers[index_zone] = seuil_fonte;
	}

	void DEGRE_JOUR_MODIFIE::ChangeSeuilFonteDecouver(size_t index_zone, float seuil_fonte)
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_decouver.size());
		_seuil_fonte_decouver[index_zone] = seuil_fonte;
	}

	void DEGRE_JOUR_MODIFIE::ChangeTauxFonteFeuillus(size_t index_zone, float taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte_feuillus.size() && taux_fonte >= 0);
		_taux_fonte_feuillus[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_MODIFIE::ChangeTauxFonteConifers(size_t index_zone, float taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte_conifers.size() && taux_fonte >= 0);
		_taux_fonte_conifers[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_MODIFIE::ChangeTauxFonteDecouver(size_t index_zone, float taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte_decouver.size() && taux_fonte >= 0);
		_taux_fonte_decouver[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_MODIFIE::ChangeTauxFonte(size_t index_zone, float taux_fonte)
	{
		BOOST_ASSERT(index_zone < _taux_fonte.size());
		_taux_fonte[index_zone] = taux_fonte;
	}

	void DEGRE_JOUR_MODIFIE::ChangeDesiteMaximale(size_t index_zone, float densite)
	{
		BOOST_ASSERT(index_zone < _densite_maximale.size());
		_densite_maximale[index_zone] = densite;
	}

	void DEGRE_JOUR_MODIFIE::ChangeConstanteTassement(size_t index_zone, float constante_tassement)
	{
		BOOST_ASSERT(index_zone < _constante_tassement.size());
		_constante_tassement[index_zone] = constante_tassement;
	}

	void DEGRE_JOUR_MODIFIE::ChangeIndexOccupationFeuillus(const std::vector<size_t>& index)
	{
		_index_occupation_feuillus = index;
		_index_occupation_feuillus.shrink_to_fit();
	}

	void DEGRE_JOUR_MODIFIE::ChangeIndexOccupationConifers(const std::vector<size_t>& index)
	{
		_index_occupation_conifers = index;
		_index_occupation_conifers.shrink_to_fit();
	}

	void DEGRE_JOUR_MODIFIE::CalculeFonte(ZONE& zone, size_t index_zone, unsigned short pas_de_temps, float proportion_terrain,
		float coeff_fonte, float temperaure_de_fonte, float& albedo, float& stock_neige, float& hauteur_neige, float& chaleur_stock, 
		float& apport, float& eau_retenue)
	{
		float dennei;
		float hneige;
		float densto;
		float rmax;
		float beta2, eq_neige, st_neige;
		int liquide = 0;

		if(stock_neige > 0.0f && hauteur_neige == 0.0f)
			hauteur_neige = 0.00000000001f;

		float temperature_moyenne = (zone.PrendreTMin() + zone.PrendreTMax()) / 2;
		float pluie = zone.PrendrePluie() / 1000.0f;	//mm -> m
		float neige = zone.PrendreNeige() / 1000.0f;	//mm -> m
		float indice_radiation = zone.PrendreIndiceRadiation();

		int pdts = pas_de_temps * 60 * 60;

		if (stock_neige > 0 || neige > 0)
		{
			float tneige = temperature_moyenne;

			// calcul de la densite relative de la neige fraiche
			float drel = CalculDensiteNeige(temperature_moyenne) / DENSITE_EAU;

			// ajout de la nouvelle neige
			stock_neige += neige * drel;
			hauteur_neige += neige;
			chaleur_stock += neige * drel * DENSITE_EAU * CHALEUR_NEIGE * temperature_moyenne;

			// calcul la densite de la neige stockee
			dennei = stock_neige / hauteur_neige;

			// pertes de chaleur par convection
			if (temperature_moyenne < temperaure_de_fonte)
			{
				tneige = chaleur_stock / (stock_neige * CHALEUR_NEIGE * DENSITE_EAU);

				hneige = (hauteur_neige < 0.4f) ? (0.5f * hauteur_neige) : (0.2f + 0.25f * (hauteur_neige - 0.4f));

				float alpha = ConductiviteNeige(dennei * DENSITE_EAU) / (dennei * DENSITE_EAU * CHALEUR_NEIGE);
				float erf = Erf(hneige / (2 * sqrt(alpha * pdts)));

				tneige = temperature_moyenne + (tneige - temperature_moyenne) * erf;
				chaleur_stock = tneige * stock_neige * DENSITE_EAU * CHALEUR_NEIGE;
			}

			// ajustement du bilan calorifique en fonction de l'eau retenue au pas precedent
			chaleur_stock += (eau_retenue * DENSITE_EAU * CHALEUR_FONTE);

			// ajout de la pluie
			stock_neige += pluie;
			chaleur_stock += pluie * DENSITE_EAU * (CHALEUR_FONTE + CHALEUR_EAU * temperature_moyenne);

			// ajout de la chaleur due au gradient geothermique
			chaleur_stock += (_taux_fonte[index_zone] * pas_de_temps / 24.0f) /1000.0f * DENSITE_EAU * CHALEUR_FONTE;

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
				eq_neige = neige * drel * 1000.0f;
				st_neige = (stock_neige - neige * drel) * 1000.0f;

				if (pluie > 0.0 || tneige >= 0)
				{
					liquide = 1;
				}
				else
				{
					liquide = 0;
				}

				if (st_neige > 0)  // s'il y a deja de la neige au sol
				{
					float alb_t_plus_1 = (1 - exp(-0.5f * eq_neige)) * 0.8f + (1 - (1 - exp(-0.5f * eq_neige))) * 
						(0.5f + (albedo - 0.5f) * exp(-0.2f * pas_de_temps / 24.0f * (1 + liquide)));

					// il y a probablement une erreur - albedo de la line precedente doit etre excluvement l'albedo de la partie specifique au stock de neige du pas precedent
					// il ne serait donc plus nécessaire de faire la conditionelle suivant mais l'Albedo stocke d'un pas de temps a l'autre devra differer
					if (albedo < 0.5f)
					{
						beta2 = 0.2f;
					}
					else
					{
						beta2 = 0.2f + (albedo - 0.5f);
					}
					albedo = (1 - exp(-beta2 * st_neige)) * alb_t_plus_1 + (1 - (1 - exp(-beta2 * st_neige))) * 0.15f;
				}
				else // s'il n'y a pas de neige au sol
				{
					albedo = (1 - exp(-0.5f * eq_neige)) * 0.8f + (1 - (1 - exp(-0.5f * eq_neige))) * 0.15f;
				}
			}

			// ajout de la chaleur de fonte par rayonnement
			float fonte = (temperature_moyenne > temperaure_de_fonte) ? 
				(coeff_fonte * (temperature_moyenne - temperaure_de_fonte) * indice_radiation * (1-albedo)) : 0.0f;

			fonte = fonte * (pas_de_temps / 24.0f);
			chaleur_stock += (fonte * DENSITE_EAU *	CHALEUR_FONTE);

			// calcul de la hauteur du stock et de sa densite apres compaction et ajout de la pluie
			float compaction = hauteur_neige * (_constante_tassement[index_zone] * (pas_de_temps / 24.0f)) * (1 - dennei / _densite_maximale[index_zone] * 1000);
			if (compaction < 0.0) 
			{
				compaction = 0.0f;
			}

			hauteur_neige -= compaction;
			densto = stock_neige / hauteur_neige;

			// si la densite depasse le maximum permis
			if (densto * 1000 > _densite_maximale[index_zone])
			{
				densto = _densite_maximale[index_zone] / 1000;
				hauteur_neige = stock_neige / densto;
			}

			// calcul du surplus calorifique et de la fonte
			if (chaleur_stock > 0.0)
			{
				fonte = chaleur_stock / CHALEUR_FONTE / DENSITE_EAU;
				if (fonte > stock_neige) 
					fonte = stock_neige;
				stock_neige -= fonte;
				if ((fonte - pluie) > 0) 
					hauteur_neige -= ((fonte - pluie) / densto);
				if (hauteur_neige <= 0.0) 
					hauteur_neige = stock_neige / densto;
				chaleur_stock -= (fonte * DENSITE_EAU * CHALEUR_FONTE);
			}
			else
			{
				fonte = 0.0f;
			}

			// reinitialisation des stocks si l'equivalent en eau est nul
			if (stock_neige < 0.0001) 
			{
				stock_neige = 0.0f;
				hauteur_neige = 0.0f;
				chaleur_stock = 0.0f;
				eau_retenue = 0.0f;
			}

			// calcul de l'eau retenue dans le stock_neige
			rmax = (0.1f * dennei) * stock_neige;
			if (rmax > fonte)
			{
				stock_neige += fonte;
				eau_retenue = fonte;
				fonte = 0.0f;
			}
			else
			{
				stock_neige += rmax;
				eau_retenue = rmax;
				fonte -= rmax;
			}

			// proportion du type de couverture
			apport = proportion_terrain * fonte;
		}
		else
		{
			apport = proportion_terrain * pluie;
		}
	}


	void DEGRE_JOUR_MODIFIE::MiseAJour(const DATE_HEURE& date_courante, size_t index_zone)
	{
		int nb_pas_par_jour = 24 / _sim_hyd.PrendrePasDeTemps();


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		_maj_conifers[index_zone].nb_pas_derniere_correction++;

		for (size_t index_station = 0; index_station < _stations_neige_conifers.PrendreNbStation(); ++index_station)
		{
			STATION_NEIGE* station_neige = static_cast<STATION_NEIGE*>(_stations_neige_conifers[index_station]);

			float ponderation;

			if((_interpolation_conifers == INTERPOLATION_THIESSEN && _sim_hyd._versionTHIESSEN == 1) || (_interpolation_conifers == INTERPOLATION_MOYENNE_3_STATIONS && _sim_hyd._versionMOY3STATION == 1))
				ponderation = _ponderation_conifersF(index_zone, index_station);
			else
				ponderation = static_cast<float>(_ponderation_conifers(index_zone, index_station));

			if (station_neige->DonneeDisponible(date_courante) && ponderation > 0)
			{				
				auto donnees = station_neige->PrendreDonnee(date_courante);

				if (_maj_conifers[index_zone].stations_utilisees[index_station] || 
					_maj_conifers[index_zone].pourcentage_corrige >= 1 ||
					_maj_conifers[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
				{
					_maj_conifers[index_zone].nb_pas_derniere_correction = 0;
					
					_maj_conifers[index_zone].pourcentage_corrige = 0;
					_maj_conifers[index_zone].pourcentage_sim_eq = 1;
					_maj_conifers[index_zone].pourcentage_sim_ha = 1;

					for (size_t n = 0; n < _maj_conifers[index_zone].stations_utilisees.size(); ++n)
						_maj_conifers[index_zone].stations_utilisees[n] = false;
				}

				_maj_conifers[index_zone].stations_utilisees[index_station] = true;
				_maj_conifers[index_zone].pourcentage_corrige += ponderation;

				if (donnees.equi_eau > 0)
				{
					float stock_ref = _stock_conifers[index_zone];
					float hauteur_ref = _hauteur_conifers[index_zone];

					float stock = stock_ref;
					
					UneMiseAJourOccupation(
						donnees.equi_eau, 
						stock,
						stock_ref,
						_maj_conifers[index_zone].pourcentage_corrige,
						ponderation,
						_maj_conifers[index_zone].pourcentage_sim_eq);

					_stock_conifers[index_zone] = stock;

					if (stock_ref == 0)
					{
						_hauteur_conifers[index_zone] = 0;
					}
					else
					{
						_chaleur_conifers[index_zone] = _chaleur_conifers[index_zone] * _stock_conifers[index_zone] / stock_ref;
						_eau_retenu_conifers[index_zone] = _eau_retenu_conifers[index_zone] * _stock_conifers[index_zone] / stock_ref;
					}

					// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
					if (donnees.hauteur <= 0)
					{
						if (stock_ref == 0)
							donnees.hauteur = donnees.equi_eau * 2.5f;
						else
							donnees.hauteur = donnees.equi_eau * hauteur_ref / stock_ref;
					}

					if (_hauteur_conifers[index_zone] > 0)
					{
						float hauteur = _hauteur_conifers[index_zone];

						UneMiseAJourOccupation(donnees.hauteur,
							hauteur,
							hauteur_ref,
							_maj_conifers[index_zone].pourcentage_corrige,
							ponderation,
							_maj_conifers[index_zone].pourcentage_sim_ha);

						_hauteur_conifers[index_zone] = hauteur;
					}
					else
					{
						if (_stock_conifers[index_zone] > 0)
						{
							if (stock_ref == 0)
								_hauteur_conifers[index_zone] = _stock_conifers[index_zone] * 2.5f;
							else
								_hauteur_conifers[index_zone] = hauteur_ref * _stock_conifers[index_zone] / stock_ref;
						}
					}
				}
			}
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		_maj_feuillus[index_zone].nb_pas_derniere_correction++;

		for (size_t index_station = 0; index_station < _stations_neige_feuillus.PrendreNbStation(); ++index_station)
		{
			STATION_NEIGE* station_neige = static_cast<STATION_NEIGE*>(_stations_neige_feuillus[index_station]);
			
			float ponderation;

			if((_interpolation_feuillus == INTERPOLATION_THIESSEN && _sim_hyd._versionTHIESSEN == 1) || (_interpolation_feuillus == INTERPOLATION_MOYENNE_3_STATIONS && _sim_hyd._versionMOY3STATION == 1))
				ponderation = _ponderation_feuillusF(index_zone, index_station);
			else
				ponderation = static_cast<float>(_ponderation_feuillus(index_zone, index_station));

			if (station_neige->DonneeDisponible(date_courante) && ponderation > 0)
			{				
				auto donnees = station_neige->PrendreDonnee(date_courante);

				if (_maj_feuillus[index_zone].stations_utilisees[index_station] || 
					_maj_feuillus[index_zone].pourcentage_corrige >= 1 ||
					_maj_feuillus[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
				{
					_maj_feuillus[index_zone].nb_pas_derniere_correction = 0;
					
					_maj_feuillus[index_zone].pourcentage_corrige = 0;
					_maj_feuillus[index_zone].pourcentage_sim_eq = 1;
					_maj_feuillus[index_zone].pourcentage_sim_ha = 1;

					for (size_t n = 0; n < _maj_feuillus[index_zone].stations_utilisees.size(); ++n)
						_maj_feuillus[index_zone].stations_utilisees[n] = false;
				}

				_maj_feuillus[index_zone].stations_utilisees[index_station] = true;
				_maj_feuillus[index_zone].pourcentage_corrige += ponderation;

				if (donnees.equi_eau > 0)
				{
					float stock_ref = _stock_feuillus[index_zone];
					float hauteur_ref = _hauteur_feuillus[index_zone];

					float stock = stock_ref;
					
					UneMiseAJourOccupation(
						donnees.equi_eau, 
						stock,
						stock_ref,
						_maj_feuillus[index_zone].pourcentage_corrige,
						ponderation,
						_maj_feuillus[index_zone].pourcentage_sim_eq);

					_stock_feuillus[index_zone] = stock;

					if (stock_ref == 0)
					{
						_hauteur_feuillus[index_zone] = 0;
					}
					else
					{
						_chaleur_feuillus[index_zone] = _chaleur_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_ref;
						_eau_retenu_feuillus[index_zone] = _eau_retenu_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_ref;
					}

					// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
					if (donnees.hauteur <= 0)
					{
						if (stock_ref == 0)
							donnees.hauteur = donnees.equi_eau * 2.5f;
						else
							donnees.hauteur = donnees.equi_eau * hauteur_ref / stock_ref;
					}

					if (_hauteur_feuillus[index_zone] > 0)
					{
						float hauteur = _hauteur_feuillus[index_zone];

						UneMiseAJourOccupation(donnees.hauteur,
							hauteur,
							hauteur_ref,
							_maj_feuillus[index_zone].pourcentage_corrige,
							ponderation,
							_maj_feuillus[index_zone].pourcentage_sim_ha);

						_hauteur_feuillus[index_zone] = hauteur;
					}
					else
					{
						if (_stock_feuillus[index_zone] > 0)
						{
							if (stock_ref == 0)
								_hauteur_feuillus[index_zone] = _stock_feuillus[index_zone] * 2.5f;
							else
								_hauteur_feuillus[index_zone] = hauteur_ref * _stock_feuillus[index_zone] / stock_ref;
						}
					}
				}
			}
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		_maj_decouver[index_zone].nb_pas_derniere_correction++;

		for (size_t index_station = 0; index_station < _stations_neige_decouver.PrendreNbStation(); ++index_station)
		{
			STATION_NEIGE* station_neige = static_cast<STATION_NEIGE*>(_stations_neige_decouver[index_station]);
			
			float ponderation;

			if((_interpolation_decouver == INTERPOLATION_THIESSEN && _sim_hyd._versionTHIESSEN == 1) || (_interpolation_decouver == INTERPOLATION_MOYENNE_3_STATIONS && _sim_hyd._versionMOY3STATION == 1))
				ponderation = _ponderation_decouverF(index_zone, index_station);
			else
				ponderation = static_cast<float>(_ponderation_decouver(index_zone, index_station));

			if (station_neige->DonneeDisponible(date_courante) && ponderation > 0)
			{				
				auto donnees = station_neige->PrendreDonnee(date_courante);

				if (_maj_decouver[index_zone].stations_utilisees[index_station] || 
					_maj_decouver[index_zone].pourcentage_corrige >= 1 ||
					_maj_decouver[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
				{
					_maj_decouver[index_zone].nb_pas_derniere_correction = 0;
					
					_maj_decouver[index_zone].pourcentage_corrige = 0;
					_maj_decouver[index_zone].pourcentage_sim_eq = 1;
					_maj_decouver[index_zone].pourcentage_sim_ha = 1;

					for (size_t n = 0; n < _maj_decouver[index_zone].stations_utilisees.size(); ++n)
						_maj_decouver[index_zone].stations_utilisees[n] = false;
				}

				_maj_decouver[index_zone].stations_utilisees[index_station] = true;
				_maj_decouver[index_zone].pourcentage_corrige += ponderation;

				if (donnees.equi_eau > 0)
				{
					float stock_ref = _stock_decouver[index_zone];
					float hauteur_ref = _hauteur_decouver[index_zone];

					float stock = stock_ref;
					
					UneMiseAJourOccupation(
						donnees.equi_eau, 
						stock,
						stock_ref,
						_maj_decouver[index_zone].pourcentage_corrige,
						ponderation,
						_maj_decouver[index_zone].pourcentage_sim_eq);

					_stock_decouver[index_zone] = stock;

					if (stock_ref == 0)
					{
						_hauteur_decouver[index_zone] = 0;
					}
					else
					{
						_chaleur_decouver[index_zone] = _chaleur_decouver[index_zone] * _stock_decouver[index_zone] / stock_ref;
						_eau_retenu_decouver[index_zone] = _eau_retenu_decouver[index_zone] * _stock_decouver[index_zone] / stock_ref;
					}

					// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
					if (donnees.hauteur <= 0)
					{
						if (stock_ref == 0)
							donnees.hauteur = donnees.equi_eau * 2.5f;
						else
							donnees.hauteur = donnees.equi_eau * hauteur_ref / stock_ref;
					}

					if (_hauteur_decouver[index_zone] > 0)
					{
						float hauteur = _hauteur_decouver[index_zone];

						UneMiseAJourOccupation(donnees.hauteur,
							hauteur,
							hauteur_ref,
							_maj_decouver[index_zone].pourcentage_corrige,
							ponderation,
							_maj_decouver[index_zone].pourcentage_sim_ha);

						_hauteur_decouver[index_zone] = hauteur;
					}
					else
					{
						if (_stock_decouver[index_zone] > 0)
						{
							if (stock_ref == 0)
								_hauteur_decouver[index_zone] = _stock_decouver[index_zone] * 2.5f;
							else
								_hauteur_decouver[index_zone] = hauteur_ref * _stock_decouver[index_zone] / stock_ref;
						}
					}
				}
			}
		}

	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void DEGRE_JOUR_MODIFIE::MiseAJourGrille(size_t index_zone, STATION_NEIGE::typeOccupationStation occupation)
	{
		size_t row, col, nbCol;
		float stock_ref, stock_avant, hauteur_ref, stock;
		float hauteur, hauteur2;	// m
		float equi_eau;				// m
		//float densite;				// pourcentage
		int iIdent, indexcell;
		
		int nb_pas_par_jour = 24 / _sim_hyd.PrendrePasDeTemps();
		
		// conversion d'occupation de sol dominante dans la zone
		stock_ref = 0.0f;
		if (occupation==STATION_NEIGE::RESINEUX)
		{
			stock_ref=_stock_conifers[index_zone];
			hauteur_ref= _hauteur_conifers[index_zone];
		}
		if (occupation==STATION_NEIGE::DECOUVERTE)
		{
			stock_ref=_stock_decouver[index_zone];
			hauteur_ref= _hauteur_decouver[index_zone];
		}
		if (occupation==STATION_NEIGE::FEUILLUS)
		{
			stock_ref=_stock_feuillus[index_zone];
			hauteur_ref= _hauteur_feuillus[index_zone];
		}

		//
		ZONE& zone = _sim_hyd.PrendreZones()[index_zone];
		iIdent = zone.PrendreIdent();

		nbCol = _grilleneige._grilleEquivalentEau[0].PrendreNbColonne();
		
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		_maj_conifers[index_zone].nb_pas_derniere_correction++;

		for (auto iter2 = begin(_grilleneige._mapPonderation[iIdent]); iter2 != end(_grilleneige._mapPonderation[iIdent]); iter2++)
		{
			indexcell = iter2->first;
			row = static_cast<size_t>(indexcell / nbCol);
			col = indexcell % nbCol;

			equi_eau = _grilleneige._grilleEquivalentEau[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;	//conversion -> [m]
			hauteur = _grilleneige._grilleHauteurNeige[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;	//conversion -> [m]

			if(equi_eau > VALEUR_MANQUANTE && hauteur > VALEUR_MANQUANTE && hauteur != 0.0f)
			{
				_maj_conifers[index_zone].pourcentage_corrige+= static_cast<float>(iter2->second);	//iter2->second -> ponderation

				if (_maj_conifers[index_zone].pourcentage_corrige >= 1.0f ||
					_maj_conifers[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
				{
					_maj_conifers[index_zone].nb_pas_derniere_correction = 0;
					
					_maj_conifers[index_zone].pourcentage_corrige = 0.0f;
					_maj_conifers[index_zone].pourcentage_sim_eq = 1.0f;
					_maj_conifers[index_zone].pourcentage_sim_ha = 1.0f;
				}

				if(equi_eau > 0.0f)
				{
					stock = stock_avant = _stock_conifers[index_zone];
					hauteur_ref = _hauteur_conifers[index_zone];
					
					UneMiseAJourOccupation(
						equi_eau, 
						stock,
						stock_ref,
						_maj_conifers[index_zone].pourcentage_corrige,
						static_cast<float>(iter2->second),	//iter2->second -> ponderation
						_maj_conifers[index_zone].pourcentage_sim_eq);

					_stock_conifers[index_zone] = stock;

					if (stock_avant == 0.0f)
						_hauteur_conifers[index_zone] = 0.0f;
					else
					{
						_chaleur_conifers[index_zone] = _chaleur_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
						_eau_retenu_conifers[index_zone] = _eau_retenu_conifers[index_zone] * _stock_conifers[index_zone] / stock_avant;
					}

					// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
					if (hauteur <= 0.0f)
					{
						if (stock_avant == 0.0f)
							hauteur = equi_eau * 2.5f;
						else
							hauteur = equi_eau * hauteur_ref / stock_avant;
					}

					if (_hauteur_conifers[index_zone] > 0.0f)
					{
						hauteur2 = _hauteur_conifers[index_zone];

						UneMiseAJourOccupation(hauteur,
							hauteur2,
							hauteur_ref,
							_maj_conifers[index_zone].pourcentage_corrige,
							static_cast<float>(iter2->second),	//iter2->second -> ponderation
							_maj_conifers[index_zone].pourcentage_sim_ha);

						_hauteur_conifers[index_zone] = hauteur2;
					}
					else
					{
						if (_stock_conifers[index_zone] > 0.0f)
						{
							if (stock_avant == 0.0f)
								_hauteur_conifers[index_zone] = _stock_conifers[index_zone] * 2.5f;
							else
								_hauteur_conifers[index_zone] = hauteur_ref * _stock_conifers[index_zone] / stock_avant;
						}
					}
				}
			}
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		_maj_feuillus[index_zone].nb_pas_derniere_correction++;

		for (auto iter2 = begin(_grilleneige._mapPonderation[iIdent]); iter2 != end(_grilleneige._mapPonderation[iIdent]); iter2++)
		{
			indexcell = iter2->first;
			row = static_cast<size_t>(indexcell / nbCol);
			col = indexcell % nbCol;

			equi_eau = _grilleneige._grilleEquivalentEau[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;
			hauteur = _grilleneige._grilleHauteurNeige[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;

			if(equi_eau > VALEUR_MANQUANTE && hauteur > VALEUR_MANQUANTE && hauteur != 0.0f)
			{
				_maj_feuillus[index_zone].pourcentage_corrige+= static_cast<float>(iter2->second);	//iter2->second -> ponderation

				if (_maj_feuillus[index_zone].pourcentage_corrige >= 1.0f ||
					_maj_feuillus[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
				{
					_maj_feuillus[index_zone].nb_pas_derniere_correction = 0;
					
					_maj_feuillus[index_zone].pourcentage_corrige = 0.0f;
					_maj_feuillus[index_zone].pourcentage_sim_eq = 1.0f;
					_maj_feuillus[index_zone].pourcentage_sim_ha = 1.0f;
				}

				if(equi_eau > 0.0f)
				{
					stock = stock_avant = _stock_feuillus[index_zone];
					hauteur_ref = _hauteur_feuillus[index_zone];
					
					UneMiseAJourOccupation(
						equi_eau, 
						stock,
						stock_ref,
						_maj_feuillus[index_zone].pourcentage_corrige,
						static_cast<float>(iter2->second),	//iter2->second -> ponderation
						_maj_feuillus[index_zone].pourcentage_sim_eq);

					_stock_feuillus[index_zone] = stock;

					if (stock_avant == 0.0f)
					{
						_hauteur_feuillus[index_zone] = 0.0f;
					}
					else
					{
						_chaleur_feuillus[index_zone] = _chaleur_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
						_eau_retenu_feuillus[index_zone] = _eau_retenu_feuillus[index_zone] * _stock_feuillus[index_zone] / stock_avant;
					}

					// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
					if (hauteur <= 0.0f)
					{
						if (stock_avant == 0.0f)
							hauteur = equi_eau * 2.5f;
						else
							hauteur = equi_eau * hauteur_ref / stock_avant;
					}

					if (_hauteur_feuillus[index_zone] > 0.0f)
					{
						hauteur2 = _hauteur_feuillus[index_zone];

						UneMiseAJourOccupation(hauteur,
							hauteur2,
							hauteur_ref,
							_maj_feuillus[index_zone].pourcentage_corrige,
							static_cast<float>(iter2->second),	//iter2->second -> ponderation
							_maj_feuillus[index_zone].pourcentage_sim_ha);

						_hauteur_feuillus[index_zone] = hauteur2;
					}
					else
					{
						if (_stock_feuillus[index_zone] > 0.0f)
						{
							if (stock_avant == 0.0f)
								_hauteur_feuillus[index_zone] = _stock_feuillus[index_zone] * 2.5f;
							else
								_hauteur_feuillus[index_zone] = hauteur_ref * _stock_feuillus[index_zone] / stock_avant;
						}
					}
				}
			}
		}

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		_maj_decouver[index_zone].nb_pas_derniere_correction++;

		for (auto iter2 = begin(_grilleneige._mapPonderation[iIdent]); iter2 != end(_grilleneige._mapPonderation[iIdent]); iter2++)
		{
			indexcell = iter2->first;
			row = static_cast<size_t>(indexcell / nbCol);
			col = indexcell % nbCol;

			equi_eau = _grilleneige._grilleEquivalentEau[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;
			hauteur = _grilleneige._grilleHauteurNeige[0](row, col) * _grilleneige._facteurMultiplicatifDonnees;

			if(equi_eau > VALEUR_MANQUANTE && hauteur > VALEUR_MANQUANTE && hauteur != 0.0f)
			{
				_maj_decouver[index_zone].pourcentage_corrige+= static_cast<float>(iter2->second);	//iter2->second -> ponderation

				if (_maj_decouver[index_zone].pourcentage_corrige >= 1.0f ||
					_maj_decouver[index_zone].nb_pas_derniere_correction >= (_nbr_jour_delai_mise_a_jour * nb_pas_par_jour))
				{
					_maj_decouver[index_zone].nb_pas_derniere_correction = 0;
					
					_maj_decouver[index_zone].pourcentage_corrige = 0.0f;
					_maj_decouver[index_zone].pourcentage_sim_eq = 1.0f;
					_maj_decouver[index_zone].pourcentage_sim_ha = 1.0f;
				}

				if(equi_eau > 0.0f)
				{
					stock = stock_avant = _stock_decouver[index_zone];
					hauteur_ref = _hauteur_decouver[index_zone];
					
					UneMiseAJourOccupation(
						equi_eau, 
						stock,
						stock_ref,
						_maj_decouver[index_zone].pourcentage_corrige,
						static_cast<float>(iter2->second),	//iter2->second -> ponderation
						_maj_decouver[index_zone].pourcentage_sim_eq);

					_stock_decouver[index_zone] = stock;

					if (stock_avant == 0.0f)
						_hauteur_decouver[index_zone] = 0.0f;
					else
					{
						_chaleur_decouver[index_zone] = _chaleur_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
						_eau_retenu_decouver[index_zone] = _eau_retenu_decouver[index_zone] * _stock_decouver[index_zone] / stock_avant;
					}

					// si la hauteur d'eau observee est manquante, on la remplace par l'equivalent en eau observe multiplie par la densite simulee.
					if (hauteur <= 0.0f)
					{
						if (stock_avant == 0.0f)
							hauteur = equi_eau * 2.5f;
						else
							hauteur = equi_eau * hauteur_ref / stock_avant;
					}

					if (_hauteur_decouver[index_zone] > 0.0f)
					{
						hauteur2 = _hauteur_decouver[index_zone];

						UneMiseAJourOccupation(hauteur,
							hauteur2,
							hauteur_ref,
							_maj_decouver[index_zone].pourcentage_corrige,
							static_cast<float>(iter2->second),	//iter2->second -> ponderation
							_maj_decouver[index_zone].pourcentage_sim_ha);

						_hauteur_decouver[index_zone] = hauteur2;
					}
					else
					{
						if (_stock_decouver[index_zone] > 0.0f)
						{
							if (stock_avant == 0.0f)
								_hauteur_decouver[index_zone] = _stock_decouver[index_zone] * 2.5f;
							else
								_hauteur_decouver[index_zone] = hauteur_ref * _stock_decouver[index_zone] / stock_avant;
						}
					}
				}
			}
		}
	}

	void DEGRE_JOUR_MODIFIE::UneMiseAJourOccupation(float mesure, float& valeur_actuelle, float valeur_ref, float fraction_corrigee, float ponderation, float &fraction_simulee)
	{
		float frac = min(fraction_corrigee, 1.0f);
		
		// ajustement de la mesure en fonction de l'occupation du sol
		float mesure_ajustee = max(mesure - (valeur_ref - valeur_actuelle), 0.0f);

		// contibution relative des observations et des simulations
		float contr_obs = (1.0f - fraction_simulee) * valeur_actuelle + ponderation * mesure_ajustee;
		float contr_sim = (fraction_simulee * valeur_actuelle * (1.0f - frac)) / ((1.0f - frac) + ponderation);

		// mise a jour 
		valeur_actuelle = contr_obs + contr_sim;

		//calcul de la fraction simulee
		if (valeur_actuelle != 0)
		{
			fraction_simulee = contr_sim / valeur_actuelle;
		}
		else
		{
			fraction_simulee = 1.0f;
		}
	}

	float DEGRE_JOUR_MODIFIE::ConductiviteNeige(float densite)
	{
		float d0 = 0.36969f;
		float d1 = 1.58688e-03f;
		float d2 = 3.02462e-06f;
		float d3 = 5.19756e-09f;
		float d4 = 1.56984e-11f;
		float p0 = 1.0f;

		float p1 = densite - 329.6f;
		float p2 = ((densite - 260.378f) * p1) - (21166.4f * p0);
		float p3 = ((densite - 320.69f) * p2) - (24555.8f * p1);
		float p4 = ((densite - 263.363f) * p3) - (11739.3f * p2);

		return d0 * p0 + d1 * p1 + d2 * p2 + d3 * p3 + d4 * p4;
	}

	float DEGRE_JOUR_MODIFIE::Erf(float x)
	{
		BOOST_ASSERT(x >= 0);

		// approximation rationnelle
		float t = 1.0f / (1.0f + 0.47047f * x);
		return 1.0f -(0.3480242f * t - 0.0958798f* t * t + 0.7478556f * t * t * t) * exp(-x * x);
	}

	float DEGRE_JOUR_MODIFIE::PrendreTauxFonte(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte.size());
		return _taux_fonte[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreDensiteMaximal(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _densite_maximale.size());
		return _densite_maximale[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreConstanteTassement(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _constante_tassement.size());
		return _constante_tassement[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreSeuilFonteFeuillus(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_feuillus.size());
		return _seuil_fonte_feuillus[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreSeuilFonteConifers(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_conifers.size());
		return _seuil_fonte_conifers[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreSeuilFonteDecouver(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _seuil_fonte_decouver.size());
		return _seuil_fonte_decouver[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreTauxFonteFeuillus(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte_feuillus.size());
		return _taux_fonte_feuillus[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreTauxFonteConifers(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte_conifers.size());
		return _taux_fonte_conifers[index_zone];
	}

	float DEGRE_JOUR_MODIFIE::PrendreTauxFonteDecouver(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _taux_fonte_decouver.size());
		return _taux_fonte_decouver[index_zone];
	}

	vector<size_t> DEGRE_JOUR_MODIFIE::PrendreIndexOccupationFeuillus() const
	{
		return _index_occupation_feuillus;
	}

	vector<size_t> DEGRE_JOUR_MODIFIE::PrendreIndexOccupationConifers() const
	{
		return _index_occupation_conifers;
	}


	void DEGRE_JOUR_MODIFIE::LectureEtat(DATE_HEURE date_courante)
	{
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; fichier etat DEGRE_JOUR_MODIFIE; " + _nom_fichier_lecture_etat);

		vector<int> vValidation;
		string ligne;
		size_t index_zone;
		int iIdent;

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);

		while(!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne != "")
			{
				auto valeurs = extrait_fvaleur(ligne, _pOutput->_sFichiersEtatsSeparator);
				iIdent = static_cast<int>(valeurs[0]);

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

					_stock_conifers[index_zone] = valeurs[1];
					_stock_feuillus[index_zone] = valeurs[2];
					_stock_decouver[index_zone] = valeurs[3];

					_hauteur_conifers[index_zone] = valeurs[4];
					_hauteur_feuillus[index_zone] = valeurs[5];
					_hauteur_decouver[index_zone] = valeurs[6];

					_chaleur_conifers[index_zone] = valeurs[7];
					_chaleur_feuillus[index_zone] = valeurs[8];
					_chaleur_decouver[index_zone] = valeurs[9];

					_eau_retenu_conifers[index_zone] = valeurs[10];
					_eau_retenu_feuillus[index_zone] = valeurs[11];
					_eau_retenu_decouver[index_zone] = valeurs[12];

					_albedo_conifers[index_zone] = valeurs[13];
					_albedo_feuillus[index_zone] = valeurs[14];
					_albedo_decouver[index_zone] = valeurs[15];

					vValidation.push_back(iIdent);
				}
			}
		}

		fichier.close();

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; fichier etat DEGRE_JOUR_MODIFIE; id mismatch; " + _nom_fichier_lecture_etat);
	}


	void DEGRE_JOUR_MODIFIE::SauvegardeEtat(DATE_HEURE date_courante) const
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier, oss;
		string sSep = _pOutput->_sFichiersEtatsSeparator;
		size_t x, nbSimuler, index_zone;

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

		fichier << "UHRH" << sSep 
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

				oss.str("");
				oss << zone.PrendreIdent() << sSep;

				oss << setprecision(12) << setiosflags(ios::fixed);
			
				oss << _stock_conifers[index_zone] << sSep;
				oss << _stock_feuillus[index_zone] << sSep;
				oss << _stock_decouver[index_zone] << sSep;

				oss << _hauteur_conifers[index_zone] << sSep;
				oss << _hauteur_feuillus[index_zone] << sSep;
				oss << _hauteur_decouver[index_zone] << sSep;

				oss << _chaleur_conifers[index_zone] << sSep;
				oss << _chaleur_feuillus[index_zone] << sSep;
				oss << _chaleur_decouver[index_zone] << sSep;

				oss << _eau_retenu_conifers[index_zone] << sSep;
				oss << _eau_retenu_feuillus[index_zone] << sSep;
				oss << _eau_retenu_decouver[index_zone] << sSep;

				oss << _albedo_conifers[index_zone] << sSep;
				oss << _albedo_feuillus[index_zone] << sSep;
				oss << _albedo_decouver[index_zone];

				fichier << oss.str() << endl;
				++x;
			}
		}

		fichier.close();
	}

	void DEGRE_JOUR_MODIFIE::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_MODIFIE");

		try
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			string cle, valeur, ligne;
			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_MODIFIE", 1);

			getline_mod(fichier, ligne);
			lire_cle_valeur(fichier, cle, valeur);
			getline_mod(fichier, ligne);

			// lecture des classes integrees

			lire_cle_valeur(fichier, cle, valeur);
			ChangeIndexOccupationConifers( extrait_valeur(valeur) );

			lire_cle_valeur(fichier, cle, valeur);
			ChangeIndexOccupationFeuillus( extrait_valeur(valeur) );

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
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_MODIFIE");

			lire_cle_valeur(fichier, cle, ligne);
			if (ligne.empty())
				_interpolation_feuillus = INTERPOLATION_AUCUNE;
			else if (ligne == "THIESSEN")
				_interpolation_feuillus = INTERPOLATION_THIESSEN;
			else if (ligne == "MOYENNE 3 STATIONS")
				_interpolation_feuillus = INTERPOLATION_MOYENNE_3_STATIONS;
			else
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_MODIFIE");

			lire_cle_valeur(fichier, cle, ligne);
			if (ligne.empty())
				_interpolation_decouver = INTERPOLATION_AUCUNE;
			else if (ligne == "THIESSEN")
				_interpolation_decouver = INTERPOLATION_THIESSEN;
			else if (ligne == "MOYENNE 3 STATIONS")
				_interpolation_decouver = INTERPOLATION_MOYENNE_3_STATIONS;
			else
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_MODIFIE");

			getline_mod(fichier, ligne); //vide

			//
			_bMAJGrilleNeige = false;	//valeur par defaut

			lire_cle_valeur(fichier, cle, ligne);
			if(cle == "MISE A JOUR GRILLE NEIGE")
			{
				if(ligne == "1")
					_bMAJGrilleNeige = true;
				
				//
				lire_cle_valeur(fichier, cle, ligne);
				if(cle == "NOM FICHIER GRILLE NEIGE")
				{
					if (!ligne.empty() && !Racine(ligne))
						ligne = Combine(_sim_hyd.PrendreRepertoireProjet(), ligne);
					_grilleneige._sPathFichierParam = ligne;

					getline_mod(fichier, ligne); //vide
					getline_mod(fichier, ligne); // commentaire
				}
				else
					throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_MODIFIE");
			}
			else
			{			
				if(cle == "NOM FICHIER GRILLE NEIGE")
				{
					if (!ligne.empty() && !Racine(ligne))
					{
						ligne = Combine(_sim_hyd.PrendreRepertoireProjet(), ligne);
						_bMAJGrilleNeige = true;
					}
					_grilleneige._sPathFichierParam = ligne;

					getline_mod(fichier, ligne); //vide
					getline_mod(fichier, ligne); // commentaire
				}
			}

			// lecture des parametres
			for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
			{
				int ident;
				float taux_fonte, densite_max, tassement, 
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

				//_methode_albedo[index_zone] = 0;
			}

			fichier.close();

		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_MODIFIE");
		}
	}

	void DEGRE_JOUR_MODIFIE::LectureParametresFichierGlobal()
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

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "DEGRE JOUR MODIFIE")
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
				if(ligne == "1")
					_bMAJGrilleNeige = true;
				else
					_bMAJGrilleNeige = false;

				++no_ligne;
				getline_mod(fichier, ligne);
				if (!ligne.empty() && !Racine(ligne))
					ligne = Combine(_sim_hyd.PrendreRepertoireProjet(), ligne);
				_grilleneige._sPathFichierParam = ligne;

				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 11)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide. DEGRE JOUR MODIFIE");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. DEGRE JOUR MODIFIE. Les ID de groupe doivent etre en ordre croissant.");

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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; DEGRE JOUR MODIFIE; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele DEGRE JOUR MODIFIE");
	}


	void DEGRE_JOUR_MODIFIE::SauvegardeParametres()
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

		fichier << "MISE A JOUR GRILLE NEIGE;0" << endl;
		fichier << "NOM FICHIER GRILLE NEIGE;" << endl;
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

	void DEGRE_JOUR_MODIFIE::ChangeNomFichierLectureEtat(std::string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}

	void DEGRE_JOUR_MODIFIE::ChangeRepertoireEcritureEtat(std::string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}

	void DEGRE_JOUR_MODIFIE::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}

	void DEGRE_JOUR_MODIFIE::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}

	string DEGRE_JOUR_MODIFIE::PrendreNomFichierLectureEtat() const
	{
		return _nom_fichier_lecture_etat;
	}

	string DEGRE_JOUR_MODIFIE::PrendreRepertoireEcritureEtat() const
	{
		return _repertoire_ecriture_etat;
	}

	bool DEGRE_JOUR_MODIFIE::PrendreSauvegardeTousEtat() const
	{
		return _sauvegarde_tous_etat;
	}

	DATE_HEURE DEGRE_JOUR_MODIFIE::PrendreDateHeureSauvegardeEtat() const
	{
		return _date_sauvegarde_etat;
	}

}
