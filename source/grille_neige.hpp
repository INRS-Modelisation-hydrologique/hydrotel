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

#ifndef GRILLE_NEIGE_H_INCLUDED
#define GRILLE_NEIGE_H_INCLUDED


#include "sim_hyd.hpp"


namespace HYDROTEL
{

	class GRILLE_NEIGE
	{
	public:

		GRILLE_NEIGE();
		~GRILLE_NEIGE();

		void Initialise();

		void LectureParametres();
		void SauvegardeParametres();

		void FormatePathFichierGrilleCourant(std::string& sPath);

		std::string									_sPathFichierParam;

		std::map<int, std::map<int, double>>		_mapPonderation;
		
		float										_facteurMultiplicatifDonnees;	//facteur multiplicatif à utiliser lors de la lecture pour convertir les donnees lues en [m]

		std::vector<RASTER<float>>					_grilleEquivalentEau;
		std::vector<RASTER<float>>					_grilleHauteurNeige;

		SIM_HYD*									_sim_hyd;

	private:

		bool LecturePonderation();
		void CalculePonderation();
		void SauvegardePonderation();

		std::string									_sPathFichierGrille;
		std::string									_sPrefixeNomFichier;
	};

}

#endif
