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

#include "output.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"
#include "util.hpp"
#include "sim_hyd.hpp"

#include <fstream>
#include <regex>

#include <netcdf.h>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	OUTPUT::OUTPUT()
		: _sSeparator(";")
		, _sFichiersEtatsSeparator(";")
		, _bSauvegardeTous(false)
		, _debit_aval(true)
		, _debit_aval_moy7j_min(false)
		, _debit_amont(false)
		, _hauteur_aval(false)
        , _apport(false)
		, _apport_glacier(false)
		, _eau_glacier(false)
        , _apport_lateral(false)
		, _ecoulement_surf(false)
		, _ecoulement_hypo(false)
		, _ecoulement_base(false)
        , _etp(false)
        , _neige(false)
        , _pluie(false)
        , _production_base(false)
        , _production_hypo(false)
        , _production_surf(false)
        , _theta1(false)
        , _theta2(false)
        , _theta3(false)
        , _tmin(false)
        , _tmax(false)
        , _tmin_jr(false)
        , _tmax_jr(false)
		, _etr1(false)
		, _etr2(false)
		, _etr3(false)
		, _etr_total(false)
		, _couvert_nival(false)
		, _hauteur_neige(false)
		, _albedo_neige(false)
		, _q12(false)
		, _q23(false)
		, _q23SumYearly(false)
		, _qRecharge(false)
		, _profondeurgel(false)
		, _weighted_avg(false)
		, _iIDTronconExutoire(1)
	{
		_sim_hyd = NULL;
		_timeVectorEpoch = NULL;
		_tronconOutputIndex = NULL;
		_tronconOutputIDs = NULL;
		_uhrhOutputIndex = NULL;
		_uhrhOutputIDs = NULL;

		_iOutputCDF = -1;
		_bOutputUhrhVar = false;
		_bOutputTronconVar = false;

		_nbDigit_dC = 2;
		_nbDigit_mm = 2;
		_nbDigit_cm = 3;
		_nbDigit_m = 5;
		_nbDigit_ratio = 4;
		_nbDigit_m3s = 5;

		_nbDigit_mm_interpolation = _nbDigit_mm;
	}


	OUTPUT::~OUTPUT()
	{
		if(_timeVectorEpoch != NULL)
			delete [] _timeVectorEpoch;

		if (_tronconOutputIndex != NULL)
		{
			delete [] _tronconOutputIndex;
			delete [] _tronconOutputIDs;
		}

		if (_uhrhOutputIndex != NULL)
		{
			delete [] _uhrhOutputIndex;
			delete [] _uhrhOutputIDs;
		}
	}


	void OUTPUT::Lecture(const string& repertoire_simulation)
	{
		if(_sim_hyd->PrendreNomInterpolationDonnees() == "THIESSEN1" || _sim_hyd->PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS1")
		{
			_nbDigit_dC = 9;
			_nbDigit_mm = 6;
			_nbDigit_ratio = 6;
			_nbDigit_m3s = 6;

			_nbDigit_mm_interpolation = 9;
		}

		string nom_fichier = Combine(repertoire_simulation, "output.csv");
		string sTemp, cle;
		size_t x;

		if (!FichierExiste(nom_fichier))
		{
			Sauvegarde(repertoire_simulation);	//crée un fichier par defaut
			_bOutputUhrhVar = false;
			_bOutputTronconVar = true;	//downstream discharge is saved by default
			return;
		}

		ifstream fichier(nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER OUTPUT; " + nom_fichier);

		_bSauvegardeTous = false;
		_weighted_avg = false;
		_vIdTronconSelect.clear();
		_wavg_IdTronconMoyPond.clear();

		_bOutputUhrhVar = false;
		_bOutputTronconVar = false;

		try
		{
			int no_ligne = 0;

			while (!fichier.eof())
			{
				string valeur, ligne;
				int iTemp;
		
				lire_cle_valeur(fichier, cle, valeur);
				++no_ligne;

				if (cle == "TRONCONS")
				{
					auto vValeur = extrait_stringValeur(valeur);

					for (x=0; x<vValeur.size(); x++)
					{
						sTemp = vValeur[x];
						boost::algorithm::to_lower(sTemp);

						if(sTemp == "tous")
							_bSauvegardeTous = true;
						else
						{
							istringstream iss(sTemp);
							iss >> iTemp;	
							_vIdTronconSelect.push_back(iTemp);
						}
					}
				}
				else if (cle == "TRONCONS_MOYENNES_PONDEREES" || cle == "TRONCONS MOYENNES PONDEREES")
				{
					auto vValeur = extrait_stringValeur(valeur);

					for (x = 0; x < vValeur.size(); x++)
					{
						sTemp = vValeur[x];
						istringstream iss(sTemp);
						iss >> iTemp;
						_wavg_IdTronconMoyPond.push_back(iTemp);
					}

					if(_wavg_IdTronconMoyPond.size() != 0)
						_weighted_avg = true;
				}
				else if (cle == "DEBITS AVAL")
					_debit_aval = (valeur == "1") ? true : false;
				else if (cle == "DEBITS_AVAL")
					_debit_aval = (valeur == "1") ? true : false;
				else if (cle == "DEBITS AMONT")
					_debit_amont = (valeur == "1") ? true : false;
				else if (cle == "DEBITS_AMONT")
					_debit_amont = (valeur == "1") ? true : false;
				else if (cle == "DEBITS AVAL MOY7J MIN")
					_debit_aval_moy7j_min = (valeur == "1") ? true : false;
				else if (cle == "DEBITS_AVAL_MOY7J_MIN")
					_debit_aval_moy7j_min = (valeur == "1") ? true : false;
				else if (cle == "HAUTEUR AVAL")
					_hauteur_aval = (valeur == "1") ? true : false;
				else if (cle == "HAUTEUR_AVAL")
					_hauteur_aval = (valeur == "1") ? true : false;
				else if (cle == "APPORT")
					_apport = (valeur == "1") ? true : false;
				else if (cle == "COUVERT NIVAL")
					_couvert_nival =  (valeur == "1") ? true : false;
				else if (cle == "COUVERT_NIVAL")
					_couvert_nival =  (valeur == "1") ? true : false;
				else if (cle == "HAUTEUR NEIGE")
					_hauteur_neige =  (valeur == "1") ? true : false;
				else if (cle == "HAUTEUR_NEIGE")
					_hauteur_neige =  (valeur == "1") ? true : false;
				else if (cle == "ALBEDO NEIGE")
					_albedo_neige =  (valeur == "1") ? true : false;
				else if (cle == "ALBEDO_NEIGE")
					_albedo_neige =  (valeur == "1") ? true : false;
				else if (cle == "APPORT GLACIER")
					_apport_glacier = (valeur == "1") ? true : false;
				else if (cle == "APPORT_GLACIER")
					_apport_glacier = (valeur == "1") ? true : false;
				else if (cle == "EAU GLACIER")
					_eau_glacier = (valeur == "1") ? true : false;
				else if (cle == "EAU_GLACIER")
					_eau_glacier = (valeur == "1") ? true : false;
				else if (cle == "APPORT LATERAL")
					_apport_lateral = (valeur == "1") ? true : false;
				else if (cle == "APPORT_LATERAL")
					_apport_lateral = (valeur == "1") ? true : false;
				else if (cle == "ECOULEMENT_SURF" || cle == "ECOULEMENT SURF")
					_ecoulement_surf = (valeur == "1") ? true : false;
				else if (cle == "ECOULEMENT_HYPO" || cle == "ECOULEMENT HYPO")
					_ecoulement_hypo = (valeur == "1") ? true : false;
				else if (cle == "ECOULEMENT_BASE" || cle == "ECOULEMENT BASE")
					_ecoulement_base = (valeur == "1") ? true : false;
				else if (cle == "ETP")
					_etp = (valeur == "1") ? true : false;
				else if (cle == "NEIGE")
					_neige =  (valeur == "1") ? true : false;
				else if (cle == "PLUIE")
					_pluie = (valeur == "1") ? true : false;
				else if (cle == "PRODUCTION BASE")
					_production_base =  (valeur == "1") ? true : false;
				else if (cle == "PRODUCTION_BASE")
					_production_base =  (valeur == "1") ? true : false;
				else if (cle == "PRODUCTION HYPO")
					_production_hypo =  (valeur == "1") ? true : false;
				else if (cle == "PRODUCTION_HYPO")
					_production_hypo =  (valeur == "1") ? true : false;
				else if (cle == "PRODUCTION SURF")
					_production_surf =  (valeur == "1") ? true : false;
				else if (cle == "PRODUCTION_SURF")
					_production_surf =  (valeur == "1") ? true : false;
				else if (cle == "Q12")
					_q12 = (valeur == "1") ? true : false;
				else if (cle == "Q23")
					_q23 = (valeur == "1") ? true : false;
				else if (cle == "Q23_SOMME_ANNUELLE")
					_q23SumYearly = (valeur == "1") ? true : false;
				else if (cle == "Q23 SOMME ANNUELLE")
					_q23SumYearly = (valeur == "1") ? true : false;
				else if (cle == "QRECHARGE")
					_qRecharge = (valeur == "1") ? true : false;
				else if (cle == "THETA1")
					_theta1 = (valeur == "1") ? true : false;
				else if (cle == "THETA2")
					_theta2 = (valeur == "1") ? true : false;
				else if (cle == "THETA3")
					_theta3 =  (valeur == "1") ? true : false;
				else if (cle == "TMIN")
					_tmin = (valeur == "1") ? true : false;
				else if (cle == "TMAX")
					_tmax = (valeur == "1") ? true : false;
				else if (cle == "TMIN JOUR")
					_tmin_jr = (valeur == "1") ? true : false;
				else if (cle == "TMIN_JOUR")
					_tmin_jr = (valeur == "1") ? true : false;
				else if (cle == "TMAX JOUR")
					_tmax_jr = (valeur == "1") ? true : false;
				else if (cle == "TMAX_JOUR")
					_tmax_jr = (valeur == "1") ? true : false;
				else if (cle == "ETR1")
					_etr1 = (valeur == "1") ? true : false;
				else if (cle == "ETR2")
					_etr2 =  (valeur == "1") ? true : false;
				else if (cle == "ETR3")
					_etr3 = (valeur == "1") ? true : false;
				else if (cle == "ETR TOTAL")
					_etr_total = (valeur == "1") ? true : false;
				else if (cle == "ETR_TOTAL")
					_etr_total = (valeur == "1") ? true : false;
				else if (cle == "SEPARATEUR")
					_sSeparator = valeur;
				else if (cle == "PROFONDEUR GEL")
					_profondeurgel =  (valeur == "1") ? true : false;
				else if (cle == "PROFONDEUR_GEL")
					_profondeurgel =  (valeur == "1") ? true : false;
				else if (cle == "FICHIERS ETATS SEPARATEUR")
					_sFichiersEtatsSeparator = valeur;
				else if (cle == "FICHIERS_ETATS_SEPARATEUR")
					_sFichiersEtatsSeparator = valeur;
				else if (cle == "OUTPUT NETCDF")
					_iOutputCDF = (valeur == "1") ? 1 : 0;
				else if (cle == "OUTPUT_NETCDF")
					_iOutputCDF = (valeur == "1") ? 1 : 0;
			}
		}
		catch (...)
		{
			if (!fichier.eof())
			{
				fichier.close();
				throw ERREUR("Error reading `output.csv` file: invalid key: " + cle);
			}
		}
		
		fichier.close();
		fichier.clear();

		if(_sSeparator.length() > 1)
			throw ERREUR_LECTURE_FICHIER("FICHIER OUTPUT: " + nom_fichier + ": invalid SEPARATEUR value");

		if(_sFichiersEtatsSeparator.length() > 1)
			throw ERREUR_LECTURE_FICHIER("FICHIER OUTPUT: " + nom_fichier + ": invalid FICHIERS_ETATS_SEPARATEUR value");

		if(_debit_aval || _debit_amont || _apport_lateral)
			_bOutputTronconVar = true;

		if(_apport || _etp || _neige || _pluie || _production_base || _production_hypo || _production_surf || _q12 || _q23 || _theta1 || _theta2
			|| _theta3 || _tmin || _tmax || _tmin_jr || _tmax_jr || _etr1 || _etr2 || _etr3 || _etr_total || _couvert_nival || _hauteur_neige || _albedo_neige || _profondeurgel)
		{
			_bOutputUhrhVar = true;
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//création d'un fichier avec valeurs par defaut
	void OUTPUT::Sauvegarde(const string& repertoire_simulation)
	{
		string nom_fichier = Combine(repertoire_simulation, "output.csv");

		ofstream fichier(nom_fichier);
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier.exceptions(ios::failbit | ios::badbit);

		_bSauvegardeTous = false;
		_vIdTronconSelect.clear();
		_wavg_IdTronconMoyPond.clear();

		_vIdTronconSelect.push_back(_iIDTronconExutoire);

		fichier << "TRONCONS;" << _iIDTronconExutoire << endl;

		fichier << "TRONCONS_MOYENNES_PONDEREES;" << endl; 
		
		fichier << "TMIN;"				<< (_tmin ? 1 : 0) << endl;
		fichier << "TMAX;"				<< (_tmax ? 1 : 0) << endl;
		fichier << "TMIN_JOUR;"			<< (_tmin_jr ? 1 : 0) << endl;
		fichier << "TMAX_JOUR;"			<< (_tmax_jr ? 1 : 0) << endl;
		fichier << "PLUIE;"				<< (_pluie ? 1 : 0) << endl;
		fichier << "NEIGE;"				<< (_neige ? 1 : 0) << endl;
		fichier << "APPORT;"			<< (_apport ? 1 : 0) << endl;
		fichier << "COUVERT_NIVAL;"		<< (_couvert_nival ? 1 : 0) << endl;
		fichier << "HAUTEUR_NEIGE;"		<< (_hauteur_neige ? 1 : 0) << endl;
		fichier << "ALBEDO_NEIGE;"		<< (_albedo_neige ? 1 : 0) << endl;
		fichier << "APPORT_GLACIER;"	<< (_apport_glacier ? 1 : 0) << endl;
		fichier << "EAU_GLACIER;"		<< (_eau_glacier ? 1 : 0) << endl;
		fichier << "PROFONDEUR_GEL;"	<< (_profondeurgel ? 1 : 0) << endl;
		fichier << "ETP;"				<< (_etp ? 1 : 0) << endl;
		fichier << "ETR1;"				<< (_etr1 ? 1 : 0) << endl;
		fichier << "ETR2;"				<< (_etr2 ? 1 : 0) << endl;
		fichier << "ETR3;"				<< (_etr3 ? 1 : 0) << endl;
		fichier << "ETR_TOTAL;"			<< (_etr_total ? 1 : 0) << endl;
		fichier << "PRODUCTION_BASE;"	<< (_production_base ? 1 : 0) << endl;
		fichier << "PRODUCTION_HYPO;"	<< (_production_hypo ? 1 : 0) << endl;
		fichier << "PRODUCTION_SURF;"	<< (_production_surf ? 1 : 0) << endl;
		fichier << "Q12;"				<< (_q12 ? 1 : 0) << endl;
		fichier << "Q23;"				<< (_q23 ? 1 : 0) << endl;
		fichier << "Q23_SOMME_ANNUELLE;"	<< (_q23SumYearly ? 1 : 0) << endl;
		fichier << "QRECHARGE;"			<< (_qRecharge ? 1 : 0) << endl;
		fichier << "THETA1;"			<< (_theta1 ? 1 : 0) << endl;
		fichier << "THETA2;"			<< (_theta2 ? 1 : 0) << endl;
		fichier << "THETA3;"			<< (_theta3 ? 1 : 0) << endl;
		fichier << "APPORT_LATERAL;"	<< (_apport_lateral ? 1 : 0) << endl;
		fichier << "ECOULEMENT_SURF;"	<< (_ecoulement_surf ? 1 : 0) << endl;
		fichier << "ECOULEMENT_HYPO;"	<< (_ecoulement_hypo ? 1 : 0) << endl;
		fichier << "ECOULEMENT_BASE;"	<< (_ecoulement_base ? 1 : 0) << endl;
		fichier << "DEBITS_AMONT;"		<< (_debit_amont ? 1 : 0) << endl;
		fichier << "DEBITS_AVAL;"		<< (_debit_aval ? 1 : 0) << endl;
		fichier << "HAUTEUR_AVAL;"		<< (_hauteur_aval ? 1 : 0) << endl;

		fichier << "DEBITS_AVAL_MOY7J_MIN;"	<< (_debit_aval_moy7j_min ? 1 : 0) << endl;
		
		fichier << "FICHIERS_ETATS_SEPARATEUR;"		<< _sFichiersEtatsSeparator << endl;
		fichier << "SEPARATEUR;"		<< _sSeparator << endl;

		fichier << "OUTPUT_NETCDF;"		<< (_iOutputCDF == 1 ? 1 : 0) << endl;

		fichier.close();
	}

	string OUTPUT::Separator() const
	{
		return _sSeparator;
	}

	bool OUTPUT::SauvegardeApport() const
	{
		return _apport;
	}

	bool OUTPUT::SauvegardeApportGlacier() const
	{
		return _apport_glacier;
	}

	bool OUTPUT::SauvegardeApportLateral() const
	{
		return _apport_lateral;
	}

	bool OUTPUT::SauvegardeDebitAval() const
	{
		return _debit_aval;
	}

	bool OUTPUT::SauvegardeDebitAmont() const
	{
		return _debit_amont;
	}

	bool OUTPUT::SauvegardeEtp() const
	{
		return _etp;
	}

	bool OUTPUT::SauvegardeNeige() const
	{
		return _neige;
	}

	bool OUTPUT::SauvegardePluie() const
	{
		return _pluie;
	}

	bool OUTPUT::SauvegardeProductionBase() const
	{
		return _production_base;
	}

	bool OUTPUT::SauvegardeProductionHypo() const
	{
		return _production_hypo;
	}

	bool OUTPUT::SauvegardeProductionSurf() const
	{
		return _production_surf;
	}

	bool OUTPUT::SauvegardeTheta1() const
	{
		return _theta1;
	}

	bool OUTPUT::SauvegardeTheta2() const
	{
		return _theta2;
	}

	bool OUTPUT::SauvegardeTheta3() const
	{
		return _theta3;
	}

	bool OUTPUT::SauvegardeTMin() const
	{
		return _tmin;
	}

	bool OUTPUT::SauvegardeTMax() const
	{
		return _tmax;
	}

	bool OUTPUT::SauvegardeTMinJour() const
	{
		return _tmin_jr;
	}

	bool OUTPUT::SauvegardeTMaxJour() const
	{
		return _tmax_jr;
	}

	bool OUTPUT::SauvegardeEtr1() const
	{
		return _etr1;
	}

	bool OUTPUT::SauvegardeEtr2() const
	{
		return _etr2;
	}

	bool OUTPUT::SauvegardeEtr3() const
	{
		return _etr3;
	}

	bool OUTPUT::SauvegardeEtrTotal() const
	{
		return _etr_total;
	}

	bool OUTPUT::SauvegardeCouvertNival() const
	{
		return _couvert_nival;
	}

	bool OUTPUT::SauvegardeHauteurNeige() const
	{
		return _hauteur_neige;
	}

	bool OUTPUT::SauvegardeAlbedoNeige() const
	{
		return _albedo_neige;
	}

	bool OUTPUT::SauvegardeQ12() const
	{
		return _q12;
	}

	bool OUTPUT::SauvegardeQ23() const
	{
		return _q23;
	}

	bool OUTPUT::SauvegardeProfondeurGel() const
	{
		return _profondeurgel;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string OUTPUT::SauvegardeOutputNetCDF(string sPathFile, bool bUHRH, 
											string sOutputVariableName, float* pData, string sUnits, string sDescription)
	{
		ostringstream oss;
		string sDimName, str;
		time_t currentTime;
		size_t chunksize[2];
		size_t lDimSize;
		int dimidsElement[1];
		int dimidsTime[1];
		int dimidsVar[2];
		int ret, iNcid, iDimID, iDimTimeID, iVarID, iElemID, iTimeID;

		if(FichierExiste(sPathFile))
			SupprimerFichier(sPathFile);

		if(bUHRH)
		{
			sDimName = "uhrh";
			lDimSize = _uhrhOutputNb;
		}
		else
		{
			sDimName = "troncon";
			lDimSize = _tronconOutputNb;
		}

		//chunksize optimisé pour série temporelle par UHRH/troncon
		chunksize[0] = _sim_hyd->_lNbPasTempsSim;	//time dimension
		chunksize[1] = 1;

		////chunksize optimisé pour obtenir tous les UHRH/troncon pour 1 pas de temps
		//chunksize[0] = 1;	//time dimension
		//chunksize[1] = lDimSize;	//troncon/uhrh dimension

		if(_timeVectorEpoch == NULL)
		{
			int step, currentdate;
			size_t x;

			step = _sim_hyd->_pas_de_temps * 60;	//minutes
			currentdate = static_cast<int>(_sim_hyd->_date_debut.EpochTime() / 60.0);	//minutes since 1970/01/01
			
			_timeVectorEpoch = new int[_sim_hyd->_lNbPasTempsSim];
			for(x=0; x<_sim_hyd->_lNbPasTempsSim; x++)
			{
				_timeVectorEpoch[x] = currentdate;
				currentdate+= step;
			}
		}

		//ret = nc_create(sPathFile.c_str(), NC_NETCDF4|NC_64BIT_OFFSET, &iNcid);
		//ret = nc_create(sPathFile.c_str(), NC_NETCDF4|NC_64BIT_DATA, &iNcid);		
		//ret = nc_create(sPathFile.c_str(), NC_64BIT_OFFSET, &iNcid);	//File type : NetCDF - 3 / CDM
		ret = nc_create(sPathFile.c_str(), NC_NETCDF4, &iNcid);	//
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 1";

		//global attributes
		str = "Variable de sortie simulation Hydrotel";
		ret = nc_put_att_text(iNcid, NC_GLOBAL, "description", str.size(), str.c_str());
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 2";

		str = _sim_hyd->PrendreRepertoireSimulation();
		ret = nc_put_att_text(iNcid, NC_GLOBAL, "initial_simulation_path", str.size(), str.c_str());
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 3";

		currentTime = time(0);   // get time now

#if defined(_WIN64) || defined(_WIN32)

		struct tm now;
		localtime_s(&now, &currentTime);

		oss << setw(2) << setfill('0') << now.tm_mday << '-' 
			<< setw(2) << setfill('0') << (now.tm_mon + 1) << '-' 
			<< (now.tm_year + 1900) << ' ' 
			<< setw(2) << setfill('0') << now.tm_hour << ":"
			<< setw(2) << setfill('0') << now.tm_min << ":"
			<< setw(2) << setfill('0') << now.tm_sec;

#else

		tm* now = localtime(&currentTime);

		oss << setw(2) << setfill('0') << now->tm_mday << '-' 
			<< setw(2) << setfill('0') << (now->tm_mon + 1) << '-' 
			<< (now->tm_year + 1900) << ' ' 
			<< setw(2) << setfill('0') << now->tm_hour << ":" 
			<< setw(2) << setfill('0') << now->tm_min << ":" 
			<< setw(2) << setfill('0') << now->tm_sec;

#endif

		str = oss.str();
		ret = nc_put_att_text(iNcid, NC_GLOBAL, "creation_time", str.size(), str.c_str());
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 4";

		//dimensions
		ret = nc_def_dim(iNcid, sDimName.c_str(), lDimSize, &iDimID);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 5";

		ret = nc_def_dim(iNcid, "time", _sim_hyd->_lNbPasTempsSim, &iDimTimeID);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 6";

		//variables

		//idtroncon/iduhrh
		dimidsElement[0] = iDimID;
		str = "id" + sDimName;

		ret = nc_def_var(iNcid, str.c_str(), NC_INT, 1, dimidsElement, &iElemID);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 7";

		//time
		dimidsTime[0] = iDimTimeID;

		ret = nc_def_var(iNcid, "time", NC_INT, 1, dimidsTime, &iTimeID);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 8"; 
		
		str = "minutes since 1970-01-01 00:00:00";
		ret = nc_put_att_text(iNcid, iTimeID, "units", str.size(), str.c_str());
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 9";

		//output var
		dimidsVar[0] = iDimTimeID;
		dimidsVar[1] = iDimID;
		
		ret = nc_def_var(iNcid, sOutputVariableName.c_str(), NC_FLOAT, 2, dimidsVar, &iVarID);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 10";

		ret = nc_def_var_chunking(iNcid, iVarID, NC_CHUNKED, chunksize);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 11";

		ret = nc_def_var_deflate(iNcid, iVarID, 0, 1, 1);	//set deflate level 1
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 12";

		ret = nc_put_att_text(iNcid, iVarID, "units", sUnits.size(), sUnits.c_str());
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 13";

		ret = nc_put_att_text(iNcid, iVarID, "description", sDescription.size(), sDescription.c_str());
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 14";

		ret = nc_enddef(iNcid);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 15";

		//save data
		if (bUHRH)
			ret = nc_put_var_int(iNcid, iElemID, _uhrhOutputIDs);
		else
			ret = nc_put_var_int(iNcid, iElemID, _tronconOutputIDs);

		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 16";

		ret = nc_put_var_int(iNcid, iTimeID, _timeVectorEpoch);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 17";

		ret = nc_put_var_float(iNcid, iVarID, pData);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 18";

		ret = nc_close(iNcid);
		if (ret != NC_NOERR)
			return "error creating file: " + sPathFile + ": error 19";

		return "";
	}

}
