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

#include "raster.hpp"

#include <string>

#include <gdal_priv.h>
#include <ogrsf_frmts.h>


namespace HYDROTEL 
{

	void WriteGeoTIFF(RASTER<int>& raster, const std::string& nom_fichier, int nodata);

	void WriteGeoTIFF(RASTER<float>& raster, const std::string& nom_fichier);

	RASTER<float> ReadGeoTIFF_float(const std::string& nom_fichier);

	RASTER<int> ReadGeoTIFF_int(const std::string& nom_fichier);

	void Polygonize(const std::string& src, const std::string& dst, const std::string& mask);

	void Polygonize(const std::string& src, const std::string& dst);

	void ReseauGeoTIFF2Shapefile(const std::string& nom_fichier_orientation, const std::string& nom_fichier_reseau,
		const std::string& nom_fichier_riviere, const std::string& nom_fichier_lac);

}

