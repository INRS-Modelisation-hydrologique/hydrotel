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

#ifndef SOUS_MODELE_H_INCLUDED
#define SOUS_MODELE_H_INCLUDED


#include "sim_hyd.hpp"


namespace HYDROTEL
{

	class SOUS_MODELE
	{
	public:
		SOUS_MODELE(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~SOUS_MODELE();

		virtual void Initialise() = 0;

		virtual void Calcule() = 0;

		virtual void Termine() = 0;

		virtual void ChangeNbParams(const ZONES& zones) = 0;

		virtual void LectureParametres() = 0;

		virtual void SauvegardeParametres() = 0;

		std::string PrendreNomSousModele() const;								//include version number sufix (ex: BV3C1)

		std::string PrendreNomSousModeleWithoutVersion() const;					//not including version number sufix (ex: BV3C)

		void ChangeNomFichierParametres(std::string nom_fichier_parametres);

		std::string PrendreNomFichierParametres() const;

		///
		//virtual int PrendreDonneesMeteoNecessaire() const = 0;

	protected:
		SIM_HYD& _sim_hyd;

	private:		
		std::string _nom_fichier_parametres;
		SOUS_MODELE& operator= (const SOUS_MODELE&); //pour eviter warning C4512 sous vc

	public:
		std::string _nom;

	};

}

#endif

