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

#ifndef TRONCONS_H_INCLUDED
#define TRONCONS_H_INCLUDED


#include "troncon.hpp"
#include "noeuds.hpp"
#include "zones.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace HYDROTEL
{

	class SIM_HYD;

	class TRONCONS
	{
	public:
		TRONCONS();
		~TRONCONS();

		std::string PrendreNomFichier() const;

		std::string PrendreNomFichierPixels() const;

		size_t PrendreNbTroncon() const;

		const std::vector<TRONCON*>& PrendreTronconsExutoire() const;

		void ChangeNomFichier(const std::string& nom_fichier);

		void ChangeNomFichierPixels(const std::string& nom_fichier);

		void LectureTroncons(ZONES& zones, NOEUDS& noeuds, bool bUpdate = true);

		void LectureFichierPixels();

		void LectureFichierLargeur(std::string sFile);

		std::string CalculeLongueurTroncons();

		/// retourne le troncon a l'index
		TRONCON* operator[] (size_t index);

		/// recherche un troncon, retourne nullptr introuvable
		TRONCON* RechercheTroncon(int ident);

		/// recherhce un troncon, retourne nullptr introuvable
		TRONCON* RechercheTroncon(int ligne, int colonne);

		/// retourne l'index d'un troncon
		size_t IdentVersIndex(int ident) const;

		void CalculeShreve();
		void CalculeStrahler();


		SIM_HYD*								_pSimHyd;

		std::vector<std::shared_ptr<TRONCON>>	_troncons;

		std::vector<int>						_tronconNoeudAval;	//[indexTronxon]	id noeud aval du troncon

		size_t*									_pRasterTronconId;

		std::vector<std::string>				_listHydroStationReservoirHistory;
		std::vector<int>						_listHydroStationReservoirHistoryIdTroncon;


	private:
		void DetruireTroncons();

		std::shared_ptr<TRONCON> LectureRiviere(std::ifstream& fichier, ZONES& zones, NOEUDS& noeuds);
		std::shared_ptr<TRONCON> LectureLac(std::ifstream& fichier, ZONES& zones, NOEUDS& noeuds);
		std::shared_ptr<TRONCON> LectureLacSansLaminage(std::ifstream& fichier, ZONES& zones, NOEUDS& noeuds);
		std::shared_ptr<TRONCON> LectureBarrageHistorique(std::ifstream& fichier, ZONES& zones, NOEUDS& noeuds, int idTroncon);

		void LectureZoneAmont(std::ifstream& fichier, ZONES& zones, TRONCON* troncon);

		std::string _nom_fichier;
		std::string _nom_fichier_pixels;

		std::vector<TRONCON*> _troncons_exutoire;

		std::map<int, TRONCON*> _map;

		int _nb_colonne;
		std::map<int, TRONCON*> _pixels;

	};

}

#endif

