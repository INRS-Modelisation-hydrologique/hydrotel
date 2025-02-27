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

#include "erreur.hpp"

#include <sstream>

#include <boost/filesystem.hpp>


using namespace std;


namespace HYDROTEL
{

	ERREUR::ERREUR(const std::string& message)
		: _message(message)
	{
	}


	ERREUR::~ERREUR() throw()
	{
	}


	const char* ERREUR::what() const throw()
	{
		return _message.c_str();
	}


	ERREUR_LECTURE_FICHIER::ERREUR_LECTURE_FICHIER(const std::string& nom_fichier)
		: ERREUR("")
	{
		_message.append("Error reading file: \"");
		_message.append(nom_fichier);
		_message.append("\"");
	}


	ERREUR_LECTURE_FICHIER::ERREUR_LECTURE_FICHIER(const std::string& nom_fichier, int ligne)
		: ERREUR("")
	{
		ostringstream ss;
		ss << "Error reading file: \"" << nom_fichier << "\" (line " << ligne << ")";
		_message = ss.str();
	}


	ERREUR_LECTURE_FICHIER::ERREUR_LECTURE_FICHIER(const std::string& nom_fichier, int ligne, const std::string& message)
		: ERREUR("")
	{
		ostringstream ss;
		ss << "Error reading file: \"" << nom_fichier << "\" (line " << ligne << ")";
		ss << " (" << message << ')';
		_message = ss.str();
	}


	ERREUR_LECTURE_FICHIER::~ERREUR_LECTURE_FICHIER() throw()
	{
	}


	ERREUR_ECRITURE_FICHIER::ERREUR_ECRITURE_FICHIER(const std::string& nom_fichier)
		: ERREUR("")
	{
		_message.append("Error writing to file: \"");
		_message.append(nom_fichier);
		_message.append("\"");
	}


	ERREUR_ECRITURE_FICHIER::~ERREUR_ECRITURE_FICHIER() throw()
	{
	}


	ERREUR_CREE_REPERTOIRES::ERREUR_CREE_REPERTOIRES(const std::string& repertoire)
		: ERREUR("")
	{
		_message.append("Error creating folder: \"");
		_message.append(repertoire);
		_message.append("\"");
	}


	ERREUR_CREE_REPERTOIRES::~ERREUR_CREE_REPERTOIRES() throw()
	{
	}

}

