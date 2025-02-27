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

#include "station_meteo_hdf5.hpp"

#include "constantes.hpp"
#include "erreur.hpp"

#include <vector>


using namespace std;


namespace HYDROTEL
{

	STATION_METEO_HDF5::STATION_METEO_HDF5(hid_t hdid, const string& nom_fichier, size_t index)
		: STATION_METEO(nom_fichier)
		, _hdid(hdid)
		, _index(index)
	{
		auto dataset = H5Dopen(_hdid, "/meteo/date", H5P_DEFAULT);
		auto dataspace = H5Dget_space(dataset);
		auto rank = H5Sget_simple_extent_ndims(dataspace);

		hsize_t dims_out[2];
		auto status_n = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
		if (status_n < 0)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		_nb_donnees = static_cast<int>(dims_out[1]);

		// lire la premiere date
		hsize_t count[2] = { 4, 1 };
		hsize_t offset[2] = { 0, 0 };
		int status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
		if (status < 0)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		hsize_t dimsm[2] = { 4, 1 };
		auto memspace = H5Screate_simple(rank, dimsm, NULL);

		unsigned short data_out[4] = { 0 };
		status = H5Dread(dataset, H5T_NATIVE_USHORT, memspace, dataspace, H5P_DEFAULT, data_out);
		if (status < 0)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		_date_debut = DATE_HEURE(data_out[0], data_out[1], data_out[2], data_out[3]);

		// lire la deuxieme date
		offset[1] = 1;
		status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
		memspace = H5Screate_simple(rank, dimsm, NULL);
		status = H5Dread(dataset, H5T_NATIVE_USHORT, memspace, dataspace, H5P_DEFAULT, data_out);
		if (status < 0)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		H5Sclose(memspace);
		H5Sclose(dataspace);
		H5Dclose(dataset);

		// determine le pas de temps
		if (_date_debut.PrendreHeure() == 0 && data_out[3] == 0)
			_pas_de_temps = 24;
		else
			_pas_de_temps = data_out[3] - _date_debut.PrendreHeure();
	}


	STATION_METEO_HDF5::~STATION_METEO_HDF5()
	{
	}

	float LectureDataset(int dataset, int memspace, int dataspace, size_t index, size_t time)
	{
		static const hsize_t count[2] = { 1, 1 };
		hsize_t offset[2] = { index, time };
		float data_out;

		int status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
		if (status < 0)
			throw ERREUR("LectureDataset::H5Sselect_hyperslab");

		status = H5Dread(dataset, H5T_NATIVE_FLOAT, memspace, dataspace, H5P_DEFAULT, &data_out);
		if (status < 0)
			throw ERREUR("LectureDataset::H5Dread");

		return data_out;
	}

	void STATION_METEO_HDF5::ChangeInfoHDF5(int dataset_tmin, int dataspace_tmin, int memspace_tmin,
		int dataset_tmax, int dataspace_tmax, int memspace_tmax,
		int dataset_pr, int dataspace_pr, int memspace_pr)
	{
		_dataset_tmin = dataset_tmin;
		_dataspace_tmin = dataspace_tmin;
		_memspace_tmin = memspace_tmin;

		_dataset_tmax = dataset_tmax;
		_dataspace_tmax = dataspace_tmax;
		_memspace_tmax = memspace_tmax;

		_dataset_pr = dataset_pr;
		_dataspace_pr = dataspace_pr;
		_memspace_pr = memspace_pr;
	}

	void STATION_METEO_HDF5::LectureDonnees(const DATE_HEURE& /*debut*/, const DATE_HEURE& /*fin*/, unsigned short pas_de_temps)
	{
		if (pas_de_temps % _pas_de_temps)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		// NOTE: la fonction ne fait rien, la lecture se fait a chaque pas de temps
	}

	void STATION_METEO_HDF5::ChangeDonnees(const DONNEE_METEO& /*donnee_meteo*/, const DATE_HEURE& /*date_heure*/, unsigned short /*pas_de_temps*/)
	{
		// NOTE: le fichier est ouvert en lecture seulement
	}

	DONNEE_METEO STATION_METEO_HDF5::PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps)
	{
		DONNEE_METEO donnee_meteo;

		int index = _date_debut.NbHeureEntre(date_heure) / _pas_de_temps;

		int nb_pas = pas_de_temps / _pas_de_temps;

		float tmin = VALEUR_MANQUANTE;
		float tmax = VALEUR_MANQUANTE;
		float prec = VALEUR_MANQUANTE;

		for (int n = 0; n < nb_pas; ++n)
		{
			float minl = LectureDataset(_dataset_tmin, _memspace_tmin, _dataspace_tmin, _index, index + n);
			float maxl = LectureDataset(_dataset_tmax, _memspace_tmax, _dataspace_tmax, _index, index + n);			
			float pr = LectureDataset(_dataset_pr, _memspace_pr, _dataspace_pr, _index, index + n);

			if (n == 0)
			{
				tmin = minl;
				tmax = maxl;
				prec = pr;
			}
			else
			{
				tmin = min(tmin, minl);
				tmax = max(tmax, maxl);
				prec += pr;
			}
		}

		if(_iVersionThiessenMoy3Station == 1)
			donnee_meteo.ChangeTemperature_v1(tmin, tmax);
		else
			donnee_meteo.ChangeTemperature(tmin, tmax);

		donnee_meteo.ChangePluie(prec);
		donnee_meteo.ChangeNeige(0);

		return donnee_meteo;
	}

	std::pair<float, float> STATION_METEO_HDF5::PrendreTemperatureJournaliere(const DATE_HEURE& date_heure)
	{
		DATE_HEURE date(date_heure.PrendreAnnee(), date_heure.PrendreMois(), date_heure.PrendreJour(), 0);

		int nb_pas = 24 / _pas_de_temps;

		auto donnees = PrendreDonnees(date, 24);

		float tmin = donnees.PrendreTMin();
		float tmax = donnees.PrendreTMax();

		for (int n = 1; n < nb_pas; ++n)
		{
			date.AdditionHeure(_pas_de_temps);
			donnees = PrendreDonnees(date, _pas_de_temps);

			if(tmin <= VALEUR_MANQUANTE)
				tmin = donnees.PrendreTMin();
			else
			{
				if(donnees.PrendreTMin() > VALEUR_MANQUANTE)
					tmin = min(tmin, donnees.PrendreTMin());
			}

			if(tmax <= VALEUR_MANQUANTE)
				tmax = donnees.PrendreTMax();
			else
			{
				if(donnees.PrendreTMax() > VALEUR_MANQUANTE)
					tmax = max(tmax, donnees.PrendreTMax());
			}
		}

		if(tmin <= VALEUR_MANQUANTE || tmax <= VALEUR_MANQUANTE)
			tmin = tmax = VALEUR_MANQUANTE;

		return make_pair(tmin, tmax);
	}

}
