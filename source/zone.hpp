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

#ifndef ZONE_H_INCLUDED
#define ZONE_H_INCLUDED


#include "types.hpp"
#include "troncon.hpp"

#include <vector>


namespace HYDROTEL 
{

	class ZONE
	{
	public:
		enum TYPE_ZONE 
		{ 
			SOUS_BASSIN, 
			LAC,
		};

		ZONE();
		~ZONE();

		/// retourne l'identifiant de la zone
		int PrendreIdent() const;

		TYPE_ZONE PrendreTypeZone() const;

		/// retourne le nombre de pixel de la zone
		size_t PrendreNbPixel() const;

		/// retourne la superficie (km2)
		double PrendreSuperficie() const;

		/// retourne l'altitue moyenne (m)
		float PrendreAltitude() const;

		/// retourne la pente moyenne de la zone [ratio]
		float PrendrePente() const;

		/// retourne l'orientation moyenne de la zone
		ORIENTATION PrendreOrientation() const;

		/// retourne le centroide de la zone (long/lat wgs84)
		COORDONNEE PrendreCentroide() const;

		/// retourne la temperature minimum pour le pas courant (C)
		float PrendreTMin() const;

		/// retourne la temperature maximum pour le pas courant (C)
		float PrendreTMax() const;

		/// retourne la temperature minimum journalière pour le jour courant (C)
		float PrendreTMinJournaliere() const;

		/// retourne la temperature maximum journalière pour le jour courant (C)
		float PrendreTMaxJournaliere() const;

		/// retourne la pluie pour le pas courant (mm) 
		float PrendrePluie() const;

		// hauteur de precipitation en mm de neige (n'est pas un equivalent en eau)
		float PrendreNeige() const;

		float PrendreRayonnementSolaire() const;

		/// prendre la duree du jour (h)
		float PrendreDureeJour() const;

		float PrendreIndiceRadiation() const;

		/// retourne l'equivalent du couvert de neige en eau (EEN) //mm
		float PrendreCouvertNival() const;

		/// retourne la hauteur du couvert de neige //m
		float PrendreHauteurCouvertNival() const;

		float PrendreAlbedoNeige() const;

		/// retourne la vitesse du vent (m/s)
		float PrendreVitesseVent() const;

		float PrendreHumiditeRelative() const;

		/// retourne le nombre d'heures d'ensoleillement (h)
		float PrendreNbHeureSoleil() const;

		/// retourne l'apport de la fonte et de la pluie (mm)
		float PrendreApport() const;

		/// retourne l'apport de la fonte des glaciers (mm)
		double PrendreApportGlacier() const;

		/// retourne l'equivalent en eau de la glace (m)
		double PrendreEauGlacier() const;

		//Retourne l'ETP pour la classe d'occupation du sol (mm)
		float PrendreEtp(size_t index) const;

		//Retourne l'ETP total (mm)
		float PrendreEtpTotal() const;

		/// retourne l'etr total (mm/pdt)
		float PrendreEtrTotal() const;

		/// retourne l'etr de la couche 1 (mm/pdt)
		float PrendreEtr1() const;

		///retourne l'etr de la couche 2 (mm/pdt)
		float PrendreEtr2() const;

		///retourne l'etr de la couche 3 (mm/pdt)
		float PrendreEtr3() const;

		/// retourne la production de surface (mm)
		float PrendreProdSurf() const;

		/// retourne la production hypodermique (mm)
		float PrendreProdHypo() const;

		/// retourne la production de base (mm)
		float PrendreProdBase() const;

		/// retourne la production total (mm)
		float PrendreProductionTotal() const;

		/// retourne l'epaisseur de la couche 1 (m)
		float PrendreZ11() const;
		
		/// retourne l'epaisseur de la couche 2 (m)
		float PrendreZ22() const;

		/// retourne l'epaisseur de la couche 3 (m)
		float PrendreZ33() const;

		/// retourne le troncon aval de la zone
		TRONCON* PrendreTronconAval() const;

