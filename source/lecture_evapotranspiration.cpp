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

#include "lecture_evapotranspiration.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	LECTURE_EVAPOTRANSPIRATION::LECTURE_EVAPOTRANSPIRATION(SIM_HYD& sim_hyd)
		: EVAPOTRANSPIRATION(sim_hyd, "LECTURE EVAPOTRANSPIRATION")
	{
	}

	LECTURE_EVAPOTRANSPIRATION::~LECTURE_EVAPOTRANSPIRATION()
	{
	}


	void LECTURE_EVAPOTRANSPIRATION::Initialise()
	{
		_fichier_evp.open(_nom_fichier_evp);
		if (!_fichier_evp)
			throw ERREUR_LECTURE_FICHIER("EVAPOTRANSPIRATION; mode lecture; fichier evp.csv; " + _nom_fichier_evp);

		_fichier_evp.exceptions(ios::failbit | ios::badbit);

		string ligne;
		getline_mod(_fichier_evp, ligne); // commentaire
		getline_mod(_fichier_evp, ligne); // entete

		//validation //les uhrh simulé doivent etre présent dans le fichier
		istringstream iss;
		vector<string> vStr;
		size_t x;
		int val;

		_vIDFichier.clear();
		SplitString(vStr, ligne, ";", false, false);
		for(x=1; x<vStr.size(); x++)
		{
			iss.clear();
			iss.str(vStr[x]);
			iss >> val;

			_vIDFichier.push_back(val);
		}

		for(x=0; x<_sim_hyd.PrendreZonesSimulesIdent().size(); x++)
		{
			val = _sim_hyd.PrendreZonesSimulesIdent()[x];
			if(find(begin(_vIDFichier), end(_vIDFichier), val) == end(_vIDFichier))
			{
				ostringstream oss;	
				oss << val;
				ligne = oss.str();

				throw ERREUR_LECTURE_FICHIER("EVAPOTRANSPIRATION; mode lecture; " + _nom_fichier_evp + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}

		EVAPOTRANSPIRATION::Initialise();
	}


	void LECTURE_EVAPOTRANSPIRATION::Calcule()
	{
		DATE_HEURE date_lue;
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		ZONES& zones = _sim_hyd.PrendreZones();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();

		size_t x, index_zone, validation;
		string ligne;
		char c;
		istringstream ss;
		ss.exceptions(ios::failbit | ios::badbit);

		do
		{
			getline_mod(_fichier_evp, ligne);
			ss.clear();
			ss.str(ligne);

			unsigned short annee, mois, jour, heure, minute;
			ss >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

			date_lue = DATE_HEURE(annee, mois, jour, heure);
		}
		while (date_lue < date_courante);

		validation = 0;
		for (x=0; x<_vIDFichier.size(); x++)
		{
			float evp;
			ss >> c >> evp;

			if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
			{
				index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);

				for (size_t index_occupation = 0; index_occupation < occupation_sol.PrendreNbClasse(); ++index_occupation)
					zones[index_zone].ChangeEtp(index_occupation, occupation_sol.PrendrePourcentage(index_zone, index_occupation) * evp);

				++validation;
			}
		}

		if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
			throw ERREUR_LECTURE_FICHIER("EVAPOTRANSPIRATION; mode lecture; " + _nom_fichier_evp + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");

		EVAPOTRANSPIRATION::Calcule();
	}

	void LECTURE_EVAPOTRANSPIRATION::Termine()
	{
		_fichier_evp.close();

		EVAPOTRANSPIRATION::Termine();
	}

	void LECTURE_EVAPOTRANSPIRATION::ChangeNbParams(const ZONES& /*zones*/)
	{
	}

	void LECTURE_EVAPOTRANSPIRATION::LectureParametres()
	{
		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
		{
			if(_sim_hyd.PrendreNomEvapotranspiration() == PrendreNomSousModele())
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_EVAPOTRANSPIRATION");
			else
				return;
		}

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_EVAPOTRANSPIRATION", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		string repertoire = _sim_hyd.PrendreRepertoireProjet();

		lire_cle_valeur(fichier, cle, valeur); 

		if (!Racine(valeur))
			valeur = Combine(repertoire, valeur);

		_nom_fichier_evp = valeur;
	}

	void LECTURE_EVAPOTRANSPIRATION::SauvegardeParametres()
	{
		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		string repertoire_projet = _sim_hyd.PrendreRepertoireProjet();

		fichier << "NOM FICHIER ETP;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_evp) << endl;
	}

}
