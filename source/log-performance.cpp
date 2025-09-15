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

#include "log-performance.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>


using namespace std;


namespace HYDROTEL
{

	LOG_PERFORMANCE::LOG_PERFORMANCE()
	{
		_iUnit = 1;
		_bForceUnit = false;

		_nbSecInterpolation = 0.0;
		_nbSecFonteNeige = 0.0;
		_nbSecFonteGlacier = 0.0;
		_nbSecTempSol = 0.0;
		_nbSecETP = 0.0;
		_nbSecBilanVertical = 0.0;
		_nbSecRuissellement = 0.0;
		_nbSecAcheminement = 0.0;
	}


	LOG_PERFORMANCE::~LOG_PERFORMANCE()
	{
	}


	void LOG_PERFORMANCE::AddStep(string sMess, bool bAddTimeString)
	{
		string str;
		time_t tt;
		tm* ptm;
		char buf[20];

		boost::chrono::system_clock::time_point tp;

		if(bAddTimeString)
		{
			tp = boost::chrono::system_clock::now();
			tt = boost::chrono::system_clock::to_time_t(tp);
			ptm = localtime(&tt);
			strftime(buf, 20, "%Y-%m-%d %H:%M:%S", ptm);

			str = buf;
			str+= "   ";
		}

		str+= sMess;
		_listStr.push_back(str);

		_listTimePointBegin.push_back(boost::chrono::steady_clock::time_point::max());	//no time for this step: values will not be used
		_listTimePointEnd.push_back(boost::chrono::steady_clock::time_point::max());	//
	}


	size_t LOG_PERFORMANCE::AddStep(string sMess, boost::chrono::steady_clock::time_point timePointBegin)
	{
		string str;
		time_t tt;
		tm* ptm;
		char buf[20];

		boost::chrono::system_clock::time_point tp;
		boost::chrono::steady_clock::time_point tp2;

		tp = boost::chrono::system_clock::now();
		tt = boost::chrono::system_clock::to_time_t(tp);
		ptm = localtime(&tt);
		strftime(buf, 20, "%Y-%m-%d %H:%M:%S", ptm);
		str = buf;

		str+= "   ";
		str+= sMess;
		_listStr.push_back(str);

		_listTimePointBegin.push_back(timePointBegin);
		_listTimePointEnd.push_back(boost::chrono::steady_clock::time_point::max());

		return _listTimePointBegin.size()-1;
	}


	void LOG_PERFORMANCE::EndStep(size_t idx, boost::chrono::steady_clock::time_point timePointEnd)
	{
		_listTimePointEnd[idx] = timePointEnd;
	}


