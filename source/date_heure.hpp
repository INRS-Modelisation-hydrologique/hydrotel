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

#ifndef DATE_HEURE_H_INCLUDED
#define DATE_HEURE_H_INCLUDED


#include <iomanip>
#include <sstream>
#include <boost/date_time/gregorian/gregorian_types.hpp>


namespace HYDROTEL 
{

	class DATE_HEURE
	{
	public:
		DATE_HEURE();
		DATE_HEURE(unsigned short annee, unsigned short mois, unsigned short jour, unsigned short heure);
		DATE_HEURE(unsigned short annee, unsigned short jour_julien, unsigned short heure);

		~DATE_HEURE();

		unsigned short PrendreAnnee() const;

		/// retourne le mois (1..12)
		unsigned short PrendreMois() const;

		/// retourne le jour (1..31)
		unsigned short PrendreJour() const;

		/// retourne l'heure (0..23)
		unsigned short PrendreHeure() const;

		/// retourne le jour julien (1..366)
		unsigned short PrendreJourJulien() const;

		/// retourne unix epoch time value	//[secondes]	//le fuseau horaire n'est pas pris en compte
		int EpochTime() const;

		/// retourne la difference en nombre de jour avec date_heure
		int NbJourEntre(const DATE_HEURE& date_heure) const;

		/// retourne la difference en nombre d'heure avec date_heure
		int NbHeureEntre(const DATE_HEURE& date_heure) const;

		/// additionne un nombre d'heure 
		void AdditionHeure(int heures);

		void SoustraitHeure(int heures);

		/// retourne vrai si la date est bissextile
		bool Bissextile();

		/// retourne vrai si l'annee est bissextile
		static bool Bissextile(unsigned short annee);

		/// convertie une chaine de caracteres en date
		static DATE_HEURE Convertie(const std::string& date);

		/// retourne vrai si la date est egal
		bool operator == (const DATE_HEURE& date_heure) const;

		/// retourne vrai si la date est differente
		bool operator != (const DATE_HEURE& date_heure) const;

		/// retourne vrai si la date est plus grande ou egal
		bool operator >= (const DATE_HEURE& date_heure) const;

		/// retourne vrai si la date est plus petite ou egal
		bool operator <= (const DATE_HEURE& date_heure) const;

		/// retourne vrai si la date est plus petite
		bool operator <  (const DATE_HEURE& date_heure) const;

		/// retourne vrai si la date est plus grande
		bool operator >  (const DATE_HEURE& date_heure) const;

		/// additionne un nombre d'heure 
		DATE_HEURE& operator += (const unsigned short heures);

		/// soustrait un nombre d'heure 
		DATE_HEURE& operator -= (const unsigned short heures);

	private:
		boost::gregorian::date _date;
		unsigned short _heure;
	};

	inline
	std::ostream& operator << (std::ostream& out, const DATE_HEURE& date_heure)
	{
		std::ostringstream ss;

		ss << std::setfill('0') << 
			std::setw(4) << date_heure.PrendreAnnee() << '-' << 
			std::setw(2) << date_heure.PrendreMois() << '-' <<
			std::setw(2) << date_heure.PrendreJour() << ' ' << 
			std::setw(2) << date_heure.PrendreHeure() << ":00";

		out << ss.str();

		return out;
	}

	/// retourne l'addition d'un nombre d'heure a la date
	DATE_HEURE operator + (const DATE_HEURE& date_heure, unsigned short heures);

	/// retourne la soustraction d'un nombre d'heure a la date
	DATE_HEURE operator - (const DATE_HEURE& date_heure, unsigned short heures);

}

#endif
