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

#include "projection.hpp"

#include "erreur.hpp"
#include "util.hpp"

#include <fstream>


using namespace std;


namespace HYDROTEL 
{

	PROJECTION::PROJECTION(const std::string& wkt)
		: _spatial_reference(wkt.c_str())
	{
	}

	PROJECTION::PROJECTION(OGRSpatialReference* sr)
		: _spatial_reference(*sr)
	{
	}

	PROJECTION::~PROJECTION()
	{
	}

	string PROJECTION::ExportWkt() const
	{
		string wkt;
		char* tmp = nullptr;
		OGRErr err = _spatial_reference.exportToPrettyWkt(&tmp);
		if (err == OGRERR_NONE)
		{
			wkt = tmp;
		}

		CPLFree(tmp);
		return wkt;
	}

	string PROJECTION::ExportProj4() const
	{
		string proj4;
		char* tmp = nullptr;
		if (_spatial_reference.exportToProj4(&tmp) == OGRERR_NONE)
		{
			proj4 = tmp;
		}

		CPLFree(tmp);
		return proj4;
	}


	PROJECTION PROJECTION::ImportFromProj4File(std::string sPath)
	{
		ifstream file;
		string sProj4;

		try{
		file.open(sPath);
		getline_mod(file, sProj4);
		file.close();
		}
		catch(...)
		{
			throw ERREUR("Erreur lors de l'initialisation du systeme de coordonnee (fichier; " + sPath + ").");
		}

		PROJECTION projection = PROJECTION::ImportFromProj4(sProj4);
		return projection;
	}


	PROJECTION PROJECTION::ImportFromProj4(const std::string& proj4)
	{
		PROJECTION projection;		
		if (projection._spatial_reference.importFromProj4(proj4.c_str()) != OGRERR_NONE)
			throw ERREUR("Erreur lors de l'initialisation du systeme de coordonnee (ImportFromProj4).");

		return projection;
	}

	PROJECTION PROJECTION::ImportFromCoordSys(const std::string& sCoordSys)
	{
		PROJECTION projection;		
		if (projection._spatial_reference.importFromMICoordSys(sCoordSys.c_str()) != OGRERR_NONE)
			throw ERREUR("Erreur lors de l'initialisation du systeme de coordonnee (fichier CoordSys.txt).");

		return projection;
	}


	PROJECTION PROJECTION::ImportFromPRJ(std::string sPathPRJ)
	{
		ifstream file;
		string sStringPRJ;

		try{
		file.open(sPathPRJ);
		getline_mod(file, sStringPRJ);
		file.close();
		}
		catch(...)
		{
			throw ERREUR("Erreur lors de l'initialisation du systeme de coordonnee (fichier CoordSys.txt).");
		}

		PROJECTION projection(sStringPRJ.c_str());
		return projection;
	}


	ostream& operator << (ostream& out, const PROJECTION& projection)
	{
		out << projection.ExportWkt();
		return out;
	}

}
