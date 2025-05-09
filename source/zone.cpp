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

#include "zone.hpp"

#include "constantes.hpp"
#include "erreur.hpp"

#include <boost/assert.hpp>


namespace HYDROTEL 
{

	ZONE::ZONE()
		: _theta1(VALEUR_MANQUANTE)
		, _theta2(VALEUR_MANQUANTE)
		, _theta3(VALEUR_MANQUANTE)
		, _prJourIrrigation(false)
		, _ident(-1)
		, _type_zone(SOUS_BASSIN)
		, _nb_pixel(0)
		, _superficie(0.0)
		, _altitude(0)
		, _pente(0)
		, _orientation(ORIENTATION_AUCUNE)
		, _centroide()
		, _troncon_aval(nullptr)
		, _tmin(VALEUR_MANQUANTE)
		, _tmax(VALEUR_MANQUANTE)
		, _pluie(VALEUR_MANQUANTE)
		, _neige(VALEUR_MANQUANTE)
		, _vitesse_vent(VALEUR_MANQUANTE)
		, _humidite_relative(VALEUR_MANQUANTE)
		, _nb_heure_soleil(VALEUR_MANQUANTE)
		, _tmin_jour(VALEUR_MANQUANTE)
		, _tmax_jour(VALEUR_MANQUANTE)
		, _etp()
		, _iga(VALEUR_MANQUANTE)
		, _hp(VALEUR_MANQUANTE)
		, _indice_radiation(VALEUR_MANQUANTE)
		, _couvert_nival(0.0f)
		, _hauteur_couvert_nival(0.0f)
		, _albedo_neige(0.0f)
		, _apport(0.0f)
		, _apport_glacier(0.0)
		, _eau_glacier(0.0)
		, _etr1(VALEUR_MANQUANTE)
		, _etr2(VALEUR_MANQUANTE)
		, _etr3(VALEUR_MANQUANTE)
		, _surf(VALEUR_MANQUANTE)
		, _hypo(VALEUR_MANQUANTE)
		, _base(VALEUR_MANQUANTE)
		, _infiltration(VALEUR_MANQUANTE)
		, _profondeurGel(VALEUR_MANQUANTE)
	{
		_dRayonnementSolaire = dVALEUR_MANQUANTE;
		_dDureeJour = dVALEUR_MANQUANTE;
		_dIndiceRadiation = dVALEUR_MANQUANTE;
		_reservoirAquifer = 0.0f;
		_apport_lateral_uhrh = VALEUR_MANQUANTE;
	}

	ZONE::~ZONE()
	{
	}

	int ZONE::PrendreIdent() const
	{
		return _ident;
	}

	ZONE::TYPE_ZONE ZONE::PrendreTypeZone() const
	{
		return _type_zone;
	}

	size_t ZONE::PrendreNbPixel() const
	{
		return _nb_pixel;
	}

	double ZONE::PrendreSuperficie() const	//[km2]
	{
		return _superficie;
	}

	float ZONE::PrendreAltitude() const		//altitude moyenne [m]
	{
		return _altitude;
	}

	float ZONE::PrendrePente() const
	{
		return _pente;
	}

	ORIENTATION ZONE::PrendreOrientation() const
	{
		return _orientation;
	}

	float ZONE::PrendreTMin() const
	{
		return _tmin;
	}

	float ZONE::PrendreTMax() const
	{
		return _tmax;
	}

	float ZONE::PrendreTMinJournaliere() const
	{
		return _tmin_jour;
	}

	float ZONE::PrendreTMaxJournaliere() const
	{
		return _tmax_jour;
	}


	float ZONE::PrendrePluie() const
	{
		return _pluie;
	}

	float ZONE::PrendreNeige() const
	{
		return _neige;
	}

	float ZONE::PrendreVitesseVent() const
	{
		return _vitesse_vent;
	}

	float ZONE::PrendreRayonnementSolaire() const
	{
		return _iga;
	}

	float ZONE::PrendreHumiditeRelative() const
	{
		return _humidite_relative;
	}

	float ZONE::PrendreNbHeureSoleil() const
	{
		return _nb_heure_soleil;
	}

	TRONCON* ZONE::PrendreTronconAval() const
	{
		return _troncon_aval;
	}

	void ZONE::ChangeIdent(int ident)
	{
		_ident = ident;
		_identABS = abs(ident);
	}

	void ZONE::ChangeTypeZone(TYPE_ZONE type_zone)
	{
		_type_zone = type_zone;
	}

