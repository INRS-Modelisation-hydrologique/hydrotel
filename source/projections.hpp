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

#ifndef PROJECTIONS_H_INCLUDED
#define PROJECTIONS_H_INCLUDED


#include "projection.hpp"


namespace HYDROTEL
{

	class PROJECTIONS
	{
	public:

		// Geographic Categories

		//static PROJECTION LONGLAT_WGS84() { return PROJECTION::ImportFromProj4("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"); }
		static PROJECTION LONGLAT_WGS84() { return PROJECTION::ImportFromProj4("+proj=longlat +datum=WGS84 +no_defs"); }

		// Projected Categories

		static PROJECTION UTM16N_WGS84() { return PROJECTION::ImportFromProj4("+proj=utm +zone=16 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"); }
		static PROJECTION UTM17N_WGS84() { return PROJECTION::ImportFromProj4("+proj=utm +zone=17 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"); }
		static PROJECTION UTM18N_WGS84() { return PROJECTION::ImportFromProj4("+proj=utm +zone=18 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"); }
		static PROJECTION UTM19N_WGS84() { return PROJECTION::ImportFromProj4("+proj=utm +zone=19 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"); }
		static PROJECTION UTM20N_WGS84() { return PROJECTION::ImportFromProj4("+proj=utm +zone=20 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"); }
		static PROJECTION UTM21N_WGS84() { return PROJECTION::ImportFromProj4("+proj=utm +zone=21 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"); }

		static PROJECTION LAMBERT() { return PROJECTION::ImportFromProj4("+proj=lcc +lat_1=50 +lat_2=70 +lat_0=40 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs "); }

		static PROJECTION MERCATOR16() { return PROJECTION::ImportFromProj4("+proj=tmerc +lat_0=0 +lon_0=16 +k=1.000000 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs "); }
		static PROJECTION MERCATOR17() { return PROJECTION::ImportFromProj4("+proj=tmerc +lat_0=0 +lon_0=17 +k=1.000000 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs "); }
		static PROJECTION MERCATOR18() { return PROJECTION::ImportFromProj4("+proj=tmerc +lat_0=0 +lon_0=18 +k=1.000000 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs "); }
		static PROJECTION MERCATOR19() { return PROJECTION::ImportFromProj4("+proj=tmerc +lat_0=0 +lon_0=19 +k=1.000000 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs "); }
		static PROJECTION MERCATOR20() { return PROJECTION::ImportFromProj4("+proj=tmerc +lat_0=0 +lon_0=20 +k=1.000000 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs "); }
		static PROJECTION MERCATOR21() { return PROJECTION::ImportFromProj4("+proj=tmerc +lat_0=0 +lon_0=21 +k=1.000000 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs "); }

	private:
		PROJECTIONS();
	};

}

#endif
