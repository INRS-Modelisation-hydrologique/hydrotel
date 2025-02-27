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

#include "linacre.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>


using namespace std;


namespace HYDROTEL
{

	LINACRE::LINACRE(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "LINACRE")
	{
	}


	LINACRE::~LINACRE()
	{
	}


	void LINACRE::ChangeNbParams(const ZONES& zones)
	{
		const size_t nbUHRH = zones.PrendreNbZone();

		_mois_plus_froid.resize(nbUHRH, -10.0f);	//C
		_mois_plus_chaud.resize(nbUHRH, 20.0f);		//C
		_fAlbedo.resize(nbUHRH, 0.23f);

		EVAPOTRANSPIRATION::ChangeNbParams(zones);
	}


	void LINACRE::Initialise()
	{
		EVAPOTRANSPIRATION::Initialise();
	}


	void LINACRE::Calcule()
	{
		float fe_tmoy, fDelta, fGamma, fP, tmer, ea, tmin, tmax, alti, tmoy, etp, etp_pond, corn;

		ZONES& zones = _sim_hyd.PrendreZones();
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();
		const size_t nb_classe = occupation_sol.PrendreNbClasse();
		const float poids = Repartition(_sim_hyd.PrendrePasDeTemps(), _sim_hyd.PrendreDateCourante().PrendreHeure());
		float dtmxn, fAlbedo;

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();
		size_t index_zone, index_classe, index;

		for (index=0; index<index_zones.size(); index++)
		{
			index_zone = index_zones[index];
			ZONE& zone = zones[index_zone];

			dtmxn = _mois_plus_chaud[index_zone] - _mois_plus_froid[index_zone];

			corn = 24.41f / (100.0f - static_cast<float>(zone.PrendreCentroide().PrendreY()));

			tmin = zone.PrendreTMinJournaliere();
			tmax = zone.PrendreTMaxJournaliere();
			alti = zone.PrendreAltitude();

			tmoy = (tmin + tmax) / 2.0f;

			//pente de la tension de vapeur d’eau saturante en fonction de la température (Delta) [kPa/dC]
			fe_tmoy = 0.6108f * exp(17.27f * tmoy / (tmoy + 237.3f));
			fDelta = 4098.0f * fe_tmoy / pow(tmoy + 237.3f, 2.0f);

			//constante psychrométrique (Gamma) [kPa/dC]
			fP = 101.3f * pow( ((293.0f - 0.0065f * zone.PrendreAltitude()) / 293.0f), 5.26f);	//pression atmosphérique (P) [kPa];
			fGamma = 0.000665f * fP;

			tmer = corn * (tmoy + 0.006f * alti);
			ea = 0.3807f * (0.0023f * alti + 0.37f * tmoy + 0.53f * (tmax - tmin) + 0.35f * dtmxn - 10.9f);

			if(zone.PrendreCouvertNival() > 0.0f)
				fAlbedo = zone.PrendreAlbedoNeige();
			else
				fAlbedo = _fAlbedo[index_zone];

			etp = poids * ((0.75f -  fAlbedo) * tmer + ea) * fDelta / (fDelta + fGamma);	//mm
			
			for (index_classe=0; index_classe<nb_classe; index_classe++)
			{
				etp_pond = etp * occupation_sol.PrendrePourcentage(index_zone, index_classe) * _coefficients_multiplicatif[index_zone];
				zone.ChangeEtp(index_classe, max(etp_pond, 0.0f));
			}
		}

		EVAPOTRANSPIRATION::Calcule();
	}


	void LINACRE::Termine()
	{
		EVAPOTRANSPIRATION::Termine();
	}


	void LINACRE::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
		{
			if(_sim_hyd.PrendreNomEvapotranspiration() == PrendreNomSousModele())
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LINACRE");
			else
				return;
		}

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LINACRE", 1);

		getline_mod(fichier, ligne);	//nom sous modele
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		size_t index_zone;
		int ident;
		float coef;
		char c;

		try{

		getline_mod(fichier, ligne);	//entete
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier >> ident >> c;
			index_zone = zones.IdentVersIndex(ident);

			fichier >> _mois_plus_froid[index_zone] >> c;
			fichier >> _mois_plus_chaud[index_zone] >> c;
			fichier >> _fAlbedo[index_zone] >> c;
			fichier >> coef;
			ChangeCoefficientMultiplicatif(index_zone, coef);
		}

		}
		catch(...)
		{
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LINACRE; fichier invalide");
		}
	}


	void LINACRE::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL;" + _sim_hyd._nomFichierParametresGlobal );

		fichier.exceptions(ios::failbit | ios::badbit);

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
			if(ligne == "LINACRE")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 5)
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "LINACRE; Nombre de colonne invalide.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "LINACRE. ID de groupe invalide. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_mois_plus_froid[index_zone] = vValeur[1];
						_mois_plus_chaud[index_zone] = vValeur[2];
						_fAlbedo[index_zone] = vValeur[3];
						ChangeCoefficientMultiplicatif(index_zone, vValeur[4]);
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; LINACRE; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele LINACRE");
	}


	void LINACRE::SauvegardeParametres()
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

		fichier << "UHRH ID; TEMPERATURE MOIS PLUS FROID (C); TEMPERATURE MOIS PLUS CHAUD (C); ALBEDO (0.23 SURFACE REFERENCE); COEFFICIENT MULTIPLICATIF OPTIMISATION" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << _mois_plus_froid[index] << ';';
			fichier << _mois_plus_chaud[index] << ';';
			fichier << _fAlbedo[index] << ';';
			fichier << PrendreCoefficientMultiplicatif(index) << endl;
		}
	}

}
