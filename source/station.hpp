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

#ifndef STATION_H_INCLUDED
#define STATION_H_INCLUDED


#include "coordonnee.hpp"


namespace HYDROTEL
{

	class STATION
	{
	public:

		STATION(const std::string& nom_fichier);
		virtual ~STATION() = 0;

		std::string PrendreIdent() const;

		std::string PrendreNom() const;

		/// retourne le nom de fichier de donnee
		std::string PrendreNomFichier() const;

		const COORDONNEE& PrendreCoordonnee() const;

		void ChangeIdent(const std::string& ident);

		void ChangeNom(const std::string& nom);

		void ChangeCoordonnee(const COORDONNEE& coordonnee);

		/// change le nom de fichier des donnees de la station
		void ChangeNomFichier(const std::string& nom_fichier);

	public:

		int				_iVersionThiessenMoy3Station;

		COORDONNEE		_coordonneeCRSprojet;	//coordinate of the station (coordinate system of the project)

	protected:

		std::string		_ident;
		std::string		_nom;
		std::string		_nom_fichier;
		COORDONNEE		_coordonnee;	//coordinate of the station (original coordinate system in stations data files)
	};

}

#endif 

