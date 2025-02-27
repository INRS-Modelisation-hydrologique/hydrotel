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

#include "barrage_historique.hpp"

#include <boost/assert.hpp>


using namespace std;


namespace HYDROTEL
{

	BARRAGE_HISTORIQUE::BARRAGE_HISTORIQUE()
		: TRONCON(TRONCON::BARRAGE_HISTORIQUE)
		, _station_hydro(nullptr)
	{
	}

	BARRAGE_HISTORIQUE::~BARRAGE_HISTORIQUE()
	{
	}

	void BARRAGE_HISTORIQUE::ChangeIdentStationHydro(const string& ident_station_hydro)
	{
		_ident_station_hydro = ident_station_hydro;
	}

	string BARRAGE_HISTORIQUE::PrendreIdentStationHydro() const
	{
		return _ident_station_hydro;
	}

	void BARRAGE_HISTORIQUE::ChangeStationHydro(STATION_HYDRO* station_hydro)
	{
		BOOST_ASSERT(station_hydro != nullptr);
		_station_hydro = station_hydro;
	}

	float BARRAGE_HISTORIQUE::PrendreDebit(DATE_HEURE date_heure, unsigned short pas_de_temps) const
	{
		BOOST_ASSERT(_station_hydro != nullptr);
		return _station_hydro->PrendreDebit(date_heure, pas_de_temps);
	}

}
