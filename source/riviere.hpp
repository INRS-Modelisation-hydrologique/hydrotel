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

#ifndef RIVIERE_H_INCLUDED
#define RIVIERE_H_INCLUDED


#include "troncon.hpp"


namespace HYDROTEL
{

	class RIVIERE : public TRONCON
	{
	public:
		RIVIERE();
		virtual ~RIVIERE();

		// retourne la largeur (m)
		float PrendreLargeur() const;

		// retourne la longueur (m)
		float PrendreLongueur() const;

		// retourne la profondeur (m)
		float PrendreProfondeur() const;

		float PrendrePente() const;

		// retourne le coefficient de manning
		float PrendreManning() const;

		// change la largeur (m)
		void ChangeLargeur(float largeur);

		// change la longueur (m)
		void ChangeLongueur(float longueur);

		// change la profondeur (m)
		void ChangeProfondeur(float profondeur);

		void ChangePente(float pente);

		void ChangeManning(float manning);

	private:
		float _largeur;		// m
		float _longueur;	// m
		float _profondeur;	// m
		float _pente;
		float _manning;
	};

}

#endif

