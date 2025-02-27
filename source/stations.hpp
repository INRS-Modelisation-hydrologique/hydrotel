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

#ifndef STATIONS_H_INCLUDED
#define STATIONS_H_INCLUDED


#include "station.hpp"
#include "projection.hpp"

#include <map>
#include <memory>
#include <vector>


namespace HYDROTEL
{

	class STATIONS
	{
	public:
		STATIONS();
		~STATIONS();

		std::string PrendreNomFichier() const;

		const PROJECTION& PrendreProjection() const;

		size_t PrendreNbStation() const;

		void ChangeNomFichier(const std::string& nom_fichier);

		void ChangeProjection(const PROJECTION& projection);

		/// retourne la station a l'index
		STATION* operator[] (size_t index) const;

		/// retourne la station ou nullptr si elle n'existe pas
		STATION* Recherche(const std::string& ident);

	protected:
		void Detruire();
		void CreeMapRecherche();

		std::string _nom_fichier;
		PROJECTION _projection;

		std::vector<std::shared_ptr<STATION>> _stations;
		std::map<std::string, STATION*> _map;
	};

}

#endif
