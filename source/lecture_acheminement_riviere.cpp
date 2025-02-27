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

#include "lecture_acheminement_riviere.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <fstream>


using namespace std;


namespace HYDROTEL
{

	LECTURE_ACHEMINEMENT_RIVIERE::LECTURE_ACHEMINEMENT_RIVIERE(SIM_HYD& sim_hyd)
		: ACHEMINEMENT_RIVIERE(sim_hyd, "LECTURE ACHEMINEMENT RIVIERE")
	{
	}

	LECTURE_ACHEMINEMENT_RIVIERE::~LECTURE_ACHEMINEMENT_RIVIERE()
	{
	}

	void LECTURE_ACHEMINEMENT_RIVIERE::Initialise()
	{
		string ligne;

		//DEBIT AMONT
		_fichier_debit_amont.open(_nom_fichier_debit_amont);
		if (!_fichier_debit_amont)
			throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; mode lecture; fichier debit_amont.csv; " + _nom_fichier_debit_amont);

		_fichier_debit_amont.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_debit_amont, ligne); // commentaire
		getline_mod(_fichier_debit_amont, ligne); // entete

		//validation //les troncons simulé doivent etre présent dans le fichier
		istringstream iss;
		vector<string> vStr;
		size_t x, index_troncon;
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

		for(x=0; x<_sim_hyd.PrendreTronconsSimules().size(); x++)
		{
			index_troncon = _sim_hyd.PrendreTronconsSimules()[x];
			val = _sim_hyd.PrendreTroncons()[index_troncon]->PrendreIdent();

			if(find(begin(_vIDFichier), end(_vIDFichier), val) == end(_vIDFichier))
			{
				ostringstream oss;	
				oss << val;
				ligne = oss.str();

				throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; mode lecture; " + _nom_fichier_debit_amont + "; le troncon " + ligne + " est simule mais absent du fichier.");
			}
		}

		//DEBIT AVAL
		_fichier_debit_aval.open(_nom_fichier_debit_aval);
		if (!_fichier_debit_aval)
			throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; mode lecture; fichier debit_aval.csv; " + _nom_fichier_debit_aval);

		_fichier_debit_aval.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_debit_aval, ligne); // commentaire
		getline_mod(_fichier_debit_aval, ligne); // entete

		//validation //les troncons simulé doivent etre présent dans le fichier
		_vIDFichier.clear();
		SplitString(vStr, ligne, ";", false, false);
		for(x=1; x<vStr.size(); x++)
		{
			iss.clear();
			iss.str(vStr[x]);
			iss >> val;

			_vIDFichier.push_back(val);
		}

		for(x=0; x<_sim_hyd.PrendreTronconsSimules().size(); x++)
		{
			index_troncon = _sim_hyd.PrendreTronconsSimules()[x];
			val = _sim_hyd.PrendreTroncons()[index_troncon]->PrendreIdent();

			if(find(begin(_vIDFichier), end(_vIDFichier), val) == end(_vIDFichier))
			{
				ostringstream oss;	
				oss << val;
				ligne = oss.str();

				throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; mode lecture; " + _nom_fichier_debit_aval + "; le troncon " + ligne + " est simule mais absent du fichier.");
			}
		}

		ACHEMINEMENT_RIVIERE::Initialise();
	}


	void LECTURE_ACHEMINEMENT_RIVIERE::Calcule()
	{
		DATE_HEURE date_lue;
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		size_t x, index_troncon, validation;
		string ligne;
		char c;

		istringstream ss;
		ss.exceptions(ios::failbit | ios::badbit);

		//DEBIT AMONT
		do
		{
			getline_mod(_fichier_debit_amont, ligne);
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
			float fVal;
			ss >> c >> fVal;

			index_troncon = _sim_hyd.PrendreTroncons().IdentVersIndex(_vIDFichier[x]);	//ident vers index
			if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index_troncon) != end(_sim_hyd.PrendreTronconsSimules()))
			{
				//troncons[index_troncon]->ChangeDebitAmont(debit_amont);
				troncons[index_troncon]->ChangeDebitAmontMoyen(fVal);
				++validation;
			}
		}

		if(validation != _sim_hyd.PrendreTronconsSimules().size())
			throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; mode lecture; " + _nom_fichier_debit_amont + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");

		//DEBIT AVAL
		do
		{
			getline_mod(_fichier_debit_aval, ligne);
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
			float fVal;
			ss >> c >> fVal;

			index_troncon = _sim_hyd.PrendreTroncons().IdentVersIndex(_vIDFichier[x]);	//ident vers index
			if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index_troncon) != end(_sim_hyd.PrendreTronconsSimules()))
			{
				//troncons[index_troncon]->ChangeDebitAval(debit_amont);
				troncons[index_troncon]->ChangeDebitAvalMoyen(fVal);
				++validation;
			}
		}

		if(validation != _sim_hyd.PrendreTronconsSimules().size())
			throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; mode lecture; " + _nom_fichier_debit_aval + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");

		ACHEMINEMENT_RIVIERE::Calcule();
	}


	void LECTURE_ACHEMINEMENT_RIVIERE::Termine()
	{
		_fichier_debit_amont.close();
		_fichier_debit_aval.close();

		ACHEMINEMENT_RIVIERE::Termine();
	}


	void LECTURE_ACHEMINEMENT_RIVIERE::ChangeNbParams(const ZONES& /*zones*/)
	{
	}


	void LECTURE_ACHEMINEMENT_RIVIERE::LectureParametres()
	{
		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
		{
			if(_sim_hyd.PrendreNomAcheminement() == PrendreNomSousModele())
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_ACHEMINEMENT_RIVIERE");
			else
				return;
		}

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_ACHEMINEMENT_RIVIERE", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		string repertoire = _sim_hyd.PrendreRepertoireProjet();

		lire_cle_valeur(fichier, cle, valeur); 
		if (!Racine(valeur))
			valeur = Combine(repertoire, valeur);
		_nom_fichier_debit_amont = valeur;

		lire_cle_valeur(fichier, cle, valeur); 
		if (!Racine(valeur))
			valeur = Combine(repertoire, valeur);
		_nom_fichier_debit_aval = valeur;
	}


	void LECTURE_ACHEMINEMENT_RIVIERE::SauvegardeParametres()
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

		fichier << "NOM FICHIER DEBIT AMONT;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_debit_amont) << endl;
		fichier << "NOM FICHIER DEBIT AVAL;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_debit_aval) << endl;
	}

}
