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

#include "netcdf_file.hpp"

//#include "projections.hpp"
#include "util.hpp"
#include "erreur.hpp"
//#include "transforme_coordonnee.hpp"

#include <fstream>
#include <map>
#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/shared_array.hpp>


using namespace std;


namespace HYDROTEL
{

	NETCDF_FILE::NETCDF_FILE()
	{
		_netCdf_fileId = 0;

		_simTimeStep = 0;
		
		_netCdf_iType = -1;

		_netCdf_lPasTemps = 0;
		_indexDebut = 0;

		_netCdf_lNbPasTemps = 0;
		_netCdf_lNbStation = 0;

		_netCdf_lNbLat = 0;
		_netCdf_lNbLong = 0;
		_netCdf_lNbCoord = 0;

		_sError = "";
	}


	NETCDF_FILE::~NETCDF_FILE()
	{
		if(_netCdf_fileId != 0)
			nc_close(_netCdf_fileId);
	}


	//void NETCDF_FILE::Lecture(const PROJECTION& projection)
	//{
	//	size_t i;
	//	string str;

	//	Detruire();

	//	_ProjectionProjet._spatial_reference.importFromProj4(projection.ExportProj4().c_str());

	//	string extension = PrendreExtension(PrendreNomFichier());

	//	if (extension == ".h5")
	//	{
	//		LectureFormatHDF5();
	//	}
	//	else if (extension == ".nc")
	//	{
	//		str = LectureExtentLimit();
	//		if (str != "")
	//			throw ERREUR(str);

	//		str = LectureFormatNetCDFConfig();
	//		if(str != "")
	//			throw ERREUR(str);

	//		switch (_netCdf_iType)
	//		{
	//		case 0:
	//			LectureFormatNetCDFTypeStation();
	//			_pSimHyd->_outputCDF = true;
	//			break;
	//		
	//		case 1:
	//			LectureFormatNetCDFTypeGrid();
	//			_pSimHyd->_outputCDF = true;
	//			break;

	//		default:
	//			throw ERREUR("Lecture NetCDF: parametre type invalide");
	//		}
	//		
	//	}
	//	else if (extension == ".stm")
	//	{
	//		LectureFormatSTM(projection);
	//	}
	//	else
	//	{
	//		throw ERREUR_LECTURE_FICHIER( PrendreNomFichier() );
	//	}

	//	//convert stations coordinate from CRS of data file to project CRS
	//	TRANSFORME_COORDONNEE trans_coord(PrendreProjection(),  _ProjectionProjet);

	//	for(i=0; i!=_stations.size(); i++)
	//		_stations[i].get()->_coordonneeCRSprojet = trans_coord.TransformeXYZ(_stations[i].get()->PrendreCoordonnee());

	//	CreeMapRecherche();
	//}


	////------------------------------------------------------------------------------------------------
	//string NETCDF_FILE::LectureExtentLimit()
	//{
	//	vector<string> valeurs;
	//	string sPathFile, str, ligne;
	//	bool bEmptyFile;

	//	sPathFile = "extent-limit.config";
	//	sPathFile = Combine(PrendreRepertoire(_nom_fichier), sPathFile);

	//	_dExtentLimitNorth = -1.0;
	//	_dExtentLimitSouth = -1.0;
	//	_dExtentLimitEast = -1.0;
	//	_dExtentLimitWest = -1.0;

	//	bEmptyFile = false;

	//	if(FichierExiste(sPathFile))
	//	{
	//		ifstream fichier(sPathFile);
	//		if(!fichier)
	//		{
	//			str = "Erreur ouverture fichier: " + sPathFile;
	//			return str;
	//		}

	//		bEmptyFile = true;

	//		try
	//		{
	//			while(!fichier.eof() || fichier.bad())
	//			{
	//				ligne = "";
	//				getline_mod(fichier, ligne);
	//				ligne = TrimString(ligne);

