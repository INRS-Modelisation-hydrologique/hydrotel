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

#include "correction.hpp"

#include "erreur.hpp"


using namespace std;


namespace HYDROTEL
{

	CORRECTION::CORRECTION(TYPE_CORRECTION variable, TYPE_GROUPE type_groupe, const std::string& nom_groupe)
		: _actif(true)
		, _date_debut()
		, _date_fin()
		, _variable(variable)
		, _type_groupe(type_groupe)
		, _nom_groupe(nom_groupe)
		, _coefficient_additif(0)
		, _coefficient_multiplicatif(1)
	{
	}

	CORRECTION::~CORRECTION()
	{
	}

	bool CORRECTION::Applicable(const DATE_HEURE& date) const
	{
		return _actif && date >= _date_debut && date <= _date_fin;
	}

	float CORRECTION::PrendreCoefficientAdditif() const
	{
		return _coefficient_additif;
	}

	float CORRECTION::PrendreCoefficientMultiplicatif() const
	{
		return _coefficient_multiplicatif;
	}

	float CORRECTION::PrendreCoeffSaturationCouche1() const
	{
		return _coeffSaturationCouche1;
	}

	float CORRECTION::PrendreCoeffSaturationCouche2() const
	{
		return _coeffSaturationCouche2;
	}

	float CORRECTION::PrendreCoeffSaturationCouche3() const
	{
		return _coeffSaturationCouche3;
	}

	TYPE_CORRECTION CORRECTION::PrendreVariable() const
	{
		return _variable;
	}

	TYPE_GROUPE CORRECTION::PrendreTypeGroupe() const
	{
		return _type_groupe;
	}

	string CORRECTION::PrendreNomGroupe() const
	{
		return _nom_groupe;
	}

	void CORRECTION::ChangePeriode(int actif, DATE_HEURE debut, DATE_HEURE fin)
	{
		//BOOST_ASSERT(debut <= fin);

		_actif = actif == 0 ? false : true;
		_date_debut = debut;
		_date_fin = fin;
	}

	void CORRECTION::ChangeCoefficient(float additif, float multiplicatif)
	{
		_coefficient_additif = additif;
		_coefficient_multiplicatif = multiplicatif;
	}

	void CORRECTION::ChangeCoefficientSaturation(float coeffCouche1, float coeffCouche2, float coeffCouche3)
	{
		_coeffSaturationCouche1 = coeffCouche1;
		_coeffSaturationCouche2 = coeffCouche2;
		_coeffSaturationCouche3 = coeffCouche3;
	}

}
