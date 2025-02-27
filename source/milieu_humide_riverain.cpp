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

#include "milieu_humide_riverain.hpp"

#include <cmath>

#include <boost/assert.hpp>


namespace HYDROTEL
{

	MILIEUHUMIDE_RIVERAIN::MILIEUHUMIDE_RIVERAIN(float wetarea, float wetaup, float wetadra, float wetdown,
												 float longueur, float amont, float aval,
												 float wetdnor, float wetdmax, float frac,
												 bool sauvegarde, float ksat_bk, float ksat_bs, float th_aq, float eauIni)
		: m_wetarea(wetarea)
		, m_wetaup(wetaup)
		, m_wetadra(wetadra)
		, m_wetdown(wetdown)
		, m_longueur(longueur)
		, m_lgrAmont(amont)
		, m_lgrAval(aval)
		, m_wetnd(wetdnor)
		, m_wetmxd(wetdmax)
		, m_frac(frac)
		, m_sauvegarde(sauvegarde)
		, m_ksat_bk(ksat_bk)	// 25.0f
		, m_ksat_bs(ksat_bs)	// 0.5f
		, m_th_aq(th_aq)		// 2.0f
		, m_eauIni(eauIni)		// 1.0f
	{
		BOOST_ASSERT(wetarea > 0);
		BOOST_ASSERT(longueur > 0);
		BOOST_ASSERT(amont >= 0);
		BOOST_ASSERT(aval >= 0);
		BOOST_ASSERT(wetdnor > 0);
		BOOST_ASSERT(wetdmax > 0);	

		m_wetmxsa = wetarea * 1000000;		// km^2 ==> m^2
		m_wetnsa = m_frac * m_wetmxsa;

		m_wetmxvol = m_wetmxd * m_wetmxsa;	// m^3
		m_wetnvol = m_wetnd * m_wetnsa;		// m^3

		m_alfa = (log10(m_wetmxsa) - log10(m_wetnsa)) / (log10(m_wetmxvol) - log10(m_wetnvol));
		m_belta = m_wetmxsa / pow(m_wetmxvol, m_alfa);
	}

	MILIEUHUMIDE_RIVERAIN::~MILIEUHUMIDE_RIVERAIN()
	{
	}

	bool MILIEUHUMIDE_RIVERAIN::get_sauvegarde() const
	{
		return m_sauvegarde;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetarea() const
	{
		return m_wetarea;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetaup() const
	{
		return m_wetaup;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetadra() const
	{
		return m_wetadra;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetdown() const
	{
		return m_wetdown;
	}


	float MILIEUHUMIDE_RIVERAIN::get_wetmxvol() const
	{
		return m_wetmxvol;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetmxsa() const
	{
		return m_wetmxsa;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetmxd() const
	{
		return m_wetmxd;
	}
	
	float MILIEUHUMIDE_RIVERAIN::get_wetnvol() const
	{
		return m_wetnvol;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetnsa() const
	{
		return m_wetnsa;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wetnd() const
	{
		return m_wetnd;
	}

	float MILIEUHUMIDE_RIVERAIN::get_alfa() const
	{
		return m_alfa;
	}

	float MILIEUHUMIDE_RIVERAIN::get_belta() const
	{
		return m_belta;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wet_v() const
	{
		return m_wet_v;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wet_a() const
	{
		return m_wet_a;
	}

	float MILIEUHUMIDE_RIVERAIN::get_wet_d() const
	{
		return m_wet_d;
	}

	void MILIEUHUMIDE_RIVERAIN::set_wet_v(float wet_v)
	{
		BOOST_ASSERT(wet_v >= 0);
		m_wet_v = wet_v;
	}

	void MILIEUHUMIDE_RIVERAIN::set_wet_a(float wet_a)
	{
		BOOST_ASSERT(wet_a >= 0);
		m_wet_a = wet_a;
	}

	void MILIEUHUMIDE_RIVERAIN::set_wet_d(float wet_d)
	{
		BOOST_ASSERT(wet_d >= 0);
		m_wet_d = wet_d;
	}

	float MILIEUHUMIDE_RIVERAIN::get_longueur() const
	{
		return m_longueur;
	}

	float MILIEUHUMIDE_RIVERAIN::get_longueur_amont() const
	{
		return m_lgrAmont;
	}

	float MILIEUHUMIDE_RIVERAIN::get_longueur_aval() const
	{
		return m_lgrAval;
	}


	float MILIEUHUMIDE_RIVERAIN::get_frac() const
	{
		return m_frac;
	}

	float MILIEUHUMIDE_RIVERAIN::get_ksat_bk() const
	{
		return m_ksat_bk;
	}

	float MILIEUHUMIDE_RIVERAIN::get_ksat_bs() const
	{
		return m_ksat_bs;
	}

	float MILIEUHUMIDE_RIVERAIN::get_th_aq() const
	{
		return m_th_aq;
	}

}
