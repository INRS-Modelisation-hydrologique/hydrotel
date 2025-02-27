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

#ifndef GROUPE_ZONE_H_INCLUDED
#define GROUPE_ZONE_H_INCLUDED


#include <string>
#include <vector>


namespace HYDROTEL
{

	class GROUPE_ZONE
	{
	public:
		GROUPE_ZONE();
		~GROUPE_ZONE();

		/// retourne le nom du groupe
		std::string PrendreNom() const;

		/// retourne le nombre de zones du groupe
		size_t PrendreNbZone() const;

		/// retourne l'ident de la zone
		int PrendreIdent(size_t index) const;

		/// change le nom du groupe
		void ChangeNom(const std::string& nom);

		/// change les id des zones du groupe
		void ChangeIdentZones(const std::vector<int>& ident_zones);

	private:
		std::string _nom;

	public:
		std::vector<int> _ident_zones;
	};

}

#endif
