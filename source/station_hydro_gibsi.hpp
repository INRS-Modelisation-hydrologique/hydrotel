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

#ifndef STATION_HYDRO_GIBSI_H_INCLUDED
#define STATION_HYDRO_GIBSI_H_INCLUDED


#include "station_hydro.hpp"
#include "date_heure.hpp"


namespace HYDROTEL
{

	class STATION_HYDRO_GIBSI : public STATION_HYDRO
	{
	public:
		STATION_HYDRO_GIBSI(const std::string& nom_fichier);
		virtual ~STATION_HYDRO_GIBSI();

		// lecture des donnees hydro
		virtual void LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);

		// retourne le debit (m3/s)
		virtual float PrendreDebit(DATE_HEURE date_heure, unsigned short pas_de_temps) const;

	private:
		std::vector<float> _debits; // m3/s
		DATE_HEURE _date_debut;
	};

}

#endif