	void ZONE::ChangeNbPixel(size_t nb_pixel)
	{
		BOOST_ASSERT(nb_pixel > 0);
		_nb_pixel = nb_pixel;
	}

	void ZONE::ChangeSuperficie(double dSuperficie)
	{
		BOOST_ASSERT(dSuperficie > 0.0);
		_superficie = dSuperficie;
	}

	void ZONE::ChangeAltitude(float altitude)
	{
		_altitude = altitude;
	}

	void ZONE::ChangePente(float pente)
	{
		BOOST_ASSERT(pente > 0);
		_pente = pente;
	}

	void ZONE::ChangeOrientation(ORIENTATION orientation)
	{
		_orientation = orientation;
	}

	void ZONE::ChangeOrientation(int orientation)
	{
		switch (orientation)
		{
		case 1:
			_orientation = ORIENTATION_EST;
			break;
		case 2:
			_orientation = ORIENTATION_NORD_EST;
			break;
		case 3:
			_orientation = ORIENTATION_NORD;
			break;
		case 4:
			_orientation = ORIENTATION_NORD_OUEST;
			break;
		case 5:
			_orientation = ORIENTATION_OUEST;
			break;
		case 6:
			_orientation = ORIENTATION_SUD_OUEST;
			break;
		case 7:
			_orientation = ORIENTATION_SUD;
			break;
		case 8:
			_orientation = ORIENTATION_SUD_EST;
			break;
		default:
			throw ERREUR("orientation non valide");
		}
	}

	void ZONE::ChangeTemperature(float tmin, float tmax)
	{
		BOOST_ASSERT(tmin <= tmax);

		_tmin = tmin;
		_tmax = tmax;
	}

	void ZONE::ChangeTemperatureJournaliere(float tmin, float tmax)
	{
		BOOST_ASSERT(tmin <= tmax);
		_tmin_jour = tmin;
		_tmax_jour = tmax;
	}

	void ZONE::ChangePluie(float pluie)
	{
		BOOST_ASSERT(pluie >= 0);
		_pluie = pluie;
	}

	void ZONE::ChangeNeige(float neige)
	{
		BOOST_ASSERT(neige >= 0);
		_neige = neige;
	}

	void ZONE::ChangeTronconAval(TRONCON* troncon_aval)
	{
		BOOST_ASSERT(troncon_aval != nullptr);
		_troncon_aval = troncon_aval;
	}

	void ZONE::ChangeNbEtp(size_t nb_classe)
	{
		_etp.resize(nb_classe, VALEUR_MANQUANTE);
	}

	void ZONE::ChangeEtp(size_t index, float etp)	//(mm)
	{
		BOOST_ASSERT(index < _etp.size() && etp >= 0);
		_etp[index] = etp;
	}

	COORDONNEE ZONE::PrendreCentroide() const
	{
		return _centroide;	//long/lat wgs84 [decimal degree]
	}

	void ZONE::ChangeCentroide(const COORDONNEE& coordonnee)
	{
		_centroide = coordonnee;	//long/lat wgs84 [decimal degree]
	}

	void ZONE::ChangeRayonnementSolaire(float rayonnement)
	{
		_iga = rayonnement;
	}

	void ZONE::ChangeDureeJour(float duree)
	{
		_hp = duree;
	}

	void ZONE::ChangeIndiceRadiation(float indice_radiation)
	{
		_indice_radiation = indice_radiation;
	}

	float ZONE::PrendreDureeJour() const
	{
		return _hp;
	}

	float ZONE::PrendreIndiceRadiation() const
	{
		return _indice_radiation;
	}

	float ZONE::PrendreCouvertNival() const
	{
		return _couvert_nival;			//equivalent en eau	//mm
	}

	float ZONE::PrendreHauteurCouvertNival() const
	{
		return _hauteur_couvert_nival;	//m
	}

	float ZONE::PrendreAlbedoNeige() const
	{
		return _albedo_neige;
	}

	float ZONE::PrendreApport() const
	{
		return _apport;
	}

	double ZONE::PrendreApportGlacier() const
	{
		return _apport_glacier;
	}

	double ZONE::PrendreEauGlacier() const
	{
		return _eau_glacier;
	}

	float ZONE::PrendreEtpTotal() const
	{
		float etp = 0;
		size_t n = 0;

		for (auto iter = begin(_etp); iter != end(_etp); ++iter)
		{
			if (*iter > VALEUR_MANQUANTE)
			{
				etp += *iter;
				++n;
			}
		}

		if (n == 0)
			etp = VALEUR_MANQUANTE;

		return etp;
	}

