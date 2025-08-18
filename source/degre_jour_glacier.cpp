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

#include "degre_jour_glacier.hpp"

#include "degre_jour_bande.hpp"
#include "interpolation_donnees.hpp"
#include "grille_meteo.hpp"
#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <fstream>
#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	DEGRE_JOUR_GLACIER::DEGRE_JOUR_GLACIER(SIM_HYD& sim_hyd)
		: FONTE_GLACIER(sim_hyd, "DEGRE JOUR GLACIER")
	{
		//_pOutput = &sim_hyd.PrendreOutput();
		//_netCdf_couvertnival = NULL;
	}


	DEGRE_JOUR_GLACIER::~DEGRE_JOUR_GLACIER()
	{
	}


	void DEGRE_JOUR_GLACIER::ChangeNbParams(const ZONES& zones)
	{
		//valeurs par defaut
		
		_taux_fonte_m1.resize(zones.PrendreNbZone(), 5.5);
		_seuil_fonte_m1.resize(zones.PrendreNbZone(), 0.0);
		_albedo_m1.resize(zones.PrendreNbZone(), 0.4);

		_densite_glace = 750.0;		//doit etre une valeur unique pour le bassin à cause du output moyenne pondéré (sim_hyd.cpp)

		_c0 = 37.1;
		_c1 = 1.31;

		_dEpaisseurGlaceMin = 100.0;	//m
		_dEpaisseurGlaceMax = 500.0;	//m

		_bFixedIceMass = false;

		_bMeltLowerBandsFirst = false;

		FONTE_GLACIER::ChangeNbParams(zones);
	}


	//------
	//pour calcul indice radiation

	//double theta, k, h;

	//_ce0.resize(nb_zone, 0);
	//_ce1.resize(nb_zone, 0);

	////compute ce0 & ce1

	//theta = zone.PrendreCentroide().PrendreY() / dRAD1;

	//k = atan(zone.PrendrePente());
	//h = ((495 - zone.PrendreOrientation() * 45) % 360) / dRAD1;

	//_ce1[i] = asin(sin(k) * cos(h) * cos(theta) + cos(k) * sin(theta)) * dRAD1;
	//_ce0[i] = atan(sin(h) * sin(k) / (cos(k) * cos(theta) - cos(h) * sin(k) * sin(theta))) * dRAD1;
	//------


	void DEGRE_JOUR_GLACIER::Initialise()
	{
		//Lorsque le modele de fonte de glace est simulé, le modele degre-jour-bande doit etre le modele de fonte de neige sélectionné
		if(_sim_hyd._fonte_neige->PrendreNomSousModele() != "DEGRE JOUR BANDE")
			throw ERREUR("DEGRE_JOUR_GLACIER erreur: le modele de fonte de neige utilise doit obligatoirement etre le modele DEGRE JOUR BANDE.");


		double dVolTotal, dVolUhrh, dAreaM1Bande, dAreaTotalM1, dAlt, dNbBande, dPixelArea, dStockUhrh, superficieUhrh, dNoDataDem, dM, dB, dEpaisseurGlaceBande, dSommeEpaisseur;
		size_t nbColZone, nbRowZone, nbColDem, nbRowDem, nbColOcc, nbRowOcc, col, row, indexUhrh, i, j, nb, nb_zone, nbPixel, idx;
		int iNoDataZone, iUhrh, iOcc, iNoDataOcc, iNbBande, x;

		ZONES& zones = _sim_hyd.PrendreZones();
		OCCUPATION_SOL& occupation_sol = _sim_hyd.PrendreOccupationSol();

		vector<int> zoneSimIdent = _sim_hyd.PrendreZonesSimulesIdent();
		vector<size_t> zoneSimIndex = _sim_hyd.PrendreZonesSimules();

		const RASTER<int>& grilleZone = zones.PrendreGrille();
		RASTER<float> grilleDem = LectureRaster_float(_sim_hyd.PrendreZones().PrendreNomFichierAltitude());
		RASTER<int>	grilleOcc = LectureRaster_int(RemplaceExtension(_sim_hyd.PrendreOccupationSol().PrendreNomFichier(), "tif"));

		superficieUhrh = 0.0;
		
		//valide que les coordonnees superieur gauche des matrices sont identique
		//trunc les valeur à 7 décimale pour eviter les problemes de resolution numerique lors de la comparaison des valeurs double
		long long int lx, ly, lxZone, lyZone;

		lxZone = static_cast<long long int>(grilleZone.PrendreCoordonnee().PrendreX() * 10000000.0);
		lyZone = static_cast<long long int>(grilleZone.PrendreCoordonnee().PrendreY() * 10000000.0);

		lx = static_cast<long long int>(grilleDem.PrendreCoordonnee().PrendreX() * 10000000.0);
		ly = static_cast<long long int>(grilleDem.PrendreCoordonnee().PrendreY() * 10000000.0);

		if(lx != lxZone || ly != lyZone)
			throw ERREUR("DEGRE_JOUR_GLACIER erreur: les coordonnees des matrice altitude et occupation du sol ne correspondent pas avec les coordonnees de la matrice uhrh.");

		lx = static_cast<long long int>(grilleOcc.PrendreCoordonnee().PrendreX() * 10000000.0);
		ly = static_cast<long long int>(grilleOcc.PrendreCoordonnee().PrendreY() * 10000000.0);

		if(lx != lxZone || ly != lyZone)
			throw ERREUR("DEGRE_JOUR_GLACIER erreur: les coordonnees des matrice altitude et occupation du sol ne correspondent pas avec les coordonnees de la matrice uhrh.");

		nbColZone = grilleZone.PrendreNbColonne();
		nbRowZone = grilleZone.PrendreNbLigne();
		nbColDem = grilleDem.PrendreNbColonne();
		nbRowDem = grilleDem.PrendreNbLigne();
		nbColOcc = grilleOcc.PrendreNbColonne();
		nbRowOcc = grilleOcc.PrendreNbLigne();

		if(nbColDem < nbColZone || nbRowDem < nbRowZone || nbColOcc < nbColZone || nbRowOcc < nbRowZone)
			throw ERREUR("DEGRE_JOUR_GLACIER erreur: les matrice altitude et occupation du sol doivent être de même taille (NbColonne et NbLigne) ou de taille supérieur à la matrice uhrh.");

		//hauteur des bandes d'altitude; meme que le modele degre-jour-bande
		_dHauteurBande = ((DEGRE_JOUR_BANDE*)_sim_hyd._fonte_neige)->_dHauteurBande;	//m

		iNoDataZone = grilleZone.PrendreNoData();
		dNoDataDem = static_cast<double>(grilleDem.PrendreNoData());
		iNoDataOcc = grilleOcc.PrendreNoData();
		
		dPixelArea = static_cast<double>(grilleZone.PrendreTailleCelluleX());
		dPixelArea = dPixelArea * dPixelArea;	//m2

		nb_zone = zones.PrendreNbZone();

		//initialisation pour les valeurs des gradients
		if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "LECTURE INTERPOLATION DONNEES")
		{
			if(_sim_hyd._versionTHIESSEN == 1)
				_sim_hyd._smThiessen1->LectureParametres();
			else
				_sim_hyd._smThiessen2->LectureParametres();	//_sim_hyd._versionTHIESSEN == 2

			std::cout << std::endl << "warning: degre_jour_glacier submodel will use parameters from the thiessen submodel file because the interpolation model is in read mode" << std::endl;
		}

		//determine les uhrh contenant de la glace
		_bUhrhGlacier.clear();
		_bUhrhGlacier.resize(nb_zone, false);
		_bUhrhSimule.clear();
		_bUhrhSimule.resize(nb_zone, false);

		_uhrhGlacierSimuleIndex.clear();

		for (i=0; i!=nb_zone; i++)
		{
			for(auto index = begin(_index_occupation_m1); index != end(_index_occupation_m1); index++)
			{
				if(occupation_sol.PrendrePourcentage_double(i, *index) > 0.0)
				{
					_bUhrhGlacier[zones[i]._identABS-1] = true;
					_uhrhGlacierIndex.push_back(i);

					if(find(begin(zoneSimIndex), end(zoneSimIndex), i) != end(zoneSimIndex))
					{
						_bUhrhSimule[i] = true;
						_uhrhGlacierSimuleIndex.push_back(i);
					}

					break;
				}
			}
		}

		//parcours les matrices uhrh, occupation et altitude
		//conserve les valeurs d'altitude de chaque pixel d'occupation glace pour chaque uhrh
		//conserve altitude min et max pour chaque uhrh contenant de la glace
		//calcul superficie total de glace pour le bassin

		_altMin.clear();
		_altMin.resize(nb_zone, 100000.0);
		_altMax.clear();
		_altMax.resize(nb_zone, -100000.0);

		_altPixelM1.clear();
		_altPixelM1.resize(nb_zone);

		_altMinM1Bassin = 100000.0;
		_altMaxM1Bassin = -100000.0;

		dAreaTotalM1 = 0.0;
		for(col=0; col!=nbColZone; col++)
		{
			for(row=0; row!=nbRowZone; row++)
			{
				iUhrh = grilleZone(row, col);
				
				if(iUhrh != iNoDataZone && _bUhrhGlacier[abs(iUhrh)-1])	//si l'uhrh est different nodata et contient superficie de glace
				{
					dAlt = static_cast<double>(grilleDem(row, col));
					if(dAlt != dNoDataDem)
					{
						iOcc = grilleOcc(row, col);
						indexUhrh = zones.IdentVersIndex(iUhrh);

						if(dAlt < _altMin[indexUhrh])
							_altMin[indexUhrh] = dAlt;
						if(dAlt > _altMax[indexUhrh])
							_altMax[indexUhrh] = dAlt;

						if(iOcc != iNoDataOcc && find(begin(_index_occupation_m1), end(_index_occupation_m1), iOcc-1) != end(_index_occupation_m1))
						{
							dAreaTotalM1+= dPixelArea;

							if(dAlt < _altMinM1Bassin)
								_altMinM1Bassin = dAlt;
							if(dAlt > _altMaxM1Bassin)
								_altMaxM1Bassin = dAlt;

							_altPixelM1[indexUhrh].push_back(dAlt);
						}
					}
				}
			}
		}

		//determination de l'equation de la droite de l'epaisseur de glace
		dM = (_dEpaisseurGlaceMax - _dEpaisseurGlaceMin) / (_altMaxM1Bassin - _altMinM1Bassin);		//(y2-y1) / (x2-x1)
		dB = _dEpaisseurGlaceMax - dM * _altMaxM1Bassin;	// b = y2 - m . x2

		//compute total initial ice volume (water equivalent) (lame) [m]
		dVolTotal = _c0 * pow(dAreaTotalM1 / 1000000.0, _c1);	//[hm3]		// /1000000: m2 -> km2
																
		//calcul des bandes pour chaque uhrh glacier; superficie de glace, altitude moy des pixels de glace
		//calcul somme total des epaisseur de glace
		_bandeSuperficieM1.clear();
		_bandeSuperficieM1.resize(nb_zone);
		_bandePourcentageM1.clear();
		_bandePourcentageM1.resize(nb_zone);
		_bandeAltMoy.clear();
		_bandeAltMoy.resize(nb_zone);
		_vol_m1.clear();
		_vol_m1.resize(nb_zone);
		_stock_m1.clear();
		_stock_m1.resize(nb_zone);
		_apport_m1.clear();
		_apport_m1.resize(nb_zone);
		_superficieUhrhM1.clear();
		_superficieUhrhM1.resize(nb_zone, 0.0);

		dSommeEpaisseur = 0.0;

		nb_zone = _uhrhGlacierIndex.size();	//nb de zone contenant de la glace

		for(i=0; i!=nb_zone; i++)
		{
			indexUhrh = _uhrhGlacierIndex[i];

			ZONE& zone = zones[indexUhrh];

			//determine le nb de bandes
			dNbBande = (_altMax[indexUhrh] - _altMin[indexUhrh]) / _dHauteurBande;
			iNbBande = static_cast<int>(dNbBande);

			if(iNbBande == 0)	//denivellation inferieur a hauteur bande
				iNbBande = 1;
			else
				++iNbBande;	//bande supplementaire pour l'exedent (altMax - altMin) / HauteurBande ne donne pas un resultat entier
							//si le resultat est entier la bande supplementaire est ajouté pour la valeur maximum (_altMax[indexUhrh]), qui theoriquement est l'altitude de depart de la bande suivante

			//calcule la superficie de glace pour chaque bande
			_bandeSuperficieM1[indexUhrh].resize(iNbBande, 0.0);
			_bandeAltMoy[indexUhrh].resize(iNbBande, 0.0);

			if(_bUhrhSimule[indexUhrh])
				_bandePourcentageM1[indexUhrh].resize(iNbBande, 0.0);
				
			nbPixel = _altPixelM1[indexUhrh].size();
			for(j=0; j!=nbPixel; j++)
			{
				idx = static_cast<size_t>( (_altPixelM1[indexUhrh][j] - _altMin[indexUhrh]) / _dHauteurBande );	//obtient l'index de la bande d'altitude pour le pixel

				_bandeSuperficieM1[indexUhrh][idx]+= dPixelArea;	//m2
				_bandeAltMoy[indexUhrh][idx]+= _altPixelM1[indexUhrh][j];	//altitude moy des bandes calculé selon les pixel de glace

				if(_bUhrhSimule[indexUhrh])
					_superficieUhrhM1[indexUhrh]+= dPixelArea;	//m2
			}

			//calcule donnees par bande
			if(_bUhrhSimule[indexUhrh])
			{
				superficieUhrh = zone.PrendreSuperficie() * 1000000.0;	//km2 -> m2

				_vol_m1[indexUhrh].resize(iNbBande, 0.0);
				_stock_m1[indexUhrh].resize(iNbBande, 0.0);
				_apport_m1[indexUhrh].resize(iNbBande, 0.0);
			}
					
			for(x=0; x!=iNbBande; x++)
			{
				dAreaM1Bande = _bandeSuperficieM1[indexUhrh][x];
				if(dAreaM1Bande != 0.0)
				{
					_bandeAltMoy[indexUhrh][x]/= (dAreaM1Bande / dPixelArea);	//calcule moyenne des altitude	//divise la somme par le nb de pixel
					
					if(_bUhrhSimule[indexUhrh])
						_bandePourcentageM1[indexUhrh][x] = dAreaM1Bande / superficieUhrh;

					dEpaisseurGlaceBande = dM * _bandeAltMoy[indexUhrh][x] + dB;
					dSommeEpaisseur+= dAreaM1Bande * dEpaisseurGlaceBande;
				}
			}
		}

		//compute uhrh initial ice volume (water equivalent) (lame) [m]
		nb_zone = _uhrhGlacierSimuleIndex.size();	//nb de zone contenant de la glace

		for(i=0; i!=nb_zone; i++)
		{
			indexUhrh = _uhrhGlacierSimuleIndex[i];
				
			ZONE& zone = zones[indexUhrh];
			dVolUhrh = 0.0;

			nb = _bandeSuperficieM1[indexUhrh].size();
			for(j=0; j!=nb; j++)	//pour chaque bande
			{
				dAreaM1Bande = _bandeSuperficieM1[indexUhrh][j];
				if(dAreaM1Bande != 0.0)
				{
					dEpaisseurGlaceBande = dM * _bandeAltMoy[indexUhrh][j] + dB;									//
																													//
					_vol_m1[indexUhrh][j] = dVolTotal * (dAreaM1Bande * dEpaisseurGlaceBande / dSommeEpaisseur);	//[hm3]	//repartition altitude/epaisseur					
					dVolUhrh+= _vol_m1[indexUhrh][j];

					_stock_m1[indexUhrh][j] = ((_vol_m1[indexUhrh][j] * 1000000.0) * _densite_glace) / 1000.0 / dAreaM1Bande;	//[m]
				}
			}

			dStockUhrh = ((dVolUhrh * 1000000.0) * _densite_glace) / 1000.0 / _superficieUhrhM1[indexUhrh];	//[m]
			zone.ChangeEauGlacier(dStockUhrh);
		}

		//initialisation fichiers output
		
		//_PasTempsCourantIndex = 0;

		//if (_sim_hyd._outputCDF && (_pOutput->SauvegardeCouvertNival() || _pOutput->SauvegardeHauteurNeige() || _pOutput->SauvegardeAlbedoNeige())
		//if (_sim_hyd._outputCDF && _pOutput->SauvegardeCouvertNival())
		//{
		//	if (_pOutput->_uhrhOutputIDs == NULL)
		//	{
		//		std::vector<size_t> vect;
		//		vect.clear();

		//		i = 0;
		//		for (size_t index=0; index<zones.PrendreNbZone(); ++index)
		//		{
		//			if (find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
		//			{
		//				if (_pOutput->_bSauvegardeTous ||
		//					find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
		//				{
		//					vect.push_back(index);
		//					++i;
		//				}
		//			}
		//		}

		//		_pOutput->_uhrhOutputNb = vect.size();
		//		_pOutput->_uhrhOutputIndex = new size_t[_pOutput->_uhrhOutputNb];
		//		_pOutput->_uhrhOutputIDs = new int[_pOutput->_uhrhOutputNb];

		//		for (i=0; i<_pOutput->_uhrhOutputNb; i++)
		//		{
		//			_pOutput->_uhrhOutputIndex[i] = vect[i];
		//			_pOutput->_uhrhOutputIDs[i] = zones[_pOutput->_uhrhOutputIndex[i]].PrendreIdent();
		//		}
		//	}
		//}

		FONTE_GLACIER::Initialise();
	}


	void DEGRE_JOUR_GLACIER::Calcule()
	{
		string sString, sString2, sString3;
		size_t index_zone, index, nbUhrh, nbBande, i;
		double apport_m1, temperature_moyenne, dStockUhrh, diff_alt, dAltMoyUhrh, dTMoyUhrh, dApportUhrh, dIndiceRadiationUhrh, dPdt, dGradientVerticalTemp, dVol, dStock;
		bool bSkipUpperBands, bSkipUpperBandsTemperatureCondition;

		ZONES& zones = _sim_hyd.PrendreZones();

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		dPdt = static_cast<double>(_sim_hyd.PrendrePasDeTemps());
		nbUhrh = _uhrhGlacierSimuleIndex.size();

		for(index=0; index!=nbUhrh; index++)
		{
			index_zone = _uhrhGlacierSimuleIndex[index];

			ZONE& zone = zones[index_zone];

			nbBande = _bandeSuperficieM1[index_zone].size();

			for(i=0; i!=nbBande; i++)
				_apport_m1[index_zone][i] = 0.0;

			dApportUhrh = 0.0;
			dStockUhrh = zone.PrendreEauGlacier();

			if (dStockUhrh != 0.0)
			{
				//pour chaque bande d'altitude
				dAltMoyUhrh = static_cast<double>(zones[index_zone].PrendreAltitude());
				dTMoyUhrh = (static_cast<double>(zone.PrendreTMin()) + static_cast<double>(zone.PrendreTMax())) / 2.0;
				dIndiceRadiationUhrh = static_cast<double>(zone.PrendreIndiceRadiation());	//zone.PrendreIndiceRadiation(): a été calculé dans le modele degre_jour_modifie
				
				//TODO lorsque le mode lecture est activé pour interpolation, le fichier de parametres du mode lecture ne contient pas le parametre gradient.
				//     ce parametre devrait etre dans le modele degre_jour_glacier.
				//     pour l'instant on prend les meme params que le modele interpolation (ou thiessen si mode lecture)

				if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "THIESSEN2")
					dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen2->PrendreGradientTemperature(index_zone));
				else
				{
					if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "MOYENNE 3 STATIONS2")
						dGradientVerticalTemp = static_cast<double>(_sim_hyd._smMoy3station2->PrendreGradientTemperature(index_zone));
					else
					{
						if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "THIESSEN1")
							dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen1->PrendreGradientTemperature(index_zone));
						else
						{
							if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "MOYENNE 3 STATIONS1")
								dGradientVerticalTemp = static_cast<double>(_sim_hyd._smMoy3station1->PrendreGradientTemperature(index_zone));
							else
							{
								if(_sim_hyd._interpolation_donnees->PrendreNomSousModele() == "GRILLE")
									dGradientVerticalTemp = static_cast<double>(_sim_hyd._smGrilleMeteo->PrendreGradientTemperature(index_zone));
								else
								{
									//LECTURE_INTERPOLATION_DONNEES	//use params of thiessen model
									if(_sim_hyd._versionTHIESSEN == 1)
										dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen1->PrendreGradientTemperature(index_zone));
									else
										dGradientVerticalTemp = static_cast<double>(_sim_hyd._smThiessen2->PrendreGradientTemperature(index_zone));
								}
							}
						}
					}
				}

				bSkipUpperBandsTemperatureCondition = false;
				bSkipUpperBands = false;

				for(i=0; i!=nbBande && !bSkipUpperBandsTemperatureCondition && (!_bMeltLowerBandsFirst || !bSkipUpperBands); i++)
				{
					if(_bandeSuperficieM1[index_zone][i] != 0.0 && 
							zone._couvert_nival_m3[i] == 0.0 && _stock_m1[index_zone][i] != 0.0)	//fonte de glace seulement s'il n'y a plus de neige sur le milieu découvert (M3) qui devrait correspondre à l'occupation du sol glace
					{
						diff_alt = _bandeAltMoy[index_zone][i] - dAltMoyUhrh;
						temperature_moyenne = dTMoyUhrh + dGradientVerticalTemp * diff_alt / 100.0;

						if(temperature_moyenne > _seuil_fonte_m1[index_zone])
						{
							bSkipUpperBands = true;	//option _bMeltLowerBandsFirst

							//CalculIndiceRadiation(date_courante, pas_de_temps, zone, index_zone);		//ce calcul est deja effectué par le modele degre_jour_bande

							//ajout de la chaleur de fonte par rayonnement
							apport_m1 = _taux_fonte_m1[index_zone] / 1000.0 * (temperature_moyenne - _seuil_fonte_m1[index_zone]) * dIndiceRadiationUhrh * (1.0 - _albedo_m1[index_zone]);
							apport_m1 = apport_m1 * (dPdt / 24.0);

							if(apport_m1 > _stock_m1[index_zone][i])
							{
								apport_m1 = _stock_m1[index_zone][i];
								
								if(!_bFixedIceMass)
									_stock_m1[index_zone][i] = 0.0;

								bSkipUpperBands = false;	//option _bMeltLowerBandsFirst; si le stock de la bande fond au complet on laisse la bande suivante fondre egalement
							}
							else
							{
								if(!_bFixedIceMass)
									_stock_m1[index_zone][i]-= apport_m1;
							}

							if(!_bFixedIceMass)
							{
								dVol = apport_m1 * _bandeSuperficieM1[index_zone][i] * 1000.0 / _densite_glace / 1000000.0;	//[hm3]
								_vol_m1[index_zone][i]-= dVol;
							
								dStock = ((dVol * 1000000.0) * _densite_glace) / 1000.0 / _superficieUhrhM1[index_zone];	//[m]
								dStockUhrh-= dStock;
								if(dStockUhrh < 0.0)
									dStockUhrh = 0.0;
							}

							_apport_m1[index_zone][i] = apport_m1;
							dApportUhrh+= apport_m1 * _bandePourcentageM1[index_zone][i];	//m
						}
						else
						{
							if(dGradientVerticalTemp <= 0.0)
								bSkipUpperBandsTemperatureCondition = true;	//si la bande ne font pas a cause de la temperature, necessairement les bandes superieures ne fondront pas egalement
						}
					}
				}

				if(!_bFixedIceMass)
					zone.ChangeEauGlacier(dStockUhrh);	//m
			}

			zone.ChangeApportGlacier(dApportUhrh * 1000.0);		//m -> mm
		}

		//string str;
		//
		////fichier couvert_nival.csv
		//if (_pOutput->SauvegardeCouvertNival())
		//{
		//	if (_netCdf_couvertnival != NULL)
		//	{
		//		size_t i, idx;

		//		idx = _PasTempsCourantIndex * _pOutput->_uhrhOutputNb;

		//		for (i=0; i<_pOutput->_uhrhOutputNb; i++)
		//		{
		//			stock_moyen = (_pourcentage_conifers[_pOutput->_uhrhOutputIndex[i]] * _stock_conifers[_pOutput->_uhrhOutputIndex[i]] +
		//				_pourcentage_autres[_pOutput->_uhrhOutputIndex[i]] * _stock_decouver[_pOutput->_uhrhOutputIndex[i]] +
		//				_pourcentage_feuillus[_pOutput->_uhrhOutputIndex[i]] * _stock_feuillus[_pOutput->_uhrhOutputIndex[i]]) * 1000.0f;	//equivalent en eau du couvert nival	//m -> mm

		//			_netCdf_couvertnival[idx+i] = stock_moyen;
		//		}
		//	}
		//	else
		//	{
		//		ostringstream oss;
		//		oss.str("");
		//	
		//		oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(6) << setiosflags(ios::fixed);

		//		for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		//		{
		//			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
		//			{
		//				if (_pOutput->_bSauvegardeTous || 
		//					find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
		//				{
		//					float stock_moyen = ( _pourcentage_conifers[index] * _stock_conifers[index] + 
		//						_pourcentage_autres[index] * _stock_decouver[index] + 
		//						_pourcentage_feuillus[index] * _stock_feuillus[index] ) * 1000.0f;	//equivalent en eau du couvert nival	//m -> mm

		//					oss << stock_moyen << _pOutput->Separator();
		//				}
		//			}
		//		}
		//	
		//		str = oss.str();
		//		str = str.substr(0, str.length()-1); //enleve le dernier separateur
		//		_fichier_couvert_nival << str << endl;
		//	}
		//}

		FONTE_GLACIER::Calcule();
		
		//++_PasTempsCourantIndex;	//netcdf
	}


	void DEGRE_JOUR_GLACIER::Termine()
	{
		string str1, str2;

		//if (_pOutput->SauvegardeCouvertNival())
		//{
		//	if (_netCdf_couvertnival != NULL)
		//	{
		//		str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "couvert_nival.nc");
		//		str2 = _pOutput->SauvegardeOutputNetCDF(str1, true, "couvert_nival", _netCdf_couvertnival, "mm", "Equivalent en eau du couvert nival");
		//		if(str2 != "")
		//			throw ERREUR(str2);

		//		delete [] _netCdf_couvertnival;
		//	}
		//	else
		//		_fichier_couvert_nival.close();
		//}

		//if (_pOutput->SauvegardeHauteurNeige())
		//{
		//	if (_sim_hyd._outputCDF)
		//	{
		//	}
		//	else
		//		_fichier_hauteur_neige.close();
		//}

		FONTE_GLACIER::Termine();
	}

	
	//L'indice de radiation est deja calculé dans le modele degree_jour_modifier (neige)

	//void DEGRE_JOUR_GLACIER::CalculIndiceRadiation(DATE_HEURE date_heure, unsigned short pas_de_temps, ZONE& zone, size_t index_zone)
	//{
	//	const double i0 = dCONSTANTE_SOLAIRE;	// constante solaire en Watt/m2.
	//	const double w = 15.0 / dRAD1;	// vitesse angulaire de la rotation de la terre

	//	double theta, theta1, alpha, i_j1, i_j2, jour, heure, e2, i_e2, decli, duree_hor, duree_pte, tampon;
	//	double t1_pte, t2_pte, t1_pte_sim, t2_pte_sim, t1_hor_sim, t2_hor_sim, t1, t2, dIndiceRadiation;

	//	theta = zone.PrendreCentroide().PrendreY() / dRAD1;
	//	theta1 = _ce1[index_zone] / dRAD1;
	//	alpha = _ce0[index_zone] / dRAD1;

	//	jour = static_cast<double>(date_heure.PrendreJourJulien());
	//	heure = static_cast<double>(date_heure.PrendreHeure());

	//	// calcul du vecteur radian
	//	e2 = pow(1.0 - dEXENTRICITE_ORBITE_TERRESTRE * cos((jour - 4.0) / dDEG1), 2.0);
	//	i_e2 = i0 / e2;

	//	// calcul de la declinaison
	//	decli = 0.410152374218 * sin((jour - 80.25) / dDEG1);

	//	duree_hor = 0.0;
	//	duree_pte = 0.0;

	//	// demi-duree du jour sur une surface horizontale
	//	tampon = -tan(theta) * tan(decli);
	//	if (tampon > 1.0)
	//		duree_hor = 0.0;
	//	else if (tampon < -1.0)
	//		duree_hor = 12.0;
	//	else
	//		duree_hor = acos(tampon) / w;

	//	// duree du jour sur une surface en pente
	//	tampon = -tan(theta1) * tan(decli);
	//	if (tampon > 1.0)
	//		duree_pte = 0.0;
	//	else if (tampon < -1.0)
	//		duree_pte = 12.0;
	//	else
	//		duree_pte = acos(tampon) / w;

	//	// leve et couche du soleil pour une surface en pente
	//	t1_pte = -duree_pte - alpha / w;
	//	t2_pte = duree_pte - alpha / w;

	//	if (t1_pte < -duree_hor)
	//		t1_pte = -duree_hor;
	//	if (t2_pte > duree_hor)
	//		t2_pte = duree_hor;

	//	// Si le pas de temps de la simulation (en heure) est inferieur a 24h
	//	// alors il ne suffit pas de calculer pour une surface en pente la duree du
	//	// jour, le leve et le couche du soleil. Mais il faut inclure seulement les
	//	// heures qu'on simule.

	//	//calculs pour un pas de temps de 24h

	//	t1_pte_sim = t1_pte;
	//	t2_pte_sim = t2_pte;
	//	t1_hor_sim = -duree_hor;
	//	t2_hor_sim = duree_hor;

	//	i_j1 = 0.0;
	//	i_j2 = 0.0;

	//	// calcul de l'ensoleillement d'une surface horizontale
	//	if (t1_hor_sim > t2_hor_sim) 
	//		i_j1 = 0.0;
	//	else
	//		i_j1 = 3600.0 * i_e2 * ((t2_hor_sim - t1_hor_sim) * sin(theta) * sin(decli) + 1.0 / w * cos(theta) * cos(decli) * (sin(w * t2_hor_sim) - sin(w * t1_hor_sim)));

	//	// calcul de l'ensoleillement d'une surface en pente
	//	if (t1_pte_sim > t2_pte_sim) 
	//		i_j2 = 0.0;
	//	else
	//		i_j2 = 3600.0 * i_e2 * ((t2_pte_sim - t1_pte_sim) * sin(theta1) * sin(decli) + 1.0 / w * cos(theta1)*cos(decli) * (sin(w * t2_pte_sim + alpha) - sin(w * t1_pte_sim + alpha)));

	//	dIndiceRadiation = i_j1 != 0.0 ? fabs(i_j2 / i_j1) : 1.0;

	//	if (pas_de_temps != 24)
	//	{
	//		//calculs pour un pas de temps inférieur à 24h

	//		double dIndiceRadiation24H = dIndiceRadiation; 

	//		t1 = heure - 12.0;
	//		t2 = heure + static_cast<double>(pas_de_temps) - 12.0;

	//		t1_pte_sim = max(t1, t1_pte);
	//		t2_pte_sim = min(t2, t2_pte);

	//		t1_hor_sim = max(t1, -duree_hor);
	//		t2_hor_sim = min(t2, duree_hor);

	//		i_j1 = 0.0;
	//		i_j2 = 0.0;

	//		// calcul de l'ensoleillement d'une surface horizontale
	//		if (t1_hor_sim > t2_hor_sim) 
	//			i_j1 = 0.0;
	//		else
	//			i_j1 = 3600.0 * i_e2 * ((t2_hor_sim - t1_hor_sim) * sin(theta) * sin(decli) + 1.0 / w * cos(theta) * cos(decli) * (sin(w * t2_hor_sim) - sin(w * t1_hor_sim)));

	//		// calcul de l'ensoleillement d'une surface en pente
	//		if (t1_pte_sim > t2_pte_sim) 
	//			i_j2 = 0.0;
	//		else
	//			i_j2 = 3600.0 * i_e2 * ((t2_pte_sim - t1_pte_sim) * sin(theta1) * sin(decli) + 1.0 / w * cos(theta1)*cos(decli) * (sin(w * t2_pte_sim + alpha) - sin(w * t1_pte_sim + alpha)));

	//		dIndiceRadiation = i_j1 != 0.0 ? fabs(i_j2 / i_j1) : 1.0;

	//		if(dIndiceRadiation24H < dIndiceRadiation)
	//			dIndiceRadiation = dIndiceRadiation24H;
	//	}

	//	zone._dRayonnementSolaire = max(0.0, i_j2);
	//	zone._dDureeJour = max(0.0, t2_hor_sim - t1_hor_sim);
	//	zone._dIndiceRadiation = dIndiceRadiation;

	//	zone.ChangeRayonnementSolaire(max(0.0f, static_cast<float>(i_j2)));
	//	zone.ChangeDureeJour(max(0.0f, static_cast<float>(t2_hor_sim - t1_hor_sim)));
	//	zone.ChangeIndiceRadiation(static_cast<float>(dIndiceRadiation));
	//}


	void DEGRE_JOUR_GLACIER::ChangeIndexOccupationM1(const std::vector<size_t>& index)
	{
		_index_occupation_m1 = index;
		_index_occupation_m1.shrink_to_fit();
	}


	vector<size_t> DEGRE_JOUR_GLACIER::PrendreIndexOccupationM1() const
	{
		return _index_occupation_m1;
	}


	void DEGRE_JOUR_GLACIER::LectureParametres()
	{
		if(_sim_hyd.PrendreNomFonteGlacier() == PrendreNomSousModele())	//si le modele est simulé
		{
			if(_sim_hyd._fichierParametreGlobal)
			{
				LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
				return;
			}

			ifstream fichier;
			
			fichier.open(PrendreNomFichierParametres(), ios_base::in);
			if (!fichier)
			{
				//cree un fichier avec valeur par defaut
				try{
				SauvegardeParametres();				
				}
				catch(...)
				{
					throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_GLACIER");
				}

				fichier.open(PrendreNomFichierParametres(), ios_base::in);
				if (!fichier)
					throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_GLACIER");

				std::cout << endl << "Warning: the file `" << PrendreNomFichierParametres() << "` was missing: a file with default parameters has been created." << endl << endl;
			}

			istringstream iss;
			double seuil_m1, taux_m1, albedo_m1;
			size_t index, index_zone;
			char c;
			int ident;

			try
			{
				ZONES& zones = _sim_hyd.PrendreZones();

				string cle, valeur, ligne;
				lire_cle_valeur(fichier, cle, valeur);

				if (cle != "PARAMETRES HYDROTEL VERSION")
					throw ERREUR("Erreur lecture FICHIER PARAMETRES DEGRE_JOUR_GLACIER");

				getline_mod(fichier, ligne);
				lire_cle_valeur(fichier, cle, valeur);
				getline_mod(fichier, ligne);

				// lecture des classes integrees

				lire_cle_valeur(fichier, cle, valeur);
				ChangeIndexOccupationM1( extrait_valeur(valeur) );

				if(PrendreIndexOccupationM1().size() == 0)
					throw ERREUR("Erreur FICHIER PARAMETRES DEGRE_JOUR_GLACIER: la classe integre glacier ne contient aucune classe d`occupation du sol.");

				getline_mod(fichier, ligne);	//ligne vide

				//densite glace
				getline_mod(fichier, ligne);
				ligne = ligne.substr(21, string::npos);	//DENSITE GLACE(kg/m3);
				iss.clear();
				iss.str(ligne);
				iss >> _densite_glace;

				getline_mod(fichier, ligne);	//ligne vide

				//constante empirique 0
				getline_mod(fichier, ligne);
				ligne = ligne.substr(22, string::npos);	//CONSTANTE EMPIRIQUE 0;
				iss.clear();
				iss.str(ligne);
				iss >> _c0;

				//constante empirique 1
				getline_mod(fichier, ligne);
				ligne = ligne.substr(22, string::npos);	//CONSTANTE EMPIRIQUE 1;
				iss.clear();
				iss.str(ligne);
				iss >> _c1;

				getline_mod(fichier, ligne);	//ligne vide

				//epaisseur de glace
				getline_mod(fichier, ligne);
				ligne = ligne.substr(23, string::npos);	//EPAISSEUR GLACE MIN(m);
				iss.clear();
				iss.str(ligne);
				iss >> _dEpaisseurGlaceMin;

				getline_mod(fichier, ligne);
				ligne = ligne.substr(23, string::npos);	//EPAISSEUR GLACE MAX(m);
				iss.clear();
				iss.str(ligne);
				iss >> _dEpaisseurGlaceMax;

				getline_mod(fichier, ligne);	//ligne vide

				getline_mod(fichier, ligne);	//uhrh header or FixedIceMass

				_bFixedIceMass = false;

				boost::algorithm::to_upper(ligne);
				if(ligne.size() > 22 && ligne.substr(0, 22) == "MASSE GLACE FIXE(0/1);")
				{
					ligne = ligne.substr(22, string::npos);
					ligne = TrimString(ligne);
					
					if(ligne == "1")
						_bFixedIceMass = true;

					getline_mod(fichier, ligne);	//empty line
					getline_mod(fichier, ligne);	//uhrh header
				}

				// lecture des parametres
				for (index = 0; index < zones.PrendreNbZone(); ++index)
				{
					fichier >> ident >> c;
					fichier >> taux_m1 >> c;
					fichier >> seuil_m1 >> c;
					fichier >> albedo_m1;

					index_zone = zones.IdentVersIndex(ident);

					_taux_fonte_m1[index_zone] = taux_m1;
					_seuil_fonte_m1[index_zone] = seuil_m1;
					_albedo_m1[index_zone] = albedo_m1;
				}
			}
			catch(const ERREUR& ex)
			{
				fichier.close();
				throw ex;
			}
			catch(...)
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES DEGRE_JOUR_GLACIER");
			}

			fichier.close();
		}
	}


	void DEGRE_JOUR_GLACIER::LectureParametresFichierGlobal()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; " + _sim_hyd._nomFichierParametresGlobal );

		bool bOK = false;

		try{

		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, 1);

		vector<double> vValeur;
		size_t nbGroupe, x, y, index_zone;
		double dVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "DEGRE JOUR GLACIER")
			{
				++no_ligne;
				getline_mod(fichier, ligne);				
				ChangeIndexOccupationM1(extrait_valeur(ligne));

				++no_ligne;
				getline_mod(fichier, ligne);
				vValeur = extrait_dvaleur(ligne, ";");

				_densite_glace = vValeur[0];
				_c0 = vValeur[1];
				_c1 = vValeur[2];
				_dEpaisseurGlaceMin = vValeur[3];
				_dEpaisseurGlaceMax = vValeur[4];

				_bFixedIceMass = false;
				if(vValeur.size() == 6)
				{
					if(vValeur[5] == 1.0)
						_bFixedIceMass = true;
				}

				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					vValeur = extrait_dvaleur(ligne, ";");

					if(vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide. DEGRE JOUR GLACIER");

					dVal = static_cast<double>(x);
					if(dVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. DEGRE JOUR GLACIER. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_taux_fonte_m1[index_zone] = vValeur[1];
						_seuil_fonte_m1[index_zone] = vValeur[2];
						_albedo_m1[index_zone] = vValeur[3];
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; DEGRE JOUR GLACIER; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele DEGRE JOUR GLACIER");
	}


	void DEGRE_JOUR_GLACIER::SauvegardeParametres()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "CLASSE INTEGRE GLACIER;";
		auto index_m1 = PrendreIndexOccupationM1();
		for (auto iter = begin(index_m1); iter != end(index_m1); iter++)
			fichier << *iter + 1 << ';';
		fichier << endl << endl;

		fichier << "DENSITE GLACE(kg/m3);" << _densite_glace << endl << endl;

		fichier << "CONSTANTE EMPIRIQUE 0;" << _c0 << endl;
		fichier << "CONSTANTE EMPIRIQUE 1;" << _c1 << endl << endl;

		fichier << "EPAISSEUR GLACE MIN(m);" << _dEpaisseurGlaceMin << endl;
		fichier << "EPAISSEUR GLACE MAX(m);" << _dEpaisseurGlaceMax << endl << endl;

		if(_bFixedIceMass)
			fichier << "MASSE GLACE FIXE(0/1);1" << endl << endl;
		else
			fichier << "MASSE GLACE FIXE(0/1);0" << endl << endl;

		fichier << "UHRH ID; TAUX DE FONTE [mm/jour/dC]; SEUIL FONTE [dC]; ALBEDO [0-1]" << endl;
		for (size_t index=0; index<zones.PrendreNbZone(); index++)
		{
			fichier << zones[index].PrendreIdent() << "; ";

			fichier << _taux_fonte_m1[index] << "; ";
			fichier << _seuil_fonte_m1[index] << "; ";
			fichier << _albedo_m1[index] << endl;
		}
	}

}
