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

#ifndef GRILLE_METEO_H_INCLUDED
#define GRILLE_METEO_H_INCLUDED


#include "interpolation_donnees.hpp"


namespace HYDROTEL
{

	class GRILLE_METEO : public INTERPOLATION_DONNEES
	{
	public:

		GRILLE_METEO(SIM_HYD& sim_hyd);
		virtual ~GRILLE_METEO();

		void ChangeNbParams(const ZONES& zones);

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

		virtual void Initialise();
		virtual void Calcule();
		virtual void Termine();

		virtual void LectureParametres();
		virtual void SauvegardeParametres();

		void FormatePathFichierGrilleCourant(std::string& sPath);

		//model parameters
		std::vector<float>					_gradient_precipitations;	// mm/100m
		std::vector<float>					_gradient_temperature;		// C/100m
		std::vector<float>					_passage_pluie_neige;		// C

	private:

		void LectureParametresFichierGlobal();

		bool LecturePonderation();
		void CalculePonderation();
		void SauvegardePonderation();

		bool LectureAltitude();
		void CalculeAltitude();
		void SauvegardeAltitude();

		void RepartieDonnees();
		void PassagePluieNeige();

		std::string									_sPathFichierGrille;
		std::string									_sPrefixeNomFichier;

		bool										_bUseTotalPrecip;

		std::map<int, std::map<int, double>>		_mapPonderation;
		std::map<int, double>						_mapAltitudes;
	};

}

#endif
