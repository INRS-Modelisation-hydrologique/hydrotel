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

#include "thorsen.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"
#include "bilan_vertical.hpp"
#include "constantes.hpp"

#include <cmath>
#include <algorithm>



using namespace std;


namespace HYDROTEL
{

	THORSEN::THORSEN(SIM_HYD& sim_hyd)
		: TEMPSOL(sim_hyd, "THORSEN")
	{
		_fProfondeurInitialeGel = 0.0f;
		_fParamEmpirique1 = 8.0f;
		_fTempGelEau = 0.0f;
		_fTeneurEau = 0.4f;

		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;
	}


	THORSEN::~THORSEN()
	{
	}


	void THORSEN::ChangeNbParams(const ZONES& zones)
	{
		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();
		if(propriete_hydroliques._bDisponible)
			_vfParamKT.resize(propriete_hydroliques.PrendreNb(), 0.8f);
		else
			_vfParamKT.resize(1, 0.8f);

		TEMPSOL::ChangeNbParams(zones);
	}


	void THORSEN::Initialise()
	{
		size_t index, index_zone;

		ZONES& zones = _sim_hyd.PrendreZones();
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		//valeurs initiales profondeur du gel
		for (index = 0; index < index_zones.size(); ++index)
		{
			index_zone = index_zones[index];
			zones[index_zone].ChangeProfondeurGel(_fProfondeurInitialeGel * 100.0f);	//m -> cm
			
			//initialise les valeurs de theta
			//s'il ne sont pas disponible lors de la simulation (ex modele cequeau), ils ne seront pas maj par le modele bilanvertical; cette valeur sera utilisé par defaut tous le long de la simulation

			//ds le cas ou les theta sont disponible (ex bv3c), les valeurs seront ecrasé par bv3c (egalement la valeur initiale..)
			//utilise theta ini de bv3c la 1ere fois, ensuite utilise la valeur calculé pour le jour précédent (le modele temp sol est appelé avant le modele bilanvertical)
			zones[index_zone]._theta1 = _fTeneurEau;
			zones[index_zone]._theta2 = _fTeneurEau;
			zones[index_zone]._theta3 = _fTeneurEau;
		}

		if (!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		TEMPSOL::Initialise();
	}


	void THORSEN::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();
		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();
		float fProfondeurGel;

		size_t index_zone, index, indexTypeSol;
		float fTAir, fB, fTemp, fDs, fDeltaT, fTheta;

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		fDeltaT = pas_de_temps * 60.0f * 60.0f;
		indexTypeSol = 0;

		for (index = 0; index < index_zones.size(); ++index)
		{
			index_zone = index_zones[index];

			fTAir = (zones[index_zone].PrendreTMax() + zones[index_zone].PrendreTMin()) / 2.0f;	//temperature de l'air

			fDs = zones[index_zone].PrendreHauteurCouvertNival(); //m
			if(fDs == -999.0)
				throw ERREUR("Erreur TEMPSOL:Calcule; lecture couvert nival");

			fProfondeurGel = zones[index_zone].PrendreProfondeurGel() / 100.0f; //cm -> m

			if(fDs != 0.0f)
			{
				if(fProfondeurGel == 0.0f)
					fB = fTAir * exp(-_fParamEmpirique1 * fDs);
				else
					fB = fTAir / (1.0f + 10.0f * (fDs / fProfondeurGel));
			}
			else
				fB = fTAir;

			if(propriete_hydroliques._bDisponible)
			{
				if(fProfondeurGel > zones[index_zone].PrendreZ11())
				{
					if(fProfondeurGel > zones[index_zone].PrendreZ11() + zones[index_zone].PrendreZ22())
						indexTypeSol = propriete_hydroliques.PrendreIndexCouche3(index_zone);	//type de sol dominant couche 3
					else
						indexTypeSol = propriete_hydroliques.PrendreIndexCouche2(index_zone);	//type de sol dominant couche 2
				}
				else
					indexTypeSol = propriete_hydroliques.PrendreIndexCouche1(index_zone);	//type de sol dominant couche 1
			}

			if(fProfondeurGel > zones[index_zone].PrendreZ11())	//la valeur pour theta est la valeur par defaut (_fTeneurEau) ou celle simulée par bv3c le cas echeant
			{
				if(fProfondeurGel > zones[index_zone].PrendreZ11() + zones[index_zone].PrendreZ22())
					fTheta = zones[index_zone]._theta3;
				else
					fTheta = zones[index_zone]._theta2;
			}
			else
				fTheta = zones[index_zone]._theta1;

			if(fTheta < 0.1f)
				fTheta = 0.1f;

			fTemp = pow(fProfondeurGel, 2.0f) + 2.0f * (_vfParamKT[indexTypeSol] * fDeltaT * (_fTempGelEau - fB) / (fTheta * DENSITE_EAU * CHALEUR_FONTE));	//CHALEUR_FONTE=CHALEUR LATENTE DE FUSION
				
			if(fTemp > 0.0f)
				fProfondeurGel = sqrt(fTemp);
			else
				fProfondeurGel = 0.0f;

			zones[index_zone].ChangeProfondeurGel(fProfondeurGel * 100.0f); //m -> cm
		}

		//variable d'etat
		if (_sauvegarde_tous_etat || (_sauvegarde_etat && _date_sauvegarde_etat - pas_de_temps == date_courante))
			SauvegardeEtat(date_courante);

		TEMPSOL::Calcule();
	}

	
	void THORSEN::Termine()
	{		
		TEMPSOL::Termine();
	}


