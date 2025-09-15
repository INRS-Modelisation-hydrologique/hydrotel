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

#include "stations_meteo.hpp"

#include "station_meteo_gibsi.hpp"
#include "station_meteo_hdf5.hpp"
#include "station_meteo_netcdf_station.hpp"
#include "constantes.hpp"
#include "projections.hpp"
#include "util.hpp"
#include "erreur.hpp"
#include "transforme_coordonnee.hpp"
#include "sim_hyd.hpp"

#include <fstream>
#include <map>
#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/shared_array.hpp>


using namespace std;


namespace HYDROTEL
{

	STATIONS_METEO::STATIONS_METEO()
		: _bAutoInverseTMinTMax(false)
		, _bStationInterpolation(true)
		, _hdid(-1)
		, _dataset_tmin(-1)
		, _dataspace_tmin(-1)
		, _rank_tmin(-1)
		, _memspace_tmin(-1)
	{
		_pSimHyd = nullptr;

		_netCdf_iType = -1;

		_netCdf_lPasTemps = 0;
		_netCdf_lNbPasTemps = 0;
		_netCdf_lNbStations = 0;
			
		_netCdf_dataStationPrecip = NULL;
		_netCdf_dataStationTMin = NULL;
		_netCdf_dataStationTMax = NULL;

		_fGradientStationTemp = -999.0f;
		_fGradientStationPrecip = -999.0f;
	}

	STATIONS_METEO::~STATIONS_METEO()
	{
		if(_netCdf_dataStationPrecip != NULL)
		{
			delete [] _netCdf_dataStationPrecip;
			delete [] _netCdf_dataStationTMin;
			delete [] _netCdf_dataStationTMax;
		}

		/*
        if (_memspace_tmin != -1)
		{
			herr_t status = H5Sclose(_memspace_tmin);
			if (status < 0)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier);
		}

        if (_dataspace_tmin != -1)
		{
			herr_t status = H5Sclose(_dataspace_tmin);
			if (status < 0)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier);
		}

        if (_dataset_tmin != -1)
		{
			herr_t status = H5Dclose(_dataset_tmin);
			if (status < 0)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier);
		}
		*/

		if(_hdid != -1)
		{
			herr_t status = H5Fclose(_hdid);
			if(status < 0)
				std::cout << "Error ~STATIONS_METEO H5Fclose(): " << _nom_fichier << endl;
		}
	}

	void STATIONS_METEO::Lecture(const PROJECTION& projection)
	{
		size_t i;
		string str;

		Detruire();

		_ProjectionProjet._spatial_reference.importFromProj4(projection.ExportProj4().c_str());

		string extension = PrendreExtension(PrendreNomFichier());

		if (extension == ".h5")
		{
			LectureFormatHDF5();
		}
		else if (extension == ".nc")
		{
			str = LectureExtentLimit();
			if (str != "")
				throw ERREUR(str);

			str = LectureFormatNetCDFConfig();
			if(str != "")
				throw ERREUR(str);

			switch (_netCdf_iType)
			{
			case 0:
				LectureFormatNetCDFTypeStation();
				_pSimHyd->_outputCDF = true;
				break;
			
			case 1:
				LectureFormatNetCDFTypeGrid();
				_pSimHyd->_outputCDF = true;
				break;

			default:
				throw ERREUR("Lecture NetCDF: parametre type invalide");
			}
			
		}
		else if (extension == ".stm")
		{
			LectureFormatSTM(projection);
		}
		else
		{
			throw ERREUR_LECTURE_FICHIER( PrendreNomFichier() );
		}

		//convert stations coordinate from CRS of data file to project CRS
		TRANSFORME_COORDONNEE trans_coord(PrendreProjection(),  _ProjectionProjet);

		for(i=0; i!=_stations.size(); i++)
			_stations[i].get()->_coordonneeCRSprojet = trans_coord.TransformeXYZ(_stations[i].get()->PrendreCoordonnee());

		CreeMapRecherche();
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string STATIONS_METEO::LectureExtentLimit()
	{
		vector<string> valeurs;
		string sPathFile, str, ligne;
		bool bEmptyFile;

		sPathFile = "extent-limit.config";
		sPathFile = Combine(PrendreRepertoire(_nom_fichier), sPathFile);

		_dExtentLimitNorth = -1.0;
		_dExtentLimitSouth = -1.0;
		_dExtentLimitEast = -1.0;
		_dExtentLimitWest = -1.0;

		bEmptyFile = false;

		if(FichierExiste(sPathFile))
		{
			ifstream fichier(sPathFile);
			if(!fichier)
			{
				str = "Erreur ouverture fichier: " + sPathFile;
				return str;
			}

			bEmptyFile = true;

			try
			{
				while(!fichier.eof() || fichier.bad())
				{
					ligne = "";
					getline_mod(fichier, ligne);
					ligne = TrimString(ligne);

					if (ligne.size() > 1 && ligne[0] == '/' && ligne[1] == '/')
						ligne = "";

					if(ligne != "")
					{
						SplitString(valeurs, ligne, ";", true, true);

						if(valeurs.size() == 2)
						{
							bEmptyFile = false;

							str = TrimString(valeurs[0]);
							boost::algorithm::to_upper(str);

							istringstream iss(valeurs[1]);

							if(str == "NORTH")
								iss >> _dExtentLimitNorth;
							else
							{
								if(str == "SOUTH")
									iss >> _dExtentLimitSouth;
								else
								{
									if(str == "EAST")
										iss >> _dExtentLimitEast;
									else
									{
										if(str == "WEST")
											iss >> _dExtentLimitWest;
										else
										{
											fichier.close();
											str = "Erreur lecture fichier: " + sPathFile + ": ligne invalide: " + ligne;
											return str;
										}
									}
								}
							}
						}
					}
				}
			}
			catch(...)
			{
				if(fichier && fichier.is_open())
					fichier.close();
				str = "Erreur lecture fichier: " + sPathFile;
				return str;
			}

			fichier.close();

			if (_dExtentLimitNorth != -1.0 || _dExtentLimitSouth != -1.0 || _dExtentLimitEast != -1.0 || _dExtentLimitWest != -1.0)
			{
				if (_dExtentLimitNorth == -1.0 || _dExtentLimitSouth == -1.0 || _dExtentLimitEast == -1.0 || _dExtentLimitWest == -1.0)
				{
					_dExtentLimitNorth = -1.0;
					_dExtentLimitSouth = -1.0;
					_dExtentLimitEast = -1.0;
					_dExtentLimitWest = -1.0;

					str = "Erreur lecture fichier: " + sPathFile + ": extent invalide";
					return str;
				}
			}
		}

		if(bEmptyFile)
			str = "Erreur lecture fichier: " + sPathFile + ": le fichier est vide ou le format est invalide";
		else
			str = "";

		return str;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string STATIONS_METEO::LectureFormatNetCDFConfig()
	{		
		vector<string> valeurs; 
		string sPathFile, str, ligne;
		
		sPathFile = _nom_fichier + ".config";

		if (!FichierExiste(sPathFile))
		{
			str = "Erreur; fichier introuvable; " + sPathFile;
			return str;
		}

		ifstream fichier(sPathFile);
		if (!fichier)
		{
			str = "Erreur ouverture fichier; " + sPathFile;
			return str;
		}

		try
		{
			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			boost::algorithm::to_upper(str);

			if (str == "STATION")
				_netCdf_iType = 0;
			else
			{
				if (str == "GRID")
					_netCdf_iType = 1;
				else
				{
					fichier.close();
					str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne + "; type invalide";
					return str;
				}
			}

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);

			if (valeurs.size() == 2)
			{
				str = TrimString(valeurs[1]);
				if (str == "")
				{
					fichier.close();
					str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
					return str;
				}

				_netCdf_sStationDim = str;
			}
			else
			{
				if(_netCdf_iType == 0)	//la variable STATION_DIM_NAME est necessaire seulement pour type == 0 (STATION)
				{
					fichier.close();
					str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
					return str;
				}
			}

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			if (str == "")
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}

			_netCdf_sLat = str;

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			if (str == "")
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}

			_netCdf_sLon = str;

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			if (str == "")
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}

