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

#include "penman.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>


using namespace std;


namespace HYDROTEL
{

	PENMAN::PENMAN(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "PENMAN")
	{
	}


	PENMAN::~PENMAN()
	{
	}


	void PENMAN::ChangeNbParams(const ZONES& zones)
	{
		const size_t nbUHRH = zones.PrendreNbZone();

		_hauteur_mesure_vent.resize(nbUHRH, 2.0f);						//m
		_fVitesseVent.resize(nbUHRH, 2.0f);								//m/s
		_fHauteurVegetation.resize(nbUHRH, 0.12f);						//m
		_resistance_aerodynamique.resize(nbUHRH, RELATION_EMPIRIQUE);

		_rayonnementNet = &_sim_hyd._rayonnementNet;
		if(_rayonnementNet == NULL)
			throw ERREUR("PENMAN; _rayonnementNet == NULL");

		EVAPOTRANSPIRATION::ChangeNbParams(zones);
	}


	void PENMAN::Initialise()
	{
		EVAPOTRANSPIRATION::Initialise();
	}


	void PENMAN::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();
		float fPasTemps = static_cast<float>(_sim_hyd.PrendrePasDeTemps());

		ostringstream oss;
		DATE_HEURE dtCourant;
		size_t index, index_zone, index_classe;
		float fRn, ea1, fd, fZom, fZoh, fP, fGamma, fEs, fDelta, fEA, fT, fe_tmin, fe_tmax, fe_tmoy;
		float fPourcentage, fETPpond, fETP;
		int iJourJulien;

		const size_t nb_classe = occupation_sol.PrendreNbClasse();

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		dtCourant = _sim_hyd.PrendreDateCourante();
		iJourJulien = dtCourant.PrendreJourJulien();

		//si c'est une annee bissextile, on fait le jour 59 (28 fevrier) 2 fois
		if(dtCourant.Bissextile() && iJourJulien > 59)
			--iJourJulien;

		//pour chaque uhrh simulé
		for (index = 0; index < index_zones.size(); ++index)
		{
			index_zone = index_zones[index];
			ZONE& zone = zones[index_zone];

			//constante psychrométrique (Gamma) [kPa/dC]
			//pression atmosphérique (P) [kPa];
			fP = 101.3f * pow( ((293.0f - 0.0065f * zone.PrendreAltitude()) / 293.0f), 5.26f);

			fGamma = 0.000665f * fP;

			//tension de vapeur a saturation moyenne (Es) [kPa]
			//estimation de la tension de vapeur d'eau [kPa]
			fe_tmin = 0.6108f * exp(17.27f * zone.PrendreTMin() / (zone.PrendreTMin() + 237.3f));	//Ea
			fe_tmax = 0.6108f * exp(17.27f * zone.PrendreTMax() / (zone.PrendreTMax() + 237.3f));

			fEs = (fe_tmax + fe_tmin) / 2.0f;

			//temperature moyenne de l'air [dC]
			fT = (zone.PrendreTMax() + zone.PrendreTMin()) / 2.0f;

			//pente de la tension de vapeur d’eau saturante en fonction de la température (Delta) [kPa/dC]
			fe_tmoy = 0.6108f * exp(17.27f * fT / (fT + 237.3f));
			fDelta = 4098.0f * fe_tmoy / pow(fT + 237.3f, 2.0f);

			//résistance aérodynamique (ra) [s/m]
			//hauteur à laquelle on considere le vent comme nul (0.08) [m]
			fd = 2.0f / 3.0f * _fHauteurVegetation[index_zone];

			//hauteur de déplacement vertical pour la quantite de mouvement	reliee à la rugosite de la surface (0.015) [m]
			fZom = 0.123f * _fHauteurVegetation[index_zone];

			//hauteur de deplacement vertical dans le cas de la vapeur d eau reliee a la rugosite de la surface (0.0015) [m]
			fZoh = 0.1f * fZom;

			//pouvoir évaporant de l'air (ea) [MJ/j/kPa/dC]
			switch (_resistance_aerodynamique[index_zone])
			{
			case RELATION_EMPIRIQUE:
				fEA = fGamma * CHALEUR_LATENTE_VAPORISATION * 2.6f * (1.0f + 0.54f * _fVitesseVent[index_zone]) * (fEs - fe_tmin);
				break;

			case RELATION_BASE_PHYSIQUE:
				ea1 = RHO * CHALEUR_SPECIFIQUE_A_PRESSION_CONSTANTE * K2 * (fEs - fe_tmin) * (3600.0f * fPasTemps);
				fEA = (ea1 * _fVitesseVent[index_zone] / (log((_hauteur_mesure_vent[index_zone] - fd) / fZom) * log((_hauteur_mesure_vent[index_zone] - fd) / fZoh)));
				break;

			default:
				throw ERREUR("PENMAN; fonction resistence aerodynamique invalide");
			}
			
			//rayonnement net à la surface (Rn) [MJ/m2/Jour]
			fRn = _rayonnementNet->PrendreRayonnementNet(iJourJulien, index_zone);
			
			//evapotranspiration maximale des vegetaux et evaporation de l'eau en mm/j
			fETP = ((fDelta * fRn + fEA) / (fDelta + fGamma)) * 1.0f / CHALEUR_LATENTE_VAPORISATION;

			//ponderation sur les differentes classe d'occupation du sol
			for (index_classe = 0; index_classe < nb_classe; index_classe++)
			{
				fPourcentage = occupation_sol.PrendrePourcentage(index_zone, index_classe);
				fETPpond = fETP * fPourcentage * _coefficients_multiplicatif[index_zone];

				zone.ChangeEtp(index_classe, max(fETPpond, 0.0f));	//[mm]
			}
		}

