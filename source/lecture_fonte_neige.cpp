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

#include "lecture_fonte_neige.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"


using namespace std;


namespace HYDROTEL
{

	LECTURE_FONTE_NEIGE::LECTURE_FONTE_NEIGE(SIM_HYD& sim_hyd)
		: FONTE_NEIGE(sim_hyd, "LECTURE FONTE NEIGE")
	{
	}

	LECTURE_FONTE_NEIGE::~LECTURE_FONTE_NEIGE()
	{
	}

	void LECTURE_FONTE_NEIGE::ChangeNomFichierApport(string nom_fichier)
	{
		_nom_fichier_apport = nom_fichier;
	}

	string LECTURE_FONTE_NEIGE::PrendreNomFichierApport() const
	{
		return _nom_fichier_apport;
	}

	void LECTURE_FONTE_NEIGE::ChangeNomFichierHauteurNeige(string nom_fichier)
	{
		_nom_fichier_hauteur_neige = nom_fichier;
	}

	string LECTURE_FONTE_NEIGE::PrendreNomFichierHauteurNeige() const
	{
		return _nom_fichier_hauteur_neige;
	}


	void LECTURE_FONTE_NEIGE::Initialise()
	{
		//APPORT (PLUIE + NEIGE)
		_fichier_apport.open(_nom_fichier_apport);
		if (!_fichier_apport)
			throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; fichier apport.csv; " + _nom_fichier_apport);

		_fichier_apport.exceptions(ios::failbit | ios::badbit);

		string ligne;
		getline_mod(_fichier_apport, ligne); // commentaire
		getline_mod(_fichier_apport, ligne); // entete

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

				throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_apport + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}

