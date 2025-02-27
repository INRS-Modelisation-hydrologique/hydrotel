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

#ifndef LECTURE_TEMPSOL_H_INCLUDED
#define LECTURE_TEMPSOL_H_INCLUDED


#include "tempsol.hpp"


namespace HYDROTEL
{

	class LECTURE_TEMPSOL : public TEMPSOL
	{
	public:
		LECTURE_TEMPSOL(SIM_HYD& sim_hyd);
		virtual ~LECTURE_TEMPSOL();

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		virtual void ChangeNbParams(const ZONES& zones);

		// change le nom de fichier des apports
		void ChangeNomFichierTempsol(std::string nom_fichier);

		// retourne le nom de fichier des apports
		std::string PrendreNomFichierTempsol() const;

		virtual void LectureParametres();
		virtual void SauvegardeParametres();

	private:		
		std::ifstream		_fichier_tempsol;

		std::vector<int>	_vIDFichier;
	};

}

#endif
