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
//USA
//

#include "bv3c1.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>
#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	BV3C1::BV3C1(SIM_HYD& sim_hyd)
		: BILAN_VERTICAL(sim_hyd, "BV3C1")
	{
		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;

		_netCdf_theta1 = NULL;
		_netCdf_theta2 = NULL;
		_netCdf_theta3 = NULL;
		_netCdf_etr1 = NULL;
		_netCdf_etr2 = NULL;
		_netCdf_etr3 = NULL;
		_netCdf_etr_total = NULL;
		_netCdf_q12 = NULL;
		_netCdf_q23 = NULL;
	}

	BV3C1::~BV3C1()
	{
	}

	float BV3C1::PrendreTheta1Initial(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _theta1_initial.size());
		return _theta1_initial[index_zone];
	}

	float BV3C1::PrendreTheta2Initial(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _theta2_initial.size());
		return _theta2_initial[index_zone];
	}

	float BV3C1::PrendreTheta3Initial(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _theta3_initial.size());
		return _theta3_initial[index_zone];
	}

	void BV3C1::ChangeTheta1Initial(size_t index_zone, float theta)
	{
		BOOST_ASSERT(index_zone < _theta1_initial.size() && theta >= 0 && theta <= 1);
		_theta1_initial[index_zone] = theta;
	}

	void BV3C1::ChangeTheta2Initial(size_t index_zone, float theta)
	{
		BOOST_ASSERT(index_zone < _theta2_initial.size() && theta >= 0 && theta <= 1);
		_theta2_initial[index_zone] = theta;
	}

	void BV3C1::ChangeTheta3Initial(size_t index_zone, float theta)
	{
		BOOST_ASSERT(index_zone < _theta3_initial.size() && theta >= 0 && theta <= 1);
		_theta3_initial[index_zone] = theta;
	}

	void BV3C1::ChangeKrec(size_t index_zone, float krec)
	{
		BOOST_ASSERT(index_zone < _krec.size());
		_krec[index_zone] = krec;
	}

	void BV3C1::ChangeDes(size_t index_zone, float des)
	{
		BOOST_ASSERT(index_zone < _des.size());
		_des[index_zone] = des;
	}

	void BV3C1::ChangeCin(size_t index_zone, float fCin)
	{
		BOOST_ASSERT(index_zone < _cin.size());
		_cin[index_zone] = fCin;
	}

	void BV3C1::ChangeCoefAssechement(size_t index_zone, float coef_assech)
	{
		BOOST_ASSERT(index_zone < _coef_assech.size());
		_coef_assech[index_zone] = coef_assech;
	}

	void BV3C1::ChangeIndexEaux(const std::vector<size_t>& index)
	{
		_index_eaux = index;
		_index_eaux.shrink_to_fit();
	}

	void BV3C1::ChangeIndexImpermeables(const std::vector<size_t>& index)
	{
		_index_impermeables = index;
		_index_impermeables.shrink_to_fit();
	}

	void BV3C1::ChangeNomFichierLectureEtat(std::string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}

	void BV3C1::ChangeRepertoireEcritureEtat(std::string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}

	void BV3C1::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}

	void BV3C1::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}


	void BV3C1::LectureEtat(DATE_HEURE date_courante)
	{
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; fichier etat BV3C; " + _nom_fichier_lecture_etat);

		vector<int> vValidation;
		string ligne;
		size_t index_zone;
		int iIdent;

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);	//date
		getline_mod(fichier, ligne);	//empty line
		getline_mod(fichier, ligne);	//colums descriptions

		ZONES& zones = _sim_hyd.PrendreZones();

		while(fichier.good())
		{
			getline_mod(fichier, ligne);
			if(ligne != "")
			{
				auto valeurs = extrait_fvaleur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);

				iIdent = static_cast<int>(valeurs[0]);
				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

					zones[index_zone]._theta1 = valeurs[1];
					zones[index_zone]._theta2 = valeurs[2];
					zones[index_zone]._theta3 = valeurs[3];

					vValidation.push_back(iIdent);
				}
			}
		}

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; fichier etat BV3C; id mismatch; " + _nom_fichier_lecture_etat);
	}

	void BV3C1::SauvegardeEtat(DATE_HEURE date_courante) const
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier;
		size_t x, nbSimuler, index_zone;
		string sSep = _sim_hyd._output._sFichiersEtatsSeparator;

		if(!RepertoireExiste(_repertoire_ecriture_etat))
			CreeRepertoire(_repertoire_ecriture_etat);

		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			        << "bilan_vertical_" 
			        << setw(4) << date_courante.PrendreAnnee() 
			        << setw(2) << date_courante.PrendreMois() 
			        << setw(2) << date_courante.PrendreJour() 
			        << setw(2) << date_courante.PrendreHeure() 
					<< ".csv";

		ofstream fichier(nom_fichier.str());
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier.exceptions(ios::failbit | ios::badbit);

		fichier << "ETATS BILAN VERTICAL" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier << "DATE_HEURE" << sSep << date_courante << endl;
		fichier << endl;

		fichier << "UHRH" << sSep << "THETA 1" << sSep << "THETA 2" << sSep << "THETA 3" << endl;

		ZONES& zones = _sim_hyd.PrendreZones();

		nbSimuler = _sim_hyd.PrendreZonesSimules().size();
		x = 0;

		for (index_zone=0; x<nbSimuler; index_zone++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				fichier << zone.PrendreIdent() << sSep;

				fichier << zones[index_zone]._theta1 << sSep;
				fichier << zones[index_zone]._theta2 << sSep;
				fichier << zones[index_zone]._theta3;

				fichier << endl;
				++x;
			}
		}

		fichier.close();
	}


	void BV3C1::Initialise()
	{
		_corrections_reserve_sol = _sim_hyd.PrendreCorrections().PrendreCorrectionsReserveSol();
		_corrections_saturation_sol = _sim_hyd.PrendreCorrections().PrendreCorrectionsSaturationReserveSol();

		ZONES& zones = _sim_hyd.PrendreZones();

		const size_t nb_zone = zones.PrendreNbZone();

		auto& occupation_sol = _sim_hyd.PrendreOccupationSol();
		auto& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();

		const size_t nb_propriete_hydrolique = propriete_hydroliques.PrendreNb();

		vector<size_t> index_autres;
		for (size_t index = 0; index < occupation_sol.PrendreNbClasse(); ++index)
		{
			if (find(begin(_index_eaux), end(_index_eaux), index) == end(_index_eaux) &&
				find(begin(_index_impermeables), end(_index_impermeables), index) == end(_index_impermeables))
			{
				index_autres.push_back(index);
			}
		}

		index_autres.shrink_to_fit();
		_index_autres.swap(index_autres);

		// calcul pourcentage des classes integrees

		_pourcentage_eau.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			float pourcentage = 0;

			for (auto index = begin(_index_eaux); index != end(_index_eaux); ++index)
				pourcentage += occupation_sol.PrendrePourcentage(index_zone, *index);

			_pourcentage_eau[index_zone] = pourcentage;
		}

		_pourcentage_impermeable.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			float pourcentage = 0;

			for (auto index = begin(_index_impermeables); index != end(_index_impermeables); ++index)
				pourcentage += occupation_sol.PrendrePourcentage(index_zone, *index);

			_pourcentage_impermeable[index_zone] = pourcentage;
		}

		_pourcentage_autre.resize(nb_zone, 0);
		for (size_t index_zone = 0; index_zone < nb_zone; ++index_zone)
			_pourcentage_autre[index_zone] = max(1.0f - (_pourcentage_impermeable[index_zone] + _pourcentage_eau[index_zone]), 0.0f);

		_q12.resize(nb_zone);
		_q23.resize(nb_zone);

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			zones[index_zone]._theta1 = _theta1_initial[index_zone] * propriete_hydroliques.PrendreProprieteHydroliqueCouche1(index_zone).PrendreThetas();
			zones[index_zone]._theta2 = _theta2_initial[index_zone] * propriete_hydroliques.PrendreProprieteHydroliqueCouche2(index_zone).PrendreThetas();
			zones[index_zone]._theta3 = _theta3_initial[index_zone] * propriete_hydroliques.PrendreProprieteHydroliqueCouche3(index_zone).PrendreThetas();
		}

		//initialisation milieux humides isoles
		if(_milieu_humide_isole.size() != 0)
		{
			// initialise le volume des milieux humides isoles au volume normal
			for (size_t idx = 0; idx < index_zones.size(); ++idx)
			{
				size_t index_zone = index_zones[idx];
				if (_milieu_humide_isole[index_zone])
				{
					_milieu_humide_isole[index_zone]->SetWetvol(_milieu_humide_isole[index_zone]->GetWetnvol());

					if(_milieu_humide_isole[index_zone]->GetSauvegarde())
					{
						SMilieuHumideResult* pRes = new SMilieuHumideResult;
						int id = zones[index_zone].PrendreIdent();					
						_milieu_humide_result[id] = pRes;
					}
				}
			}

			//fichier output
			string path = Combine(_sim_hyd.PrendreRepertoireResultat(), "wetland_isole.csv");
			m_wetfichier.open(path.c_str());
		}

		_b.resize(nb_propriete_hydrolique);
		_omegpi.resize(nb_propriete_hydrolique);
		_mm.resize(nb_propriete_hydrolique);
		_nn.resize(nb_propriete_hydrolique);

		for (size_t index = 0; index < nb_propriete_hydrolique; ++index)
		{
			auto& propriete = propriete_hydroliques.Prendre(index);

			_b[index] = 1.0f / propriete.PrendreLambda();
			_omegpi[index] = ((1.0f + 2.0f * _b[index]) / (2.0f + 2.0f * _b[index]));

			float res_pow = pow(_omegpi[index], -_b[index]);
			float psipi = propriete.PrendrePsis() * res_pow;

			res_pow = pow(1.0f - _omegpi[index], 2.0f);

			_mm[index] = psipi / res_pow - (psipi * _b[index] / _omegpi[index] / (1.0f - _omegpi[index]));
			_nn[index] = 2.0f * _omegpi[index] - psipi * _b[index] / _mm[index] / _omegpi[index] - 1.0f;
		}

		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeTheta1())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_theta1 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_theta1( Combine(_sim_hyd.PrendreRepertoireResultat(), "theta1.csv") );
				_fichier_theta1.open(nom_fichier_theta1);
				_fichier_theta1 << "theta 1" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_theta1 << str << endl;
			}
		}

		if (output.SauvegardeTheta2())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_theta2 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_theta2( Combine(_sim_hyd.PrendreRepertoireResultat(), "theta2.csv") );
				_fichier_theta2.open(nom_fichier_theta2);
				_fichier_theta2 << "theta 2" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_theta2 << str << endl;
			}
		}

		if (output.SauvegardeTheta3())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_theta3 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_theta3( Combine(_sim_hyd.PrendreRepertoireResultat(), "theta3.csv") );
				_fichier_theta3.open(nom_fichier_theta3);
				_fichier_theta3 << "theta 3" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_theta3 << str << endl;
			}
		}

		if (output.SauvegardeEtr1())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_etr1 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_etr1( Combine(_sim_hyd.PrendreRepertoireResultat(), "etr1.csv") );
				_fichier_etr1.open(nom_fichier_etr1);
				_fichier_etr1 << "etr 1" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr1 << str << endl;
			}
		}

		if (output.SauvegardeEtr2())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_etr2 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_etr2( Combine(_sim_hyd.PrendreRepertoireResultat(), "etr2.csv") );
				_fichier_etr2.open(nom_fichier_etr2);
				_fichier_etr2 << "etr 2" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr2 << str << endl;
			}
		}

		if (output.SauvegardeEtr3())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_etr3 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_etr3( Combine(_sim_hyd.PrendreRepertoireResultat(), "etr3.csv") );
				_fichier_etr3.open(nom_fichier_etr3);
				_fichier_etr3 << "etr 3" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr3 << str << endl;
			}
		}

		if (output.SauvegardeEtrTotal())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_etr_total = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_etr_total( Combine(_sim_hyd.PrendreRepertoireResultat(), "etr_total.csv") );
				_fichier_etr_total.open(nom_fichier_etr_total);
				_fichier_etr_total << "etr total" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr_total << str << endl;
			}
		}

		if (output.SauvegardeQ12())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_q12 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "q12.csv") );
				_fichier_q12.open(nom_fichier);
				_fichier_q12 << "q12 (mm/pdt)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_q12 << str << endl;
			}
		}

		if (output.SauvegardeQ23())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_q23 = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "q23.csv") );
				_fichier_q23.open(nom_fichier);
				_fichier_q23 << "q23 (mm/pdt)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_q23 << str << endl;
			}
		}

		if (!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		//determination du pas de temps interne minimum possible
		float fPDT = static_cast<float>(_sim_hyd.PrendrePasDeTemps());
		_fDTCMin = fPDT / (24.0f * 60.0f * 60.0f * 1000.0f);

		BILAN_VERTICAL::Initialise();
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void BV3C1::CalculeUHRH(int iIndexZone)
	{
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();
		ZONES& zones = _sim_hyd.PrendreZones();
		
		size_t index_zone = static_cast<size_t>(iIndexZone);

		ZONE& zone = zones[index_zone];

		float lprec = 0.0f;
		float leau  = 0.0f;
		float lruis = 0.0f;
		float lhyp  = 0.0f;
		float lbase = 0.0f;
		float lpinf = 0.0f;

		float tr = pas_de_temps;

		_q12[index_zone] = 0.0;	//la valeur est cumulé ds le pdt de bv3c, doit reinitialiser a 0 pour avoir en sortie la valeur pour le pdt courant
		_q23[index_zone] = 0.0;

		while (tr > 0)
		{
			float pinf = 0.0f;
			float ruis = 0.0f;

			CalculeRuisselement(zone, index_zone, pinf, ruis);

			float q2 = 0.0f;
			float q3 = 0.0f;

			float dtc = tr;
			TriCoucheOct97(zone, index_zone, pinf, ruis, q2, q3, dtc);
			tr -= dtc;

			float prec = zone.PrendreApport() / (1000.0f * pas_de_temps);	//mm -> m
			float pres = prec;

			if (_pourcentage_eau[index_zone] > 0.0f)
			{
				for (size_t index_cot = 0; index_cot < _index_eaux.size(); ++index_cot)
				{
					float poids = occupation_sol.PrendrePourcentage(index_zone, _index_eaux[index_cot]) / _pourcentage_eau[index_zone];
					if (poids > 0)
					{
						float etp = zone.PrendreEtpTotal() / 1000.0f;	//mm -> m
						pres -= etp * poids / pas_de_temps;
					}
				}
			}
					
			pres = max(0.0f, pres);

			lruis += ruis * dtc;
			lprec += prec * dtc; 
			leau  += pres * dtc; 
			lhyp  += q2 * dtc;
			lbase += q3 * dtc;
			lpinf += pinf * dtc;
		}

		float fsi = _pourcentage_impermeable[index_zone];
		float fse = _pourcentage_eau[index_zone];
		float fsa = _pourcentage_autre[index_zone];

		float prod_surf = lruis * fsa + leau * fse + lprec * fsi;
		float prod_hypo = lhyp * fsa;
		float prod_base = lbase * fsa;

		zone.ChangeProdSurf(max(0.0f, prod_surf * 1000.0f));	//m -> mm
		zone.ChangeProdHypo(max(0.0f, prod_hypo * 1000.0f));	//m -> mm
		zone.ChangeProdBase(max(0.0f, prod_base * 1000.0f));	//m -> mm

		// calcul des milieux humides isoles
		MILIEUHUMIDE_ISOLE* pMilieuHumide = nullptr;

		if (_milieu_humide_isole.size() > 0)
			pMilieuHumide = _milieu_humide_isole[index_zone];

		if (pMilieuHumide)
		{
			float evp, apport, superficie, prod, prodOld, surfOld, hypOld, baseOld, tempo;

			evp = zone.PrendreEtpTotal();
			apport = zone.PrendreApport();
					
			surfOld = zone.PrendreProdSurf();
			hypOld = zone.PrendreProdHypo();
			baseOld = zone.PrendreProdBase();

			prod = prodOld = surfOld + hypOld + baseOld;

			// calcul fraction superficie milieu humide
			superficie = pMilieuHumide->GetWetfr();

			// calcul fraction drainee milieu humide
			float wet_fr = pMilieuHumide->GetWetdrafr(); 

			apport = apport * superficie;

			float hru_ha = static_cast<float>(zone.PrendreSuperficie()) * 100.0f; //km2 -> ha
			int ident;

			ident = zone.PrendreIdent();

			CalculMilieuHumideIsole(pMilieuHumide, ident, hru_ha, wet_fr, evp, apport, prod, pas_de_temps);

			zone.ChangeApport(apport);
					
			if(prodOld != 0.0)
			{
				//repartie la production dans les 3 couches
				tempo = surfOld / prodOld * prod;
				zone.ChangeProdSurf(tempo);

				tempo = hypOld / prodOld * prod;
				zone.ChangeProdHypo(tempo);

				tempo = baseOld / prodOld * prod;
				zone.ChangeProdBase(tempo);
			}
			else
			{
				if(prod != 0.0f)
					zone.ChangeProdBase(prod);
			}
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void BV3C1::Calcule()
	{
		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();
		
		occupation_sol.LectureIndicesFolieres(date_courante.PrendreAnnee());
		occupation_sol.LectureProfondeursRacinaires(date_courante.PrendreAnnee());

		CalculeEtr();

		ZONES& zones = _sim_hyd.PrendreZones();

		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		size_t szindex;
		int index;

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();
		int nb_zone_simule = static_cast<int>(index_zones.size());

		// correction de la reserve sol
		for(auto iter = begin(_corrections_reserve_sol); iter != end(_corrections_reserve_sol); ++iter)
		{
			CORRECTION* correction = *iter;

			if (correction->Applicable(date_courante))
			{
				GROUPE_ZONE* groupe_zone = nullptr;

				switch (correction->PrendreTypeGroupe())
				{
				case TYPE_GROUPE_ALL:
					groupe_zone = _sim_hyd.PrendreToutBassin();
					break;

				case TYPE_GROUPE_HYDRO:
					groupe_zone = _sim_hyd.RechercheGroupeZone(correction->PrendreNomGroupe());
					break;

				case TYPE_GROUPE_CORRECTION:
					groupe_zone = _sim_hyd.RechercheGroupeCorrection(correction->PrendreNomGroupe());
					break;
				}

				auto& typesol = _sim_hyd.PrendreProprieteHydrotliques();

				for (szindex = 0; szindex < groupe_zone->PrendreNbZone(); ++szindex)
				{
					int ident = groupe_zone->PrendreIdent(szindex);
					size_t index_zone = zones.IdentVersIndex(ident);

					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
					{
						//si la zone est simulé on applique la correction
						auto& typeSolCouche1 = typesol.PrendreProprieteHydroliqueCouche1(index_zone);
						auto& typeSolCouche2 = typesol.PrendreProprieteHydroliqueCouche2(index_zone);
						auto& typeSolCouche3 = typesol.PrendreProprieteHydroliqueCouche3(index_zone);

						float saturationCouche1 = typeSolCouche1.PrendreThetas();
						float saturationCouche2 = typeSolCouche2.PrendreThetas();
						float saturationCouche3 = typeSolCouche3.PrendreThetas();

						zones[index_zone]._theta3 = (correction->PrendreCoefficientMultiplicatif() * 
							(zones[index_zone].PrendreZ11() * zones[index_zone]._theta1 + zones[index_zone].PrendreZ22() * zones[index_zone]._theta2 + zones[index_zone].PrendreZ33() * zones[index_zone]._theta3) - 
							(zones[index_zone].PrendreZ11() * zones[index_zone]._theta1 + zones[index_zone].PrendreZ22() * zones[index_zone]._theta2)) / zones[index_zone].PrendreZ33();

						if (zones[index_zone]._theta3 > saturationCouche3)
						{
							zones[index_zone]._theta2 += (zones[index_zone]._theta3 - saturationCouche3) * zones[index_zone].PrendreZ33() / zones[index_zone].PrendreZ22();
							zones[index_zone]._theta3 = saturationCouche3;
						
							if (zones[index_zone]._theta2 > saturationCouche2)
							{
								zones[index_zone]._theta1 += (zones[index_zone]._theta2 - saturationCouche2) * zones[index_zone].PrendreZ22() / zones[index_zone].PrendreZ11();
								zones[index_zone]._theta2 = saturationCouche2;
							
								if (zones[index_zone]._theta1 > saturationCouche1)
									zones[index_zone]._theta1 = saturationCouche1;
							}
						}
					}
				}
			}
		}

		// correction pour la saturation de la reserve en eau des 3 couches de sol
		for(auto iter = begin(_corrections_saturation_sol); iter != end(_corrections_saturation_sol); ++iter)
		{
			CORRECTION* correction = *iter;

			if (correction->Applicable(date_courante))
			{
				GROUPE_ZONE* groupe_zone = nullptr;

				switch (correction->PrendreTypeGroupe())
				{
				case TYPE_GROUPE_ALL:
					groupe_zone = _sim_hyd.PrendreToutBassin();
					break;

				case TYPE_GROUPE_HYDRO:
					groupe_zone = _sim_hyd.RechercheGroupeZone(correction->PrendreNomGroupe());
					break;

				case TYPE_GROUPE_CORRECTION:
					groupe_zone = _sim_hyd.RechercheGroupeCorrection(correction->PrendreNomGroupe());
					break;
				}

				auto& typesol = _sim_hyd.PrendreProprieteHydrotliques();

				for (szindex = 0; szindex < groupe_zone->PrendreNbZone(); ++szindex)
				{
					int ident = groupe_zone->PrendreIdent(szindex);
					size_t index_zone = zones.IdentVersIndex(ident);

					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
					{
						auto& typeSolCouche1 = typesol.PrendreProprieteHydroliqueCouche1(index_zone);
						auto& typeSolCouche2 = typesol.PrendreProprieteHydroliqueCouche2(index_zone);
						auto& typeSolCouche3 = typesol.PrendreProprieteHydroliqueCouche3(index_zone);

						float saturationCouche1 = typeSolCouche1.PrendreThetas();
						float saturationCouche2 = typeSolCouche2.PrendreThetas();
						float saturationCouche3 = typeSolCouche3.PrendreThetas();

						zones[index_zone]._theta1 = correction->PrendreCoeffSaturationCouche1() * saturationCouche1;
						zones[index_zone]._theta2 = correction->PrendreCoeffSaturationCouche2() * saturationCouche2;
						zones[index_zone]._theta3 = correction->PrendreCoeffSaturationCouche3() * saturationCouche3;
					}
				}
			}
		}

		//calcule

		int iIndexZone;
		for (index = 0; index < nb_zone_simule; ++index)
		{
			iIndexZone = static_cast<int>(index_zones[index]);
			CalculeUHRH(iIndexZone);
		}

		//sauvegarde des variables intermediaires

		OUTPUT& output = _sim_hyd.PrendreOutput();

		//milieu humide

		if (_milieu_humide_isole.size() > 0)
		{
			int jj, mm, aa, hh;

			aa = date_courante.PrendreAnnee();
			mm = date_courante.PrendreMois();
			jj = date_courante.PrendreJour();
			hh = date_courante.PrendreHeure();

			for (auto iter = _milieu_humide_result.begin(); iter != _milieu_humide_result.end(); iter++)
			{
				m_wetfichier 
					<< iter->first << output.Separator()
					<< aa << output.Separator()
					<< mm << output.Separator()
					<< jj << output.Separator()
					<< hh << output.Separator()
					<< iter->second->apport << output.Separator()	// mm
					<< iter->second->evp << output.Separator()		// mm
					<< iter->second->wetsep << output.Separator()	// m^3
					<< _milieu_humide_isole[zones.IdentVersIndex(iter->first)]->GetWetvol() << output.Separator()	// m^3
					<< iter->second->wetflwi << output.Separator()	// m^3
					<< iter->second->wetflwo << output.Separator()	// m^3
					<< iter->second->wetprod << std::endl;			// mm
			}
		}

		//

		size_t i, idx;
		string str;

		if (output.SauvegardeTheta1())
		{
			if (_netCdf_theta1 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_theta1[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]]._theta1;
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[szindex]._theta1 << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_theta1 << str << endl;
			}
		}

		if (output.SauvegardeTheta2())
		{
			if (_netCdf_theta2 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_theta2[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]]._theta2;
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[szindex]._theta2 << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_theta2 << str << endl;
			}
		}

		if (output.SauvegardeTheta3())
		{
			if (_netCdf_theta3 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_theta3[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]]._theta3;
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[szindex]._theta3 << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_theta3 << str << endl;
			}
		}

		if (output.SauvegardeEtr1())
		{
			if (_netCdf_etr1 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_etr1[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreEtr1();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[szindex].PrendreEtr1() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr1 << str << endl;
			}
		}

		if (output.SauvegardeEtr2())
		{
			if (_netCdf_etr2 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_etr2[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreEtr2();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[szindex].PrendreEtr2() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr2 << str << endl;
			}
		}

		if (output.SauvegardeEtr3())
		{
			if (_netCdf_etr3 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_etr3[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreEtr3();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[szindex].PrendreEtr3() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr3 << str << endl;
			}
		}

		if (output.SauvegardeEtrTotal())
		{
			if (_netCdf_etr_total != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_etr_total[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreEtrTotal();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[szindex].PrendreEtrTotal() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etr_total << str << endl;
			}
		}

		if (output.SauvegardeQ12())
		{
			if (_netCdf_q12 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_q12[idx+i] = _q12[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]];
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << _q12[szindex] << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_q12 << str << endl;
			}
		}

		if (output.SauvegardeQ23())
		{
			if (_netCdf_q23 != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_q23[idx+i] = _q23[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]];
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(6) << setiosflags(ios::fixed);

				for (szindex = 0; szindex < zones.PrendreNbZone(); ++szindex)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), szindex) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[szindex].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << _q23[szindex] << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_q23 << str << endl;
			}
		}

		//variable d'etat
		if (_sauvegarde_tous_etat || (_sauvegarde_etat && _date_sauvegarde_etat - pas_de_temps == date_courante))
			SauvegardeEtat(date_courante);

		BILAN_VERTICAL::Calcule();
	}


	void BV3C1::CalculMilieuHumideIsole(MILIEUHUMIDE_ISOLE* pMilieuHumide, int ident, float hru_ha, 
										float wet_fr, float evp, float& apport, float& prod, unsigned short pdt)
	{
		const float wet_k = pMilieuHumide->GetKsatBs(); // mm/h

		const float bw1 = pMilieuHumide->GetB();
		const float bw2 = pMilieuHumide->GetA();

		float wet_nvol = pMilieuHumide->GetWetnvol();
		float wet_vol = pMilieuHumide->GetWetvol();
		float wet_mxvol = pMilieuHumide->GetWetmxvol();

		// calcul de la surface
		float wetsa =  bw1 * pow(wet_vol, bw2) / 10000;

		// calcul du bilan hydrique 
		float wetev = 10.0f * pMilieuHumide->GetCEv() * evp * wetsa; 
		float wetsep = wet_k * wetsa * (pdt * 10);
		float wetpcp = apport * wetsa * 10;

		// calcul de l'apport dans le milieu humide
		float wetflwi = prod * 10 * (hru_ha * wet_fr - wetsa);
		prod = prod - prod * wet_fr;

		// calcul du nouveau volume
		wet_vol = wet_vol - wetsep - wetev + wetflwi + wetpcp;

		if (wet_vol < 0.001)
		{
			wetsep = wetsep + wet_vol;
			wet_vol = 0;

			if (wetsep < 0)
			{
				wetev = wetev + wetsep;
				wetsep = 0;
			}
		}

		// calcul de la production du milieu humide
		float wetflwo = 0.0f;
		
		if (wet_vol > wet_nvol)
		{
			if (wet_vol <= wet_mxvol)
			{
				wetflwo = (wet_vol - wet_nvol) / pMilieuHumide->GetCProd();
				wet_vol = wet_vol - wetflwo;
			}
			else
			{
				wetflwo = wet_vol - wet_mxvol;
				wet_vol = wet_mxvol;
			}
		}

		float wetprod = wetflwo / (hru_ha * 10) + wetsep / (hru_ha * 10);
		prod = prod + wetprod;

		pMilieuHumide->SetWetvol(wet_vol);

		if (pMilieuHumide->GetSauvegarde())
		{
			_milieu_humide_result[ident]->apport = apport;
			_milieu_humide_result[ident]->evp = evp;
			_milieu_humide_result[ident]->wetsep = wetsep;
			_milieu_humide_result[ident]->wetflwi = wetflwi;
			_milieu_humide_result[ident]->wetflwo = wetflwo;
			_milieu_humide_result[ident]->wetprod = wetprod;
		}
	}

	void BV3C1::Termine()
	{
		string str1, str2;

		_pourcentage_eau.clear();
		_pourcentage_impermeable.clear();
		_pourcentage_autre.clear();

		_q12.clear();
		_q23.clear();

		_b.clear();
		_omegpi.clear();
		_mm.clear();
		_nn.clear();

		_milieu_humide_isole.clear();

		for (auto iter = _milieu_humide_result.begin(); iter != _milieu_humide_result.end(); iter++)
			delete iter->second;
		_milieu_humide_result.clear();

		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeTheta1())
		{
			if (_netCdf_theta1 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-theta1.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "theta1.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-theta1", _netCdf_theta1, "[0-1]", "Teneur en eau de la couche 1");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "theta1", _netCdf_theta1, "[0-1]", "Teneur en eau de la couche 1");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_theta1;
			}
			else
				_fichier_theta1.close();
		}

		if (output.SauvegardeTheta2())
		{
			if (_netCdf_theta2 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-theta2.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "theta2.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-theta2", _netCdf_theta2, "[0-1]", "Teneur en eau de la couche 2");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "theta2", _netCdf_theta2, "[0-1]", "Teneur en eau de la couche 2");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_theta2;
			}
			else
				_fichier_theta2.close();
		}

		if (output.SauvegardeTheta3())
		{
			if (_netCdf_theta3 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-theta3.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "theta3.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-theta3", _netCdf_theta3, "[0-1]", "Teneur en eau de la couche 3");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "theta3", _netCdf_theta3, "[0-1]", "Teneur en eau de la couche 3");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_theta3;
			}
			else
				_fichier_theta3.close();
		}

		if (output.SauvegardeEtr1())
		{
			if (_netCdf_etr1 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-etr1.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "etr1.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-etr1", _netCdf_etr1, "mm", "Evapotranspiration reelle de la couche 1");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "etr1", _netCdf_etr1, "mm", "Evapotranspiration reelle de la couche 1");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_etr1;
			}
			else
				_fichier_etr1.close();
		}

		if (output.SauvegardeEtr2())
		{
			if (_netCdf_etr2 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-etr2.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "etr2.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-etr2", _netCdf_etr2, "mm", "Evapotranspiration reelle de la couche 2");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "etr2", _netCdf_etr2, "mm", "Evapotranspiration reelle de la couche 2");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_etr2;
			}
			else
				_fichier_etr2.close();
		}

		if (output.SauvegardeEtr3())
		{
			if (_netCdf_etr3 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-etr3.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "etr3.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-etr3", _netCdf_etr3, "mm", "Evapotranspiration reelle de la couche 3");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "etr3", _netCdf_etr3, "mm", "Evapotranspiration reelle de la couche 3");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_etr3;
			}
			else
				_fichier_etr3.close();
		}

		if (output.SauvegardeEtrTotal())
		{
			if (_netCdf_etr_total != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-etr_total.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "etr_total.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-etr_total", _netCdf_etr_total, "mm", "Evapotranspiration reelle totale");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "etr_total", _netCdf_etr_total, "mm", "Evapotranspiration reelle totale");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_etr_total;
			}
			else
				_fichier_etr_total.close();
		}

		if (output.SauvegardeQ12())
		{
			if (_netCdf_q12 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-q12.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "q12.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-q12", _netCdf_q12, "mm/pdt", "Ecoulement vertical de la couche 1 a 2");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "q12", _netCdf_q12, "mm/pdt", "Ecoulement vertical de la couche 1 a 2");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_q12;
			}
			else
				_fichier_q12.close();
		}

		if (output.SauvegardeQ23())
		{
			if (_netCdf_q23 != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-bv3c-q23.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "q23.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-bv3c-q23", _netCdf_q23, "mm/pdt", "Ecoulement vertical de la couche 2 a 3");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "q23", _netCdf_q23, "mm/pdt", "Ecoulement vertical de la couche 2 a 3");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_q23;
			}
			else
				_fichier_q23.close();
		}

		if(m_wetfichier.is_open())
			m_wetfichier.close();


		BILAN_VERTICAL::Termine();
	}

	float BV3C1::PrendreDes(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _des.size());
		return _des[index_zone];
	}

	float BV3C1::PrendreKrec(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _krec.size());
		return _krec[index_zone];
	}

	float BV3C1::PrendreCin(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _cin.size());
		return _cin[index_zone];
	}

	float BV3C1::PrendreCoeffAssechement(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _coef_assech.size());
		return _coef_assech[index_zone];
	}

	vector<size_t> BV3C1::PrendreIndexEaux() const
	{
		return _index_eaux;
	}

	vector<size_t> BV3C1::PrendreIndexImpermeables() const
	{
		return _index_impermeables;
	}

	float BV3C1::ConductiviteHydrolique(float theta, PROPRIETE_HYDROLIQUE& typesol, size_t index_sol)
	{
		float v_b = _b[index_sol];
		float omega = max(theta / typesol.PrendreThetas(), 0.05f);
		
		float res_pow = pow(omega, (2.0f * v_b + 3.0f));		
		//float res_pow = fastPow_f_p(omega, (2.0f * v_b + 3.0f));		
		//float res_pow = fastPow_f(omega, (2.0f * v_b + 3.0f));

		return typesol.PrendreKs() * res_pow;
	}

	float BV3C1::CalculePsi(float theta, PROPRIETE_HYDROLIQUE& typesol, size_t index_sol)
	{
		float psi;

		float omega = max(theta / typesol.PrendreThetas(), 0.05f);

		if (omega < _omegpi[index_sol])
		{
			float res_pow = pow(omega, -_b[index_sol]);			
			//float res_pow = fastPow_f_p(omega, -_b[index_sol]);
			//float res_pow = 1.0f/fastPow_f(omega, _b[index_sol]);

			psi = typesol.PrendrePsis() * res_pow;
		}
		else
		{
			psi = -_mm[index_sol] * (omega - _nn[index_sol]) * (omega - 1.0f);
		}

		return psi;
	}

	void BV3C1::TriCoucheOct97(ZONE& zone, size_t index_zone, float pinf, float& ruis, float& q2, float& q3, float& dtc)
	{
		int pas_de_temps = _sim_hyd.PrendrePasDeTemps();
		float fpas_de_temps = static_cast<float>(pas_de_temps);

		float pte = zone.PrendrePente();

		float z11 = zone.PrendreZ11();
		float z22 = zone.PrendreZ22();
		float z33 = zone.PrendreZ33();

		float krec = _krec[index_zone];

		auto& typesol = _sim_hyd.PrendreProprieteHydrotliques();

		auto& typeSolCouche1 = typesol.PrendreProprieteHydroliqueCouche1(index_zone);
		auto& typeSolCouche2 = typesol.PrendreProprieteHydroliqueCouche2(index_zone);
		auto& typeSolCouche3 = typesol.PrendreProprieteHydroliqueCouche3(index_zone);

		size_t tsol1 = typesol.PrendreIndexCouche1(index_zone);
		size_t tsol2 = typesol.PrendreIndexCouche2(index_zone);
		size_t tsol3 = typesol.PrendreIndexCouche3(index_zone);

		float k1 = ConductiviteHydrolique(zone._theta1, typeSolCouche1, tsol1);
		float k2 = ConductiviteHydrolique(zone._theta2, typeSolCouche2, tsol2);
		float k3 = ConductiviteHydrolique(zone._theta3, typeSolCouche3, tsol3);

		float psi1 = CalculePsi(zone._theta1, typeSolCouche1, tsol1);
		float psi2 = CalculePsi(zone._theta2, typeSolCouche2, tsol2);
		float psi3 = CalculePsi(zone._theta3, typeSolCouche3, tsol3);

		float k12 = max(k1, k2);
		float k23 = max(k2, k3);

		// estimation des lames par unites de surface horizontale
		float qq12, qq23;
		float fProfondeur, fTempo, fCouvertNival;

		qq12 = k12* (2.0f * (psi2 - psi1) / (z11 + z22) + 1.0f);		// m/h  (*m2 2D horizontal)
		qq23 = k23* (2.0f * (psi3 - psi2) / (z22 + z33) + 1.0f);		// m/h  (*m2 2D horizontal)
		q2 = k2 * sin(atan(pte)) * z22;									// m2/h (*m  1D horizontal)
		q3 = krec * z33 * zone._theta3;									// m2/h (*m  1D horizontal)

		// si le modele de temperature du sol a ete simule, on met a jour selon la profondeur du gel simule		
		fProfondeur = zone.PrendreProfondeurGel();
		fCouvertNival = zone.PrendreCouvertNival();	//mm

		if(fProfondeur > 0.0f && fCouvertNival < 10.0f)
		{
			fProfondeur/= 100.0f;	//cm -> m
			if(fProfondeur < z11)
				qq12 = (1.0f - fProfondeur / z11) * k12 * (2.0f * (psi2 - psi1) / (z11 + z22) + 1.0f);				// m/h  (*m2 2D horizontal)
			else
			{
				if(fProfondeur < z11 + z22)
				{
					qq12 = 0.0f;																					// m/h  (*m2 2D horizontal)
					qq23 = (1.0f - (fProfondeur - z11) / z22) * k23 * (2.0f * (psi3 - psi2) / (z22 + z33) + 1.0f);	// m/h  (*m2 2D horizontal)
					q2 = (1.0f - (fProfondeur - z11) / z22) * k2 * sin(atan(pte)) * z22;							// m2/h (*m  1D horizontal)
				}
				else
				{
					qq12 = 0.0f;																					// m/h  (*m2 2D horizontal)
					qq23 = 0.0f;																					// m/h  (*m2 2D horizontal)
					q2 = 0.0f;																						// m2/h (*m  1D horizontal)
					
					fTempo = 1.0f - (fProfondeur - (z11 + z22)) / z33;
					if(fTempo >= 0.5f)
						fTempo = 0.5f;

					q3 = fTempo * krec * z33 * zone._theta3;														// m2/h (*m  1D horizontal)
				}
			}
		}		

		float v_etr1 = zone.PrendreEtr1() / 1000.0f;	//mm/pdt -> m/pdt
		float v_etr2 = zone.PrendreEtr2() / 1000.0f;	//mm/pdt -> m/pdt
		float v_etr3 = zone.PrendreEtr3() / 1000.0f;	//mm/pdt -> m/pdt

		v_etr1/= pas_de_temps;	//m/pdt -> m/h
		v_etr2/= pas_de_temps;	//m/pdt -> m/h
		v_etr3/= pas_de_temps;	//m/pdt -> m/h

		// si il y  a de l'infiltration : pas de temps est au maximum de 1 heure 
		if(pinf > 0.0f)
		{
			if(dtc > 1.0f)
				dtc = 1.0f;
		}

		// transformation des flux en quantite d'eau relative a la profondeur des couches
		float q12z = qq12 / z11;
		float q23z = qq23 / z22;
		float q2s = q2 / z22;

		float theta1 = zone._theta1;
		float theta2 = zone._theta2;
		float theta3 = zone._theta3;

		// determination du pas de temps interne a BV3C
		float fCin = _cin[index_zone];

		
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//correctif pas de temps bv3c, 2019/02/05	//revision 2019/09/11
		
		float dtcTemp, dVal1, dVal2, fVal;
		bool bDtcMod;
		int iVal;

		bDtcMod = false;
		dtcTemp = dVal1 = dVal2 = 0.0f;
		
		if ((fabs(q12z * dtc) >= fCin * theta1) || (fabs((q23z + q2s) * dtc) >= fCin * theta2))
		{
			//couche1
			if (theta1 != 0.0f && q12z != 0.0f)
				dVal1 = fCin * theta1 / fabs(q12z);

			//couche2
			if (theta2 != 0.0f && q23z + q2s != 0.0f)
				dVal2 = fCin * theta2 / fabs(q23z + q2s);

			if (dVal1 != 0.0f && dVal2 != 0.0f)
			{
				if (dVal1 < dVal2)
					dtcTemp = dVal1;
				else
					dtcTemp = dVal2;
			}
			else
			{
				if (dVal1 != 0.0f)
					dtcTemp = dVal1;
				else
					dtcTemp = dVal2;
			}

			if (dtcTemp != 0.0f)
			{
				if(dtcTemp < _fDTCMin)
					dtcTemp = _fDTCMin;

				fVal = fpas_de_temps / dtcTemp;

				iVal = static_cast<int>(fVal);	//valeur maximum possible pour iVal: 86 400 000
				//if(iVal == std::numeric_limits<int>::min() || iVal == std::numeric_limits<int>::max())
				//	throw ERREUR("Erreur: BV3C::TriCoucheOct97: depassement de capacite lors du calcul du pas de temps interne (iVal).");

				if (iVal % 2 == 0)
					dtcTemp = pas_de_temps / (iVal + 2.0f);	//iVal est un nombre pair
				else
					dtcTemp = pas_de_temps / (iVal + 1.0f);	//iVal est un nombre impair

				dtcTemp = min(dtc, dtcTemp);

				bDtcMod = true;
			}
		}

		//correctif pas de temps bv3c, 2019/01/15

		//if ((fabs(q12z * dtc) >= fCin * theta1) || (fabs((q23z + q2s) * dtc) >= fCin * theta2))
		//{
		//	dtcTemp = min(fCin * theta1 / fabs(q12z), fCin * theta2 / fabs(q23z + q2s));
		//	iVal = static_cast<int>(pas_de_temps / dtcTemp);

		//	if (iVal < 0)
		//		iVal = iVal;

		//	if(iVal % 2 == 0)
		//		dtcTemp = pas_de_temps / (iVal + 2.0f);	//iVal est un nombre pair
		//	else
		//		dtcTemp = pas_de_temps / (iVal + 1.0f);	//iVal est un nombre impair

		//	dtcTemp = min(dtc, dtcTemp);

		//	bDtcMod = true;
		//}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


		if ((fabs(q12z * dtc) >= fCin * theta1) || (fabs((q23z + q2s) * dtc) >= fCin * theta2))
		{
			dtc = pas_de_temps / 48.0f;
			if ((fabs(q12z * dtc) >= fCin * theta1) || (fabs((q23z + q2s) * dtc) >= fCin * theta2))
			{
				dtc = pas_de_temps / 288.0f;
				if ((fabs(q12z * dtc) >= fCin * theta1) || (fabs((q23z + q2s) * dtc) >= fCin * theta2))
				{
					dtc = pas_de_temps/1152.0f;
				}
			}
		}

		if(bDtcMod)
			dtc = min(dtc, dtcTemp);

		theta1+= dtc * (pinf - qq12 - v_etr1) / z11;
		theta2+= (dtc / z22) * (qq12 - qq23 - v_etr2 - q2);
		theta3+= (dtc / z33) * (qq23 - q3 - v_etr3);

		float surplus = 0;

		float saturationCouche1 = typeSolCouche1.PrendreThetas();
		float saturationCouche2 = typeSolCouche2.PrendreThetas();
		float saturationCouche3 = typeSolCouche3.PrendreThetas();

		if (theta3 > saturationCouche3 || theta2 > saturationCouche2 || theta1 > saturationCouche1)
		{
			if (theta3 > saturationCouche3)
			{
				surplus = (theta3 - saturationCouche3) * z33;
				qq23 -= surplus / dtc;
				theta2 += surplus / z22;
				theta3 = saturationCouche3;
			}

			if (theta2 > saturationCouche2 && theta3 < saturationCouche3)
			{
				surplus = (theta2 - saturationCouche2) * z22;
				qq23 += surplus / dtc;
				theta3 += surplus / z33;
				theta2 = saturationCouche2;

				if (theta3 > saturationCouche3)
				{
					surplus = (theta3 - saturationCouche3) * z33;
					qq23 -= surplus / dtc;
					theta2 += surplus / z22;
					theta3 = saturationCouche3;
				}
			}

			if (theta2 > saturationCouche2)
			{
				surplus = (theta2 - saturationCouche2) * z22;
				qq12 -= surplus / dtc;
				theta1 += surplus / z11;
				theta2 = saturationCouche2;
			}

			if (theta1 > saturationCouche1 && (theta2 < saturationCouche2 || theta3 < saturationCouche3))
			{
				surplus = (theta1 - saturationCouche1) * z11;
				qq12 += surplus / dtc;
				theta2 += surplus / z22;
				theta1 = saturationCouche1;

				if (theta2 > saturationCouche2 && theta3 < saturationCouche3)
				{
					surplus = (theta2 - saturationCouche2) * z22;
					qq23 += surplus / dtc;
					theta3 += surplus / z33;
					theta2 = saturationCouche2;
					if (theta3 > saturationCouche3)
					{
						surplus = (theta3 - saturationCouche3) * z33;
						qq23 -= surplus / dtc;
						theta2 += surplus / z22;
						theta3 = saturationCouche3;
					}
				}

				if (theta2 > saturationCouche2)
				{
					surplus = (theta2 - saturationCouche2) * z22;
					qq12 -= surplus / dtc;
					theta1 += surplus / z11;
					theta2 = saturationCouche2;
				}
			}

			if (theta1 > saturationCouche1)
			{
				ruis += ((theta1 - saturationCouche1) * z11 / dtc);
				theta1 = saturationCouche1;
			}
		}

		if (theta1 < 0.0f)
		{
			qq12 += theta1 * z11 / dtc;
			theta2 += theta1 * z11 / z22;
			theta1 = 0.0f;
		}

		if (theta2 < 0.0f)
		{
			qq23 += theta2 * z22 / dtc;
			theta3 += theta2 * z22 / z33;
			theta2 = 0.0f;
		}

		if (theta3 < 0.0f)
		{
			if (fabs(z33 * theta3) < z22 * theta2)
			{
				qq23 -= theta3 * z33 / dtc;
				theta2 += theta3 * z33 / z22;
				theta3 = 0.0f;
			}
			else
			{
				if (fabs(z33 * theta3) < z22 * theta2 + z11 * theta1)
				{
					qq23 -= theta3 * z33 / dtc;
					qq12 -= theta2 * z22 / dtc + theta3 * z33 / dtc;
					theta1 += theta2 * z22 / z11 + theta3 * z33 / z11;
					theta2 = 0.0f;
					theta3 = 0.0f;
				}
				else
				{
					// manque d'eau
					theta1 = 0.0f;
					theta2 = 0.0f;
					theta3 = 0.0f;
				}
			}
		}

		zone._theta1 = theta1;
		zone._theta2 = theta2;
		zone._theta3 = theta3;

		// il faut conserver la somme des lames transitees sur le pas de temps externe
		float q12 = _q12[index_zone];
		float q23 = _q23[index_zone];

		q12 += qq12 * dtc;
		q23 += qq23 * dtc;

		_q12[index_zone] = q12;
		_q23[index_zone] = q23;
	}

	void BV3C1::CalculeRuisselement(ZONE& zone, size_t index_zone, float& pinf, float& ruis)
	{
		float apport = zone.PrendreApport();

		if (apport > 0.0f)
		{			
			float prec = apport / (1000.0f * _sim_hyd.PrendrePasDeTemps()); // * 1000 -> mm a m

			// utilise la couche1
			auto& typesol = _sim_hyd.PrendreProprieteHydrotliques().PrendreProprieteHydroliqueCouche1(index_zone);

			float ks = typesol.PrendreKs();
			float fCouvertNival = zone.PrendreCouvertNival();	//mm

			if(zone.PrendreProfondeurGel() > 0.0f && fCouvertNival < 10.0f)
				pinf = 0.0f;
			else
			{
				if (zone._theta1 == typesol.PrendreThetas())
					pinf = 0.0f;
				else if (prec > ks) 
					pinf = ks;
				else
					pinf = prec;
			}

			ruis = prec - pinf;
		}
		else
		{
			pinf = 0.0f;
			ruis = 0.0f;
		}
	}

	void BV3C1::CalculeEtr()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();
		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();


		for (size_t index = 0; index < index_zones.size(); ++index)
		{
			size_t index_zone = index_zones[index];

			ZONE& zone = zones[index_zone];

			float etr1 = 0.0f;
			float etr2 = 0.0f;
			float etr3 = 0.0f;

			auto& typesol = propriete_hydroliques.PrendreProprieteHydroliqueCouche1(index_zone);

			for (auto iter = begin(_index_autres); iter != end(_index_autres); ++iter)
			{
				float etp = zone.PrendreEtp(*iter) / 1000.0f;	//mm -> m

				if (etp > 0.0f)
				{
					float z = occupation_sol.PrendreProfondeurRacinaire(*iter, date_courante.PrendreJourJulien());
					float indice_foliaire = occupation_sol.PrendreIndiceFoliaire(*iter, date_courante.PrendreJourJulien());

					float evapo = etp * exp(-_des[index_zone] * indice_foliaire);

					if(typesol.PrendreThetacc() <= 0.0f || typesol.PrendreThetapf() <= 0.0f)
						throw ERREUR_LECTURE_FICHIER("PARAMETRES PROPRIETES HYDRAULIQUE INVALIDE");

					float ftheta = min(max(zones[index_zone]._theta1 - typesol.PrendreThetapf(), 0.0f) / (typesol.PrendreThetacc() - typesol.PrendreThetapf()), 1.0f);
					float kas = ((1.0f - exp(-typesol.PrendreAlpha() * ftheta)) / (1.0f - 2.0f * exp(-typesol.PrendreAlpha()) + exp(-typesol.PrendreAlpha() * ftheta)));
					
					float esnu = (_coef_assech[index_zone] * kas) * evapo;
					
					etr1 = etr1 + esnu;

					float dz1 = 0.0f;
					float dz2 = 0.0f;
					float dz3 = 0.0f;

					//profondeur [m]
					float z1 = zone.PrendreZ11();
					float z2 = zone.PrendreZ11() + zone.PrendreZ22();
					float z3 = zone.PrendreZ11() + zone.PrendreZ22() + zone.PrendreZ33();

					if (z > 0.0f  &&  z <= z1)
					{
						dz1 = z;
						dz2 = 0.0f;
						dz3 = 0.0f;
					}
					else if (z > z1 && z <= z2)
					{
						dz1 = z1;
						dz2 = z - z1;
						dz3 = 0.0f;
					}
					else if (z > z2 &&  z <= z3)
					{
						dz1 = z1;
						dz2 = z2 - z1;
						dz3 = z - z2;
					}
					else if (z > z3)
					{
						dz1 = z1;
						dz2 = z2 - z1;
						dz3 = z3 - z2;
						z = z3;
					}

					if (z > 0.0f)
					{
						float theta = (zones[index_zone]._theta1 * dz1 + zones[index_zone]._theta2 * dz2 + zones[index_zone]._theta3 * dz3) / z;

						ftheta = min(max(theta - typesol.PrendreThetapf(), 0.0f) / (typesol.PrendreThetacc() - typesol.PrendreThetapf()), 1.0f);
						float kat = (float)((1.0f - exp(-typesol.PrendreAlpha() * ftheta)) / (1.0f - 2.0f * exp(-typesol.PrendreAlpha()) + exp(-typesol.PrendreAlpha() * ftheta)));

						float tp = (_coef_assech[index_zone] * kat) * ((etp - evapo) * (BETA + (1.0f - BETA) * esnu / evapo));

						// repartition entre les couches de l'etr
						if ((theta * z) != 0.0f)
						{
							etr1 += tp * (zones[index_zone]._theta1 * dz1) / (theta * z);
							etr2 += tp * (zones[index_zone]._theta2 * dz2) / (theta * z);
							etr3 += tp * (zones[index_zone]._theta3 * dz3) / (theta * z);
						}
					}

				}
			}

			zone.ChangeEtr1(etr1 * 1000.0f);	//m -> mm
			zone.ChangeEtr2(etr2 * 1000.0f);	//m -> mm
			zone.ChangeEtr3(etr3 * 1000.0f);	//m -> mm
		}
	}

	void BV3C1::ChangeNbParams(const ZONES& zones)
	{
		const size_t nb_zone = zones.PrendreNbZone();

		_theta1_initial.resize(nb_zone, 0.9f);
		_theta2_initial.resize(nb_zone, 0.9f);
		_theta3_initial.resize(nb_zone, 0.9f);

		ZONES& zones2 = _sim_hyd.PrendreZones();
		for (size_t index=0; index<nb_zone; index++)
			zones2[index].ChangeZ1Z2Z3(0.1f, 0.5f, 1.5f);	//profondeurs

		_krec.resize(nb_zone, 0.000001f);
		_des.resize(nb_zone, 0.6f);
		_cin.resize(nb_zone, 0.3f);
		_coef_assech.resize(nb_zone, 1.0f);
	}

	void BV3C1::LectureParametres()
	{
		if(_sim_hyd._fichierParametreGlobal)
		{
			LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
			return;
		}

		int ident;
		char c;
		float z11, z22, z33, theta1, theta2, theta3, des, rec, coef_assech, fCin;
		size_t index_zone;

		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( PrendreNomFichierParametres() );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES BV3C");

		fichier.exceptions(ios::failbit | ios::badbit);

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES HYDROTEL VERSION")
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES BV3C", 1);
		}

		getline_mod(fichier, ligne);
		lire_cle_valeur(fichier, cle, valeur);
		getline_mod(fichier, ligne);

		// lecture des classes integrees
		lire_cle_valeur(fichier, cle, valeur);
		ChangeIndexImpermeables( extrait_valeur(valeur) );

		lire_cle_valeur(fichier, cle, valeur);
		ChangeIndexEaux( extrait_valeur(valeur) );

		getline_mod(fichier, ligne);

		//parametres des UHRH
		getline_mod(fichier, ligne); // commentaire

		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier >> ident >> c;
				
			fichier >> z11 >> c;
			fichier >> z22 >> c;
			fichier >> z33 >> c;

			fichier >> theta1 >> c;
			fichier >> theta2 >> c;
			fichier >> theta3 >> c;

			fichier >> des >> c;
			fichier >> rec >> c;
			fichier >> coef_assech >> c;
			fichier >> fCin;

			index_zone = zones.IdentVersIndex(ident);

			zones[index_zone].ChangeZ1Z2Z3(z11, z11 + z22, z11 + z22 + z33);
				
			ChangeTheta1Initial(index_zone, theta1);
			ChangeTheta2Initial(index_zone, theta2);
			ChangeTheta3Initial(index_zone, theta3);

			ChangeDes(index_zone, des);
			ChangeKrec(index_zone, rec);
			ChangeCin(index_zone, fCin);
			ChangeCoefAssechement(index_zone, coef_assech);
		}

		fichier.close();

		//lecture des parametres pour les milieux humides isolées
		if(_sim_hyd._bSimuleMHIsole)
			LectureMilieuHumideIsole();
	}

	void BV3C1::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; " + _sim_hyd._nomFichierParametresGlobal );

		fichier.exceptions(ios::failbit | ios::badbit);

		bool bOK = false;

		try{

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; " + _sim_hyd._nomFichierParametresGlobal, 1 );

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "BV3C")
			{
				++no_ligne;
				getline_mod(fichier, ligne);				
				ChangeIndexImpermeables(extrait_valeur(ligne));

				++no_ligne;
				getline_mod(fichier, ligne);				
				ChangeIndexEaux(extrait_valeur(ligne));

				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 11)
						throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; BV3C; " + _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; BV3C; " + _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						zones[index_zone].ChangeZ1Z2Z3(vValeur[1], vValeur[1] + vValeur[2], vValeur[1] + vValeur[2] + vValeur[3]);
				
						ChangeTheta1Initial(index_zone, vValeur[4]);
						ChangeTheta2Initial(index_zone, vValeur[5]);
						ChangeTheta3Initial(index_zone, vValeur[6]);

						ChangeDes(index_zone, vValeur[7]);
						ChangeKrec(index_zone, vValeur[8]);
						ChangeCoefAssechement(index_zone, vValeur[9]);
						ChangeCin(index_zone, vValeur[10]);						
					}
				}

				bOK = true;
				break;
			}

			++no_ligne;
		}

		fichier.close();

		}
		catch(...)
		{
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; BV3C; " + _sim_hyd._nomFichierParametresGlobal);
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; BV3C; " + _sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele BV3C");

		//lecture des parametres pour les milieux humides isolées
		if(_sim_hyd._bSimuleMHIsole)
			LectureMilieuHumideIsole();
	}

	void BV3C1::LectureMilieuHumideIsole()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream file(_sim_hyd._nom_fichier_milieu_humide_isole);
		if (!file)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES MILIEUX HUMIDES ISOLES" );

		//int lineNumber = 1;

		string line;
		getline_mod(file, line);  // ligne de commentaires

		const size_t nb_zone = zones.PrendreNbZone();
		_milieu_humide_isole.resize(nb_zone, 0);

		getline_mod(file, line);
		while (!line.empty())
		{
			line.erase(remove(line.begin(), line.end(), 'f'), line.end());	//HACK; pour fixer un ancien bug dans le fichier d'entree

			istringstream iss(line);

			// lecture des donnees
			int nUhrhId;
			float fSuperficie, fSuperficieWet, fWetDraFr, frac, wetdnor, wetdmax, ksat_bs, c_ev, c_prod;
			char c;
			bool sauvegarde;

			iss >> nUhrhId >> c
				>> fSuperficie >> c
				>> fSuperficieWet >> c
				>> fWetDraFr >> c
				>> frac >> c
				>> wetdnor >> c
				>> wetdmax >> c
				>> ksat_bs >> c
				>> c_ev >> c
				>> c_prod >> c
				>> sauvegarde;

			size_t index;
			string mess;

			try{
			index = zones.IdentVersIndex(nUhrhId);
			}
			catch(const exception& e)
			{
				mess = e.what();

				try{
				index = zones.IdentVersIndex(-nUhrhId);
				}
				catch(...)
				{
					mess = "Lecture des milieux humides isoles; " + mess;
					throw ERREUR(mess);
				}
			}
			
			if(fWetDraFr == 0.0)
				fWetDraFr = fSuperficieWet / fSuperficie;

			_milieu_humide_isole[index] = new MILIEUHUMIDE_ISOLE(
				fSuperficieWet, (fSuperficieWet / fSuperficie), fWetDraFr, frac, wetdnor, wetdmax, sauvegarde, ksat_bs, c_ev, c_prod, 1.0f);

			//++lineNumber;
			getline_mod(file, line);
		}

		file.close();
	}


	void BV3C1::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModeleWithoutVersion() << endl;
		fichier << endl;

		fichier << "CLASSE INTEGRE IMPERMEABLE;";
		auto index_impermeables = PrendreIndexImpermeables();
		for (auto iter = begin(index_impermeables); iter != end(index_impermeables); ++iter)
			fichier << *iter + 1<< ';';
		fichier << endl;

		fichier << "CLASSE INTEGRE EAU;";
		auto index_eaux = PrendreIndexEaux();
		for (auto iter = begin(index_eaux); iter != end(index_eaux); ++iter)
			fichier << *iter + 1<< ';';
		fichier << endl << endl;

		ostringstream oss;

		fichier << "UHRH ID;EPAISSEUR COUCHE 1 (m);EPAISSEUR COUCHE 2 (m);EPAISSEUR COUCHE 3 (m);HUMIDITE RELATIVE INITIALE COUCHE 1;HUMIDITE RELATIVE INITIALE COUCHE 2;HUMIDITE RELATIVE INITIALE COUCHE 3;COEFFICIENT D'EXTINCTION;COEFFICIENT DE RECESSION (m/h);COEFFICIENT MULTIPLICATIF DE L'ASSECHEMENT;VARIATION MAXIMALE DE L'HUMIDITE RELATIVE" << endl;
		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';

			fichier << zones[index].PrendreZ11() << ';';
			fichier << zones[index].PrendreZ22() << ';';
			fichier << zones[index].PrendreZ33() << ';';

			fichier << PrendreTheta1Initial(index) << ';';
			fichier << PrendreTheta2Initial(index) << ';';
			fichier << PrendreTheta3Initial(index) << ';';

			fichier << PrendreDes(index) << ';';
			
			oss.str("");
			oss << setprecision(8) << setiosflags(ios::fixed) << PrendreKrec(index);
			fichier << oss.str() << ';';

			fichier << PrendreCoeffAssechement(index) << ';';
			fichier << PrendreCin(index) << endl;
		}
	}

	string BV3C1::PrendreNomFichierLectureEtat() const
	{
		return _nom_fichier_lecture_etat;
	}

	string BV3C1::PrendreRepertoireEcritureEtat() const
	{
		return _repertoire_ecriture_etat;
	}

	bool BV3C1::PrendreSauvegardeTousEtat() const
	{
		return _sauvegarde_tous_etat;
	}

	DATE_HEURE BV3C1::PrendreDateHeureSauvegardeEtat() const
	{
		return _date_sauvegarde_etat;
	}

}
