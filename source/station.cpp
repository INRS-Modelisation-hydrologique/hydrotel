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

#include "station.hpp"


using namespace std;


namespace HYDROTEL
{

	STATION::STATION(const string& nom_fichier)
		: _nom_fichier(nom_fichier)
	{
		_iVersionThiessenMoy3Station = 2;
	}


	STATION::~STATION()
	{
	}


	std::string STATION::PrendreIdent() const
	{
		return _ident;
	}


	std::string STATION::PrendreNom() const
	{
		return _nom;
	}


	std::string STATION::PrendreNomFichier() const
	{
		return _nom_fichier;
	}


	const COORDONNEE& STATION::PrendreCoordonnee() const
	{
		return _coordonnee;
	}


	void STATION::ChangeIdent(const string& ident)
	{
		_ident = ident;
	}


	void STATION::ChangeNom(const string& nom)
	{
		_nom = nom;
	}


	void STATION::ChangeCoordonnee(const COORDONNEE& coordonnee)
	{
		_coordonnee = coordonnee;
	}


	void STATION::ChangeNomFichier(const string& nom_fichier)
	{
		_nom_fichier = nom_fichier;
	}

}
