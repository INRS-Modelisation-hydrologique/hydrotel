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

#include "cequeau.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"
#include "constantes.hpp"

#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	CEQUEAU::CEQUEAU(SIM_HYD& sim_hyd) : BILAN_VERTICAL(sim_hyd, "CEQUEAU")
	{
		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;
	}

	CEQUEAU::~CEQUEAU()
	{
	}

	void CEQUEAU::ChangeNbParams(const ZONES& zones)
	{
		const size_t nb_zone = zones.PrendreNbZone();

		//valeurs par defaut
		
		_sol.resize(nb_zone, 0.0f);
		_nappe.resize(nb_zone, 0.0f);
		_lacma.resize(nb_zone, 0.0f);

		_seuil_min_rui.resize(nb_zone, 0.0f);
		_seuil_max_sol.resize(nb_zone, 75.0f);
		_seuil_vid_sol.resize(nb_zone, 60.0f);
		_coef_vid_sol1.resize(nb_zone, 0.35f);
		_coef_vid_sol2.resize(nb_zone, 0.35f);
		_seuil_perc_sol.resize(nb_zone, 60.0f);
		_coef_perc_sol.resize(nb_zone, 0.15f);
		_max_perc_sol.resize(nb_zone, 10.0f);
		_seuil_etp_etr.resize(nb_zone, 60.0f);
		_coef_rec_nappe_haute.resize(nb_zone, 0.02f);
		_coef_rec_nappe_basse.resize(nb_zone, 0.02f);
		_fract_etp_nappe.resize(nb_zone, 0.0f);
		_niv_vid_nappe.resize(nb_zone, 50.0f);
		_seuil_vid_lacma.resize(nb_zone, 250.0f);
		_coef_vid_lacma.resize(nb_zone, 0.025f);
		_init_sol.resize(nb_zone, 65.0f);
		_init_nappe.resize(nb_zone, 30.0f);
		_init_lacma.resize(nb_zone, 250.0f);
	}

	void CEQUEAU::Initialise()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		const size_t nb_zone = zones.PrendreNbZone();
		auto& occupation_sol = _sim_hyd.PrendreOccupationSol();

		// calcul pourcentage des classes integrees

		size_t index_zone;
		float pourcentage;

		_freau.resize(nb_zone, 0.0f);
		for (index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			pourcentage = 0.0f;

			for (auto index = begin(_index_eaux); index != end(_index_eaux); ++index)
				pourcentage+= occupation_sol.PrendrePourcentage(index_zone, *index);

			_freau[index_zone] = pourcentage;
		}

		_frimp.resize(nb_zone, 0.0f);
		for (index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			pourcentage = 0.0f;

			for (auto index = begin(_index_impermeables); index != end(_index_impermeables); ++index)
				pourcentage+= occupation_sol.PrendrePourcentage(index_zone, *index);

			_frimp[index_zone] = pourcentage;
		}

		_frfor.resize(nb_zone, 0.0f);
		for (index_zone = 0; index_zone < nb_zone; ++index_zone)
		{
			pourcentage = 0.0f;

			for (auto index = begin(_index_forets); index != end(_index_forets); ++index)
				pourcentage+= occupation_sol.PrendrePourcentage(index_zone, *index);

			_frfor[index_zone] = pourcentage;
		}

		//initialisation milieux humides isoles
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		if(_milieu_humide_isole.size() != 0)
		{
			// initialise le volume des milieux humides isoles au volume normal
			for (size_t idx = 0; idx < index_zones.size(); ++idx)
			{
				index_zone = index_zones[idx];
				if (_milieu_humide_isole[index_zone])
				{
					_milieu_humide_isole[index_zone]->SetWetvol(_milieu_humide_isole[index_zone]->m_eauIni * _milieu_humide_isole[index_zone]->GetWetnvol());

					if(_milieu_humide_isole[index_zone]->GetSauvegarde())
					{
						SMilieuHumideResult* pRes = new SMilieuHumideResult;
						int id = zones[index_zone].PrendreIdent();					
						_milieu_humide_result[id] = pRes;
					}
				}
			}

			//fichier output
			string path = Combine(_sim_hyd.PrendreRepertoireResultat(), "wetland_isole.csv");
			m_wetfichier.open(path.c_str());
		}

		if (!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		BILAN_VERTICAL::Initialise();
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void CEQUEAU::Calcule()
	{
		float frter;              // Frac. du carreau occupÇ par le sol.
		float arr27;
		float eauter;             // Hauteur de la lame d'eau venant alimenter le sol (mm).
		float vidint;             // Hauteur de la lame de l'ecoulement retarde.
		float slama;              // Hauteur de la lame ecoulee des lacs et marais.
		float rimp;               // Hauteur de la lame ruissele sur les surf. imper.
		float ruiss;              // Hauteur de la lame d'eau ruissele en surface.
		float xinf;               // Hauteur de la lame infiltree du sol a la nappe.
		float sonap;              // Ecoulement en provenance de la nappe (mm).
		float mevp;               // Moyenne des evp de chaque classe d'occupation du territoire.
		//float mevpeau;            // Moyenne des evp pour classe d'occupation "eau".
		float etrlac;             // Etr en provenance des lac et marais.
		float etrnap;             // Etr puisee dans la nappe.
		float etrsol;             // Etr puisee dans le sol.
		float etot;               // Etp potentielle sur le sol modulee par l'occ. du sol.
		float evnaf;              // Frac. effective de l'etp puisee dans la nappe ([0, 1]).

		float t_min, t_max, apport, v_sol, v_nappe, v_lacma, prod, fTemp, fTemp2, fCouvertNivalEau, fProfondeurGel;
		int index;

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		ZONES& zones = _sim_hyd.PrendreZones();
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();
		int nb_zone_simule = static_cast<int>(index_zones.size());
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		for (index = 0; index < nb_zone_simule; ++index)
		{				
			size_t index_zone = index_zones[index];
			ZONE& zone = zones[index_zone];

			//int jj, mm, aa, hh;
			//
			//aa = date_courante.PrendreAnnee();
			//mm = date_courante.PrendreMois();
			//jj = date_courante.PrendreJour();
			//hh = date_courante.PrendreHeure();

			apport = zone.PrendreApport() + static_cast<float>(zone.PrendreApportGlacier());	//mm

			//if(zone.PrendreTypeZone() == ZONE::SOUS_BASSIN)
			//{

			zone.ChangeProdSurf(0.0f);
			zone.ChangeProdHypo(0.0f);
			zone.ChangeProdBase(0.0f);

			t_min = zone.PrendreTMin();
			t_max = zone.PrendreTMax();

   			v_sol = _sol[index_zone];
	   		v_nappe = _nappe[index_zone];
   			v_lacma = _lacma[index_zone];

			frter = max(1.0f - _freau[index_zone], 0.0f);
			arr27 = (4.0f + _freau[index_zone] + _frfor[index_zone]) / 5.0f;

			//alimentation et percolation du reservoir sol
			fCouvertNivalEau = zone.PrendreCouvertNival(); //mm
			fProfondeurGel = zone.PrendreProfondeurGel(); //cm

			if(fProfondeurGel > 0.0f && fCouvertNivalEau < 10.0f)
				rimp = apport - _seuil_min_rui[index_zone];
			else
	   			rimp = _frimp[index_zone] * (apport - _seuil_min_rui[index_zone]);
			
			if(rimp < 0.0f)
				rimp = 0.0f;

   			eauter = apport - rimp;
	   		v_sol+= eauter;

   			xinf = v_sol - _seuil_perc_sol[index_zone];
	   		if (xinf < 0.0f)
				xinf = 0.0f;

			xinf*= _coef_perc_sol[index_zone];
    		if(xinf > _max_perc_sol[index_zone])
				xinf = _max_perc_sol[index_zone];

			xinf*= arr27;

			fTemp = (_seuil_max_sol[index_zone] - _seuil_perc_sol[index_zone]) / 10.0f;	//mm -> cm
			if(fProfondeurGel > VALEUR_MANQUANTE && fProfondeurGel > fTemp && fCouvertNivalEau < 10.0f)
				xinf*= 0.5f;	//diminue l'infiltration de 50%

   			//bilan du reservoir nappe
			if(v_nappe > _niv_vid_nappe[index_zone])
				sonap = _coef_rec_nappe_haute[index_zone] * (v_nappe - _niv_vid_nappe[index_zone]) + _coef_rec_nappe_basse[index_zone] * _niv_vid_nappe[index_zone];
			else
				sonap = _coef_rec_nappe_basse[index_zone] * v_nappe;

			sonap = min(sonap, v_nappe + xinf);
			v_nappe = v_nappe - sonap + xinf;

   			//eêvapotranspiration
			ruiss = 0.0f;
   			vidint = 0.0f;
	   		etrlac = 0.0f;
   			etrnap = 0.0f;
	   		etrsol = 0.0f;
   			etot = 0.0f;

			///////////////////////////////////////////////////////////////
			//if (etr_disp)		//etr_disp tjrs ‡ false ds version 2.6...
			//{
			//	mevpeau= (float)0;
			//	mevp= (float)0;

			//	for (k= 0; k < nb_ind_lacma; k++)
			//	{
			//		(*zones)(i).PrendreEvp((*cot_lacma)[k], evp);
			//		mevpeau+= evp;
			//	}

			//	for (k= 0; k < nb_cot; k++)
			//	{
			//		(*zones)(i).PrendreEvp(k, evp);
			//		mevp+= evp;
			//	}

			//	mevp-= mevpeau;
			//	mevpeau= mevp/ nb_ind_lacma*1000;      /* x 1000 pour avoir mm */
			//	mevp= mevp/(nb_cot - nb_ind_lacma)*1000; /* x 1000 pour avoir mm */

			//	etot= mevp * arr27;
			//	evnaf= (float)(Min(1.0f, fract_etp_nappe*v_nappe / (niv_vid_nappe+25.0f)));
			//	etrnap= (float)(Min(v_nappe, etot*evnaf));
			//	v_nappe -= etrnap;
			//	etrsol= etot - etrnap;
			//	etrlac= mevpeau;
			//}
			//else if((t_min + t_max) / 2.0f > 0.0f)
			///////////////////////////////////////////////////////////////

			if((t_min + t_max) / 2.0f > 0.0f)
	   		{
				mevp = zone.PrendreEtpTotal();	//mm

	   			etot = mevp * arr27;
		   		evnaf = min(1.0f, _fract_etp_nappe[index_zone] * v_nappe / (_niv_vid_nappe[index_zone] + 25.0f));
				etrnap = min(v_nappe, etot * evnaf);
   				v_nappe-= etrnap;
	   			etrsol = etot - etrnap;
   				etrlac = mevp * 0.8f;
	   		}

			//bilan du reservoir sol
   			if(v_sol <= _seuil_etp_etr[index_zone])
				etrsol = etrsol * (v_sol / (_seuil_etp_etr[index_zone] + 0.2f));

			v_sol-= xinf;
   			if(v_sol <= 0.0f)
	   		{
				v_sol = 0.0f;
   				etrsol = 0.0f;
			}
	   		else
   			{
	   			etrsol = min(v_sol, etrsol);
		   		v_sol-= etrsol;

				fTemp = (_seuil_max_sol[index_zone] - _seuil_vid_sol[index_zone]) / 10.0f;	//mm -> cm				
				fTemp2 = _seuil_vid_sol[index_zone] * _coef_vid_sol2[index_zone];

				if (v_sol > _seuil_max_sol[index_zone])
   				{
					ruiss = v_sol - _seuil_max_sol[index_zone];
					if(fProfondeurGel > 0.0f && fCouvertNivalEau < 10.0f)
						ruiss*= 0.5f;	//diminue le ruissellement de 50%

					vidint = (_seuil_max_sol[index_zone] - _seuil_vid_sol[index_zone]) * _coef_vid_sol1[index_zone];
					
					if(fProfondeurGel > VALEUR_MANQUANTE && fProfondeurGel > fTemp && fCouvertNivalEau < 10.0f)
						vidint*= 0.5f;	//diminue la vidange du sol de 50%

					if(fProfondeurGel > VALEUR_MANQUANTE && fProfondeurGel >= (_seuil_max_sol[index_zone] / 10.0f) && fCouvertNivalEau < 10.0f)
						fTemp2*= 0.5f;	//diminue la vidange du sol de 50%

					vidint+= fTemp2;
				}
   				else
				{
					if(v_sol > _seuil_vid_sol[index_zone])
					{
						vidint = (v_sol-_seuil_vid_sol[index_zone]) * _coef_vid_sol1[index_zone];

						if(fProfondeurGel > VALEUR_MANQUANTE && fProfondeurGel > fTemp && fCouvertNivalEau < 10.0f)
							vidint*= 0.5f;	//diminue la vidange du sol de 50%

						if(fProfondeurGel > VALEUR_MANQUANTE && fProfondeurGel >= (_seuil_max_sol[index_zone] / 10.0f) && fCouvertNivalEau < 10.0f)	//divise par 10; mm -> cm
							fTemp2*= 0.5f;	//diminue la vidange du sol de 50%

						vidint+= fTemp2;
					}
					else
					{
						vidint = v_sol * _coef_vid_sol2[index_zone];

						if(fProfondeurGel > VALEUR_MANQUANTE && fProfondeurGel >= (_seuil_max_sol[index_zone] / 10.0f) && fCouvertNivalEau < 10.0f)
							vidint*= 0.5f;	//diminue la vidange du sol de 50%
					}
				}

				vidint = min(vidint, v_sol);
			}
   					
			v_sol = v_sol - (ruiss + vidint);

			//reservoir lacs et marais
   			v_lacma+= apport;
	   		etrlac = min(etrlac, v_lacma);
			v_lacma-= etrlac;

			slama = (v_lacma - _seuil_vid_lacma[index_zone]) *_coef_vid_lacma[index_zone];

   			slama = max(0.0f, slama);
	   		v_lacma-= slama;

			prod = max(0.0f, (rimp + ruiss) * frter + slama * _freau[index_zone]);
			zone.ChangeProdSurf(prod);	//mm
			
			zone.ChangeProdHypo(vidint * frter);		//mm
			zone.ChangeProdBase(sonap * frter);			//mm

	   		_sol[index_zone] = v_sol;
			_nappe[index_zone] = v_nappe;
   			_lacma[index_zone] = v_lacma;

			//}
			//else
			//{
			//	if(zone.PrendreTypeZone() == ZONE::LAC)
			//	{
			//		//si la zone est un lac l'eau provenant de l'apport (pluie+fonte) est disponible, i.e. pas d'infiltration.
			//		zone.ChangeProdSurf(apport);	//mm
			//		zone.ChangeProdHypo(0.0f);		//mm
			//		zone.ChangeProdBase(0.0f);		//mm
			//	}
			//}

			//if(zone.PrendreIdent() == 1)
			//	fichier << jj << ";" << mm << ";" << apport << ";" << prod + (vidint * frter) + (sonap * frter) << ";" << _sol[index_zone] << ";" << _nappe[index_zone] << ";" << _lacma[index_zone] << "\n";

			// calcul des milieux humides isoles
			MILIEUHUMIDE_ISOLE* pMilieuHumide = nullptr;

			if (_milieu_humide_isole.size() > 0)
				pMilieuHumide = _milieu_humide_isole[index_zone];

			if (pMilieuHumide)
			{
				float evp, superficie, prodOld, surfOld, hypOld, baseOld, tempo;

				evp = zone.PrendreEtpTotal();
				apport = zone.PrendreApport() + static_cast<float>(zone.PrendreApportGlacier());
					
				surfOld = zone.PrendreProdSurf();
				hypOld = zone.PrendreProdHypo();
				baseOld = zone.PrendreProdBase();

				prod = prodOld = surfOld + hypOld + baseOld;

				// calcul fraction superficie milieu humide
				superficie = pMilieuHumide->GetWetfr();

				// calcul fraction drainee milieu humide
				float wet_fr = pMilieuHumide->GetWetdrafr(); 

				apport = apport * superficie;

				float hru_ha = static_cast<float>(zone.PrendreSuperficie()) * 100.0f; //km2 -> ha
				int ident;

				ident = zone.PrendreIdent();

				CalculMilieuHumideIsole(pMilieuHumide, ident, hru_ha, wet_fr, evp, apport, prod, pas_de_temps);

				zone.ChangeApport(apport);
					
				if(prodOld != 0.0)
				{
					//repartie la production dans les 3 couches
					tempo = surfOld / prodOld * prod;
					zone.ChangeProdSurf(tempo);

					tempo = hypOld / prodOld * prod;
					zone.ChangeProdHypo(tempo);

					tempo = baseOld / prodOld * prod;
					zone.ChangeProdBase(tempo);
				}
				else
				{
					if(prod != 0.0f)
						zone.ChangeProdBase(prod);
				}
			}

		}

		//output milieu humide
		OUTPUT& output = _sim_hyd.PrendreOutput();
		
		if (_milieu_humide_isole.size() > 0)
		{
			int jj, mm, aa, hh;

			aa = date_courante.PrendreAnnee();
			mm = date_courante.PrendreMois();
			jj = date_courante.PrendreJour();
			hh = date_courante.PrendreHeure();

			for (auto iter = _milieu_humide_result.begin(); iter != _milieu_humide_result.end(); iter++)
			{
				m_wetfichier 
					<< iter->first << output.Separator()
					<< aa << output.Separator()
					<< mm << output.Separator()
					<< jj << output.Separator()
					<< hh << output.Separator()
					<< iter->second->apport << output.Separator()	// mm
					<< iter->second->evp << output.Separator()		// mm
					<< iter->second->wetsep << output.Separator()	// m^3
					<< _milieu_humide_isole[zones.IdentVersIndex(iter->first)]->GetWetvol() << output.Separator()	// m^3
					<< iter->second->wetflwi << output.Separator()	// m^3
					<< iter->second->wetflwo << output.Separator()	// m^3
					<< iter->second->wetprod << output.Separator()	// mm
					<< std::endl;
			}
		}

		//variable d'etat
		if (_sauvegarde_tous_etat || (_sauvegarde_etat && _date_sauvegarde_etat - pas_de_temps == date_courante))
			SauvegardeEtat(date_courante);

		BILAN_VERTICAL::Calcule();
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void CEQUEAU::CalculMilieuHumideIsole(MILIEUHUMIDE_ISOLE* pMilieuHumide, int ident, float hru_ha, 
											float wet_fr, float evp, float& apport, float& prod, unsigned short pdt)
	{
		//const float wet_k = 0.5f; // mm/h <---------- PARAMETRE FICHIER
		const float wet_k = pMilieuHumide->GetKsatBs(); // mm/h

		const float bw1 = pMilieuHumide->GetB();
		const float bw2 = pMilieuHumide->GetA();

		float wet_nvol = pMilieuHumide->GetWetnvol();
		float wet_vol = pMilieuHumide->GetWetvol();
		float wet_mxvol = pMilieuHumide->GetWetmxvol();

		// calcul de la surface
		float wetsa =  bw1 * pow(wet_vol, bw2) / 10000;

		// calcul du bilan hydrique 
		//float wetev = 10.0f * 0.6f * evp * wetsa; // 0.6f <---------- PARAMETRE FICHIER
		float wetev = 10.0f * pMilieuHumide->GetCEv() * evp * wetsa; 
		float wetsep = wet_k * wetsa * (pdt * 10);
		float wetpcp = apport * wetsa * 10;

		// calcul de l'apport dans le milieu humide
		float wetflwi = prod * 10 * (hru_ha * wet_fr - wetsa);
		prod = prod - prod * wet_fr;

		// calcul du nouveau volume
		wet_vol = wet_vol - wetsep - wetev + wetflwi + wetpcp;

		if (wet_vol < 0.001)
		{
			wetsep = wetsep + wet_vol;
			wet_vol = 0;

			if (wetsep < 0)
			{
				wetev = wetev + wetsep;
				wetsep = 0;
			}
		}

		// calcul de la production du milieu humide
		float wetflwo = 0.0f;
		
		if (wet_vol > wet_nvol)
		{
			if (wet_vol <= wet_mxvol)
			{
				//wetflwo = (wet_vol - wet_nvol) / 10; // <---------- PARAMETRE FICHIER
				wetflwo = (wet_vol - wet_nvol) / pMilieuHumide->GetCProd();
				wet_vol = wet_vol - wetflwo;
			}
			else
			{
				wetflwo = wet_vol - wet_mxvol;
				wet_vol = wet_mxvol;
			}
		}

		float wetprod = wetflwo / (hru_ha * 10) + wetsep / (hru_ha * 10);
		prod = prod + wetprod;

		pMilieuHumide->SetWetvol(wet_vol);

		if (pMilieuHumide->GetSauvegarde())
		{
			_milieu_humide_result[ident]->apport = apport;
			_milieu_humide_result[ident]->evp = evp;
			_milieu_humide_result[ident]->wetsep = wetsep;
			_milieu_humide_result[ident]->wetflwi = wetflwi;
			_milieu_humide_result[ident]->wetflwo = wetflwo;
			_milieu_humide_result[ident]->wetprod = wetprod;
		}
	}

	void CEQUEAU::Termine()
	{
		if(m_wetfichier.is_open())
			m_wetfichier.close();

		BILAN_VERTICAL::Termine();
	}

	
	void CEQUEAU::LectureParametres()
	{
		if(_sim_hyd.PrendreNomBilanVertical() == PrendreNomSousModele())	//si le modele CEQUEAU est simulÈ
		{
			if(_sim_hyd._fichierParametreGlobal)
			{
				LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activÈ
				return;
			}

			ZONES& zones = _sim_hyd.PrendreZones();

			ifstream fichier( PrendreNomFichierParametres() );
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES CEQUEAU");

			string cle, valeur, ligne;
			lire_cle_valeur(fichier, cle, valeur);

			if (cle != "PARAMETRES HYDROTEL VERSION")
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES CEQUEAU", 1);
			}

			getline_mod(fichier, ligne);				//ligne vide
			lire_cle_valeur(fichier, cle, valeur);	//nom du sous modele
			getline_mod(fichier, ligne);				//ligne vide

			// lecture des classes integrees

			lire_cle_valeur(fichier, cle, valeur);
			_index_eaux = extrait_valeur(valeur);			//CLASSE INTEGRE EAU (LACS ET MARECAGES)
			_index_eaux.shrink_to_fit();

			lire_cle_valeur(fichier, cle, valeur);
			_index_impermeables = extrait_valeur(valeur);	//CLASSE INTEGRE IMPERMEABLE
			_index_impermeables.shrink_to_fit();

			lire_cle_valeur(fichier, cle, valeur);
			_index_forets = extrait_valeur(valeur);			//CLASSE INTEGRE FORETS
			_index_forets.shrink_to_fit();

			getline_mod(fichier, ligne);	//ligne vide
			getline_mod(fichier, ligne);	//commentaire

			size_t index_zone, index;
			int ident;
			char c;

			//parametres UHRHs
			for (index = 0; index < zones.PrendreNbZone(); ++index)
			{
				fichier >> ident >> c;

				index_zone = zones.IdentVersIndex(ident);
				
				fichier >> _seuil_min_rui[index_zone] >> c;
				fichier >> _seuil_max_sol[index_zone] >> c;
				fichier >> _seuil_vid_sol[index_zone] >> c;
				fichier >> _coef_vid_sol1[index_zone] >> c;
				fichier >> _coef_vid_sol2[index_zone] >> c;
				fichier >> _seuil_perc_sol[index_zone] >> c;
				fichier >> _coef_perc_sol[index_zone] >> c;
				fichier >> _max_perc_sol[index_zone] >> c;		//mm/j
				fichier >> _seuil_etp_etr[index_zone] >> c;
				fichier >> _coef_rec_nappe_haute[index_zone] >> c;
				fichier >> _coef_rec_nappe_basse[index_zone] >> c;
				fichier >> _fract_etp_nappe[index_zone] >> c;
				fichier >> _niv_vid_nappe[index_zone] >> c;
				fichier >> _seuil_vid_lacma[index_zone] >> c;
				fichier >> _coef_vid_lacma[index_zone] >> c;
				fichier >> _init_sol[index_zone] >> c;
				fichier >> _init_nappe[index_zone] >> c;
				fichier >> _init_lacma[index_zone];				

				_sol[index_zone] = _init_sol[index_zone];
				_nappe[index_zone] = _init_nappe[index_zone];
				_lacma[index_zone] = _init_lacma[index_zone];
			}

			fichier.close();

			//lecture des parametres pour les milieux humides isolÈes
			if(_sim_hyd._bSimuleMHIsole)
				LectureMilieuHumideIsole();
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void CEQUEAU::LectureParametresFichierGlobal()
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; " + _sim_hyd._nomFichierParametresGlobal, 1 );

		size_t nbGroupe, x, y, index_zone;
		float fVal;
		int no_ligne = 2;
		int ident;

		nbGroupe = _sim_hyd.PrendreNbGroupe();

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "CEQUEAU")
			{
				++no_ligne;
				getline_mod(fichier, ligne);
				_index_eaux = extrait_valeur(ligne);
				_index_eaux.shrink_to_fit();

				++no_ligne;
				getline_mod(fichier, ligne);				
				_index_impermeables = extrait_valeur(ligne);
				_index_impermeables.shrink_to_fit();

				++no_ligne;
				getline_mod(fichier, ligne);				
				_index_forets = extrait_valeur(ligne);
				_index_forets.shrink_to_fit();

				for(x=0; x<nbGroupe; x++)
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					auto vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 19)
						throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; CEQUEAU; " + _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide.");

					fVal = static_cast<float>(x);
					if(fVal != vValeur[0])
						throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; CEQUEAU; " + _sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. Les ID de groupe doivent etre en ordre croissant.");

					for(y=0; y<_sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
					{
						ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
						index_zone = zones.IdentVersIndex(ident);

						_seuil_min_rui[index_zone] = vValeur[1];
						_seuil_max_sol[index_zone] = vValeur[2];
						_seuil_vid_sol[index_zone] = vValeur[3];
						_coef_vid_sol1[index_zone] = vValeur[4];
						_coef_vid_sol2[index_zone] = vValeur[5];
						_seuil_perc_sol[index_zone] = vValeur[6];
						_coef_perc_sol[index_zone] = vValeur[7];
						_max_perc_sol[index_zone] = vValeur[8];		//mm/j
						_seuil_etp_etr[index_zone] = vValeur[9];
						_coef_rec_nappe_haute[index_zone] = vValeur[10];
						_coef_rec_nappe_basse[index_zone] = vValeur[11];
						_fract_etp_nappe[index_zone] = vValeur[12];
						_niv_vid_nappe[index_zone] = vValeur[13];
						_seuil_vid_lacma[index_zone] = vValeur[14];
						_coef_vid_lacma[index_zone] = vValeur[15];
						_init_sol[index_zone] = vValeur[16];
						_init_nappe[index_zone] = vValeur[17];
						_init_lacma[index_zone]  = vValeur[18];

						_sol[index_zone] = _init_sol[index_zone];
						_nappe[index_zone] = _init_nappe[index_zone];
						_lacma[index_zone] = _init_lacma[index_zone];
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; CEQUEAU; " + _sim_hyd._nomFichierParametresGlobal);
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; CEQUEAU; " + _sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele CEQUEAU");

		//lecture des parametres pour les milieux humides isolÈes
		if(_sim_hyd._bSimuleMHIsole)
			LectureMilieuHumideIsole();
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void CEQUEAU::LectureMilieuHumideIsole()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		const size_t nb_zone = zones.PrendreNbZone();

		vector<string> sList;
		istringstream iss;
		ostringstream oss;
		string line, mess;
		size_t index;
		float fSuperficie, fSuperficieWet, fWetDraFr, frac, wetdnor, wetdmax, ksat_bs, c_ev, c_prod, eauIni;
		bool sauvegarde;
		int nUhrhId, lineNumber;

		ifstream file(_sim_hyd._nom_fichier_milieu_humide_isole);
		if (!file)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES MILIEUX HUMIDES ISOLES" );

		lineNumber = 1;

		_milieu_humide_isole.resize(nb_zone, 0);

		try{

			getline_mod(file, line);  //entete

			while(!file.eof())
			{
				++lineNumber;
				getline_mod(file, line);

				if(line != "")
				{
					//line.erase(remove(line.begin(), line.end(), 'f'), line.end());	//HACK; pour fixer un ancien bug dans le fichier d'entree

					SplitString(sList, line, ";", true, true);

					if(sList.size() >= 11)
					{
						iss.clear();
						iss.str(sList[0]);
						iss >> nUhrhId;

						iss.clear();
						iss.str(sList[1]);
						iss >> fSuperficie;

						iss.clear();
						iss.str(sList[2]);
						iss >> fSuperficieWet;

						iss.clear();
						iss.str(sList[3]);
						iss >> fWetDraFr;

						iss.clear();
						iss.str(sList[4]);
						iss >> frac;

						iss.clear();
						iss.str(sList[5]);
						iss >> wetdnor;

						iss.clear();
						iss.str(sList[6]);
						iss >> wetdmax;

						iss.clear();
						iss.str(sList[7]);
						iss >> ksat_bs;

						iss.clear();
						iss.str(sList[8]);
						iss >> c_ev;

						iss.clear();
						iss.str(sList[9]);
						iss >> c_prod;

						iss.clear();
						iss.str(sList[10]);
						iss >> sauvegarde;

						if(sList.size() >= 12)
						{
							iss.clear();
							iss.str(sList[11]);
							iss >> eauIni;
						}
						else
							eauIni = 1.0f;

						try{
							index = zones.IdentVersIndex(nUhrhId);
						}
						catch(const exception& e)
						{
							mess = e.what();

							try{
								index = zones.IdentVersIndex(-nUhrhId);
							}
							catch(...)
							{
								mess = "Lecture des milieux humides isoles; " + mess;
								throw ERREUR(mess);
							}
						}

						if(fWetDraFr == 0.0f)
							fWetDraFr = fSuperficieWet / fSuperficie;

						if(wetdnor > wetdmax)
							throw ERREUR("Reading of isolated wetlands; the wetdnor parameter must be less than the wetdmax parameter.");

						if(eauIni < 0.0f || eauIni > 1.0f)
							throw ERREUR("Reading of isolated wetlands; the eauIni parameter must be >= 0 and <= 1.");

						_milieu_humide_isole[index] = new MILIEUHUMIDE_ISOLE(fSuperficieWet, (fSuperficieWet / fSuperficie), fWetDraFr, frac, wetdnor, wetdmax, sauvegarde, ksat_bs, c_ev, c_prod, eauIni);
					}
					else
					{
						oss.str("");
						oss << "CEQUEAU: error LectureMilieuHumideIsole: line number " << lineNumber << " is invalid.";
						throw ERREUR(oss.str());
					}
				}
			}

		}
		catch(const ERREUR& err)
		{
			file.close();
			throw err;
		}

		catch(...)
		{
			if(!file.eof())
			{
				file.close();
				throw ERREUR("CEQUEAU: error LectureMilieuHumideIsole");
			}
		}

		file.close();
	}

	void CEQUEAU::SauvegardeParametres()
	{
		if(PrendreNomFichierParametres() == "")	//creation d'un fichier par defaut s'il n'existe pas
			ChangeNomFichierParametres(Combine(_sim_hyd.PrendreRepertoireSimulation(), "cequeau.csv"));

		ZONES& zones = _sim_hyd.PrendreZones();

		string nom_fichier = PrendreNomFichierParametres();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "CLASSE INTEGRE EAU (LACS ET MARECAGES);";
		for (auto iter = begin(_index_eaux); iter != end(_index_eaux); ++iter)
			fichier << *iter + 1 << ';';
		fichier << endl;

		fichier << "CLASSE INTEGRE IMPERMEABLE;";
		for (auto iter = begin(_index_impermeables); iter != end(_index_impermeables); ++iter)
			fichier << *iter + 1 << ';';
		fichier << endl;

		fichier << "CLASSE INTEGRE FORETS;";
		for (auto iter = begin(_index_forets); iter != end(_index_forets); ++iter)
			fichier << *iter + 1 << ';';
		fichier << endl << endl;

		ostringstream oss;

		fichier << "UHRH ID; SEUIL MINIMAL RUISSELLEMENT SURFACE IMPERMEABLE; NIVEAU EAU MAXIMAL (SOL); SEUIL VIDANGE (SOL); COEFFICIENT VIDANGE RETARDE 1 (SOL); COEFFICIENT VIDANGE RETARDE 2 (SOL); SEUIL PERCOLATION (SOL); COEFFICIENT PERCOLATION (SOL); TAUX MAXIMUM PERCOLATION (SOL) (mm/j); NIVEAU EAU (SOL) ETP=ETR; COEFFICIENT VIDANGE HAUTE (NAPPE); COEFFICIENT VIDANGE BASSE (NAPPE); FRACTION ETP PUISEE (NAPPE)(0:1); HAUTEUR VIDANGE HAUTE (NAPPE); SEUIL VIDANGE (EAU); COEFFICIENT VIDANGE (EAU); INITIALISATION (SOL); INITIALISATION(NAPPE); INITIALISATION (EAU)" << endl;
		for (size_t index_zone = 0; index_zone < zones.PrendreNbZone(); ++index_zone)
		{
			fichier << zones[index_zone].PrendreIdent() << ';';

			fichier << _seuil_min_rui[index_zone] << ';';
			fichier << _seuil_max_sol[index_zone] << ';';
			fichier << _seuil_vid_sol[index_zone] << ';';
			fichier << _coef_vid_sol1[index_zone] << ';';
			fichier << _coef_vid_sol2[index_zone] << ';';
			fichier << _seuil_perc_sol[index_zone] << ';';
			fichier << _coef_perc_sol[index_zone] << ';';
			fichier << _max_perc_sol[index_zone] << ';';		//mm/j
			fichier << _seuil_etp_etr[index_zone] << ';';
			fichier << _coef_rec_nappe_haute[index_zone] << ';';
			fichier << _coef_rec_nappe_basse[index_zone] << ';';
			fichier << _fract_etp_nappe[index_zone] << ';';
			fichier << _niv_vid_nappe[index_zone] << ';';
			fichier << _seuil_vid_lacma[index_zone] << ';';
			fichier << _coef_vid_lacma[index_zone] << ';';
			fichier << _init_sol[index_zone] << ';';
			fichier << _init_nappe[index_zone] << ';';
			fichier << _init_lacma[index_zone] << endl;		
		}

		fichier.close();
	}

	void CEQUEAU::ChangeNomFichierLectureEtat(string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}

	void CEQUEAU::ChangeRepertoireEcritureEtat(string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}

	void CEQUEAU::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}

	void CEQUEAU::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}

	void CEQUEAU::LectureEtat(DATE_HEURE date_courante)
	{
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; fichier etat CEQUEAU; " + _nom_fichier_lecture_etat);

		vector<int> vValidation;
		string ligne;
		size_t index_zone;
		int iIdent;

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);

		while(!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne != "")
			{
				auto valeurs = extrait_fvaleur(ligne, _sim_hyd._output._sFichiersEtatsSeparator);
				iIdent = static_cast<int>(valeurs[0]);

				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

					_sol[index_zone] = valeurs[1];
					_nappe[index_zone] = valeurs[2];
   					_lacma[index_zone] = valeurs[3];

					if(_milieu_humide_isole.size() != 0 && _milieu_humide_isole[index_zone] && valeurs.size() >= 5)
						_milieu_humide_isole[index_zone]->SetWetvol(valeurs[4]);

					vValidation.push_back(iIdent);
				}
			}
		}

		fichier.close();

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("BILAN_VERTICAL; fichier etat CEQUEAU; id mismatch; " + _nom_fichier_lecture_etat);
	}

	void CEQUEAU::SauvegardeEtat(DATE_HEURE date_courante) const
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier, oss;
		size_t x, nbSimuler, index_zone;
		string sSep = _sim_hyd._output._sFichiersEtatsSeparator;
		float fVal;

		if(!RepertoireExiste(_repertoire_ecriture_etat))
			CreeRepertoire(_repertoire_ecriture_etat);

		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			        << "bilan_vertical_" 
			        << setw(4) << date_courante.PrendreAnnee() 
			        << setw(2) << date_courante.PrendreMois() 
			        << setw(2) << date_courante.PrendreJour() 
			        << setw(2) << date_courante.PrendreHeure() 
					<< ".csv";

		ofstream fichier(nom_fichier.str());
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier.exceptions(ios::failbit | ios::badbit);

		fichier << "ETATS BILAN VERTICAL" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier << "DATE_HEURE" << sSep << date_courante << endl;
		fichier << endl;

		fichier << "UHRH" << sSep << " SOL (mm)" << sSep << " NAPPE (mm)" << sSep << " EAU (mm)" << endl;

		ZONES& zones = _sim_hyd.PrendreZones();

		nbSimuler = _sim_hyd.PrendreZonesSimules().size();
		x = 0;

		for (index_zone=0; x<nbSimuler; index_zone++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index_zone];

				oss.str("");
				oss << zone.PrendreIdent() << sSep;

				oss << setprecision(12) << setiosflags(ios::fixed);

				oss << _sol[index_zone] << sSep;
				oss << _nappe[index_zone] << sSep;
				oss << _lacma[index_zone] << sSep;

				fVal = -999.0f;
				if(_milieu_humide_isole.size() != 0 && _milieu_humide_isole[index_zone])
					fVal = _milieu_humide_isole[index_zone]->GetWetvol();
				oss << fVal;

				fichier << oss.str() << endl;
				++x;
			}
		}

		fichier.close();
	}

}
