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

#ifndef CORRECTION_H_INCLUDED
#define CORRECTION_H_INCLUDED


#include "date_heure.hpp"


namespace HYDROTEL
{

	enum TYPE_CORRECTION
	{
		TYPE_CORRECTION_TEMPERATURE = 1,
		TYPE_CORRECTION_PLUIE,
		TYPE_CORRECTION_NEIGE,
		TYPE_CORRECTION_RESERVE_SOL,
		TYPE_CORRECTION_NEIGE_AU_SOL,
		TYPE_CORRECTION_SATURATION,
	};

	enum TYPE_GROUPE
	{
		TYPE_GROUPE_ALL,
		TYPE_GROUPE_HYDRO,
		TYPE_GROUPE_CORRECTION,
	};

	class CORRECTION
	{
	public:
		CORRECTION(TYPE_CORRECTION variable, TYPE_GROUPE type_groupe, const std::string& nom_groupe);
		~CORRECTION();

		/// retourne vrai si la correction est applicable a cette date
		bool Applicable(const DATE_HEURE& date) const;

		float PrendreCoefficientAdditif() const;

		float PrendreCoefficientMultiplicatif() const;

		/// retourne la variable corrige
		TYPE_CORRECTION PrendreVariable() const;

		TYPE_GROUPE PrendreTypeGroupe() const;

		std::string PrendreNomGroupe() const;

		void ChangePeriode(int actif, DATE_HEURE debut, DATE_HEURE fin);

		void ChangeCoefficient(float additif, float multiplicatif);

		/// saturation de la reserve en eau des couches de sol
		void ChangeCoefficientSaturation(float coeffCouche1, float coeffCouche2, float coeffCouche3);

		float PrendreCoeffSaturationCouche1() const;		
		float PrendreCoeffSaturationCouche2() const;		
		float PrendreCoeffSaturationCouche3() const;

	protected:
		bool _actif;

		DATE_HEURE _date_debut;
		DATE_HEURE _date_fin;

		TYPE_CORRECTION _variable;
		TYPE_GROUPE _type_groupe;
		std::string _nom_groupe;

		float _coefficient_additif;
		float _coefficient_multiplicatif;

		float _coeffSaturationCouche1;
		float _coeffSaturationCouche2;
		float _coeffSaturationCouche3;
	};

}

#endif