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

#include "propriete_hydrolique.hpp"


namespace HYDROTEL
{

	PROPRIETE_HYDROLIQUE::PROPRIETE_HYDROLIQUE()
	{
	}

	PROPRIETE_HYDROLIQUE::PROPRIETE_HYDROLIQUE(const std::string& nom, float thetas, float thetapf, float thetacc,
		float ks, float lambda, float psis, float alpha)
		: _nom(nom)
		, _thetas(thetas)
		, _thetapf(thetapf)
		, _thetacc(thetacc)
		, _ks(ks)
		, _lambda(lambda)
		, _psis(psis)
		, _alpha(alpha)
	{

	}

	PROPRIETE_HYDROLIQUE::~PROPRIETE_HYDROLIQUE()
	{
	}

	std::string PROPRIETE_HYDROLIQUE::PrendreNom() const
	{
		return _nom;
	}

	float PROPRIETE_HYDROLIQUE::PrendreThetas() const
	{
		return _thetas;
	}

	float PROPRIETE_HYDROLIQUE::PrendreThetapf() const
	{
		return _thetapf;
	}

	float PROPRIETE_HYDROLIQUE::PrendreThetacc() const
	{
		return _thetacc;
	}

	float PROPRIETE_HYDROLIQUE::PrendreKs() const
	{
		return _ks;
	}

	float PROPRIETE_HYDROLIQUE::PrendreLambda() const
	{
		return _lambda;
	}

	float PROPRIETE_HYDROLIQUE::PrendrePsis() const
	{
		return _psis;
	}

	float PROPRIETE_HYDROLIQUE::PrendreAlpha() const
	{
		return _alpha;
	}

}
