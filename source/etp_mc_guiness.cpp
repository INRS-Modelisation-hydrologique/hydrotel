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
//USA
//

#include "etp_mc_guiness.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>


using namespace std;


namespace HYDROTEL
{

	ETP_MC_GUINESS::ETP_MC_GUINESS(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "ETP-MC-GUINESS")
	{
	}


	ETP_MC_GUINESS::~ETP_MC_GUINESS()
	{
	}


	void ETP_MC_GUINESS::ChangeNbParams(const ZONES& zones)
	{
		EVAPOTRANSPIRATION::ChangeNbParams(zones);
	}


	void ETP_MC_GUINESS::Initialise()
	{
		vector<double> vRe;
		double lat, Re, dPoids;
		size_t index;
		int iJour, iHeureDebut, iHeureCourante, iHeureTemp, iPdt;

		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();

		_lambda = 2.264;	//MJ/kg
		_rho = 1.0;	//kg/L

		_index_zones = _sim_hyd.PrendreZonesSimules();
		_nbZonesSimule = _index_zones.size();
		_nbClasseOccsol = occupation_sol.PrendreNbClasse();

		ZONES& zones = _sim_hyd.PrendreZones();

		_Re.clear();
		_Re.resize(_nbZonesSimule);

		vRe.resize(365);

		//calcule rayonnement extraterrestre
		for (index=0; index<_nbZonesSimule; index++)
		{
			ZONE& zone = zones[_index_zones[index]];
			lat = zone.PrendreCentroide().PrendreY();	//[dd]

			if(lat > 66.5)
				throw ERREUR("ETP-MC-GUINESS; the extraterrestrial radiation function is invalid for latitudes above the polar circle (> 66.5 dd).");

			for(iJour=1; iJour<= 365; iJour++)
			{
				Re = RayonnementExtraterrestre(iJour, lat);   //MJ.m-2.j-1
				vRe[iJour-1] = Re;
			}

			_Re[index] = vRe;
		}

		//calcul des poids des pas de temps si pdt != 24 car le modele mcguiness est basé sur une equation journaliere
		_poidPasTemps.clear();
		_poidPasTemps.resize(24, -1.0);

		iHeureDebut = _sim_hyd.PrendreDateDebut().PrendreHeure();
		iPdt = _sim_hyd.PrendrePasDeTemps();
		
		if(iPdt == 24)
			_poidPasTemps[iHeureDebut] = 1.0;
		else
		{
			double repartition[] = { 0.5, 0.5, 0.5, 0.5, 0.5, 1.0, 2.2, 4.0, 
			                         5.4, 8.0, 8.4, 9.6, 10.4, 10.8, 10.8, 9.6, 
									 7.8, 5.0, 2.0, 0.5, 0.5, 0.5, 0.5, 0.5 };

			iHeureCourante = iHeureDebut;

			do
			{
				iHeureTemp = iHeureCourante;
				dPoids = 0.0;

				do
				{
					dPoids+= repartition[iHeureTemp];
					++iHeureTemp;
				} 
				while( (iHeureTemp < 24) && (iHeureTemp < iHeureCourante+iPdt) );

				dPoids/= 100.0;
				_poidPasTemps[iHeureCourante] = dPoids;

				iHeureCourante+= iPdt;
				if(iHeureCourante >= 24)
					iHeureCourante-= 24;
			}
			while(iHeureCourante != iHeureDebut);
		}

		EVAPOTRANSPIRATION::Initialise();
	}


	void ETP_MC_GUINESS::Calcule()
	{
		double dTMoy, dETP, dETPpond, dPourcentage;
		size_t index, index_zone, index_classe;
		int iJourJulien, iHeureCourante;

		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();
		ZONES& zones = _sim_hyd.PrendreZones();

		iJourJulien = _sim_hyd.PrendreDateCourante().PrendreJourJulien();
		iHeureCourante = _sim_hyd.PrendreDateCourante().PrendreHeure();

		if(iJourJulien == 366)
			iJourJulien = 365;	//pour annee bisextile: met le jour 366 egal au jour 365

		//pour chaque uhrh simulé
		for (index=0; index<_nbZonesSimule; index++)
		{
			index_zone = _index_zones[index];
			ZONE& zone = zones[index_zone];

			dTMoy = static_cast<double>((zone.PrendreTMinJournaliere() + zone.PrendreTMaxJournaliere()) / 2.0f);	//[dC]

			dETP = _Re[index][iJourJulien-1]/(_lambda*_rho) * max(dTMoy+5.0, 0.0) / 68.0;	//[mm/Jour]

			dETP*= _poidPasTemps[iHeureCourante];	//ponderation des pas de temps	//si pdt = 24 -> poids = 1

			//ponderation sur les differentes classe d'occupation du sol et application du coefficient d'optimisation
			for (index_classe=0; index_classe<_nbClasseOccsol; index_classe++)
			{
				dPourcentage = occupation_sol.PrendrePourcentage(index_zone, index_classe);
				dETPpond = dETP * dPourcentage * _coefficients_multiplicatif[index_zone];

				zone.ChangeEtp(index_classe, static_cast<float>(max(dETPpond, 0.0)));	//[mm]
			}
		}

		EVAPOTRANSPIRATION::Calcule();
	}