		/// retourne la profondeur du gel au sol (cm)
		float PrendreProfondeurGel() const;

		/// change l'identificateur de la zone
		void ChangeIdent(int ident);

		void ChangeTypeZone(TYPE_ZONE type_zone);

		/// change le nombre de pixel de la zone
		void ChangeNbPixel(size_t nb_pixel);

		/// change la superficie (km2)
		void ChangeSuperficie(double dSuperficie);

		/// change l'altitude moyenne de la zone (m)
		void ChangeAltitude(float altitude);

		/// change la pente moyenne de la zone [ratio]
		void ChangePente(float pente);

		/// change l'orientation moyenne de la zone
		void ChangeOrientation(ORIENTATION orientation);

		/// change l'orientation moyenne de la zone
		void ChangeOrientation(int orientation);

		/// change le troncon aval de la zone
		void ChangeTronconAval(TRONCON* troncon_aval);

		/// change les temperatures min/max pour le pas courant (C)
		void ChangeTemperature(float tmin, float tmax);

		/// change les temperatures min/max pour le jour courant (C)
		void ChangeTemperatureJournaliere(float tmin, float tmax);

		/// change la pluie (mm)
		void ChangePluie(float pluie);

		/// change la neige (mm)	hauteur de precipitation en mm de neige
		void ChangeNeige(float neige);

		/// change le nombre de classe d'etp
		void ChangeNbEtp(size_t nb_classe);

		/// change l'evapotranspiration potentielle pour une classe d'occupation du sol (mm)
		void ChangeEtp(size_t index, float etp);

		/// change le centroide de la zone (long/lat wgs84)
		void ChangeCentroide(const COORDONNEE& coordonnee);

		/// change le rayonnement solaire
		void ChangeRayonnementSolaire(float rayonnement);

		/// change la duree du jour (h)
		void ChangeDureeJour(float duree);

		/// change l'indice de radiation
		void ChangeIndiceRadiation(float indice_radiation);

		/// change la hauteur du couvert de neige en eau //m
		void ChangeHauteurCouvertNival(float hauteur_couvert_nival);

		/// change l'equivalent du couvert de neige en eau //mm
		void ChangeCouvertNival(float couvert_nival);

		void ChangeAlbedoNeige(float albedo_neige);

		/// change l'apport de la fonte et de la pluie (mm)
		void ChangeApport(float apport);

		void ChangeApportGlacier(double dApportGlacier);

		void ChangeEauGlacier(double dEauGlacier);	//m

		/// change l'etr de la couche 1 (mm/pdt)
		void ChangeEtr1(float etr);

		/// change l'etr de la couche 2 (mm/pdt)
		void ChangeEtr2(float etr);

		/// change l'etr de la couche 3 (mm/pdt)
		void ChangeEtr3(float etr);

		/// change la production de surface (mm)
		void ChangeProdSurf(float surf);

		/// change la production hypodermique (mm)
		void ChangeProdHypo(float hypo);

		/// change la production de base (mm)
		void ChangeProdBase(float base);

		void ChangeInfiltration(float infiltration);

		/// change les profondeurs des couches de sol (m)
		void ChangeZ1Z2Z3(float z1, float z2, float z3);

		/// change la profondeur du gel au sol (cm)
		void ChangeProfondeurGel(float fProfondeurGel);

	public:

		float					_theta1;					//bilan vertical	//teneur en eau couche 1 pour le pas de temps courant
		float					_theta2;					//bilan vertical	//teneur en eau couche 2 pour le pas de temps courant
		float					_theta3;					//bilan vertical	//teneur en eau couche 3 pour le pas de temps courant

		float					_reservoirAquifer;			//m

		float					_ecoulementSurf;			//ecoulement vers le troncon (surface) (m3/s)
		float					_ecoulementHypo;			//ecoulement vers le troncon (hypo) (m3/s)
		float					_ecoulementBase;			//ecoulement vers le troncon (base) (m3/s)

