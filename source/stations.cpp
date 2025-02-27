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

#include "stations.hpp"

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	STATIONS::STATIONS()
	{
	}


	STATIONS::~STATIONS()
	{
	}


	std::string STATIONS::PrendreNomFichier() const
	{
		return _nom_fichier;
	}


	const PROJECTION& STATIONS::PrendreProjection() const
	{
		return _projection;
	}


	size_t STATIONS::PrendreNbStation() const
	{
		return _stations.size();
	}


	STATION* STATIONS::operator[] (size_t index) const
	{
		return _stations[index].get();
	}


	void STATIONS::Detruire()
	{
		_stations.clear();
	}


	void STATIONS::ChangeNomFichier(const string& nom_fichier)
	{
		//Detruire();
		_nom_fichier = nom_fichier;
	}


	void STATIONS::ChangeProjection(const PROJECTION& projection)
	{
		_projection = projection;
	}


	STATION* STATIONS::Recherche(const string& ident)
	{
		if(_map.empty())	// NOTE: il faut que la fonction CreeMapRecherche() soit appele
			return nullptr;
		else
		{
			string str2 = ident;
			boost::algorithm::to_lower(str2);
			return _map.find(str2) == _map.end() ? nullptr : _map[str2];
		}
	}


	void STATIONS::CreeMapRecherche()
	{
		string str;

		for (auto iter = begin(_stations); iter != end(_stations); ++iter)
		{
			str.clear();
			str = iter->get()->PrendreIdent();
			boost::algorithm::to_lower(str);

			_map[str] = iter->get();
		}
	}

}
