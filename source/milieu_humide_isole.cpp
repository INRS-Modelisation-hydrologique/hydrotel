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

#include "milieu_humide_isole.hpp"

#include <cmath>

#include <boost/assert.hpp>


using namespace std;


namespace HYDROTEL
{

	MILIEUHUMIDE_ISOLE::MILIEUHUMIDE_ISOLE(float superficie, float wetfr, float wetdrafr, float frac, float wetdnor, float wetdmax, bool sauvegarde,
										   float ksat_bs, float c_ev, float c_prod, float eauIni)
		: m_superficie(superficie)
		, m_wetfr(wetfr)
		, m_wetdrafr(wetdrafr)
		, m_frac(frac)
		, m_wetdnor(wetdnor)
		, m_wetdmax(wetdmax)
		, m_sauvegarde(sauvegarde)
		, m_ksat_bs(ksat_bs)	 // 0.5f
		, m_c_ev(c_ev)			 // 0.6f
		, m_c_prod(c_prod)		 // 10.0f
		, m_eauIni(eauIni)		 // 1.0f
	{
		BOOST_ASSERT(superficie > 0.0f);
		BOOST_ASSERT(wetfr > 0.0f && wetfr <= 1.0f);
		BOOST_ASSERT(wetdrafr > 0.0f && wetdrafr <= 1.0f);
		BOOST_ASSERT(frac > 0.0f && frac <= 1.0f);
		BOOST_ASSERT(c_prod > 0.0f);

		m_wetvol = 0.0f;

		m_wetmxsa = superficie * 1000000.0f;	// m^2
		m_wetmxvol = m_wetdmax * m_wetmxsa;	// m^3

		float wet_nsa = m_frac * m_wetmxsa; // m^2
		m_wetnvol = m_wetdnor * wet_nsa;	// m^3

		m_a = (log10(m_wetmxsa) - log10(wet_nsa)) / (log10(m_wetmxvol) - log10(m_wetnvol));
		m_b = m_wetmxsa / pow(m_wetmxvol, m_a);
	}


	MILIEUHUMIDE_ISOLE::~MILIEUHUMIDE_ISOLE()
	{
	}

	bool MILIEUHUMIDE_ISOLE::GetSauvegarde() const
	{
		return m_sauvegarde;
	}

	float MILIEUHUMIDE_ISOLE::GetSuperficie() const
	{
		return m_superficie;
	}

	float MILIEUHUMIDE_ISOLE::GetWetmxsa() const
	{
		return m_wetmxsa;
	}

	// fraction du uhrh qui est en milieu humide
	float MILIEUHUMIDE_ISOLE::GetWetfr() const
	{
		return m_wetfr;
	}

	// fraction du uhrh qui est draine par le milieu humide
	float MILIEUHUMIDE_ISOLE::GetWetdrafr() const
	{
		return m_wetdrafr;
	}

	float MILIEUHUMIDE_ISOLE::GetWetmxvol() const
	{
		return m_wetmxvol;
	}

	float MILIEUHUMIDE_ISOLE::GetWetnvol() const
	{
		return m_wetnvol;
	}

	float MILIEUHUMIDE_ISOLE::GetA() const
	{
		return m_a;
	}

	float MILIEUHUMIDE_ISOLE::GetB() const
	{
		return m_b;
	}

	float MILIEUHUMIDE_ISOLE::GetKsatBs() const
	{
		return m_ksat_bs;
	}

	float MILIEUHUMIDE_ISOLE::GetCEv() const
	{
		return m_c_ev;
	}

	float MILIEUHUMIDE_ISOLE::GetCProd() const
	{
		return m_c_prod;
	}

	size_t MILIEUHUMIDE_ISOLE::GetNbOccup() const
	{
		return m_occup.size();
	}

	void MILIEUHUMIDE_ISOLE::SetOccup(const std::vector<int>& occup)
	{
		//BOOST_ASSERT(occup.size() == m_occup.size());
		m_occup = occup;
	}

	float MILIEUHUMIDE_ISOLE::GetWetvol()
	{
		return m_wetvol;
	}

	void MILIEUHUMIDE_ISOLE::SetWetvol(float wetvol)
	{
		BOOST_ASSERT(wetvol >= 0);
		m_wetvol = wetvol;
	}

	float MILIEUHUMIDE_ISOLE::GetWetdnor() const
	{
		return m_wetdnor;
	}

	float MILIEUHUMIDE_ISOLE::GetWetdmax() const
	{
		return m_wetdmax;
	}

	float MILIEUHUMIDE_ISOLE::GetFrac() const
	{
		return m_frac;
	}

}