		//COUVERT NIVAL (EQUIVALENT EN EAU)
		if( (_sim_hyd._tempsol && !_sim_hyd._bLectBilan) || _sim_hyd._bRayonnementNet || (_sim_hyd._evapotranspiration && !_sim_hyd._bLectEvap && _sim_hyd.PrendreNomEvapotranspiration() == "LINACRE") )
		{
			_fichier_couvert_nival.open(_nom_fichier_couvert_nival);
			if (!_fichier_couvert_nival)
				throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; fichier couvert_nival.csv; [" + _nom_fichier_couvert_nival + "]. Ces donnees sont necessaire pour les modeles suivant: TempSol, Linacre, RayonnementNet (Penman, Penman-Monteith, Priestlay-Taylor).");

			_fichier_couvert_nival.exceptions(ios::failbit | ios::badbit);

			getline_mod(_fichier_couvert_nival, ligne); // commentaire
			getline_mod(_fichier_couvert_nival, ligne); // entete

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

					throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_couvert_nival + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
				}
			}
		}

		//HAUTEUR COUVERT NIVAL
		if(_sim_hyd._tempsol)
		{
			//fichier hauteur couvert nival
			_fichier_hauteur_neige.open(_nom_fichier_hauteur_neige);
			if (!_fichier_hauteur_neige)
				throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; fichier hauteur_neige.csv; [" + _nom_fichier_hauteur_neige + "]. Ces donnees sont necessaire pour le modele TempSol.");

			_fichier_hauteur_neige.exceptions(ios::failbit | ios::badbit);

			getline_mod(_fichier_hauteur_neige, ligne); // commentaire
			getline_mod(_fichier_hauteur_neige, ligne); // entete

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

					throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_hauteur_neige + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
				}
			}
		}

		//ALBEDO NEIGE
		if( _sim_hyd._bRayonnementNet || (_sim_hyd._evapotranspiration && !_sim_hyd._bLectEvap && _sim_hyd.PrendreNomEvapotranspiration() == "LINACRE") )
		{
			//fichier albedo neige
			_fichier_albedo_neige.open(_nom_fichier_albedo_neige);
			if (!_fichier_albedo_neige)
				throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; fichier albedo_neige.csv; [" + _nom_fichier_albedo_neige + "]. Ces donnees sont necessaire pour les modeles suivant: Linacre, RayonnementNet (Penman, Penman-Monteith, Priestlay-Taylor).");

			_fichier_albedo_neige.exceptions(ios::failbit | ios::badbit);

			getline_mod(_fichier_albedo_neige, ligne); // commentaire
			getline_mod(_fichier_albedo_neige, ligne); // entete

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

					throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_albedo_neige + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
				}
			}
		}

		FONTE_NEIGE::Initialise();
	}


	void LECTURE_FONTE_NEIGE::Calcule()
	{
		DATE_HEURE date_lue;
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		ZONES& zones = _sim_hyd.PrendreZones();

		string ligne;
		size_t x, index_zone, validation;
		char c;
		istringstream ss;
		ss.exceptions(ios::failbit | ios::badbit);

		//APPORT (PLUIE+NEIGE)
		do
		{
			getline_mod(_fichier_apport, ligne);
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
			ss >> c >> apport;

			if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
			{
				index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);
				zones[index_zone].ChangeApport(apport);
				++validation;
			}
		}

		if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
			throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_apport + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");

		//COUVERT NIVAL (EQUIVALENT EN EAU) mm
		if( (_sim_hyd._tempsol && !_sim_hyd._bLectBilan) || _sim_hyd._bRayonnementNet || (_sim_hyd._evapotranspiration && !_sim_hyd._bLectEvap && _sim_hyd.PrendreNomEvapotranspiration() == "LINACRE") )
		{
			do
			{
				getline_mod(_fichier_couvert_nival, ligne);
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
				float dval;
				ss >> c >> dval;

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);
					zones[index_zone].ChangeCouvertNival(dval);
					++validation;
				}
			}

			if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
				throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_couvert_nival + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		//HAUTEUR COUVERT NIVAL
		if(_sim_hyd._tempsol)	//si le modele de temperature du sol est simulé
		{
			do
			{
				getline_mod(_fichier_hauteur_neige, ligne);
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
				float dval;
				ss >> c >> dval;

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);
					zones[index_zone].ChangeHauteurCouvertNival(dval);
					++validation;
				}
			}

			if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
				throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_hauteur_neige + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		//ALBEDO NEIGE
		if( _sim_hyd._bRayonnementNet || (_sim_hyd._evapotranspiration && !_sim_hyd._bLectEvap && _sim_hyd.PrendreNomEvapotranspiration() == "LINACRE") )
		{
			do
			{
				getline_mod(_fichier_albedo_neige, ligne);
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
				float dval;
				ss >> c >> dval;

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);
					zones[index_zone].ChangeAlbedoNeige(dval);
					++validation;
				}
			}

			if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
				throw ERREUR_LECTURE_FICHIER("FONTE_NEIGE; mode lecture; " + _nom_fichier_albedo_neige + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		FONTE_NEIGE::Calcule();
	}


	void LECTURE_FONTE_NEIGE::Termine()
	{
		_fichier_apport.close();
		
		if(_fichier_couvert_nival.is_open())
			_fichier_couvert_nival.close();
		if(_fichier_hauteur_neige.is_open())
			_fichier_hauteur_neige.close();
		if(_fichier_albedo_neige.is_open())
			_fichier_albedo_neige.close();

		FONTE_NEIGE::Termine();
	}


	void LECTURE_FONTE_NEIGE::ChangeNbParams(const ZONES& /*zones*/)
	{
	}


	void LECTURE_FONTE_NEIGE::LectureParametres()
	{
		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
		{
			if(_sim_hyd.PrendreNomFonteNeige() == PrendreNomSousModele())
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_FONTE_NEIGE");
			else
				return;
		}

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_FONTE_NEIGE", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		string repertoire = _sim_hyd.PrendreRepertoireProjet();

		lire_cle_valeur(fichier, cle, valeur);
		if (valeur.size() != 0 && !Racine(valeur) )
			valeur = Combine(repertoire, valeur);
		ChangeNomFichierApport(valeur);

		if(lire_cle_valeur_try(fichier, cle, valeur))
		{
			if (valeur.size() != 0 && !Racine(valeur) )
				valeur = Combine(repertoire, valeur);
			ChangeNomFichierHauteurNeige(valeur);
		}
		else
			ChangeNomFichierHauteurNeige("");

		if(_nom_fichier_hauteur_neige.find('/') != string::npos)
		{
			_nom_fichier_couvert_nival = _nom_fichier_hauteur_neige.substr(0, _nom_fichier_hauteur_neige.find_last_of('/')) + "/couvert_nival.csv";
			_nom_fichier_albedo_neige = _nom_fichier_hauteur_neige.substr(0, _nom_fichier_hauteur_neige.find_last_of('/')) + "/albedo_neige.csv";
		}
	}


	void LECTURE_FONTE_NEIGE::SauvegardeParametres()
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

		fichier << "NOM FICHIER APPORT;" << PrendreRepertoireRelatif(repertoire_projet, PrendreNomFichierApport()) << endl;
		fichier << "NOM FICHIER HAUTEUR COUVERT NIVAL;" << PrendreRepertoireRelatif(repertoire_projet, PrendreNomFichierHauteurNeige()) << endl;
	}

}
