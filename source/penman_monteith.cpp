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

#include "penman_monteith.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>


using namespace std;


namespace HYDROTEL
{

	PENMAN_MONTEITH::PENMAN_MONTEITH(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "PENMAN-MONTEITH")
	{
		
	}


	PENMAN_MONTEITH::~PENMAN_MONTEITH()
	{
	}


	void PENMAN_MONTEITH::ChangeNbParams(const ZONES& zones)
	{
		const size_t nbUHRH = zones.PrendreNbZone();

		_fHauteurMesureVitesseVent.resize(nbUHRH, 2.0f);		//m
		_fHauteurMesureHumidite.resize(nbUHRH, 2.0f);			//m
		_fVitesseVent.resize(nbUHRH, 2.0f);						//m/s
		_fHauteurVegetation.resize(nbUHRH, 0.12f);				//m
		_fResistanceStomatale.resize(nbUHRH, 100.0f);			//s/m

		_rayonnementNet = &_sim_hyd._rayonnementNet;
		if(_rayonnementNet == NULL)
			throw ERREUR("PENMAN-MONTEITH; _rayonnementNet == NULL");

		EVAPOTRANSPIRATION::ChangeNbParams(zones);
	}


	void PENMAN_MONTEITH::Initialise()
	{
		EVAPOTRANSPIRATION::Initialise();
	}


	void PENMAN_MONTEITH::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();

		ostringstream oss;
		DATE_HEURE dtCourant;
		size_t index, index_zone, index_classe;
		float fRn, fra, fRs, fGamma, fPa, fEs, fDelta, fG, fET;
		float fd, fZom, fZoh, fLAI, fLAIactive, fP, fTkv, fT, fe_tmin, fe_tmax, fe_tmoy;
		float fPourcentage, fETPpond;
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

			//résistance aérodynamique (ra) [s/m]
			//hauteur à laquelle on considere le vent comme nul (0.08) [m]
			fd = 2.0f / 3.0f * _fHauteurVegetation[index_zone];

			//hauteur de déplacement vertical pour la quantite de mouvement	reliee à la rugosite de la surface (0.015) [m]
			fZom = 0.123f * _fHauteurVegetation[index_zone];

			//hauteur de deplacement vertical dans le cas de la vapeur d eau reliee a la rugosite de la surface (0.0015) [m]
			fZoh = 0.1f * fZom;

			fra = log((_fHauteurMesureVitesseVent[index_zone] - fd) / fZom) * log((_fHauteurMesureHumidite[index_zone] - fd) / fZoh) / (pow(K, 2.0f) * _fVitesseVent[index_zone]);

			//résistance de la surface (Rs) [s/m];
			//indice foliaire [m2 (surface de feuille) m-2 (surface du sol)]
			fLAI = 24.0f * _fHauteurVegetation[index_zone];
			fLAIactive = 0.5f * fLAI;

			fRs = _fResistanceStomatale[index_zone] / fLAIactive;

			//rayonnement net à la surface (Rn) [MJ/m2/Jour]
			fRn = _rayonnementNet->PrendreRayonnementNet(iJourJulien, index_zone);

			//constante psychrométrique (Gamma) [kPa/dC]
			//pression atmosphérique (P) [kPa];
			fP = 101.3f * pow( ((293.0f - 0.0065f * zone.PrendreAltitude()) / 293.0f), 5.26f);

			fGamma = 0.000665f * fP;

			//densite moyenne de l air pour une pression constante (Pa) [kg/m3]			
			//temperature moyenne de l'air [dC]
			fT = (zone.PrendreTMax() + zone.PrendreTMin()) / 2.0f;
			
			//temperature virtuelle en fonction de la temperature
			fTkv = 1.01f * (fT + 273.15f);

			fPa = fP / (fTkv * (RD / 1000.0f));	//(RD / 1000.0f); constante spécifique des gaz [KJ/kg*K]

			//tension de vapeur a saturation moyenne (Es) [kPa]
			//estimation de la tension de vapeur d'eau [kPa]
			fe_tmin = 0.6108f * exp(17.27f * zone.PrendreTMin() / (zone.PrendreTMin() + 237.3f));	//Ea
			fe_tmax = 0.6108f * exp(17.27f * zone.PrendreTMax() / (zone.PrendreTMax() + 237.3f));

			fEs = (fe_tmax + fe_tmin) / 2.0f;

			//pente de la tension de vapeur d’eau saturante en fonction de la température (Delta) [kPa/dC]
			fe_tmoy = 0.6108f * exp(17.27f * fT / (fT + 237.3f));

			fDelta = 4098.0f * fe_tmoy / pow(fT + 237.3f, 2.0f);

			//flux de chaleur du sol (MJ m-2 Jour-1);
			fG = 0.0f;	//en raison de la faible importance du flux de chaleur du sol en comparaison avec le rayonnement net, 
						//et ce plus particulièrement en présence de vegetation, il est juge acceptable de negliger ce parametre dans le calcul de ET (G=0)

