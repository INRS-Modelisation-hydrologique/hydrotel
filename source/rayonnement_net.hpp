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

#ifndef RAYONNEMENT_NET_H_INCLUDED
#define RAYONNEMENT_NET_H_INCLUDED


#include <string>
#include <vector>

#include "zones.hpp"


namespace HYDROTEL
{

	class SIM_HYD;

	class RAYONNEMENT_NET
	{	
	public:
		std::string						_nom;
		std::string						_nom_fichier_parametres;

		std::vector<std::vector<float>>	_vRa;							//rayonnement net à la surface [MJ/m2/Jour] pour chaque jour (0 a 364), pour chaque uhrh (index uhrh)

		std::vector<float>				_fAlbedo;

		std::vector<float>				_fCoeffATransmissiviteAtmos;
		std::vector<float>				_fCoeffBTransmissiviteAtmos;
		std::vector<float>				_fCoeffCTransmissiviteAtmos;

		std::vector<float>				_fCoeffAEmissiviteAtmos;
		std::vector<float>				_fCoeffBEmissiviteAtmos;
		std::vector<float>				_fCoeffCEmissiviteAtmos;

		std::vector<float>				_fCoeffAEmissiviteSurface;
		std::vector<float>				_fCoeffBEmissiviteSurface;

		SIM_HYD*						_sim_hyd;

		//
		RAYONNEMENT_NET();
		virtual ~RAYONNEMENT_NET();

		void			ChangeNbParams(const ZONES& zones);

		void			LectureParametres();
		void			LectureParametresFichierGlobal();

		void			SauvegardeParametres();


		float			PrendreRayonnementNet(int iJour, size_t index_zone);	//rayonnement net à la surface (Rn) [MJ/m2/Jour]

		//rayonnement net à la surface [MJ/m2/Jour]
		float CalculRayonnementNet(float fTMin, float fTMax, float fAlbedo, float fRa, 
								   float fCoeffATransmissiviteAtmos, float fCoeffBTransmissiviteAtmos, float fCoeffCTransmissiviteAtmos,
								   float fCoeffAEmissiviteAtmos, float fCoeffBEmissiviteAtmos, float fCoeffCEmissiviteAtmos,
								   float fCoeffAEmissiviteSurface, float fCoeffBEmissiviteSurface);

		//rayonnement de courtes longueurs d’onde extra-atmosphérique (Ra)
		void Calcul_Ra(float lon, float lat, float az, float in, float sc, bool daily, int jj, int ihr, float& out1, float& out2, float& out3);

		float __round(float v);
	};

}

#endif

