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

#ifndef LAC_H_INCLUDED
#define LAC_H_INCLUDED


#include "troncon.hpp"


namespace HYDROTEL
{

	class LAC : public TRONCON
	{
	public:
		LAC();
		virtual ~LAC();

		/// retourne la surface du lac (km2)
		float PrendreSurface() const;

		//m
		float PrendreLongueur() const;

		//m
		float PrendreProfondeur() const;

		/// retourne le coefficient c
		float PrendreC() const;

		/// retourne le coefficient k
		float PrendreK() const;

		/// change la longueur (m)
		void ChangeLongueur(float longueur);

		/// change la surface du lac (km2)
		void ChangeSurface(float surface);

		//m
		void ChangeProfondeur(float profondeur);

		/// change le coefficient c
		void ChangeC(float c);

		/// change le coefficient k
		void ChangeK(float k);

	private:
		float _longueur;	// m
		float _surface;		// km2
		float _profondeur;	// m
		float _c;
		float _k;
	};

}

#endif
