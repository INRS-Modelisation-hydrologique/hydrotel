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

#include "transforme_coordonnee.hpp"

#include "erreur.hpp"

#include <memory>


using namespace std;


namespace HYDROTEL
{

	TRANSFORME_COORDONNEE::TRANSFORME_COORDONNEE(const PROJECTION& src, const PROJECTION& dst)
		: _src(src)
		, _dst(dst)
	{
	}
	
	TRANSFORME_COORDONNEE::~TRANSFORME_COORDONNEE()
	{
	}

	COORDONNEE TRANSFORME_COORDONNEE::TransformeXY(const COORDONNEE& coordonnee)
	{
		unique_ptr<OGRCoordinateTransformation> p(OGRCreateCoordinateTransformation(&_src._spatial_reference, &_dst._spatial_reference));

		if (p == nullptr)
			throw ERREUR("Erreur COORDONNEE TRANSFORME_COORDONNEE::TransformeXY.");

		double x = coordonnee.PrendreX();
		double y = coordonnee.PrendreY();

		p->Transform(1, &x, &y);

		return COORDONNEE(x, y, coordonnee.PrendreZ());
	}

	COORDONNEE TRANSFORME_COORDONNEE::TransformeXYZ(const COORDONNEE& coordonnee)
	{
		unique_ptr<OGRCoordinateTransformation> p(OGRCreateCoordinateTransformation(&_src._spatial_reference, &_dst._spatial_reference));

		double x = coordonnee.PrendreX();
		double y = coordonnee.PrendreY();
		double z = coordonnee.PrendreZ();

		p->Transform(1, &x, &y, &z);

		return COORDONNEE(x, y, z);
	}

}
