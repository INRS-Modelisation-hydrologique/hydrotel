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

#ifndef TRONCON_H_INCLUDED
#define TRONCON_H_INCLUDED


#include "types.hpp"
#include "noeud.hpp"

#include <string>
#include <vector>


namespace HYDROTEL 
{

	class ZONE;

	class TRONCON
	{
	public:
		enum TYPE_TRONCON
		{
			RIVIERE,
			LAC,
			LAC_SANS_LAMINAGE,
			BARRAGE_HISTORIQUE,
		};

		TRONCON(TRONCON::TYPE_TRONCON type_troncon);
		virtual ~TRONCON() = 0;

		// retourne l'identifiant (1 a x)
		int PrendreIdent() const;
		
		// retourne le type du troncon
		TRONCON::TYPE_TRONCON PrendreType() const;

		const std::vector<NOEUD*>& PrendreNoeudsAval() const;

		const std::vector<NOEUD*>& PrendreNoeudsAmont() const;

		std::vector<TRONCON*> PrendreTronconsAval() const;

		std::vector<TRONCON*> PrendreTronconsAmont() const;

		std::vector<ZONE*> PrendreZonesAmont() const;

		float PrendreSurfaceDrainee() const;

		// retourne l'apport lateral (m3/s)
		float PrendreApportLateral() const;

		// retourne le debit amont (m3/s)
		float PrendreDebitAmont() const;
		float PrendreDebitAmontMoyen() const;

		// retourne le debit aval (m3/s)
		float PrendreDebitAval() const;
		float PrendreDebitAvalMoyen() const;

		// change l'identifiant
		void ChangeIdent(int ident);
		
		void ChangeNoeudsAval(std::vector<NOEUD*>& noeuds_aval);

		void ChangeNoeudsAmont(std::vector<NOEUD*>& noeuds_amont);

		void ChangeZonesAmont(std::vector<ZONE*>& zones_amont);

		void ChangeTronconsAval(std::vector<TRONCON*>& troncon_aval);

		void ChangeTronconsAmont(std::vector<TRONCON*>& troncons_amont);

		// change l'apport lateral (m3/s)
		void ChangeApportLateral(float apport);

		// change le debit aval (m3/s)
		void ChangeDebitAval(float debit);
		void ChangeDebitAvalMoyen(float debitMoyen);

		// change le debit amont (m3/s)
		void ChangeDebitAmont(float debit);
		void ChangeDebitAmontMoyen(float debitMoyen);

		double	_hauteurAvalMoy;			//hauteur d'eau en aval du troncon (moyenne des pas de temps interne)	//m

		float	_surf;
		float	_hypo;
		float	_base;

		//prelevement
		double	_prPrelevementTotal;	//prelevement gpe + pr + elevage				//m3/s		//pour le pas de temps courant
		double	_prPrelevementCulture;	//prelevement cultures							//m3/s		//

		double	_prRejetTotal;			//rejets totaux excluant les rejets effluents	//m3/s		//
		double	_prRejetEffluent;		//rejets effluents								//m3/s		//

		double  _prIndicePression;														//0-1		//

		//DEBITS AVAL MOY7J MIN
		std::vector<float>	_debit_aval_7jrs;	//m3/s		//valeurs du jour courant et des 6 derniers jours (7 jours)

		int						_ident;

		TYPE_TRONCON			_type_troncon;

		std::vector<NOEUD*>		_noeuds_aval;
		std::vector<NOEUD*>		_noeuds_amont;

		std::vector<ZONE*>		_zones_amont;

		std::vector<TRONCON*>	_troncons_aval;
		std::vector<TRONCON*>	_troncons_amont;

		//variables de simulations

		float					_apport_lateral;	// m3/s

		float					_debit_aval;		// m3/s		//pas de temps interne du modele 'acheminement riviere'
		float					_debit_amont;		// m3/s		//pas de temps interne du modele 'acheminement riviere'

		float					_debit_amont_moyen;	// m3/s		//pas de temps externe
		float					_debit_aval_moyen;	// m3/s		//pas de temps externe

		//shreve method of stream ordering
		int						_iSchreve;	//[1 - x]

	};

}

#endif

