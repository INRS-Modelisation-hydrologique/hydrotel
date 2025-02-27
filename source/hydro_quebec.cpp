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

#include "hydro_quebec.hpp"

#include "constantes.hpp"
#include "donnee_meteo.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>


using namespace std;


namespace HYDROTEL
{

	HYDRO_QUEBEC::HYDRO_QUEBEC(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "HYDRO-QUEBEC")
	{
	}

	HYDRO_QUEBEC::~HYDRO_QUEBEC()
	{
	}

	void HYDRO_QUEBEC::Initialise()
	{
		EVAPOTRANSPIRATION::Initialise();
	}

	void HYDRO_QUEBEC::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();

		const size_t nb_classe = occupation_sol.PrendreNbClasse();

		const float poids = Repartition(_sim_hyd.PrendrePasDeTemps(), _sim_hyd.PrendreDateCourante().PrendreHeure());

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			float tmin = zone.PrendreTMinJournaliere();
			float tmax = zone.PrendreTMaxJournaliere();

			float etp = poids * 0.029718f * (tmax - tmin) * exp(0.019f * ((9.0f/5.0f) * (tmax + tmin) + 64.0f));

			for (size_t index_classe = 0; index_classe < nb_classe; ++index_classe)
			{
				float fPourcentage = occupation_sol.PrendrePourcentage(index_zone, index_classe);
				float etp_pond = etp * fPourcentage * _coefficients_multiplicatif[index_zone];
				zone.ChangeEtp(index_classe, max(etp_pond, 0.0f));
			}
		}

		EVAPOTRANSPIRATION::Calcule();
	}

	void HYDRO_QUEBEC::Termine()
	{
		EVAPOTRANSPIRATION::Termine();
	}

	void HYDRO_QUEBEC::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES HYDRO_QUEBEC");

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES HYDRO_QUEBEC", 1);

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

	void HYDRO_QUEBEC::LectureParametresFichierGlobal()
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
			if(ligne == "HYDRO-QUEBEC")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 2)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide HYDRO-QUEBEC.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide HYDRO-QUEBEC. Les ID de groupe doivent etre en ordre croissant.");

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
		catch(const ERREUR_LECTURE_FICHIER& ex)
		{
			fichier.close();
			throw ex;
		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; HYDRO-QUEBEC; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele HYDRO-QUEBEC");
	}

	void HYDRO_QUEBEC::SauvegardeParametres()
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

		fichier << "UHRH ID;COEFFICIENT MULTIPLICATIF D'OPTIMISATION" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';
			fichier << PrendreCoefficientMultiplicatif(index) << endl;
		}

		fichier.close();
	}

}
