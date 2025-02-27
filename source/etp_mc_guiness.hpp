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

#ifndef ETP_MC_GUINESS_H_INCLUDED
#define ETP_MC_GUINESS_H_INCLUDED


#include "evapotranspiration.hpp"
//#include "rayonnement_net.hpp"


namespace HYDROTEL
{
	class ETP_MC_GUINESS : public EVAPOTRANSPIRATION
	{
	public:
		ETP_MC_GUINESS(SIM_HYD& sim_hyd);
		virtual ~ETP_MC_GUINESS();

		virtual void	ChangeNbParams(const ZONES& zones);

		virtual void	Initialise();
		virtual void	Calcule();
		virtual void	Termine();

		double			RayonnementExtraterrestre(int iJulianDay, double dLatitudeDD);

		virtual void	LectureParametres();
		void			LectureParametresFichierGlobal();

		virtual void	SauvegardeParametres();

		std::vector<size_t>					_index_zones;	//index zones simulés
		size_t								_nbZonesSimule;
		size_t								_nbClasseOccsol;

		std::vector<double>					_poidPasTemps;

		double								_lambda;
		double								_rho;

		std::vector<std::vector<double>>	_Re;

		//RAYONNEMENT_NET*					_rayonnementNet;

		//std::vector<float>				_fHauteurMesureVitesseVent;		//m
		//std::vector<float>				_fHauteurMesureHumidite;		//m

		//std::vector<float>				_fVitesseVent;					//m/s
		//std::vector<float>				_fHauteurVegetation;			//m
		//std::vector<float>				_fResistanceStomatale;			//s/m
	};

}

#endif
