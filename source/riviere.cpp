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

#include "riviere.hpp"

#include <boost/assert.hpp>


namespace HYDROTEL
{

	RIVIERE::RIVIERE()
		: TRONCON(TRONCON::RIVIERE)
		, _largeur(1)
		, _longueur(1)
		, _profondeur(1)
		, _pente(0.0025f)
		, _manning(0.04f)
	{
	}

	RIVIERE::~RIVIERE()
	{
	}

	float RIVIERE::PrendreLargeur() const
	{
		return _largeur;
	}

	float RIVIERE::PrendreLongueur() const
	{
		return _longueur;
	}

	float RIVIERE::PrendreProfondeur() const
	{
		return _profondeur;
	}

	float RIVIERE::PrendrePente() const
	{
		return _pente;
	}

	float RIVIERE::PrendreManning() const
	{
		return _manning;
	}

	void RIVIERE::ChangeLargeur(float largeur)
	{
		BOOST_ASSERT(largeur > 0);
		_largeur = largeur;
	}

	void RIVIERE::ChangeLongueur(float longueur)
	{
		BOOST_ASSERT(longueur > 0);
		_longueur = longueur;
	}

	void RIVIERE::ChangeProfondeur(float profondeur)
	{
		BOOST_ASSERT(profondeur > 0);
		_profondeur = profondeur;
	}

	void RIVIERE::ChangePente(float pente)
	{
		BOOST_ASSERT(pente > 0);
		_pente = pente;
	}

	void RIVIERE::ChangeManning(float manning)
	{
		BOOST_ASSERT(manning > 0.02f);
		_manning = manning;
	}

}
