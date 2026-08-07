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

#ifndef NETCDF_FILE_H_INCLUDED
#define NETCDF_FILE_H_INCLUDED


#include "date_heure.hpp"

#include <netcdf.h>

#include <string>
#include <vector>


namespace HYDROTEL
{

	class NETCDF_FILE
	{
	public:

		NETCDF_FILE();
		~NETCDF_FILE();

		bool					Open(std::string sFilename, bool bListUhrhTroncon);

		bool					ReadData(std::string sVariableName, float* pData);


		std::string				_sError;

		std::string				_sFilename;

		size_t					_simTimeStep;
		DATE_HEURE				_dtDebutSim;
		DATE_HEURE				_dtFinSim;

		int						_netCdf_iType;	//0=STATION, 1=GRID

		std::string				_netCdf_TimeDimName;
		std::string				_netCdf_TimeVarName;

		std::string				_netCdf_LonDimName;
		std::string				_netCdf_LonVarName;

		std::string				_netCdf_LatDimName;
		std::string				_netCdf_LatVarName;

		std::string				_netCdf_StationDimName;
		std::string				_netCdf_StationVarName;

		//
		int						_netCdf_fileId;

		size_t					_netCdf_lPasTemps;

		size_t					_indexDebut;
		size_t					_netCdf_lNbPasTemps;

		size_t					_netCdf_lNbStation;		//for STATION and ID type 
		
		size_t					_netCdf_lNbLat;			//for GRID type
		size_t					_netCdf_lNbLong;		//
		size_t					_netCdf_lNbCoord;		//(_netCdf_lNbLat*_netCdf_lNbLong)

		DATE_HEURE				_netCdf_dateDebutVecteur;

		std::vector<double>		_netCdf_latitudes;
		std::vector<double>		_netCdf_longitudes;

		std::vector<int>		_netCdf_ids;

		//float*			    _data;					//STATION	[time*stations]	//size_t idxTime = _pStations->_pSimHyd->_date_debut.NbHeureEntre(date_heure) / pas_de_temps;
		//												//			[time*stations]	//size_t idx = idxTime * _pStations->_netCdf_lNbStations + _lIndexStation;
		//												//			[time*stations]
		// 
		//												//
		//												//GRID		[time*y*x]
		//												//			[time*y*x]	//size_t idx = (idxTime * _pStations->_netCdf_lNbCoord) + (idxLat * _pStations->_netCdf_lNbLong + idxLong);
		//												//			[time*y*x]


	private:	

		bool					OpenTypeGrid(std::string sFilename);
		bool					OpenTypeStation(std::string sFilename, bool bListUhrhTroncon);

		//std::string			LectureExtentLimit();

		//double				_dExtentLimitNorth;		//coordinate system must be long/lat wgs84
		//double				_dExtentLimitSouth;
		//double				_dExtentLimitEast;
		//double				_dExtentLimitWest;

	};

}

#endif

