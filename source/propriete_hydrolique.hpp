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

#ifndef PROPRIETE_HYDROLIQUE_H_INCLUDED
#define PROPRIETE_HYDROLIQUE_H_INCLUDED


#include <string>


namespace HYDROTEL
{

	class PROPRIETE_HYDROLIQUE
	{
	public:
		PROPRIETE_HYDROLIQUE();

		PROPRIETE_HYDROLIQUE(const std::string& nom, float thetas, float thetapf, float thetacc,
			float ks, float lambda, float psis, float alpha);

		~PROPRIETE_HYDROLIQUE();

		// retourne le nom de la propriete
		std::string PrendreNom() const;

		float PrendreThetas() const;

		float PrendreThetapf() const;

		float PrendreThetacc() const;

		float PrendreKs() const;

		float PrendreLambda() const;

		float PrendrePsis() const;

		float PrendreAlpha() const;

	private:
		std::string _nom;
		float _thetas;			// teneur en eau a saturation
		float _thetapf;			// teneur en eau au point de fletrissement
		float _thetacc;			// teneur en eau a la capacite au champs
		float _ks;				// conductivite hydrolique a saturation (m/h)
		float _lambda;			// indice de distribution de la grosseur des pores
		float _psis;			// potentielle au voisinage a la saturation, equ. Campbell (m)
		float _alpha;			// parametre d'ajustement de la courbe de variation de kat et kas
								// en fonction de la teneur en eau relative par rapport a la reserve utile
	};

}

#endif
