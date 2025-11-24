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

#include "noeuds.hpp"

#include "erreur.hpp"
#include "util.hpp"

#include <fstream>
#include <sstream>


using namespace std;


namespace HYDROTEL
{

	NOEUDS::NOEUDS()
	{
	}

	NOEUDS::~NOEUDS()
	{
	}

	
	//------------------------------------------------------------------------------------------------
	void NOEUDS::Lecture()
	{
		DetruireNoeuds();

		ifstream stream(_nom_fichier);
		if (!stream)
			throw ERREUR_LECTURE_FICHIER("FICHIER NOEUDS; " + _nom_fichier);

		int type;
		size_t nb_noeud;
		string commentaire, str;
		istringstream iss;

		getline_mod(stream, str);
		iss.clear();
		iss.str(str);
		iss >> type;

		getline_mod(stream, str);
		iss.clear();
		iss.str(str);
		iss >> nb_noeud;

		getline_mod(stream, commentaire);

		vector<NOEUD> noeuds(nb_noeud);
		vector<string> sList;
		float alt = 0;

		for (size_t index = 0; index < nb_noeud; ++index)
		{
			int id;
			float x, y, altitude, largeur;

			getline_mod(stream, str);
			SplitString(sList, str, " \t", true, true);	//separateur; espace et \t
			if(sList.size() != 5)
				throw ERREUR_LECTURE_FICHIER("FICHIER NOEUDS; " + _nom_fichier);
			
			iss.clear();
			iss.str(sList[0]);
			iss >> id;

			iss.clear();
			iss.str(sList[1]);
			iss >> x;

			iss.clear();
			iss.str(sList[2]);
			iss >> y;

			iss.clear();
			iss.str(sList[3]);
			iss >> altitude;

			iss.clear();
			iss.str(sList[4]);
			iss >> largeur;

			NOEUD noeud;

			noeud.ChangeIdent(id);
			noeud.ChangeCoordonnee(COORDONNEE(x, y, altitude));

			noeuds[index] = noeud;

			if (altitude < alt || index == 0)
			{
				alt = altitude;
				_exutoire = index;
			}
		}

		_noeuds.swap(noeuds);

		for (size_t index = 0; index < nb_noeud; ++index)
		{
			_map[_noeuds[index].PrendreIdent()] = &_noeuds[index];
		}		
	}

	void NOEUDS::ChangeNomFichier(const string& nom_fichier)
	{
		//DetruireNoeuds();
		_nom_fichier = nom_fichier;
	}

	void NOEUDS::DetruireNoeuds()
	{
		_noeuds.clear();
		_map.clear();
	}

	size_t NOEUDS::PrendreNbNoeud() const
	{
		return _noeuds.size();
	}

	string NOEUDS::PrendreNomFichier() const
	{
		return _nom_fichier;
	}

	size_t NOEUDS::PrendreIndexExutoire() const
	{
		return _exutoire;
	}

	NOEUD& NOEUDS::operator[](size_t index)
	{
		BOOST_ASSERT(index < PrendreNbNoeud());
		return _noeuds[index];
	}

	NOEUD* NOEUDS::Recherche(int ident)
	{
		return _map.find(ident) == _map.end() ? nullptr : _map[ident];
	}

}
