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

#ifndef MILIEU_HUMIDE_RIVERAIN_H_INCLUDED
#define MILIEU_HUMIDE_RIVERAIN_H_INCLUDED


namespace HYDROTEL
{

	class MILIEUHUMIDE_RIVERAIN
	{
	public:
		MILIEUHUMIDE_RIVERAIN(float wetarea, float wetaup, float wetadra, float wetdown, 
							  float longueur, float amont, float aval, 
							  float wetdnor, float wetdmax, float frac,
							  bool sauvegarde, float ksat_bk, float ksat_bs, float th_aq, float eauIni);

		virtual ~MILIEUHUMIDE_RIVERAIN();

		bool get_sauvegarde() const;

		float get_wetarea() const;
		float get_wetaup() const;
		float get_wetadra() const;
		float get_wetdown() const;

		float get_wetmxvol() const;
		float get_wetmxsa() const;
		float get_wetmxd() const;
	
		float get_wetnvol() const;
		float get_wetnsa() const;
		float get_wetnd() const;

		float get_alfa() const;
		float get_belta() const;

		float get_wet_v() const;
		float get_wet_a() const;
		float get_wet_d() const;

		float get_longueur() const;
		float get_longueur_amont() const;
		float get_longueur_aval() const;
		float get_frac() const;

		float get_ksat_bk() const; 
		float get_ksat_bs() const;
		float get_th_aq() const;

		void set_wet_v(float wet_v);
		void set_wet_a(float wet_a);
		void set_wet_d(float wet_d);

		//void set_wetchn_q(float wetchn_q);
		//void set_wetchn_d(float wetchn_d);

		//float get_wetchn_d();

	private:
		float m_wetarea;	// riparian wetland area
		float m_wetaup;		// drainage area upstream of the wetland
		float m_wetadra;	// drainage area to the riparian wetland
		float m_wetdown;	// drainage area downstream of the wetland
		float m_longueur;	// longueur		m
		float m_lgrAmont;	// longueur		m
		float m_lgrAval;	// longueur		m
		float m_wetnd;  	// normal depth		m
		float m_wetmxd;		// max depth	m
		float m_frac;
		bool  m_sauvegarde;
		float m_ksat_bk;
		float m_ksat_bs;
		float m_th_aq;

	public:
		float m_eauIni;			//[0-1] fraction de la hauteur normale à utiliser pour déterminer le niveau d'eau initial du milieu humide

	private:
		float m_wetmxvol;	// max volume	m^3
		float m_wetmxsa;	// max surface	m^2		
	
		float m_wetnvol;	// normal volume	m^3
		float m_wetnsa;		// normal surface	m^2		

		float m_alfa;
		float m_belta;

		float m_wet_v;		// volume		m^3
		float m_wet_a;		// aire			m^2
		float m_wet_d;		// profondeur	m
	};

}

#endif

