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

#include "onde_cinematique_modifiee.hpp"

#include "barrage_historique.hpp"
#include "constantes.hpp"
#include "erreur.hpp"
#include "lac.hpp"
#include "lac_sans_laminage.hpp"
#include "riviere.hpp"
#include "util.hpp"
#include "version.hpp"

#include <algorithm>
#include <deque>
//#include <future>


using namespace std;


namespace HYDROTEL
{

	ONDE_CINEMATIQUE_MODIFIEE::ONDE_CINEMATIQUE_MODIFIEE(SIM_HYD& sim_hyd)
		: ACHEMINEMENT_RIVIERE(sim_hyd, "ONDE CINEMATIQUE MODIFIEE")
	{
		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;
	}

	ONDE_CINEMATIQUE_MODIFIEE::~ONDE_CINEMATIQUE_MODIFIEE()
	{
	}

	float ONDE_CINEMATIQUE_MODIFIEE::PrendreOptimisationRugosite(size_t index_troncon) const
	{
		BOOST_ASSERT(index_troncon < _coefficient_optimisation_rugosite.size());
		return _coefficient_optimisation_rugosite[index_troncon];
	}

	float ONDE_CINEMATIQUE_MODIFIEE::PrendreOptimisationLargeurRiviere(size_t index_troncon) const
	{
		BOOST_ASSERT(index_troncon < _coefficient_largeurs_rivieres.size());
		return _coefficient_largeurs_rivieres[index_troncon];
	}

	void ONDE_CINEMATIQUE_MODIFIEE::ChangeOptimisationRugosite(size_t index_troncon, float coef)
	{
		BOOST_ASSERT(index_troncon < _coefficient_optimisation_rugosite.size());
		_coefficient_optimisation_rugosite[index_troncon] = coef;
	}

	void ONDE_CINEMATIQUE_MODIFIEE::ChangeOptimisationLargeurRiviere(size_t index_troncon, float coef)
	{
		BOOST_ASSERT(index_troncon < _coefficient_largeurs_rivieres.size());
		_coefficient_largeurs_rivieres[index_troncon] = coef;
	}

	void ONDE_CINEMATIQUE_MODIFIEE::ChangeNomFichierLectureEtat(std::string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}

	void ONDE_CINEMATIQUE_MODIFIEE::ChangeRepertoireEcritureEtat(std::string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}