	bool LOG_PERFORMANCE::SaveFile(string sPathFile)
	{
		boost::chrono::duration<double, boost::milli> ms_double;
		ostringstream oss;
		ofstream file;
		size_t i;
		string str;
		double dVal, dValTotal;

		_sErr = "";

		try{
		file.open(sPathFile, ios_base::trunc);
		if(!file.is_open())
			_sErr = "error creating file: " + sPathFile;
		else
		{
			file << endl;

			ms_double = _tpInitAndSimEnd - _tpInitAndSimBegin;
			dValTotal = ms_double.count() / 1000.0;

			if(!_bForceUnit)
			{
				if(dValTotal <= 600.0)
					_iUnit = 1;
				else
					_iUnit = 2;	//set in minutes if total simulation time > 10 min
			}

			for(i=0; i!=_listStr.size(); i++)
			{
				oss.str("");

				if(_listTimePointBegin[i] != boost::chrono::steady_clock::time_point::max())
				{
					//add step duration
					//std::chrono::milliseconds ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
					ms_double = _listTimePointEnd[i] - _listTimePointBegin[i];
					dVal = ms_double.count() / 1000.0;

					oss << _listStr[i] << " (" << GetTimeValueString(dVal) << ").";
				}
				else
					oss << _listStr[i] << ".";

				file << oss.str() << endl;
			}

			file << endl;

			//data read time
			ms_double = _tpLectureEnd - _tpLectureBegin;
			dVal = ms_double.count() / 1000.0;
			oss.str("");
			oss << "Simulation data reading: " << GetTimeValueString(dVal) << ".";
			file << oss.str() << endl;

			file << endl;

			//initialization time
			ms_double = _tpInitEnd - _tpInitBegin;
			dVal = ms_double.count() / 1000.0;
			oss.str("");
			oss << "Initialization: " << GetTimeValueString(dVal) << ".";
			file << oss.str() << endl;

			file << endl;
			
			//simulation time (simulation only)
			ms_double = _tpSimEnd - _tpSimBegin;
			dVal = ms_double.count() / 1000.0;
			oss.str("");
			oss << "Simulation: " << GetTimeValueString(dVal) << ".";
			file << oss.str() << endl;

			file << endl;

			if(_nbSecInterpolation != 0.0)
			{
				oss.str("");
				oss << "   Weather interpolation:           " << GetTimeValueString(_nbSecInterpolation) << ".";
				file << oss.str() << endl;
			}
			if(_nbSecFonteNeige != 0.0)
			{
				oss.str("");
				oss << "   Snow cover evolution:            " << GetTimeValueString(_nbSecFonteNeige) << ".";
				file << oss.str() << endl;
			}
			if(_nbSecFonteGlacier != 0.0)
			{
				oss.str("");
				oss << "   Glacier melt:                    " << GetTimeValueString(_nbSecFonteGlacier) << ".";
				file << oss.str() << endl;
			}
			if(_nbSecTempSol != 0.0)
			{
				oss.str("");
				oss << "   Soil temp/Frost depth:           " << GetTimeValueString(_nbSecTempSol) << ".";
				file << oss.str() << endl;
			}
			if(_nbSecETP != 0.0)
			{
				oss.str("");
				oss << "   Potential evp:                   " << GetTimeValueString(_nbSecETP) << ".";
				file << oss.str() << endl;
			}
			if(_nbSecBilanVertical != 0.0)
			{
				oss.str("");
				oss << "   Vertical water balance:          " << GetTimeValueString(_nbSecBilanVertical) << ".";
				file << oss.str() << endl;
			}
			if(_nbSecRuissellement != 0.0)
			{
				oss.str("");
				oss << "   Flow towards network (runoff):   " << GetTimeValueString(_nbSecRuissellement) << ".";
				file << oss.str() << endl;
			}
			if(_nbSecAcheminement != 0.0)
			{
				oss.str("");
				oss << "   Flow in the network (routing):   " << GetTimeValueString(_nbSecAcheminement) << ".";
				file << oss.str() << endl;
			}

			file << endl;

			//post-processing
			ms_double = _tpPostEnd - _tpPostBegin;
			dVal = ms_double.count() / 1000.0;
			oss.str("");
			oss << "Post-processing: " << GetTimeValueString(dVal) << ".";
			file << oss.str() << endl;

			file << endl;

			//total time (initialization, simulation and post-processing)
			oss.str("");
			oss << "Total duration: " << GetTimeValueString(dValTotal) << ".";
			file << oss.str() << endl;

			file << endl;

			file.close();
		}

		}
		catch(...)
		{
			if(file && file.is_open())
				file.close();
			_sErr = "error writing to file: " + sPathFile;
		}

		return (_sErr == "");
	}


	//Return string in seconds or minutes depending selected output unit
	string LOG_PERFORMANCE::GetTimeValueString(double dTimeValSec)
	{
		ostringstream oss;

		if(_iUnit == 2)
			oss << setprecision(2) << setiosflags(ios::fixed) << (dTimeValSec / 60.0) << " min";
		else
			oss << setprecision(2) << setiosflags(ios::fixed) << dTimeValSec << " sec";

		return oss.str();
	}

}