			_netCdf_sAlt = str;

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			if (str == "")
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}

			_netCdf_sTime = str;

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			if (str == "")
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}

			_netCdf_sTMin = str;

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			if (str == "")
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}

			_netCdf_sTMax = str;

			//
			ligne = "";
			while (ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			SplitString(valeurs, ligne, ";", true, false);
			if (valeurs.size() != 2)
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}
			str = TrimString(valeurs[1]);
			if (str == "")
			{
				fichier.close();
				str = "Erreur lecture fichier; " + sPathFile + "; ligne; " + ligne;
				return str;
			}

			_netCdf_sPrecip = str;
		}

		catch (...)
		{
			fichier.close();
			str = "Erreur lecture fichier; " + sPathFile;
			return str;
		}

		fichier.close();
		return "";
	}


	//ret = nc_inq_var(_ncid, latid, 0, &rh_type, &rh_ndims, rh_dimids, &rh_natts);
	//ret = nc_inq(_ncid, &ndims, &nvars, &ngatts, &unlimdimid);


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Pour type == STATION
	//Format H2.1

	void STATIONS_METEO::LectureFormatNetCDFTypeStation()
	{
		unsigned short yy, mm, dd, hh, min, ss;
		istringstream iss;
		ostringstream oss;
		DATE_HEURE dtDebutFichier;
		DATE_HEURE dtFinFichier;
		DATE_HEURE dtTimeUnit;
		DATE_HEURE dt;
		size_t lNbPasTempsFichier, i, j, indexDebut, indexFin;
		double dVal;
		string str1, str2, str3;
		bool bMinutesUnit;
		int iNcid, ret, iVal;
		int latid, lonid, elvid, timedimid, timeid, iStationDimID, precipid, tminid, tmaxid;

		oss.str("");

		ret = nc_open(_nom_fichier.c_str(), NC_NOWRITE, &iNcid);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error opening NetCDF file: " + _nom_fichier + ": nc_open return code " + oss.str() + ".");
		}

		//valide l'unité des pas de temps
		//l'unité doit etre "days since yyyy-mm-dd hh:00:00" ou "minutes since yyyy-mm-dd hh:00:00"
		ret = nc_inq_varid(iNcid, _netCdf_sTime.c_str(), &timeid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time variable `" + _netCdf_sTime.c_str() + "` not found.");

		ret = nc_inq_attlen(iNcid, timeid, "units", &i);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time variable `" + _netCdf_sTime.c_str() + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`.");

		boost::shared_array<char> str_att(new char[i+1]);
		ret = nc_get_att(iNcid, timeid, "units", reinterpret_cast<void*>(&str_att[0]));
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time variable `" + _netCdf_sTime.c_str() + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`.");

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
				throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid time units: minutes and seconds must be 0.");

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
					throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid time units: minutes and seconds must be 0.");

				dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
			}
			else
				throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid time units: must be `minutes since yyyy-mm-dd hh:00:00` or `days since yyyy-mm-dd hh:00:00`" + ".");
		}

		//lecture nb station et variable ids
		ret = nc_inq_dimid(iNcid, _netCdf_sStationDim.c_str(), &iStationDimID);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": station dimension `" + _netCdf_sStationDim.c_str() + "` not found.");

		ret = nc_inq_dimlen(iNcid, iStationDimID, &_netCdf_lNbStations);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading station dimension length: error code " + oss.str() + ".");
		}

		ret = nc_inq_varid(iNcid, _netCdf_sLat.c_str(), &latid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": latitude variable `" + _netCdf_sLat.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sLon.c_str(), &lonid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": longitude variable `" + _netCdf_sLon.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sAlt.c_str(), &elvid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": elevation variable `" + _netCdf_sAlt.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sPrecip.c_str(), &precipid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": precip variable `" + _netCdf_sPrecip.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sTMin.c_str(), &tminid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": tmin variable `" + _netCdf_sTMin.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sTMax.c_str(), &tmaxid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": tmax variable `" + _netCdf_sTMax.c_str() + "` not found.");

		//lecture des coordonnees et elevations
		_projection = PROJECTIONS::LONGLAT_WGS84();

		vector<double> latitudes(_netCdf_lNbStations);
		ret = nc_get_var_double(iNcid, latid, &latitudes[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading latitude data: error code " + oss.str() + ".");
		}

		vector<double> longitudes(_netCdf_lNbStations);
		ret = nc_get_var_double(iNcid, lonid, &longitudes[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading longitude data: error code " + oss.str() + ".");
		}

		vector<double> elevations(_netCdf_lNbStations);
		ret = nc_get_var_double(iNcid, elvid, &elevations[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading elevation data: error code " + oss.str() + ".");
		}

		//lecture des pas de temps
		ret = nc_inq_dimid(iNcid, _netCdf_sTime.c_str(), &timedimid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time dimension `" + _netCdf_sTime.c_str() + "` not found.");

		ret = nc_inq_dimlen(iNcid, timedimid, &lNbPasTempsFichier);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading time dimension length: error code " + oss.str() + ".");
		}

		if(lNbPasTempsFichier < 2)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid timestep count.");

		vector<double> dTimes;
		vector<int> iTimes;
		
		if(bMinutesUnit)
		{
			iTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_int(iNcid, timeid, &iTimes[0]);
		}
		else
		{
			dTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_double(iNcid, timeid, &dTimes[0]);
		}

		if(ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading time data: error code " + oss.str() + ".");
		}

		//determine dates debut et fin du fichier
		if(bMinutesUnit)
			_netCdf_lPasTemps = static_cast<size_t>((iTimes[1] - iTimes[0]) / 60);		//time step [hrs]
		else
			_netCdf_lPasTemps = static_cast<size_t>((dTimes[1] - dTimes[0]) * 24.0);	//

		if(!bMinutesUnit && (_netCdf_lPasTemps != _pSimHyd->_pas_de_temps))	//only for days unit because of double precision
		{
			//on passe d'une valeur en jours (epoch time) vers une valeur en heures
			//essaie d'ajouter ou d'enlever 1 sec pour corriger les problemes de précision des réels lors de la conversion
			dVal = (dTimes[1] - dTimes[0]) * 24.0 + 0.000011574074074074074074074074074074;	//ajoute 1 sec
			_netCdf_lPasTemps = static_cast<size_t>(dVal);

			if(_netCdf_lPasTemps != _pSimHyd->_pas_de_temps)
			{
				dVal = (dTimes[1] - dTimes[0]) * 24.0 - 0.000011574074074074074074074074074074;	//enleve 1 sec
				_netCdf_lPasTemps = static_cast<size_t>(dVal);
			}
		}

		if (_netCdf_lPasTemps != _pSimHyd->_pas_de_temps)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": timestep of data must be equal to simulation timestep.");

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
		_netCdf_dateDebutVecteur = DATE_HEURE(_pSimHyd->_date_debut.PrendreAnnee(), _pSimHyd->_date_debut.PrendreMois(), _pSimHyd->_date_debut.PrendreJour(), 0);

		if (_netCdf_dateDebutVecteur < dtDebutFichier)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": data missing for simulation begin date.");

		indexDebut = dtDebutFichier.NbHeureEntre(_netCdf_dateDebutVecteur) / _netCdf_lPasTemps;

		//date fin
		dt = DATE_HEURE(_pSimHyd->_date_fin.PrendreAnnee(), _pSimHyd->_date_fin.PrendreMois(), _pSimHyd->_date_fin.PrendreJour(), 0); 		
		if (_pSimHyd->_pas_de_temps != 24)
			dt.AdditionHeure(24);	//si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		

		if (dt > dtFinFichier)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": data missing for simulation end date.");

		indexFin = dtDebutFichier.NbHeureEntre(dt) / _netCdf_lPasTemps;

		//heure lu en fin de pas de temps et remise en debut de pas de temps.
		if (_pSimHyd->_pas_de_temps != 24)
			indexDebut = indexDebut + 1;

		_netCdf_lNbPasTemps = indexFin - indexDebut + 1;

		//lit et conserve les donnees en ram		
		size_t start[] = { indexDebut, 0 };	//row, col
		size_t count[] = { _netCdf_lNbPasTemps, _netCdf_lNbStations };	//row, col

		_netCdf_dataStationPrecip = new float[_netCdf_lNbPasTemps*_netCdf_lNbStations];
		_netCdf_dataStationTMin = new float[_netCdf_lNbPasTemps*_netCdf_lNbStations];
		_netCdf_dataStationTMax = new float[_netCdf_lNbPasTemps*_netCdf_lNbStations];

		ret = nc_get_vara_float(iNcid, precipid, start, count, &_netCdf_dataStationPrecip[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading precip data: error code " + oss.str() + ".");
		}

		ret = nc_get_vara_float(iNcid, tminid, start, count, &_netCdf_dataStationTMin[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading tmin data: error code " + oss.str() + ".");
		}

		ret = nc_get_vara_float(iNcid, tmaxid, start, count, &_netCdf_dataStationTMax[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading tmax data: error code " + oss.str() + ".");
		}

		//initialisation des objets station
		_stations.clear();

		j = 0;
		for (i=0; i<_netCdf_lNbStations; i++)
		{
			//pour cadrant nord/west
			if (_dExtentLimitNorth == -1.0 || 
				(latitudes[i] <= _dExtentLimitNorth && latitudes[i] >= _dExtentLimitSouth &&
				 longitudes[i] <= _dExtentLimitEast && longitudes[i] >= _dExtentLimitWest) )
			{
				shared_ptr<STATION> st = make_shared<STATION_METEO_NETCDF_STATION>(_nom_fichier, this, i);
				if(_pSimHyd->PrendreNomInterpolationDonnees() == "THIESSEN1" || _pSimHyd->PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS1")
					st.get()->_iVersionThiessenMoy3Station = 1;

				oss.str("");
				oss << "station" << j + 1;

				st->ChangeNom(oss.str());
				st->ChangeIdent(oss.str());
				st->ChangeCoordonnee(COORDONNEE(longitudes[i], latitudes[i], elevations[i]));

				_stations.push_back(st);
				++j;
			}
		}

		ret = nc_close(iNcid);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error closing NetCDF file: " + _nom_fichier + ": nc_close error code " + oss.str() + ".");
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Pour type == GRID
	//Format 9.3.1

	void STATIONS_METEO::LectureFormatNetCDFTypeGrid()
	{
		unsigned short yy, mm, dd, hh, min, ss;
		istringstream iss;
		ostringstream oss;
		DATE_HEURE dtDebutFichier;
		DATE_HEURE dtFinFichier;
		DATE_HEURE dtTimeUnit;
		DATE_HEURE dt;
		size_t lNbPasTempsFichier, i, j, k, indexDebut, indexFin;
		double dVal;
		string str1, str2, str3;
		bool bMinutesUnit;
		int iNcid, ret, iVal;
		int latid, lonid, elvid, timedimid, timeid, iLatDimID, iLonDimID, precipid, tminid, tmaxid;

		oss.str("");

		ret = nc_open(_nom_fichier.c_str(), NC_NOWRITE, &iNcid);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error opening NetCDF file: " + _nom_fichier + ": nc_open return code " + oss.str() + ".");
		}

		//valide l'unité des pas de temps
		//l'unité doit etre "days since yyyy-mm-dd hh:00:00" ou "minutes since yyyy-mm-dd hh:00:00"
		ret = nc_inq_varid(iNcid, _netCdf_sTime.c_str(), &timeid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time variable `" + _netCdf_sTime.c_str() + "` not found.");

		ret = nc_inq_attlen(iNcid, timeid, "units", &i);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time variable `" + _netCdf_sTime.c_str() + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`.");

		boost::shared_array<char> str_att(new char[i+1]);
		ret = nc_get_att(iNcid, timeid, "units", reinterpret_cast<void*>(&str_att[0]));
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time variable `" + _netCdf_sTime.c_str() + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`.");

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
				throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid time units: minutes and seconds must be 0.");

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
					throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid time units: minutes and seconds must be 0.");

				dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
			}
			else
				throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid time units: must be `minutes since yyyy-mm-dd hh:00:00` or `days since yyyy-mm-dd hh:00:00`" + ".");
		}

		//lecture longitude et latitude dimensions
		ret = nc_inq_dimid(iNcid, _netCdf_sLat.c_str(), &iLatDimID);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": latitude dimension `" + _netCdf_sLat.c_str() + "` not found.");

		ret = nc_inq_dimlen(iNcid, iLatDimID, &_netCdf_lNbLat);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading latitude dimension length: error code " + oss.str() + ".");
		}

		ret = nc_inq_dimid(iNcid, _netCdf_sLon.c_str(), &iLonDimID);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": longitude dimension `" + _netCdf_sLon.c_str() + "` not found.");

		ret = nc_inq_dimlen(iNcid, iLonDimID, &_netCdf_lNbLong);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading longitude dimension length: error code " + oss.str() + ".");
		}

		_netCdf_lNbCoord = _netCdf_lNbLat * _netCdf_lNbLong;
		
		//lecture variables ids
		ret = nc_inq_varid(iNcid, _netCdf_sLat.c_str(), &latid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": latitude variable `" + _netCdf_sLat.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sLon.c_str(), &lonid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": longitude variable `" + _netCdf_sLon.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sAlt.c_str(), &elvid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": elevation variable `" + _netCdf_sAlt.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sPrecip.c_str(), &precipid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": precip variable `" + _netCdf_sPrecip.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sTMin.c_str(), &tminid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": tmin variable `" + _netCdf_sTMin.c_str() + "` not found.");

		ret = nc_inq_varid(iNcid, _netCdf_sTMax.c_str(), &tmaxid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": tmax variable `" + _netCdf_sTMax.c_str() + "` not found.");

		//lecture des coordonnees et elevations
		_projection = PROJECTIONS::LONGLAT_WGS84();

		vector<double> latitudes(_netCdf_lNbLat);
		ret = nc_get_var_double(iNcid, latid, &latitudes[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading latitude data: error code " + oss.str() + ".");
		}

		vector<double> longitudes(_netCdf_lNbLong);
		ret = nc_get_var_double(iNcid, lonid, &longitudes[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading longitude data: error code " + oss.str() + ".");
		}

		double* elevations = NULL;
		size_t start2[] = { 0, 0 };	//row, col	//y, x
		size_t count2[] = { _netCdf_lNbLat, _netCdf_lNbLong };

		elevations = new double[_netCdf_lNbCoord];

		ret = nc_get_vara_double(iNcid, elvid, start2, count2, &elevations[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading elevation data: error code " + oss.str() + ".");
		}

		//lecture des pas de temps
		ret = nc_inq_dimid(iNcid, _netCdf_sTime.c_str(), &timedimid);
		if (ret != NC_NOERR)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": time dimension `" + _netCdf_sTime.c_str() + "` not found.");

		ret = nc_inq_dimlen(iNcid, timedimid, &lNbPasTempsFichier);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading time dimension length: error code " + oss.str() + ".");
		}

		if(lNbPasTempsFichier < 2)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": invalid timestep count.");

		vector<double> dTimes;
		vector<int> iTimes;

		if(bMinutesUnit)
		{
			iTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_int(iNcid, timeid, &iTimes[0]);
		}
		else
		{
			dTimes.resize(lNbPasTempsFichier);
			ret = nc_get_var_double(iNcid, timeid, &dTimes[0]);
		}
		
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading time data: error code " + oss.str() + ".");
		}

		//determine dates debut et fin du fichier
		if(bMinutesUnit)
			_netCdf_lPasTemps = static_cast<size_t>((iTimes[1] - iTimes[0]) / 60);		//time step [hrs]
		else
			_netCdf_lPasTemps = static_cast<size_t>((dTimes[1] - dTimes[0]) * 24.0);	//

		if(!bMinutesUnit && (_netCdf_lPasTemps != _pSimHyd->_pas_de_temps))	//only for days unit because of double precision
		{
			//on passe d'une valeur en jours (epoch time) vers une valeur en heures
			//essaie d'ajouter ou d'enlever 1 sec pour corriger les problemes de précision des réels lors de la conversion
			dVal = (dTimes[1] - dTimes[0]) * 24.0 + 0.000011574074074074074074074074074074;	//ajoute 1 sec
			_netCdf_lPasTemps = static_cast<size_t>(dVal);

			if (_netCdf_lPasTemps != _pSimHyd->_pas_de_temps)
			{
				dVal = (dTimes[1] - dTimes[0]) * 24.0 - 0.000011574074074074074074074074074074;	//enleve 1 sec
				_netCdf_lPasTemps = static_cast<size_t>(dVal);
			}
		}

		if (_netCdf_lPasTemps != _pSimHyd->_pas_de_temps)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": timestep of data must be equal to simulation timestep.");

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
		_netCdf_dateDebutVecteur = DATE_HEURE(_pSimHyd->_date_debut.PrendreAnnee(), _pSimHyd->_date_debut.PrendreMois(), _pSimHyd->_date_debut.PrendreJour(), 0);

		if (_netCdf_dateDebutVecteur < dtDebutFichier)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": data missing for simulation begin date.");

		indexDebut = dtDebutFichier.NbHeureEntre(_netCdf_dateDebutVecteur) / _netCdf_lPasTemps;

		//date fin
		dt = DATE_HEURE(_pSimHyd->_date_fin.PrendreAnnee(), _pSimHyd->_date_fin.PrendreMois(), _pSimHyd->_date_fin.PrendreJour(), 0); 		
		if (_pSimHyd->_pas_de_temps != 24)
			dt.AdditionHeure(24);	//si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		

		if (dt > dtFinFichier)
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": data missing for simulation end date.");

		indexFin = dtDebutFichier.NbHeureEntre(dt) / _netCdf_lPasTemps;

		//heure lu en fin de pas de temps et remise en debut de pas de temps.
		if (_pSimHyd->_pas_de_temps != 24)
			indexDebut = indexDebut + 1;

		_netCdf_lNbPasTemps = indexFin - indexDebut + 1;

		//lit et conserve les donnees en ram
		size_t start[] = { indexDebut, 0, 0 };	//depth, row, col	//time, y, x
		size_t count[] = { _netCdf_lNbPasTemps, _netCdf_lNbLat, _netCdf_lNbLong };

		_netCdf_dataStationPrecip = new float[_netCdf_lNbPasTemps*_netCdf_lNbCoord];
		_netCdf_dataStationTMin = new float[_netCdf_lNbPasTemps*_netCdf_lNbCoord];
		_netCdf_dataStationTMax = new float[_netCdf_lNbPasTemps*_netCdf_lNbCoord];

		ret = nc_get_vara_float(iNcid, precipid, start, count, &_netCdf_dataStationPrecip[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading precip data: error code " + oss.str() + ".");
		}

		ret = nc_get_vara_float(iNcid, tminid, start, count, &_netCdf_dataStationTMin[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading tmin data: error code " + oss.str() + ".");
		}

		ret = nc_get_vara_float(iNcid, tmaxid, start, count, &_netCdf_dataStationTMax[0]);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error reading NetCDF file: " + _nom_fichier + ": error reading tmax data: error code " + oss.str() + ".");
		}

		//initialisation des objets station
		_stations.clear();

		k = 0;
		for (i=0; i<_netCdf_lNbLat; i++)
		{
			for (j=0; j<_netCdf_lNbLong; j++)
			{
				//pour cadrant nord/west
				if (_dExtentLimitNorth == -1.0 || 
					(latitudes[i] <= _dExtentLimitNorth && latitudes[i] >= _dExtentLimitSouth &&
						longitudes[j] <= _dExtentLimitEast && longitudes[j] >= _dExtentLimitWest) )
				{
					shared_ptr<STATION> st = make_shared<STATION_METEO_NETCDF_STATION>(_nom_fichier, this, i, j);
					if(_pSimHyd->PrendreNomInterpolationDonnees() == "THIESSEN1" || _pSimHyd->PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS1")
						st.get()->_iVersionThiessenMoy3Station = 1;

					oss.str("");
					oss << "station" << k + 1;

					st->ChangeNom(oss.str());
					st->ChangeIdent(oss.str());
					st->ChangeCoordonnee(COORDONNEE(longitudes[j], latitudes[i], elevations[i * _netCdf_lNbLong + j]));

					_stations.push_back(st);
					++k;
				}
			}
		}

		ret = nc_close(iNcid);
		if (ret != NC_NOERR)
		{
			oss << ret;
			throw ERREUR("Error closing NetCDF file: " + _nom_fichier + ": nc_close error code " + oss.str() + ".");
		}
	}


	//ANCIENNES FONCTIONS

	void STATIONS_METEO::LectureFormatHDF5()
	{
		_hdid = H5Fopen(_nom_fichier.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

		hid_t elevation_dataset = H5Dopen(_hdid, "/meteo/elevation", H5P_DEFAULT);

		hid_t dataspace = H5Dget_space(elevation_dataset);
		int rank = H5Sget_simple_extent_ndims(dataspace);
		vector<hsize_t> dims_out(rank);
		int status_n = H5Sget_simple_extent_dims(dataspace, &dims_out[0], nullptr);
		if (status_n < 0)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		size_t nb_station = static_cast<size_t>(dims_out[0]);
		vector<float> elevation(nb_station);

		herr_t status = H5Dread(elevation_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &elevation[0]);
		if (status < 0)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);
		H5Dclose(elevation_dataset);

		hid_t latitude_dataset = H5Dopen(_hdid, "/meteo/latitude", H5P_DEFAULT);
		vector<float> latitude(nb_station);
		status = H5Dread(latitude_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &latitude[0]);
		if (status < 0)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);
		H5Dclose(latitude_dataset);

		hid_t longitude_dataset = H5Dopen(_hdid, "/meteo/longitude", H5P_DEFAULT);
		vector<float> longitude(nb_station);
		status = H5Dread(longitude_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &longitude[0]);
		if (status < 0)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);
		H5Dclose(longitude_dataset);

		_stations.resize(nb_station);

		_projection = PROJECTIONS::LONGLAT_WGS84();

        // NOTE: buffer pour les differents dataspace, memspace et dataset

		hsize_t dimsm[2] = { 1, 1 };

		{
		    _dataset_tmin = static_cast<int>(H5Dopen(_hdid, "meteo/tmin", H5P_DEFAULT));
		    _dataspace_tmin = static_cast<int>(H5Dget_space(_dataset_tmin));
		    rank = H5Sget_simple_extent_ndims(_dataspace_tmin);	
		    _memspace_tmin = static_cast<int>(H5Screate_simple(rank, dimsm, NULL));
		}

		{
		    _dataset_tmax = static_cast<int>(H5Dopen(_hdid, "meteo/tmax", H5P_DEFAULT));
		    _dataspace_tmax = static_cast<int>(H5Dget_space(_dataset_tmax));
		    rank = H5Sget_simple_extent_ndims(_dataspace_tmax);	
		    _memspace_tmax = static_cast<int>(H5Screate_simple(rank, dimsm, NULL));
		}

		{
		    _dataset_pr = static_cast<int>(H5Dopen(_hdid, "meteo/pr", H5P_DEFAULT));
		    _dataspace_pr = static_cast<int>(H5Dget_space(_dataset_pr));
		    rank = H5Sget_simple_extent_ndims(_dataspace_pr);	
		    _memspace_pr = static_cast<int>(H5Screate_simple(rank, dimsm, NULL));
		}


		for (unsigned int index = 0; index < nb_station; ++index)
		{
			auto station = make_shared<STATION_METEO_HDF5>(_hdid, _nom_fichier, index);
			if(_pSimHyd->PrendreNomInterpolationDonnees() == "THIESSEN1" || _pSimHyd->PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS1")
				station.get()->_iVersionThiessenMoy3Station = 1;

			ostringstream ss;
			ss << index + 1;

			station->ChangeNom(ss.str());
			station->ChangeIdent(ss.str());
			station->ChangeCoordonnee(COORDONNEE(longitude[index], latitude[index], elevation[index]));

			station->ChangeInfoHDF5(
				_dataset_tmin, _dataspace_tmin, _memspace_tmin,
				_dataset_tmax, _dataspace_tmax, _memspace_tmax,
				_dataset_pr, _dataspace_pr, _memspace_pr);

			_stations[index] = station;
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Il ne doit pas y avoir de ligne vide dans le fichier, excepté pour le commentaire.
	void STATIONS_METEO::LectureFormatSTM(const PROJECTION& projection)
	{
		if(!FichierExiste(_nom_fichier))
			return;

		ifstream fichier(_nom_fichier);
		if (!fichier)
		{
			string str = "STATIONS_METEO; erreur lecture fichier; ";
			str+= _nom_fichier;
			throw ERREUR(str);
		}

		vector<std::string> valeurs;
		istringstream iss;
		double x, y, z;
		string id, ligne;
		size_t nb_station, n, nbLu;
		int format, type;
		
		nbLu = 0;

		try
		{
			ligne = "";
			while(ligne == "")	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			iss.clear();
			iss.str(ligne);
			iss >> type;					//type; 1=long/lat wgs84 //2=meme sys. coord. que le dem (ex: UTM)

			ligne = "";
			while(ligne == "")
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}
			iss.clear();
			iss.str(ligne);
			iss >> nb_station;				//nb_station

			ligne = "";
			while(ligne == "")
			{
				getline_mod(fichier, ligne);	//commentaire
				ligne = TrimString(ligne);
			}

			switch (type)
			{
			case 1: // long/lat wgs84
				_projection = PROJECTIONS::LONGLAT_WGS84();
				break;

			case 2: // meme format de la grille
				_projection = projection;
				break;

			default:
				ligne = "STATIONS_METEO; erreur lecture fichier; ";
				ligne+= _nom_fichier;
				ligne+= "; type invalide.";
				throw ERREUR(ligne);
			}

			vector<shared_ptr<STATION>> stations(nb_station);

			for (n = 0; n < nb_station; ++n)
			{
				ligne = "";
				while(ligne == "")	//ignore les lignes vides s'il y en a
				{
					getline_mod(fichier, ligne);
					ligne = TrimString(ligne);
				}

				SplitString(valeurs, ligne, " \t", true, true);	//separateur; espace et \t
				if(valeurs.size() < 5)
				{
					ligne = "STATIONS_METEO; erreur lecture fichier; ";
					ligne+= _nom_fichier;
					ligne+= "; le nb de colonne est invalide.";
					throw ERREUR(ligne);
				}

				if(type == 1)	//long/lat
				{
					x = ParseLatLongCoord(valeurs[1], true);
					y = ParseLatLongCoord(valeurs[2], false);
				}
				else
				{
					iss.clear();
					iss.str((valeurs[1]));
					iss >> x;

					iss.clear();
					iss.str((valeurs[2]));
					iss >> y;
				}

				iss.clear();
				iss.str((valeurs[4]));
				iss >> format;

				switch (format)
				{
				case 3:	// FORMAT GIBSI

					ligne = valeurs[0];
					ligne.append(".met");
					ligne = Combine(PrendreRepertoire(_nom_fichier), ligne);

					stations[n] = make_shared<STATION_METEO_GIBSI>(ligne, _bAutoInverseTMinTMax);
					if(_pSimHyd->_interpolation_donnees == nullptr || //_pSimHyd->_interpolation_donnees is null when program option -update
						(_pSimHyd->PrendreNomInterpolationDonnees() == "THIESSEN1" || _pSimHyd->PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS1"))
					{
						stations[n].get()->_iVersionThiessenMoy3Station = 1;
					}
					break;

				default:
					ligne = "STATIONS_METEO; erreur lecture fichier; ";
					ligne+= _nom_fichier;
					ligne+= "; numero de format invalide.";
					throw ERREUR(ligne);
				}

				stations[n].get()->ChangeIdent(valeurs[0]);
				stations[n].get()->ChangeNom(valeurs[0]);

				iss.clear();
				iss.str((valeurs[3]));
				iss >> z;

				stations[n].get()->ChangeCoordonnee(COORDONNEE(x, y, z));

				++nbLu;
			}

			fichier.close();

			if(nbLu != nb_station)
            {
                ligne = "STATIONS_METEO; erreur lecture fichier; ";
				ligne+= _nom_fichier;
				ligne+= "; nombre de station invalide.";
				throw ERREUR(ligne);
            }

			_stations.swap(stations);			
		}
		
		catch(const ERREUR& err)
		{
			fichier.close();
			throw ERREUR(err.what());
		}

		catch(...)
		{
			fichier.close();
			string str = "STATIONS_METEO; erreur lecture fichier; ";
			str+= _nom_fichier;
			throw ERREUR(str);
		}
	}


	//********************************************************************************************************************
	//LectureDonnees_v1 used by moyenne3station v1 and thiessen v1 models
	void STATIONS_METEO::LectureDonnees_v1(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps)
	{
		// NOTE: on doit lire au moins une journee complete afin d'avoir les temperatures journalieres

		shared_ptr<STATION_METEO> station, station2;
		STATION_METEO* station1;
		DONNEE_METEO donnee, donnee1, donnee2;
		DATE_HEURE date_courante;
		vector<size_t> distances;
		size_t index;
		double diff_alt;
		float tmin, tmax, pluie;
		bool done;

		DATE_HEURE date_debut(debut.PrendreAnnee(), debut.PrendreMois(), debut.PrendreJour(), 0);
		DATE_HEURE date_fin(fin.PrendreAnnee(), fin.PrendreMois(), fin.PrendreJour(), 0);

		date_fin.AdditionHeure(24);

		std::cout << endl << "Reading weather data...   " << GetCurrentTimeStr() << flush;

		if(_pSimHyd->_bLogPerf)
			_pSimHyd->_logPerformance.AddStep("Reading weather data");

		const size_t nb_station = _stations.size();

		if(_netCdf_iType == -1)	//lecture seulement si type different de NetCDF (si NetCDF les donnees sont deja lu)
		{
			for(index=0; index<nb_station; ++index)
			{
				station = static_pointer_cast<STATION_METEO>(_stations[index]);
				station->LectureDonnees(date_debut, date_fin, pas_de_temps);
			}
		}

		if(_bStationInterpolation)
		{
			//determine les stations ou il y a des donnees manquantes
			std::cout << endl << "Checking missing weather data...   " << GetCurrentTimeStr() << flush;

			if(_pSimHyd->_bLogPerf)
				_pSimHyd->_logPerformance.AddStep("Checking missing weather data");

			vector<STATION_METEO*> stations;

			for(index=0; index<nb_station; ++index)
			{
				station = static_pointer_cast<STATION_METEO>(_stations[index]);
				date_courante = debut;

				done = false;
				while(date_courante < fin && !done)
				{
					donnee = station->PrendreDonnees(date_courante, pas_de_temps);

					if (donnee.PrendreTMin() <= VALEUR_MANQUANTE || donnee.PrendreTMax() <= VALEUR_MANQUANTE || donnee.PrendrePluie() <= VALEUR_MANQUANTE)
					{
						stations.push_back(station.get());
						done = true;
					}

					date_courante+= pas_de_temps;
				}
			}

			//interpole les donnees manquantes

			if(!stations.empty())
			{
				std::cout << endl << "Missing weather data interpolation...   " << GetCurrentTimeStr() << flush;

				if(_pSimHyd->_bLogPerf)
					_pSimHyd->_logPerformance.AddStep("Missing weather data interpolation");

				vector<COORDONNEE> coordonnees_station;

				for(auto iter = begin(_stations); iter != end(_stations); iter++)
					coordonnees_station.push_back(iter->get()->_coordonneeCRSprojet);

				for(auto iter = begin(stations); iter != end(stations); ++iter)
				{
					station1 = *iter;
					date_courante = debut;

					while(date_courante < fin)
					{
						donnee1 = station1->PrendreDonnees(date_courante, pas_de_temps);

						distances = CalculDistance_v1(coordonnees_station, station1->_coordonneeCRSprojet);
				
						for(index = 1; index < distances.size(); ++index)
						{
							station2 = static_pointer_cast<STATION_METEO>(_stations[distances[index]]);
							diff_alt = station1->PrendreCoordonnee().PrendreZ() - station2->PrendreCoordonnee().PrendreZ();

							donnee2 = station2->PrendreDonnees(date_courante, pas_de_temps);

							if(donnee1.PrendreTMin() <= VALEUR_MANQUANTE && donnee2.PrendreTMin() > VALEUR_MANQUANTE)
							{
								tmin = donnee2.PrendreTMin() + (-0.5f) * static_cast<float>(diff_alt) / 100.0f;
							
								if(donnee1.PrendreTMax() > VALEUR_MANQUANTE)
									tmin = min(tmin, donnee1.PrendreTMax());

								donnee1.ChangeTemperature_v1(tmin, donnee1.PrendreTMax());
								station1->ChangeDonnees(donnee1, date_courante, pas_de_temps);
							}

							if(donnee1.PrendreTMax() <= VALEUR_MANQUANTE && donnee2.PrendreTMax() > VALEUR_MANQUANTE)
							{
								tmax = donnee2.PrendreTMax() + (-0.5f) * static_cast<float>(diff_alt) / 100.0f;

								if(donnee1.PrendreTMin() > VALEUR_MANQUANTE)
									tmax = max(tmax, donnee1.PrendreTMin());

								donnee1.ChangeTemperature_v1(donnee1.PrendreTMin(), tmax);
								station1->ChangeDonnees(donnee1, date_courante, pas_de_temps);
							}

							if(donnee1.PrendrePluie() <= VALEUR_MANQUANTE && donnee2.PrendrePluie() > VALEUR_MANQUANTE)
							{
								pluie = donnee2.PrendrePluie() + (-0.5f) * static_cast<float>(diff_alt) / 100.0f;

								donnee1.ChangePluie(max(pluie, 0.0f));
								station1->ChangeDonnees(donnee1, date_courante, pas_de_temps);
							}
						}

						date_courante+= pas_de_temps;
					}
				}

				if(_pSimHyd->_bLogPerf)
					_pSimHyd->_logPerformance.AddStep("Completed");
			}
		}
	}


	//********************************************************************************************************************
	//LectureDonnees_v2 used by moyenne3station v2 and thiessen v2 models
	void STATIONS_METEO::LectureDonnees_v2(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps)
	{
		// NOTE: on doit lire au moins une journee complete afin d'avoir les temperatures journalieres

		DATE_HEURE date_courante;
		DONNEE_METEO donnee1, donnee2;
		STATION_METEO* station1;
		vector<size_t> distances;
		shared_ptr<STATION_METEO> station;
		size_t idxLoop, index;
		float pluie, tmax, tmin, diff_alt;
		bool done;
		
		DATE_HEURE date_debut(debut.PrendreAnnee(), debut.PrendreMois(), debut.PrendreJour(), 0);
		DATE_HEURE date_fin(fin.PrendreAnnee(), fin.PrendreMois(), fin.PrendreJour(), 0);

		date_fin.AdditionHeure(24);

		std::cout << endl << "Reading weather data...   " << GetCurrentTimeStr() << flush;

		if(_pSimHyd->_bLogPerf)
			_pSimHyd->_logPerformance.AddStep("Reading weather data");

		const size_t nb_station = _stations.size();

		if(_netCdf_iType == -1)	//lecture seulement si type different de NetCDF (si cest NetCDF les donnees ont deja ete lu)
		{
			for(index=0; index!=nb_station; index++)
			{
				station = static_pointer_cast<STATION_METEO>(_stations[index]);
				station->LectureDonnees(date_debut, date_fin, pas_de_temps);
			}
		}

		if(_bStationInterpolation)
		{
			//determine les stations ou il y a des donnees manquantes
			std::cout << endl << "Checking missing weather data...   " << GetCurrentTimeStr() << flush;

			if(_pSimHyd->_bLogPerf)
				_pSimHyd->_logPerformance.AddStep("Checking missing weather data");

			vector<STATION_METEO*> stations;

			for(index=0; index!=nb_station; index++)
			{
				station = static_pointer_cast<STATION_METEO>(_stations[index]);
				date_courante = debut;
				done = false;

				while(date_courante != fin && !done)
				{
					donnee1 = station->PrendreDonnees(date_courante, pas_de_temps);

					if(donnee1.PrendreTMin() <= VALEUR_MANQUANTE || donnee1.PrendreTMax() <= VALEUR_MANQUANTE || donnee1.PrendrePluie() <= VALEUR_MANQUANTE)
					{
						stations.push_back(station.get());
						done = true;
					}

					date_courante+= pas_de_temps;
				}
			}

			//interpole les donnees manquantes

			if(!stations.empty())
			{
				std::cout << endl << "Missing weather data interpolation...   " << GetCurrentTimeStr() << flush;

				if(_pSimHyd->_bLogPerf)
					_pSimHyd->_logPerformance.AddStep("Missing weather data interpolation");

				vector<COORDONNEE> coordonnees_station;

				for(auto iter = begin(_stations); iter != end(_stations); iter++)
					coordonnees_station.push_back(iter->get()->_coordonneeCRSprojet);

				idxLoop = 0;
				for(auto iter = begin(stations); iter != end(stations); iter++)
				{
					station1 = *iter;

					//int yy = fin.PrendreAnnee();
					//int mm = fin.PrendreMois();
					//int dd = fin.PrendreJour();

					date_courante = debut;
					while(date_courante != fin)
					{
						//int yy = date_courante.PrendreAnnee();
						//int mm = date_courante.PrendreMois();
						//int dd = date_courante.PrendreJour();
						//if(yy==1993&&mm==1&&dd==16&&station1->PrendreIdent() == "2100500")
						//	yy=yy;

						donnee1 = station1->PrendreDonnees(date_courante, pas_de_temps);
						distances = CalculDistance(coordonnees_station, station1->_coordonneeCRSprojet);
				
						for(index=0; index!=distances.size(); index++)
						{
							if(distances[index] != idxLoop)
							{
								station = static_pointer_cast<STATION_METEO>(_stations[distances[index]]);
								diff_alt = static_cast<float>(station1->PrendreCoordonnee().PrendreZ() - station->PrendreCoordonnee().PrendreZ());

								donnee2 = station->PrendreDonnees(date_courante, pas_de_temps);

								if(donnee1.PrendreTMin() <= VALEUR_MANQUANTE && donnee2.PrendreTMin() > VALEUR_MANQUANTE)
								{
									tmin = donnee2.PrendreTMin() + _fGradientStationTemp * diff_alt / 100.0f;
							
									if(donnee1.PrendreTMax() > VALEUR_MANQUANTE)
										tmin = min(tmin, donnee1.PrendreTMax());

									donnee1.ChangeTemperature(tmin, donnee1.PrendreTMax());
									station1->ChangeDonnees(donnee1, date_courante, pas_de_temps);
								}

								if(donnee1.PrendreTMax() <= VALEUR_MANQUANTE && donnee2.PrendreTMax() > VALEUR_MANQUANTE)
								{
									tmax = donnee2.PrendreTMax() + _fGradientStationTemp * diff_alt / 100.0f;

									if(donnee1.PrendreTMin() > VALEUR_MANQUANTE)
										tmax = max(tmax, donnee1.PrendreTMin());

									donnee1.ChangeTemperature(donnee1.PrendreTMin(), tmax);
									station1->ChangeDonnees(donnee1, date_courante, pas_de_temps);
								}

								if(donnee1.PrendrePluie() <= VALEUR_MANQUANTE && donnee2.PrendrePluie() > VALEUR_MANQUANTE)
								{
									if(donnee2.PrendrePluie() == 0.0f)
										pluie = 0.0f;
									else
										pluie = donnee2.PrendrePluie() + _fGradientStationPrecip * diff_alt / 100.0f;

									donnee1.ChangePluie(max(pluie, 0.0f));
									station1->ChangeDonnees(donnee1, date_courante, pas_de_temps);
								}
							}
						}

						date_courante+= pas_de_temps;
					}

					++idxLoop;
				}

				if(_pSimHyd->_bLogPerf)
					_pSimHyd->_logPerformance.AddStep("Completed");
			}
		}
	}

}
