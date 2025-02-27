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

#include "date_heure.hpp"

#include <boost/assert.hpp>


using namespace std;


namespace HYDROTEL 
{

	DATE_HEURE::DATE_HEURE()
		: _date(1973, 6, 1)
		, _heure(0)
	{
	}

	DATE_HEURE::DATE_HEURE(unsigned short annee, unsigned short mois, unsigned short jour, unsigned short heure)
		: _date(annee, mois, jour)
		, _heure(heure)
	{
		BOOST_ASSERT((heure >= 0 && heure <= 23) || heure == 24);

		if (heure == 24)
		{
			_date += boost::gregorian::days(1);
			_heure = 0;
		}
	}

	DATE_HEURE::DATE_HEURE(unsigned short annee, unsigned short jour_julien, unsigned short heure)		
		: _date(annee, 1, 1)
		, _heure(heure)
	{
		BOOST_ASSERT((heure >= 0 && heure <= 23) || heure == 24);

		_date += boost::gregorian::days(jour_julien - 1);

		if (heure == 24)
		{
			_date += boost::gregorian::days(1);
			_heure = 0;
		}
	}

	DATE_HEURE::~DATE_HEURE()
	{
	}


	unsigned short DATE_HEURE::PrendreAnnee() const
	{
		return _date.year();
	}


	unsigned short DATE_HEURE::PrendreMois() const
	{
		return _date.month();
	}


	unsigned short DATE_HEURE::PrendreJour() const
	{
		return _date.day();
	}


	unsigned short DATE_HEURE::PrendreHeure() const
	{
		return _heure;
	}


	unsigned short DATE_HEURE::PrendreJourJulien() const
	{
		return _date.day_of_year();
	}


	int DATE_HEURE::EpochTime() const
	{
		return (_date - boost::gregorian::date(1970, 1, 1)).days() * 24 * 60 * 60 + (_heure * 60 * 60);
	}


	int DATE_HEURE::NbJourEntre(const DATE_HEURE& date_heure) const
	{
		return (date_heure._date - _date).days();
	}


	int DATE_HEURE::NbHeureEntre(const DATE_HEURE& date_heure) const
	{
		return NbJourEntre(date_heure) * 24 + (date_heure._heure - _heure);
	}


	void DATE_HEURE::AdditionHeure(int heures)
	{
		int h = _heure + heures;
		_date += boost::gregorian::days(h / 24);
		_heure = h % 24;
	}


	void DATE_HEURE::SoustraitHeure(int heures)
	{
		int h = _heure - heures;

		_date -= boost::gregorian::days(abs(h) / 24);

		if(h % 24 != 0 && h < 0)
		{
			_date -= boost::gregorian::days(1);
			_heure = 24 - abs(h) % 24;
		}
		else
		{
			if(h >= 0)
				_heure = static_cast<unsigned short>(h);
		}
	}


	bool DATE_HEURE::operator == (const DATE_HEURE& date_heure) const
	{
		return _date == date_heure._date && _heure == date_heure._heure;
	}


	bool DATE_HEURE::operator != (const DATE_HEURE& date_heure) const
	{
		return !((*this) == date_heure);
	}


	bool DATE_HEURE::operator >= (const DATE_HEURE& date_heure) const
	{
		return (*this) == date_heure || (*this) > date_heure;
	}


	bool DATE_HEURE::operator <= (const DATE_HEURE& date_heure) const
	{
		return (*this) == date_heure || (*this) < date_heure;
	}


	bool DATE_HEURE::operator > (const DATE_HEURE& date_heure) const
	{
		return _date == date_heure._date ? _heure > date_heure._heure : _date > date_heure._date;
	}


	bool DATE_HEURE::operator < (const DATE_HEURE& date_heure) const
	{
		return _date == date_heure._date ? _heure < date_heure._heure : _date < date_heure._date;
	}


	DATE_HEURE& DATE_HEURE::operator += (const unsigned short heures)
	{
		AdditionHeure(heures);
		return *this;
	}


	DATE_HEURE& DATE_HEURE::operator -= (const unsigned short heures)
	{
		SoustraitHeure(heures);
		return *this;
	}


	DATE_HEURE operator + (const DATE_HEURE& date_heure, unsigned short heures)
	{
		DATE_HEURE d(date_heure);
		d += heures;
		return d;
	}


	DATE_HEURE operator - (const DATE_HEURE& date_heure, unsigned short heures)
	{
		DATE_HEURE d(date_heure);
		d -= heures;
		return d;
	}

	bool DATE_HEURE::Bissextile()
	{
		return Bissextile( PrendreAnnee() );
	}

	bool DATE_HEURE::Bissextile(unsigned short annee)
	{
		return boost::gregorian::gregorian_calendar::is_leap_year(annee);
	}

	DATE_HEURE DATE_HEURE::Convertie(const std::string& date)
	{
		unsigned short annee, mois, jour, heure;
		char c;

		istringstream ss(date);
		ss >> annee >> c >> mois >> c >> jour >> heure;
		
		return DATE_HEURE(annee, mois, jour, heure);	//exception is raised if date is not valid
	}

}