		float                   _apport_lateral_uhrh;		//apport lateral du uhrh vers le troncon (m3/s)

		int						_identABS;

		double					_dRayonnementSolaire;		//rayonnement solaire sur une pente en l'absence d'atmosphere
		double					_dDureeJour;				//duree du jour sur une surface horizontale
		double					_dIndiceRadiation;			//indice de radiation

		bool					_prJourIrrigation;			//pour les prélèvements: indique si le jour en cours est un jour d'irrigation

															//donnees modele fonte neige (degre_jour_bande)
		std::vector<double>		_couvert_nival_m1;			//equivalent en eau de la neige (EEN), pour le milieu 1 (conifères), pour chaque bande de l'uhrh //mm
		std::vector<double>		_couvert_nival_m2;			//equivalent en eau de la neige (EEN), pour le milieu 2 (feuillus), pour chaque bande de l'uhrh //mm
		std::vector<double>		_couvert_nival_m3;			//equivalent en eau de la neige (EEN), pour le milieu 3 (découvert), pour chaque bande de l'uhrh //mm

		std::vector<double>		_apport_m1;					//apport, pour le milieu 1 (conifères), pour chaque bande de l'uhrh //mm
		std::vector<double>		_apport_m2;					//apport, pour le milieu 2 (feuillus), pour chaque bande de l'uhrh //mm
		std::vector<double>		_apport_m3;					//apport, pour le milieu 3 (découvert), pour chaque bande de l'uhrh //mm

		std::vector<double>		_precip_m1;					//precip totale, pour le milieu 1 (conifères), pour chaque bande de l'uhrh	//mm
		std::vector<double>		_precip_m2;					//precip totale, pour le milieu 2 (feuillus), pour chaque bande de l'uhrh	//mm
		std::vector<double>		_precip_m3;					//precip totale, pour le milieu 3 (decouvert), pour chaque bande de l'uhrh	//mm

		TYPE_ZONE				_type_zone_original;		//type zone d'origine tel que déterminé lors du montage du projet. provient du fichier uhrh.csv.

	private:

		int			_ident;
		
		TYPE_ZONE	_type_zone;

		size_t		_nb_pixel;				//nombre de pixel de la zone
		double		_superficie;				//superficie km2
		float		_altitude;				//altitude moyenne m
		float		_pente;					//pente moyenne de l'uhrh [ratio]
		ORIENTATION _orientation;		//orientation moyenne
		COORDONNEE	_centroide;			//centroide de la zone (long/lat wgs84) [decimal degree]
		TRONCON*	_troncon_aval;

		float		_tmin;					// C
		float		_tmax;					// C
		float		_pluie;					// mm
		float		_neige;					// hauteur de precipitation en mm de neige (n'est pas un equivalent en eau)
		float		_vitesse_vent;			// m/s
		float		_humidite_relative;
		float		_nb_heure_soleil;			//h

		float		_tmin_jour;				//C
		float		_tmax_jour;				//C

		std::vector<float> _etp;		// mm	//pour chaque classe d'occupation du sol

		// variables de simulations

		float _iga;						//rayonnement solaire sur une pente en l'absence d'atmosphere
		float _hp;						//duree du jour sur une surface horizontale
		float _indice_radiation;		//indice de radiation

		float _couvert_nival;			//equivalent de la neige en eau //mm
		float _hauteur_couvert_nival;   //m
		float _albedo_neige;			//0-1

		float _apport;					//mm	//neige, pluie

		double _apport_glacier;			//mm
		double _eau_glacier;			//m

		float _etr1;					//mm
		float _etr2;					//mm
		float _etr3;					//mm

		float _surf;					//mm
		float _hypo;					//mm
		float _base;					//mm

		float _infiltration;

		float _profondeurGel;			//cm		//profondeur du gel au sol		//calculer par le sous-modele TEMPSOL

		float _z1;						//m		//profondeur des couches de sol
		float _z2;						//m
		float _z3;						//m
	};

}

#endif