	void ONDE_CINEMATIQUE_MODIFIEE::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}

	void ONDE_CINEMATIQUE_MODIFIEE::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}

	string ONDE_CINEMATIQUE_MODIFIEE::PrendreNomFichierLectureEtat() const
	{
		return _nom_fichier_lecture_etat;
	}

	string ONDE_CINEMATIQUE_MODIFIEE::PrendreRepertoireEcritureEtat() const
	{
		return _repertoire_ecriture_etat;
	}

	bool ONDE_CINEMATIQUE_MODIFIEE::PrendreSauvegardeTousEtat() const
	{
		return _sauvegarde_tous_etat;
	}

	DATE_HEURE ONDE_CINEMATIQUE_MODIFIEE::PrendreDateHeureSauvegardeEtat() const
	{
		return _date_sauvegarde_etat;
	}


	void ONDE_CINEMATIQUE_MODIFIEE::ChangeNbParams(const ZONES& /*zones*/)
	{
		const size_t nb_troncon = _sim_hyd.PrendreTroncons().PrendreNbTroncon();

		_coefficient_optimisation_rugosite.resize(nb_troncon, 1.0f);
		_coefficient_largeurs_rivieres.resize(nb_troncon, 1.0f);

		_hauteurMethodeCalcul = 1;		//1: section rectangulaire		//2: section trapezoidal (Tiwari et al. (2012))		//3: approche HAND (fichier débits/hauteurs)

		_hauteurTrapezeSideSlope.resize(nb_troncon, 1.0);
	}

	
	void ONDE_CINEMATIQUE_MODIFIEE::LectureEtat(DATE_HEURE date_courante)
	{
		//acheminement_riviere
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; fichier etat ONDE_CINEMATIQUE_MODIFIEE; " + _nom_fichier_lecture_etat);

		vector<int> vValidation;
		string ligne, str;
		size_t index_troncon;
		int iIdent;

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		while(!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne != "")
			{
				vector<float> valeurs = extrait_fvaleur(ligne, _pOutput->_sFichiersEtatsSeparator);
				vector<double> dValeurs = extrait_dvaleur(ligne, _pOutput->_sFichiersEtatsSeparator);

				iIdent = static_cast<int>(valeurs[0]);
				index_troncon = _sim_hyd.PrendreTroncons().IdentVersIndex(iIdent);

				if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index_troncon) != end(_sim_hyd.PrendreTronconsSimules()))
				{
					TRONCON* troncon = troncons[index_troncon];
					troncon->ChangeDebitAmont(valeurs[1]);
					troncon->ChangeDebitAval(valeurs[2]);
					troncon->ChangeApportLateral(valeurs[3]);

					_ocm[index_troncon].qamont = valeurs[1];
					_ocm[index_troncon].qaval = valeurs[2];
					_ocm[index_troncon].qapportlat = valeurs[3];

					if(valeurs.size() >= 5)
						_hauteur[index_troncon] = dValeurs[4];

					vValidation.push_back(iIdent);
				}
			}
		}

		fichier.close();

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreTronconsSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; fichier etat ONDE_CINEMATIQUE_MODIFIEE; " + _nom_fichier_lecture_etat);

		
		//acheminement_riviere MH
		if(_milieu_humide_riverain.size() != 0)
		{
			if(_nom_fichier_lecture_etat.length() >= 15)
			{
				str = _nom_fichier_lecture_etat;
				str = str.insert(str.length()-14, "MH_");
				if(boost::filesystem::exists(str))
				{
					//pour validation
					vValidation.clear();

					vector<int> tronconsMH_simules_ident;
					size_t nbSimuler, x;

					nbSimuler = _sim_hyd.PrendreTronconsSimules().size();
					x = 0;

					for(index_troncon=0; x<nbSimuler; index_troncon++)
					{
						if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index_troncon) != end(_sim_hyd.PrendreTronconsSimules()))
						{
							if(_milieu_humide_riverain[index_troncon])
								tronconsMH_simules_ident.push_back(troncons[index_troncon]->PrendreIdent());

							++x;
						}
					}
					//

					ifstream fichier2(str);
					if (!fichier2)
						throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; fichier etat ONDE_CINEMATIQUE_MODIFIEE; " + str);

					getline_mod(fichier2, ligne);
					getline_mod(fichier2, ligne);
					getline_mod(fichier2, ligne);
					getline_mod(fichier2, ligne);

					while(!fichier2.eof())
					{
						getline_mod(fichier2, ligne);
						if(ligne != "")
						{
							auto valeurs = extrait_fvaleur(ligne, _pOutput->_sFichiersEtatsSeparator);
							iIdent = static_cast<int>(valeurs[0]);
							index_troncon = _sim_hyd.PrendreTroncons().IdentVersIndex(iIdent);

							if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index_troncon) != end(_sim_hyd.PrendreTronconsSimules()))
							{
								_milieu_humide_riverain[index_troncon]->set_wet_v(valeurs[1]);
								_milieu_humide_riverain[index_troncon]->set_wet_a(valeurs[2]);
								_milieu_humide_riverain[index_troncon]->set_wet_d(valeurs[3]);

								_ocm_mh[iIdent][0].qamont = valeurs[4];
								_ocm_mh[iIdent][0].qapportlat = valeurs[5];
								_ocm_mh[iIdent][0].qaval = valeurs[6];

								_ocm_mh[iIdent][1].qamont = valeurs[7];
								_ocm_mh[iIdent][1].qapportlat = valeurs[8];
								_ocm_mh[iIdent][1].qaval = valeurs[9];

								_ocm_mh[iIdent][2].qamont = valeurs[10];
								_ocm_mh[iIdent][2].qapportlat = valeurs[11];
								_ocm_mh[iIdent][2].qaval = valeurs[12];

								vValidation.push_back(iIdent);
							}
						}
					}

					fichier2.close();

					std::sort(vValidation.begin(), vValidation.end());
					if(!equal(vValidation.begin(), vValidation.end(), tronconsMH_simules_ident.begin()))
						throw ERREUR_LECTURE_FICHIER("ACHEMINEMENT_RIVIERE; fichier etat ONDE_CINEMATIQUE_MODIFIEE; " + str);
				}
			}
		}
	}


	void ONDE_CINEMATIQUE_MODIFIEE::SauvegardeEtat(DATE_HEURE date_courante)
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier, oss;
		size_t x, nbSimuler, index_troncon;
		int iIdent;

		string sSep = _pOutput->_sFichiersEtatsSeparator;

		if(!RepertoireExiste(_repertoire_ecriture_etat))
			CreeRepertoire(_repertoire_ecriture_etat);

		//acheminement_riviere
		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			        << "acheminement_riviere_" 
			        << setw(4) << date_courante.PrendreAnnee() 
			        << setw(2) << date_courante.PrendreMois() 
			        << setw(2) << date_courante.PrendreJour() 
			        << setw(2) << date_courante.PrendreHeure() 
					<< ".csv";

		ofstream fichier(nom_fichier.str());
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier.exceptions(ios::failbit | ios::badbit);

		fichier << "ETATS ACHEMINEMENT RIVIERE" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier << "DATE_HEURE" << sSep << date_courante << endl;
		fichier << endl;

		fichier << "TRONCON" << sSep << "DEBIT AMONT" << sSep << "DEBIT AVAL" << sSep << "APPORTLAT" << sSep << "HAUTEUR" << endl;

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		nbSimuler = _sim_hyd.PrendreTronconsSimules().size();
		x = 0;

		for (index_troncon=0; x<nbSimuler; index_troncon++)
		{
			if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index_troncon) != end(_sim_hyd.PrendreTronconsSimules()))
			{
				TRONCON* troncon = troncons[index_troncon];

				oss.str("");
				oss << troncon->PrendreIdent() << sSep;

				oss << setprecision(12) << setiosflags(ios::fixed);

				oss << troncon->PrendreDebitAmont() << sSep;
				oss << troncon->PrendreDebitAval() << sSep;
				oss << troncon->PrendreApportLateral() << sSep;

				oss << setprecision(21) << setiosflags(ios::fixed);

				oss << _hauteur[index_troncon];

				fichier << oss.str() << endl;
				++x;
			}
		}

		fichier.close();


		//acheminement_riviere MH
		if(_milieu_humide_riverain.size() != 0)
		{
			nom_fichier.str("");
			nom_fichier << _repertoire_ecriture_etat;
			if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
				nom_fichier << "/";

			nom_fichier << setfill('0') 
				<< "acheminement_riviere_MH_" 
				<< setw(4) << date_courante.PrendreAnnee() 
				<< setw(2) << date_courante.PrendreMois() 
				<< setw(2) << date_courante.PrendreJour() 
				<< setw(2) << date_courante.PrendreHeure() 
				<< ".csv";

			ofstream fichier2(nom_fichier.str());
			if (!fichier2)
				throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

			fichier2.exceptions(ios::failbit | ios::badbit);

			fichier2 << "ETATS ACHEMINEMENT RIVIERE MILIEUX HUMIDES RIVERAIN" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
			fichier2 << "DATE_HEURE" << sSep << date_courante << endl;
			fichier2 << endl;

			fichier2 << "TRONCON" << sSep << "WET V" << sSep << "WET A" << sSep << "WET D" << sSep << "OCM QAMONT 1" << sSep << "OCM QAPPORTLAT 1" << sSep << "OCM QAVAL 1" << sSep << "OCM QAMONT 2" << sSep << "OCM QAPPORTLAT 2" << sSep << "OCM QAVAL 2" << sSep << "OCM QAMONT 3" << sSep << "OCM QAPPORTLAT 3" << sSep << "OCM QAVAL 3" << endl;

			nbSimuler = _sim_hyd.PrendreTronconsSimules().size();
			x = 0;

			for (index_troncon=0; x<nbSimuler; index_troncon++)
			{
				if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index_troncon) != end(_sim_hyd.PrendreTronconsSimules()))
				{
					if(_milieu_humide_riverain[index_troncon])
					{
						iIdent = troncons[index_troncon]->PrendreIdent();

						oss.str("");
						oss << iIdent << sSep;

						oss << setprecision(12) << setiosflags(ios::fixed);
					
						oss << _milieu_humide_riverain[index_troncon]->get_wet_v() << sSep;
						oss << _milieu_humide_riverain[index_troncon]->get_wet_a() << sSep;
						oss << _milieu_humide_riverain[index_troncon]->get_wet_d() << sSep;

						oss << _ocm_mh[iIdent][0].qamont << sSep;
						oss << _ocm_mh[iIdent][0].qapportlat << sSep;
						oss << _ocm_mh[iIdent][0].qaval << sSep;

						oss << _ocm_mh[iIdent][1].qamont << sSep;
						oss << _ocm_mh[iIdent][1].qapportlat << sSep;
						oss << _ocm_mh[iIdent][1].qaval << sSep;

						oss << _ocm_mh[iIdent][2].qamont << sSep;
						oss << _ocm_mh[iIdent][2].qapportlat << sSep;
						oss << _ocm_mh[iIdent][2].qaval;

						fichier2 << oss.str() << endl;
					}
					++x;
				}
			}
		
			fichier2.close();
		}
	}


	void ONDE_CINEMATIQUE_MODIFIEE::Initialise()
	{
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		TRONCON* troncon;
		size_t i, nbTroncon, index_troncon;
		string path, str1, str2;
		float wet_v, wet_a, wet_d;
		int ident;

		nbTroncon = troncons.PrendreNbTroncon();

		//initialise le vecteur _ocm
		OCM ocmVal;
		ocmVal.qamont = 0.0f;
		ocmVal.qapportlat = 0.0f;
		ocmVal.qaval = 0.0f;

		_ocm.clear();
		for(i=0; i!=nbTroncon; i++)
			_ocm.push_back(ocmVal);

		//determination de l'ordre de calcul des troncons
		
		TrieTroncons();

		//vector<size_t> index_troncons = _sim_hyd.PrendreTronconsSimules();
		//size_t index;

		//mapShreveTroncon.clear();
		//iShreveMax = 1;

		//for(index=0; index<index_troncons.size(); index++)
		//{
		//	mapShreveTroncon[troncons[index_troncons[index]]->_iSchreve].push_back(index_troncons[index]);
		//	if( troncons[index_troncons[index]]->_iSchreve > iShreveMax)
		//		iShreveMax = troncons[index_troncons[index]]->_iSchreve;
		//}

		//conserve la hauteur d'eau (m)
		_hauteur.resize(nbTroncon, 0.0);

		//initialisation milieux humides riverains
		_ocm_mh.clear();

		if(_milieu_humide_riverain.size() != 0)
		{
			for(index_troncon=0; index_troncon!=nbTroncon; index_troncon++)
			{
				if(_milieu_humide_riverain[index_troncon])
				{
					troncon = troncons[index_troncon];
					ident = troncon->PrendreIdent();

					//initialise le vecteur _ocm_mh
					_ocm_mh[ident].push_back(ocmVal);	//indice 0: amont
					_ocm_mh[ident].push_back(ocmVal);	//indice 1: 
					_ocm_mh[ident].push_back(ocmVal);	//indice 2: aval

					wet_v =  _milieu_humide_riverain[index_troncon]->get_wetnvol() * _milieu_humide_riverain[index_troncon]->m_eauIni;	//auparavant m_eauIni etait par defaut 0.5
					wet_a = _milieu_humide_riverain[index_troncon]->get_belta() * pow(wet_v, _milieu_humide_riverain[index_troncon]->get_alfa());
					wet_d = wet_v / (wet_a + 0.001f);

					_milieu_humide_riverain[index_troncon]->set_wet_v(wet_v);
					_milieu_humide_riverain[index_troncon]->set_wet_a(wet_a);
					_milieu_humide_riverain[index_troncon]->set_wet_d(wet_d);
				}
			}

			//fichier output
			path = Combine(_sim_hyd.PrendreRepertoireResultat(), "wetland_riverain.csv");
			m_wetfichier.open(path.c_str());

			m_wetfichier << "IdTroncon" << _pOutput->Separator()
							<< "Annee" << _pOutput->Separator()
							<< "Mois" << _pOutput->Separator()
							<< "Jour" << _pOutput->Separator()
							<< "Heure" << _pOutput->Separator()
							<< "wet_v (m3)" << _pOutput->Separator()
							<< "wet_a (m2)" << _pOutput->Separator()
							<< "wet_d (m)" << _pOutput->Separator()
							<< "sur_q (m3/s)" << _pOutput->Separator()
							<< "Hauteur d'eau (m)" << _pOutput->Separator()
							<< "qd (m3/s)" << endl;
		}

		if (!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		if(!_sim_hyd._pr->LecturePrelevements())
			throw ERREUR("Erreur lecture des prelevements");

		if(_sim_hyd._pr->_bSimulePrelevements)
		{
			path = Combine(_sim_hyd.PrendreRepertoireResultat(), "prelevements_calcule.csv");
			_sim_hyd._pr->_ofsPrelevementCalcule.open(path);
			_sim_hyd._pr->_ofsPrelevementCalcule << "DATE" << _pOutput->Separator() << "TRONCON" << _pOutput->Separator() << "TYPE" << _pOutput->Separator() << "ID" << _pOutput->Separator() << "ID_PR" << _pOutput->Separator() << "PRELEVEMENT/REJET (m3s)" << endl;

			//path = Combine(_sim_hyd.PrendreRepertoireResultat(), "prelevements_effectue.csv");
			//_sim_hyd._pr->_ofsPrelevementEffectue.open(path);
			//_sim_hyd._pr->_ofsPrelevementEffectue << "DATE" << _pOutput->Separator() << "TRONCON" << _pOutput->Separator() << "TYPE" << _pOutput->Separator() << "PRELEVEMENT/REJET (m3s)" << endl;
		}

		if(_pOutput->_debit_aval_moy7j_min)
		{
			_q7avgNbPdt = 24 / _sim_hyd.PrendrePasDeTemps() * 7;
			
			_q7avgYearCurrentY = -1;			//yearly minimum
			_q7avgYearEndY = -1;				//
			_q7avgMinY.resize(nbTroncon);		//

			_q7avgYearCurrentS = -1;			//summery minimum (month 6 to 11)
			_q7avgYearEndS = -1;				//
			_q7avgMinS.resize(nbTroncon);		//
		}

		if(_hauteurMethodeCalcul == 3)		//3: approche HAND (fichier débits/hauteurs)
			LectureFichierDebitsHauteurs();

		//périmètre mouillé
		str1 = Combine(_sim_hyd.PrendreRepertoireProjet(), "physio/P_H-ponderees.csv");
		str2 = Combine(_sim_hyd.PrendreRepertoireProjet(), "physio/P_H.csv");
		if(boost::filesystem::exists(str1) || boost::filesystem::exists(str2))
		{
			LectureFichierPerimetreMouilleHauteurs();

			str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "perimetre_mouille.csv");

			_fichier_pm.open(str1.c_str());

			_fichier_pm << "Périmètre mouillé (m)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl;
			_fichier_pm << "date heure\\troncon" << _pOutput->Separator();

			ostringstream oss;
			oss.str("");

			for(size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
			{
				if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
				{
					if(_pOutput->_bSauvegardeTous ||
						find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
					{
						oss << troncons[index]->PrendreIdent() << _pOutput->Separator();
					}
				}
			}

			str1 = oss.str();
			str1 = str1.substr(0, str1.length()-1); //enleve le dernier separateur
			_fichier_pm << str1 << endl;
		}

		ACHEMINEMENT_RIVIERE::Initialise();
	}


	void ONDE_CINEMATIQUE_MODIFIEE::Calcule()
	{
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();
		int pdts = pas_de_temps * 60 * 60;

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		TRONCON::TYPE_TRONCON type_troncon;
		RIVIERE* riviere;
		TRONCON* troncon;

		ostringstream oss;
		string str;
		size_t index, i, nbTronconSim;
		float lrg, lng, pte, manning, c, fVal;
		int t;

		int ntmax = 0;
		int nt = 0;
		int dt = 0;

		// determine le pas de temps de calcul
		
		vector<size_t> index_troncons = _sim_hyd.PrendreTronconsSimules();
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		nbTronconSim = index_troncons.size();

		for(index=0; index!=nbTronconSim; index++)
		{
			i = index_troncons[index];			
			troncon = troncons[i];
			type_troncon = troncon->PrendreType();

			if(type_troncon == TRONCON::RIVIERE)
			{
				riviere = static_cast<RIVIERE*>(troncon);

				lrg = riviere->PrendreLargeur();
				lng = riviere->PrendreLongueur();
				pte = riviere->PrendrePente();
				manning = riviere->PrendreManning();

				// determine la vitesse maximale de l'onde
				c = Celerite(lng, lrg, pte, manning, _ocm[i].qamont, _ocm[i].qaval);
				if (c < EPSILON)
					c = EPSILON;

				nt = pdts / static_cast<int>(lng / c + 1.0f) + 1;
				if (nt > ntmax)
					ntmax = nt;
			}
		}

		if(static_cast<int>(pdts / ntmax) < 1800)
		{
			dt = 1800;
			nt = static_cast<int>(pdts / dt);
		}
		else
		{
			nt = ntmax;
			dt = static_cast<int>(pdts / nt);
		}

		//prelevements
		if(_sim_hyd._pr->_bSimulePrelevements)
		{
			if(!_sim_hyd._pr->CalculePrelevements())	//calcule les prelevements total et les rejets total des troncons pour le pas de temps courant
				throw ERREUR("erreur CalculePrelevements");
		}
		//

		//vector<future<void>> vFutures;		
		//int iIndex;

		for(t=0; t!=nt; t++)
		{
			//initialise les debits au pas de temps courants
			for(index=0; index!=nbTronconSim; index++)
			{
				i = index_troncons[index];

				troncons[i]->ChangeDebitAmont(0.0f);
				troncons[i]->ChangeDebitAval(0.0f);
			}

			//----------------------
			for(auto iter=_troncons_tries.rbegin(); iter!=_troncons_tries.rend(); iter++)
			{
				i = *iter;
				CalculeTroncon(i, t, dt);

				//DEBITS AVAL MOY7J MIN
				if(_pOutput->_debit_aval_moy7j_min && t == nt-1)	//pour le dernier pdt interne seulement
				{
					troncons[i]->_debit_aval_7jrs.push_back(troncons[i]->PrendreDebitAvalMoyen());

					if(troncons[i]->_debit_aval_7jrs.size() > _q7avgNbPdt)
						troncons[i]->_debit_aval_7jrs.erase(troncons[i]->_debit_aval_7jrs.begin());
				}
			}

			//----------------------
			//for (index=0; index<mapShreveTroncon[1].size(); ++index)
			//	vFutures.push_back(async(launch::any, &HYDROTEL::ONDE_CINEMATIQUE_MODIFIEE::CalculeTroncon, this, mapShreveTroncon[1][index], t, dt));

			//for(auto &e : vFutures)
			//	e.get();

			//vFutures.clear();

			//----------------------
			//for (iIndex=1; iIndex<=iShreveMax; iIndex++)
			//{
			//	//if(mapShreveTroncon[iIndex].size() > 80)
			//	//{
			//		for (index=0; index<mapShreveTroncon[iIndex].size(); ++index)
			//			vFutures.push_back(async(launch::any, &HYDROTEL::ONDE_CINEMATIQUE_MODIFIEE::CalculeTroncon, this, mapShreveTroncon[iIndex][index], t, dt));

			//		for(auto &e : vFutures)
			//			e.get();
			//	//}
			//	//else
			//	//{
			//		//for (index=0; index<mapShreveTroncon[iIndex].size(); ++index)
			//		//	CalculeTroncon(mapShreveTroncon[iIndex][index], t, dt);
			//	//}

			//	vFutures.clear();
			//}
			//----------------------

		}	//for	t

		//output perimetre mouille si la grille P_H est disponible
		if(_hauteurPMVal.size() != 0)
		{
			oss.str("");
			oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_m) << setiosflags(ios::fixed);

			for(index = 0; index < troncons.PrendreNbTroncon(); ++index)
			{
				if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
				{
					if(_pOutput->_bSauvegardeTous ||
						find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
					{

						fVal = static_cast<float>(ObtientPMGrillePH(index, troncons[index]->_hauteurAvalMoy));
						oss << fVal << _pOutput->Separator();
					}
				}
			}
			
			str = oss.str();
			str = str.substr(0, str.length()-1); //enleve le dernier separateur
			_fichier_pm << str << endl;
		}

		if(_pOutput->_debit_aval_moy7j_min)
		{
			int iCurrentHour = date_courante.PrendreHeure();
			if(iCurrentHour == 24 - _sim_hyd.PrendrePasDeTemps())
			{
				//si c'est le dernier pdt de la journée en cours

				size_t j;
				float fQMoy7j;
				int iCurrentDay, iCurrentMonth, iCurrentYear;

				iCurrentDay = date_courante.PrendreJour();
				iCurrentMonth = date_courante.PrendreMois();
				iCurrentYear = date_courante.PrendreAnnee();

				if(iCurrentMonth == 1 && iCurrentDay == 1)
				{
					//yearly minimum
					if(_q7avgYearCurrentY == -1)
					{
						_q7avgYearCurrentY = 0;
						_q7avgYearStartY = iCurrentYear;
					}
					else
						++_q7avgYearCurrentY;

					if(troncons[index_troncons[0]]->_debit_aval_7jrs.size() == _q7avgNbPdt)	//si on a assez de pdt pour calculer la moyenne 7 jours
					{
						for(index=0; index!=nbTronconSim; index++)
						{
							i = index_troncons[index];
							if(_pOutput->_bSauvegardeTous || find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[i]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
							{
								//si le troncon est simulé et en output
								fQMoy7j = 0.0f;										//calcule la moyenne
								for(j=0; j!=_q7avgNbPdt; j++)						//
									fQMoy7j+= troncons[i]->_debit_aval_7jrs[j];		//
								fQMoy7j/= _q7avgNbPdt;								//

								_q7avgMinY[i].push_back(fQMoy7j);
							}
						}
					}
					else
					{
						for(index=0; index!=nbTronconSim; index++)
						{
							i = index_troncons[index];
							if(_pOutput->_bSauvegardeTous || find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[i]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
							{
								//si le troncon est simulé et en output
								_q7avgMinY[i].push_back(1000000000.0f);	//la moyenne au 1er janvier n'est pas disponible (elle sera disponible au plus tard le 7 janvier)
							}
						}
					}
				}
				else
				{
					//yearly minimum
					if(_q7avgYearCurrentY != -1)
					{
						if(troncons[index_troncons[0]]->_debit_aval_7jrs.size() == _q7avgNbPdt)	//si on a assez de pdt pour calculer la moyenne 7 jours
						{
							for(index=0; index!=nbTronconSim; index++)
							{
								i = index_troncons[index];
								if(_pOutput->_bSauvegardeTous || find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[i]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
								{
									//si le troncon est simulé et en output
									fQMoy7j = 0.0f;										//calcule la moyenne
									for(j=0; j!=_q7avgNbPdt; j++)						//
										fQMoy7j+= troncons[i]->_debit_aval_7jrs[j];		//
									fQMoy7j/= _q7avgNbPdt;								//

									if(fQMoy7j < _q7avgMinY[i][_q7avgYearCurrentY])
										_q7avgMinY[i][_q7avgYearCurrentY] = fQMoy7j;
								}
							}
						}

						if(iCurrentMonth == 12 && iCurrentDay == 31)
							_q7avgYearEndY = iCurrentYear;
					}

					//summery minimum
					if(iCurrentMonth >= 6 && iCurrentMonth <= 11)	//1er juin au 30 Novembre
					{
						if(iCurrentMonth == 6 && iCurrentDay == 1)
						{
							if(_q7avgYearCurrentS == -1)
							{
								_q7avgYearCurrentS = 0;
								_q7avgYearStartS = iCurrentYear;
							}
							else
								++_q7avgYearCurrentS;

							if(troncons[index_troncons[0]]->_debit_aval_7jrs.size() == _q7avgNbPdt)	//si on a assez de pdt pour calculer la moyenne 7 jours
							{
								for(index=0; index!=nbTronconSim; index++)
								{
									i = index_troncons[index];
									if(_pOutput->_bSauvegardeTous || find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[i]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
									{
										//si le troncon est simulé et en output
										fQMoy7j = 0.0f;										//calcule la moyenne
										for(j=0; j!=_q7avgNbPdt; j++)						//
											fQMoy7j+= troncons[i]->_debit_aval_7jrs[j];		//
										fQMoy7j/= _q7avgNbPdt;								//

										_q7avgMinS[i].push_back(fQMoy7j);
									}
								}
							}
							else
							{
								for(index=0; index!=nbTronconSim; index++)
								{
									i = index_troncons[index];
									if(_pOutput->_bSauvegardeTous || find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[i]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
									{
										//si le troncon est simulé et en output
										_q7avgMinS[i].push_back(1000000000.0f);	//la moyenne au 1er juin n'est pas disponible (elle sera disponible au plus tard le 7 juin)
									}
								}
							}
						}
						else
						{
							if(_q7avgYearCurrentS != -1)
							{
								if(troncons[index_troncons[0]]->_debit_aval_7jrs.size() == _q7avgNbPdt)	//si on a assez de pdt pour calculer la moyenne 7 jours
								{
									for(index=0; index!=nbTronconSim; index++)
									{
										i = index_troncons[index];
										if(_pOutput->_bSauvegardeTous || find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[i]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
										{
											//si le troncon est simulé et en output
											fQMoy7j = 0.0f;										//calcule la moyenne
											for(j=0; j!=_q7avgNbPdt; j++)						//
												fQMoy7j+= troncons[i]->_debit_aval_7jrs[j];		//
											fQMoy7j/= _q7avgNbPdt;								//

											if(fQMoy7j < _q7avgMinS[i][_q7avgYearCurrentS])
												_q7avgMinS[i][_q7avgYearCurrentS] = fQMoy7j;
										}
									}
								}

								if(iCurrentMonth == 11 && iCurrentDay == 30)
									_q7avgYearEndS = iCurrentYear;
							}
						}
					}
				}
			}
		}

		if (_sauvegarde_tous_etat || (_sauvegarde_etat && _date_sauvegarde_etat - pas_de_temps == date_courante))
			SauvegardeEtat(date_courante);

		ACHEMINEMENT_RIVIERE::Calcule();
	}


	//---------------------------------------------------------------------------------
	void ONDE_CINEMATIQUE_MODIFIEE::CalculeTroncon(size_t indexTroncon, int t, int dt)
	{
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();
		TRONCON::TYPE_TRONCON type_troncon = troncons[indexTroncon]->PrendreType();
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		
		MILIEUHUMIDE_RIVERAIN* pMilieuHumide = nullptr;
		BARRAGE_HISTORIQUE* barrage = nullptr;
		RIVIERE* riviere = nullptr;
		TRONCON* lacSL = nullptr;
		LAC* lac = nullptr;

		ostringstream oss;
		double dPdt;
		float qd, qamont, qAvalTemp, qAmontTemp, qAvalNew, qAmontNew, fPdt;
		float lrg, lng, pte, manning, qapportlat, hauteur, section, sur_q, haut, coefC, coefK;
		int annee, mois, jour, heure, ident;

		coefC = coefK = lrg = pte = manning = 0.0f;

		troncons[indexTroncon]->_prIndicePression = 0.0;	//indice de pression pour les prélèvements

		qd = 0.0f;

		if(_sim_hyd._bActiveTronconDeconnecte && _sim_hyd._mapIndexTronconDeconnecte.find(indexTroncon) != _sim_hyd._mapIndexTronconDeconnecte.end())
			qamont = 0.0f;	//troncon deconnecte
		else
			qamont = troncons[indexTroncon]->PrendreDebitAmont();

		qapportlat = troncons[indexTroncon]->PrendreApportLateral();

		switch (type_troncon)
		{
		case TRONCON::RIVIERE:

				riviere = static_cast<RIVIERE*>(troncons[indexTroncon]);

				lrg = riviere->PrendreLargeur();
				lng = riviere->PrendreLongueur();
				pte = riviere->PrendrePente();
				manning = riviere->PrendreManning();

				hauteur = 0.0f;
				section = 0.0f;

				// calcul milieu humide riverain
				if(_milieu_humide_riverain.size() > 0)
					pMilieuHumide = _milieu_humide_riverain[indexTroncon];

				if(pMilieuHumide)
				{
					sur_q = 0.0f;

					CalculMilieuHumideRiverain(dt, indexTroncon, troncons[indexTroncon], pMilieuHumide, sur_q, qd);

					if(pMilieuHumide->get_sauvegarde())
					{
						annee = date_courante.PrendreAnnee();
						mois = date_courante.PrendreMois();
						jour = date_courante.PrendreJour();
						heure = date_courante.PrendreHeure();

						haut = static_cast<float>(_hauteur[indexTroncon]);

						ident = troncons[indexTroncon]->PrendreIdent();

						oss.str("");
						oss << ident << _pOutput->Separator()
							<< annee << _pOutput->Separator()
							<< mois << _pOutput->Separator()
							<< jour << _pOutput->Separator()
							<< heure << _pOutput->Separator()
							<< setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed) << pMilieuHumide->get_wet_v() << _pOutput->Separator()	//m3
							<< pMilieuHumide->get_wet_a() << _pOutput->Separator()																	//m2
							<< setprecision(_pOutput->_nbDigit_m) << setiosflags(ios::fixed) << pMilieuHumide->get_wet_d() << _pOutput->Separator()	//m
							<< setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed) << sur_q << _pOutput->Separator()						//m3/s
							<< setprecision(_pOutput->_nbDigit_m) << setiosflags(ios::fixed) << haut << _pOutput->Separator()							//m
							<< setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed) << qd;												//m3/s

						m_wetfichier << oss.str() << endl;
					}
				}
				else
				{
					//-----------------------------------------------------------
					//Prélèvements
					//Pour le cas des troncons de type riviere non milieu humide

					if(_sim_hyd._pr->_bSimulePrelevements)
					{
						float apportTot, qdMin, fPrelevement, fPrelevement2, fPrelevementCulture, fPrelevementCulture2, fRejet;

						apportTot = qamont + qapportlat;
						if(apportTot > 0.0001f)
						{
							//prelevements
							if(troncons[indexTroncon]->_prPrelevementTotal != 0.0 || troncons[indexTroncon]->_prPrelevementCulture != 0.0)
							{
								fPrelevement = static_cast<float>(troncons[indexTroncon]->_prPrelevementTotal);
								fPrelevementCulture = static_cast<float>(troncons[indexTroncon]->_prPrelevementCulture);
	
								qdMin = max(apportTot * 0.001f, 0.0001f);

								if(apportTot - fPrelevement - fPrelevementCulture < qdMin)
								{
									fPrelevement2 = (apportTot - qdMin) * fPrelevement / (fPrelevement + fPrelevementCulture);					//atténuation proportionnelle
									fPrelevementCulture2 = (apportTot - qdMin) * fPrelevementCulture / (fPrelevement + fPrelevementCulture);	//

									fPrelevement = fPrelevement2;
									fPrelevementCulture = fPrelevementCulture2;
								}

								qamont-= ( (fPrelevement + fPrelevementCulture) * qamont / apportTot );
								qapportlat-= ( (fPrelevement + fPrelevementCulture) * qapportlat / apportTot );

								troncons[indexTroncon]->_prIndicePression = static_cast<double>( (apportTot - (qamont+qapportlat)) / apportTot );	//indice total de pression (0-1)

								//rejets
								if(fPrelevement != 0.0f && troncons[indexTroncon]->_prRejetTotal != 0.0)
								{
									fRejet = static_cast<float>(troncons[indexTroncon]->_prRejetTotal);
									if(fRejet > fPrelevement)
										fRejet = fPrelevement;

									qamont+= fRejet;
								}
							}
						}

						//effluents
						if(troncons[indexTroncon]->_prRejetEffluent != 0.0)
							qamont+= static_cast<float>(troncons[indexTroncon]->_prRejetEffluent);
					}
					//-----------------------------------------------------------

					TransfertRiviere(
						indexTroncon,
						dt, 
						lng, 
						lrg, 
						pte,				
						manning,
						_ocm[indexTroncon].qamont,
						_ocm[indexTroncon].qapportlat, 
						_ocm[indexTroncon].qaval,
						qamont, 
						qapportlat, 
						hauteur, 
						section, 
						qd);

					_hauteur[indexTroncon] = static_cast<double>(hauteur);	//m
				}
				break;

		case TRONCON::LAC:

				lac = static_cast<LAC*>(troncons[indexTroncon]);

				coefC = lac->PrendreC();
				coefK = lac->PrendreK();

				hauteur = 0.0f;

				TransfertLac(dt, 
					lac->PrendreSurface() * 1000000.0f, 
					coefC, 
					coefK, 
					_ocm[indexTroncon].qamont,
					_ocm[indexTroncon].qapportlat, 
					_ocm[indexTroncon].qaval, 
					qamont,
					qapportlat, 
					hauteur, 
					qd);

				_hauteur[indexTroncon] = static_cast<double>(hauteur);
				break;

		case TRONCON::LAC_SANS_LAMINAGE:

				lacSL = troncons[indexTroncon];
						
				if(_sim_hyd._bActiveTronconDeconnecte && _sim_hyd._mapIndexTronconDeconnecte.find(indexTroncon) != _sim_hyd._mapIndexTronconDeconnecte.end())
					qd = qapportlat;	//troncon deconnecte
				else
					qd = qapportlat + lacSL->PrendreDebitAmont();
				break;

		case TRONCON::BARRAGE_HISTORIQUE:

				barrage = static_cast<BARRAGE_HISTORIQUE*>(troncons[indexTroncon]);
				qd = barrage->PrendreDebit(_sim_hyd.PrendreDateCourante(), _sim_hyd.PrendrePasDeTemps());
				break;

		default:
			throw ERREUR("invalid river reach type");
		}

		//------------------------------------------------------------------------------------------------
		//Prélèvements
		//Fichier output test des prélèvements calculés (prelevements_calcule.csv)
		if(_sim_hyd._pr->_bSimulePrelevements && t == 0)
		{
			annee = date_courante.PrendreAnnee();
			mois = date_courante.PrendreMois();
			jour = date_courante.PrendreJour();
			heure = date_courante.PrendreHeure();

			//if(troncons[indexTroncon]->PrendreIdent() == 168)
			//{
			size_t i;

			if(_sim_hyd._pr->_tronconsPrelevementString.count(indexTroncon) != 0)
			{
				for(i=0; i!=_sim_hyd._pr->_tronconsPrelevementString[indexTroncon].size(); i++)
				{
					oss.str("");
					oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "PR " << _sim_hyd._pr->_tronconsPrelevementString[indexTroncon][i];
					_sim_hyd._pr->_ofsPrelevementCalcule << oss.str() << endl;
				}
			}
			if(_sim_hyd._pr->_tronconsPrelevementCultureString.count(indexTroncon) != 0)
			{
				for(i=0; i!=_sim_hyd._pr->_tronconsPrelevementCultureString[indexTroncon].size(); i++)
				{
					oss.str("");
					oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "PR " << _sim_hyd._pr->_tronconsPrelevementCultureString[indexTroncon][i];
					_sim_hyd._pr->_ofsPrelevementCalcule << oss.str() << endl;
				}
			}
			if(_sim_hyd._pr->_tronconsRejetString.count(indexTroncon) != 0)
			{
				for(i=0; i!=_sim_hyd._pr->_tronconsRejetString[indexTroncon].size(); i++)
				{
					oss.str("");
					oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "RE " << _sim_hyd._pr->_tronconsRejetString[indexTroncon][i];
					_sim_hyd._pr->_ofsPrelevementCalcule << oss.str() << endl;
				}
			}
			if(_sim_hyd._pr->_tronconsRejetEffluentString.count(indexTroncon) != 0)
			{
				for(i=0; i!=_sim_hyd._pr->_tronconsRejetEffluentString[indexTroncon].size(); i++)
				{
					oss.str("");
					oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "RE " << _sim_hyd._pr->_tronconsRejetEffluentString[indexTroncon][i];
					_sim_hyd._pr->_ofsPrelevementCalcule << oss.str() << endl;
				}
			}
			//}
		}
	
		//--------------------------------------------------------------------------------------------------------------
		//Prélèvements
		//Pour le cas des lacs, des rivieres avec milieu humide, des lacs sans laminage et des barrages avec historique

		if( _sim_hyd._pr->_bSimulePrelevements && !(type_troncon == TRONCON::RIVIERE && pMilieuHumide == nullptr) )
		{
			float qdMin, fPrelevement, fPrelevement2, fPrelevementCulture, fPrelevementCulture2, fRejet, qdIni, fDt;
			float hauteurMin, fHauteurPr, fHauteurCulture, fHauteurPr2, fHauteurCulture2, fHauteur, fHauteurRejet, fHauteurEff;
			//float qdPr, qdCulture, fHauteurTemp, qdTemp;	//pour tests

			//annee = mois = jour = heure = 0;
			fHauteur = -1.0f;
			fDt = static_cast<float>(dt);

			if(qd > 0.0001f)
			{
				//prélèvements
				if(troncons[indexTroncon]->_prPrelevementTotal != 0.0 || troncons[indexTroncon]->_prPrelevementCulture != 0.0)
				{
					fPrelevement = static_cast<float>(troncons[indexTroncon]->_prPrelevementTotal);
					fPrelevementCulture = static_cast<float>(troncons[indexTroncon]->_prPrelevementCulture);
					qdIni = qd;

					if(type_troncon == TRONCON::LAC)
					{
						fHauteur = pow(qd / coefC, 1.0f / coefK);
						
						fHauteurPr = fPrelevement * fDt / (lac->PrendreSurface() * 1000000.0f);					//on transforme le prelevement en hauteur d'eau en fonction de la durée du pas de temps interne dt
						fHauteurCulture = fPrelevementCulture * fDt / (lac->PrendreSurface() * 1000000.0f);		//

						hauteurMin = max( pow(qd * 0.001f / coefC, 1.0f / coefK), pow(0.0001f / coefC, 1.0f / coefK) );

						if(fHauteur - fHauteurPr - fHauteurCulture < hauteurMin)
						{
							fHauteurPr2 = (fHauteur - hauteurMin) * fHauteurPr / (fHauteurPr + fHauteurCulture);				//atténuation proportionnelle
							fHauteurCulture2 = (fHauteur - hauteurMin) * fHauteurCulture / (fHauteurPr + fHauteurCulture);		//

							fHauteurPr = fHauteurPr2;
							fHauteurCulture = fHauteurCulture2;
						}

						////pour tests
						//fHauteurTemp = fHauteur - fHauteurPr;
						//qdTemp = coefC * pow(fHauteurTemp, coefK);
						//qdPr = qd - qdTemp;

						//fHauteurTemp = fHauteur - fHauteurCulture;
						//qdTemp = coefC * pow(fHauteurTemp, coefK);
						//qdCulture = qd - qdTemp;
						////

						fHauteur-= (fHauteurPr + fHauteurCulture);

						qd = coefC * pow(fHauteur, coefK);
						_hauteur[indexTroncon] = fHauteur;
					}
					else
					{
						qdMin = max(qd * 0.001f, 0.0001f);

						if(qd - fPrelevement - fPrelevementCulture < qdMin)
						{
							fPrelevement2 = (qd - qdMin) * fPrelevement / (fPrelevement + fPrelevementCulture);					//atténuation proportionnelle
							fPrelevementCulture2 = (qd - qdMin) * fPrelevementCulture / (fPrelevement + fPrelevementCulture);	//

							fPrelevement = fPrelevement2;
							fPrelevementCulture = fPrelevementCulture2;
						}

						qd-= (fPrelevement + fPrelevementCulture);

						if(type_troncon == TRONCON::RIVIERE)
						{
							//calcul hauteur aval
							if(_hauteurMethodeCalcul == 1)
								_hauteur[indexTroncon] = pow((manning * qd / lrg), 0.6f) / pow(pte, 0.3f);
							else
							{
								if(_hauteurMethodeCalcul == 2)
									_hauteur[indexTroncon] = CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[indexTroncon], manning, pte);
								else
									_hauteur[indexTroncon] = ObtientHauteurGrilleQH(indexTroncon, qd);
							}
						}
					}

					troncons[indexTroncon]->_prIndicePression = static_cast<double>( (qdIni-qd) / qdIni );	//indice total de pression (0-1)

					////pour tests
					//qdIni = qd;
					//fRejet = 0.0f;
					////

					//rejets
					if(fPrelevement != 0.0f && troncons[indexTroncon]->_prRejetTotal != 0.0)
					{
						fRejet = static_cast<float>(troncons[indexTroncon]->_prRejetTotal);
						if(fRejet > fPrelevement)
							fRejet = fPrelevement;

						if(type_troncon == TRONCON::LAC)
						{
							if(fHauteur == -1.0f)
								fHauteur = pow(qd / coefC, 1.0f / coefK);

							fHauteurRejet = fRejet * fDt / (lac->PrendreSurface() * 1000000.0f);
							fHauteur+= fHauteurRejet;
							
							qd = coefC * pow(fHauteur, coefK);
							_hauteur[indexTroncon] = fHauteur;
						}
						else
						{
							qd+= fRejet;

							if(type_troncon == TRONCON::RIVIERE)
							{
								//calcul hauteur aval
								if(_hauteurMethodeCalcul == 1)
									_hauteur[indexTroncon] = pow((manning * qd / lrg), 0.6f) / pow(pte, 0.3f);
								else
								{
									if(_hauteurMethodeCalcul == 2)
										_hauteur[indexTroncon] = CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[indexTroncon], manning, pte);
									else
										_hauteur[indexTroncon] = ObtientHauteurGrilleQH(indexTroncon, qd);
								}
							}
						}
					}

					////pour tests
					//if(t == 0)
					//{
					//	if(troncons[indexTroncon]->PrendreIdent() == 168)
					//	{

					//	if(fPrelevement != 0.0f)
					//	{
					//		oss.str("");
					//		oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "PR TOTAL" << ";" << fPrelevement;
					//		_sim_hyd._pr->_ofsPrelevementEffectue << oss.str() << endl;
					//	}
					//	if(fPrelevementCulture != 0.0f)
					//	{
					//		oss.str("");
					//		oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "PR CULTURE" << ";" << fPrelevementCulture;
					//		_sim_hyd._pr->_ofsPrelevementEffectue << oss.str() << endl;
					//	}
					//	if(fRejet != 0.0f)
					//	{
					//		oss.str("");
					//		oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "RE TOTAL" << ";" << qd-qdIni;
					//		_sim_hyd._pr->_ofsPrelevementEffectue << oss.str() << endl;
					//	}
					//	}
					//}
					////
				}
			}

			//effluent
			if(troncons[indexTroncon]->_prRejetEffluent != 0.0)
			{
				//qdIni = qd;	//pour tests

				if(type_troncon == TRONCON::LAC)
				{
					if(fHauteur == -1.0f)
						fHauteur = pow(qd / coefC, 1.0f / coefK);

					fHauteurEff = static_cast<float>(troncons[indexTroncon]->_prRejetEffluent) * fDt / (lac->PrendreSurface() * 1000000.0f);
					fHauteur+= fHauteurEff;
						
					qd = coefC * pow(fHauteur, coefK);
					_hauteur[indexTroncon] = fHauteur;
				}
				else
				{
					qd+= static_cast<float>(troncons[indexTroncon]->_prRejetEffluent);

					if(type_troncon == TRONCON::RIVIERE)
					{
						//calcul hauteur aval
						if(_hauteurMethodeCalcul == 1)
							_hauteur[indexTroncon] = pow((manning * qd / lrg), 0.6f) / pow(pte, 0.3f);
						else
						{
							if(_hauteurMethodeCalcul == 2)
								_hauteur[indexTroncon] = CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[indexTroncon], manning, pte);
							else
								_hauteur[indexTroncon] = ObtientHauteurGrilleQH(indexTroncon, qd);
						}
					}
				}

				////pour tests
				//if(t == 0)
				//{
				//	if(troncons[indexTroncon]->PrendreIdent() == 168)
				//	{
				//	oss.str("");
				//	//oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "RE EFFLUENT" << ";" << static_cast<float>(troncons[indexTroncon]->_prRejetEffluent);
				//	oss << annee << "-" << setfill('0') << setw(2) << mois << "-" << setw(2) << jour << " " << setw(2) << heure << ";" << troncons[indexTroncon]->PrendreIdent() << ";" << "RE EFFLUENT" << ";" << qd-qdIni;
				//	_sim_hyd._pr->_ofsPrelevementEffectue << oss.str() << endl;
				//	}
				//}
				////
			}
		}
		//------------------------------------------------------------------------------------------------

		_ocm[indexTroncon].qamont = qamont;
		_ocm[indexTroncon].qaval = qd;
		_ocm[indexTroncon].qapportlat = qapportlat;

		//NOTE: a modifier pour la gestion des sorties multiples

		//conserve les debits pour l'iteration courante
		if(!troncons[indexTroncon]->PrendreTronconsAval().empty())
		{
			TRONCON* aval = troncons[indexTroncon]->PrendreTronconsAval()[0];

			////**********************************************************************************
			////il devrait y avoir 1 seul troncon aval, ne devrait donc pas être un vecteur, a moins de sorties multiples
			//if(troncons[indexTroncon]->PrendreTronconsAval().size() != 1)
			//	 std::cout << "********* troncon " << troncons[indexTroncon]->PrendreIdent() << ", nb troncon aval=" << troncons[indexTroncon]->PrendreTronconsAval().size() << endl;
			////**********************************************************************************

			if(aval != nullptr)
			{
				float qamontTemp;
						
				if(_sim_hyd._bActiveTronconDeconnecte && _sim_hyd._mapIndexTronconDeconnecte.find(indexTroncon) != _sim_hyd._mapIndexTronconDeconnecte.end())
					qamontTemp = 0.0f;
				else
					qamontTemp = aval->PrendreDebitAmont();

				qamontTemp+= qd;
				aval->ChangeDebitAmont(max(0.0f, qamontTemp));
			}
		}

		//calcule et conserve les débits au pas de temps externe de la simulation
		qAvalNew = max(0.0f, qd);
		troncons[indexTroncon]->ChangeDebitAval(qAvalNew);

		qAmontNew = max(0.0f, qamont);

		double qHauteurAvalNew = _hauteur[indexTroncon];
		if(qHauteurAvalNew < 0.0)
			qHauteurAvalNew = 0.0;

		if(t != 0)
		{
			fPdt = static_cast<float>(t);
			dPdt = static_cast<double>(t);

			qAvalTemp = troncons[indexTroncon]->PrendreDebitAvalMoyen();
			qAmontTemp = troncons[indexTroncon]->PrendreDebitAmontMoyen();

			qAvalNew = (qAvalTemp * fPdt + qAvalNew) / (fPdt + 1.0f);
			qAmontNew = (qAmontTemp * fPdt + qAmontNew) / (fPdt + 1.0f);

			qHauteurAvalNew = (troncons[indexTroncon]->_hauteurAvalMoy * dPdt + qHauteurAvalNew) / (dPdt + 1.0);
		}

		troncons[indexTroncon]->ChangeDebitAvalMoyen(qAvalNew);
		troncons[indexTroncon]->ChangeDebitAmontMoyen(qAmontNew);

		troncons[indexTroncon]->_hauteurAvalMoy = qHauteurAvalNew;
	}


	void ONDE_CINEMATIQUE_MODIFIEE::Termine()
	{
		_ocm.clear();
		_troncons_tries.clear();

		_milieu_humide_riverain.clear();
		_ocm_mh.clear();
		_hauteur.clear();

		if(m_wetfichier.is_open())
			m_wetfichier.close();

		if(_hauteurPMVal.size() != 0)
			_fichier_pm.close();

		if(_sim_hyd._pr->_bSimulePrelevements)
		{
			_sim_hyd._pr->_ofsPrelevementCalcule.close();
			//_sim_hyd._pr->_ofsPrelevementEffectue.close();
		}

		if(_pOutput->_debit_aval_moy7j_min)
		{
			TRONCONS& troncons = _sim_hyd.PrendreTroncons();
			size_t nbTroncon = troncons.PrendreNbTroncon();
			int x;

			if(_q7avgYearEndY != -1)	//sinon il n'y a pas eu d'année complete de simulé
			{
				ostringstream oss;
				ofstream ofs;
				size_t index;
				string str;
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "qmoy7j_min_annuel.csv") );

				ofs.open(nom_fichier);
				ofs << "qmoy7j_min_annuel (m3/s)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "annee\\troncon" << _pOutput->Separator();

				oss.str("");
				for(index=0; index!=nbTroncon; index++)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << _pOutput->Separator();
						}
					}
				}
				str = oss.str();
				if(str.length() != 0)
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
				ofs << str << endl;

				for(x=0; x!=_q7avgYearEndY-_q7avgYearStartY+1; x++)
				{
					oss.str("");			
					oss << _q7avgYearStartY+x << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed);

					for(index=0; index!=nbTroncon; index++)
					{
						if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
						{
							if(_pOutput->_bSauvegardeTous || 
								find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
							{
								oss << _q7avgMinY[index][x] << _pOutput->Separator();	//m3s
							}
						}
					}
			
					str = oss.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					ofs << str << endl;
				}

				ofs.close();
			}

			if(_q7avgYearEndS != -1)	//sinon il n'y a pas eu d'année complete de simulé
			{
				ostringstream oss;
				ofstream ofs;
				size_t index;
				string str;
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "qmoy7j_min_estival.csv") );

				ofs.open(nom_fichier);
				ofs << "qmoy7j_min_estival (m3/s)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "annee\\troncon" << _pOutput->Separator();

				oss.str("");
				for(index=0; index!=nbTroncon; index++)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << _pOutput->Separator();
						}
					}
				}
				str = oss.str();
				if(str.length() != 0)
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
				ofs << str << endl;

				for(x=0; x!=_q7avgYearEndS-_q7avgYearStartS+1; x++)
				{
					oss.str("");			
					oss << _q7avgYearStartS+x << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed);

					for(index=0; index!=nbTroncon; index++)
					{
						if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
						{
							if(_pOutput->_bSauvegardeTous || 
								find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
							{
								oss << _q7avgMinS[index][x] << _pOutput->Separator();	//m3s
							}
						}
					}
			
					str = oss.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					ofs << str << endl;
				}

				ofs.close();
			}
		}

		ACHEMINEMENT_RIVIERE::Termine();
	}


	float ONDE_CINEMATIQUE_MODIFIEE::Celerite(float lng, float lrg, float pte, float man, float qamont, float qaval)
	{
		// calcul un debit moyen sur le troncon
		float qmoy = (qamont + qaval) / 2.0f;

		// calcul de l'aire d'une section moyenne		
		float alpha = pow(man * pow(lrg, 0.67f), 0.6f);

		float beta = 0.6f;

		float r = (alpha * pow(pte, -beta / 2.0f) / lrg);

		float s = beta;

		float sf = (pte - r * (pow(qaval, s) - pow(qamont, s)) / lng);

		if (sf < 0.00125f)
		{
			sf = 0.00125f;
		}

		float section = (alpha * pow(qmoy, beta) * pow(sf, -beta / 2.0f));
		float celerite = 0;

		// calcul de la celerite
		if (section == 0.0f)
		{
			celerite = 0.0f;
		}
		else
		{
			celerite = 1.67f * qmoy / section;
		}

		return celerite;
	}

	size_t TronconToIndex(TRONCONS& troncons, TRONCON* troncon)
	{
		size_t index = 0;

		while (troncon != troncons[index])
			++index;

		return index;
	}

	void ONDE_CINEMATIQUE_MODIFIEE::TrieTroncons()
	{
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		//NOTE: a modifier pour la gestion des sorties multiples

		deque<TRONCON*> recherche;
		recherche.push_back(troncons.PrendreTronconsExutoire()[0]);

		vector<TRONCON*> troncons_tries;

		while (!recherche.empty())
		{
			TRONCON* troncon = recherche.front();
			troncons_tries.push_back(troncon);
			recherche.pop_front();

			auto troncons_amont = troncon->PrendreTronconsAmont();

			for (auto iter = begin(troncons_amont); iter != end(troncons_amont); ++iter)
				recherche.push_back(*iter);
		}

		const size_t nb_troncon = troncons.PrendreNbTroncon();
		vector<size_t> index_troncons = _sim_hyd.PrendreTronconsSimules();

		_troncons_tries.clear();

		for (size_t i = 0; i < nb_troncon; ++i)
		{
			size_t index_troncon = TronconToIndex(troncons, troncons_tries[i]);
		
			if (find(begin(index_troncons), end(index_troncons), index_troncon) != end(index_troncons))
				_troncons_tries.push_back( TronconToIndex(troncons, troncons_tries[i]) );
		}

		_troncons_tries.shrink_to_fit();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void ONDE_CINEMATIQUE_MODIFIEE::TransfertRiviere(size_t idxTroncon, int pdts, float lng, float lrg, float pte, float man, float qa,
														float ql,  float qb, float qc, float qm, float& hauteur, float& section, float& qd)
	{
		float alpha;				//
		float beta;					// Coefficients d'un troncon
		float r;					//
		float s;					//
		float c1, c2, c3, c4, c5;
		float v1, v2, v3, v4;
		float f0, f1;
		float fPdts;

		fPdts = static_cast<float>(pdts);

		// Initialisation des variables de calcul pour le troncon.
		alpha = pow(man*pow(lrg, 2.0f / 3.0f), 0.6f);

		beta = 0.6f;
		
		r = pow(alpha / lrg * pte, -0.3f);

		s = 0.6f;

		c1 = 2.0f * alpha * lng / fPdts;
		c2 = r / lng;
		
		c3 = pow(qc, s);

		c4 = pow(qb, beta);

		c5 = qc - qb + qa + ql + qm;

		qd = pow(pte, beta / 2.0f) / c1;

		qd = qd * (2.0f * (qa - qb) + ql + qm) + c4;
		if(qd <= 0.0f) 
			qd = (qa + qb) / 2.0f + (ql + qm) / 2.0f;

		if(qd <= 0.0f)
			qd = (qa + qc) / 2.0f + (ql + qm) / 2.0f;

		if(qd <= 0.0f)
			qd = qc + qm;

		qd = pow(qd, 1.0f / beta);

		if(qd == 0.0f) 
			qd = 1.0E-20f;

		int converge = 0;
		int iter = 0;

		while(iter < MAXITER && !converge)
		{
			++iter;
			v1 = pow(qd, s);

			v2 = pow(qd, beta);

			v3 = pte - c2 * (v1 - c3);

			if(v3 < pte)
			{
				v3 = pte;
				c2 = 0.0f;
			}

			v4 = pow(v3, -beta / 2.0f);

			f0 = qd + c1 * v4 * (v2 - c4) - c5;
			f1 = beta / 2.0f * v4 / v3 * c2 * s * v1 / qd * (v2 - c4);
			f1 = f1 + v4 * beta * v2 / qd;
			f1 = 1.0f + c1 * f1;

			qd = qd - f0 / f1;
			if(qd <= 0.0f)
				qd = 1.0E-20f;

			if(fabs(f0 / f1) < EPSILON) 
				converge = 1;
		}

		//calcul hauteur aval
		if(_hauteurMethodeCalcul == 1)
			hauteur = pow((man * qd / lrg), 0.6f) / pow(pte, 0.3f);
		else
		{
			if(_hauteurMethodeCalcul == 2)
				hauteur = static_cast<float>(CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[idxTroncon], man, pte));
			else
				hauteur = static_cast<float>(ObtientHauteurGrilleQH(idxTroncon, qd));
		}

		section = hauteur * lrg;									// section aval		//non utilisé
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------------------------
	void ONDE_CINEMATIQUE_MODIFIEE::TransfertLac(int dt, float aire, float c, float k, float qa, float ql, float qb, float qc, float qm, float& haut, float& qd)
	{
		float fDt, f0, f1;

		fDt = static_cast<float>(dt);

		// transforme le debit au pas precedent en hauteur.
		float hb = pow(qb / c, 1.0f / k);

		// calcule la premiere approximation de la hauteur du lac
		haut = max(0.0f, hb + ((qa + qc) / 2.0f - qb + (ql + qm) / 2.0f) * fDt / aire);

		int converge = 0;
		int iter = 0;

		while (iter < MAXITER && !converge)
		{
			++iter;

			f0 = haut - hb + (qb + (c * pow(haut, k)) - qa - qc - ql - qm) * fDt / aire / 2.0f;

			f1 = 1.0f + c * k * pow(haut, (k - 1.0f)) * fDt / aire / 2.0f;

			haut = haut - f0 / f1;

			if(fabs(f0 / f1) < EPSILON) 
				converge = 1;
		}

		// transforme la hauteur au pas courant en debit
		qd = c * pow(haut, k);		
	}


	void ONDE_CINEMATIQUE_MODIFIEE::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ONDE_CINEMATIQUE_MODIFIEE");

		istringstream iss;
		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ONDE_CINEMATIQUE_MODIFIEE", 1);

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		//parametres des troncons
		getline_mod(fichier, ligne); // commentaire ou METHODE_CALCUL_HAUTEUR

		if(ligne.size() > 23 && ligne.substr(0, 23) == "METHODE_CALCUL_HAUTEUR;")
		{
			ligne = ligne.substr(23, string::npos);
			iss.clear();
			iss.str(ligne);
			iss >> _hauteurMethodeCalcul;

			if(_hauteurMethodeCalcul < 1 || _hauteurMethodeCalcul > 3)
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES ONDE_CINEMATIQUE_MODIFIEE: parametre invalide: METHODE_CALCUL_HAUTEUR", 5);
			}

			getline_mod(fichier, ligne);
			getline_mod(fichier, ligne); // commentaire
		}

		for (size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
		{
			int ident;
			char c;
			float coef_rugo, coef_largeur;

			fichier >> ident >> c;
			fichier >> coef_rugo >> c;
			fichier >> coef_largeur;

			size_t index_troncon = troncons.IdentVersIndex(ident);

			ChangeOptimisationRugosite(index_troncon, coef_rugo);
			ChangeOptimisationLargeurRiviere(index_troncon, coef_largeur);
		}

		fichier.close();

		//lecture des parametres pour les milieux humides riverains
		if(_sim_hyd._bSimuleMHRiverain)
		{
			LectureMilieuHumideRiverain();
			LectureProfondeur();
		}
	}

	void ONDE_CINEMATIQUE_MODIFIEE::LectureParametresFichierGlobal()
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

		size_t nbGroupe, x, y, index_zone, index_troncon;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "ONDE CINEMATIQUE MODIFIEE")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(x == 0 && vValeur.size() == 1)
					{
						//lecture parametre METHODE_CALCUL_HAUTEUR
						_hauteurMethodeCalcul = static_cast<int>(vValeur[0]);

						if(_hauteurMethodeCalcul < 1 || _hauteurMethodeCalcul > 3)
							throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Parametre invalide: METHODE_CALCUL_HAUTEUR: ONDE CINEMATIQUE MODIFIEE.");

						++no_ligne;
						getline_mod(fichier, ligne);
						vValeur = extrait_fvaleur(ligne, ";");
					}

					if(vValeur.size() != 3)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide. ONDE CINEMATIQUE MODIFIEE.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. ONDE CINEMATIQUE MODIFIEE. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);
						index_troncon = _sim_hyd.PrendreTroncons().IdentVersIndex(zones[index_zone].PrendreTronconAval()->PrendreIdent());

						ChangeOptimisationRugosite(index_troncon, vValeur[1]);
						ChangeOptimisationLargeurRiviere(index_troncon, vValeur[2]);
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; ONDE CINEMATIQUE MODIFIEE; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele ONDE CINEMATIQUE MODIFIEE");

		//lecture des parametres pour les milieux humides riverains
		if(_sim_hyd._bSimuleMHRiverain)
		{
			LectureMilieuHumideRiverain();
			LectureProfondeur();
		}
	}


	void ONDE_CINEMATIQUE_MODIFIEE::LectureMilieuHumideRiverain()
	{
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();
		const size_t nb_troncon = troncons.PrendreNbTroncon();

		vector<string> sList;
		istringstream iss;
		ostringstream oss;
		string line;
		float uhrh_a, wet_a, wetaup, wetadra, wetdown, longueur, amont, aval, wetdnor, wetdmax, frac, ksat_bk, ksat_bs, th_aq, eauIni;
		bool sauvegarde;
		int lineNumber, nTronconId;

		ifstream file(_sim_hyd._nom_fichier_milieu_humide_riverain);
		if (!file)
			throw ERREUR_LECTURE_FICHIER( "FICHIER MILIEUX HUMIDES RIVERAINS" );

		lineNumber = 1;

		_milieu_humide_riverain.resize(nb_troncon, nullptr);

		try{
			
		getline_mod(file, line); //entete

		while(!file.eof())
		{
			++lineNumber;
			getline_mod(file, line);

			if(line != "")
			{
				SplitString(sList, line, ";", true, true);

				if(sList.size() >= 16)
				{
					iss.clear();
					iss.str(sList[0]);
					iss >> nTronconId;

					iss.clear();
					iss.str(sList[1]);
					iss >> uhrh_a;

					iss.clear();
					iss.str(sList[2]);
					iss >> wet_a;

					iss.clear();
					iss.str(sList[3]);
					iss >> wetaup;

					iss.clear();
					iss.str(sList[4]);
					iss >> wetadra;

					iss.clear();
					iss.str(sList[5]);
					iss >> wetdown;

					iss.clear();
					iss.str(sList[6]);
					iss >> longueur;

					iss.clear();
					iss.str(sList[7]);
					iss >> amont;

					iss.clear();
					iss.str(sList[8]);
					iss >> aval;

					iss.clear();
					iss.str(sList[9]);
					iss >> wetdnor;

					iss.clear();
					iss.str(sList[10]);
					iss >> wetdmax;

					iss.clear();
					iss.str(sList[11]);
					iss >> frac;

					iss.clear();
					iss.str(sList[12]);
					iss >> ksat_bk;

					iss.clear();
					iss.str(sList[13]);
					iss >> ksat_bs;

					iss.clear();
					iss.str(sList[14]);
					iss >> th_aq;

					iss.clear();
					iss.str(sList[15]);
					iss >> sauvegarde;

					if(sList.size() >= 17)
					{
						iss.clear();
						iss.str(sList[16]);
						iss >> eauIni;
					}
					else
						eauIni = 0.5f;	// eauIni = 1.0f;

					if(wetdnor > wetdmax)
						throw ERREUR("Reading of riparian wetlands: the wetdnor parameter must be less than the wetdmax parameter.");

					if(eauIni < 0.0f || eauIni > 1.0f)
						throw ERREUR("Reading of riparian wetlands: the eauIni parameter must be >= 0 and <= 1.");

					size_t index_troncon = troncons.IdentVersIndex(nTronconId);
			
					_milieu_humide_riverain[index_troncon] = new MILIEUHUMIDE_RIVERAIN(wet_a, wetaup, wetadra, wetdown,longueur, amont, aval, wetdnor, wetdmax, frac, sauvegarde, ksat_bk, ksat_bs, th_aq, eauIni);
				}
				else
				{
					oss.str("");
					oss << "ONDE_CINEMATIQUE_MODIFIEE; erreur LectureMilieuHumideRiverain: ligne no " << lineNumber << " invalide.";
					throw ERREUR(oss.str());
				}
			}
		}

		}
		catch(const ERREUR& err)
		{
			file.close();
			throw err;
		}

		catch(...)
		{
			if(!file.eof())
			{
				file.close();
				throw ERREUR("ONDE_CINEMATIQUE_MODIFIEE: error LectureMilieuHumideRiverain");
			}
		}

		file.close();
	}

	void ONDE_CINEMATIQUE_MODIFIEE::LectureProfondeur()
	{
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		if(!FichierExiste(_sim_hyd._nom_fichier_milieu_humide_profondeur_troncon))
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES MILIEUX HUMIDES PROFONDEUR TRONCONS");

		ifstream file(_sim_hyd._nom_fichier_milieu_humide_profondeur_troncon);

		string line;
		getline_mod(file, line); // ligne de commentaires

		for (size_t n = 0; n < troncons.PrendreNbTroncon(); ++n)
		{
			char c;
			int id;
			float tmp, profondeur;
			file >> id >> c 
					>> tmp >> c
					>> tmp >> c
					>> tmp >> c
					>> profondeur;

			size_t index_troncon = troncons.IdentVersIndex(id);
			TRONCON* troncon = troncons[index_troncon];
			TRONCON::TYPE_TRONCON type_troncon = troncon->PrendreType();
			if (type_troncon == TRONCON::RIVIERE)
			{
				RIVIERE* ptr = static_cast<RIVIERE*>(troncon);
				ptr->ChangeProfondeur(profondeur);
			}
			else
			{
				if(type_troncon == TRONCON::LAC)
				{
					LAC* ptr = static_cast<LAC*>(troncon);
					ptr->ChangeProfondeur(profondeur);
				}
				else
				{
					if(type_troncon == TRONCON::LAC_SANS_LAMINAGE)
					{
						LAC_SANS_LAMINAGE* ptr = static_cast<LAC_SANS_LAMINAGE*>(troncon);
						ptr->ChangeProfondeur(profondeur);
					}
					else
					{
						if(type_troncon != TRONCON::BARRAGE_HISTORIQUE)
							throw ERREUR("error: ONDE_CINEMATIQUE_MODIFIEE::LectureProfondeur: invalid river reach type");
					}
				}
			}
		}

		file.close();
	}

	void ONDE_CINEMATIQUE_MODIFIEE::SauvegardeParametres()
	{
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "METHODE_CALCUL_HAUTEUR;" << _hauteurMethodeCalcul << endl;
		fichier << endl;

		fichier << "TRONCON ID;OPTIMISATION RUGOSITE;OPTIMISATION LARGEUR RIVIERES" << endl;
		for (size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
		{
			fichier << troncons[index]->PrendreIdent() << ';';

			fichier << PrendreOptimisationRugosite(index) << ';';
			fichier << PrendreOptimisationLargeurRiviere(index) << endl;
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//Calcul des milieux humides riverains
	void ONDE_CINEMATIQUE_MODIFIEE::CalculMilieuHumideRiverain(int pdts, size_t index_troncon, TRONCON* pTroncon, MILIEUHUMIDE_RIVERAIN* wetland, float& sur_q, float& qd)
	{
		float fPdts;

		fPdts = static_cast<float>(pdts);

		const float chside_k = wetland->get_ksat_bk();	//channel bed hydraulic conductivity, moderate loss rate mm/h

		float chk = chside_k / 1000.0f * fPdts / 3600.0f; //(m)	//assigning channel and wetland bed hydrolic conductivity

		float wetk = wetland->get_ksat_bs() / 1000.0f * fPdts / 3600.0f;

		const float bb = wetland->get_th_aq(); //thickness of the aquifer for subsurface flow exchange (m)

		const float alfa = wetland->get_alfa();
		const float belta = wetland->get_belta();

		float fchn_d;
		float chd, chw;
		float man, lrg, pte, section, fQtempo;

		section = 0.0f;
		
		TRONCON::TYPE_TRONCON type_troncon = pTroncon->PrendreType();
		if(type_troncon != TRONCON::RIVIERE)
			throw ERREUR("error: ONDE_CINEMATIQUE_MODIFIEE::CalculMilieuHumideRiverain: invalid river reach type");

		RIVIERE* ptr = static_cast<RIVIERE*>(pTroncon);

		chd = ptr->PrendreProfondeur();
		chw = lrg = ptr->PrendreLargeur();
		man = ptr->PrendreManning();
		pte = ptr->PrendrePente();

		float v_area, v_grad; // area and gradient of subsurface flow between wetland and channel

		float subsur_q = 0.0f; // subsurface flow exchange

		// surface flow discharge and volume exchange between wetland and channel
		float v_volu; 
		float v_disc;

		float chq;
		float debitAmont;

		bool cond_type1 = false;
		bool cond_type2 = false;

		if(_sim_hyd._bActiveTronconDeconnecte && _sim_hyd._mapIndexTronconDeconnecte.find(index_troncon) != _sim_hyd._mapIndexTronconDeconnecte.end())
			debitAmont = 0.0f;
		else
			debitAmont = pTroncon->PrendreDebitAmont();

		if(_hauteurMethodeCalcul == 1)
			chq = pow(chd, 5.0f/3.0f) * pow(pte, 0.5f) * lrg / man; // m^3/s
		else
		{
			if(_hauteurMethodeCalcul == 2)
				chq = static_cast<float>(CalculDebitTrapezoidal(chd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
			else
				chq = static_cast<float>(ObtientDebitGrilleQH(index_troncon, chd));
		}

		float wyld;
		wyld = pTroncon->PrendreApportLateral();	// m^3/s

		//pTroncon->_prod_surf, _prod_hypo et _prod_base sont en mm
		float surq = pTroncon->_surf; // m^3/s
		float latq = pTroncon->_hypo; // m^3/s
		float gw_q = pTroncon->_base; // m^3/s

		// runoff from upwetland, wetland and downwetland drainage area
		float upwet_q = wyld * wetland->get_wetaup();
		
		float wet_q = (surq + latq) * wetland->get_wetadra() * fPdts;

		gw_q = gw_q * wetland->get_wetadra();
		float downwet_q = wyld * wetland->get_wetdown();

		float wetdnor = wetland->get_wetnd();	// normal depth
		float wetvnor = wetland->get_wetnvol();	// normal volume
		float wetanor = wetland->get_wetnsa();  // normal area

		if(chd < wetdnor)
		{
			chd = wetdnor;
			
			if(_hauteurMethodeCalcul == 1)
				chq = pow(chd, 5.0f/3.0f) * pow(pte, 0.5f) * lrg / man; // m^3/s
			else
			{
				if(_hauteurMethodeCalcul == 2)
					chq = static_cast<float>(CalculDebitTrapezoidal(chd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
				else
					chq = static_cast<float>(ObtientDebitGrilleQH(index_troncon, chd));
			}
		}

		float wetchn_d = chd - wetdnor;	// depth from channel bed to wetland bed

		float wet_v = wetland->get_wet_v();
		float wet_v_tmp = wet_v;

		float wet_d;

		float wet_a = wetland->get_wet_a();
		float wet_a_tmp = wet_a;
		int ident = pTroncon->PrendreIdent();

		float section_amont, qd_amont, hauteur_amont;

		// TransfertRiviere AMONT
		if (wetland->get_longueur_amont() > 0.0f)
		{
			TransfertRiviere(
				index_troncon,
				pdts, 
				wetland->get_longueur_amont(), 
				lrg, 
				pte, 
				man, 
				_ocm_mh[ident][0].qamont,
				_ocm_mh[ident][0].qapportlat, 
				_ocm_mh[ident][0].qaval,
				debitAmont,
				upwet_q,
				hauteur_amont,
				section_amont,
				qd_amont);
		}
		else
			qd_amont = debitAmont;

		_ocm_mh[ident][0].qamont = debitAmont;
		_ocm_mh[ident][0].qapportlat = upwet_q;
		_ocm_mh[ident][0].qaval = qd_amont;

		float qd_ini = qd_amont + gw_q + (wetk * wet_a_tmp) / fPdts;
		if(qd_ini == 0.0f) 
			qd_ini = 1.0E-20f;

		//calcul hauteur aval
		if(_hauteurMethodeCalcul == 1)
			fchn_d = pow((man * qd_ini / lrg), 0.6f) / pow(pte, 0.3f);
		else
		{
			if(_hauteurMethodeCalcul == 2)
				fchn_d = static_cast<float>(CalculHauteurEauTrapezoidal(qd_ini, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
			else
				fchn_d = static_cast<float>(ObtientHauteurGrilleQH(index_troncon, qd_ini));
		}

		// if no lateral surface flow

		wet_v = max(0.0f, wet_v_tmp + wet_q - (wetk * wet_a_tmp));
		wet_d = pow(wet_v, (1.0f - alfa)) / belta;
		wet_a = belta * pow(wet_v, alfa);

		if(fchn_d > chd || wet_d > wetdnor)
		{
			if(fchn_d > (wet_d + wetchn_d))
			{
				v_disc = qd_ini - chq;
				v_volu = v_disc * fPdts;
				fchn_d = chd;
				qd = chq;
				cond_type1 = true;
			}
			else if(fchn_d > chd)
			{
				v_volu = pow(belta * (fchn_d - wetchn_d), (1.0f / (1.0f - alfa))) - pow(belta * wet_d, (1.0f / (1.0f - alfa)));
				v_disc = v_volu / fPdts;
			}
			else
			{
				v_volu = pow(belta * wetdnor, (1.0f / (1.0f - alfa))) - pow(belta * wet_d, (1.0f / (1.0f - alfa)));
				v_disc = v_volu / fPdts;

				wet_v = wetvnor;
				wet_d = wetdnor;
				wet_a = wetanor;

				cond_type2 = true;
			}

			//TransfertRiviere
			if(cond_type1)
			{
				qd = qd_ini - v_disc;
				
				//calcul hauteur aval
				if(_hauteurMethodeCalcul == 1)
					fchn_d = pow((man * qd / lrg), 0.6f) / pow(pte, 0.3f);
				else
				{
					if(_hauteurMethodeCalcul == 2)
						fchn_d = static_cast<float>(CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
					else
						fchn_d = static_cast<float>(ObtientHauteurGrilleQH(index_troncon, qd));
				}

				wet_v = max(0.0f, wet_v_tmp + v_volu + wet_q - (wetk * wet_a_tmp));
				wet_d = pow(wet_v, (1.0f - alfa)) / belta;
				wet_a = belta * pow(wet_v, alfa);
			}

			//wetland water balance
			if(cond_type2)
			{
				qd = qd_ini - v_disc;
				
				//calcul hauteur aval
				if(_hauteurMethodeCalcul == 1)
					fchn_d = pow((man * qd / lrg), 0.6f) / pow(pte, 0.3f);
				else
				{
					if(_hauteurMethodeCalcul == 2)
						fchn_d = static_cast<float>(CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
					else
						fchn_d = static_cast<float>(ObtientHauteurGrilleQH(index_troncon, qd));
				}

				wet_v = max(0.0f, wet_v_tmp + v_volu + wet_q - (wetk * wet_a_tmp));
				wet_d = pow(wet_v, (1.0f - alfa)) / belta;
				wet_a = belta * pow(wet_v, alfa);
			}

			if((cond_type1 && wet_d <= wetdnor) || (cond_type2 && fchn_d <= chd))
			{
				// wetland storage less than normal storage
				v_area = bb * wetland->get_longueur();
				v_grad = (fchn_d - wetchn_d - wet_d) / (3.0f * chw);

				if(wet_v <= 0.0f)
					subsur_q = max(0.0f, chk * v_area * v_grad);
				else
					subsur_q = chk * v_area * v_grad;

				if(subsur_q > 0.0f)
				{
					if(_hauteurMethodeCalcul == 1)
						fQtempo = pow((wetchn_d + wet_d), 5.0f/3.0f) * pow(pte, 0.5f) * lrg / man;
					else
					{
						if(_hauteurMethodeCalcul == 2)
							fQtempo = static_cast<float>(CalculDebitTrapezoidal(wetchn_d + wet_d, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
						else
							fQtempo = static_cast<float>(ObtientDebitGrilleQH(index_troncon, wetchn_d + wet_d));
					}

					subsur_q = min(subsur_q, (qd - fQtempo) * fPdts);
				}

				if(subsur_q <= 0.0f)
				{
					if(fchn_d > wetchn_d)
						subsur_q = max(subsur_q, pow(belta * (fchn_d - wetchn_d),(1.0f / (1.0f - alfa))) - pow(belta * wet_d, (1.0f / (1.0f - alfa))));
					else
						subsur_q = max(subsur_q, -pow(belta * wet_d, (1.0f / (1.0f - alfa))));
				}

				//TransfertRiviere
				qd = qd - subsur_q / fPdts;
				
				//calcul hauteur aval
				if(_hauteurMethodeCalcul == 1)
					fchn_d = pow((man * qd / lrg), 0.6f) / pow(pte, 0.3f);
				else
				{
					if(_hauteurMethodeCalcul == 2)
						fchn_d = static_cast<float>(CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
					else
						fchn_d = static_cast<float>(ObtientHauteurGrilleQH(index_troncon, qd));
				}

				wet_v = max(0.0f, wet_v_tmp + v_volu + wet_q - (wetk * wet_a_tmp) + subsur_q);
				wet_d = pow(wet_v, (1.0f - alfa)) / belta;
				wet_a = belta * pow(wet_v, alfa);
			}
			else
			{
				// wetland storage more than normal storage
				float qq1 = min(0.0f, v_disc);
				float qq2 = max(0.0f, v_disc);
				float dd = 10.0f;

				int iter = 0;

				while(abs(dd) > 0.005f && iter < 30)
				{
					v_disc = (qq1 + qq2) / 2.0f;
					++iter;

					//wetland water depth
					v_volu = v_disc * fPdts;

					//TransfertRiviere
					qd = qd_ini - v_disc;
					
					//calcul hauteur aval
					if(_hauteurMethodeCalcul == 1)
						fchn_d = pow((man * qd / lrg), 0.6f) / pow(pte, 0.3f);
					else
					{
						if(_hauteurMethodeCalcul == 2)
							fchn_d = static_cast<float>(CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
						else
							fchn_d = static_cast<float>(ObtientHauteurGrilleQH(index_troncon, qd));
					}

					wet_v = max(0.0f, wet_v_tmp + v_volu + wet_q - (wetk * wet_a_tmp));
					wet_d = pow(wet_v, (1.0f - alfa)) / belta;
					wet_a = belta * pow(wet_v, alfa);

					if(fchn_d > chd && wet_d < wetdnor)
						dd = fchn_d - chd;
					else if(fchn_d < chd && wet_d > wetdnor)
						dd = wetdnor - wet_d;
					else
						dd = fchn_d - (wet_d + wetchn_d);

					//set next trial lateral flowrate
					if(v_volu < 0.0f && dd > 0.005f)
						qq1 = v_disc;
					else if(v_volu < 0.0f && dd <= 0.005f)
						qq2 = v_disc;
					else if(v_volu >= 0.0f && dd < 0.005f)
						qq2 = v_disc;
					else if(v_volu >= 0.0f && dd >= 0.005f)
						qq1 = v_disc;
				}
			}

			sur_q = v_disc;
			subsur_q = 0.0f;
		}
		else 
		{
			//low flow
			sur_q = 0.0f;
			v_area = bb * wetland->get_longueur();
			v_grad = (fchn_d - wetchn_d - wet_d) / (3.0f * chw);

			if(wet_v <= 0.0f)
				subsur_q = max(0.0f, chk * v_area * v_grad);
			else
				subsur_q = chk * v_area * v_grad;

			if(subsur_q > 0.0f)
			{
				if(_hauteurMethodeCalcul == 1)
					fQtempo = pow((wetchn_d + wet_d), 5.0f/3.0f) * pow(pte, 0.5f) * lrg / man;
				else
				{
					if(_hauteurMethodeCalcul == 2)
						fQtempo = static_cast<float>(CalculDebitTrapezoidal(wetchn_d + wet_d, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
					else
						fQtempo = static_cast<float>(ObtientDebitGrilleQH(index_troncon, wetchn_d + wet_d));
				}

				subsur_q = min(subsur_q, (qd_ini - fQtempo) * fPdts);
			}

			if(subsur_q <= 0.0f)
			{
				if(fchn_d > wetchn_d)
					subsur_q = max(subsur_q, pow(belta * (fchn_d - wetchn_d),(1.0f / (1.0f - alfa)))-pow(belta * wet_d, (1.0f / (1.0f - alfa))));
				else
					subsur_q = max(subsur_q,-pow(belta * wet_d, (1.0f / (1.0f - alfa))));
			}

			if(subsur_q / fPdts > qd_ini)
				subsur_q = 0.999f * qd_ini * fPdts;

			qd = qd_ini - subsur_q / fPdts;
			
			//calcul hauteur aval
			if(_hauteurMethodeCalcul == 1)
				fchn_d = pow((man * qd / lrg), 0.6f) / pow(pte, 0.3f);
			else
			{
				if(_hauteurMethodeCalcul == 2)
					fchn_d = static_cast<float>(CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
				else
					fchn_d = static_cast<float>(ObtientHauteurGrilleQH(index_troncon, qd));
			}

			wet_v = max(0.0f, wet_v_tmp + wet_q - (wetk * wet_a_tmp) + subsur_q);
			wet_d = pow(wet_v, 1.0f - alfa) / belta;
			wet_a = belta * pow(wet_v, alfa);
		}

		if(wet_v < 0.001f)
			wet_v = 0.001f;

		wet_d = pow(wet_v, (1.0f - alfa)) / belta;
		wet_a = belta * pow(wet_v, alfa);

		if(wet_v > wetland->get_wetmxvol())
		{
			v_volu = pow(belta * wetland->get_wetmxd(), (1.0f / (1.0f - alfa))) - pow(belta * wet_d, (1.0f / (1.0f - alfa)));

			sur_q = sur_q + v_volu / fPdts;

			wet_v = wetland->get_wetmxvol();
			wet_d = wetland->get_wetmxd();
			wet_a = wetland->get_wetmxsa();

			qd = qd_ini - sur_q;
						
			//calcul hauteur aval
			if(_hauteurMethodeCalcul == 1)
				fchn_d = pow((man * qd / lrg), 0.6f) / pow(pte, 0.3f);
			else
			{
				if(_hauteurMethodeCalcul == 2)
					fchn_d = static_cast<float>(CalculHauteurEauTrapezoidal(qd, lrg, _hauteurTrapezeSideSlope[index_troncon], man, pte));
				else
					fchn_d = static_cast<float>(ObtientHauteurGrilleQH(index_troncon, qd));
			}
		}

		_ocm_mh[ident][1].qamont = qd_amont;
		_ocm_mh[ident][1].qapportlat = gw_q - sur_q - (subsur_q / fPdts) + (wetk * wet_a_tmp) / fPdts;
		_ocm_mh[ident][1].qaval = qd;

		wetland->set_wet_a(wet_a);
		wetland->set_wet_d(wet_d);
		wetland->set_wet_v(wet_v);

		float rch_out, hauteur;

		//TransfertRiviere AVAL
		if(wetland->get_longueur_aval() > 0.0f)
		{
			TransfertRiviere(
				index_troncon,
				pdts, 
				wetland->get_longueur_aval(), 
				lrg, 
				pte, 
				man, 
				_ocm_mh[ident][2].qamont,
				_ocm_mh[ident][2].qapportlat, 
				_ocm_mh[ident][2].qaval,
				qd,
				downwet_q,
				hauteur,
				section,
				rch_out);
		}
		else
		{
			hauteur = fchn_d;
			rch_out = qd;
		}

		hauteur = max(0.0f, hauteur);

		_ocm_mh[ident][2].qamont = qd;
		_ocm_mh[ident][2].qapportlat = downwet_q;
		_ocm_mh[ident][2].qaval = rch_out;

		_hauteur[index_troncon] = static_cast<double>(hauteur);	//m
		qd = max(0.0f, rch_out);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//Calcul de la hauteur d'eau dans une section trapézoidale
	//ManningTiwariEtAl.m (Tiwari et al. (2012))
	double ONDE_CINEMATIQUE_MODIFIEE::CalculHauteurEauTrapezoidal(double dDischarge, double dLargeur, double dSideSlope, double dManningCoeff, double dLongitudinalSlope)
	{
		//Input Parameters for Trapezoidal cross section
		//
		//dDischarge: Q= input('discharge in cumec=');
		//dLargeur: b= input('bottom width of channel in m=');		//largeur du tronçon
		//dSideSlope: z= input('side slope H to V=');				//pente des berges
		//dManningCoeff: n= input('manning coefficient=');
		//dLongitudinalSlope: S= input('Longitudinal slope=');		//pente du troncon

		double yn, dyn, An, Tn, Pn, Rn, fn, ffn;	//, Dn;

		yn = 0.01; //yinitial	//initial guess to start the iterations
			
		//normal depth calculation
		dyn = 0.01;
		while(abs(dyn) > 0.0001)
		{
			An = dLargeur * yn + dSideSlope * pow(yn, 2.0);
			Tn = dLargeur + 2.0 * dSideSlope * yn;
			Pn = dLargeur + 2.0 * pow( (pow(dSideSlope, 2.0) + 1.0 ), 0.50) * yn;
			Rn = An / Pn;
			//Dn = An / (dLargeur + 2.0 * dSideSlope * yn);
			fn = sqrt(dLongitudinalSlope) * An * pow(Rn, 2.0/3.0) * pow(dManningCoeff, -1.0) - dDischarge;
			
			ffn = ( sqrt(dLongitudinalSlope) * pow(dManningCoeff, -1.0) ) * ( (pow(Rn, 2.0/3.0) * Tn) + (Tn / Pn) - (2.0 * yn *Rn / Pn) );
			yn = yn - fn / ffn;
			dyn = (-fn) / ffn;
		}

		return yn;	//Normaldepth
	}


	//----------------------------------------------------------------------------------------------------------------------------------------------------------------
	//Calcul du débit dans une section trapézoidale
	double ONDE_CINEMATIQUE_MODIFIEE::CalculDebitTrapezoidal(double dProfondeur, double dLargeur, double dSideSlope, double dManningCoeff, double dLongitudinalSlope)
	{
		double A, P, R, Q;

		A = dLargeur * dProfondeur + dSideSlope * pow(dProfondeur, 2.0);				//Aire d'un trapèze
		P = dLargeur + 2.0 * pow(pow(dSideSlope, 2.0) + 1.0, 0.5) * dProfondeur;		//Périmètre mouillé d'un trapèze
		R = A / P;																		//Rayon hydraulique
		Q = A * pow(R, 2.0/3.0) * pow(dLongitudinalSlope, 0.5) / dManningCoeff;			//Débit

		return Q;	//m3/s
	}


	//------------------------------------------------------------------------------------------------
	//Important: la première hauteur (colonne 2) doit etre 0
	void ONDE_CINEMATIQUE_MODIFIEE::LectureFichierDebitsHauteurs()
	{
		vector<string> sList;
		istringstream iss;
		double dval;
		size_t nbTroncon, x;
		string str, sFile;
		int ival, iNbTroncon;

		sFile = Combine(_sim_hyd.PrendreRepertoireProjet(), "physio/Q_H-ponderees.csv");	//priorité sur le fichier non-pondéré si les 2 existe
		if(!boost::filesystem::exists(sFile))
		{
			sFile = Combine(_sim_hyd.PrendreRepertoireProjet(), "physio/Q_H.csv");
			if(!boost::filesystem::exists(sFile))
				throw ERREUR("error reading file: file dont exist: " + sFile);
		}

		ifstream file(sFile);
		if(!file)
			throw ERREUR("error reading file: " + sFile);

		nbTroncon = _sim_hyd.PrendreTroncons().PrendreNbTroncon();
		iNbTroncon = static_cast<int>(nbTroncon);

		try{

		getline_mod(file, str);	//comment line		IDTroncon;Hauteur1;Hauteur2;Hauteur3;etc...
		SplitString(sList, str, ";", false, true);

		if(sList.size() < 4)
			throw ERREUR("error LectureFichierDebitsHauteurs: invalid columns number: " + sFile);

		iss.str(sList[2]);
		iss >> _hauteurHandIncrement;

		_hauteurHandValDebits.clear();
		_hauteurHandValDebits.resize(nbTroncon);

		while(!file.eof())
		{
			getline_mod(file, str);
			if(str != "")
			{
				SplitString(sList, str, ";", false, true);
				if(sList.size() < 4)
					throw ERREUR("error LectureFichierDebitsHauteurs: invalid columns number: " + sFile);

				iss.clear();
				iss.str(sList[0]);
				iss >> ival;	//id troncon

				if(ival < 1 || ival > iNbTroncon)
					throw ERREUR("error LectureFichierDebitsHauteurs: invalid river reach id: " + sList[0]);

				--ival;	//idtroncon -> zero based index

				for(x=1; x!=sList.size(); x++)
				{
					iss.clear();
					iss.str(sList[x]);
					iss >> dval;

					_hauteurHandValDebits[ival].push_back(dval);
				}
			}
		}

		file.close();

		}
		catch(const ERREUR& ex)
		{
			if(file && file.is_open())
				file.close();
			throw ex;
		}
		catch(...)
		{
			if(file && file.is_open())
				file.close();
			throw ERREUR("error reading file: exception: " + sFile);
		}
	}


	//------------------------------------------------------------------------------------------------
	//Important: la première hauteur (colonne 2) doit etre 0
	void ONDE_CINEMATIQUE_MODIFIEE::LectureFichierPerimetreMouilleHauteurs()
	{
		vector<string> sList;
		istringstream iss;
		double dval;
		size_t nbTroncon, x;
		string str, sFile;
		int ival, iNbTroncon;

		sFile = Combine(_sim_hyd.PrendreRepertoireProjet(), "physio/P_H-ponderees.csv");	//priorité sur le fichier pondéré si les 2 existes
		if(!boost::filesystem::exists(sFile))
		{
			sFile = Combine(_sim_hyd.PrendreRepertoireProjet(), "physio/P_H.csv");
			if(!boost::filesystem::exists(sFile))
				throw ERREUR("error reading file: file dont exist: " + sFile);
		}

		ifstream file(sFile);
		if(!file)
			throw ERREUR("error reading file: " + sFile);

		nbTroncon = _sim_hyd.PrendreTroncons().PrendreNbTroncon();
		iNbTroncon = static_cast<int>(nbTroncon);

		try{

		getline_mod(file, str);	//comment line		IDTroncon;Hauteur1;Hauteur2;Hauteur3;etc...
		SplitString(sList, str, ";", false, true);

		if(sList.size() < 4)
			throw ERREUR("error LectureFichierPerimetreMouilleHauteurs: invalid columns number: " + sFile);

		iss.str(sList[2]);
		iss >> _hauteurPMIncrement;

		_hauteurPMVal.clear();
		_hauteurPMVal.resize(nbTroncon);

		while(!file.eof())
		{
			getline_mod(file, str);
			if(str != "")
			{
				SplitString(sList, str, ";", false, true);
				if(sList.size() < 4)
					throw ERREUR("error LectureFichierPerimetreMouilleHauteurs: invalid columns number: " + sFile);

				iss.clear();
				iss.str(sList[0]);
				iss >> ival;	//id troncon

				if(ival < 1 || ival > iNbTroncon)
					throw ERREUR("error LectureFichierPerimetreMouilleHauteurs: invalid river reach id: " + sList[0]);

				--ival;	//idtroncon -> zero based index

				for(x=1; x!=sList.size(); x++)
				{
					iss.clear();
					iss.str(sList[x]);
					iss >> dval;

					_hauteurPMVal[ival].push_back(dval);
				}
			}
		}

		file.close();

		}
		catch(const ERREUR& ex)
		{
			if(file && file.is_open())
				file.close();
			throw ex;
		}
		catch(...)
		{
			if(file && file.is_open())
				file.close();
			throw ERREUR("error reading file: exception: " + sFile);
		}
	}


	double ONDE_CINEMATIQUE_MODIFIEE::ObtientHauteurGrilleQH(size_t idxTroncon, double dQ)
	{
		double hauteur, QrefMin, QrefMax, HrefMin, HrefMax;
		size_t i;

		if(dQ > _hauteurHandValDebits[idxTroncon][_hauteurHandValDebits[idxTroncon].size()-1])
		{
			QrefMax = _hauteurHandValDebits[idxTroncon][_hauteurHandValDebits[idxTroncon].size()-1];
			QrefMin = _hauteurHandValDebits[idxTroncon][_hauteurHandValDebits[idxTroncon].size()-2];
			HrefMax = (_hauteurHandValDebits[idxTroncon].size()-1) * _hauteurHandIncrement;
			HrefMin = (_hauteurHandValDebits[idxTroncon].size()-2) * _hauteurHandIncrement;

			hauteur = ( (dQ - QrefMin) * (HrefMax - HrefMin) / (QrefMax - QrefMin) ) + HrefMin;
		}
		else
		{
			for(i=0; i!=_hauteurHandValDebits[idxTroncon].size(); i++)
			{
				if(dQ <= _hauteurHandValDebits[idxTroncon][i])
					break;
			}

			if(dQ == _hauteurHandValDebits[idxTroncon][i])
				hauteur = i * _hauteurHandIncrement;
			else
			{
				QrefMax = _hauteurHandValDebits[idxTroncon][i];
				QrefMin = _hauteurHandValDebits[idxTroncon][i-1];
				HrefMax = i * _hauteurHandIncrement;
				HrefMin = (i-1) * _hauteurHandIncrement;

				hauteur = ( (dQ - QrefMin) * (HrefMax - HrefMin) / (QrefMax - QrefMin) ) + HrefMin;
			}
		}

		return hauteur;
	}


	double ONDE_CINEMATIQUE_MODIFIEE::ObtientDebitGrilleQH(size_t idxTroncon, double dHauteur)
	{
		double QrefMin, QrefMax, HrefMin, HrefMax, dQ;
		size_t i;

		if(dHauteur > (_hauteurHandValDebits[idxTroncon].size()-1) * _hauteurHandIncrement)
		{
			HrefMax = (_hauteurHandValDebits[idxTroncon].size()-1) * _hauteurHandIncrement;
			HrefMin = (_hauteurHandValDebits[idxTroncon].size()-2) * _hauteurHandIncrement;			
			QrefMax = _hauteurHandValDebits[idxTroncon][_hauteurHandValDebits[idxTroncon].size()-1];
			QrefMin = _hauteurHandValDebits[idxTroncon][_hauteurHandValDebits[idxTroncon].size()-2];
			
			dQ = ( (dHauteur - HrefMin) * (QrefMax - QrefMin) / (HrefMax - HrefMin) ) + QrefMin;
		}
		else
		{
			for(i=0; i!=_hauteurHandValDebits[idxTroncon].size(); i++)
			{
				if(dHauteur <= (i * _hauteurHandIncrement))
					break;
			}

			if(dHauteur == (i * _hauteurHandIncrement))
				dQ = _hauteurHandValDebits[idxTroncon][i];
			else
			{
				HrefMax = i * _hauteurHandIncrement;
				HrefMin = (i-1) * _hauteurHandIncrement;
				QrefMax = _hauteurHandValDebits[idxTroncon][i];
				QrefMin = _hauteurHandValDebits[idxTroncon][i-1];

				dQ = ( (dHauteur - HrefMin) * (QrefMax - QrefMin) / (HrefMax - HrefMin) ) + QrefMin;
			}
		}

		return dQ;
	}


	double ONDE_CINEMATIQUE_MODIFIEE::ObtientPMGrillePH(size_t idxTroncon, double dHauteur)
	{
		double PrefMin, PrefMax, HrefMin, HrefMax, dP;
		size_t i;

		if(dHauteur > (_hauteurPMVal[idxTroncon].size()-1) * _hauteurPMIncrement)
		{
			HrefMax = (_hauteurPMVal[idxTroncon].size()-1) * _hauteurPMIncrement;
			HrefMin = (_hauteurPMVal[idxTroncon].size()-2) * _hauteurPMIncrement;			
			PrefMax = _hauteurPMVal[idxTroncon][_hauteurPMVal[idxTroncon].size()-1];
			PrefMin = _hauteurPMVal[idxTroncon][_hauteurPMVal[idxTroncon].size()-2];
			
			dP = ( (dHauteur - HrefMin) * (PrefMax - PrefMin) / (HrefMax - HrefMin) ) + PrefMin;
		}
		else
		{
			for(i=0; i!=_hauteurPMVal[idxTroncon].size(); i++)
			{
				if(dHauteur <= (i * _hauteurPMIncrement))
					break;
			}

			if(dHauteur == (i * _hauteurPMIncrement))
				dP = _hauteurPMVal[idxTroncon][i];
			else
			{
				HrefMax = i * _hauteurPMIncrement;
				HrefMin = (i-1) * _hauteurPMIncrement;
				PrefMax = _hauteurPMVal[idxTroncon][i];
				PrefMin = _hauteurPMVal[idxTroncon][i-1];

				dP = ( (dHauteur - HrefMin) * (PrefMax - PrefMin) / (HrefMax - HrefMin) ) + PrefMin;
			}
		}

		return dP;
	}

}