	//				if (ligne.size() > 1 && ligne[0] == '/' && ligne[1] == '/')
	//					ligne = "";

	//				if(ligne != "")
	//				{
	//					SplitString(valeurs, ligne, ";", true, true);

	//					if(valeurs.size() == 2)
	//					{
	//						bEmptyFile = false;

	//						str = TrimString(valeurs[0]);
	//						boost::algorithm::to_upper(str);

	//						istringstream iss(valeurs[1]);

	//						if(str == "NORTH")
	//							iss >> _dExtentLimitNorth;
	//						else
	//						{
	//							if(str == "SOUTH")
	//								iss >> _dExtentLimitSouth;
	//							else
	//							{
	//								if(str == "EAST")
	//									iss >> _dExtentLimitEast;
	//								else
	//								{
	//									if(str == "WEST")
	//										iss >> _dExtentLimitWest;
	//									else
	//									{
	//										fichier.close();
	//										str = "Erreur lecture fichier: " + sPathFile + ": ligne invalide: " + ligne;
	//										return str;
	//									}
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//		catch(...)
	//		{
	//			if(fichier && fichier.is_open())
	//				fichier.close();
	//			str = "Erreur lecture fichier: " + sPathFile;
	//			return str;
	//		}

	//		fichier.close();

	//		if (_dExtentLimitNorth != -1.0 || _dExtentLimitSouth != -1.0 || _dExtentLimitEast != -1.0 || _dExtentLimitWest != -1.0)
	//		{
	//			if (_dExtentLimitNorth == -1.0 || _dExtentLimitSouth == -1.0 || _dExtentLimitEast == -1.0 || _dExtentLimitWest == -1.0)
	//			{
	//				_dExtentLimitNorth = -1.0;
	//				_dExtentLimitSouth = -1.0;
	//				_dExtentLimitEast = -1.0;
	//				_dExtentLimitWest = -1.0;

	//				str = "Erreur lecture fichier: " + sPathFile + ": extent invalide";
	//				return str;
	//			}
	//		}
	//	}

	//	if(bEmptyFile)
	//		str = "Erreur lecture fichier: " + sPathFile + ": le fichier est vide ou le format est invalide";
	//	else
	//		str = "";

	//	return str;
	//}


	//ret = nc_inq_var(_ncid, latid, 0, &rh_type, &rh_ndims, rh_dimids, &rh_natts);
	//ret = nc_inq(_ncid, &ndims, &nvars, &ngatts, &unlimdimid);


	//------------------------------------------------------------------------------------------------
	bool NETCDF_FILE::Open(string sFilename, bool bListUhrhTroncon)
	{
		bool ret;

		if(_netCdf_iType == 0)
			ret = OpenTypeStation(sFilename, bListUhrhTroncon);
		else
		{
			if(_netCdf_iType == 1)
				ret = OpenTypeGrid(sFilename);
			else
				ret = false;
		}

		return ret;
	}

	//------------------------------------------------------------------------------------------------
	//Pour type == GRID
	//Format 9.3.1

	bool NETCDF_FILE::OpenTypeGrid(string sFilename)
	{
		unsigned short yy, mm, dd, hh, min, ss;
		istringstream iss;
		ostringstream oss;
		DATE_HEURE dtDebutFichier;
		DATE_HEURE dtFinFichier;
		DATE_HEURE dtTimeUnit;
		DATE_HEURE dt;
		size_t lNbPasTempsFichier, i, indexFin;
		double dVal;
		string str1, str2, str3;
		bool bMinutesUnit;
		int ret, iVal;
		int latid, lonid, timedimid, timeid, iLatDimID, iLonDimID;

		_sError = "";

		oss.str("");

		ret = nc_open(sFilename.c_str(), NC_NOWRITE, &_netCdf_fileId);
		if(ret != NC_NOERR)
		{
			oss << "Error opening NetCDF file: " << sFilename << ": nc_open return code " << ret;
			_sError = oss.str();
			return false;
		}

		//valide l'unité des pas de temps
		//l'unité doit etre "days since yyyy-mm-dd hh:00:00" ou "minutes since yyyy-mm-dd hh:00:00"
		ret = nc_inq_varid(_netCdf_fileId, _netCdf_TimeVarName.c_str(), &timeid);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time variable `" + _netCdf_TimeVarName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_attlen(_netCdf_fileId, timeid, "units", &i);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time variable `" + _netCdf_TimeVarName + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`";
			nc_close(_netCdf_fileId);
			return false;
		}

