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

#include "rayonnement_net.hpp"

#include "constantes.hpp"
#include "sim_hyd.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>
#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	RAYONNEMENT_NET::RAYONNEMENT_NET()
		: _nom("RAYONNEMENT NET")
	{
	}


	RAYONNEMENT_NET::~RAYONNEMENT_NET()
	{
	}


	void RAYONNEMENT_NET::ChangeNbParams(const ZONES& zones)
	{
		const size_t nbUHRH = zones.PrendreNbZone();

		vector<float> vTemp;
		vTemp.resize(nbUHRH, VALEUR_MANQUANTE);
		_vRa.resize(365, vTemp);

		_fAlbedo.resize(nbUHRH, 0.23f);

		_fCoeffATransmissiviteAtmos.resize(nbUHRH, 0.9232f);
		_fCoeffBTransmissiviteAtmos.resize(nbUHRH, 0.1121f);
		_fCoeffCTransmissiviteAtmos.resize(nbUHRH, 0.8038f);

		_fCoeffAEmissiviteAtmos.resize(nbUHRH, 0.7363f);
		_fCoeffBEmissiviteAtmos.resize(nbUHRH, 0.0009f);
		_fCoeffCEmissiviteAtmos.resize(nbUHRH, 0.9828f);

		_fCoeffAEmissiviteSurface.resize(nbUHRH, 0.9828f);
		_fCoeffBEmissiviteSurface.resize(nbUHRH, 0.0009f);
	}


	float RAYONNEMENT_NET::PrendreRayonnementNet(int iJour, size_t index_zone)
	{
		ostringstream oss;
		ORIENTATION ori;
		float fSlopeAzimuth, fOut1, fOut2, fOut3, fRn, fAlbedo;

		ZONE& zone = _sim_hyd->PrendreZones()[index_zone];

		//calcule Ra si cela n'a pas deja ete fait (les valeurs sont les memes d'annee en annee pour une journee donnee)
		if(_vRa[iJour-1][index_zone] <= VALEUR_MANQUANTE)
		{
			ori = zone.PrendreOrientation();
			switch(ori)
			{
			case ORIENTATION_EST:
				fSlopeAzimuth = 90.0f;
				break;
			case ORIENTATION_NORD_EST:
				fSlopeAzimuth = 45.0f;
				break;
			case ORIENTATION_NORD:
				fSlopeAzimuth = 0.0f;
				break;
			case ORIENTATION_NORD_OUEST:
				fSlopeAzimuth = 315.0f;
				break;
			case ORIENTATION_OUEST:
				fSlopeAzimuth = 270.0f;
				break;
			case ORIENTATION_SUD_OUEST:
				fSlopeAzimuth = 225.0f;
				break;
			case ORIENTATION_SUD:
				fSlopeAzimuth = 180.0f;
				break;
			case ORIENTATION_SUD_EST:
				fSlopeAzimuth = 135.0f;
				break;
			default:
				oss << zone.PrendreIdent();
				throw ERREUR("RAYONNEMENT NET; orientation invalide pour uhrh " + oss.str());
			}

			Calcul_Ra(static_cast<float>(zone.PrendreCentroide().PrendreX()), 
						static_cast<float>(zone.PrendreCentroide().PrendreY()), 
						fSlopeAzimuth, 
						atan(zone.PrendrePente()), 
						CONSTANTE_SOLAIRE, 
						true, 
						iJour, 
						0, 
						fOut1, 
						fOut2, 
						fOut3);

			_vRa[iJour-1][index_zone] = fOut1;
		}

		if(zone.PrendreCouvertNival() > 0.0f)
			fAlbedo = zone.PrendreAlbedoNeige();
		else
			fAlbedo = _fAlbedo[index_zone];

		fRn = CalculRayonnementNet(zone.PrendreTMin(), zone.PrendreTMax(), fAlbedo, _vRa[iJour-1][index_zone], 
									_fCoeffATransmissiviteAtmos[index_zone], _fCoeffBTransmissiviteAtmos[index_zone], _fCoeffCTransmissiviteAtmos[index_zone], 
									_fCoeffAEmissiviteAtmos[index_zone], _fCoeffBEmissiviteAtmos[index_zone], _fCoeffCEmissiviteAtmos[index_zone], 
									_fCoeffAEmissiviteSurface[index_zone], _fCoeffBEmissiviteSurface[index_zone]);

		return fRn;
	}


	void RAYONNEMENT_NET::LectureParametres()
	{
		string cle, valeur, ligne;
		size_t index_zone, compteur;
		int iIdent;

		ZONES& zones = _sim_hyd->PrendreZones();
		int no_ligne = 6;

		if(_sim_hyd->PrendreNomEvapotranspiration() == "LINACRE" || 
			_sim_hyd->PrendreNomEvapotranspiration() == "PENMAN" || 
			_sim_hyd->PrendreNomEvapotranspiration() == "PENMAN-MONTEITH" || 
			_sim_hyd->PrendreNomEvapotranspiration() == "PRIESTLAY-TAYLOR")	//si le modele doit etre simulé
		{
			if(_sim_hyd->PrendrePasDeTemps() != 24)
				throw ERREUR("RAYONNEMENT NET: the computation of the net radiation at the surface requires a simulation timestep of 24 hours.");

			if(_sim_hyd->_fichierParametreGlobal)
			{
				LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
				return;
			}

			ifstream fichier( _nom_fichier_parametres );
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES RAYONNEMENT NET");

			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES RAYONNEMENT NET", 1);
			}

			getline_mod(fichier, ligne);
			lire_cle_valeur(fichier, cle, valeur);
			getline_mod(fichier, ligne);

			getline_mod(fichier, ligne); // commentaire

			for (compteur=0; compteur<zones.PrendreNbZone(); compteur++)
			{
				getline_mod(fichier, ligne);
				auto vValeur = extrait_fvaleur(ligne, ";");

				if(vValeur.size() != 10)
				{
					fichier.close();
					throw ERREUR_LECTURE_FICHIER( _nom_fichier_parametres, no_ligne, "Nombre de colonne invalide.");
				}

				iIdent = static_cast<int>(vValeur[0]);
				index_zone = zones.IdentVersIndex(iIdent);

				_fAlbedo[index_zone] = vValeur[1];

				_fCoeffATransmissiviteAtmos[index_zone] = vValeur[2];
				_fCoeffBTransmissiviteAtmos[index_zone] = vValeur[3];
				_fCoeffCTransmissiviteAtmos[index_zone] = vValeur[4];
				_fCoeffAEmissiviteAtmos[index_zone] = vValeur[5];
				_fCoeffBEmissiviteAtmos[index_zone] = vValeur[6];
				_fCoeffCEmissiviteAtmos[index_zone] = vValeur[7];
				_fCoeffAEmissiviteSurface[index_zone] = vValeur[8];
				_fCoeffBEmissiviteSurface[index_zone] = vValeur[9];

				++no_ligne;
			}

			fichier.close();
		}
	}


	void RAYONNEMENT_NET::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd->PrendreZones();

		ifstream fichier( _sim_hyd->_nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL;" + _sim_hyd->_nomFichierParametresGlobal );

		bool bOK = false;

		try{

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( _sim_hyd->_nomFichierParametresGlobal, 1);

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd->PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "RAYONNEMENT NET")
			{
				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 10)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd->_nomFichierParametresGlobal, no_ligne, "RAYONNEMENT NET");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd->_nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd->PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd->PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_fAlbedo[index_zone] = vValeur[1];

						_fCoeffATransmissiviteAtmos[index_zone] = vValeur[2];
						_fCoeffBTransmissiviteAtmos[index_zone] = vValeur[3];
						_fCoeffCTransmissiviteAtmos[index_zone] = vValeur[4];

						_fCoeffAEmissiviteAtmos[index_zone] = vValeur[5];
						_fCoeffBEmissiviteAtmos[index_zone] = vValeur[6];
						_fCoeffCEmissiviteAtmos[index_zone] = vValeur[7];

						_fCoeffAEmissiviteSurface[index_zone] = vValeur[8];
						_fCoeffBEmissiviteSurface[index_zone] = vValeur[9];
					}
				}

				bOK = true;
				break;
			}

			++no_ligne;
		}

		fichier.close();

		}
		catch(const ERREUR_LECTURE_FICHIER& ex)
		{
			fichier.close();
			throw ex;
		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; RAYONNEMENT NET; " + _sim_hyd->_nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd->_nomFichierParametresGlobal, 0, "Parametres sous-modele RAYONNEMENT NET");
	}


	void RAYONNEMENT_NET::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd->PrendreZones();

		if(_nom_fichier_parametres == "")	//creation d'un fichier par defaut s'il n'existe pas
			_nom_fichier_parametres = Combine(_sim_hyd->PrendreRepertoireSimulation(), "rayonnement_net.csv");

		string nom_fichier = _nom_fichier_parametres;
		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << _nom << endl;
		fichier << endl;

		fichier << "UHRH ID;"
			       " ALBEDO (0.23 SURFACE REFERENCE);"
			       " TRANSMISSIVITE ATMOSPHERIQUE (COEFFICIENT A);"
				   " TRANSMISSIVITE ATMOSPHERIQUE (COEFFICIENT B);"
				   " TRANSMISSIVITE ATMOSPHERIQUE (COEFFICIENT C);"
				   " EMISSIVITE ATMOSPHERIQUE (COEFFICIENT A);"
				   " EMISSIVITE ATMOSPHERIQUE (COEFFICIENT B);"
				   " EMISSIVITE ATMOSPHERIQUE (COEFFICIENT C);"
				   " EMISSIVITE DE LA SURFACE (COEFFICIENT A);"
				   " EMISSIVITE DE LA SURFACE (COEFFICIENT B)" << endl;

		for (size_t index=0; index<zones.PrendreNbZone(); ++index)
		{
			fichier << zones[index].PrendreIdent() << ';';

			fichier << _fAlbedo[index] << ';';
			fichier << _fCoeffATransmissiviteAtmos[index] << ';';
			fichier << _fCoeffBTransmissiviteAtmos[index] << ';';
			fichier << _fCoeffCTransmissiviteAtmos[index] << ';';
			fichier << _fCoeffAEmissiviteAtmos[index] << ';';
			fichier << _fCoeffBEmissiviteAtmos[index] << ';';
			fichier << _fCoeffCEmissiviteAtmos[index] << ';';
			fichier << _fCoeffAEmissiviteSurface[index] << ';';
			fichier << _fCoeffBEmissiviteSurface[index] << endl;
		}

		fichier.close();
	}

	
	//------------------------------------------------------------------------------------------------
	//Calcule le rayonnement net à la surface [MJ/m2/Jour]
	//
	//fTMin; Température minimale journalière (°C)
	//fTMax; Température maximale journalière (°C)
	//
	float RAYONNEMENT_NET::CalculRayonnementNet(float fTMin, float fTMax, 
											    float fAlbedo, float fRa, 
											    float fCoeffATransmissiviteAtmos, float fCoeffBTransmissiviteAtmos, float fCoeffCTransmissiviteAtmos,
											    float fCoeffAEmissiviteAtmos, float fCoeffBEmissiviteAtmos, float fCoeffCEmissiviteAtmos,
											    float fCoeffAEmissiviteSurface, float fCoeffBEmissiviteSurface)
	{		
		float fRs_inc, fRs_ref, fRl_atm, fRl_surf, fRn;
		float fDeltaT, fTr, fCloud, fpea, fEs, fTemp;

		fDeltaT = fTMax - fTMin;

		//transmissivité atmosphérique (Tr)
		fTr = fCoeffATransmissiviteAtmos * (1.0f - exp(-fCoeffBTransmissiviteAtmos * pow(fDeltaT, fCoeffCTransmissiviteAtmos)));

		//rayonnement de courtes longueurs d’onde incident (Rs_inc) [MJ/m2/Jour]
		fRs_inc = fTr * fRa;

		//rayonnement de courtes longueurs d’onde (solaire) réfléchi (Rs_ref) [MJ/m2/Jour]
		fRs_ref = fAlbedo * fRs_inc;
		
		//rayonnement de grandes longueurs d’onde incident (Rl_atm) [MJ/m2/Jour]

		//ennuagement (cloud)
		if(fTr > 0.75f)
			fCloud = 0.0f;
		else
		{
			if(fTr < 0.15f)
				fCloud = 1.0f;
			else
				fCloud = 1.0f - (fTr - 0.15f) / 0.6f;
		}

		//pseudo émissivité atmosphérique
		fTemp = (fTMax + fTMin) / 2.0f;	//temperature moyenne [dC]
		fpea = (fCoeffAEmissiviteAtmos + fCoeffBEmissiviteAtmos * fTemp) * (1.0f - fCoeffCEmissiviteAtmos * fCloud) + fCoeffCEmissiviteAtmos * fCloud;
		fpea = min(fpea, 1.0f);

		fRl_atm = fpea * (SIGMA/1000000.0f) * pow(fTemp+273.15f, 4.0f);	//(SIGMA/1000000.0f); constante de Stephen Boltzman	[MJ/JOUR/M2/K4]

		//rayonnement de grandes longueurs d’onde émis par la surface (Rl_surf) [MJ/m2/Jour]

		//pseudo émissivité de la surface (fEs)
		fEs = fCoeffAEmissiviteSurface + fCoeffBEmissiviteSurface * fTemp;
		fEs = min(fEs, 1.0f);

		fRl_surf = fEs * (SIGMA/1000000.0f) * pow(fTemp+273.15f, 4.0f);

		//rayonnement net à la surface [MJ/m2/Jour]
		fRn = fRs_inc - fRs_ref + fRl_atm - fRl_surf;
		return fRn;
	}


	//------------------------------------------------------------------------------------------------
	//Extraterrestrial Solar Radiation on Inclined Surfaces
	//C. David Whiteman and K. Jerry Allwine
	//
	//lon;			longitude [dd] (-180 - 180)
	//lat;			latitude [dd] (-90 - 90)
	//az;			slope azimuth angle (0 - 359)
	//in;			slope inclinaison angle (rad)
	//sc;			solar constant [e.g. 1353 W/m²]
	//daily;		daily total or instantaneous (true or false)
	//jj;			[julien day]
	//ihr;			hour (0 - 23) (used if daily is false)
	//out1 (Ra);	if daily is true; total radiation [MJ/m²], else; instantaneous radiation [w/m²]
	//out2;			if daily is true; sunrise (hours lst), else; sun's zenith angle [dd]
	//out3;			if daily is true; sunset (hours lst), else; sun's azimuth angle [dd]
	//
	void RAYONNEMENT_NET::Calcul_Ra(float lon, float lat, float az, float in, float sc, bool daily, int jj, int ihr, float& out1, float& out2, float& out3)
	{
		const float acof[] = { 0.00839f, -0.05391f, -0.00154f, -0.00222f };
		const float bcof[] = { -0.12193f, -0.15699f, -0.00657f, -0.0037f };

		const int dzero = JOUR_EQUINOXE_VERNAL;

		const float eccent = EXENTRICITE_ORBITE_TERRESTRE;
		const float calint = 1.0f;

		const float rtod = PI / 180.0f;
		const float decmax = (23.0f + 26.0f / 60.0f) * rtod;
		const float omega = 2.0f * PI / 365.0f;
		const float onehr = 15.0f * rtod;

		// julian date
		float d = static_cast<float>(jj);

		// ratio of radius vector squared
		float omd = omega * d;
		float omdzero = omega * dzero;
		float rdvecsq = 1.0f / pow((1.0f - eccent * cos(omd)), 2.0f);

		// declination of sun
		float longsun = omega * (d - dzero) + 2.0f * eccent * (sin(omd) - sin(omdzero));
		float declin = asin(sin(decmax) * sin(longsun));
		float sdec = sin(declin);
		float cdec = cos(declin);

		// check for polar night or day
		float sr;

		float arg = ((PI / 2.0f) - abs(declin)) / rtod;
		if (abs(lat) > arg)
		{
			if ((lat > 0 && declin < 0) || (lat < 0 && declin > 0))
			{
				out1 = 0.0f;
				out2 = 0.0f;
				out3 = 0.0f;
				return;
			}
			sr = -PI;
		}
		else
			sr = -abs(acos(-tan(lat * rtod) * tan(declin)));	//sunrise hour angle

		// standard time meridian for site

		float stdmrdn = __round(lon / 15.0f) * 15.0f;
		float longcor = (lon - stdmrdn) / 15.0f;

		// compute time correction from equation of time
		float b = 2.0f * PI * (d - 0.4f) / 365.0f;
		float em = 0.0f;

		for (int i = 0; i < 4; ++i)
			em+= (bcof[i] * sin(i * b) + acof[i] * cos(i * b));

		// time of solar noon
		float timnoon = 12.0f - em - longcor;

		float azslo = az * rtod;
		float inslo = in;	// * rtod;	//deg -> rad		//mod; la valeur in en input est deja en radian
		float slat = sin(lat * rtod);
		float clat = cos(lat * rtod);
		float caz = cos(azslo);
		float saz = sin(azslo);
		float sinc = sin(inslo);
		float cinc = cos(inslo);

		if (daily)
		{
			// compute daily total
			ihr = 0;

			float hinc = calint * onehr / 60.0f;
			float ik = (2.0f * abs(sr) / hinc) + 2.0f;
			bool first = true;
			out1 = 0.0f;

			for (float i = 1; i <= ik; ++i)
			{
				float h = sr + hinc * (i - 1.0f);
				float cosz = slat * sdec + clat * cdec * cos(h);
				float cosbeta = cdec * ((slat * cos(h)) * (-caz * sinc) - sin(h) * (saz * sinc) + (clat * cos(h)) * cinc) + sdec * (clat * (caz * sinc) + slat * cinc);
				float extra = sc * rdvecsq * cosz;

				if (extra < 0.0f) 
					extra = 0.0f;

				float extslo = sc * rdvecsq * cosbeta;

				if (extra <= 0.0f || extslo < 0.0f) 
					extslo = 0.0f;

				if (first && extslo > 0.0f)
				{
					out2 = (h - hinc) / onehr + timnoon;
					first = false;
				}

				if (!first && extslo <= 0.0f) 
					out3 = h / onehr + timnoon;

				out1 = extslo + out1;
			}

			out1 = out1 * calint * 60.0f / 1000000.0f;
		}
		else
		{
			// compute at one time
			float t1 = static_cast<float>(ihr);
			float h = onehr  * (t1 - timnoon);
			float cosz = slat * sdec + clat * cdec * cos(h);
			float cosbeta = cdec * ((slat * cos(h)) * (-caz * sinc) -
				sin(h) * (saz * sinc) + (clat * cos(h)) * cinc) +
				sdec * (clat * (caz * sinc) + slat * cinc);
			float extra = sc * rdvecsq * cosbeta;

			if (extra < 0.0f) 
				extra = 0.0f;

			float extslo = sc * rdvecsq * cosbeta;

			if (extra <= 0.0f || extslo < 0.0f) 
				extslo = 0.0f;

			out1 = extslo;
			float z = acos(cosz);
			float cosa = (slat * cosz - sdec) / (clat * sin(z));

			if (cosa < -1.0f) 
				cosa = -1.0f;

			if (cosa > 1.0f) 
				cosa = 1.0f;

			float a = abs(acos(cosa));

			if (h < 0.0f) 
				a = -a;

			out3 = z / rtod;
			out2 = a / rtod + 180.0f;
		}
	}


	//------------------------------------------------------------------------------------------------------------------------------------------------
	float RAYONNEMENT_NET::__round(float v)	// NOTE: cette fonction devrait etre dans le standard C++11 mais n'est pas inclus avec Visual Studio 2012
	{
		return floor(v + 0.5f);
	}

}
