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

#ifndef MILIEU_HUMIDE_ISOLE_H_INCLUDE
#define MILIEU_HUMIDE_ISOLE_H_INCLUDE


#include <vector>
#include <cstddef>


namespace HYDROTEL
{

	class MILIEUHUMIDE_ISOLE
	{
	public:
		MILIEUHUMIDE_ISOLE(float superficie, float wetfr, float wetdrafr, float frac, 
			               float wetdnor, float wetdmax, bool sauvegarde, float ksat_bs, float c_ev, float c_prod, float eauIni);

		virtual ~MILIEUHUMIDE_ISOLE();

		bool GetSauvegarde() const;

		float GetSuperficie() const;
		float GetWetfr() const;
		float GetWetdrafr() const;
		float GetFrac() const;
		float GetWetdnor() const;
		float GetWetdmax() const;
		float GetWetmxsa() const;
		float GetWetmxvol() const;
		float GetWetnvol() const;
		float GetA() const;
		float GetB() const;
	
		float GetKsatBs() const;
		float GetCEv() const;
		float GetCProd() const;

		size_t GetNbOccup() const;

		void SetOccup(const std::vector<int>& occup);

		float GetWetvol();
		void  SetWetvol(float wetvol);

	private:
		float m_superficie;			// km^2
		float m_wetfr;
		float m_wetdrafr;
		float m_frac;
		float m_wetdnor;
		float m_wetdmax;
		bool  m_sauvegarde;
		float m_ksat_bs;
		float m_c_ev;
		float m_c_prod;

	public:
		float m_eauIni;			//[0-1] fraction de la hauteur normale à utiliser pour déterminer le niveau d'eau initial du milieu humide

	private:
		float m_wetmxsa;			// m^2
		float m_wetmxvol;			// m^3
		float m_wetnvol;			// m^3
		float m_a;
		float m_b;
		float m_wetvol;				// m^3

		std::vector<int> m_occup;	// pixels
	};

}

#endif