	//------------------------------------------------------------------------------------------------
	//Calcul du rayonnement extraterrestre pour la formule d'ETP de McGuiness - Bordne. 
	//Fonction invalide au-delà du cercle polaire. Invalide if(dLatitudeDD > 66.5).
	//
	//Entrée :
	//  latitude: la latitude des points d'intérêt [dd]
	//
	//Sortie : 
	//  Re : Le rayonnement extraterrestre (aucune diffusion par l'atmosphère)
	//       incident à la latitude d'intérêt pour chaque jour de l'année (1 a 365) [MJ.m-2.j-1].
	//       Les années bissexiles doivent etre  gérées avant l'appel de la fonction.
	//
	double ETP_MC_GUINESS::RayonnementExtraterrestre(int iJulianDay, double dLatitudeDD)
	{
		double Lambda, I_sc, Gamma, E0, omega, delta, T_hr;

		//latitude (radian)
		Lambda = dLatitudeDD * (2.0 * dPI / 360.0);

		//constante solaire (MJ.m-2.jr-1)
		I_sc = 118.1;

		//moment de l'année entre 0 et 2*pi.
		Gamma = 2.0 * dPI * (iJulianDay-1.0) / 365.0;

		E0 = 1.00011 + 0.034221 * cos(Gamma) + 0.00128 * sin(Gamma) + 0.000719 * cos(2.0*Gamma) + 0.000077 * sin(2.0*Gamma);

		//vitesse angulaire de la terre (radian/hr)
		omega = 0.2618;

		//Déclinaison (radian)
		delta = 0.006918 - 0.399912 * cos(Gamma) + 0.070257 * sin(Gamma) - 0.006758 * cos(2.0*Gamma) + 0.000907 * sin(2.0*Gamma) - 0.002697 * cos(3.0*Gamma) + 0.00148 * sin(3.0*Gamma);

		T_hr = acos(min(-tan(delta) * tan(Lambda), 1.0)) / omega;

		//Énergie totale MJ.m-2.j-1
		return (2.0 * I_sc * E0 * (cos(delta) * cos(Lambda) * sin(omega * T_hr)/omega + sin(delta) * sin(Lambda)* T_hr)) / 24.0;
	}


	void ETP_MC_GUINESS::Termine()
	{
		EVAPOTRANSPIRATION::Termine();
	}


	void ETP_MC_GUINESS::LectureParametres()
	{
		if(_sim_hyd.PrendreNomEvapotranspiration() == PrendreNomSousModele())	//si le modele est simulé
		{
			if(_sim_hyd._fichierParametreGlobal)
			{
				LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
				return;
			}

			ZONES& zones = _sim_hyd.PrendreZones();

			ifstream fichier;
			
			fichier.open(PrendreNomFichierParametres(), ios_base::in);
			if (!fichier)
			{
				//cree un fichier avec valeur par defaut
				try{
				SauvegardeParametres();				
				}
				catch(...)
				{
					throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ETP-MC-GUINESS");
				}

				fichier.open(PrendreNomFichierParametres(), ios_base::in);
				if (!fichier)
					throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ETP-MC-GUINESS");

				Log("");
				Log("warning: the file `" + PrendreNomFichierParametres() + "` was missing: a file with default parameters has been created.");
				Log("");
			}

			string cle, valeur, ligne;
			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ETP-MC-GUINESS", 1);
			}

			getline_mod(fichier, ligne);
			lire_cle_valeur(fichier, cle, valeur);
			getline_mod(fichier, ligne);

			getline_mod(fichier, ligne); // commentaire

			for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
			{
				int ident;
				float coef;
				char c;

				fichier >> ident >> c;
				fichier >> coef;

				size_t index_zone = zones.IdentVersIndex(ident);

				ChangeCoefficientMultiplicatif(index_zone, coef);
			}

			fichier.close();
		}
	}


	void ETP_MC_GUINESS::LectureParametresFichierGlobal()
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
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, 1);
		}

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "ETP-MC-GUINESS")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 2)
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide ETP-MC-GUINESS.");
					}

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide ETP-MC-GUINESS. Les ID de groupe doivent etre en ordre croissant.");
					}

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						ChangeCoefficientMultiplicatif(index_zone, vValeur[1]);
					}
				}

				bOK = true;
				break;
			}

			++no_ligne;
		}

		fichier.close();

		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; ETP-MC-GUINESS; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele ETP-MC-GUINESS");
	}


	void ETP_MC_GUINESS::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		if(PrendreNomFichierParametres() == "")	//creation d'un fichier par defaut s'il n'existe pas
			ChangeNomFichierParametres(Combine(_sim_hyd.PrendreRepertoireSimulation(), "etp-mc-guiness.csv"));

		string nom_fichier = PrendreNomFichierParametres();
		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "UHRH ID;COEFFICIENT MULTIPLICATIF D'OPTIMISATION" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << PrendreCoefficientMultiplicatif(index) << endl;
		}

		fichier.close();
	}

}
