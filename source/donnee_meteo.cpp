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

#include "donnee_meteo.hpp"

#include "constantes.hpp"

#include <boost/assert.hpp>


namespace HYDROTEL
{

	DONNEE_METEO::DONNEE_METEO()
		: _tmin(VALEUR_MANQUANTE)
		, _tmax(VALEUR_MANQUANTE)
		, _pluie(VALEUR_MANQUANTE)
		, _neige(VALEUR_MANQUANTE)
	{
	}


	DONNEE_METEO::~DONNEE_METEO()
	{
	}


	void DONNEE_METEO::ChangeTemperature(float min, float max)
	{
		BOOST_ASSERT(min <= max || min <= VALEUR_MANQUANTE || max <= VALEUR_MANQUANTE);	//assert si min > tmax et que tmin et tmax ne sont pas nodata

		_tmin = min;
		_tmax = max;
	}

	void DONNEE_METEO::ChangeTemperature_v1(float min, float max)
	{
		//BOOST_ASSERT(min <= max || min <= VALEUR_MANQUANTE || max <= VALEUR_MANQUANTE);

		if(min > max)
		{
			_tmin = max;
			_tmax = min;
		}
		else
		{
			_tmin = min;
			_tmax = max;
		}
	}


	void DONNEE_METEO::ChangePluie(float pluie)
	{
		BOOST_ASSERT(pluie >= 0.0f || pluie <= VALEUR_MANQUANTE);

		_pluie = pluie;
	}


	void DONNEE_METEO::ChangeNeige(float neige)
	{
		BOOST_ASSERT(neige >= 0.0f || neige <= VALEUR_MANQUANTE);

		_neige = neige;
	}


	float DONNEE_METEO::PrendreTMin() const
	{
		return _tmin;
	}


	float DONNEE_METEO::PrendreTMax() const
	{
		return _tmax;
	}


	float DONNEE_METEO::PrendrePluie() const
	{
		return _pluie;
	}


	float DONNEE_METEO::PrendreNeige() const
	{
		return _neige;
	}

}
