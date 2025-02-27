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

#ifndef GRILLE_PREVISION_H_INCLUDED
#define GRILLE_PREVISION_H_INCLUDED


#include <string>
#include <vector>
#include <map>

#include "date_heure.hpp"


namespace HYDROTEL
{

	class SIM_HYD;

	class GRILLE_PREVISION
	{
	public:
		GRILLE_PREVISION();
		~GRILLE_PREVISION();

		void Initialise();
		void Calcule();

		void LectureParametres();
		void SauvegardeParametres();

		void FormatePathFichierGrilleCourant(std::string& sPath);

		std::string									_sPathFichierParam;
		
		DATE_HEURE									_date_debut_prevision;

		std::map<int, std::map<int, double>>		_mapPonderation;
		std::map<int, double>						_mapAltitudes;

		SIM_HYD*									_sim_hyd;

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

		std::vector<float>							_gradient_precipitations;	// mm/100m
		std::vector<float>							_gradient_temperature;		// C/100m
		std::vector<float>							_passage_pluie_neige;		// C
	};

}

#endif
