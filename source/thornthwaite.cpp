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

#include "thornthwaite.hpp"

#include "constantes.hpp"
#include "donnee_meteo.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>


using namespace std;


namespace HYDROTEL
{

	THORNTHWAITE::THORNTHWAITE(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "THORNTHWAITE")
	{
	}

	THORNTHWAITE::~THORNTHWAITE()
	{
	}

	void THORNTHWAITE::Initialise()
	{
		EVAPOTRANSPIRATION::Initialise();
	}

	void THORNTHWAITE::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();

		const size_t nb_classe = occupation_sol.PrendreNbClasse();

		const float poids = Repartition(_sim_hyd.PrendrePasDeTemps(), _sim_hyd.PrendreDateCourante().PrendreHeure());

		int jour_julien = _sim_hyd.PrendreDateCourante().PrendreJourJulien();

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			float indice_thermique = _indice_thermique[index_zone];

			float xaa = 0.675E-06f * pow(indice_thermique, 3.0f) -
				0.771E-04f * pow(indice_thermique, 2.0f) +
				0.0179f * indice_thermique + 0.492f;

			float c1 (-tan(asin(23.45f * PI / 180.0f) * sin(2.0f * PI * (jour_julien - _facteur_dephasage[index_zone]) / 365.0f)));

			float latitude = static_cast<float>( zone.PrendreCentroide().PrendreY() );

			float cvaran = (2.0f * acos(c1) * tan(latitude * PI / 180.0f) / PI);

			float tmoy = (zone.PrendreTMinJournaliere() + zone.PrendreTMaxJournaliere()) / 2;

			float etp = 0;

			if (tmoy > 0.0f)
				etp = poids * (16.2f / 30.4f * pow((10.0f * tmoy / indice_thermique), (xaa)) * cvaran);

			for (size_t index_classe = 0; index_classe < nb_classe; ++index_classe)
			{
				float etp_pond = etp * occupation_sol.PrendrePourcentage(index_zone, index_classe) * _coefficients_multiplicatif[index_zone];
				zone.ChangeEtp(index_classe, max(etp_pond, 0.0f));
			}
		}

		EVAPOTRANSPIRATION::Calcule();
	}

	void THORNTHWAITE::Termine()
	{
		EVAPOTRANSPIRATION::Termine();
	}

	void THORNTHWAITE::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THORNTHWAITE");

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THORNTHWAITE", 1);
		}

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		getline_mod(fichier, ligne); // commentaire

		int no_ligne = 6;

		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			int ident, facteur_dephasage;
			float coef, indice_thermique;
			char c;

			fichier >> ident >> c;
			fichier >> indice_thermique >> c;
			fichier >> facteur_dephasage >> c;
			fichier >> coef;

			size_t index_zone = zones.IdentVersIndex(ident);

			//validation parametres
			if (indice_thermique < 1.0f || indice_thermique > 100.0f)
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne);
			}

			if (facteur_dephasage < 1.0f || facteur_dephasage > 80.0f)
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne);
			}

			//
			_indice_thermique[index_zone] = indice_thermique;
			_facteur_dephasage[index_zone] = facteur_dephasage;

			ChangeCoefficientMultiplicatif(index_zone, coef);

			++no_ligne;
		}

		fichier.close();
	}

	void THORNTHWAITE::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal );

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
			if(ligne == "THORNTHWAITE")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide. THORNTHWAITE.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. THORNTHWAITE. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_indice_thermique[index_zone] = vValeur[1];
						_facteur_dephasage[index_zone] = static_cast<int>(vValeur[2]);
						ChangeCoefficientMultiplicatif(index_zone, vValeur[3]);
					}
				}

				bOK = true;
				break;
			}

			++no_ligne;
		}

		}
		catch(const ERREUR_LECTURE_FICHIER& ex)
		{
			fichier.close();
			throw ex;
		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal + "; THORNTHWAITE");
		}

		fichier.close();

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele THORNTHWAITE");
	}

	void THORNTHWAITE::SauvegardeParametres()
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

		fichier << "UHRH ID;INDICE THERMIQUE;FACTEUR DEPHASAGE;COEFFICIENT MULTIPLICATIF D'OPTIMISATION" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';

			fichier << _indice_thermique[index] << ';';
			fichier << _facteur_dephasage[index] << ';';

			fichier << PrendreCoefficientMultiplicatif(index) << endl;
		}
	}

	void THORNTHWAITE::ChangeNbParams(const ZONES& zones)
	{
		const size_t nb_zone = zones.PrendreNbZone();

		_indice_thermique.resize(nb_zone, 30);
		_facteur_dephasage.resize(nb_zone, 80);

		EVAPOTRANSPIRATION::ChangeNbParams(zones);
	}

	void THORNTHWAITE::ChangeIndiceThermique(size_t index_zone, float indice_thermique)
	{
		BOOST_ASSERT(index_zone < _indice_thermique.size() && indice_thermique >= 1 && indice_thermique <= 100);		
		_indice_thermique[index_zone] = indice_thermique;
	}

	void THORNTHWAITE::ChangeFacteurDephasage(size_t index_zone, int facteur_dephasage)
	{
		BOOST_ASSERT(index_zone < _coefficients_multiplicatif.size() && facteur_dephasage >= 1 && facteur_dephasage <= 80);
		_facteur_dephasage[index_zone] = facteur_dephasage;
	}

}
