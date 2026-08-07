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

#include "modelecture.hpp"

#include "util.hpp"
#include "netcdf_file.hpp"
#include "erreur.hpp"
#include "station_meteo_netcdf_station.hpp"
#include "sim_hyd.hpp"
#include "projections.hpp"
#include "transforme_coordonnee.hpp"

#include <fstream>
#include <algorithm>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	ModeLecture::ModeLecture(SIM_HYD& sim_hyd)
	{
		_pSimHyd = &sim_hyd;

		_idxProd1 = (size_t)-1;
		_idxProd2 = (size_t)-1;
		_idxProd3 = (size_t)-1;
		_idxProdTot = (size_t)-1;

		_idxQLat1 = (size_t)-1;
		_idxQLat2 = (size_t)-1;
		_idxQLat3 = (size_t)-1;
		_idxQLatTot = (size_t)-1;

		_idxVCont = (size_t)-1;

		_iVarMode = -1;
		_iDimMode = -1;

		_fDISTRIBUTION_COEFF1 = -1.0f;
		_fDISTRIBUTION_COEFF2 = -1.0f;

		_ponderation2 = nullptr;
	}


	ModeLecture::~ModeLecture()
	{
		if(_ponderation2)
			delete [] _ponderation2;

		for(int x=0; x!=_pVal.size(); x++)
			delete [] _pVal[x];
	}


	bool ModeLecture::LectureParametres(string sFile)
	{
		map<string, string> mapParam;
		vector<string> vVar;
		vector<string> vSource;
		istringstream iss;
		size_t i, j, k, m;
		string str, str2, sVarName, sDimType, sDataFilename;
		float* pValTemp;
		bool bListUhrhTroncon, bNeedDistributionCoeff;
		int x;

		NETCDF_FILE* netcdfFile = nullptr;

		_sError = "";
		bNeedDistributionCoeff = false;

		if(!boost::filesystem::exists(sFile))
		{
			_sError = "error: file not found: " + sFile;
			return false;
		}

		str = ReadParameterFile(sFile, mapParam);
		if(str != "")
		{
			_sError = str;
			return false;
		}

		if(mapParam["DIMTYPE"] == "GRID")
			_iDimMode = 1;
		else
		{
			if(mapParam["DIMTYPE"] == "STATION")
				_iDimMode = 2;
			else
			{
				if(mapParam["DIMTYPE"] == "RHHU")
					_iDimMode = 3;
				else
				{
					if(mapParam["DIMTYPE"] == "REACH")
						_iDimMode = 4;
					else
					{
						_sError = "error reading parameters: " + sFile + ": variable DIMTYPE value must be equal `GRID`, `STATION`, `RHHU` or `REACH`";
						return false;
					}
				}
			}
		}

		if(mapParam.find("PROD1_SOURCE") != mapParam.end())
		{
			//PROD1, PROD2, PROD3
			if(mapParam.find("PRODTOT_SOURCE") != mapParam.end())
			{
				_sError = "error reading parameters: " + sFile + ": variable PRODTOT_SOURCE cannot be used if PROD1_SOURCE, PROD2_SOURCE and PROD3_SOURCE are specified";
				return false;
			}
					
			if(mapParam.find("PROD2_SOURCE") == mapParam.end() || mapParam.find("PROD3_SOURCE") == mapParam.end())
			{
				_sError = "error reading parameters: " + sFile + ": variables PROD2_SOURCE and PROD3_SOURCE must be specified when using PROD1_SOURCE";
				return false;
			}

			if(mapParam.find("VCONT_SOURCE") != mapParam.end() || mapParam.find("QLAT1_SOURCE") != mapParam.end() || mapParam.find("QLAT2_SOURCE") != mapParam.end() || mapParam.find("QLAT3_SOURCE") != mapParam.end() || mapParam.find("QLATTOT_SOURCE") != mapParam.end())
			{
				_sError = "error reading parameters: " + sFile + ": variables VCONT_SOURCE, QLAT1_SOURCE, QLAT2_SOURCE, QLAT3_SOURCE and QLATTOT_SOURCE cannot be used when PROD1_SOURCE, PROD2_SOURCE and PROD3_SOURCE are specified";
				return false;
			}

			//if(mapParam["PROD1_DIMTYPE"] != "GRID" && mapParam["PROD1_DIMTYPE"] != "STATION" && mapParam["PROD1_DIMTYPE"] != "RHHU")
			//{
			//	_sError = "error reading parameters: " + sFile + ": variable PROD1_DIMTYPE value must be equal `GRID`, `STATION` or `RHHU`";
			//	return false;
			//}
			if(_iDimMode == 4)
			{
				_sError = "error reading parameters: " + sFile + ": variable DIMTYPE value must be equal `GRID`, `STATION` or `RHHU` when using PROD1, PROD2 and PROD3 variables";
				return false;
			}

			//if(mapParam["PROD1_DIMTYPE"] != mapParam["PROD2_DIMTYPE"] || mapParam["PROD1_DIMTYPE"] != mapParam["PROD3_DIMTYPE"])
			//{
			//	_sError = "error reading parameters: " + sFile + ": variables PROD1_DIMTYPE, PROD2_DIMTYPE and PROD3_DIMTYPE must all have the same value (`GRID`, `STATION` or `RHHU`)";
			//	return false;
			//}

			if(mapParam.find("PROD1_VARNAME") == mapParam.end() || mapParam["PROD1_VARNAME"] == "" || 
				mapParam.find("PROD2_VARNAME") == mapParam.end() || mapParam["PROD2_VARNAME"] == "" || 
				mapParam.find("PROD3_VARNAME") == mapParam.end() || mapParam["PROD3_VARNAME"] == "")
			{
				_sError = "error reading parameters: " + sFile + ": variables PROD1_VARNAME, PROD2_VARNAME and PROD3_VARNAME must be specified";
				return false;
			}

			//if(mapParam["Q1_DIMTYPE"] == "GRID")
			//	_iDimMode = 1;
			//else
			//{
			//	if(mapParam["Q1_DIMTYPE"] == "STATION")
			//		_iDimMode = 2;
			//	else
			//		_iDimMode = 3;	//RHHU
			//}

			vVar.push_back("PROD1");
			vVar.push_back("PROD2");
			vVar.push_back("PROD3");

			_iVarMode = 1;
		}
		else
		{
			if(mapParam.find("PRODTOT_SOURCE") != mapParam.end())
			{
				//PRODTOT
				if(mapParam.find("PROD2_SOURCE") != mapParam.end() || mapParam.find("PROD3_SOURCE") != mapParam.end())
				{
					_sError = "error reading parameters: " + sFile + ": variables PROD1_SOURCE, PROD2_SOURCE and PROD3_SOURCE cannot be used when PRODTOT_SOURCE is specified";
					return false;
				}

				if(mapParam.find("VCONT_SOURCE") != mapParam.end() || mapParam.find("QLAT1_SOURCE") != mapParam.end() || mapParam.find("QLAT2_SOURCE") != mapParam.end() || mapParam.find("QLAT3_SOURCE") != mapParam.end() || mapParam.find("QLATTOT_SOURCE") != mapParam.end())
				{
					_sError = "error reading parameters: " + sFile + ": variables VCONT_SOURCE, QLAT1_SOURCE, QLAT2_SOURCE, QLAT3_SOURCE and QLATTOT_SOURCE cannot be used when PRODTOT_SOURCE is specified";
					return false;
				}

				//if(mapParam["PRODTOT_DIMTYPE"] != "GRID" && mapParam["PRODTOT_DIMTYPE"] != "STATION" && mapParam["PRODTOT_DIMTYPE"] != "RHHU")
				//{
				//	_sError = "error reading parameters: " + sFile + ": variable PRODTOT_DIMTYPE value must be equal `GRID`, `STATION` or `RHHU`";
				//	return false;
				//}
				if(_iDimMode == 4)
				{
					_sError = "error reading parameters: " + sFile + ": variable DIMTYPE value must be equal `GRID`, `STATION` or `RHHU` when using PRODTOT variable";
					return false;
				}

				if(mapParam.find("PRODTOT_VARNAME") == mapParam.end() || mapParam["PRODTOT_VARNAME"] == "")
				{
					_sError = "error reading parameters: " + sFile + ": variable PRODTOT_VARNAME must be specified";
					return false;
				}

				//if(mapParam["QTOT_DIMTYPE"] == "GRID")
				//	_iDimMode = 1;
				//else
				//{
				//	if(mapParam["QTOT_DIMTYPE"] == "STATION")
				//		_iDimMode = 2;
				//	else
				//		_iDimMode = 3;	//RHHU
				//}

				bNeedDistributionCoeff = true;

				vVar.push_back("PRODTOT");

				_iVarMode = 2;
			}
			else
			{
				if(mapParam.find("QLAT1_SOURCE") != mapParam.end())
				{
					//QLAT1, QLAT2, QLAT3
					if(mapParam.find("QLATTOT_SOURCE") != mapParam.end())
					{
						_sError = "error reading parameters: " + sFile + ": variable QLATTOT_SOURCE cannot be used if QLAT1_SOURCE, QLAT2_SOURCE and QLAT3_SOURCE are specified";
						return false;
					}
					
					if(mapParam.find("QLAT2_SOURCE") == mapParam.end() || mapParam.find("QLAT3_SOURCE") == mapParam.end())
					{
						_sError = "error reading parameters: " + sFile + ": variables QLAT2_SOURCE and QLAT3_SOURCE must be specified when using QLAT1_SOURCE";
						return false;
					}

					if(mapParam.find("VCONT_SOURCE") != mapParam.end() || mapParam.find("PROD2_SOURCE") != mapParam.end() || mapParam.find("PROD3_SOURCE") != mapParam.end())
					{
						_sError = "error reading parameters: " + sFile + ": variables VCONT_SOURCE, PROD1_SOURCE, PROD2_SOURCE, PROD3_SOURCE and PRODTOT_SOURCE cannot be used when QLAT1_SOURCE, QLAT2_SOURCE and QLAT3_SOURCE are specified";
						return false;
					}

					//if(mapParam["C1_DIMTYPE"] != "RHHU" && mapParam["C1_DIMTYPE"] != "REACH")
					//{
					//	_sError = "error reading parameters: " + sFile + ": variable C1_DIMTYPE value must be equal `RHHU` or `REACH`";
					//	return false;
					//}
					if(_iDimMode != 3 && _iDimMode != 4)
					{
						_sError = "error reading parameters: " + sFile + ": variable DIMTYPE value must be equal `RHHU` or `REACH` when using QLAT1, QLAT2 and QLAT3 variables";
						return false;
					}

					//if(mapParam["QLAT1_DIMTYPE"] != mapParam["QLAT2_DIMTYPE"] || mapParam["QLAT1_DIMTYPE"] != mapParam["QLAT3_DIMTYPE"])
					//{
					//	_sError = "error reading parameters: " + sFile + ": variables QLAT1_DIMTYPE, QLAT2_DIMTYPE and QLAT3_DIMTYPE must all have the same value (`RHHU` or `REACH`)";
					//	return false;
					//}

					if(mapParam.find("QLAT1_VARNAME") == mapParam.end() || mapParam["QLAT1_VARNAME"] == "" || 
						mapParam.find("QLAT2_VARNAME") == mapParam.end() || mapParam["QLAT2_VARNAME"] == "" || 
						mapParam.find("QLAT3_VARNAME") == mapParam.end() || mapParam["QLAT3_VARNAME"] == "")
					{
						_sError = "error reading parameters: " + sFile + ": variables QLAT1_VARNAME, QLAT2_VARNAME and QLAT3_VARNAME must be specified";
						return false;
					}

					//if(mapParam["C1_DIMTYPE"] == "RHHU")
					//	_iDimMode = 3;
					//else
					//	_iDimMode = 4;	//REACH

					vVar.push_back("QLAT1");
					vVar.push_back("QLAT2");
					vVar.push_back("QLAT3");

					_iVarMode = 3;
				}
				else
				{
					if(mapParam.find("QLATTOT_SOURCE") != mapParam.end())
					{
						//QLATTOT
						if(mapParam.find("QLAT2_SOURCE") != mapParam.end() || mapParam.find("QLAT3_SOURCE") != mapParam.end())
						{
							_sError = "error reading parameters: " + sFile + ": variables QLAT1_SOURCE, QLAT2_SOURCE and QLAT3_SOURCE cannot be used when QLATTOT_SOURCE is specified";
							return false;
						}

						if(mapParam.find("VCONT_SOURCE") != mapParam.end() || mapParam.find("PROD2_SOURCE") != mapParam.end() || mapParam.find("PROD3_SOURCE") != mapParam.end())
						{
							_sError = "error reading parameters: " + sFile + ": variables VCONT_SOURCE, PROD1_SOURCE, PROD2_SOURCE, PROD3_SOURCE and PRODTOT_SOURCE cannot be used when QLATTOT_SOURCE is specified";
							return false;
						}

						//if(mapParam["QLATTOT_DIMTYPE"] != "RHHU" && mapParam["QLATTOT_DIMTYPE"] != "REACH")
						//{
						//	_sError = "error reading parameters: " + sFile + ": variable QLATTOT_DIMTYPE value must be equal `RHHU` or `REACH`";
						//	return false;
						//}
						if(_iDimMode != 3 && _iDimMode != 4)
						{
							_sError = "error reading parameters: " + sFile + ": variable DIMTYPE value must be equal `RHHU` or `REACH` when using QLATTOT variable";
							return false;
						}

						if(mapParam.find("QLATTOT_VARNAME") == mapParam.end() || mapParam["QLATTOT_VARNAME"] == "")
						{
							_sError = "error reading parameters: " + sFile + ": variable QLATTOT_VARNAME must be specified";
							return false;
						}

						//if(mapParam["CTOT_DIMTYPE"] == "RHHU")
						//	_iDimMode = 3;
						//else
						//	_iDimMode = 4;	//REACH

						bNeedDistributionCoeff = true;

						vVar.push_back("QLATTOT");

						_iVarMode = 4;
					}
					else
					{
						if(mapParam.find("VCONT_SOURCE") != mapParam.end())
						{
							//VCONT
							if(mapParam.find("PROD2_SOURCE") != mapParam.end() || mapParam.find("PROD3_SOURCE") != mapParam.end())
							{
								_sError = "error reading parameters: " + sFile + ": variables PROD1_SOURCE, PROD2_SOURCE, PROD3_SOURCE and PRODTOT_SOURCE cannot be used when VCONT_SOURCE is specified";
								return false;
							}

							if(mapParam.find("QLAT2_SOURCE") != mapParam.end() || mapParam.find("QLAT3_SOURCE") != mapParam.end())
							{
								_sError = "error reading parameters: " + sFile + ": variables QLAT1_SOURCE, QLAT2_SOURCE, QLAT3_SOURCE and QLATTOT_SOURCE cannot be used when VCONT_SOURCE is specified";
								return false;
							}

							//if(mapParam["VCONT_DIMTYPE"] != "GRID" && mapParam["VCONT_DIMTYPE"] != "STATION" && mapParam["VCONT_DIMTYPE"] != "RHHU")
							//{
							//	_sError = "error reading parameters: " + sFile + ": variable VCONT_DIMTYPE value must be equal `GRID`, `STATION` or `RHHU`";
							//	return false;
							//}
							if(_iDimMode == 4)
							{
								_sError = "error reading parameters: " + sFile + ": variable DIMTYPE value must be equal `GRID`, `STATION` or `RHHU` when using VCONT variable";
								return false;
							}

							if(mapParam.find("VCONT_VARNAME") == mapParam.end() || mapParam["VCONT_VARNAME"] == "")
							{
								_sError = "error reading parameters: " + sFile + ": variable VCONT_VARNAME must be specified";
								return false;
							}

							//if(mapParam["VCONT_DIMTYPE"] == "GRID")
							//	_iDimMode = 1;
							//else
							//{
							//  if(mapParam["VCONT_DIMTYPE"] == "STATION")
							//	  _iDimMode = 2;
							//  else
							//    _iDimMode = 3;	//RHHU
							//}

							vVar.push_back("VCONT");

							_iVarMode = 5;
						}
						else
						{
							_sError = "error reading parameters: " + sFile + ": no variable is specified";
							return false;
						}
					}
				}
			}
		}

		if(mapParam.find("TIME_DIMNAME") == mapParam.end() || mapParam["TIME_DIMNAME"] == "" || 
			mapParam.find("TIME_VARNAME") == mapParam.end() || mapParam["TIME_VARNAME"] == "")
		{
			_sError = "error reading parameters: " + sFile + ": variables TIME_DIMNAME and TIME_VARNAME must be specified";
			return false;
		}

		if(_iDimMode == 1) //GRID
		{
			if(mapParam.find("LON_DIMNAME") == mapParam.end() || mapParam["LON_DIMNAME"] == "" || 
				mapParam.find("LAT_DIMNAME") == mapParam.end() || mapParam["LAT_DIMNAME"] == "")
			{
				_sError = "error reading parameters: " + sFile + ": variables LON_DIMNAME and LAT_DIMNAME must be specified";
				return false;
			}

			if(mapParam.find("LON_VARNAME") == mapParam.end() || mapParam["LON_VARNAME"] == "" || 
				mapParam.find("LAT_VARNAME") == mapParam.end() || mapParam["LAT_VARNAME"] == "")
			{
				_sError = "error reading parameters: " + sFile + ": variables LON_VARNAME and LAT_VARNAME must be specified";
				return false;
			}
		}
		else
		{
			if(_iDimMode == 2) //STATION
			{
				if(mapParam.find("STATION_DIMNAME") == mapParam.end() || mapParam["STATION_DIMNAME"] == "")
				{
					_sError = "error reading parameters: " + sFile + ": variable STATION_DIMNAME must be specified";
					return false;
				}

				if(mapParam.find("LON_VARNAME") == mapParam.end() || mapParam["LON_VARNAME"] == "" || 
					mapParam.find("LAT_VARNAME") == mapParam.end() || mapParam["LAT_VARNAME"] == "")
				{
					_sError = "error reading parameters: " + sFile + ": variables LON_VARNAME and LAT_VARNAME must be specified";
					return false;
				}
			}
			else
			{
				//3 & 4 (RHHU & REACH)
				if(mapParam.find("ID_DIMNAME") == mapParam.end() || mapParam["ID_DIMNAME"] == "" || 
					mapParam.find("ID_VARNAME") == mapParam.end() || mapParam["ID_VARNAME"] == "")
				{
					_sError = "error reading parameters: " + sFile + ": variables ID_DIMNAME and ID_VARNAME must be specified";
					return false;
				}
			}
		}

		if(bNeedDistributionCoeff)
		{
			if(mapParam.find("DISTRIBUTION_COEFF1") == mapParam.end() || mapParam["DISTRIBUTION_COEFF1"] == "" || 
				mapParam.find("DISTRIBUTION_COEFF2") == mapParam.end() || mapParam["DISTRIBUTION_COEFF2"] == "")
			{
				_sError = "error reading parameters: " + sFile + ": variables DISTRIBUTION_COEFF1 and DISTRIBUTION_COEFF2 must be specified when using PRODTOT or QLATTOT variable";
				return false;
			}

			iss.clear();
			iss.str(mapParam["DISTRIBUTION_COEFF1"]);
			iss >> _fDISTRIBUTION_COEFF1;
			if(_fDISTRIBUTION_COEFF1 < 0.0f || _fDISTRIBUTION_COEFF1 > 1.0f)
			{
				_sError = "error reading parameters: " + sFile + ": variable DISTRIBUTION_COEFF1 must have a value greater than or equal to 0 and less than or equal to 1";
				return false;
			}

			iss.clear();
			iss.str(mapParam["DISTRIBUTION_COEFF2"]);
			iss >> _fDISTRIBUTION_COEFF2;
			if(_fDISTRIBUTION_COEFF2 < 0.0f || _fDISTRIBUTION_COEFF2 > 1.0f)
			{
				_sError = "error reading parameters: " + sFile + ": variable DISTRIBUTION_COEFF2 must have a value greater than or equal to 0 and less than or equal to 1";
				return false;
			}

			if(_fDISTRIBUTION_COEFF1 + _fDISTRIBUTION_COEFF2 > 1.0f)
			{
				_sError = "error reading parameters: " + sFile + ": the sum of variables DISTRIBUTION_COEFF1 and DISTRIBUTION_COEFF2 must have a value less than or equal to 1";
				return false;
			}
		}

		//obtient le nb de sources differentes
		for(i=0; i!=vVar.size(); i++)
		{
			str = vVar[i] + "_SOURCE";
			if(mapParam[str] == "")
			{
				_sError = "error reading parameters: " + sFile + ": variable " + str + " must have a value.";
				return false;
			}

			if(std::find(vSource.begin(), vSource.end(), mapParam[str]) == vSource.end())
				vSource.push_back(mapParam[str]);
		}

		bListUhrhTroncon = false;
		if(_iDimMode == 3 || _iDimMode == 4)
			bListUhrhTroncon = true;

		//pour chaque source
		for(i=0; i!=vSource.size(); i++)
		{
			netcdfFile = new NETCDF_FILE();

			netcdfFile->_simTimeStep = _simTimeStep;
			netcdfFile->_dtDebutSim = _dtDebutSim;
			netcdfFile->_dtFinSim = _dtFinSim;

			netcdfFile->_netCdf_TimeDimName = mapParam["TIME_DIMNAME"];
			netcdfFile->_netCdf_TimeVarName = mapParam["TIME_VARNAME"];

			if(_iDimMode == 1) //GRID
			{
				netcdfFile->_netCdf_iType = 1;	//_netCdf_iType: 0=STATION, 1=GRID

				netcdfFile->_netCdf_LonDimName = mapParam["LON_DIMNAME"];
				netcdfFile->_netCdf_LatDimName = mapParam["LAT_DIMNAME"];

				netcdfFile->_netCdf_LonVarName = mapParam["LON_VARNAME"];
				netcdfFile->_netCdf_LatVarName = mapParam["LAT_VARNAME"];
			}
			else
			{
				netcdfFile->_netCdf_iType = 0;	//type 0(STATION) pour _iDimMode 2, 3 et 4 (STATION, RHHU & REACH)

				if(_iDimMode == 2) //STATION
				{
					netcdfFile->_netCdf_StationDimName = mapParam["STATION_DIMNAME"];

					netcdfFile->_netCdf_LonVarName = mapParam["LON_VARNAME"];
					netcdfFile->_netCdf_LatVarName = mapParam["LAT_VARNAME"];
				}
				else
				{
					//_iDimMode == 3 || _iDimMode == 4	//RHHU, REACH
					netcdfFile->_netCdf_StationDimName = mapParam["ID_DIMNAME"];
					netcdfFile->_netCdf_StationVarName = mapParam["ID_VARNAME"];
				}
			}
					
			sDataFilename = vSource[i];
			if(!Racine(sDataFilename))
				sDataFilename = Combine(_sPathProjet, sDataFilename);

			if(!netcdfFile->Open(sDataFilename, bListUhrhTroncon))
			{
				_sError = netcdfFile->_sError;
				delete netcdfFile;
				return false;
			}

			if(i == 0)
			{
				_netCdf_dateDebutVecteur = netcdfFile->_netCdf_dateDebutVecteur;
				_netCdf_lNbPasTemps = netcdfFile->_netCdf_lNbPasTemps;

				if(_iDimMode == 1 || _iDimMode == 2)	//GRID ou STATION
				{
					_netCdf_longitudes = netcdfFile->_netCdf_longitudes;
					_netCdf_latitudes = netcdfFile->_netCdf_latitudes;
				}

				if(_iDimMode == 1)	//GRID
				{
					_netCdf_lNbCoord = _netCdf_longitudes.size() * _netCdf_latitudes.size();
					_netCdf_lNbLong = _netCdf_longitudes.size();
				}
				else
				{
					_netCdf_lNbStation = netcdfFile->_netCdf_lNbStation;

					if(_iDimMode != 2)
						_netCdf_ids = netcdfFile->_netCdf_ids; //type RHHU et REACH
				}
			}

			for(j=0; j!=vVar.size(); j++)
			{
				str = vVar[j] + "_SOURCE";
				if(mapParam[str] == vSource[i]) //if current source is the source for the current variable
				{
					if(_iDimMode == 1)	//GRID
						pValTemp = new float[_netCdf_lNbPasTemps*_netCdf_lNbCoord];
					else
						pValTemp = new float[_netCdf_lNbPasTemps*_netCdf_lNbStation];

					str = vVar[j] + "_VARNAME";
					if(!netcdfFile->ReadData(mapParam[str], pValTemp))
					{
						_sError = netcdfFile->_sError;
						delete netcdfFile;
						return false;
					}

					_pVal.push_back(pValTemp);

					if(vVar[j] == "PROD1")
						_idxProd1 = _pVal.size() - 1;
					else
					{
						if(vVar[j] == "PROD2")
							_idxProd2 = _pVal.size() - 1;
						else
						{
							if(vVar[j] == "PROD3")
								_idxProd3 = _pVal.size() - 1;
							else
							{
								if(vVar[j] == "PRODTOT")
									_idxProdTot = _pVal.size() - 1;
								else
								{
									if(vVar[j] == "QLAT1")
										_idxQLat1 = _pVal.size() - 1;
									else
									{
										if(vVar[j] == "QLAT2")
											_idxQLat2 = _pVal.size() - 1;
										else
										{
											if(vVar[j] == "QLAT3")
												_idxQLat3 = _pVal.size() - 1;
											else
											{
												if(vVar[j] == "QLATTOT")
													_idxQLatTot = _pVal.size() - 1;
												else
													_idxVCont = _pVal.size() - 1;	//VCONT
											}
										}
									}
								}
							}
						}
					}
				}
			}

			if(i == 0)
			{
				ostringstream oss;

				_gridPoint.clear();

				if(_iDimMode == 1)	//GRID
				{
					//création et initialisation des objets station pour interpolation des données (pour type grille)
					m = 0;
					for(j=0; j!=netcdfFile->_netCdf_lNbLat; j++)
					{
						for(k=0; k!=netcdfFile->_netCdf_lNbLong; k++)
						{
							////pour cadrant nord/west
							//if (_dExtentLimitNorth == -1.0 || 
							//	(latitudes[j] <= _dExtentLimitNorth && latitudes[j] >= _dExtentLimitSouth &&
							//		longitudes[k] <= _dExtentLimitEast && longitudes[k] >= _dExtentLimitWest) )
							//{
								shared_ptr<STATION_METEO_NETCDF_STATION> st = make_shared<STATION_METEO_NETCDF_STATION>("", nullptr, j, k);

								oss.str("");
								oss << "gridpoint" << m + 1;

								st->ChangeNom(oss.str());
								st->ChangeIdent(oss.str());
								st->ChangeCoordonnee(COORDONNEE(_netCdf_longitudes[k], _netCdf_latitudes[j], 0.0));

								_gridPoint.push_back(st);
								++m;
							//}
						}
					}
				}
				else
				{
					if(_iDimMode == 2)	//STATION
					{
						for(j=0; j!=_netCdf_lNbStation; j++)
						{
							////pour cadrant nord/west
							//if (_dExtentLimitNorth == -1.0 || 
							//	(latitudes[j] <= _dExtentLimitNorth && latitudes[j] >= _dExtentLimitSouth &&
							//	 longitudes[j] <= _dExtentLimitEast && longitudes[j] >= _dExtentLimitWest) )
							//{
								shared_ptr<STATION_METEO_NETCDF_STATION> st = make_shared<STATION_METEO_NETCDF_STATION>("", nullptr, j);

								oss.str("");
								oss << "gridpoint" << j + 1;

								st->ChangeNom(oss.str());
								st->ChangeIdent(oss.str());
								st->ChangeCoordonnee(COORDONNEE(_netCdf_longitudes[j], _netCdf_latitudes[j], 0.0));

								_gridPoint.push_back(st);
							//}
						}
					}
					else
					{
						vector<size_t>::iterator it;
						vector<size_t> vIndexSim;

						if(_iDimMode == 3)	//RHHU
						{
							for(j=0; j!=_netCdf_ids.size(); j++)
							{
								x = abs(_netCdf_ids[j]);
								if(std::find(_pSimHyd->PrendreZones()._listIdUhrh.begin(), _pSimHyd->PrendreZones()._listIdUhrh.end(), x) == _pSimHyd->PrendreZones()._listIdUhrh.end())
								{
									//id uhrh non existant dans le projet
									oss.str("");
									oss << _netCdf_ids[j];
									_sError = "Error reading NetCDF file: " + sDataFilename + ": The RHHU ID `" + oss.str() + "` does not exist in the project";
									delete netcdfFile;
									return false;
								}

								m = _pSimHyd->PrendreZones().IdentVersIndex(_netCdf_ids[j]);
								vIndexSim.push_back(m);
							}

							vector<size_t> idxZoneSim = _pSimHyd->PrendreZonesSimules();
							for(j=0; j!=idxZoneSim.size(); j++)
							{
								it = std::find(vIndexSim.begin(), vIndexSim.end(), idxZoneSim[j]);
								if(it == vIndexSim.end())
								{
									//id troncon non existant dans le netcdf source
									oss.str("");
									oss << _pSimHyd->PrendreZones()[idxZoneSim[j]].PrendreIdent();
									_sError = "Error ExternalDataRouting: RHHU ID `" + oss.str() + "` not found in source file: " + sDataFilename;
									delete netcdfFile;
									return false;
								}

								_netCdf_IndexSimToIndexList.push_back(it - vIndexSim.begin());
							}
						}
						else
						{
							//_iDimMode == 4	//REACH
							for(j=0; j!=_netCdf_ids.size(); j++)
							{
								if(_pSimHyd->PrendreTroncons().RechercheTroncon(_netCdf_ids[j]) == nullptr)
								{
									//id troncon non existant dans le projet
									oss.str("");
									oss << _netCdf_ids[j];
									_sError = "Error reading NetCDF file: " + sDataFilename + ": The REACH ID `" + oss.str() + "` does not exist in the project";
									delete netcdfFile;
									return false;
								}

								m = _pSimHyd->PrendreTroncons().IdentVersIndex(_netCdf_ids[j]);
								vIndexSim.push_back(m);
							}

							vector<size_t> idxTronconSim = _pSimHyd->PrendreTronconsSimules();
							for(j=0; j!=idxTronconSim.size(); j++)
							{
								it = std::find(vIndexSim.begin(), vIndexSim.end(), idxTronconSim[j]);
								if(it == vIndexSim.end())
								{
									//id troncon non existant dans le netcdf source
									oss.str("");
									oss << _pSimHyd->PrendreTroncons()[idxTronconSim[j]]->PrendreIdent();
									_sError = "Error ExternalDataRouting: REACH ID `" + oss.str() + "` not found in source file: " + sDataFilename;
									delete netcdfFile;
									return false;
								}

								_netCdf_IndexSimToIndexList.push_back(it - vIndexSim.begin());
							}
						}
					}
				}
			}

			delete netcdfFile;
		}

		_sParamFilePath = sFile;

		return true;
	}


	void ModeLecture::Initialise()
	{
		if(_iDimMode == 1 || _iDimMode == 2)	//GRID ou STATION
		{
			string str;

			//pondération

			if(_iVarMode == 1)
			{
				if(_iDimMode == 1)
					str = Combine(PrendreRepertoire(_sParamFilePath), "prod123-grid.pth");
				else
					str = Combine(PrendreRepertoire(_sParamFilePath), "prod123-station.pth");
			}
			else
			{
				if(_iVarMode == 2)
				{
					if(_iDimMode == 1)
						str = Combine(PrendreRepertoire(_sParamFilePath), "prodtot-grid.pth");
					else
						str = Combine(PrendreRepertoire(_sParamFilePath), "prodtot-station.pth");
				}
				else //_iVarMode == 5
				{
					if(_iDimMode == 1)
						str = Combine(PrendreRepertoire(_sParamFilePath), "vcont-grid.pth");
					else
						str = Combine(PrendreRepertoire(_sParamFilePath), "vcont-station.pth");
				}
			}

			if(_ponderation2)
				delete [] _ponderation2;

			if(!LecturePonderation(str))
			{
				CalculePonderation();
				SauvegardePonderation(str);
			}
		}
	}


	bool ModeLecture::LecturePonderation(string sPathFile)
	{
		STATION* station;
		vector<size_t> indexStation;
		double x, y, z;
		size_t idxZone, index;
		string ident;
		bool bSimule;
		int ident_zone;

		if(!FichierExiste(sPathFile))
			return false;

		ifstream fichier(sPathFile);
		if(!fichier)
			throw ERREUR_LECTURE_FICHIER(sPathFile);

		size_t nb_station, ligne, colonne;
		fichier >> nb_station;

		fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		if(nb_station != _gridPoint.size())
		{
			fichier.close();
			return false;
		}

		for(index = 0; index < nb_station; ++index)
		{
			fichier >> ident >> x >> y >> z;

			station = _gridPoint[index].get();

			if (!AlmostEqual(x, station->PrendreCoordonnee().PrendreX(), 0.0001) || 
				!AlmostEqual(y, station->PrendreCoordonnee().PrendreY(), 0.0001) || 
				!AlmostEqual(z, station->PrendreCoordonnee().PrendreZ(), 0.0001))
			{
				fichier.close();
				return false;
			}
		}

		MATRICE<double> pond(_pSimHyd->PrendreZones().PrendreNbZone(), _gridPoint.size(), 0.0);
		vector<size_t> idxZoneSim = _pSimHyd->PrendreZonesSimules();

		_pondIndexUhrh.clear();
		_pondIndexStations.clear();
		_ponderation2 = new double[_pSimHyd->PrendreZones().PrendreNbZone() * nb_station];

		for(ligne = 0; ligne < _pSimHyd->PrendreZones().PrendreNbZone(); ++ligne)
		{
			if(fichier.eof())
			{
				fichier.close();
				return false;
			}

			fichier >> ident_zone;

			if (_pSimHyd->PrendreZones().Recherche(ident_zone) == nullptr)
			{
				fichier.close();
				return false;
			}

			idxZone = _pSimHyd->PrendreZones().IdentVersIndex(ident_zone);

			if(find(begin(idxZoneSim), end(idxZoneSim), idxZone) != end(idxZoneSim))
			{
				bSimule = true;
				indexStation.clear();
			}
			else
				bSimule = false;

			for (colonne = 0; colonne < nb_station; ++colonne)
			{
				fichier >> pond(ligne, colonne);

				_ponderation2[ligne*nb_station+colonne] = pond(ligne, colonne);

				if(bSimule && pond(ligne, colonne) != 0.0)
					indexStation.push_back(colonne);
			}

			if(bSimule)
			{
				_pondIndexUhrh.push_back(idxZone);
				_pondIndexStations.push_back(indexStation);
			}
		}

		fichier.close();

		_ponderation = pond;
		return true;
	}


	void ModeLecture::CalculePonderation()
	{
		vector<double> uhrhPondValue;
		vector<size_t> index_stations;
		COORDONNEE coordonnee;
		size_t i, j, nbStation, idxNearestStation, idx, index, nbZone;
		string sOrigin;
		bool bSimule;
		int ligne, colonne, ident, iNoData;

		nbStation = _gridPoint.size();
		if(nbStation == 0)
			throw ERREUR("Error: ExternalDataRouting::CalculePonderation: there must be at least 1 grid point available.");

		const RASTER<int>& grille = _pSimHyd->PrendreZones().PrendreGrille();

		iNoData = grille.PrendreNoData();
		nbZone = _pSimHyd->PrendreZones().PrendreNbZone();
		
		TRANSFORME_COORDONNEE trans_coord(PROJECTIONS::LONGLAT_WGS84(), grille.PrendreProjection());

		MATRICE<double> pond(_pSimHyd->PrendreZones().PrendreNbZone(), nbStation, 0.0);
		
		vector<size_t> idxZoneSim = _pSimHyd->PrendreZonesSimules();		

		vector<size_t> indexStation;

		_pondIndexUhrh.clear();
		_pondIndexStations.clear();

		_ponderation2 = new double[_pSimHyd->PrendreZones().PrendreNbZone() * nbStation];

		vector<COORDONNEE> coordonnees(nbStation);
		for(index = 0; index < nbStation; index++)
			coordonnees[index] = trans_coord.TransformeXYZ(_gridPoint[index]->PrendreCoordonnee());

		uhrhPondValue.resize(nbZone, 0.0);
		for(index=0; index!=nbZone; index++)
			uhrhPondValue[index] = 1.0 / (_pSimHyd->PrendreZones()[index]).PrendreNbPixel();

		const int nb_ligne = static_cast<int>(grille.PrendreNbLigne());
		const int nb_colonne = static_cast<int>(grille.PrendreNbColonne());

		//nbPixelTotal = nb_ligne * nb_colonne;
		//pixelEnCours = 0;

		if(_iVarMode == 1)
		{
			if(_iDimMode == 1)
				sOrigin = "prod123-grid.pth";
			else
				sOrigin = "prod123-station.pth";
		}
		else
		{
			if(_iVarMode == 2)
			{
				if(_iDimMode == 1)
					sOrigin = "prodtot-grid.pth";
				else
					sOrigin = "prodtot-station.pth";
			}
			else //_iVarMode == 5
			{
				if(_iDimMode == 1)
					sOrigin = "vcont-grid.pth";
				else
					sOrigin = "vcont-station.pth";
			}
		}

		std::cout << endl << "Computing grid point/RHHU weightings (" << sOrigin << ")...   " << GetCurrentTimeStr() << flush;
		_listLog.push_back("Computing grid point/RHHU weightings (" + sOrigin + ")...   " + GetCurrentTimeStr());

		//std::cout << endl << "pixel " << pixelEnCours << "/" << nbPixelTotal << '\r' << std::flush;
		
		if(_pSimHyd->_bLogPerf)
			_pSimHyd->_logPerformance.AddStep("Computing grid point/RHHU weightings (" + sOrigin + ")");

		for(ligne=0; ligne!=nb_ligne; ligne++)
		{
			for(colonne=0; colonne!=nb_colonne; colonne++)
			{
				ident = grille(ligne, colonne);

				if(ident != iNoData)
				{
					coordonnee = grille.LigColVersCoordonnee(ligne, colonne);
					
					idxNearestStation = GetIndexNearestCoord(coordonnees, coordonnee);
					idx = _pSimHyd->PrendreZones()._vIdentVersIndex[abs(ident)];

					pond(idx, idxNearestStation)+= uhrhPondValue[idx];
				}

				//if(bDisplay)
				//{
				//	++pixelEnCours;
				//	std::cout << "pixel " << pixelEnCours << "/" << nbPixelTotal << '\r' << std::flush;
				//}
			}
		}

		for(i=0; i!=pond.PrendreNbLigne(); i++)
		{
			if(find(begin(idxZoneSim), end(idxZoneSim), i) != end(idxZoneSim))
			{
				bSimule = true;
				indexStation.clear();
			}
			else
				bSimule = false;

			for(j=0; j<pond.PrendreNbColonne(); j++)
			{
				//pond(i, j)/= zones[i].PrendreNbPixel();
				
				_ponderation2[i*nbStation+j] = pond(i, j);

				if(bSimule && pond(i, j) != 0.0)
					indexStation.push_back(j);
			}

			if(bSimule)
			{
				_pondIndexUhrh.push_back(i);
				_pondIndexStations.push_back(indexStation);
			}
		}

		_ponderation = pond;

		if(_pSimHyd->_bLogPerf)
			_pSimHyd->_logPerformance.AddStep("Completed");
	}


	void ModeLecture::SauvegardePonderation(string sPathFile)
	{
		ofstream fichier(sPathFile);
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(sPathFile);

		const size_t nb_station = _gridPoint.size();

		fichier << nb_station << endl;

		for(size_t index = 0; index < nb_station; ++index)
		{
			COORDONNEE coordonnee = _gridPoint[index]->PrendreCoordonnee();

			fichier << fixed 
				    << _gridPoint[index]->PrendreIdent() << ' '
				    << coordonnee.PrendreX() << ' '
				    << coordonnee.PrendreY() << ' '
				    << coordonnee.PrendreZ() << endl;
		}

		for(size_t index = 0; index < _pSimHyd->PrendreZones().PrendreNbZone(); ++index)
		{
			fichier << (_pSimHyd->PrendreZones()[index]).PrendreIdent() << ' ';

			for (size_t n = 0; n < nb_station; ++n)			
				fichier << _ponderation(index, n) << ' ';
			
			fichier << endl;
		}

		fichier.close();
	}


	void ModeLecture::InterpolationDonnees()
	{
		ZONES& zones = _pSimHyd->PrendreZones();

		DATE_HEURE date_courante = _pSimHyd->PrendreDateCourante();
		unsigned short pas_de_temps = _pSimHyd->PrendrePasDeTemps();

		vector<size_t> index_zones = _pSimHyd->PrendreZonesSimules();

		std::pair<float, float> temp_jour;
		DONNEE_METEO donnee_station;
		shared_ptr<STATION_METEO_NETCDF_STATION> pGridPoint;
		ZONE* pZone;
		size_t nbStation, nbStationTotal, index_station, index, index_zone, idxTime, idx;
		float ponderation;
		float fVal1, fVal2, fVal3;

		nbStationTotal = _gridPoint.size();

		for(index=0; index!=_pondIndexUhrh.size(); index++)
		{
			index_zone = _pondIndexUhrh[index];
			
			pZone = &zones[index_zone];

			fVal1 = 0.0f;
			fVal2 = 0.0f;
			fVal3 = 0.0f;

			nbStation = _pondIndexStations[index].size();
			for(index_station=0; index_station!=nbStation; index_station++)
			{
				ponderation = static_cast<float>(_ponderation2[index_zone*nbStationTotal+_pondIndexStations[index][index_station]]);
				
				pGridPoint = _gridPoint[_pondIndexStations[index][index_station]];

				idxTime = _netCdf_dateDebutVecteur.NbHeureEntre(date_courante) / pas_de_temps;
				
				if(_iDimMode == 1) //GRID
					idx = (idxTime * _netCdf_lNbCoord) + (pGridPoint->_lIndexLat * _netCdf_lNbLong + pGridPoint->_lIndexLon);
				else //STATION
					idx = idxTime * _netCdf_lNbStation + pGridPoint->_lIndexStation;

				if(_iVarMode == 1) //PROD1, PROD2, PROD3
				{
					fVal1+= _pVal[_idxProd1][idx] * ponderation;
					fVal2+= _pVal[_idxProd2][idx] * ponderation;
					fVal3+= _pVal[_idxProd3][idx] * ponderation;
				}
				else
				{
					if(_iVarMode == 2) //PRODTOT
						fVal1+= _pVal[_idxProdTot][idx] * ponderation;
					else
						fVal1+= _pVal[_idxVCont][idx] * ponderation;	//VCONT
				}
			}

			if(_iVarMode == 2) //PRODTOT
			{
				fVal2 = fVal1 * _fDISTRIBUTION_COEFF2;
				fVal3 = fVal1 * (1.0f - (_fDISTRIBUTION_COEFF1 + _fDISTRIBUTION_COEFF2));
				fVal1 = fVal1 * _fDISTRIBUTION_COEFF1;
			}

			if(_iVarMode == 1 || _iVarMode == 2)
			{
				pZone->ChangeProdSurf(fVal1);	//PROD1 (mm)
				pZone->ChangeProdHypo(fVal2);	//PROD2 (mm)
				pZone->ChangeProdBase(fVal3);	//PROD3 (mm)
			}
			else //_iVarMode == 5
				pZone->ChangeApport(fVal1);		//VCONT (mm)
		}
	}


	void ModeLecture::AttributionDonneesUhrh()
	{
		ZONES& zones = _pSimHyd->PrendreZones();

		DATE_HEURE date_courante = _pSimHyd->PrendreDateCourante();
		unsigned short pas_de_temps = _pSimHyd->PrendrePasDeTemps();

		vector<size_t> idxZoneSim = _pSimHyd->PrendreZonesSimules();

		vector<int>::iterator itId;
		std::pair<float, float> temp_jour;
		DONNEE_METEO donnee_station;
		shared_ptr<STATION_METEO_NETCDF_STATION> pGridPoint;
		TRONCON* pTroncon;
		ZONE* pZone;
		size_t index, idxTime, idx;
		float fVal;

		idxTime = _netCdf_dateDebutVecteur.NbHeureEntre(date_courante) / pas_de_temps;

		//for(index=0; index!=_pondIndexUhrh.size(); index++)
		for(index=0; index!=idxZoneSim.size(); index++)
		{
			pZone = &zones[idxZoneSim[index]];

			idx = idxTime * _netCdf_lNbStation + _netCdf_IndexSimToIndexList[index];

			switch(_iVarMode)
			{
			case 1:
				pZone->ChangeProdSurf(_pVal[_idxProd1][idx]);
				pZone->ChangeProdHypo(_pVal[_idxProd2][idx]);
				pZone->ChangeProdBase(_pVal[_idxProd3][idx]);
				break;

			case 2:
				pZone->ChangeProdSurf(_pVal[_idxProdTot][idx] * _fDISTRIBUTION_COEFF1);
				pZone->ChangeProdHypo(_pVal[_idxProdTot][idx] * _fDISTRIBUTION_COEFF2);
				pZone->ChangeProdBase(_pVal[_idxProdTot][idx] * (1.0f - (_fDISTRIBUTION_COEFF1 + _fDISTRIBUTION_COEFF2)));
				break;

			case 3:
				pZone->_ecoulementSurf = _pVal[_idxQLat1][idx];
				pZone->_ecoulementHypo = _pVal[_idxQLat2][idx];
				pZone->_ecoulementBase = _pVal[_idxQLat3][idx];

				pZone->_apport_lateral_uhrh = pZone->_ecoulementSurf + pZone->_ecoulementHypo + pZone->_ecoulementBase;

				pTroncon = pZone->PrendreTronconAval();

				fVal = pTroncon->PrendreApportLateral();
				fVal+= pZone->_apport_lateral_uhrh;
				pTroncon->ChangeApportLateral(fVal);

				pTroncon->_surf+= pZone->_ecoulementSurf;
				pTroncon->_hypo+= pZone->_ecoulementHypo;
				pTroncon->_base+= pZone->_ecoulementBase;
				break;

			case 4:
				pZone->_ecoulementSurf = _pVal[_idxQLatTot][idx] * _fDISTRIBUTION_COEFF1;
				pZone->_ecoulementHypo = _pVal[_idxQLatTot][idx] * _fDISTRIBUTION_COEFF2;
				pZone->_ecoulementBase = _pVal[_idxQLatTot][idx] * (1.0f - (_fDISTRIBUTION_COEFF1 + _fDISTRIBUTION_COEFF2));

				pZone->_apport_lateral_uhrh = pZone->_ecoulementSurf + pZone->_ecoulementHypo + pZone->_ecoulementBase;

				pTroncon = pZone->PrendreTronconAval();

				fVal = pTroncon->PrendreApportLateral();
				fVal+= pZone->_apport_lateral_uhrh;
				pTroncon->ChangeApportLateral(fVal);

				pTroncon->_surf+= pZone->_ecoulementSurf;
				pTroncon->_hypo+= pZone->_ecoulementHypo;
				pTroncon->_base+= pZone->_ecoulementBase;
				break;

			case 5:
				pZone->ChangeApport(_pVal[_idxVCont][idx]);
			}
		}
	}


	void ModeLecture::AttributionDonneesTroncon()
	{
		DATE_HEURE date_courante = _pSimHyd->PrendreDateCourante();
		unsigned short pas_de_temps = _pSimHyd->PrendrePasDeTemps();

		vector<size_t> idxTronconSim = _pSimHyd->PrendreTronconsSimules();

		vector<int>::iterator itId;

		std::pair<float, float> temp_jour;
		DONNEE_METEO donnee_station;
		shared_ptr<STATION_METEO_NETCDF_STATION> pGridPoint;
		TRONCON* pTroncon;
		size_t i, idxTime, idx;

		idxTime = _netCdf_dateDebutVecteur.NbHeureEntre(date_courante) / pas_de_temps;
		
		for(i=0; i!=idxTronconSim.size(); i++)
		{
			pTroncon = _pSimHyd->PrendreTroncons()[idxTronconSim[i]];

			idx = idxTime * _netCdf_lNbStation + _netCdf_IndexSimToIndexList[i];

			if(_iVarMode == 3)	//QLat1, QLat2, QLat3
			{
				pTroncon->ChangeApportLateral(_pVal[_idxQLat1][idx] + _pVal[_idxQLat2][idx] + _pVal[_idxQLat3][idx]);

				pTroncon->_surf = _pVal[_idxQLat1][idx];
				pTroncon->_hypo = _pVal[_idxQLat2][idx];
				pTroncon->_base = _pVal[_idxQLat3][idx];
			}
			else
			{
				//CTot
				pTroncon->ChangeApportLateral(_pVal[_idxQLatTot][idx]);

				pTroncon->_surf = _pVal[_idxQLatTot][idx] * _fDISTRIBUTION_COEFF1;
				pTroncon->_hypo = _pVal[_idxQLatTot][idx] * _fDISTRIBUTION_COEFF2;
				pTroncon->_base = _pVal[_idxQLatTot][idx] * (1.0f - (_fDISTRIBUTION_COEFF1 + _fDISTRIBUTION_COEFF2));
			}
		}
	}

}
