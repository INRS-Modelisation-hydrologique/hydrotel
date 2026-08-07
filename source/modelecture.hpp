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

#ifndef MODELECTURE_H_INCLUDED
#define MODELECTURE_H_INCLUDED


#include "date_heure.hpp"
#include "matrice.hpp"
#include "station_meteo_netcdf_station.hpp"

#include <string>
#include <vector>


namespace HYDROTEL
{

	class SIM_HYD;

	class ModeLecture
	{
	public:
		ModeLecture(SIM_HYD& sim_hyd);
		~ModeLecture();

		bool LectureParametres(std::string sFile);

		void Initialise();

		void InterpolationDonnees();		//interpolation et attribution des données aux variables interne pour le pas de temps courant (type GRID et STATION)

		void AttributionDonneesUhrh();		//attribution des données aux variables interne pour le pas de temps courant (type RHHU)
		void AttributionDonneesTroncon();	//attribution des données aux variables interne pour le pas de temps courant (type REACH)

		//
		std::string				_sError;

		//

		std::string				_sPathProjet;

		size_t					_simTimeStep;
		DATE_HEURE				_dtDebutSim;
		DATE_HEURE				_dtFinSim;

		//

		int						_iVarMode;	//1=PROD1,PROD2,PROD3, 2=PRODTOT, 3=QLAT1,QLAT2,QLAT3, 4=QLATTOT, 5=VCONT
		int						_iDimMode;	//1=GRID, 2=STATION, 3=RHHU, 4=REACH

		//std::vector<double*>	_pVal;
		std::vector<float*>		_pVal;

		size_t					_idxProd1;
		size_t					_idxProd2;
		size_t					_idxProd3;
		size_t					_idxProdTot;

		size_t					_idxQLat1;
		size_t					_idxQLat2;
		size_t					_idxQLat3;
		size_t					_idxQLatTot;

		size_t					_idxVCont;

		float					_fDISTRIBUTION_COEFF1;
		float					_fDISTRIBUTION_COEFF2;

	private:

		bool					LecturePonderation(std::string sPathFile);
		void					CalculePonderation();
		void					SauvegardePonderation(std::string sPathFile);

		//

		SIM_HYD*				_pSimHyd;

		std::string				_sParamFilePath;


		//ponderation pour type GRID
		std::vector<size_t>						_pondIndexUhrh;
		std::vector<std::vector<size_t>>		_pondIndexStations;
		MATRICE<double>							_ponderation;
		double*									_ponderation2;

		std::vector<std::shared_ptr<STATION_METEO_NETCDF_STATION>>	_gridPoint;

		DATE_HEURE								_netCdf_dateDebutVecteur;

		std::vector<double>						_netCdf_longitudes;
		std::vector<double>						_netCdf_latitudes;
		
		size_t									_netCdf_lNbCoord;
		size_t									_netCdf_lNbLong;

		size_t									_netCdf_lNbStation;

		size_t									_netCdf_lNbPasTemps;

		std::vector<int>						_netCdf_ids;
		
		std::vector<size_t>						_netCdf_IndexSimToIndexList;	//correspondance entre le vecteur des zones simulées et le vecteur des identifiants en input
																				//les éléments du vecteur sont l'index du vecteur des idientifiant (input) correspondant à l'index du vecteur des zones simulées

	};

}

#endif
