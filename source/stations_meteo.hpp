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

#ifndef STATIONS_METEO_H_INCLUDED
#define STATIONS_METEO_H_INCLUDED


#include "stations.hpp"
#include "date_heure.hpp"

#include <hdf5.h>


namespace HYDROTEL
{

	class SIM_HYD;

	class STATIONS_METEO : public STATIONS
	{
	public:

		STATIONS_METEO();
		~STATIONS_METEO();

		void			Lecture(const PROJECTION& projection);

		void			LectureDonnees_v1(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);	//for thiessen1 & moy3station1
		void			LectureDonnees_v2(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);	//for thiessen2 & moy3station2

		PROJECTION		_ProjectionProjet;
		
		bool			_bAutoInverseTMinTMax;
		bool			_bStationInterpolation;

		SIM_HYD*		_pSimHyd;

		float			_fGradientStationTemp;		// C/100m	//gradient vertical pour interpolation des donnees manquante aux stations
		float			_fGradientStationPrecip;	// mm/100m	//gradient vertical pour interpolation des donnees manquante aux stations

		int				_netCdf_iType;	//0=STATION, 1=GRID

		std::string		_netCdf_sStationDim; 
		std::string		_netCdf_sLat;
		std::string		_netCdf_sLon;
		std::string		_netCdf_sAlt;
		std::string		_netCdf_sTime;
		std::string		_netCdf_sTMin;
		std::string		_netCdf_sTMax;
		std::string		_netCdf_sPrecip;

		size_t			_netCdf_lPasTemps;
		size_t			_netCdf_lNbPasTemps;
		
		size_t			_netCdf_lNbStations;	//for STATION type 

		size_t			_netCdf_lNbLat;			//for GRID type
		size_t			_netCdf_lNbLong;		//
		size_t			_netCdf_lNbCoord;		//(_netCdf_lNbLat*_netCdf_lNbLong)

		DATE_HEURE		_netCdf_dateDebutVecteur;

		float*			_netCdf_dataStationPrecip;	//STATION	[time*stations]	//size_t idxTime = _pStations->_pSimHyd->_date_debut.NbHeureEntre(date_heure) / pas_de_temps;
		float*			_netCdf_dataStationTMin;	//			[time*stations]	//size_t idx = idxTime * _pStations->_netCdf_lNbStations + _lIndexStation;
		float*			_netCdf_dataStationTMax;	//			[time*stations]
													//
													//GRID			[time*y*x]
													//				[time*y*x]	//size_t idx = (idxTime * _pStations->_netCdf_lNbCoord) + (idxLat * _pStations->_netCdf_lNbLong + idxLong);
													//				[time*y*x]

        //NetCDF	//OLD
		//hdf5

		void LectureFormatHDF5();

		hid_t _hdid;

        int _dataset_tmin;
        int _dataspace_tmin;
        int _rank_tmin;
        int _memspace_tmin;

        int _dataset_tmax;
        int _dataspace_tmax;
        int _rank_tmax;
        int _memspace_tmax;

        int _dataset_pr;
        int _dataspace_pr;
        int _rank_pr;
        int _memspace_pr;

	private:	

		void			LectureFormatSTM(const PROJECTION& projection);

		std::string		LectureExtentLimit();

		double			_dExtentLimitNorth;		//coordinate system must be long/lat wgs84
		double			_dExtentLimitSouth;
		double			_dExtentLimitEast;
		double			_dExtentLimitWest;

		//NetCDF

		std::string LectureFormatNetCDFConfig();

		void		LectureFormatNetCDFTypeStation();
		void		LectureFormatNetCDFTypeGrid();

	};

}

#endif

