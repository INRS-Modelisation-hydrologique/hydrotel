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

#ifndef STATION_NEIGE_H_INCLUDED
#define STATION_NEIGE_H_INCLUDED


#include "station.hpp"
#include "date_heure.hpp"

#include <map>


namespace HYDROTEL
{

	class STATION_NEIGE : public STATION
	{
	public:
	
		enum typeOccupationStation {RESINEUX=0,FEUILLUS=1,DECOUVERTE=2};	

		struct DONNEE_NEIGE
		{
			float hauteur;	    // m
			float equi_eau;		// m
			float densite;		// pourcentage

			DONNEE_NEIGE()
				: hauteur(0)
				, equi_eau(0)
				, densite(0)
			{
			}

			DONNEE_NEIGE(float hauteur, float equi_eau, float densite)
				: hauteur(hauteur)
				, equi_eau(equi_eau)
				, densite(densite)
			{				
			}
		};

		STATION_NEIGE(const std::string& nom_fichier);
		virtual ~STATION_NEIGE();

		virtual void LectureDonnees(const DATE_HEURE& date_debut, const DATE_HEURE& date_fin) = 0;

		// determine s'il y a des donnees observes a une date
		bool DonneeDisponible(const DATE_HEURE& date);

		// retourne les donnees observes pour une date
		DONNEE_NEIGE PrendreDonnee(const DATE_HEURE& date);

	public:
		typeOccupationStation occupation;	//pour maj grille neige

	protected:
		std::map<DATE_HEURE, DONNEE_NEIGE> _donnees;
	};

}

#endif
