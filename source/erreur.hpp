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

#ifndef ERREUR_H_INCLUDED
#define ERREUR_H_INCLUDED


#include <stdexcept>


namespace HYDROTEL
{

	class ERREUR : public std::exception
	{
	public:
		ERREUR(const std::string& message);
		virtual ~ERREUR() throw();

		/// retourne le message d'erreur
		virtual const char* what() const throw();

	protected:
		std::string _message;
	};


	class ERREUR_LECTURE_FICHIER : public ERREUR
	{
	public:
		ERREUR_LECTURE_FICHIER(const std::string& nom_fichier);
		ERREUR_LECTURE_FICHIER(const std::string& nom_fichier, int ligne);
		ERREUR_LECTURE_FICHIER(const std::string& nom_fichier, int ligne, const std::string& message);

		virtual ~ERREUR_LECTURE_FICHIER() throw();
	};


	class ERREUR_ECRITURE_FICHIER : public ERREUR
	{
	public:
		ERREUR_ECRITURE_FICHIER(const std::string& nom_fichier);
		virtual ~ERREUR_ECRITURE_FICHIER() throw();
	};


	class ERREUR_CREE_REPERTOIRES : public ERREUR
	{
	public:
		ERREUR_CREE_REPERTOIRES(const std::string& repertoire);
		virtual ~ERREUR_CREE_REPERTOIRES() throw();
	};

}

#endif
