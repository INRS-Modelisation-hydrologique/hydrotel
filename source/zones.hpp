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

#ifndef ZONES_H_INCLUDED
#define ZONES_H_INCLUDED


#include "raster.hpp"
#include "raster_int2.hpp"
#include "zone.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace HYDROTEL 
{

	class ZONES
	{
	public:

		ZONES();
		~ZONES();

		// retourne le nom de fichier de la matrice des zones
		std::string PrendreNomFichierZone() const;

		// retourne le nom de fichier de la matrice d'altitude
		std::string PrendreNomFichierAltitude() const;

		// retourne le nom de fichier de la matrice de pente
		std::string PrendreNomFichierPente() const;

		// retourne le nom de fichier de la matrice d'orientation
		std::string PrendreNomFichierOrientation() const;

		size_t PrendreNbZone() const;

		// retourne la projection des matrices
		const PROJECTION& PrendreProjection() const;

		// retourne le coin inferieur gauche des matrices
		const COORDONNEE& PrendreCoordonnee() const;

		// retourne la resolution des matrices (m)
		float PrendreResolution() const;

		// retourne la grille des zones
		const RASTER<int>& PrendreGrille() const;

		// change le nom de fichier de la matrice des zones
		void ChangeNomFichierZone(const std::string& nom_fichier);

		// change le nom de fichier de la matrice des altitudes
		void ChangeNomFichierAltitude(const std::string& nom_fichier);

		// change le nom de fichier de la matrice des pentes
		void ChangeNomFichierPente(const std::string& nom_fichier);

		// change le nom de fichier de la matrice des orientations
		void ChangeNomFichierOrientation(const std::string& nom_fichier);

		// lecture du resumer, ou creation du fichier resumer
		void LectureZones();

		// retourne la zone a l'index
		ZONE& operator[] (size_t index);

		// retourne la zone a l'index
		const ZONE& operator[] (size_t index) const;

		// recherche une zone
		ZONE* Recherche(int ident);

		// retourne l'index de la zone	//ident recu en parametre doit etre positif pour les lacs
		size_t IdentVersIndex(int ident) const;

	public:

		void SauvegardeResumer(const std::string& nom_fichier);

		std::vector<size_t>		_vIdentVersIndex;
		std::string				_nom_fichier_zoneTemp;
		std::string				_nom_fichier_zone;

		RasterInt2*				_pRasterUhrhId;

		bool					_bSaveUhrhCsvFile;		//la sauvegarde (SauvegardeResumer) doit etre effectue apres la lecture des troncons (type zone)

	private:

		void DetruireZones();
		void LectureResumerCsv(const std::string& nom_fichier);
		void LectureResumerRsm(const std::string& nom_fichier);
		void CalculResumer();

		std::string _nom_fichier_altitude;
		std::string _nom_fichier_pente;
		std::string _nom_fichier_orientation;

		std::vector<std::shared_ptr<ZONE>> _zones;

		RASTER<int> _grille;

		std::map<int, ZONE*> _map;
	};

}

#endif