	void THORSEN::LectureParametres()
	{
		istringstream iss;
		string cle, valeur, ligne, nom;
		size_t index;
		int iLigne;
		
		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();

		if(_sim_hyd.PrendreNomTempSol() == PrendreNomSousModele())	//si le modele est simulé
		{
			if(_sim_hyd._fichierParametreGlobal)
			{
				LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
				return;
			}

			ifstream fichier( PrendreNomFichierParametres() );
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THORSEN");

			//version
			lire_cle_valeur(fichier, cle, valeur);
			if (cle != "PARAMETRES HYDROTEL VERSION")
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES THORSEN", 1);
			}

			//nom du sous modele
			getline_mod(fichier, ligne);	//ligne vide
			lire_cle_valeur(fichier, cle, valeur);
			getline_mod(fichier, ligne);	//ligne vide

			lire_cle_valeur(fichier, cle, valeur);
			iss.clear();
			iss.str(valeur);
			iss >> _fProfondeurInitialeGel;
		
			lire_cle_valeur(fichier, cle, valeur);	
			iss.clear();
			iss.str(valeur);
			iss >> _fParamEmpirique1;

			lire_cle_valeur(fichier, cle, valeur);	
			iss.clear();
			iss.str(valeur);
			iss >> _fTempGelEau;

			lire_cle_valeur(fichier, cle, valeur);	
			iss.clear();
			iss.str(valeur);
			iss >> _fTeneurEau;
		
			//parametres pour les differents type de sol
			getline_mod(fichier, ligne);	//ligne vide
			getline_mod(fichier, ligne);	//entête

			iLigne = 13;
			if(propriete_hydroliques._bDisponible)
			{
				for (index = 0; index < propriete_hydroliques.PrendreNb(); ++index)
				{
					string nomTypeSol;
					float KT;

					lire_cle_valeur(fichier, nomTypeSol, valeur);
					auto vValeur = extrait_fvaleur(valeur, ";");

					if(vValeur.size() != 1)
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), iLigne, "Nombre de colonne invalide.");

					KT = vValeur[0];

					nom = propriete_hydroliques.Prendre(index).PrendreNom();
					if(nom != nomTypeSol)
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), iLigne, "Type de sol invalide; le type de sol doit respecter l'ordre du fichier contenant les proprietes hydroliques des sols.");
					}

					_vfParamKT[index] = KT;
					++iLigne;
				}
			}
			else
			{
				lire_cle_valeur(fichier, cle, valeur);
				auto vValeur = extrait_fvaleur(valeur, ";");

				if(vValeur.size() != 1)
					throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), iLigne, "Nombre de colonne invalide.");

				_vfParamKT[0] = vValeur[0];
			}

			fichier.close();
		}
	}


	void THORSEN::LectureParametresFichierGlobal()
	{
		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL;" + _sim_hyd._nomFichierParametresGlobal );

		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();
		bool bOK = false;

		try{

		vector<float> vValeur;
		string cle, valeur, ligne;
		lire_cle_valeur(fichier, cle, valeur);

		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, 1);

		float fVal;
		int no_ligne = 2;

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "THORSEN")
			{
				++no_ligne;
				getline_mod(fichier, ligne);
				vValeur = extrait_fvaleur(ligne, ";");

				if(vValeur.size() != 4)
					throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide THORSEN.");

				_fProfondeurInitialeGel = vValeur[0];
				_fParamEmpirique1 = vValeur[1];
				_fTempGelEau = vValeur[2];
				_fTeneurEau = vValeur[3];

				if(propriete_hydroliques._bDisponible)
				{
					for (size_t index = 0; index < propriete_hydroliques.PrendreNb(); ++index)
					{
						++no_ligne;
						getline_mod(fichier, ligne);
						vValeur = extrait_fvaleur(ligne, ";");

						if(vValeur.size() != 2)
							throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "THORSEN; Nombre de colonne invalide.");

						fVal = static_cast<float>(index);
						if(fVal != vValeur[0])
							throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "THORSEN; ID type sol invalide. Les ID type sol doivent etre en ordre croissant.");

						_vfParamKT[index] = vValeur[1];
					}
				}
				else
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 2)
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "THORSEN; Nombre de colonne invalide.");

					_vfParamKT[0] = vValeur[1];
				}

				bOK = true;
				break;
			}

			++no_ligne;
		}

		}
		catch(const ERREUR_LECTURE_FICHIER& ex)
		{
			fichier.close();
			throw ex;
		}
		catch(...)
		{
			fichier.close();
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; THORSEN; " + _sim_hyd._nomFichierParametresGlobal );
		}

		fichier.close();

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele THORSEN");
	}


	void THORSEN::SauvegardeParametres()
	{
		if(PrendreNomFichierParametres() == "")	//creation d'un fichier par defaut s'il n'existe pas
			ChangeNomFichierParametres(Combine(_sim_hyd.PrendreRepertoireSimulation(), "thorsen.csv"));

		string nom;
		string nom_fichier = PrendreNomFichierParametres();

		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();

		ofstream fichier(nom_fichier);

		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
		fichier << endl;

		fichier << "SOUS MODELE;" << PrendreNomSousModele() << endl;
		fichier << endl;

		fichier << "PROFONDEUR INITIALE DU GEL DANS LE SOL (m);" << setprecision(2) << setiosflags(ios::fixed) << _fProfondeurInitialeGel << endl;
		fichier << "PARAMETRE EMPIRIQUE 1 (m-1);" << setprecision(2) << setiosflags(ios::fixed) << _fParamEmpirique1 << endl;
		fichier << "TEMPERATURE DU GEL DE L'EAU DANS LE SOL (dC);" << setprecision(2) << setiosflags(ios::fixed) << _fTempGelEau << endl;
		fichier << "TENEUR EN EAU DISPONIBLE (INITIAL/PAR DEFAUT) (0:1);" << setprecision(2) << setiosflags(ios::fixed) << _fTeneurEau << endl;
		fichier << endl;

		fichier << "TEXTURE SOL; CONDUCTIVITE THERMIQUE D'UN SOL GELE (KT) (W/m/s)" << endl;

		if(propriete_hydroliques._bDisponible)
		{
			for (size_t index = 0; index < propriete_hydroliques.PrendreNb(); ++index)
			{
				nom = propriete_hydroliques.Prendre(index).PrendreNom();
				fichier << nom << ';' << setprecision(4) << setiosflags(ios::fixed) << _vfParamKT[index] << endl;
			}
		}
		else
			fichier << "valeurs par defaut" << ';' << setprecision(4) << setiosflags(ios::fixed) << _vfParamKT[0] << endl;	//valeur utilisé par defaut

		fichier.close();
	}

	string THORSEN::PrendreNomFichierLectureEtat() const
	{
		return _nom_fichier_lecture_etat;
	}

	string THORSEN::PrendreRepertoireEcritureEtat() const
	{
		return _repertoire_ecriture_etat;
	}

	bool THORSEN::PrendreSauvegardeTousEtat() const
	{
		return _sauvegarde_tous_etat;
	}

	DATE_HEURE THORSEN::PrendreDateHeureSauvegardeEtat() const
	{
		return _date_sauvegarde_etat;
	}

	void THORSEN::ChangeNomFichierLectureEtat(std::string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}

	void THORSEN::ChangeRepertoireEcritureEtat(std::string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}

	void THORSEN::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}

	void THORSEN::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}

	void THORSEN::LectureEtat(DATE_HEURE date_courante)
	{
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("TEMPERATURE DU SOL; fichier etat THORSEN; " + _nom_fichier_lecture_etat);

		vector<int> vValidation;
		string ligne;
		size_t index_zone;
		int iIdent;

		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);
		getline_mod(fichier, ligne);

		ZONES& zones = _sim_hyd.PrendreZones();

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

					zones[index_zone].ChangeProfondeurGel(valeurs[1]); //cm

					vValidation.push_back(iIdent);
				}
			}
		}

		fichier.close();

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("TEMPERATURE DU SOL; fichier etat THORSEN; " + _nom_fichier_lecture_etat);
	}


	void THORSEN::SauvegardeEtat(DATE_HEURE date_courante) const
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier, oss;
		string sSep = _sim_hyd._output._sFichiersEtatsSeparator;

		if(!RepertoireExiste(_repertoire_ecriture_etat))
			CreeRepertoire(_repertoire_ecriture_etat);

		nom_fichier << _repertoire_ecriture_etat;
		if(_repertoire_ecriture_etat[_repertoire_ecriture_etat.size()-1] != '/')
			nom_fichier << "/";

		nom_fichier << setfill('0') 
			        << "tempsol_" 
			        << setw(4) << date_courante.PrendreAnnee() 
			        << setw(2) << date_courante.PrendreMois() 
			        << setw(2) << date_courante.PrendreJour() 
			        << setw(2) << date_courante.PrendreHeure() 
					<< ".csv";

		ofstream fichier(nom_fichier.str());
		if (!fichier)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier.str());

		fichier.exceptions(ios::failbit | ios::badbit);

		fichier << "ETATS TEMPERATURE DU SOL" << sSep << PrendreNomSousModele() << "( " << HYDROTEL_VERSION << " )" << endl;
		fichier << "DATE_HEURE" << sSep << date_courante << endl;
		fichier << endl;

		fichier << "UHRH" << sSep << "Profondeur du gel (cm)" << endl;

		ZONES& zones = _sim_hyd.PrendreZones();
		
		for (size_t index=0; index<zones.PrendreNbZone(); index++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
			{
				ZONE& zone = zones[index];

				oss.str("");
				oss << zone.PrendreIdent() << sSep;

				oss << setprecision(12) << setiosflags(ios::fixed);

				oss << zones[index].PrendreProfondeurGel();

				fichier << oss.str() << endl;
			}
		}

		fichier.close();
	}

}
