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

#include "priestlay_taylor.hpp"

#include "constantes.hpp"
#include "donnee_meteo.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>


using namespace std;


namespace HYDROTEL
{

	PRIESTLAY_TAYLOR::PRIESTLAY_TAYLOR(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "PRIESTLAY-TAYLOR")
	{
	}


	PRIESTLAY_TAYLOR::~PRIESTLAY_TAYLOR()
	{
	}


	void PRIESTLAY_TAYLOR::ChangeNbParams(const ZONES& zones)
	{
		const size_t nbUHRH = zones.PrendreNbZone();

		_coefficient_proportionnalite_alpha.resize(nbUHRH, 1.26f);

		_rayonnementNet = &_sim_hyd._rayonnementNet;
		if(_rayonnementNet == NULL)
			throw ERREUR("PRIESTLAY-TAYLOR; _rayonnementNet == NULL");

		EVAPOTRANSPIRATION::ChangeNbParams(zones);
	}


	void PRIESTLAY_TAYLOR::Initialise()
	{
		EVAPOTRANSPIRATION::Initialise();
	}


	void PRIESTLAY_TAYLOR::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();

		ostringstream oss;
		DATE_HEURE dtCourant;
		size_t index, index_zone, index_classe;
		float fRn, fGamma, fDelta;
		float fP, fT, fe_tmoy;
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

			//rayonnement net à la surface	(Rn) [MJ/m2/Jour]
			fRn = _rayonnementNet->PrendreRayonnementNet(iJourJulien,  index_zone);

			//constante psychrométrique (Gamma) [kPa/dC]
			//pression atmosphérique (P) [kPa];
			fP = 101.3f * pow( ((293.0f - 0.0065f * zone.PrendreAltitude()) / 293.0f), 5.26f);

			fGamma = 0.000665f * fP;

			//temperature moyenne de l'air [dC]
			fT = (zone.PrendreTMax() + zone.PrendreTMin()) / 2.0f;

			//pente de la tension de vapeur d’eau saturante en fonction de la température (Delta) [kPa/dC]
			fe_tmoy = 0.6108f * exp(17.27f * fT / (fT + 237.3f));
			fDelta = 4098.0f * fe_tmoy / pow(fT + 237.3f, 2.0f);
			
			//evapotranspiration maximale des vegetaux et evaporation de l'eau en mm/j
			float fETP = _coefficient_proportionnalite_alpha[index_zone] * (fDelta * fRn) / (fDelta + fGamma) * 1.0f / CHALEUR_LATENTE_VAPORISATION;

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


	void PRIESTLAY_TAYLOR::Termine()
	{
		EVAPOTRANSPIRATION::Termine();
	}


	void PRIESTLAY_TAYLOR::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			 throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES PRIESTLAY-TAYLOR");

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES PRIESTLAY-TAYLOR", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		size_t index_zone;
		float coef, coefProp;
		char c;
		int ident;

		try{

		getline_mod(fichier, ligne);	//entete
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier >> ident >> c;
			fichier >> coefProp >> c;
			fichier >> coef;

			index_zone = zones.IdentVersIndex(ident);

			_coefficient_proportionnalite_alpha[index_zone] = coefProp;
			ChangeCoefficientMultiplicatif(index_zone, coef);
		}

		}
		catch(...)
		{
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES PRIESTLAY_TAYLOR; fichier invalide");
		}
	}


	void PRIESTLAY_TAYLOR::LectureParametresFichierGlobal()
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
			if(ligne == "PRIESTLAY-TAYLOR" || ligne == "PRIESTLAY TAYLOR")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 3)
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "PRIESTLAY-TAYLOR; Nombre de colonne invalide.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "PRIESTLAY-TAYLOR. ID de groupe invalide. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_coefficient_proportionnalite_alpha[index_zone] = vValeur[1];
						ChangeCoefficientMultiplicatif(index_zone, vValeur[2]);
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; PRIESTLAY-TAYLOR; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele PRIESTLAY-TAYLOR");
	}


	void PRIESTLAY_TAYLOR::SauvegardeParametres()
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

		fichier << "UHRH ID;COEFFICIENT PROPORTIONNALITE ALPHA;COEFFICIENT MULTIPLICATIF D'OPTIMISATION" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << _coefficient_proportionnalite_alpha[index] << ';';
			fichier << PrendreCoefficientMultiplicatif(index) << endl;
		}
	}

}
