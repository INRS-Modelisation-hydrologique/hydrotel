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

#ifndef LOG_PERFORMANCE_H_INCLUDED
#define LOG_PERFORMANCE_H_INCLUDED


#include "log-performance.hpp"

#include <vector>
#include <string>

#include <boost/chrono.hpp>


namespace HYDROTEL
{

	class LOG_PERFORMANCE
	{

	public:

		LOG_PERFORMANCE();
		virtual ~LOG_PERFORMANCE();

		void	AddStep(std::string sMess, bool bAddTimeString = true);

		size_t	AddStep(std::string sMess, boost::chrono::steady_clock::time_point timePointBegin);

		void	EndStep(size_t idx, boost::chrono::steady_clock::time_point timePointEnd);

		bool	SaveFile(std::string sPathFile);


		std::string												_sErr;

		int														_iUnit;			//unit for output	//1=seconds		//2=minutes		//(default 1)
		bool													_bForceUnit;	//if true disable automatic adjustement

		double													_nbSecInterpolation;
		double													_nbSecFonteNeige;
		double													_nbSecFonteGlacier;
		double													_nbSecTempSol;
		double													_nbSecETP;
		double													_nbSecBilanVertical;
		double													_nbSecRuissellement;
		double													_nbSecAcheminement;

		boost::chrono::steady_clock::time_point					_tpInitAndSimBegin;
		boost::chrono::steady_clock::time_point					_tpInitAndSimEnd;

		boost::chrono::steady_clock::time_point					_tpLectureBegin;
		boost::chrono::steady_clock::time_point					_tpLectureEnd;

		boost::chrono::steady_clock::time_point					_tpInitBegin;
		boost::chrono::steady_clock::time_point					_tpInitEnd;

		boost::chrono::steady_clock::time_point					_tpSimBegin;
		boost::chrono::steady_clock::time_point					_tpSimEnd;

		boost::chrono::steady_clock::time_point					_tpPostBegin;
		boost::chrono::steady_clock::time_point					_tpPostEnd;


	private:

		std::string GetTimeValueString(double dTimeValSec);		//return string in seconds or minutes depending selected output unit


		std::vector<std::string>								_listStr;				//log file content

		std::vector<boost::chrono::steady_clock::time_point>      _listTimePointBegin;    //time point at the beginning of the step (according to _listStr)
		std::vector<boost::chrono::steady_clock::time_point>      _listTimePointEnd;		//time point at the end of the step (according to _listStr)

	};

}

#endif
