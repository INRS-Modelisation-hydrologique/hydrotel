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

#ifndef THIESSEN1_H_INCLUDED
#define THIESSEN1_H_INCLUDED


#include "interpolation_donnees.hpp"
#include "stations_neige.hpp"


namespace HYDROTEL
{
	
	class THIESSEN1 : public INTERPOLATION_DONNEES
	{
	public:
		THIESSEN1(SIM_HYD& sim_hyd);
		virtual ~THIESSEN1();

		void ChangeNbParams(const ZONES& zones);

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		virtual void LectureParametres();

		void LectureParametresFichierGlobal();

		virtual void SauvegardeParametres();

		bool LecturePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& ponderation);
		//bool LecturePonderation(STATIONS_NEIGE& stations, ZONES& zones, MATRICE<float>& ponderation);

		void CalculePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& ponderation, std::string sOrigin);

		void SauvegardePonderation(STATIONS& stations, ZONES& zones, MATRICE<float>& ponderation);

		/// retourne le gradient de precipitation (mm/100m)
		float PrendreGradientPrecipitation(size_t index) const;

		/// retourne le gradient de temperature (C/100m)
		float PrendreGradientTemperature(size_t index) const;

		/// retourne la temperature de passage de la pluie en neige (C)
		float PrendrePassagePluieNeige(size_t index) const;

		/// change le gradient de precipitation (mm/100m)
		void ChangeGradientPrecipitation(size_t index, float gradient_precipitation);

		/// change le gradient de temperature (C/100m)
		void ChangeGradientTemperature(size_t index, float gradient_temperature);

		/// change la temperature de passage de la pluie en neige (C)
		void ChangePassagePluieNeige(size_t index, float passage_pluie_neige);

		bool LecturePonderation();
		void CalculePonderation();
		void SauvegardePonderation();


		//model parameters
		std::vector<float>					_gradient_precipitations;	// mm/100m
		std::vector<float>					_gradient_temperature;		// C/100m
		std::vector<float>					_passage_pluie_neige;		// C

		//internal variable
		SIM_HYD*							_pSim_hyd;

		std::vector<size_t>					_pondIndexUhrh;
		std::vector<std::vector<size_t>>	_pondIndexStations;

		float*								_ponderation2;

	private:

		void RepartieDonnees();
		void PassagePluieNeige();

		MATRICE<float> _ponderation;
		//MATRICE<double> _ponderation;
	};

}

#endif
