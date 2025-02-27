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

#ifndef CORRECTIONS_H_INCLUDED
#define CORRECTIONS_H_INCLUDED


#include "correction.hpp"

#include <string>
#include <vector>


namespace HYDROTEL
{

	class SIM_HYD;	// forward class declaration

	class CORRECTIONS
	{
	public:
		CORRECTIONS();
		~CORRECTIONS();

		void ChangeNomFichier(const std::string& nom_fichier);

		void LectureFichier(SIM_HYD& sim_hyd);

		std::string PrendreNomFichier() const;

		std::vector<CORRECTION*> PrendreCorrectionsPluie();

		std::vector<CORRECTION*> PrendreCorrectionsNeige();

		std::vector<CORRECTION*> PrendreCorrectionsTemperature();

		std::vector<CORRECTION*> PrendreCorrectionsReserveSol();

		std::vector<CORRECTION*> PrendreCorrectionsNeigeAuSol();

		std::vector<CORRECTION*> PrendreCorrectionsSaturationReserveSol();

	public:
		bool	_bActiver;	//indique si les paramètres du fichier de correction sont activé

	protected:
		std::string _nom_fichier;
		std::vector<CORRECTION> _corrections;
	};

}

#endif
