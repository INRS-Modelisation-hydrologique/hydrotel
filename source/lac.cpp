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

#include "lac.hpp"

#include <boost/assert.hpp>


namespace HYDROTEL
{

	LAC::LAC()
		: TRONCON(TRONCON::LAC)
		, _surface(1)
		, _profondeur(1)
	{
	}

	LAC::~LAC()
	{
	}

	float LAC::PrendreLongueur() const
	{
		return _longueur;
	}

	float LAC::PrendreSurface() const
	{
		return _surface;
	}

	float LAC::PrendreProfondeur() const
	{
		return _profondeur;
	}

	float LAC::PrendreC() const
	{
		return _c;
	}

	float LAC::PrendreK() const
	{
		return _k;
	}

	void LAC::ChangeLongueur(float longueur)
	{
		BOOST_ASSERT(longueur > 0);
		_longueur = longueur;
	}

	void LAC::ChangeSurface(float surface)
	{
		BOOST_ASSERT(surface > 0);
		_surface = surface;
	}

	void LAC::ChangeProfondeur(float profondeur)
	{
		BOOST_ASSERT(profondeur > 0);
		_profondeur = profondeur;
	}

	void LAC::ChangeC(float c)
	{
		BOOST_ASSERT(c >= 0);
		_c = c;
	}

	void LAC::ChangeK(float k)
	{
		BOOST_ASSERT(k >= 0);
		_k = k;
	}

}
