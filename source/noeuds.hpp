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

#ifndef NOEUDS_H_INCLUDED
#define NOEUDS_H_INCLUDED


#include "noeud.hpp"

#include <map>
#include <string>
#include <vector>


namespace HYDROTEL
{

	class NOEUDS
	{
	public:
		NOEUDS();
		~NOEUDS();

		std::string PrendreNomFichier() const;

		size_t PrendreNbNoeud() const;

		/// retourne l'index du noeud exutoire
		size_t PrendreIndexExutoire() const;

		/// change le nom de fichier
		void ChangeNomFichier(const std::string& nom_fichier);

		/// lecture du fichier
		void Lecture();

		/// recherche un noeud, nullptr si introuvable
		NOEUD* Recherche(int ident);

		/// retourne le noeud a l'index
		NOEUD& operator[](size_t index);

	private:
		void DetruireNoeuds();

		std::string _nom_fichier;
		std::vector<NOEUD> _noeuds;
		size_t _exutoire;

		std::map<int, NOEUD*> _map;
	};

}

#endif
