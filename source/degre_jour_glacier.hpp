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

#ifndef DEGRE_JOUR_GLACIER_H_INCLUDED
#define DEGRE_JOUR_GLACIER_H_INCLUDED


#include "fonte_glacier.hpp"


namespace HYDROTEL
{

	class DEGRE_JOUR_GLACIER : public FONTE_GLACIER
	{
	public:

		DEGRE_JOUR_GLACIER(SIM_HYD& sim_hyd);
		virtual ~DEGRE_JOUR_GLACIER();

		virtual void		Initialise();

		virtual void		Calcule();

		virtual void		Termine();

		virtual void		LectureParametres();

		void				LectureParametresFichierGlobal();

		virtual void		SauvegardeParametres();

		virtual void		ChangeNbParams(const ZONES& zones);

		void				ChangeIndexOccupationM1(const std::vector<size_t>& index);
		std::vector<size_t> PrendreIndexOccupationM1() const;
		
		//void				CalculIndiceRadiation(DATE_HEURE date_heure, unsigned short pas_de_temps, ZONE& zone, size_t index_zone);

		//model parameters

		double								_dHauteurBande;				//m

		std::vector<double>					_taux_fonte_m1;				//[mm/jour/dC]
		std::vector<double>					_seuil_fonte_m1;			//[dC]
		std::vector<double>					_albedo_m1;					//[0-1]
		
		double								_densite_glace;				//[kg/m3]
		
		double								_c0;						//empirical constant 0
		double								_c1;						//empirical constant 1

		double								_dEpaisseurGlaceMin;		//m
		double								_dEpaisseurGlaceMax;		//m

		bool								_bFixedIceMass;				//0/1	//prevent melting of ice

		bool								_bMeltLowerBandsFirst;		//If true, the lower band with ice will melt first, before melting following band, and so on, to the last band of a uhrh.
		
		//simulation variables
		
		std::vector<size_t>					_index_occupation_m1;

		//donnees pour le bassin entier
		double								_altMinM1Bassin;
		double								_altMaxM1Bassin;

		//donnees par uhrh
		std::vector<bool>					_bUhrhGlacier;				//indique si un uhrh contient de la glace
		std::vector<bool>					_bUhrhSimule;				//indique si un uhrh est simulé

		std::vector<size_t>					_uhrhGlacierIndex;			//contient l'index des uhrh contenant de la glace
		std::vector<size_t>					_uhrhGlacierSimuleIndex;	//contient l'index des uhrh contenant de la glace qui sont simulé (qui font partie du bassin amont de l'exutoire sélectionné)

		std::vector<double>					_altMin;
		std::vector<double>					_altMax;

		std::vector<std::vector<double>>	_altPixelM1;

		//donnees par uhrh par bandes d'altitude
		std::vector<std::vector<double>>	_bandeSuperficieM1;
		std::vector<std::vector<double>>	_bandePourcentageM1;
		std::vector<std::vector<double>>	_bandeAltMoy;

		std::vector<std::vector<double>>	_vol_m1;					//volume de l'equivalent en eau de la glace [hm3]
		std::vector<std::vector<double>>	_stock_m1;					//stock (equivalent en eau) de la glace [m]

		std::vector<std::vector<double>>	_apport_m1;

		std::vector<double>					_superficieUhrhM1;			//superficie de glace
	};

}

#endif
