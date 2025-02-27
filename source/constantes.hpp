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

#ifndef CONSTANTES_H_INCLUDED
#define CONSTANTES_H_INCLUDED


namespace HYDROTEL 
{

	const float		VALEUR_MANQUANTE						= -999.0f;
	const double	dVALEUR_MANQUANTE						= -999.0;

	const float		DENSITE_EAU								= 1000.0f;				//kg/m3
	const double	dDENSITE_EAU							= 1000.0;

	const float		CHALEUR_FONTE							= 335000.0f;			//j/kg		//CHALEUR LATENTE DE FUSION	
	const float		CHALEUR_NEIGE							= 2093.4f;
	const float		CHALEUR_EAU								= 4184.0f;

	const double	dCHALEUR_FONTE							= 335000.0;				//j/kg		//CHALEUR LATENTE DE FUSION
	const double	dCHALEUR_NEIGE							= 2093.4;
	const double	dCHALEUR_EAU							= 4184.0;

	const float		RAD1									= 57.295779513f;		// 1 radian en degre
	const double	dRAD1									= 57.295779513078550;

	const float		DEG1									= 58.1313429644f;		// 1 jour en degre
	const double	dDEG1									= 58.1313429643110;

	const float		BETA									= 1.1f;

	const int		MAXITER									= 20;					// maximum d'iterations
	const float		EPSILON									= 0.0001f;				// valeur minimale

	const float		PI										= 3.141592f;
	const double	dPI										= 3.14159265359;

	const float		RHO										= 1.2923f;				// densite de l'aire en kg/m**3
	const float		LE										= 2466000.0f;			// chaleur de vaporisation de l'eau en J/kg a 15C
	const float		RD										= 287.04f;				// constante des gaz en J/kg*K
	const float		K										= 0.41f;				// constante de Von Karman
	const float		K2										= 0.1681f;				// constante de Von Karman au carre
	
	const float		C1_ES									= 0.622f * LE / RD;		// constante d'estimation de ES

	const float		GAMMA									= 0.6638f;				// constante psychometrique en mb/K
	const float		SIGMA									= 0.004903f;			// constante de Stephen Boltzman	//J/m*m*K(-4)*j		//J/JOUR/M²/K4

	const float		A0										= 6984.505294f;
	const float		A1										= -188.903931f;
	const float		A2										= 2.133357679f;
	const float		A3										= -1.288580973E-2f;
	const float		A4										= 4.393587233E-5f;
	const float		A5										= -8.023923082E-8f;
	const float		A6										= 6.136820929E-11f;

	const float		CONSTANTE_SOLAIRE						= 1360.8f;				// W/m²		//2008
	const double	dCONSTANTE_SOLAIRE						= 1360.8;				// W/m²		//2008
	
	const float		EXENTRICITE_ORBITE_TERRESTRE			= 0.01671022f;
	const double	dEXENTRICITE_ORBITE_TERRESTRE			= 0.01671022;

	const int		JOUR_EQUINOXE_VERNAL					= 80;					// jour julien
	const float		CHALEUR_LATENTE_VAPORISATION			= 2.45f;				// MJ/kg
	const float		CHALEUR_SPECIFIQUE_A_PRESSION_CONSTANTE	= 0.001013f;			// MJ/kg/dC

}

#endif
