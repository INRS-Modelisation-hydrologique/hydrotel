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

#ifndef RASTER_FLOAT2_H_INCLUDED
#define RASTER_FLOAT2_H_INCLUDED


#include <string>


namespace HYDROTEL
{

    class RasterFloat2
    {
    public:
    
		RasterFloat2();

		bool Open(std::string path);

		void Close();        
	
	public:

		float*			_values;

		size_t			_xSize;
		size_t			_ySize;

		float			_noData;

		std::string		_projectionStr;

		double			_geotransform[6];

		std::string		_sError;
    };

}

#endif

