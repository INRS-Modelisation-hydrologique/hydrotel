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

#include "groupe_zone.hpp"

#include <boost/assert.hpp>


using namespace std;


namespace HYDROTEL
{

	GROUPE_ZONE::GROUPE_ZONE()
	{
	}

	GROUPE_ZONE::~GROUPE_ZONE()
	{
	}

	string GROUPE_ZONE::PrendreNom() const
	{
		return _nom;
	}

	size_t GROUPE_ZONE::PrendreNbZone() const
	{
		return _ident_zones.size();
	}

	int GROUPE_ZONE::PrendreIdent(size_t index) const
	{
		BOOST_ASSERT(index < _ident_zones.size());
		return _ident_zones[index];
	}

	void GROUPE_ZONE::ChangeNom(const std::string& nom)
	{
		_nom = nom;
	}

	void GROUPE_ZONE::ChangeIdentZones(const std::vector<int>& ident_zones)
	{
		_ident_zones = ident_zones;
		_ident_zones.shrink_to_fit();
	}

}
