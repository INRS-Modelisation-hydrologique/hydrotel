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

#include "lecture_fonte_glacier.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"


using namespace std;


namespace HYDROTEL
{

	LECTURE_FONTE_GLACIER::LECTURE_FONTE_GLACIER(SIM_HYD& sim_hyd)
		: FONTE_GLACIER(sim_hyd, "LECTURE FONTE GLACIER")
	{
	}

	LECTURE_FONTE_GLACIER::~LECTURE_FONTE_GLACIER()
	{
	}

	void LECTURE_FONTE_GLACIER::ChangeNomFichierIn1(string nom_fichier)
	{
		_nom_fichier_out1 = nom_fichier;
	}

	string LECTURE_FONTE_GLACIER::PrendreNomFichierIn1() const
	{
		return _nom_fichier_out1;
	}

	void LECTURE_FONTE_GLACIER::Initialise()
	{
		//apport glacier [mm]
		_fichier_in1.open(_nom_fichier_out1);
		if (!_fichier_in1)
			throw ERREUR_LECTURE_FICHIER("FONTE_GLACIER; mode lecture; fichier glacier-apport.csv; " + _nom_fichier_out1);

		_fichier_in1.exceptions(ios::failbit | ios::badbit);

		string ligne;
		getline_mod(_fichier_in1, ligne); // commentaire
		getline_mod(_fichier_in1, ligne); // entete

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

				_fichier_in1.close();
				throw ERREUR_LECTURE_FICHIER("FONTE_GLACIER; mode lecture; " + _nom_fichier_out1 + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}

		FONTE_GLACIER::Initialise();
	}


	void LECTURE_FONTE_GLACIER::Calcule()
	{
		DATE_HEURE date_lue;
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		ZONES& zones = _sim_hyd.PrendreZones();

		string ligne;
		size_t x, index_zone, validation;
		char c;
		istringstream ss;

		ss.exceptions(ios::failbit | ios::badbit);

		//apport glacier [mm]
		do
		{
			getline_mod(_fichier_in1, ligne);
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
			float apport;
			ss >> c >> apport;	//mm

			if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
			{
				index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);
				zones[index_zone].ChangeApportGlacier(apport);
				++validation;
			}
		}

		if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
			throw ERREUR_LECTURE_FICHIER("FONTE_GLACIER; mode lecture; " + _nom_fichier_out1 + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");

		FONTE_GLACIER::Calcule();
	}


	void LECTURE_FONTE_GLACIER::Termine()
	{
		_fichier_in1.close();
		
		FONTE_GLACIER::Termine();
	}


	void LECTURE_FONTE_GLACIER::ChangeNbParams(const ZONES& /*zones*/)
	{
	}


	void LECTURE_FONTE_GLACIER::LectureParametres()
	{
		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
		{
			if(_sim_hyd.PrendreNomFonteGlacier() == PrendreNomSousModele())
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_FONTE_GLACIER");
			else
				return;
		}

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_FONTE_GLACIER", 1);
		}

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		string repertoire = _sim_hyd.PrendreRepertoireProjet();

		lire_cle_valeur(fichier, cle, valeur);
		if (valeur.size() != 0 && !Racine(valeur) )
			valeur = Combine(repertoire, valeur);
		ChangeNomFichierIn1(valeur);

		fichier.close();
	}


	void LECTURE_FONTE_GLACIER::SauvegardeParametres()
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

		fichier << "NOM FICHIER APPORT GLACIER;" << PrendreRepertoireRelatif(repertoire_projet, PrendreNomFichierIn1()) << endl;

		fichier.close();
	}

}
