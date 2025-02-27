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

#include "coordonnee.hpp"

#include <ogr_geometry.h>


using namespace std;


namespace HYDROTEL 
{

	COORDONNEE::COORDONNEE(double x, double y, double z)
		: _x(x)
		, _y(y)
		, _z(z)
	{
	}


	COORDONNEE::~COORDONNEE()
	{
	}


	double COORDONNEE::PrendreX() const
	{
		return _x;
	}


	double COORDONNEE::PrendreY() const
	{
		return _y;
	}


	double COORDONNEE::PrendreZ() const
	{
		return _z;
	}

	ostream& operator << (ostream& out, const COORDONNEE& coordonnee)
	{
		out << fixed
			<< "x=" << coordonnee.PrendreX()
			<< " y=" << coordonnee.PrendreY()
			<< " z=" << coordonnee.PrendreZ();

		return out;
	}

}