			//evapotranspiration journalière [mm/Jour]
			fET = fDelta * (fRn - fG) / (CHALEUR_LATENTE_VAPORISATION * (fDelta + fGamma * (1.0f + fRs / fra))) + fPa * (CHALEUR_SPECIFIQUE_A_PRESSION_CONSTANTE) * ((fEs - fe_tmin) / fra) / (CHALEUR_LATENTE_VAPORISATION * (fDelta + fGamma * (1.0f + fRs / fra))) * _sim_hyd.PrendrePasDeTemps() * 60.0f * 60.0f;

			//ponderation sur les differentes classe d'occupation du sol
			for (index_classe = 0; index_classe < nb_classe; index_classe++)
			{
				fPourcentage = occupation_sol.PrendrePourcentage(index_zone, index_classe);
				fETPpond = fET * fPourcentage * _coefficients_multiplicatif[index_zone];

				zone.ChangeEtp(index_classe, max(fETPpond, 0.0f));	//[mm]
			}
		}

		EVAPOTRANSPIRATION::Calcule();
	}


	void PENMAN_MONTEITH::Termine()
	{
		EVAPOTRANSPIRATION::Termine();
	}


	void PENMAN_MONTEITH::LectureParametres()
	{
		string cle, valeur, ligne;
		size_t index_zone, compteur;
		int iIdent;

		ZONES& zones = _sim_hyd.PrendreZones();
		int no_ligne = 6;

		if(_sim_hyd.PrendreNomEvapotranspiration() == PrendreNomSousModele())	//si le modele est simulé
		{
			if(_sim_hyd.PrendrePasDeTemps() != 24)
				throw ERREUR("PENMAN-MONTEITH; le pas de temps de la simulation est invalide; le pas de temps doit etre 24h; limitation du modele de calcul du rayonnement net à la surface.");

			if(_sim_hyd._fichierParametreGlobal)
			{
				LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
				return;
			}

			ifstream fichier( PrendreNomFichierParametres() );
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES PENMAN-MONTEITH");

			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES PENMAN-MONTEITH", 1);
			}

			getline_mod(fichier, ligne);
			lire_cle_valeur(fichier, cle, valeur); //nom sous modele
			getline_mod(fichier, ligne);

			getline_mod(fichier, ligne); //entete

			for (compteur=0; compteur<zones.PrendreNbZone(); compteur++)
			{
				getline_mod(fichier, ligne);
				auto vValeur = extrait_fvaleur(ligne, ";");

				if(vValeur.size() != 7)
				{
					fichier.close();
					throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "PENMAN-MONTEITH; Nombre de colonne invalide.");
				}

				iIdent = static_cast<int>(vValeur[0]);
				index_zone = zones.IdentVersIndex(iIdent);

				_fHauteurMesureVitesseVent[index_zone] = vValeur[1];	//m
				_fHauteurMesureHumidite[index_zone] = vValeur[2];		//m
				_fVitesseVent[index_zone] = vValeur[3];					//m/s
				_fHauteurVegetation[index_zone] = vValeur[4];			//m
				_fResistanceStomatale[index_zone] = vValeur[5];			//s/m
				ChangeCoefficientMultiplicatif(index_zone, vValeur[6]);

				++no_ligne;
			}

			fichier.close();
		}
	}


	void PENMAN_MONTEITH::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL;" + _sim_hyd._nomFichierParametresGlobal );

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
			if(ligne == "PENMAN-MONTEITH")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 7)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "PENMAN-MONTEITH; Nombre de colonne invalide.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "PENMAN-MONTEITH; ID de groupe invalide. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_fHauteurMesureVitesseVent[index_zone] = vValeur[1];
						_fHauteurMesureHumidite[index_zone] = vValeur[2];
						_fVitesseVent[index_zone] = vValeur[3];
						_fHauteurVegetation[index_zone] = vValeur[4];
						_fResistanceStomatale[index_zone] = vValeur[5];						
						ChangeCoefficientMultiplicatif(index_zone, vValeur[6]);
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; PENMAN-MONTEITH; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele PENMAN-MONTEITH");
	}


	void PENMAN_MONTEITH::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		if(PrendreNomFichierParametres() == "")	//creation d'un fichier par defaut s'il n'existe pas
			ChangeNomFichierParametres(Combine(_sim_hyd.PrendreRepertoireSimulation(), "penman_monteith.csv"));

		string nom_fichier = PrendreNomFichierParametres();
		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "UHRH ID;"
				   " HAUTEUR A LAQUELLE LA VITESSE DU VENT EST MESUREE (2m);"
				   " HAUTEUR A LAQUELLE L'HUMIDITE EST MESUREE (2m);"
				   " VITESSE DU VENT A LA HAUTEUR Z (m/s);"
				   " HAUTEUR DE LA VEGETATION (0.12 SURFACE REFERENCE) (m);"
				   " RESISTANCE STOMATALE (100 SURFACE REFERENCE) (s/m);"
				   " COEFFICIENT MULTIPLICATIF OPTIMISATION" << endl;

		for (size_t index=0; index<zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';

			fichier << _fHauteurMesureVitesseVent[index] << ';';
			fichier << _fHauteurMesureHumidite[index] << ';';
			fichier << _fVitesseVent[index] << ';';
			fichier << _fHauteurVegetation[index] << ';';
			fichier << _fResistanceStomatale[index] << ';';
			fichier << PrendreCoefficientMultiplicatif(index) << endl;
		}

		fichier.close();
	}

}
