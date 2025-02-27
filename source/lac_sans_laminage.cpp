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

#include "lac_sans_laminage.hpp"

#include <boost/assert.hpp>


namespace HYDROTEL
{

	LAC_SANS_LAMINAGE::LAC_SANS_LAMINAGE()
		: TRONCON(TRONCON::LAC_SANS_LAMINAGE)
	{
	}

	LAC_SANS_LAMINAGE::~LAC_SANS_LAMINAGE()
	{
	}

	float LAC_SANS_LAMINAGE::PrendreProfondeur() const
	{
		return _profondeur;
	}

	void LAC_SANS_LAMINAGE::ChangeProfondeur(float profondeur)
	{
		BOOST_ASSERT(profondeur > 0.0f);
		_profondeur = profondeur;
	}

}
