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

#ifndef TRANSFORME_COORDONNEE_H_INCLUDED
#define TRANSFORME_COORDONNEE_H_INCLUDED


#include "coordonnee.hpp"
#include "projection.hpp"


namespace HYDROTEL
{

	class TRANSFORME_COORDONNEE
	{
	public:
		TRANSFORME_COORDONNEE(const PROJECTION& src, const PROJECTION& dst);
		~TRANSFORME_COORDONNEE();

		// transforme une coordonnee x et y
		COORDONNEE TransformeXY(const COORDONNEE& coordonnee);

		// transforme une coordonnee x, y et z
		COORDONNEE TransformeXYZ(const COORDONNEE& coordonnee);

	private:
		PROJECTION _src;
		PROJECTION _dst;
	};

}

#endif
