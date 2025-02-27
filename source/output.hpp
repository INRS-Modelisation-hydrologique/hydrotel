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

#ifndef OUTPUT_H_INCLUDED
#define OUTPUT_H_INCLUDED


#include <iostream>
#include <string>
#include <vector>


namespace HYDROTEL
{

	class SIM_HYD;

	class OUTPUT
	{
	public:
		OUTPUT();
		~OUTPUT();

		void Lecture(const std::string& repertoire_simulation);

		void Sauvegarde(const std::string& repertoire_simulation);

		std::string	Separator() const;

		bool SauvegardeApport() const;

		bool SauvegardeApportGlacier() const;

		bool SauvegardeApportLateral() const;

		bool SauvegardeDebitAval() const;

		bool SauvegardeDebitAmont() const;

		bool SauvegardeEtp() const;

		bool SauvegardeNeige() const;

		bool SauvegardePluie() const;

		bool SauvegardeProductionBase() const;

		bool SauvegardeProductionHypo() const;

		bool SauvegardeProductionSurf() const;

		bool SauvegardeTheta1() const;

		bool SauvegardeTheta2() const;

		bool SauvegardeTheta3() const;

		bool SauvegardeTMin() const;

		bool SauvegardeTMax() const;

		bool SauvegardeTMinJour() const;

		bool SauvegardeTMaxJour() const;

		bool SauvegardeEtr1() const;

		bool SauvegardeEtr2() const;

		bool SauvegardeEtr3() const;

		bool SauvegardeEtrTotal() const;

		bool SauvegardeCouvertNival() const;

		bool SauvegardeHauteurNeige() const;

		bool SauvegardeAlbedoNeige() const;

		bool SauvegardeQ12() const;

		bool SauvegardeQ23() const;

		bool SauvegardeProfondeurGel() const;

		std::string SauvegardeOutputNetCDF(std::string sPathFile, bool bUHRH, 
											std::string sOutputVariableName, float* pData, std::string sUnits, std::string sDescription);


	public:
		SIM_HYD*			_sim_hyd;

		std::vector<int>	_vIdTronconSelect;

		//output variables
		std::string			_sSeparator;
		std::string			_sFichiersEtatsSeparator;
		bool				_bSauvegardeTous;
		bool				_debit_aval;
		bool				_debit_aval_moy7j_min;
		bool				_debit_amont;
		bool				_hauteur_aval;
		bool				_apport;
		bool				_apport_glacier;
		bool				_eau_glacier;
		bool				_apport_lateral;
		bool				_ecoulement_surf;	//m3/s
		bool				_ecoulement_hypo;	//m3/s
		bool				_ecoulement_base;	//m3/s
		bool				_etp;
		bool				_neige;
		bool				_pluie;
		bool				_production_base;
		bool				_production_hypo;
		bool				_production_surf;
		bool				_theta1;
		bool				_theta2;
		bool				_theta3;
		bool				_tmin;
		bool				_tmax;
		bool				_tmin_jr;
		bool				_tmax_jr;
		bool				_etr1;
		bool				_etr2;
		bool				_etr3;
		bool				_etr_total;
		bool				_couvert_nival;
		bool				_hauteur_neige;
		bool				_albedo_neige;
		bool				_q12;
		bool				_q23;
		bool				_q23SumYearly;
		bool				_qRecharge;
		bool				_profondeurgel;
		bool				_weighted_avg;
		int					_iIDTronconExutoire;

		//nb digits
		int					_nbDigit_dC;
		int					_nbDigit_mm;
		int					_nbDigit_mm_interpolation;
		int					_nbDigit_cm;
		int					_nbDigit_m;
		int					_nbDigit_ratio;
		int					_nbDigit_m3s;

		//netcdf
		int					_iOutputCDF;

		bool				_bOutputUhrhVar;
		bool				_bOutputTronconVar;

		int*				_timeVectorEpoch; 
		
		size_t				_uhrhOutputNb; 
		size_t*				_uhrhOutputIndex;
		int*				_uhrhOutputIDs;
		
		size_t				_tronconOutputNb; 
		size_t*				_tronconOutputIndex;
		int*				_tronconOutputIDs;

		//for weighted avg computations
		std::vector<int>	_wavg_IdTronconMoyPond;

		float				_wavg_TMin;					//dC
		float				_wavg_TMax;					//dC
		float				_wavg_TMoy;					//dC
		float				_wavg_Pluie;				//mm
		float				_wavg_Neige;				//mm
		float				_wavg_CouvertNival;			//mm
		float				_wavg_ETP;					//mm
		float				_wavg_ETR;					//mm

		double				_wavg_TotalApportGlace;		//mm
		double				_wavg_TotalEquiEauGlace;	//m
	};

}

#endif
