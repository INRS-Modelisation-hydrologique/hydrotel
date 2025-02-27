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

#ifndef DONNEE_METEO_H_INCLUDED
#define DONNEE_METEO_H_INCLUDED


namespace HYDROTEL
{

	enum TYPE_DONNEES_METEO
	{
		TYPE_DONNEES_METEO_TEMPERATURE_MINIMUM		= 1,
		TYPE_DONNEES_METEO_TEMPERATURE_MAXIMUM		= 2,
		TYPE_DONNEES_METEO_PLUIE					= 4,
		TYPE_DONNEES_METEO_NEIGE					= 8,
		TYPE_DONNEES_METEO_VENT						= 16,
		TYPE_DONNEES_METEO_RAYONNEMENT_SOLAIRE		= 32,
		TYPE_DONNEES_METEO_TEMPERATURE_MINIMUM_JOUR = 64,
		TYPE_DONNEES_METEO_TEMPERATURE_MAXIMUM_JOUR = 128,
	};

	class DONNEE_METEO
	{
	public:
		DONNEE_METEO();
		~DONNEE_METEO();

		// retourne la temperature minimum (C)
		float PrendreTMin() const;

		// retourne la temperature maximum (C)
		float PrendreTMax() const;

		// retourne la pluie (mm)
		float PrendrePluie() const;

		// retourne la neige (mm)
		float PrendreNeige() const;

		// change les temperatures min et max (C)
		void ChangeTemperature(float min, float max);
		void ChangeTemperature_v1(float min, float max);	//pour modele THIESSEN1 et MOYENNE_3_STATIONS1

		// change la pluie (mm)
		void ChangePluie(float pluie);

		// change la neige (mm)
		void ChangeNeige(float neige);

	private:
		float _tmin;		// C
		float _tmax;		// C
		float _pluie;		// mm
		float _neige;		// mm
		//float _rayonnement;
		//float _vent;
		//int _nb_heure;	// h
	};

}

#endif
