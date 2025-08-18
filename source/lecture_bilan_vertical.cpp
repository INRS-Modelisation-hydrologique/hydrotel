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

#include "lecture_bilan_vertical.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <algorithm>
#include <fstream>


using namespace std;


namespace HYDROTEL
{

	LECTURE_BILAN_VERTICAL::LECTURE_BILAN_VERTICAL(SIM_HYD& sim_hyd)
		: BILAN_VERTICAL(sim_hyd, "LECTURE BILAN VERTICAL")
	{
	}

	LECTURE_BILAN_VERTICAL::~LECTURE_BILAN_VERTICAL()
	{
	}

	void LECTURE_BILAN_VERTICAL::Initialise()
	{
		string ligne;

		_fichier_production_base.open(_nom_fichier_production_base);
		if (!_fichier_production_base)
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; fichier production_base.csv; " + _nom_fichier_production_base);

		_fichier_production_base.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_production_base, ligne); // commentaire
		getline_mod(_fichier_production_base, ligne); // entete

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

				throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; " + _nom_fichier_production_base + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}
		//

		_fichier_production_hypo.open(_nom_fichier_production_hypo);
		if (!_fichier_production_hypo)
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; fichier production_hypo.csv; " + _nom_fichier_production_hypo);

		_fichier_production_hypo.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_production_hypo, ligne); // commentaire
		getline_mod(_fichier_production_hypo, ligne); // entete

		//validation //les uhrh simulé doivent etre présent dans le fichier
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

				throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; " + _nom_fichier_production_hypo + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}
		//

		_fichier_production_surf.open(_nom_fichier_production_surf);
		if (!_fichier_production_surf)
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; fichier production_surf.csv; " + _nom_fichier_production_surf);

		_fichier_production_surf.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_production_surf, ligne); // commentaire
		getline_mod(_fichier_production_surf, ligne); // entete

		//validation //les uhrh simulé doivent etre présent dans le fichier
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

				throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; " + _nom_fichier_production_surf + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}

		BILAN_VERTICAL::Initialise();
	}


	void LECTURE_BILAN_VERTICAL::Calcule()
	{
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		ZONES& zones = _sim_hyd.PrendreZones();

		size_t x, index_zone, validation;
		string ligne;
		char c;

		bool bSkipSeparator = false;	//la lecture du separateur doit etre skipper si c'est une tabulation ('\t')
		ligne = _sim_hyd._output.Separator();
		if(ligne.length() > 0 && ligne[0] == '\t')
			bSkipSeparator = true;

		istringstream ss_base, ss_hypo, ss_surf;
		ss_base.exceptions(ios::failbit | ios::badbit);
		ss_hypo.exceptions(ios::failbit | ios::badbit);
		ss_surf.exceptions(ios::failbit | ios::badbit);

		DATE_HEURE date_lue_base;
		do
		{
			getline_mod(_fichier_production_base, ligne);
			ss_base.clear();
			ss_base.str(ligne);

			unsigned short annee, mois, jour, heure, minute;
			ss_base >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

			date_lue_base = DATE_HEURE(annee, mois, jour, heure);

		}
		while (date_lue_base < date_courante);		

		DATE_HEURE date_lue_hypo;
		do
		{
			getline_mod(_fichier_production_hypo, ligne);
			ss_hypo.clear();
			ss_hypo.str(ligne);

			unsigned short annee, mois, jour, heure, minute;
			ss_hypo >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

			date_lue_hypo = DATE_HEURE(annee, mois, jour, heure);

		}
		while (date_lue_hypo < date_courante);		

		DATE_HEURE date_lue_surf;
		do
		{
			getline_mod(_fichier_production_surf, ligne);
			ss_surf.clear();
			ss_surf.str(ligne);

			unsigned short annee, mois, jour, heure, minute;
			ss_surf >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

			date_lue_surf = DATE_HEURE(annee, mois, jour, heure);

		}
		while (date_lue_surf < date_courante);		

		try{

		validation = 0;
		for (x=0; x<_vIDFichier.size(); x++)
		{
			float production_base;
			float production_hypo;
			float production_surf;

			if(bSkipSeparator)
			{
				ss_base >> production_base;
				ss_hypo >> production_hypo;
				ss_surf >> production_surf;
			}
			else
			{
				ss_base >> c >> production_base;
				ss_hypo >> c >> production_hypo;
				ss_surf >> c >> production_surf;
			}

			if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
			{
				index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);

				zones[index_zone].ChangeProdBase(production_base);
				zones[index_zone].ChangeProdHypo(production_hypo);
				zones[index_zone].ChangeProdSurf(production_surf);

				++validation;
			}
		}

		}
		catch(...)
		{
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; mode lecture; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");

		BILAN_VERTICAL::Calcule();
	}


	void LECTURE_BILAN_VERTICAL::Termine()
	{
		_fichier_production_base.close();
		_fichier_production_hypo.close();
		_fichier_production_surf.close();

		BILAN_VERTICAL::Termine();
	}

	void LECTURE_BILAN_VERTICAL::ChangeNbParams(const ZONES& /*zones*/)
	{
	}

	void LECTURE_BILAN_VERTICAL::LectureParametres()
	{
		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
		{
			if(_sim_hyd.PrendreNomBilanVertical() == PrendreNomSousModele())
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_BILAN_VERTICAL");
			else
				return;
		}

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_BILAN_VERTICAL", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		string repertoire = _sim_hyd.PrendreRepertoireProjet();

		lire_cle_valeur(fichier, cle, valeur); 
		if (!Racine(valeur))
			valeur = Combine(repertoire, valeur);
		_nom_fichier_production_base = valeur;

		lire_cle_valeur(fichier, cle, valeur); 
		if (!Racine(valeur))
			valeur = Combine(repertoire, valeur);
		_nom_fichier_production_hypo = valeur;

		lire_cle_valeur(fichier, cle, valeur); 
		if (!Racine(valeur))
			valeur = Combine(repertoire, valeur);
		_nom_fichier_production_surf = valeur;
	}

	void LECTURE_BILAN_VERTICAL::SauvegardeParametres()
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

		fichier << "NOM FICHIER PRODUCTION BASE;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_production_base) << endl;
		fichier << "NOM FICHIER PRODUCTION HYPO;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_production_hypo) << endl;
		fichier << "NOM FICHIER PRODUCTION SURF;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_production_surf) << endl;
	}

}