	float ZONE::PrendreEtr1() const
	{
		return _etr1;
	}

	float ZONE::PrendreEtr2() const
	{
		return _etr2;
	}

	float ZONE::PrendreEtr3() const
	{
		return _etr3;
	}

	float ZONE::PrendreEtrTotal() const
	{
		float total = _etr1 + _etr2 + _etr3;
		
		if (total == (-999.0f * 3))
			total = -999.0f;

		return total;
	}

	//Retourne l'ETP pour la classe d'occupation du sol [mm]
	float ZONE::PrendreEtp(size_t index) const
	{
		BOOST_ASSERT(index < _etp.size());
		return _etp[index];
	}

	float ZONE::PrendreProdSurf() const
	{
		return _surf;
	}

	float ZONE::PrendreProdHypo() const
	{
		return _hypo;
	}

	float ZONE::PrendreProdBase() const
	{
		return _base;
	}

	float ZONE::PrendreProductionTotal() const
	{
		return _surf + _hypo + _base;
	}

	float ZONE::PrendreZ11() const	//epaisseur [m]
	{
		return _z1;
	}

	float ZONE::PrendreZ22() const	//epaisseur [m]
	{
		return _z2 - _z1;
	}

	float ZONE::PrendreZ33() const	//epaisseur [m]
	{
		return _z3 - _z2;
	}

	float ZONE::PrendreProfondeurGel() const
	{
		return _profondeurGel;	//cm
	}

	void ZONE::ChangeCouvertNival(float couvert_nival)
	{
		BOOST_ASSERT(couvert_nival >= 0);
		_couvert_nival = couvert_nival;		//equivalent de la neige en eau //mm
	}

	void ZONE::ChangeHauteurCouvertNival(float hauteur_couvert_nival)
	{
		BOOST_ASSERT(hauteur_couvert_nival >= 0);
		_hauteur_couvert_nival = hauteur_couvert_nival;	//m
	}

	void ZONE::ChangeAlbedoNeige(float albedo_neige)
	{
		_albedo_neige = albedo_neige;
	}

	void ZONE::ChangeApport(float apport)
	{
		BOOST_ASSERT(apport >= 0);
		_apport = apport;
	}

	void ZONE::ChangeApportGlacier(double dApportGlacier)
	{
		BOOST_ASSERT(dApportGlacier >= 0.0);
		_apport_glacier = dApportGlacier;
	}

	void ZONE::ChangeEauGlacier(double dEauGlacier)
	{
		BOOST_ASSERT(dEauGlacier >= 0.0);
		_eau_glacier = dEauGlacier;
	}

	void ZONE::ChangeEtr1(float etr)
	{
		BOOST_ASSERT(etr >= 0);
		_etr1 = etr;
	}

	void ZONE::ChangeEtr2(float etr)
	{
		BOOST_ASSERT(etr >= 0);
		_etr2 = etr;
	}

	void ZONE::ChangeEtr3(float etr)
	{
		BOOST_ASSERT(etr >= 0);
		_etr3 = etr;
	}

	void ZONE::ChangeProdSurf(float surf)
	{
		BOOST_ASSERT(surf >= 0);
		_surf = surf;
	}

	void ZONE::ChangeProdHypo(float hypo)
	{
		BOOST_ASSERT(hypo >= 0);
		_hypo = hypo;
	}

	void ZONE::ChangeProdBase(float base)
	{
		BOOST_ASSERT(base >= 0);
		_base = base;
	}

	void ZONE::ChangeInfiltration(float infiltration)
	{
		BOOST_ASSERT(infiltration >= 0);
		_infiltration = infiltration;
	}

	void ZONE::ChangeZ1Z2Z3(float z1, float z2, float z3)	//profondeur des couches de sol
	{
		BOOST_ASSERT(z1 > 0.0f && z2 > z1 && z3 > z2);

		_z1 = z1;
		_z2 = z2;
		_z3 = z3;
	}

	void ZONE::ChangeProfondeurGel(float fProfondeurGel)
	{
		BOOST_ASSERT(fProfondeurGel >= 0.0);

		//tronque à 1 decimale
		fProfondeurGel+= 0.05f;
		fProfondeurGel*= 10.0f;
		int iVal = static_cast<int>(fProfondeurGel);		
		_profondeurGel = static_cast<float>(iVal) / 10.0f;
	}

}
