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

#ifndef STATIONS_HYDRO_H_INCLUDED
#define STATIONS_HYDRO_H_INCLUDED


#include "stations.hpp"
#include "date_heure.hpp"


namespace HYDROTEL
{

	class STATIONS_HYDRO : public STATIONS
	{
	public:
		STATIONS_HYDRO();
		~STATIONS_HYDRO();

		void Lecture(const PROJECTION& projection);

	private:
		void LectureFormatSTH(const PROJECTION& projection);
	};

}

#endif