		boost::shared_array<char> str_att(new char[i+1]);
		ret = nc_get_att(_netCdf_fileId, timeid, "units", reinterpret_cast<void*>(&str_att[0]));
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time variable `" + _netCdf_TimeVarName + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`";
			nc_close(_netCdf_fileId);
			return false;
		}

		str1 = str_att.get();
		str2 = str1.substr(0, i);
		boost::algorithm::to_lower(str2);

		if(str2.substr(0, 14) == "minutes since " && str2.length() == 33)
		{
			bMinutesUnit = true;	//"minutes since 1970-01-01 00:00:00"

			str3 = str2.substr(14);

			iss.str(str3.substr(0, 4));
			iss >> yy;
			iss.clear();
			iss.str(str3.substr(5, 2));
			iss >> mm;
			iss.clear();
			iss.str(str3.substr(8, 2));
			iss >> dd;
			iss.clear();
			iss.str(str3.substr(11, 2));
			iss >> hh;
			iss.clear();
			iss.str(str3.substr(14, 2));
			iss >> min;
			iss.clear();
			iss.str(str3.substr(17, 2));
			iss >> ss;

			if(min != 0 || ss != 0)
			{
				_sError = "Error reading NetCDF file: " + sFilename + ": invalid time units: minutes and seconds must be 0";
				nc_close(_netCdf_fileId);
				return false;
			}

			dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
		}
		else
		{
			if(str2.substr(0, 11) == "days since " && str2.length() == 30)
			{
				bMinutesUnit = false;	//"days since 1970-01-01 00:00:00"

				str3 = str2.substr(11);

				iss.str(str3.substr(0, 4));
				iss >> yy;
				iss.clear();
				iss.str(str3.substr(5, 2));
				iss >> mm;
				iss.clear();
				iss.str(str3.substr(8, 2));
				iss >> dd;
				iss.clear();
				iss.str(str3.substr(11, 2));
				iss >> hh;
				iss.clear();
				iss.str(str3.substr(14, 2));
				iss >> min;
				iss.clear();
				iss.str(str3.substr(17, 2));
				iss >> ss;

				if(min != 0 || ss != 0)
				{
					_sError = "Error reading NetCDF file: " + sFilename + ": invalid time units: minutes and seconds must be 0";
					nc_close(_netCdf_fileId);
					return false;
				}

				dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
			}
			else
			{
				_sError = "Error reading NetCDF file: " + sFilename + ": invalid time units: must be `minutes since yyyy-mm-dd hh:00:00` or `days since yyyy-mm-dd hh:00:00`";
				nc_close(_netCdf_fileId);
				return false;
			}
		}

		//lecture longitude et latitude dimensions
		ret = nc_inq_dimid(_netCdf_fileId, _netCdf_LatDimName.c_str(), &iLatDimID);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": latitude dimension `" + _netCdf_LatDimName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_dimlen(_netCdf_fileId, iLatDimID, &_netCdf_lNbLat);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading latitude dimension length: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_dimid(_netCdf_fileId, _netCdf_LonDimName.c_str(), &iLonDimID);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": longitude dimension `" + _netCdf_LonDimName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_dimlen(_netCdf_fileId, iLonDimID, &_netCdf_lNbLong);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading longitude dimension length: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		_netCdf_lNbCoord = _netCdf_lNbLat * _netCdf_lNbLong;
		
		//lecture variables ids
		ret = nc_inq_varid(_netCdf_fileId, _netCdf_LatVarName.c_str(), &latid);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": latitude variable `" + _netCdf_LatVarName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_varid(_netCdf_fileId, _netCdf_LonVarName.c_str(), &lonid);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": longitude variable `" + _netCdf_LonVarName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		//lecture des coordonnees
		//_projection = PROJECTIONS::LONGLAT_WGS84();

		_netCdf_latitudes.resize(_netCdf_lNbLat);

		ret = nc_get_var_double(_netCdf_fileId, latid, &_netCdf_latitudes[0]);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading latitude data: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		_netCdf_longitudes.resize(_netCdf_lNbLong);

		ret = nc_get_var_double(_netCdf_fileId, lonid, &_netCdf_longitudes[0]);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading longitude data: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		//lecture des pas de temps
		ret = nc_inq_dimid(_netCdf_fileId, _netCdf_TimeDimName.c_str(), &timedimid);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time dimension `" + _netCdf_TimeDimName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_dimlen(_netCdf_fileId, timedimid, &lNbPasTempsFichier);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading time dimension length: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		if(lNbPasTempsFichier < 2)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": invalid timestep count";
			nc_close(_netCdf_fileId);
			return false;
		}

		vector<double> dTimes;
		vector<int> iTimes;

		if(bMinutesUnit)
		{
			iTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_int(_netCdf_fileId, timeid, &iTimes[0]);
		}
		else
		{
			dTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_double(_netCdf_fileId, timeid, &dTimes[0]);
		}
		
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading time data: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		//determine dates debut et fin du fichier
		if(bMinutesUnit)
			_netCdf_lPasTemps = static_cast<size_t>((iTimes[1] - iTimes[0]) / 60);		//time step [hrs]
		else
			_netCdf_lPasTemps = static_cast<size_t>((dTimes[1] - dTimes[0]) * 24.0);	//

		//---------------------------------------------
		if(!bMinutesUnit && (_netCdf_lPasTemps != _simTimeStep))	//only for days unit because of double precision
		{
			//on passe d'une valeur en jours (epoch time) vers une valeur en heures
			//essaie d'ajouter ou d'enlever 1 sec pour corriger les problemes de précision des réels lors de la conversion
			dVal = (dTimes[1] - dTimes[0]) * 24.0 + 0.000011574074074074074074074074074074;	//ajoute 1 sec
			_netCdf_lPasTemps = static_cast<size_t>(dVal);

			if (_netCdf_lPasTemps != _simTimeStep)
			{
				dVal = (dTimes[1] - dTimes[0]) * 24.0 - 0.000011574074074074074074074074074074;	//enleve 1 sec
				_netCdf_lPasTemps = static_cast<size_t>(dVal);
			}
		}

		if(_netCdf_lPasTemps != _simTimeStep)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": timestep of data must be equal to simulation timestep";
			nc_close(_netCdf_fileId);
			return false;
		}
		//---------------------------------------------

		//
		dtDebutFichier = DATE_HEURE(dtTimeUnit.PrendreAnnee(), dtTimeUnit.PrendreMois(), dtTimeUnit.PrendreJour(), dtTimeUnit.PrendreHeure());

		if(bMinutesUnit)
			iVal = iTimes[0] / 60;	//hrs
		else
			iVal = static_cast<int>(dTimes[0] * 24.0);	//hrs

		if(iVal > 0)
			dtDebutFichier.AdditionHeure(iVal);
		else
		{
			if (iVal < 0)
				dtDebutFichier.SoustraitHeure(abs(iVal));
		}

		//
		dtFinFichier = DATE_HEURE(dtTimeUnit.PrendreAnnee(), dtTimeUnit.PrendreMois(), dtTimeUnit.PrendreJour(), dtTimeUnit.PrendreHeure());

		if(bMinutesUnit)
			iVal = iTimes[lNbPasTempsFichier-1] / 60;	//hrs
		else
			iVal = static_cast<int>(dTimes[lNbPasTempsFichier-1] * 24.0);	//hrs

		if (iVal > 0)
			dtFinFichier.AdditionHeure(iVal);
		else
		{
			if (iVal < 0)
				dtFinFichier.SoustraitHeure(abs(iVal));
		}

		//determine la plage de données a lire selon les date de debut et fin de la simulation

		//date debut
		//met heure debut à 0; //si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		
		_netCdf_dateDebutVecteur = DATE_HEURE(_dtDebutSim.PrendreAnnee(), _dtDebutSim.PrendreMois(), _dtDebutSim.PrendreJour(), 0);

		if(_netCdf_dateDebutVecteur < dtDebutFichier)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": data missing for simulation begin date";
			nc_close(_netCdf_fileId);
			return false;
		}

		_indexDebut = dtDebutFichier.NbHeureEntre(_netCdf_dateDebutVecteur) / _netCdf_lPasTemps;

		//date fin
		dt = DATE_HEURE(_dtFinSim.PrendreAnnee(), _dtFinSim.PrendreMois(), _dtFinSim.PrendreJour(), 0); 		
		if(_simTimeStep != 24)
			dt.AdditionHeure(24);	//si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		

		if(dt > dtFinFichier)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": data missing for simulation end date";
			nc_close(_netCdf_fileId);
			return false;
		}

		indexFin = dtDebutFichier.NbHeureEntre(dt) / _netCdf_lPasTemps;

		//heure lu en fin de pas de temps et remise en debut de pas de temps.
		if(_simTimeStep != 24)
			_indexDebut = _indexDebut + 1;

		_netCdf_lNbPasTemps = indexFin - _indexDebut + 1;

		_sFilename = sFilename;

		return true;
	}


	//------------------------------------------------------------------------------------------------
	//Pour type == STATION
	//Format H2.1

	bool NETCDF_FILE::OpenTypeStation(string sFilename, bool bListUhrhTroncon)
	{
		unsigned short yy, mm, dd, hh, min, ss;
		istringstream iss;
		ostringstream oss;
		DATE_HEURE dtDebutFichier;
		DATE_HEURE dtFinFichier;
		DATE_HEURE dtTimeUnit;
		DATE_HEURE dt;
		size_t lNbPasTempsFichier, i, indexFin;
		double dVal;
		string str1, str2, str3;
		bool bMinutesUnit;
		int ret, iVal;
		int latid, lonid, timedimid, timeid, iListDimID, iListID;

		oss.str("");

		ret = nc_open(sFilename.c_str(), NC_NOWRITE, &_netCdf_fileId);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error opening NetCDF file: " + sFilename + ": nc_open return code " + oss.str();
			return false;
		}

		//valide l'unité des pas de temps
		//l'unité doit etre "days since yyyy-mm-dd hh:00:00" ou "minutes since yyyy-mm-dd hh:00:00"
		ret = nc_inq_varid(_netCdf_fileId, _netCdf_TimeVarName.c_str(), &timeid);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time variable `" + _netCdf_TimeVarName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_attlen(_netCdf_fileId, timeid, "units", &i);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time variable `" + _netCdf_TimeVarName + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`";
			nc_close(_netCdf_fileId);
			return false;
		}

		boost::shared_array<char> str_att(new char[i+1]);
		ret = nc_get_att(_netCdf_fileId, timeid, "units", reinterpret_cast<void*>(&str_att[0]));
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time variable `" + _netCdf_TimeVarName + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`";
			nc_close(_netCdf_fileId);
			return false;
		}

		str1 = str_att.get();
		str2 = str1.substr(0, i);
		boost::algorithm::to_lower(str2);

		if(str2.substr(0, 14) == "minutes since " && str2.length() == 33)
		{
			bMinutesUnit = true;	//"minutes since 1970-01-01 00:00:00"

			str3 = str2.substr(14);
			
			iss.str(str3.substr(0, 4));
			iss >> yy;
			iss.clear();
			iss.str(str3.substr(5, 2));
			iss >> mm;
			iss.clear();
			iss.str(str3.substr(8, 2));
			iss >> dd;
			iss.clear();
			iss.str(str3.substr(11, 2));
			iss >> hh;
			iss.clear();
			iss.str(str3.substr(14, 2));
			iss >> min;
			iss.clear();
			iss.str(str3.substr(17, 2));
			iss >> ss;

			if(min != 0 || ss != 0)
			{
				_sError = "Error reading NetCDF file: " + sFilename + ": invalid time units: minutes and seconds must be 0";
				nc_close(_netCdf_fileId);
				return false;
			}

			dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
		}
		else
		{
			if(str2.substr(0, 11) == "days since " && str2.length() == 30)
			{
				bMinutesUnit = false;	//"days since 1970-01-01 00:00:00"

				str3 = str2.substr(11);

				iss.str(str3.substr(0, 4));
				iss >> yy;
				iss.clear();
				iss.str(str3.substr(5, 2));
				iss >> mm;
				iss.clear();
				iss.str(str3.substr(8, 2));
				iss >> dd;
				iss.clear();
				iss.str(str3.substr(11, 2));
				iss >> hh;
				iss.clear();
				iss.str(str3.substr(14, 2));
				iss >> min;
				iss.clear();
				iss.str(str3.substr(17, 2));
				iss >> ss;

				if(min != 0 || ss != 0)
				{
					_sError = "Error reading NetCDF file: " + sFilename + ": invalid time units: minutes and seconds must be 0";
					nc_close(_netCdf_fileId);
					return false;
				}

				dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
			}
			else
			{
				_sError = "Error reading NetCDF file: " + sFilename + ": invalid time units: must be `minutes since yyyy-mm-dd hh:00:00` or `days since yyyy-mm-dd hh:00:00`";
				nc_close(_netCdf_fileId);
				return false;
			}
		}

		//lecture nb item (gridpoint, rhhu or reach)
		ret = nc_inq_dimid(_netCdf_fileId, _netCdf_StationDimName.c_str(), &iListDimID);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": STATION/ID dimension `" + _netCdf_StationDimName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_dimlen(_netCdf_fileId, iListDimID, &_netCdf_lNbStation);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading STATION/ID dimension length: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		if(bListUhrhTroncon)
		{
			//type UHRH, REACH
			ret = nc_inq_varid(_netCdf_fileId, _netCdf_StationVarName.c_str(), &iListID);
			if(ret != NC_NOERR)
			{
				_sError = "Error reading NetCDF file: " + sFilename + ": id variable `" + _netCdf_StationVarName + "` not found";
				nc_close(_netCdf_fileId);
				return false;
			}

			//lecture des identifiants

			_netCdf_ids.resize(_netCdf_lNbStation);

			ret = nc_get_var_int(_netCdf_fileId, iListID, &_netCdf_ids[0]);
			if(ret != NC_NOERR)
			{
				oss << ret;
				_sError = "Error reading NetCDF file: " + sFilename + ": error reading id data: error code " + oss.str();
				nc_close(_netCdf_fileId);
				return false;
			}
		}
		else
		{
			//type STATION
			ret = nc_inq_varid(_netCdf_fileId, _netCdf_LatVarName.c_str(), &latid);
			if(ret != NC_NOERR)
			{
				_sError = "Error reading NetCDF file: " + sFilename + ": latitude variable `" + _netCdf_LatVarName + "` not found";
				nc_close(_netCdf_fileId);
				return false;
			}

			ret = nc_inq_varid(_netCdf_fileId, _netCdf_LonVarName.c_str(), &lonid);
			if (ret != NC_NOERR)
			{
				_sError = "Error reading NetCDF file: " + sFilename + ": longitude variable `" + _netCdf_LonVarName + "` not found";
				nc_close(_netCdf_fileId);
				return false;
			}

			//lecture des coordonnees
			//_projection = PROJECTIONS::LONGLAT_WGS84();

			_netCdf_latitudes.resize(_netCdf_lNbStation);

			ret = nc_get_var_double(_netCdf_fileId, latid, &_netCdf_latitudes[0]);
			if(ret != NC_NOERR)
			{
				oss << ret;
				_sError = "Error reading NetCDF file: " + sFilename + ": error reading latitude data: error code " + oss.str();
				nc_close(_netCdf_fileId);
				return false;
			}

			_netCdf_longitudes.resize(_netCdf_lNbStation);

			ret = nc_get_var_double(_netCdf_fileId, lonid, &_netCdf_longitudes[0]);
			if(ret != NC_NOERR)
			{
				oss << ret;
				_sError = "Error reading NetCDF file: " + sFilename + ": error reading longitude data: error code " + oss.str();
				nc_close(_netCdf_fileId);
				return false;
			}
		}

		//lecture des pas de temps
		ret = nc_inq_dimid(_netCdf_fileId, _netCdf_TimeDimName.c_str(), &timedimid);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": time dimension `" + _netCdf_TimeDimName + "` not found";
			nc_close(_netCdf_fileId);
			return false;
		}

		ret = nc_inq_dimlen(_netCdf_fileId, timedimid, &lNbPasTempsFichier);
		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading time dimension length: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		if(lNbPasTempsFichier < 2)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": invalid timestep count";
			nc_close(_netCdf_fileId);
			return false;
		}

		vector<double> dTimes;
		vector<int> iTimes;
		
		if(bMinutesUnit)
		{
			iTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_int(_netCdf_fileId, timeid, &iTimes[0]);
		}
		else
		{
			dTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_double(_netCdf_fileId, timeid, &dTimes[0]);
		}

		if(ret != NC_NOERR)
		{
			oss << ret;
			_sError = "Error reading NetCDF file: " + sFilename + ": error reading time data: error code " + oss.str();
			nc_close(_netCdf_fileId);
			return false;
		}

		//determine dates debut et fin du fichier
		if(bMinutesUnit)
			_netCdf_lPasTemps = static_cast<size_t>((iTimes[1] - iTimes[0]) / 60);		//time step [hrs]
		else
			_netCdf_lPasTemps = static_cast<size_t>((dTimes[1] - dTimes[0]) * 24.0);	//

		if(!bMinutesUnit && (_netCdf_lPasTemps != _simTimeStep))	//only for days unit because of double precision
		{
			//on passe d'une valeur en jours (epoch time) vers une valeur en heures
			//essaie d'ajouter ou d'enlever 1 sec pour corriger les problemes de précision des réels lors de la conversion
			dVal = (dTimes[1] - dTimes[0]) * 24.0 + 0.000011574074074074074074074074074074;	//ajoute 1 sec
			_netCdf_lPasTemps = static_cast<size_t>(dVal);

			if(_netCdf_lPasTemps != _simTimeStep)
			{
				dVal = (dTimes[1] - dTimes[0]) * 24.0 - 0.000011574074074074074074074074074074;	//enleve 1 sec
				_netCdf_lPasTemps = static_cast<size_t>(dVal);
			}
		}

		if(_netCdf_lPasTemps != _simTimeStep)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": timestep of data must be equal to simulation timestep";
			nc_close(_netCdf_fileId);
			return false;
		}

		//
		dtDebutFichier = DATE_HEURE(dtTimeUnit.PrendreAnnee(), dtTimeUnit.PrendreMois(), dtTimeUnit.PrendreJour(), dtTimeUnit.PrendreHeure());

		if(bMinutesUnit)
			iVal = iTimes[0] / 60;	//hrs
		else
			iVal = static_cast<int>(dTimes[0] * 24.0);	//hrs

		if(iVal > 0)
			dtDebutFichier.AdditionHeure(iVal);
		else
		{
			if(iVal < 0)
				dtDebutFichier.SoustraitHeure(abs(iVal));
		}

		//
		dtFinFichier = DATE_HEURE(dtTimeUnit.PrendreAnnee(), dtTimeUnit.PrendreMois(), dtTimeUnit.PrendreJour(), dtTimeUnit.PrendreHeure());

		if(bMinutesUnit)
			iVal = iTimes[lNbPasTempsFichier-1] / 60;	//hrs
		else
			iVal = static_cast<int>(dTimes[lNbPasTempsFichier-1] * 24.0);	//hrs

		if(iVal > 0)
			dtFinFichier.AdditionHeure(iVal);
		else
		{
			if(iVal < 0)
				dtFinFichier.SoustraitHeure(abs(iVal));
		}

		//determine la plage de données a lire selon les date de debut et fin de la simulation
		
		//date debut
		//met heure debut à 0; //si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		
		_netCdf_dateDebutVecteur = DATE_HEURE(_dtDebutSim.PrendreAnnee(), _dtDebutSim.PrendreMois(), _dtDebutSim.PrendreJour(), 0);

		if(_netCdf_dateDebutVecteur < dtDebutFichier)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": data missing for simulation begin date";
			nc_close(_netCdf_fileId);
			return false;
		}

		_indexDebut = dtDebutFichier.NbHeureEntre(_netCdf_dateDebutVecteur) / _netCdf_lPasTemps;

		//date fin
		dt = DATE_HEURE(_dtFinSim.PrendreAnnee(), _dtFinSim.PrendreMois(), _dtFinSim.PrendreJour(), 0); 		
		if(_simTimeStep != 24)
			dt.AdditionHeure(24);	//si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		

		if(dt > dtFinFichier)
		{
			_sError = "Error reading NetCDF file: " + sFilename + ": data missing for simulation end date";
			nc_close(_netCdf_fileId);
			return false;
		}

		indexFin = dtDebutFichier.NbHeureEntre(dt) / _netCdf_lPasTemps;

		//heure lu en fin de pas de temps et remise en debut de pas de temps.
		if(_simTimeStep != 24)
			_indexDebut = _indexDebut + 1;

		_netCdf_lNbPasTemps = indexFin - _indexDebut + 1;

		_sFilename = sFilename;

		return true;
	}
	
	
	//-------------------------------------------------------------
	bool NETCDF_FILE::ReadData(string sVariableName, float* pData)
	{
		//lit et conserve les donnees en ram
		int ret, varId;

		ret = nc_inq_varid(_netCdf_fileId, sVariableName.c_str(), &varId);
		if(ret != NC_NOERR)
		{
			_sError = "Error reading NetCDF file: " + _sFilename + ": variable `" + sVariableName + "` not found";
			return false;
		}

		if(_netCdf_iType == 1) //GRID
		{
			size_t start[] = { _indexDebut, 0, 0 };	//depth, row, col	//time, y, x
			size_t count[] = { _netCdf_lNbPasTemps, _netCdf_lNbLat, _netCdf_lNbLong };

			ret = nc_get_vara_float(_netCdf_fileId, varId, start, count, &pData[0]);
		}
		else
		{
			//STATION
			size_t start[] = { _indexDebut, 0 };	//time, id
			size_t count[] = { _netCdf_lNbPasTemps, _netCdf_lNbStation };

			ret = nc_get_vara_float(_netCdf_fileId, varId, start, count, &pData[0]);
		}

		if(ret != NC_NOERR)
		{
			ostringstream oss;
			oss << ret;
			_sError = "Error reading NetCDF file: " + _sFilename + ": error reading `" + sVariableName + "` data: error code " + oss.str();
			return false;
		}

		return true;
	}


}
