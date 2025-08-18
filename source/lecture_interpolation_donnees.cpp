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

#include "lecture_interpolation_donnees.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "constantes.hpp"
#include "version.hpp"

#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	LECTURE_INTERPOLATION_DONNEES::LECTURE_INTERPOLATION_DONNEES(SIM_HYD& sim_hyd)
		: INTERPOLATION_DONNEES(sim_hyd, "LECTURE INTERPOLATION DONNEES")
	{
	}


	LECTURE_INTERPOLATION_DONNEES::~LECTURE_INTERPOLATION_DONNEES()
	{
	}


	void LECTURE_INTERPOLATION_DONNEES::Initialise()
	{
		string ligne;

		//TMIN
		_fichier_tmin.open(_nom_fichier_tmin);
		if (!_fichier_tmin)
			throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier tmin.csv; " + _nom_fichier_tmin);

		_fichier_tmin.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_tmin, ligne); // commentaire
		getline_mod(_fichier_tmin, ligne); // entete

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

				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_tmin + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}

		//TMAX
		_fichier_tmax.open(_nom_fichier_tmax);
		if (!_fichier_tmax)
			throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier tmax.csv; " + _nom_fichier_tmax);

		_fichier_tmax.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_tmax, ligne); // commentaire
		getline_mod(_fichier_tmax, ligne); // entete

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

				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_tmax + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}

		//TMIN JOUR
		if(_sim_hyd.PrendreNomEvapotranspiration() == "HYDRO-QUEBEC" || _sim_hyd.PrendreNomEvapotranspiration() == "LINACRE" || _sim_hyd.PrendreNomEvapotranspiration() == "THORNTHWAITE")
		{
			_fichier_tmin_jour.open(_nom_fichier_tmin_jour);
			if (!_fichier_tmin_jour)
				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier tmin_jour.csv; " + _nom_fichier_tmin_jour);

			_fichier_tmin_jour.exceptions(ios::failbit | ios::badbit);

			getline_mod(_fichier_tmin_jour, ligne); // commentaire
			getline_mod(_fichier_tmin_jour, ligne); // entete

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

					throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_tmin_jour + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
				}
			}

			//TMAX JOUR
			_fichier_tmax_jour.open(_nom_fichier_tmax_jour);
			if (!_fichier_tmax_jour)
				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier tmax_jour.csv; " + _nom_fichier_tmax_jour);

			_fichier_tmax_jour.exceptions(ios::failbit | ios::badbit);

			getline_mod(_fichier_tmax_jour, ligne); // commentaire
			getline_mod(_fichier_tmax_jour, ligne); // entete

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

					throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_tmax_jour + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
				}
			}
		}

		//PLUIE
		_fichier_pluie.open(_nom_fichier_pluie);
		if (!_fichier_pluie)
			throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier pluie.csv; " + _nom_fichier_pluie);

		_fichier_pluie.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_pluie, ligne); // commentaire
		getline_mod(_fichier_pluie, ligne); // entete

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

				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_pluie + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}

		//NEIGE
		_fichier_neige.open(_nom_fichier_neige);
		if (!_fichier_neige)
			throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier neige.csv; " + _nom_fichier_neige);

		_fichier_neige.exceptions(ios::failbit | ios::badbit);

		getline_mod(_fichier_neige, ligne); // commentaire
		getline_mod(_fichier_neige, ligne); // entete

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

				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_neige + "; l`uhrh " + ligne + " est simule mais absent du fichier.");
			}
		}


		INTERPOLATION_DONNEES::Initialise();
	}


	void LECTURE_INTERPOLATION_DONNEES::Calcule()
	{
		size_t index_zone, validation;
		string sNomFichier;
		size_t x;

		try{

		// lecture temperatures
		{
			DATE_HEURE date_lue;
			DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

			ZONES& zones = _sim_hyd.PrendreZones();

			string ligne;
			char c;
			istringstream sstmin, sstmax;
			double tmin, tmax;
			float ftmin, ftmax;

			sNomFichier = _nom_fichier_tmin;

			sstmin.exceptions(ios::failbit | ios::badbit);
			sstmax.exceptions(ios::failbit | ios::badbit);

			do
			{
				getline_mod(_fichier_tmin, ligne);
				sstmin.clear();
				sstmin.str(ligne);

				unsigned short annee, mois, jour, heure, minute;
				sstmin >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

				date_lue = DATE_HEURE(annee, mois, jour, heure);
			}
			while (date_lue < date_courante);		

			sNomFichier = _nom_fichier_tmax;

			do
			{
				getline_mod(_fichier_tmax, ligne);
				sstmax.clear();
				sstmax.str(ligne);

				unsigned short annee, mois, jour, heure, minute;
				sstmax >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

				date_lue = DATE_HEURE(annee, mois, jour, heure);
			}
			while (date_lue < date_courante);		

			validation = 0;
			for (x=0; x<_vIDFichier.size(); x++)
			{
				sstmin >> c >> tmin;
				sstmax >> c >> tmax;

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);

					ftmin = static_cast<float>(tmin);
					ftmax = static_cast<float>(tmax);

					zones[index_zone].ChangeTemperature(ftmin, ftmax);
					++validation;
				}
			}

			if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier tmin/tmax; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		// lecture temperatures journalieres
		if(_sim_hyd.PrendreNomEvapotranspiration() == "HYDRO-QUEBEC" || _sim_hyd.PrendreNomEvapotranspiration() == "LINACRE" || _sim_hyd.PrendreNomEvapotranspiration() == "THORNTHWAITE")
		{
			DATE_HEURE date_lue;
			DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

			ZONES& zones = _sim_hyd.PrendreZones();

			string ligne;
			char c;
			istringstream sstmin, sstmax;
			double tmin, tmax;
			float ftmin, ftmax;

			sNomFichier = _nom_fichier_tmin_jour;

			sstmin.exceptions(ios::failbit | ios::badbit);
			sstmax.exceptions(ios::failbit | ios::badbit);

			do
			{
				getline_mod(_fichier_tmin_jour, ligne);
				sstmin.clear();
				sstmin.str(ligne);

				unsigned short annee, mois, jour, heure, minute;
				sstmin >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

				date_lue = DATE_HEURE(annee, mois, jour, heure);
			}
			while (date_lue < date_courante);		

			sNomFichier = _nom_fichier_tmax_jour;

			do
			{
				getline_mod(_fichier_tmax_jour, ligne);
				sstmax.clear();
				sstmax.str(ligne);

				unsigned short annee, mois, jour, heure, minute;
				sstmax >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

				date_lue = DATE_HEURE(annee, mois, jour, heure);
			}
			while (date_lue < date_courante);

			validation = 0;
			for (x=0; x<_vIDFichier.size(); x++)
			{
				sstmin >> c >> tmin;
				sstmax >> c >> tmax;

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);

					ftmin = static_cast<float>(tmin);
					ftmax = static_cast<float>(tmax);

					zones[index_zone].ChangeTemperatureJournaliere(ftmin, ftmax);
					++validation;
				}
			}

			if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; fichier tminjour/tmaxjour; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		// pluie
		{
			DATE_HEURE date_lue;
			DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

			ZONES& zones = _sim_hyd.PrendreZones();

			istringstream ss;
			double pluie;
			string ligne;
			float fPluie;
			char c;
			
			sNomFichier = _nom_fichier_pluie;

			ss.exceptions(ios::failbit | ios::badbit);

			do
			{
				getline_mod(_fichier_pluie, ligne);
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
				ss >> c >> pluie;

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);

					fPluie = static_cast<float>(pluie);
					zones[index_zone].ChangePluie(fPluie);
					++validation;
				}
			}

			if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_pluie + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		// neige
		{
			DATE_HEURE date_lue;
			DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

			ZONES& zones = _sim_hyd.PrendreZones();

			string ligne;
			char c;
			istringstream ss;

			sNomFichier = _nom_fichier_neige;

			ss.exceptions(ios::failbit | ios::badbit);

			do
			{
				getline_mod(_fichier_neige, ligne);
				ss.clear();
				ss.str(ligne);

				unsigned short annee, mois, jour, heure, minute;
				ss >> annee >> c >> mois >> c >> jour >> heure >> c >> minute;

				date_lue = DATE_HEURE(annee, mois, jour, heure);
			}
			while (date_lue < date_courante);

			double neige;
			float fneige, densite_neige, tmoy;

			validation = 0;
			for (x=0; x<_vIDFichier.size(); x++)
			{
				ss >> c >> neige;

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), _vIDFichier[x]) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_vIDFichier[x]);

					fneige = static_cast<float>(neige);

					//equivalent en eau de la neige -> hauteur de precipitation en neige
					if (_sim_hyd.PrendrePasDeTemps() == 1)
						densite_neige = CalculDensiteNeige(zones[index_zone].PrendreTMin()) / DENSITE_EAU;
					else
					{
						tmoy = (zones[index_zone].PrendreTMax() + zones[index_zone].PrendreTMin()) / 2.0f;
						densite_neige = CalculDensiteNeige(tmoy) / DENSITE_EAU;
					}

					fneige = fneige / densite_neige;
					zones[index_zone].ChangeNeige(fneige);
					++validation;
				}
			}

			if(validation != _sim_hyd.PrendreZonesSimulesIdent().size())
				throw ERREUR_LECTURE_FICHIER("INTERPOLATION_DONNEES; mode lecture; " + _nom_fichier_neige + "; fichier invalide; le nombre de donnees ne correspond pas avec l`entete du fichier.");
		}

		}
		catch(...)
		{
			throw ERREUR("INTERPOLATION::LECTURE; erreur lors de la lecture du fichier; " + sNomFichier);
		}

		INTERPOLATION_DONNEES::Calcule();
	}


	void LECTURE_INTERPOLATION_DONNEES::Termine()
	{
		_fichier_tmin.close();
		_fichier_tmax.close();
		if(_fichier_tmin_jour.is_open())
			_fichier_tmin_jour.close();
		if(_fichier_tmax_jour.is_open())
			_fichier_tmax_jour.close();
		_fichier_pluie.close();
		_fichier_neige.close();

		INTERPOLATION_DONNEES::Termine();
	}


	void LECTURE_INTERPOLATION_DONNEES::ChangeNbParams(const ZONES& /*zones*/)
	{
	}


	void LECTURE_INTERPOLATION_DONNEES::LectureParametres()
	{
		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
		{
			if(_sim_hyd.PrendreNomInterpolationDonnees() == PrendreNomSousModele())
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_INTERPOLATION_DONNEES");
			else
				return;
		}

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES LECTURE_INTERPOLATION_DONNEES", 1);

		getline_mod(fichier, ligne);				//ligne vide
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);				//ligne vide

		string repertoire = _sim_hyd.PrendreRepertoireProjet();

		lire_cle_valeur(fichier, cle, valeur);
		if (!Racine(valeur) )
			valeur = Combine(repertoire, valeur);
		_nom_fichier_tmin = valeur;

		lire_cle_valeur(fichier, cle, valeur);
		if (!Racine(valeur) )
			valeur = Combine(repertoire, valeur);
		_nom_fichier_tmax = valeur;

		lire_cle_valeur(fichier, cle, valeur);
		if(_sim_hyd.PrendrePasDeTemps() == 24)
			_nom_fichier_tmin_jour = _nom_fichier_tmin;
		else
		{
			if (!Racine(valeur) )
				valeur = Combine(repertoire, valeur);
			_nom_fichier_tmin_jour = valeur;
		}

		lire_cle_valeur(fichier, cle, valeur);
		if(_sim_hyd.PrendrePasDeTemps() == 24)
			_nom_fichier_tmax_jour = _nom_fichier_tmax;
		else
		{			
			if (!Racine(valeur) )
				valeur = Combine(repertoire, valeur);
			_nom_fichier_tmax_jour = valeur;
		}

		lire_cle_valeur(fichier, cle, valeur);
		if (!Racine(valeur) )
			valeur = Combine(repertoire, valeur);
		_nom_fichier_pluie = valeur;

		lire_cle_valeur(fichier, cle, valeur);
		if (!Racine(valeur) )
			valeur = Combine(repertoire, valeur);
		_nom_fichier_neige = valeur;
	}


	void LECTURE_INTERPOLATION_DONNEES::SauvegardeParametres()
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

		fichier << "NOM FICHIER TMIN;"		<< PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_tmin) << endl;
		fichier << "NOM FICHIER TMAX;"		<< PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_tmax) << endl;
		fichier << "NOM FICHIER TMIN_JOUR;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_tmin_jour) << endl;
		fichier << "NOM FICHIER TMAX_JOUR;" << PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_tmax_jour) << endl;
		fichier << "NOM FICHIER PLUIE;"		<< PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_pluie) << endl;
		fichier << "NOM FICHIER NEIGE;"		<< PrendreRepertoireRelatif(repertoire_projet, _nom_fichier_neige) << endl;
	}

}
