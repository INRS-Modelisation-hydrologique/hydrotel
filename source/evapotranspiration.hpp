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

#ifndef EVAPOTRANSPIRATION_H_INCLUDED
#define EVAPOTRANSPIRATION_H_INCLUDED


#include "sous_modele.hpp"

#include <fstream>


namespace HYDROTEL
{

	class EVAPOTRANSPIRATION : public SOUS_MODELE
	{
	public:
		EVAPOTRANSPIRATION(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~EVAPOTRANSPIRATION() = 0;

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		float PrendreCoefficientMultiplicatif(size_t index_zone) const;

		virtual void ChangeNbParams(const ZONES& zones);

		void ChangeCoefficientMultiplicatif(size_t index_zone, float coefficient);

		std::string			_nom_fichier_evp;

		//NetCDF	
		//results are kept in memory while running and are saved at the end of the simulation
		float*				_netCdf_etp;	//[time*stations]
		//

	protected:
		float Repartition(unsigned short pas_de_temps, unsigned short heure);

		std::vector<float> _coefficients_multiplicatif;

	private:
		std::ofstream _fichier_etp;
	};

}

#endif
