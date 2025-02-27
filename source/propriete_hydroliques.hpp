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

#ifndef PROPRIETE_HYDROLIQUES_H_INCLUDED
#define PROPRIETE_HYDROLIQUES_H_INCLUDED


#include "propriete_hydrolique.hpp"
#include "zones.hpp"

#include <vector>


namespace HYDROTEL
{

	class SIM_HYD;	// forward class declaration

	class PROPRIETE_HYDROLIQUES
	{
	public:
		PROPRIETE_HYDROLIQUES();
		~PROPRIETE_HYDROLIQUES();

		void ChangeNomFichier(const std::string& nom_fichier);

		void ChangeNomFichierCouche1(const std::string& nom_fichier);

		void ChangeNomFichierCouche2(const std::string& nom_fichier);

		void ChangeNomFichierCouche3(const std::string& nom_fichier);

		std::string PrendreNomFichier() const;

		std::string PrendreNomFichierCouche1() const;

		std::string PrendreNomFichierCouche2() const;

		std::string PrendreNomFichierCouche3() const;

		PROPRIETE_HYDROLIQUE& Prendre(size_t index);

		PROPRIETE_HYDROLIQUE& PrendreProprieteHydroliqueCouche1(size_t index_zone);

		PROPRIETE_HYDROLIQUE& PrendreProprieteHydroliqueCouche2(size_t index_zone);

		PROPRIETE_HYDROLIQUE& PrendreProprieteHydroliqueCouche3(size_t index_zone);

		size_t PrendreIndexCouche1(size_t index_zone) const;

		size_t PrendreIndexCouche2(size_t index_zone) const;

		size_t PrendreIndexCouche3(size_t index_zone) const;

		// retourne le nombre de propriete hydrolique
		size_t PrendreNb() const;

		// lecture du fichier
		void Lecture(SIM_HYD& sim_hyd);

	private:
		bool LectureProprieteHydrolique();
		void LectureCouches(SIM_HYD& sim_hyd);
		void LectureCouche(const std::string& nom_fichier, std::vector<int>& couche, SIM_HYD& sim_hyd);

	public:
		bool								_bDisponible;
		std::vector<int>					_coefficient_additif;	//coefficient additif pour chaque groupe d'UHRH

	private:
		std::string							_nom_fichier;

		std::string							_nom_fichier_couche1;
		std::string							_nom_fichier_couche2;
		std::string							_nom_fichier_couche3;
		
		std::vector<PROPRIETE_HYDROLIQUE>	_propriete_hydroliques;
		
		std::vector<int>					_couche1;
		std::vector<int>					_couche2;
		std::vector<int>					_couche3;
	};

}

#endif
