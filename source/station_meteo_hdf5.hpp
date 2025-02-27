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

#ifndef STATION_METEO_HDF5_INCLUDED
#define STATION_METEO_HDF5_INCLUDED


#include "station_meteo.hpp"

#include <hdf5.h>


namespace HYDROTEL
{

	class STATION_METEO_HDF5 : public STATION_METEO
	{
	public:
		STATION_METEO_HDF5(hid_t hdid, const std::string& nom_fichier, size_t index);
		virtual ~STATION_METEO_HDF5();

		virtual void LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps);

		virtual DONNEE_METEO PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual void ChangeDonnees(const DONNEE_METEO& donnee_meteo, const DATE_HEURE& date_heure, unsigned short pas_de_temps);

		virtual std::pair<float, float> PrendreTemperatureJournaliere(const DATE_HEURE& date_heure);

		void ChangeInfoHDF5(
			int dataset_tmin, int dataspace_tmin, int memspace_tmin,
			int dataset_tmax, int dataspace_tmax, int memspace_tmax,
			int dataset_pr, int dataspace_pr, int memspace_pr);

	private:
		hid_t _hdid;

        int _dataset_tmin;
        int _dataspace_tmin;
        int _memspace_tmin;

        int _dataset_tmax;
        int _dataspace_tmax;
        int _memspace_tmax;

        int _dataset_pr;
        int _dataspace_pr;
        int _memspace_pr;

		size_t _index;
		DATE_HEURE _date_debut;
		unsigned short _pas_de_temps; //pas de temps du fichier
		int _nb_donnees;
	};

}

#endif