		EVAPOTRANSPIRATION::Calcule();
	}


	void PENMAN::Termine()
	{
		EVAPOTRANSPIRATION::Termine();
	}


	void PENMAN::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES PENMAN");

		size_t index_zone, index;
		string cle, valeur, ligne;
		int iIdent;

		int no_ligne = 6;

		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES PENMAN", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur); //nom sous modele
		getline_mod(fichier, ligne);

		getline_mod(fichier, ligne); //entete

		for (index = 0; index < zones.PrendreNbZone(); ++index)
		{
			getline_mod(fichier, ligne);
			auto vValeur = extrait_fvaleur(ligne, ";");

			if(vValeur.size() != 6)
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "PENMAN; Nombre de colonne invalide.");
			}

			iIdent = static_cast<int>(vValeur[0]);
			index_zone = zones.IdentVersIndex(iIdent);

			_hauteur_mesure_vent[index_zone] = vValeur[1];	//m
			_fVitesseVent[index_zone] = vValeur[2];			//m/s
			_fHauteurVegetation[index_zone] = vValeur[3];	//m

			if(vValeur[4] == 1.0f)
				_resistance_aerodynamique[index_zone] = RELATION_BASE_PHYSIQUE;
			else
				_resistance_aerodynamique[index_zone] = RELATION_EMPIRIQUE;

			ChangeCoefficientMultiplicatif(index_zone, vValeur[5]);

			++no_ligne;
		}
	}


	void PENMAN::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

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
			if(ligne == "PENMAN")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 6)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide. PENMAN.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. PENMAN. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_hauteur_mesure_vent[index_zone] = vValeur[1];	//m
						_fVitesseVent[index_zone] = vValeur[2];			//m/s
						_fHauteurVegetation[index_zone] = vValeur[3];	//m

						if(vValeur[4] == 1.0f)
							_resistance_aerodynamique[index_zone] = RELATION_BASE_PHYSIQUE;
						else
							_resistance_aerodynamique[index_zone] = RELATION_EMPIRIQUE;

						ChangeCoefficientMultiplicatif(index_zone, vValeur[5]);
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; PENMAN; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele PENMAN");
	}


	void PENMAN::SauvegardeParametres()
	{
		string nom_fichier = PrendreNomFichierParametres();

		ZONES& zones = _sim_hyd.PrendreZones();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "UHRH ID; HAUTEUR A LAQUELLE LA VITESSE DU VENT EST MESUREE (2m); VITESSE DU VENT A LA HAUTEUR Z (m/s); HAUTEUR DE LA VEGETATION (0.12 SURFACE REFERENCE) (m); RELATION RESISTANCE AERODYNAMIQUE (0=empirique,1=physique); COEFFICIENT MULTIPLICATIF D'OPTIMISATION" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';

			fichier << _hauteur_mesure_vent[index] << ';';
			fichier << _fVitesseVent[index] << ';';
			fichier << _fHauteurVegetation[index] << ';';
			fichier << _resistance_aerodynamique[index] << ';';
			fichier << PrendreCoefficientMultiplicatif(index) << endl;
		}
	}

}
