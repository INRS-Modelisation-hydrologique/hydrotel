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

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED


namespace HYDROTEL 
{

	enum ORIENTATION
	{
		ORIENTATION_AUCUNE		= 0,
		ORIENTATION_EST			= 1,
		ORIENTATION_NORD_EST	= 2,
		ORIENTATION_NORD		= 3,
		ORIENTATION_NORD_OUEST	= 4,
		ORIENTATION_OUEST		= 5,
		ORIENTATION_SUD_OUEST	= 6,
		ORIENTATION_SUD			= 7,
		ORIENTATION_SUD_EST		= 8,
	};

}

#endif
