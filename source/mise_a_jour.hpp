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

#ifndef MISE_A_JOUR_H_INCLUDED
#define MISE_A_JOUR_H_INCLUDED


#include "sim_hyd.hpp"

#include <string>


namespace HYDROTEL
{

	void Info(const std::string& fichier_prj);

	void CropRaster(const std::string& sPathRasterSrc, const std::string& sPathRasterMask);
	void CropRasterFloat(const std::string& sPathRasterFloatSrc, const std::string& sPathRasterMask);
	void CropRasterInt_FloatMask(const std::string& sPathRasterIntSrc, const std::string& sPathRasterFloatMask);

	// mise a jour d'un projet 2.6 v47 ou v49 a 2.8
	void MiseAJour(const std::string& fichier_prj, const std::string& repertoire);

	// mise a jour d'une matrice au format bin vers un GeoTIFF d'entier
	void PhysitelRaster2GeoTIFF_int(const std::string& src, const std::string& dst, int nodata);

	// mise a jour d'une matrice au format bin vers un GeoTIFF de reel
	void PhysitelRaster2GeoTIFF_float(const std::string& src, const std::string& dst);

	// conversion du fichier points et troncons vers un GeoTIFF avec un masque
	void PhysitelPoint2GeoTIFF(const std::string& fichier_points, const std::string& fichier_troncons, const std::string& dst, const std::string& masque);

}

#endif
