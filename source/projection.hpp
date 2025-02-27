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

#ifndef PROJECTION_H_INCLUDED
#define PROJECTION_H_INCLUDED


#include <iostream>
#include <string>
#include <ogr_spatialref.h>


namespace HYDROTEL 
{

	class PROJECTION
	{
	friend class COORDONNEE;
	friend class TRANSFORME_COORDONNEE;

	public:
		PROJECTION(const std::string& wkt = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AUTHORITY[\"EPSG\",\"4326\"]]");
		PROJECTION(OGRSpatialReference* sr);

		~PROJECTION();

		std::string ExportWkt() const;

		std::string ExportProj4() const;

		static PROJECTION ImportFromProj4(const std::string& proj4);
		static PROJECTION ImportFromProj4File(std::string sPath);

		static PROJECTION ImportFromCoordSys(const std::string& sCoordSys);
		static PROJECTION ImportFromPRJ(std::string sPathPRJ);


		OGRSpatialReference _spatial_reference;
	};

	std::ostream& operator << (std::ostream& out, const PROJECTION& projection);

}

#endif
