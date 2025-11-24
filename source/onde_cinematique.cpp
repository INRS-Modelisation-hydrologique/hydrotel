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

#include "onde_cinematique.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <fstream>
#include <regex>
#include <omp.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/thread/thread.hpp>


using namespace std;


namespace HYDROTEL
{

	ONDE_CINEMATIQUE::ONDE_CINEMATIQUE(SIM_HYD& sim_hyd)
		: RUISSELEMENT_SURFACE(sim_hyd, "ONDE CINEMATIQUE")
	{
		_pSim_hyd = &sim_hyd;

		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;

		_lame = 0.006f;
		_nb_debit = 0;
	}


	ONDE_CINEMATIQUE::~ONDE_CINEMATIQUE()
	{
	}


	double ONDE_CINEMATIQUE::PrendreLame() const
	{
		return _lame;
	}


	string ONDE_CINEMATIQUE::PrendreNomFichierHgm() const
	{
		return _nom_fichier_hgm;
	}


	vector<size_t> ONDE_CINEMATIQUE::PrendreIndexForets() const
	{
		return _index_forets;
	}


	vector<size_t> ONDE_CINEMATIQUE::PrendreIndexEaux() const
	{
		return _index_eaux;
	}


	double ONDE_CINEMATIQUE::PrendreManningForet(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _manning_forets.size());
		return _manning_forets[index_zone];
	}


	double ONDE_CINEMATIQUE::PrendreManningEaux(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _manning_eaux.size());
		return _manning_eaux[index_zone];
	}


	double ONDE_CINEMATIQUE::PrendreManningAutres(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _manning_autres.size());
		return _manning_autres[index_zone];
	}


	void ONDE_CINEMATIQUE::ChangeNomFichierLectureEtat(std::string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}


	void ONDE_CINEMATIQUE::ChangeRepertoireEcritureEtat(std::string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}


	void ONDE_CINEMATIQUE::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}


	void ONDE_CINEMATIQUE::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}


	string ONDE_CINEMATIQUE::PrendreNomFichierLectureEtat() const
	{
		return _nom_fichier_lecture_etat;
	}


	string ONDE_CINEMATIQUE::PrendreRepertoireEcritureEtat() const
	{
		return _repertoire_ecriture_etat;
	}


	bool ONDE_CINEMATIQUE::PrendreSauvegardeTousEtat() const
	{
		return _sauvegarde_tous_etat;
	}


	DATE_HEURE ONDE_CINEMATIQUE::PrendreDateHeureSauvegardeEtat() const
	{
		return _date_sauvegarde_etat;
	}


	void ONDE_CINEMATIQUE::LectureEtat(DATE_HEURE date_courante)
	{
		//_oc_zone
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + _nom_fichier_lecture_etat);

		vector<int> vValidation;
		string ligne, str;
		size_t index_zone;
		int iIdent;

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);

		while(!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne != "")
			{
				vector<double> valeurs = extrait_dvaleur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);
				iIdent = static_cast<int>(valeurs[0]);

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

					if(valeurs.size()-1 != _nb_debit)
					{
						fichier.close();
						ostringstream oss;
						oss.str("");
						oss << "RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + _nom_fichier_lecture_etat + "; nombre de valeur invalide; selon les parametres actuels, il devrait y avoir "  << _nb_debit << " pas de temps pour les debits.";
						throw ERREUR_LECTURE_FICHIER(oss.str());
					}

					for (size_t index = 0; index < _nb_debit; ++index)
						_oc_zone[index_zone].debits[index] = valeurs[index+1];

					vValidation.push_back(iIdent);
				}
			}
		}
		fichier.close();

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; id mismatch; " + _nom_fichier_lecture_etat);


		//_oc_surf
		if(_nom_fichier_lecture_etat.length() >= 15)
		{
			str = _nom_fichier_lecture_etat;
			str = str.insert(str.length()-14, "surf_");
			if(boost::filesystem::exists(str))
			{
				ifstream fichier2(str);
				if (!fichier2)
					throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + str);

				vValidation.clear();

				getline_mod(fichier2, ligne);
				getline_mod(fichier2, ligne);
				getline_mod(fichier2, ligne);
				getline_mod(fichier2, ligne);

				while(!fichier2.eof())
				{
					getline_mod(fichier2, ligne);
					if(ligne != "")
					{
						vector<double> valeurs = extrait_dvaleur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);
						iIdent = static_cast<int>(valeurs[0]);

						if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
						{
							index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

							if(valeurs.size()-1 != _nb_debit)
							{
								fichier2.close();
								ostringstream oss;
								oss.str("");
								oss << "RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + str + "; nombre de valeur invalide; selon les parametres actuels, il devrait y avoir "  << _nb_debit << " pas de temps pour les debits.";
								throw ERREUR_LECTURE_FICHIER(oss.str());
							}

							for (size_t index = 0; index < _nb_debit; ++index)
								_oc_surf[index_zone].debits[index] = valeurs[index+1];

							vValidation.push_back(iIdent);
						}
					}
				}
				fichier2.close();

				std::sort(vValidation.begin(), vValidation.end());
				if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
					throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; id mismatch; " + str);
			}
		}

		//_oc_hypo
		if(_nom_fichier_lecture_etat.length() >= 15)
		{
			str = _nom_fichier_lecture_etat;
			str = str.insert(str.length()-14, "hypo_");
			if(boost::filesystem::exists(str))
			{
				ifstream fichier3(str);
				if (!fichier3)
					throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + str);

				vValidation.clear();

				getline_mod(fichier3, ligne);
				getline_mod(fichier3, ligne);
				getline_mod(fichier3, ligne);
				getline_mod(fichier3, ligne);

				while(!fichier3.eof())
				{
					getline_mod(fichier3, ligne);
					if(ligne != "")
					{
						vector<double> valeurs = extrait_dvaleur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);
						iIdent = static_cast<int>(valeurs[0]);

						if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
						{
							index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

							if(valeurs.size()-1 != _nb_debit)
							{
								fichier3.close();
								ostringstream oss;
								oss.str("");
								oss << "RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + str + "; nombre de valeur invalide; selon les parametres actuels, il devrait y avoir "  << _nb_debit << " pas de temps pour les debits.";
								throw ERREUR_LECTURE_FICHIER(oss.str());
							}

							for (size_t index = 0; index < _nb_debit; ++index)
								_oc_hypo[index_zone].debits[index] = valeurs[index+1];

							vValidation.push_back(iIdent);
						}
					}
				}
				fichier3.close();

				std::sort(vValidation.begin(), vValidation.end());
				if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
					throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; id mismatch; " + str);
			}
		}


		//_oc_base
		if(_nom_fichier_lecture_etat.length() >= 15)
		{
			str = _nom_fichier_lecture_etat;
			str = str.insert(str.length()-14, "base_");
			if(boost::filesystem::exists(str))
			{
				ifstream fichier4(str);
				if (!fichier4)
					throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + str);

				vValidation.clear();

				getline_mod(fichier4, ligne);
				getline_mod(fichier4, ligne);
				getline_mod(fichier4, ligne);
				getline_mod(fichier4, ligne);

				while(!fichier4.eof())
				{
					getline_mod(fichier4, ligne);
					if(ligne != "")
					{
						vector<double> valeurs = extrait_dvaleur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);
						iIdent = static_cast<int>(valeurs[0]);

						if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
						{
							index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

							if(valeurs.size()-1 != _nb_debit)
							{
								fichier4.close();
								ostringstream oss;
								oss.str("");
								oss << "RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; " + str + "; nombre de valeur invalide; selon les parametres actuels, il devrait y avoir "  << _nb_debit << " pas de temps pour les debits.";
								throw ERREUR_LECTURE_FICHIER(oss.str());
							}

							for (size_t index = 0; index < _nb_debit; ++index)
								_oc_base[index_zone].debits[index] = valeurs[index+1];

							vValidation.push_back(iIdent);
						}
					}
				}
				fichier4.close();

				std::sort(vValidation.begin(), vValidation.end());
				if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
					throw ERREUR_LECTURE_FICHIER("RUISSELEMENT_SURFACE; fichier etat ONDE_CINEMATIQUE; id mismatch; " + str);
			}
		}
	}


	void ONDE_CINEMATIQUE::SauvegardeEtat(DATE_HEURE date_courante) const
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier, oss;
		size_t x, nbSimuler, index_zone;
		string str;

		string sSep = _sim_hyd._output._sFichiersEtatsSeparator;

		if(!RepertoireExiste(_repertoire_ecriture_etat))
			CreeRepertoire(_repertoire_ecriture_etat);

		ZONES& zones = _sim_hyd.PrendreZones();
		nbSimuler = _sim_hyd.PrendreZonesSimules().size();

		//_oc_zone
		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			        << "ruisselement_surface_" 
			        << setw(4) << date_courante.PrendreAnnee() 
			        << setw(2) << date_courante.PrendreMois() 
			        << setw(2) << date_courante.PrendreJour() 
			        << setw(2) << date_courante.PrendreHeure() 
					<< ".csv";

		ofstream fichier(nom_fichier.str());
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier.exceptions(ios::failbit | ios::badbit);

		fichier << "ETATS RUISSELEMENT SURFACE" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier << "DATE_HEURE" << sSep << date_courante << endl;
		fichier << endl;

		fichier << "UHRH" << sSep << "DEBITS" << endl;
		
		x = 0;
		for (index_zone=0; x<nbSimuler; index_zone++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				oss.str("");
				oss << zone.PrendreIdent() << sSep;

				oss << setprecision(21) << setiosflags(ios::fixed);

				for (size_t index = 0; index < _nb_debit; ++index)
					oss << _oc_zone[index_zone].debits[index] << sSep;

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				fichier << str << endl;
				++x;
			}
		}
		fichier.close();


		//_oc_surf
		nom_fichier.str("");
		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			<< "ruisselement_surface_surf_" 
			<< setw(4) << date_courante.PrendreAnnee() 
			<< setw(2) << date_courante.PrendreMois() 
			<< setw(2) << date_courante.PrendreJour() 
			<< setw(2) << date_courante.PrendreHeure() 
			<< ".csv";

		ofstream fichier2(nom_fichier.str());
		if (!fichier2)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier2.exceptions(ios::failbit | ios::badbit);

		fichier2 << "ETATS RUISSELEMENT SURFACE (oc_surf)" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier2 << "DATE_HEURE" << sSep << date_courante << endl;
		fichier2 << endl;

		fichier2 << "UHRH" << sSep << "DEBITS" << endl;

		x = 0;
		for (index_zone=0; x<nbSimuler; index_zone++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				oss.str("");
				oss << zone.PrendreIdent() << sSep;

				oss << setprecision(21) << setiosflags(ios::fixed);

				for (size_t index = 0; index < _nb_debit; ++index)
					oss << _oc_surf[index_zone].debits[index] << sSep;

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				fichier2 << str << endl;
				++x;
			}
		}
		fichier2.close();


		//_oc_hypo
		nom_fichier.str("");
		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			<< "ruisselement_surface_hypo_" 
			<< setw(4) << date_courante.PrendreAnnee() 
			<< setw(2) << date_courante.PrendreMois() 
			<< setw(2) << date_courante.PrendreJour() 
			<< setw(2) << date_courante.PrendreHeure() 
			<< ".csv";

		ofstream fichier3(nom_fichier.str());
		if (!fichier3)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier3.exceptions(ios::failbit | ios::badbit);

		fichier3 << "ETATS RUISSELEMENT SURFACE (oc_hypo)" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier3 << "DATE_HEURE" << sSep << date_courante << endl;
		fichier3 << endl;

		fichier3 << "UHRH" << sSep << "DEBITS" << endl;

		x = 0;
		for (index_zone=0; x<nbSimuler; index_zone++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				oss.str("");
				oss << zone.PrendreIdent() << sSep;

				oss << setprecision(21) << setiosflags(ios::fixed);

				for (size_t index = 0; index < _nb_debit; ++index)
					oss << _oc_hypo[index_zone].debits[index] << sSep;

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				fichier3 << str << endl;
				++x;
			}
		}
		fichier3.close();


		//_oc_base
		nom_fichier.str("");
		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			<< "ruisselement_surface_base_" 
			<< setw(4) << date_courante.PrendreAnnee() 
			<< setw(2) << date_courante.PrendreMois() 
			<< setw(2) << date_courante.PrendreJour() 
			<< setw(2) << date_courante.PrendreHeure() 
			<< ".csv";

		ofstream fichier4(nom_fichier.str());
		if (!fichier4)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier4.exceptions(ios::failbit | ios::badbit);

		fichier4 << "ETATS RUISSELEMENT SURFACE (oc_base)" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier4 << "DATE_HEURE" << sSep << date_courante << endl;
		fichier4 << endl;

		fichier4 << "UHRH" << sSep << "DEBITS" << endl;

		x = 0;
		for (index_zone=0; x<nbSimuler; index_zone++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				oss.str("");
				oss << zone.PrendreIdent() << sSep;

				oss << setprecision(21) << setiosflags(ios::fixed);

				for (size_t index = 0; index < _nb_debit; ++index)
					oss << _oc_base[index_zone].debits[index] << sSep;

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				fichier4 << str << endl;
				++x;
			}
		}
		fichier4.close();
	}


	void ONDE_CINEMATIQUE::Initialise()
	{
		auto& occupation_sol = _sim_hyd.PrendreOccupationSol();
		const size_t nb_zone = _sim_hyd.PrendreZones().PrendreNbZone();

		vector<size_t> index_autres;
		for (size_t index = 0; index < occupation_sol.PrendreNbClasse(); ++index)
		{
			if (find(begin(_index_eaux), end(_index_eaux), index) == end(_index_eaux) &&
				find(begin(_index_forets), end(_index_forets), index) == end(_index_forets))
				index_autres.push_back(index);
		}
		index_autres.shrink_to_fit();
		_index_autres.swap(index_autres);

		// calcul pourcentage des classes integrees

		_pourcentage_eaux.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			double pourcentage = 0.0;

			for (auto index = begin(_index_eaux); index != end(_index_eaux); ++index)
				pourcentage += occupation_sol.PrendrePourcentage_double(index_zone, *index);

			_pourcentage_eaux[index_zone] = pourcentage;
		}

		_pourcentage_forets.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			double pourcentage = 0.0;

			for (auto index = begin(_index_forets); index != end(_index_forets); ++index)
				pourcentage += occupation_sol.PrendrePourcentage_double(index_zone, *index);

			_pourcentage_forets[index_zone] = pourcentage;
		}

		_pourcentage_autres.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
			_pourcentage_autres[index_zone] = max(1.0 - (_pourcentage_forets[index_zone] + _pourcentage_eaux[index_zone]), 0.0);

		//obtient la liste des fichiers hgm disponible
		vector<string> listHGM;
		string srcDir, str, sHGMini;
		bool ok;

		size_t x = _nom_fichier_hgm.find_last_of('/');
		if(x != string::npos && x < _nom_fichier_hgm.size()-1)
			srcDir = _nom_fichier_hgm.substr(0, x);

		if(srcDir != "")
		{
			boost::filesystem::path source(srcDir);
			if(boost::filesystem::exists(source))
			{
				for(boost::filesystem::directory_iterator file(source); file != boost::filesystem::directory_iterator(); ++file)
				{
					try
					{
						boost::filesystem::path current(file->path());
						if(!boost::filesystem::is_directory(current))
						{
							boost::filesystem::path fp = current.extension();
							str = fp.string();
							boost::algorithm::to_lower(str);
						
							if(str == ".hgm")
							{
								str = current.string();
								std::replace(str.begin(), str.end(), '\\', '/');
								listHGM.push_back(str);
							}
						}					
					}
					catch(const boost::filesystem::filesystem_error& e)
					{
						e.what();
						break;
					}
				}
			}
			else
				throw ERREUR("The folder specified for the HGM file is invalid.");
		}

		vector<string> listHGM2(listHGM);
		ok = false;

		try{
		LectureHgm();
		ok = true;
		}
		catch(ERREUR&)
		{
			auto it = find(begin(listHGM), end(listHGM), _nom_fichier_hgm);
			if(it != end(listHGM))
				listHGM.erase(it);

			sHGMini = _nom_fichier_hgm;
			while(!ok && listHGM.size() > 0)
			{
				_nom_fichier_hgm = listHGM[listHGM.size()-1];
				listHGM.pop_back();

				try{
				LectureHgm();
				ok = true;
				}
				catch(const ERREUR& err)
				{
					Log("");
					Log(err.what());
				}
			}
		}
		
		if(!ok)
		{
			time_t debut, fin;
			std::time(&debut);

			//std::cout << endl;
			//std::cout << "calcul de l'hydrogramme geomorphologique...     " << flush;

			_nom_fichier_hgm = sHGMini;
			auto it = find(begin(listHGM2), end(listHGM2), _nom_fichier_hgm);
			
			ostringstream oss;	

			int i=2;
			x = sHGMini.find_last_of('/');
			while(it != end(listHGM2))
			{
				_nom_fichier_hgm = sHGMini.substr(0, x+1);
				_nom_fichier_hgm += "hydrogramme";

				oss.str("");
				oss << i;
				_nom_fichier_hgm+= oss.str() + ".hgm";

				it = find(begin(listHGM2), end(listHGM2), _nom_fichier_hgm);
				++i;
			}

			CalculeHgm();
			SauvegardeHgm();

			std::time(&fin);
			oss.str("");

			if((fin - debut)/60.0/60.0 < 1.0)
			{
				if((fin - debut)/60.0 < 1.0)
					oss << "   completed in " << setprecision(0) << setiosflags(ios::fixed) << (fin - debut) << " sec";
				else
					oss << "   completed in " << setprecision(2) << setiosflags(ios::fixed) << (fin - debut) / 60.0 << " min";
			}
			else
				oss << "   completed in " << setprecision(2) << setiosflags(ios::fixed) << (fin - debut) / 60.0 / 60.0 << " h";

			std::cout << oss.str() << flush;

			_listLog[_listLog.size()-1]+= oss.str();

			_sim_hyd._bHGMCalculer = true;
		}
		else
		{
			if(_nom_fichier_hgm != sHGMini)
			{
				if(_sim_hyd._fichierParametreGlobal)
				{
					//TODO maj du fichier parametre global
				}
				else
					SauvegardeParametres();
			}
		}

		if(_nb_debit == 0)
			throw ERREUR("Error: hgm file is invalid (flows number equal 0): " + _nom_fichier_hgm);

		if(!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		RUISSELEMENT_SURFACE::Initialise();
	}


	void ONDE_CINEMATIQUE::Calcule()
	{
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();
		ZONES& zones = _sim_hyd.PrendreZones();
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		TRONCON* troncon;
		size_t nb_pixel, index_zone, index, j;
		double dPdts, m2_sec, resolution, production, prod_surf, prod_hypo, prod_base, tmp1, tmp2;
		float apport;
		int pdts;

		pdts = _sim_hyd.PrendrePasDeTemps() * 60 * 60;
		dPdts = static_cast<double>(pdts);

		resolution = static_cast<double>(zones.PrendreResolution());

		for(index = 0; index < index_zones.size(); index++)
		{
			index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			production = static_cast<double>(zone.PrendreProductionTotal()) / 1000.0; // mm ====> m

			prod_surf = static_cast<double>(zone.PrendreProdSurf()) / 1000.0;	//mm -> m
			prod_hypo = static_cast<double>(zone.PrendreProdHypo()) / 1000.0;	//mm -> m
			prod_base = static_cast<double>(zone.PrendreProdBase()) / 1000.0;	//mm -> m

			switch (zone.PrendreTypeZone())
			{
			case ZONE::SOUS_BASSIN:

				for(j=0; j<_nb_debit; j++)
				{						
					tmp1 = _oc_zone[index_zone].debits[j];
					tmp2 = _oc_zone[index_zone].distri[j];
					tmp1 = tmp1 + tmp2 * production / _lame;
					_oc_zone[index_zone].debits[j] = tmp1;

					tmp1 = _oc_surf[index_zone].debits[j];
					tmp2 = _oc_surf[index_zone].distri[j];
					tmp1 = tmp1 + tmp2 * prod_surf / _lame;
					_oc_surf[index_zone].debits[j] = tmp1;

					tmp1 = _oc_hypo[index_zone].debits[j];
					tmp2 = _oc_hypo[index_zone].distri[j];
					tmp1 = tmp1 + tmp2 * prod_hypo / _lame;
					_oc_hypo[index_zone].debits[j] = tmp1;

					tmp1 = _oc_base[index_zone].debits[j];
					tmp2 = _oc_base[index_zone].distri[j];
					tmp1 = tmp1 + tmp2 * prod_base / _lame;
					_oc_base[index_zone].debits[j] = tmp1;
				}
				break;

			case ZONE::LAC:

				nb_pixel = zone.PrendreNbPixel();

				m2_sec = static_cast<double>(nb_pixel) * resolution * resolution / dPdts;

				_oc_zone[index_zone].debits[0] = m2_sec * production;

				_oc_surf[index_zone].debits[0] = _oc_zone[index_zone].debits[0];
				_oc_hypo[index_zone].debits[0] = 0.0;
				_oc_base[index_zone].debits[0] = 0.0;
				break;

			default:
				throw ERREUR("invalid rhhu type");
			}

			troncon = zone.PrendreTronconAval();

			zone._apport_lateral_uhrh = static_cast<float>(_oc_zone[index_zone].debits[0]);

			apport = troncon->PrendreApportLateral() + zone._apport_lateral_uhrh;
			troncon->ChangeApportLateral(max(0.0f, apport));

			troncon->_surf = max(0.0f, troncon->_surf + static_cast<float>(_oc_surf[index_zone].debits[0]));
			troncon->_hypo = max(0.0f, troncon->_hypo + static_cast<float>(_oc_hypo[index_zone].debits[0]));
			troncon->_base = max(0.0f, troncon->_base + static_cast<float>(_oc_base[index_zone].debits[0]));

			zone._ecoulementSurf = static_cast<float>(_oc_surf[index_zone].debits[0]);
			zone._ecoulementHypo = static_cast<float>(_oc_hypo[index_zone].debits[0]);
			zone._ecoulementBase = static_cast<float>(_oc_base[index_zone].debits[0]);

			// decale les debits
			for(j=0; j<_nb_debit-1; j++)
			{
				_oc_zone[index_zone].debits[j] = _oc_zone[index_zone].debits[j + 1];

				_oc_surf[index_zone].debits[j] = _oc_surf[index_zone].debits[j + 1];
				_oc_hypo[index_zone].debits[j] = _oc_hypo[index_zone].debits[j + 1];
				_oc_base[index_zone].debits[j] = _oc_base[index_zone].debits[j + 1];
			}
			
			_oc_zone[index_zone].debits[_nb_debit - 1] = 0.0;

			_oc_surf[index_zone].debits[_nb_debit - 1] = 0.0;
			_oc_hypo[index_zone].debits[_nb_debit - 1] = 0.0;
			_oc_base[index_zone].debits[_nb_debit - 1] = 0.0;
		}

		if (_sauvegarde_tous_etat || (_sauvegarde_etat && _date_sauvegarde_etat - pas_de_temps == date_courante))
			SauvegardeEtat(date_courante);

		RUISSELEMENT_SURFACE::Calcule();
	}


	void ONDE_CINEMATIQUE::Termine()
	{
		_pourcentage_forets.clear();
		_pourcentage_eaux.clear();
		_pourcentage_autres.clear();

		_oc_zone.clear();

		_oc_surf.clear();
		_oc_hypo.clear();
		_oc_base.clear();

		RUISSELEMENT_SURFACE::Termine();
	}


	void ONDE_CINEMATIQUE::ChangeNbParams(const ZONES& zones)
	{
		const size_t nb_zone = zones.PrendreNbZone();

		_manning_forets.resize(nb_zone, 0.3);
		_manning_eaux.resize(nb_zone, 0.03);
		_manning_autres.resize(nb_zone, 0.1);
	}


	void ONDE_CINEMATIQUE::SauvegardeHgm()
	{
		string repertoire = PrendreRepertoire(_nom_fichier_hgm);

		if (!RepertoireExiste(repertoire))
			CreeRepertoire(repertoire);

		ofstream fichier(_nom_fichier_hgm);
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(_nom_fichier_hgm);

		fichier << "HGM HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "PAS DE TEMPS;" << _sim_hyd.PrendrePasDeTemps() << endl;
		fichier << endl;

		fichier << "LAME;" << _lame << endl;
		fichier << endl;

		fichier << "CLASSE INTEGRE FORETS;";
		auto index_foret = PrendreIndexForets();
		for (auto iter = begin(index_foret); iter != end(index_foret); ++iter)
			fichier << *iter + 1 << ';';
		fichier << endl;

		fichier << "CLASSE INTEGRE EAUX;";
		auto index_eaux = PrendreIndexEaux();
		for (auto iter = begin(index_eaux); iter != end(index_eaux); ++iter)
			fichier << *iter + 1 << ';';
		fichier << endl << endl;

		ZONES& zones = _sim_hyd.PrendreZones();
		const size_t nb_zone = zones.PrendreNbZone();

		fichier << "UHRH; MANNING FORETS; MANNING EAUX; MANNING AUTRES" << endl;
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			fichier << zones[index_zone].PrendreIdent() << ';'
				<< _manning_forets[index_zone] << ';'
				<< _manning_eaux[index_zone] << ';'
				<< _manning_autres[index_zone] << endl;
		}
		fichier << endl;

		fichier << "NOMBRE DE DEBITS;" << _nb_debit << endl;
		fichier << endl;

		fichier << "UHRH";
		for (size_t n = 1; n <= _nb_debit; ++n)
		{
			fichier << ";DISTRI " << n;
		}
		fichier << endl;

		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			fichier << zones[index_zone].PrendreIdent();
			for (size_t index_distri = 0; index_distri < _nb_debit; ++index_distri)
			{
				fichier << ';' << _oc_zone[index_zone].distri[index_distri];
			}
			fichier << endl;
		}
	}


	void ONDE_CINEMATIQUE::LectureHgm()
	{
		ifstream fichier(_nom_fichier_hgm);

		if (!fichier)
			throw ERREUR("The specified HGM file does not exist");

		fichier.exceptions(ios::failbit | ios::badbit);

		string ligne;
		getline_mod(fichier, ligne);

		fichier.close();

		if (regex_match(ligne, regex("HGM HYDROTEL VERSION(.*)")))	//determine le type de fichier
		{
			// lecture de la nouvelle version avec les parametres
			ifstream fichier2(_nom_fichier_hgm);

			if (!fichier2)
				throw ERREUR("The specified HGM file does not exist");

			fichier2.exceptions(ios::failbit | ios::badbit);

			try
			{
				string cle, valeur, version;
				lire_cle_valeur(fichier2, cle, version);
				getline_mod(fichier2, ligne);

				lire_cle_valeur(fichier2, cle, valeur); // pas de temps
				getline_mod(fichier2, ligne);

				if (_sim_hyd.PrendrePasDeTemps() != string_to_ushort(valeur))
					throw ERREUR("the timestep of the HGM file is different from the timestep of the simulation");

				lire_cle_valeur(fichier2, cle, valeur); // lame
				getline_mod(fichier2, ligne);

				if (_lame != string_to_double(valeur))
					throw ERREUR("the depth specified in the HGM file is different from the depth specified in the simulation file for the kinematic wave model");

				// lecture des classes integrees
				lire_cle_valeur(fichier2, cle, valeur); // forets
				vector<size_t> index_forets = extrait_svaleur(valeur, ";");
			
				// zero base index
				for (auto iter = begin(index_forets); iter != end(index_forets); ++iter)
					*iter -= 1; 
			
				if (index_forets != _index_forets)
					throw ERREUR("the forest classes of the HGM file are different from the classes specified in the simulation file for the kinematic wave model");

				lire_cle_valeur(fichier2, cle, valeur); // eaux
				vector<size_t> index_eaux = extrait_svaleur(valeur, ";");

				// zero base index
				for (auto iter = begin(index_eaux); iter != end(index_eaux); ++iter)
					*iter -= 1; 
			
				if (index_eaux != _index_eaux)
					throw ERREUR("the water classes of the HGM file are different from the classes specified in the simulation file for the kinematic wave model");

				getline_mod(fichier2, ligne);
			
				ZONES& zones = _sim_hyd.PrendreZones();
				const size_t nb_zone = zones.PrendreNbZone();

				getline_mod(fichier2, ligne); // commentaires
				for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
				{
					char c;
					int ident;
					double manning_foret, manning_eau, manning_autre;
					fichier2 
						>> ident >> c
						>> manning_foret >> c
						>> manning_eau >> c
						>> manning_autre;

					size_t index = zones.IdentVersIndex(ident);

					if (_manning_forets[index] != manning_foret)
						throw ERREUR("the `forest` class manning value of the HGM file are different from the classes specified in the simulation file for the kinematic wave model");

					if (_manning_eaux[index] != manning_eau)
						throw ERREUR("the `water` class manning value of the HGM file are different from the classes specified in the simulation file for the kinematic wave model");

					if (_manning_autres[index] != manning_autre)
						throw ERREUR("the `other` class manning value of the HGM file are different from the classes specified in the simulation file for the kinematic wave model");
				}
				getline_mod(fichier2, ligne);

				lire_cle_valeur(fichier2, cle, valeur); // nombre de debit
				getline_mod(fichier2, ligne);

				istringstream iss(valeur);
				iss >> _nb_debit;

				vector<oc_zone> oc_zone(nb_zone);

				getline_mod(fichier2, ligne); // commentaires
				for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
				{
					oc_zone[index_zone].debits.resize(_nb_debit);
					oc_zone[index_zone].distri.resize(_nb_debit);

					getline_mod(fichier2, ligne);
					auto val = extrait_fvaleur(ligne, ";");

					if (val.size() != _nb_debit + 1)
						throw ERREUR_LECTURE_FICHIER("FICHIER HGM; " + _nom_fichier_hgm);

					for (size_t index_distri = 0; index_distri < _nb_debit; ++index_distri)
					{
						oc_zone[index_zone].distri[index_distri] = val[index_distri + 1];
					}
				}

				_oc_zone.swap(oc_zone);

				_oc_surf = _oc_zone;
				_oc_hypo = _oc_zone;
				_oc_base = _oc_zone;
			}
			catch(const ERREUR& err)
			{
				throw err;
			}
			catch(...)
			{
				throw ERREUR_LECTURE_FICHIER("FICHIER HGM; " + _nom_fichier_hgm);
			}
		}
		else
		{
			//lecture de l'ancienne version
			ifstream fichier3(_nom_fichier_hgm);

			if (!fichier3)
				throw ERREUR_LECTURE_FICHIER("FICHIER HGM; " + _nom_fichier_hgm);

			fichier3.exceptions(ios::failbit | ios::badbit);

			try
			{
				const size_t nb_zone = _sim_hyd.PrendreZones().PrendreNbZone();

				fichier3 >> _nb_debit;

				vector<oc_zone> oc_zone(nb_zone);
				for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
				{
					oc_zone[index_zone].debits.resize(_nb_debit);
					oc_zone[index_zone].distri.resize(_nb_debit);

					int ident;
					fichier3 >> ident;

					for (size_t index_distri = 0; index_distri < _nb_debit; ++index_distri)
						fichier3 >> oc_zone[index_zone].distri[index_distri];
				}
				
				_oc_zone.swap(oc_zone);

				_oc_surf = _oc_zone;
				_oc_hypo = _oc_zone;
				_oc_base = _oc_zone;
			}
			catch(...)
			{
				throw ERREUR_LECTURE_FICHIER("FICHIER HGM; " + _nom_fichier_hgm);
			}
		}
	}


	void ONDE_CINEMATIQUE::CalculeHgm()
	{
		ostringstream oss;
		double DIV1, POW1, MULT1, EQ1, EQ2;
		double man;
		string str;
		size_t ligne, colonne, idx3, idxZone, stMaxdeb, z;
		int uhrhNoData, ident, tmax, maxThread, nbThread, maxdeb;

		vector<string> vErr;
		vector<int> vTmax;

		std::cout << endl;
		std::cout << "Computing geomorphological hydrograph " << flush;

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();
		ZONES& zones = _sim_hyd.PrendreZones();
		
		int pdts = _sim_hyd.PrendrePasDeTemps() * 60 * 60;		
		const int nb_zone = static_cast<int>(zones.PrendreNbZone());

		if(zones._pRasterUhrhId == nullptr)
		{
			if(zones._nom_fichier_zoneTemp != "")
				str = zones._nom_fichier_zoneTemp;
			else
				str = zones._nom_fichier_zone;

			zones._pRasterUhrhId = new RasterInt2();
			if(!zones._pRasterUhrhId->Open(str))
			{
				_listLog.push_back("Computing geomorphological hydrograph");
				throw ERREUR(zones._pRasterUhrhId->_sError);
			}
		}

		const size_t nb_ligne = zones._pRasterUhrhId->_ySize;
		const size_t nb_colonne = zones._pRasterUhrhId->_xSize;

		uhrhNoData = zones._pRasterUhrhId->_noData;

		if(_sim_hyd._pRasterOri == nullptr)
		{
			_sim_hyd._pRasterOri = new RasterInt2();
			if(!_sim_hyd._pRasterOri->Open(zones.PrendreNomFichierOrientation()))
			{
				_listLog.push_back("Computing geomorphological hydrograph");
				throw ERREUR(_sim_hyd._pRasterOri->_sError);
			}
		}

		if(_sim_hyd._pRasterPente == nullptr)
		{
			_sim_hyd._pRasterPente = new RasterDouble2();
			if(!_sim_hyd._pRasterPente->Open(zones.PrendreNomFichierPente()))
			{
				_listLog.push_back("Computing geomorphological hydrograph");
				throw ERREUR(_sim_hyd._pRasterPente->_sError);
			}
		}

		double resolution = _sim_hyd._pRasterPente->_geotransform[1];	//pentes.PrendreTailleCelluleX();

		DIV1 = 5.0/3.0;
		POW1 = pow(_lame, 2.0/3.0);
		MULT1 = _lame * resolution * resolution;
		EQ1 = _lame / (double)pdts * resolution;
		EQ2 = 2.0/3.0;

		vector<rectangle> eng_zones(nb_zone);	//limite des zones
		if(eng_zones[0].nb_car != 0)	//validation pour s'assurer que nb_car a été initialisé à 0
		{
			_listLog.push_back("Computing geomorphological hydrograph");
			throw ERREUR("CalculeHGM: error: eng_zones[0].nb_car != 0");
		}
		
		double* manning = new double[nb_colonne*nb_ligne]();	//Zero-initialized -> ()

		for(ligne=0; ligne!=nb_ligne; ligne++)
		{
			for(colonne=0; colonne!=nb_colonne; colonne++)
			{
				idx3 = ligne * nb_colonne + colonne;

				//on s'assure que les pentes sont valides
				_sim_hyd._pRasterPente->_values[idx3] = max(2.5, _sim_hyd._pRasterPente->_values[idx3]);

				//limite des zones
				ident = zones._pRasterUhrhId->_values[idx3];

				if(ident != uhrhNoData)
				{
					idxZone = zones.IdentVersIndex(ident);

					if(eng_zones[idxZone].nb_car == 0)
					{
						eng_zones[idxZone].min_col = colonne;
						eng_zones[idxZone].min_lig = ligne;
						eng_zones[idxZone].max_col = colonne;
						eng_zones[idxZone].max_lig = ligne;
						eng_zones[idxZone].nb_car++;
					}
					else
					{
						eng_zones[idxZone].min_col = min(colonne, eng_zones[idxZone].min_col);
						eng_zones[idxZone].min_lig = min(ligne, eng_zones[idxZone].min_lig);
						eng_zones[idxZone].max_col = max(colonne, eng_zones[idxZone].max_col);
						eng_zones[idxZone].max_lig = max(ligne, eng_zones[idxZone].max_lig);
						eng_zones[idxZone].nb_car++;
					}

					//matrice de manning
					if(zones[idxZone].PrendreTypeZone() == ZONE::TYPE_ZONE::SOUS_BASSIN)	//si ce n'est pas un uhrh lac
					{
						man = _pourcentage_forets[idxZone] * _manning_forets[idxZone] + _pourcentage_eaux[idxZone] * _manning_eaux[idxZone] + _pourcentage_autres[idxZone] * _manning_autres[idxZone];
						manning[idx3] = man;
					}
				}
			}
		}

		vector<oc_zone> oc_zone(zones.PrendreNbZone());

		_oc_surf.resize(zones.PrendreNbZone());
		_oc_hypo.resize(zones.PrendreNbZone());
		_oc_base.resize(zones.PrendreNbZone());

		maxdeb = 240 / _sim_hyd.PrendrePasDeTemps();		//10 jours si pdt 24h
		stMaxdeb = static_cast<size_t>(maxdeb);

		for(auto iter = begin(oc_zone); iter!=end(oc_zone); iter++)
		{
			iter->debits.resize(stMaxdeb);
			iter->distri.resize(stMaxdeb);
		}

		vErr.resize(zones.PrendreNbZone(), "");
		vTmax.resize(zones.PrendreNbZone(), 0);

		//omp_set_dynamic(0);     //explicitly disable dynamic teams
		//omp_set_num_threads(n); //use n threads for all consecutive parallel regions

		maxThread = omp_get_max_threads();

		if(_sim_hyd._nbThread == 0)
			nbThread = maxThread;
		else
		{
			if(_sim_hyd._nbThread > maxThread)
				nbThread = maxThread;
			else
				nbThread = _sim_hyd._nbThread;
		}

		omp_set_num_threads(nbThread);

		#pragma omp parallel
		{

		if(omp_get_thread_num() == 0)
		{
			//std::cout << endl;
			//std::cout << "calcul de l'hydrogramme geomorphologique (nb_threads=" << omp_get_num_threads() << ")...     " << flush;
			oss.str("");
			oss << "(nb thread=" << omp_get_num_threads() << ")...   " << GetCurrentTimeStr();
			std::cout << oss.str() << flush;

			_listLog.push_back("Computing geomorphological hydrograph " + oss.str());

			if(_pSim_hyd->_bLogPerf)
			{
				oss.str("");
				oss << "Computing geomorphological hydrograph (nb thread=" << omp_get_num_threads() << ")";
				_pSim_hyd->_logPerformance.AddStep(oss.str());
			}
		}

		double c, v_pte, v_manning, volte, volts, volume_pas, v_ra, v_rb, v_rc, rd, dVolte, d, fLameCalculee;
		size_t index_zone, nbCarreau, i, j, ii, jj, li, ci, sizeM2, nbColM2, nocar, idx, idx2, ind_zone;
		int ntmax, nt, dt, pas, v_ori, k, k_zone, t, x;

		//#pragma omp for nowait		
		#pragma omp for
		for(int loop=0; loop<nb_zone; loop++)	//for(loop=omp_get_thread_num(); loop<nb_zone; loop+=omp_get_num_threads())
		{
			index_zone = static_cast<size_t>(loop);

			if(zones[index_zone].PrendreTypeZone() == ZONE::SOUS_BASSIN)
			{
				nbCarreau = eng_zones[index_zone].nb_car;
				vector<size_t> ind(nbCarreau);

				TriCarreaux3(zones._pRasterUhrhId, eng_zones[index_zone], zones[index_zone].PrendreIdent(), ind);

				li = eng_zones[index_zone].min_lig;
				ci = eng_zones[index_zone].min_col;

				//initialise les ruissellement amont et aval au pas precedent et le ruissellement amont au pas courant
				nbColM2 = eng_zones[index_zone].max_col - eng_zones[index_zone].min_col + 1;
				sizeM2 = nbColM2 * (eng_zones[index_zone].max_lig - eng_zones[index_zone].min_lig + 1);

				double* ra = new double[sizeM2]();	//zero-initialized -> ()
				double* rb = new double[sizeM2]();	//
				//double* rc = new double[sizeM2]();	//

				//determine le pas de temps de calcul
				ntmax = 1;
				nt = 0;
				c = 0.0;

				for(nocar=0; nocar!=nbCarreau; nocar++)
				{
					idx = ind[nocar];

					//si la maille n'appartient pas au reseau
					if(troncons._pRasterTronconId[idx] == 0)
					{
						//determine la vitesse maximale de l'onde
						v_pte = _sim_hyd._pRasterPente->_values[idx] * 0.001;	//pente: pour mille -> ratio
						v_manning = manning[idx];

						if(v_manning > 0.0)
						{
							//c = (5.0/3.0 * pow(v_pte, 0.5) / v_manning * pow(_lame, 2.0 / 3.0));
							c = DIV1 * pow(v_pte, 0.5) / v_manning * POW1;
						}
						else
							c = 0.0;

						if(c == 0.0)
							nt = 1;
						else
							nt = (int)( ((double)pdts) / (resolution / c + 1.0) + 1.0 );

						if(nt > ntmax) 
							ntmax = nt;
					}
				}

				nt = static_cast<int>(1.3 * (double)ntmax);
					
				//le pas de temps minimum doit etre de une seconde
				dt = max(pdts / nt, 1);
				
				// ruisselle la lame initiale
				volte = 0.0;
				volts = 0.0;
				volume_pas = 0.0;

				ind_zone = 0;

				for(pas=0; pas!=nt; pas++)
				{
					//ind_zone=ind_zone;
					//for(nocar=0; nocar!=nbCarreau; nocar++)
					//{
					//	i = static_cast<size_t>(ind[nocar] / nb_colonne);
					//	j = static_cast<size_t>(ind[nocar] % nb_colonne);
					//	idx = (i-li) * nbColM2 + (j-ci);

					//	rc[idx] = 0.0;
					//}

					double* rc = new double[sizeM2]();	//zero-initialized -> ()

					for(nocar=0; nocar!=nbCarreau; nocar++)
					{
						//volte+= _lame * resolution * resolution / (double)nt;
						volte+= MULT1 / (double)nt;

						idx = ind[nocar];
						v_pte = _sim_hyd._pRasterPente->_values[idx] * 0.001;	//pente: pour mille -> ratio
						v_manning = manning[idx];

						i = static_cast<size_t>(idx / nb_colonne);
						j = static_cast<size_t>(idx % nb_colonne);
						idx2 = (i-li) * nbColM2 + (j-ci);

						v_ra = ra[idx2];
						v_rb = rb[idx2];
						v_rc = rc[idx2];

						rd = 0.0;

						//si la maille n'appartient pas au reseau
						if(troncons._pRasterTronconId[idx] == 0)
							Ruisselement(resolution, v_pte, v_manning, v_ra, v_rb, v_rc, _lame/nt, dt, rd);
						else
						{
							//sinon tous les apports a la maille sont transferes a la maille suivante 
							//jusqu'a l'exutoire de maniere a ne pas simuler ici l'effet de 
							//l'ecoulement par le reseau

							//rd = v_rc + (_lame / (double)pdts * resolution);
							rd = v_rc + EQ1;
						}

						//conserve les donnees necessaire pour la suite des calculs
						ra[idx2] = rc[idx2];
						rb[idx2] = rd;

						v_ori = _sim_hyd._pRasterOri->_values[idx];
						k = zones._pRasterUhrhId->_values[idx];
							
						CarreauAval2(i, j, v_ori, ii, jj);
						if(ii >= nb_ligne || jj >= nb_colonne)
						{
							//si troncon exutoire, on vient de sortir de la matrice: fin du troncon. //sinon erreur
							if(troncons._pRasterTronconId[idx] == 1)
								k_zone = 0;
							else
							{
								//delete[] manning;
								delete[] ra;
								delete[] rb;
								delete[] rc;
								//std::cout << "Erreur CalculeHGM(): un pixel se deverse hors du bassin (err1)";
								//throw ERREUR("Erreur CalculeHGM(): un pixel se deverse hors du bassin (err1)");

								vErr[index_zone] = "CalculeHGM: erreur: un pixel se deverse hors du bassin (err1)";
								break;
							}
						}
						else
						{
							idx = ii * nb_colonne + jj;
							k_zone = zones._pRasterUhrhId->_values[idx];
						}

						if(k != k_zone)
						{
							ind_zone = zones.IdentVersIndex(k);	//conserve l'indicateur de la zone
							volume_pas = volume_pas + (double)dt * rd * resolution;	//conserve les volumes a l'exutoire de la zone.
						}
						else
						{
							//transfert le ruissellement au carreau aval.
							if(ii < nb_ligne && jj < nb_colonne)
							{
								idx = (ii-li) * nbColM2 + (jj-ci);
								rc[idx]+= rd;
							}
							else
							{
								//delete[] manning;
								delete[] ra;
								delete[] rb;
								delete[] rc;
								//std::cout << "Erreur CalculeHGM(): un pixel se deverse hors du bassin. (temp1)";
								//throw ERREUR("Erreur CalculeHGM(): un pixel se deverse hors du bassin. (temp1)");

								vErr[index_zone] = "CalculeHGM: erreur: un pixel se deverse hors du bassin. (temp1)";
								break;
							}
						}

						//indique que le ruissellement a ete calculer sur le carreau (i, j)
						//rc[idx2] = -1.0;
					}

					delete[] rc;
				}

				oc_zone[ind_zone].debits[0] = volume_pas / pdts;
				volts+= volume_pas;

				//vide le bassin
				t = 1;
				volts = volte - volts;
				dVolte = 0.05 * volte;

				while(volts > dVolte)
				{
					//le ruissellement de la lame unitaire est trop long
					if(t >= maxdeb)
						break;

					//détermine le pas de temps de calcul
					ntmax = 1;
					for(nocar=0; nocar!=nbCarreau; nocar++)
					{
						idx = ind[nocar];

						//pour les mailles qui n'appartiennent pas au reseau
						if(troncons._pRasterTronconId[idx] == 0)
						{
							//determine la lame
							v_manning = manning[idx];
							v_pte = _sim_hyd._pRasterPente->_values[idx] * 0.001;	//pente: pour mille -> ratio

							c = pow(v_manning / sqrt(v_pte), 0.6);
							d = 0.6;

							i = idx / nb_colonne;
							j = idx % nb_colonne;
							idx = (i-li) * nbColM2 + (j-ci);

							v_rb = rb[idx];
							fLameCalculee = c * pow(v_rb, d);

							//determine la vitesse maximale de l'onde
							//c =  (5.0 / 3.0 * pow(v_pte, 0.5) / v_manning * pow(fLameCalculee, 2.0 / 3.0));
							c =  DIV1 * pow(v_pte, 0.5) / v_manning * pow(fLameCalculee, EQ2);

							if(c == 0.0)
								nt = 1;
							else
								nt = (int)((double)pdts / (resolution / c + 1.0) + 1.0);

							if(nt > ntmax) 
								ntmax = nt;
						}
					}

					nt = static_cast<int>(1.3 * (double)ntmax);
						
					//le pas de temps minimum doit etre de 1 seconde
					dt = max(pdts/nt, 1);

					volume_pas = 0.0;

					//réinitialise à 0 les debits amonts des carreaux
					//delete [] rc;
					//double* rc = new double[sizeM2]();		//Zero-initialized -> ()

					//ruisselle sur les carreaux
					for(pas=0; pas!=nt; pas++)
					{
						//for(nocar=0; nocar!=nbCarreau; nocar++)		//
						//{
						//	i = static_cast<size_t>(ind[nocar] / nb_colonne);
						//	j = static_cast<size_t>(ind[nocar] % nb_colonne);
						//	idx = (i-li) * nbColM2 + (j-ci);

						//	rc[idx] = 0.0;
						//}

						double* rc = new double[sizeM2]();		//Zero-initialized -> ()

						for(nocar=0; nocar!=nbCarreau; nocar++)
						{
							idx = ind[nocar];

							i = idx / nb_colonne;
							j = idx % nb_colonne;
							idx2 = (i-li) * nbColM2 + (j-ci);

							//verifie si le ruissellement a deja ete calcule
							v_rc = rc[idx2];

							if(v_rc < 0.0)
							{
								//delete[] manning;
								delete[] ra;
								delete[] rb;
								delete[] rc;
								//std::cout << "Erreur CalculeHGM(): v_rc < 0";
								//throw ERREUR("Erreur CalculeHGM(): v_rc < 0");

								vErr[index_zone] = "CalculeHGM: erreur: v_rc < 0";
								break;
							}

							v_pte = _sim_hyd._pRasterPente->_values[idx] * 0.001;	//pente: pour mille -> ratio
							v_manning = manning[idx];

							v_ra = ra[idx2];
							v_rb = rb[idx2];
							v_rc = rc[idx2];

							rd = 0.0;

							//si la maille n'appartient pas au reseau
							if(troncons._pRasterTronconId[idx] == 0)
								Ruisselement(resolution, v_pte, v_manning, v_ra, v_rb, v_rc, 0.0, dt, rd);
							else
							{
								//sinon tous les apports a la maille sont transferes a la maille suivante 
								//jusqu'a l'exutoire de maniere a ne pas simuler ici l'effet de 
								//l'ecoulement par le reseau
								rd = v_rc;
							}

							//conserve les donnees necessaire pour la suite des calculs
							ra[idx2] = rc[idx2];
							rb[idx2] = rd;

							v_ori = _sim_hyd._pRasterOri->_values[idx];
							k = zones._pRasterUhrhId->_values[idx];

							CarreauAval2(i, j, v_ori, ii, jj);
							if(ii >= nb_ligne || jj >= nb_colonne)
							{
								//si troncon exutoire (troncon 1), on vient de sortir de la matrice: fin du troncon. //sinon erreur
								if(troncons._pRasterTronconId[idx] == 1)
									k_zone = 0;
								else
								{
									//delete [] manning;
									delete [] ra;
									delete [] rb;
									delete [] rc;
									//std::cout << "Erreur CalculeHGM(): un pixel se deverse hors du bassin (err2)";
									//throw ERREUR("Erreur CalculeHGM(): un pixel se deverse hors du bassin (err2)");

									vErr[index_zone] = "CalculeHGM: erreur: un pixel se deverse hors du bassin (err2)";
									break;
								}
							}
							else
							{
								idx = ii * nb_colonne + jj;
								k_zone = zones._pRasterUhrhId->_values[idx];
							}

							if(k != k_zone)
							{
								volume_pas+= dt * rd * resolution;	//conserve les debits a l'exutoire de la zone
							}
							else if(ii < nb_ligne && jj < nb_colonne)
							{
								//transfert le ruissellement au carreau aval
								idx = (ii-li) * nbColM2 + (jj-ci);
								rc[idx]+= rd;
							}
							else
							{
								//delete [] manning;
								delete [] ra;
								delete [] rb;
								delete [] rc;
								//std::cout << "Erreur CalculeHGM(): un pixel se deverse hors du bassin (temp2)";
								//throw ERREUR("Erreur CalculeHGM(): un pixel se deverse hors du bassin (temp2)");

								vErr[index_zone] = "CalculeHGM: erreur: un pixel se deverse hors du bassin (temp2)";
								break;
							}

							//indique que le ruissellement a ete calculer sur le carreau (i,j)
							//rc[idx2] = -1.0;
						}

						delete [] rc;
					}

					oc_zone[ind_zone].debits[t] = volume_pas / (double)pdts;
					volts-= volume_pas;

					++t;
					if(t > vTmax[index_zone])
						vTmax[index_zone] = t;

				} //fin while(volts > dVolte)

				delete [] ra;
				delete [] rb;
				//delete [] rc;

				//redistribue egalement sur chaque pas de temps le volume restant
				volume_pas = volts / (double)t;
				for(x=0; x!=t; x++)
					oc_zone[ind_zone].debits[x]+= (volume_pas / (double)pdts);
			}
		}

		} // omp parallel


		delete[] manning;

		tmax = 0;
		for(idxZone=0; idxZone!=zones.PrendreNbZone(); idxZone++)
		{
			if(vErr[idxZone] != "")
				throw ERREUR(vErr[idxZone]);

			if(vTmax[idxZone] > tmax)
				tmax = vTmax[idxZone];
		}
		_nb_debit = tmax;

		_oc_zone.swap(oc_zone);

		_oc_surf = _oc_zone;
		_oc_hypo = _oc_zone;
		_oc_base = _oc_zone;

		for(idxZone=0; idxZone!=zones.PrendreNbZone(); idxZone++)
		{
			for(z=0; z!=stMaxdeb; z++)
			{
				_oc_zone[idxZone].distri[z] = _oc_zone[idxZone].debits[z];
				_oc_zone[idxZone].debits[z] = 0.0;

				_oc_surf[idxZone].distri[z] = _oc_zone[idxZone].debits[z];
				_oc_surf[idxZone].debits[z] = 0.0;
				_oc_hypo[idxZone].distri[z] = _oc_zone[idxZone].debits[z];
				_oc_hypo[idxZone].debits[z] = 0.0;
				_oc_base[idxZone].distri[z] = _oc_zone[idxZone].debits[z];
				_oc_base[idxZone].debits[z] = 0.0;
			}

			_oc_zone[idxZone].distri.resize(_nb_debit);
			_oc_zone[idxZone].debits.resize(_nb_debit);

			_oc_surf[idxZone].distri.resize(_nb_debit);
			_oc_surf[idxZone].debits.resize(_nb_debit);
			_oc_hypo[idxZone].distri.resize(_nb_debit);
			_oc_hypo[idxZone].debits.resize(_nb_debit);
			_oc_base[idxZone].distri.resize(_nb_debit);
			_oc_base[idxZone].debits.resize(_nb_debit);
		}

		if(_pSim_hyd->_bLogPerf)
			_pSim_hyd->_logPerformance.AddStep("Completed");
	}


	void ONDE_CINEMATIQUE::Ruisselement(double arete, double pte, double man, double ra, double rb, double rc, double p, int dt, double& rd)
	{
		double v1, f, f1;

		//initialisation des differentes variables
		double c = pow(man / sqrt(pte), 0.6);
		double d = 0.6;

		//calcul du ruisselement sur le carreau
		double c1 = 2.0 * c * arete / dt;
		double c2 = pow(rb, d);
		double c3 = rc - rb + ra + 2.0 * p * arete / dt;

		double DIV1;

		DIV1 = 1.0 / d;

		rd = (ra - rb + p * arete / dt) * 2.0 / c1 + c2;
		if(rd < 0.0) 
			rd = rc + p * arete / dt;

		//rd = pow(rd, 1.0 / d);
		rd = pow(rd, DIV1);

		if(rd == 0.0) 
			rd = 1.0E-20;

		//iteration pour le calcul du ruisselement
		int iter = 0;
		int not_fini = 1;

		while(iter < MAXITER && not_fini)
		{
			iter++;

			v1 = pow(rd, d);
			f = rd + c1 *(v1 - c2) - c3;
			f1 = 1.0 + c1 * d * v1 / rd;
			rd = rd - f / f1;

			if(rd <= 0.0) 
				rd = -rd + 1.0E-20;

			if(fabs(f / f1) < (EPSILON / arete)) 
				not_fini = 0;
		}
	}


	//void ONDE_CINEMATIQUE::TriCarreaux(const RASTER<int>& grille, const RASTER<int>& orientations, rectangle r, int ident, vector<size_t>& ind_lig, vector<size_t>& ind_col)
	//{
	//	int nb_ligne = r.max_lig - r.min_lig + 1;
	//	int nb_colonne = r.max_col - r.min_col + 1;

	//	int nb_carreau = static_cast<int>(r.nb_car);

	//	int li = r.min_lig;
	//	int ci = r.min_col;

	//	// allocation temporaire pour le calcul
	//	MATRICE<int> mat(nb_ligne, nb_colonne);

	//	// trouve l'ordre devaluation
	//	int fini= 0;
	//	int l = 0;
	//	for (int k = 0; !fini && k < nb_carreau; k++)
	//	{
	//		fini = 1;
	//		for (int i = 0; i < nb_ligne; i++)
	//		{
	//			for (int j = 0; j < nb_colonne; j++)
	//			{
	//				int zone = grille(li + i, ci + j);
	//				int v_ori = orientations(li + i, ci + j);
	//				int v_mat = mat(i, j);

	//				if ((zone == ident) && (v_mat >= k))
	//				{
	//					fini = 0;

	//					int ii = 0, jj = 0;

	//					CarreauAval(li + i, ci + j, v_ori, ii, jj);

	//					ii -= li;
	//					jj -= ci;

	//					if (ii >= 0 && ii < nb_ligne && jj >= 0 && jj < nb_colonne)
	//					{
	//						mat(ii, jj) = k + 1;
	//						l++;
	//					}
	//					else
	//					{
	//					}
	//				}
	//			}
	//		}
	//	}

	//	// On devrait faire un test ici pour valider la matrice d'ordre "mat".
	//	// Si un de ces elements est egale au nombre de carreaux "nb_carreau"
	//	// alors il y a un cycle, d'ou un message d'erreur a l'usager indiquant
	//	// qu'il y a un cycle dans sa matrice d'orientations.
	//	// Aussi, l'exutoire de la zone doit avoir le nombre le plus eleve dans "mat".

	//	// transfert dans les vecteurs
	//	l = 0;
	//	fini = 0;
	//	for (int k = 0; !fini && k < nb_carreau; k++)
	//	{
	//		fini = 1;
	//		for (int i = 0; i < nb_ligne; i++)
	//		{
	//			for (int j = 0; j < nb_colonne; j++)
	//			{
	//				int zone = grille(li + i, ci + j);
	//				int v_mat = mat(i, j);

	//				if ((zone == ident) && (v_mat == k))
	//				{
	//					fini = 0;
	//					ind_lig[l] = li + i;
	//					ind_col[l] = ci + j;
	//					l++;
	//				}
	//			}
	//		}
	//	}
	//}


	void ONDE_CINEMATIQUE::TriCarreaux3(RasterInt2* uhrh, rectangle r, int ident, vector<size_t>& ind)
	{
		size_t idx;
		int k, zone, v_ori, v_mat, l, i, j;
		int a, b, iNbCarreau;

		size_t nbPixel = (r.max_lig - r.min_lig + 1) * (r.max_col - r.min_col + 1);
		int nb_ligne = static_cast<int>(r.max_lig - r.min_lig + 1);
		int nb_colonne = static_cast<int>(r.max_col - r.min_col + 1);
		int li = static_cast<int>(r.min_lig);
		int ci = static_cast<int>(r.min_col);

		//allocation temporaire pour le calcul
		int* mat = new int[nbPixel]();	//Zero-initialized -> ()

		//trouve l'ordre d'évaluation
		int fini = 0;

		iNbCarreau = static_cast<int>(r.nb_car);

		for(k=0; !fini && k!=iNbCarreau; k++)
		{
			fini = 1;
			for(i=0; i!=nb_ligne; i++)
			{
				for(j=0; j!=nb_colonne; j++)
				{
					idx = (li+i) * uhrh->_xSize + ci + j;

					zone = uhrh->_values[idx];
					v_ori = _sim_hyd._pRasterOri->_values[idx];

					v_mat = mat[i*nb_colonne+j];

					if(zone == ident && v_mat >= k)
					{
						fini = 0;

						a = 0;

						CarreauAval(li+i, ci+j, v_ori, a, b);

						a-= li;
						b-= ci;

						if(a >= 0 && a < nb_ligne && b >= 0 && b < nb_colonne)
						{
							idx = a * nb_colonne + b;
							mat[idx] = k + 1;
						}
					}
				}
			}
		}

		//On devrait faire un test ici pour valider la matrice d'ordre "mat".
		//Si un de ces elements est egale au nombre de carreaux "iNbCarreau"
		//alors il y a un cycle, d'ou un message d'erreur a l'usager indiquant
		//qu'il y a un cycle dans sa matrice d'orientations.
		//Aussi, l'exutoire de la zone doit avoir le nombre le plus eleve dans "mat".

		//transfert dans les vecteurs
		l = 0;
		fini = 0;
		for(k=0; !fini && k!=iNbCarreau; k++)
		{
			fini = 1;
			for(i=0; i!=nb_ligne; i++)
			{
				for(j=0; j!=nb_colonne; j++)
				{
					idx = (li+i) * uhrh->_xSize + ci + j;

					zone = uhrh->_values[idx];

					v_mat = mat[i * nb_colonne + j];

					if(zone == ident && v_mat == k)
					{
						fini = 0;
						ind[l] = idx;

						++l;
					}
				}
			}
		}
	}


	void ONDE_CINEMATIQUE::ChangeNomFichierHGM(const std::string& nom_fichier)
	{
		_nom_fichier_hgm = nom_fichier;
	}


	void ONDE_CINEMATIQUE::ChangeLame(double lame)
	{
		BOOST_ASSERT(lame > 0);
		_lame = lame;
	}


	void ONDE_CINEMATIQUE::ChangeManningForet(size_t index_zone, double manning)
	{
		BOOST_ASSERT(index_zone < _manning_forets.size());
		_manning_forets[index_zone] = manning;
	}


	void ONDE_CINEMATIQUE::ChangeManningEau(size_t index_zone, double manning)
	{
		BOOST_ASSERT(index_zone < _manning_eaux.size());
		_manning_eaux[index_zone] = manning;
	}


	void ONDE_CINEMATIQUE::ChangeManningAutre(size_t index_zone, double manning)
	{
		BOOST_ASSERT(index_zone < _manning_autres.size());
		_manning_autres[index_zone] = manning;
	}


	void ONDE_CINEMATIQUE::ChangeIndexForets(std::vector<size_t> index)
	{
		_index_forets.swap(index);
		_index_forets.shrink_to_fit();
	}


	void ONDE_CINEMATIQUE::ChangeIndexEaux(std::vector<size_t> index)
	{
		_index_eaux.swap(index);
		_index_eaux.shrink_to_fit();
	}


	void ONDE_CINEMATIQUE::CalculePourcentageOccupation()
	{
		auto& occupation_sol = _sim_hyd.PrendreOccupationSol();
		const size_t nb_zone = _sim_hyd.PrendreZones().PrendreNbZone();

		vector<size_t> index_autres;
		for (size_t index = 0; index < occupation_sol.PrendreNbClasse(); ++index)
		{
			if (find(begin(_index_eaux), end(_index_eaux), index) == end(_index_eaux) &&
				find(begin(_index_forets), end(_index_forets), index) == end(_index_forets))
				index_autres.push_back(index);
		}
		index_autres.shrink_to_fit();
		_index_autres.swap(index_autres);

		// calcul pourcentage des classes integrees

		_pourcentage_eaux.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			double pourcentage = 0;

			for (auto index = begin(_index_eaux); index != end(_index_eaux); ++index)
				pourcentage += occupation_sol.PrendrePourcentage_double(index_zone, *index);

			_pourcentage_eaux[index_zone] = pourcentage;
		}

		_pourcentage_forets.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			double pourcentage = 0;

			for (auto index = begin(_index_forets); index != end(_index_forets); ++index)
				pourcentage += occupation_sol.PrendrePourcentage_double(index_zone, *index);

			_pourcentage_forets[index_zone] = pourcentage;
		}

		_pourcentage_autres.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
			_pourcentage_autres[index_zone] = max(1.0 - (_pourcentage_forets[index_zone] + _pourcentage_eaux[index_zone]), 0.0);
	}


	void ONDE_CINEMATIQUE::CalculeHgm(double lame, string nom_fichier)
	{
		double lame_tmp = _lame;
		_lame = lame;

		string nom_fichier_tmp = _nom_fichier_hgm;
		_nom_fichier_hgm = nom_fichier;

		//test output for error before computing hgm
		string repertoire = PrendreRepertoire(_nom_fichier_hgm);
		if (!RepertoireExiste(repertoire))
			CreeRepertoire(repertoire);

		ofstream fichier(_nom_fichier_hgm);
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(_nom_fichier_hgm);

		fichier << "test" << endl;
		fichier.close();

		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		
		boost::filesystem::remove(_nom_fichier_hgm);

		//
		CalculePourcentageOccupation();

		CalculeHgm();

		SauvegardeHgm();

		_lame = lame_tmp;
		_nom_fichier_hgm = nom_fichier_tmp;
	}


	void ONDE_CINEMATIQUE::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ONDE_CINEMATIQUE");

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ONDE_CINEMATIQUE", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		// lecture des classes integrees

		lire_cle_valeur(fichier, cle, valeur);
		ChangeIndexForets( extrait_valeur(valeur) );

		lire_cle_valeur(fichier, cle, valeur);
		ChangeIndexEaux( extrait_valeur(valeur) );

		getline_mod(fichier, ligne);

		// lecture de la lame

		lire_cle_valeur(fichier, cle, valeur);
		istringstream iss(valeur);
		double lame;
		iss >> lame;

		ChangeLame(lame);

		getline_mod(fichier, ligne);

		// lecture nom de fichier hgm
		string nom_fichier_hgm;
		lire_cle_valeur(fichier, cle, nom_fichier_hgm);

		nom_fichier_hgm = TrimString(nom_fichier_hgm);
		if(nom_fichier_hgm == "")
			nom_fichier_hgm = "hgm/hydrogramme.hgm";
		else
		{
			if(Racine(nom_fichier_hgm))
				ChangeNomFichierHGM(nom_fichier_hgm);	//absolute path
			else
				ChangeNomFichierHGM(Combine(_sim_hyd.PrendreRepertoireProjet(), nom_fichier_hgm));	//relative path
		}

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne); // commentaire

		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			int ident;
			char c;
			double manning_foret, manning_eau, manning_autre;

			fichier >> ident >> c;
			fichier >> manning_foret >> c;
			fichier >> manning_eau >> c;
			fichier >> manning_autre;

			size_t index_zone = zones.IdentVersIndex(ident);

			ChangeManningForet(index_zone, manning_foret);
			ChangeManningEau(index_zone, manning_eau);
			ChangeManningAutre(index_zone, manning_autre);
		}
	}


	void ONDE_CINEMATIQUE::LectureParametresFichierGlobal()
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
		double dVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "ONDE CINEMATIQUE")
			{
				++no_ligne;
				getline_mod(fichier, ligne);				
				ChangeIndexForets(extrait_valeur(ligne));

				++no_ligne;
				getline_mod(fichier, ligne);
				ChangeIndexEaux(extrait_valeur(ligne));

				++no_ligne;
				getline_mod(fichier, ligne);
				istringstream iss(ligne);
				double lame;
				iss >> lame;
				ChangeLame(lame);

				++no_ligne;
				getline_mod(fichier, ligne);
				string repertoire = _sim_hyd.PrendreRepertoireProjet();
				if(ligne == "")
					ligne = "hgm/hydrogramme.hgm";
				ChangeNomFichierHGM( Combine(repertoire, ligne) );

				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					vector<double> vValeur = extrait_dvaleur(ligne, ";");

					if(vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide. ONDE CINEMATIQUE");

					dVal = static_cast<double>(x);
					if(dVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. ONDE CINEMATIQUE. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						ChangeManningForet(index_zone, vValeur[1]);
						ChangeManningEau(index_zone, vValeur[2]);
						ChangeManningAutre(index_zone, vValeur[3]);
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; ONDE CINEMATIQUE; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele ONDE CINEMATIQUE");
	}


	void ONDE_CINEMATIQUE::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "CLASSE INTEGRE FORETS;";
		auto index_foret = PrendreIndexForets();
		for (auto iter = begin(index_foret); iter != end(index_foret); ++iter)
			fichier << *iter + 1<< ';';
		fichier << endl;

		fichier << "CLASSE INTEGRE EAUX;";
		auto index_eaux = PrendreIndexEaux();
		for (auto iter = begin(index_eaux); iter != end(index_eaux); ++iter)
			fichier << *iter + 1<< ';';
		fichier << endl << endl;

		fichier << "LAME;" << PrendreLame() << endl << endl;

		fichier << "NOM FICHIER HGM;" << PrendreRepertoireRelatif(_sim_hyd.PrendreRepertoireProjet(), PrendreNomFichierHgm()) << endl << endl;

		fichier << "UHRH ID;MANNING FORETS;MANNING EAUX;MANNING AUTRES" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';

			fichier << PrendreManningForet(index) << ';';
			fichier << PrendreManningEaux(index) << ';';
			fichier << PrendreManningAutres(index) << endl;
		}
	}

}
