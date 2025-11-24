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

#include "troncon.hpp"

#include "constantes.hpp"
#include "zone.hpp"

#include <algorithm>

#include <boost/assert.hpp>


using namespace std;


namespace HYDROTEL
{

	TRONCON::TRONCON(TRONCON::TYPE_TRONCON type_troncon)
		: _ident(-1)
		, _type_troncon(type_troncon)
		, _noeuds_aval()
		, _noeuds_amont()
		, _zones_amont()
		, _troncons_aval()
		, _troncons_amont()
		, _apport_lateral(VALEUR_MANQUANTE)
		, _debit_aval(VALEUR_MANQUANTE)
		, _debit_amont(VALEUR_MANQUANTE)
		, _iSchreve(-1)
	{
		_dLongueur = -1.0;

		_hauteurAvalMoy = dVALEUR_MANQUANTE;

		_prPrelevementTotal = 0.0;
		_prPrelevementCulture = 0.0;
		_prRejetTotal = 0.0;
		_prRejetEffluent = 0.0;
		_prIndicePression = 0.0;
	}

	TRONCON::~TRONCON()
	{
	}

	int TRONCON::PrendreIdent() const
	{
		return _ident;
	}

	TRONCON::TYPE_TRONCON TRONCON::PrendreType() const
	{
		return _type_troncon;
	}

	const vector<NOEUD*>& TRONCON::PrendreNoeudsAmont() const
	{
		return _noeuds_amont;
	}

	const vector<NOEUD*>& TRONCON::PrendreNoeudsAval() const
	{
		return _noeuds_aval;
	}

	vector<TRONCON*> TRONCON::PrendreTronconsAval() const
	{
		return _troncons_aval;
	}

	vector<TRONCON*> TRONCON::PrendreTronconsAmont() const
	{
		return _troncons_amont;
	}

	vector<ZONE*> TRONCON::PrendreZonesAmont() const
	{
		return _zones_amont;
	}

	void TRONCON::ChangeIdent(int ident)
	{
		_ident = ident;
	}

	void TRONCON::ChangeNoeudsAval(std::vector<NOEUD*>& noeuds_aval)
	{
		BOOST_ASSERT(!noeuds_aval.empty());
		_noeuds_aval = noeuds_aval;
	}

	void TRONCON::ChangeNoeudsAmont(std::vector<NOEUD*>& noeuds_amont)
	{
		//BOOST_ASSERT(!noeuds_amont.empty());
		_noeuds_amont = noeuds_amont;
	}

	void TRONCON::ChangeZonesAmont(vector<ZONE*>& zones_amont)
	{
		BOOST_ASSERT(!zones_amont.empty());
		_zones_amont = zones_amont;
	}

	void TRONCON::ChangeTronconsAval(std::vector<TRONCON*>& troncon_aval)
	{
		BOOST_ASSERT(!troncon_aval.empty());
		_troncons_aval = troncon_aval;
	}

	void TRONCON::ChangeTronconsAmont(std::vector<TRONCON*>& troncons_amont)
	{
		BOOST_ASSERT(!troncons_amont.empty());
		_troncons_amont = troncons_amont;
	}

	double TRONCON::PrendreSuperficieDrainee() const
	{
		double superficie = 0.0;

		for(auto iter = begin(_troncons_amont); iter != end(_troncons_amont); ++iter)
			superficie+= (*iter)->PrendreSuperficieDrainee();

		for(auto iter = begin(_zones_amont); iter != end(_zones_amont); ++iter)
			superficie+= (*iter)->PrendreSuperficie();	//km2

		return superficie;	//km2
	}

	float TRONCON::PrendreApportLateral() const
	{
		return _apport_lateral;
	}

	void TRONCON::ChangeApportLateral(float apport)
	{
		BOOST_ASSERT(apport >= 0);
		_apport_lateral = apport;
	}

	void TRONCON::ChangeDebitAval(float debit)
	{
		BOOST_ASSERT(debit >= 0);
		_debit_aval = debit;
	}

	void TRONCON::ChangeDebitAvalMoyen(float debitMoyen)
	{
		BOOST_ASSERT(debitMoyen >= 0);
		_debit_aval_moyen = debitMoyen;
	}

	void TRONCON::ChangeDebitAmont(float debit)
	{
		BOOST_ASSERT(debit >= 0);
		_debit_amont = debit;
	}

	void TRONCON::ChangeDebitAmontMoyen(float debitMoyen)
	{
		BOOST_ASSERT(debitMoyen >= 0);
		_debit_amont_moyen = debitMoyen;
	}

	float TRONCON::PrendreDebitAmont() const
	{
		return _debit_amont;
	}

	float TRONCON::PrendreDebitAmontMoyen() const
	{
		return _debit_amont_moyen;
	}

	float TRONCON::PrendreDebitAval() const
	{
		return _debit_aval;
	}

	float TRONCON::PrendreDebitAvalMoyen() const
	{
		return _debit_aval_moyen;
	}

}
