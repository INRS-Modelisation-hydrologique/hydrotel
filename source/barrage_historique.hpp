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

#ifndef BARRAGE_HISTORIQUE_H_INCLUDED
#define BARRAGE_HISTORIQUE_H_INCLUDED


#include "troncon.hpp"
#include "station_hydro.hpp"


namespace HYDROTEL
{

	class ACHEMINEMENT_RIVIERE;		//forward declaration

	class BARRAGE_HISTORIQUE : public TRONCON
	{
	friend ACHEMINEMENT_RIVIERE;

	public:
		BARRAGE_HISTORIQUE();
		virtual ~BARRAGE_HISTORIQUE();

		/// retourne l'identificateur de la station associee
		std::string PrendreIdentStationHydro() const;

		/// change l'identificateur de la station associee
		void ChangeIdentStationHydro(const std::string& ident_station_hydro);

		/// retourne le debit (m3/s)
		float PrendreDebit(DATE_HEURE date_heure, unsigned short pas_de_temps) const;

	private:
		void ChangeStationHydro(STATION_HYDRO* station_hydro);

		std::string _ident_station_hydro;
		STATION_HYDRO* _station_hydro;
	};

}

#endif
