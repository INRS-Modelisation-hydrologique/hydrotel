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

#include "rankinen.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "version.hpp"

#include <cmath>
#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	RANKINEN::RANKINEN(SIM_HYD& sim_hyd)
		: TEMPSOL(sim_hyd, "RANKINEN")
	{
		_fIntervalleProfil = 0.05f;
		_fTempIniProfondeur = 4.0f;
		_fSeuilGel = -0.5f;
		_fParamFS = 2.35f;

		_sauvegarde_etat = false;
		_sauvegarde_tous_etat = false;
	}


	RANKINEN::~RANKINEN()
	{
	}


	void RANKINEN::ChangeNbParams(const ZONES& zones)
	{
		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();
		size_t nb;

		if(propriete_hydroliques._bDisponible)
			nb = propriete_hydroliques.PrendreNb();
		else
			nb = 1;

		_vfParamKT.resize(nb, 0.8f);
		_vfParamCS.resize(nb, 1000000.0f);
		_vfParamCIce.resize(nb, 4000000.0f);

		TEMPSOL::ChangeNbParams(zones);
	}


	void RANKINEN::Initialise()
	{
		size_t index, idx, index_zone;
		int iNbIntervalle;
		float fProfondeur, fTemp, z3Temp;

		ZONES& zones = _sim_hyd.PrendreZones();
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();

		//validation intervalle du profil
		fProfondeur = 1000000.0;
		for (index = 0; index < zones.PrendreNbZone(); ++index)	//determine la profondeur minimum possible pour la couche 3
		{
			z3Temp = zones[index].PrendreZ11() + zones[index].PrendreZ22() + zones[index].PrendreZ33();
			if(z3Temp < fProfondeur)
				fProfondeur = z3Temp;
		}

		if(_fIntervalleProfil >= fProfondeur)
			throw ERREUR("TEMPERATURE DU SOL:RANKINEN; The interval for the profile should be less than the minimum depth possible for soil layer 3.");

		for (index = 0; index < index_zones.size(); ++index)
		{
			index_zone = index_zones[index];

			//determine le nb d'intervalle pour l'UHRH
			fProfondeur = zones[index_zone].PrendreZ11() + zones[index_zone].PrendreZ22() + zones[index_zone].PrendreZ33();	//m
			iNbIntervalle = static_cast<int>(fProfondeur / _fIntervalleProfil);

			fTemp = fProfondeur / _fIntervalleProfil;
			fTemp-= iNbIntervalle;
			if(fTemp > 0.0)
				iNbIntervalle+= 2;
			else
				iNbIntervalle+= 1;

			_mapTemperature[index_zone].resize(iNbIntervalle, -1.0);
		}

		OUTPUT& output = _sim_hyd.PrendreOutput();

		ostringstream oss;
		for(index=0; index < _vOutTempUHRH.size(); index++)
		{
			oss.str("");
			oss << "tempsol_uhrh" << _vOutTempUHRH[index] << ".csv";
			
			string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), oss.str()) );
			std::ofstream* pFile;
			pFile = new std::ofstream;
			_vOutTempFile.push_back(pFile);
			_vOutTempFile[index]->open(nom_fichier);

			//entete
			(*_vOutTempFile[index]) << "UHRH " << _vOutTempUHRH[index] << " Temperature du sol (C)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl;
			
			oss.str("");
			oss << "date heure\\profondeur (cm)" << output.Separator() << setprecision(2) << setiosflags(ios::fixed);

			index_zone = zones.IdentVersIndex(static_cast<int>(_vOutTempUHRH[index]));
			
			for(idx=0; idx < _mapTemperature[index_zone].size(); idx++)
				oss << (idx * _fIntervalleProfil) << output.Separator();

			nom_fichier = oss.str();
			nom_fichier = nom_fichier.substr(0, nom_fichier.length()-1);	//enleve le dernier separateur
			(*_vOutTempFile[index]) << nom_fichier << endl;
		}

		if (!_nom_fichier_lecture_etat.empty())
			LectureEtat( _sim_hyd.PrendreDateDebut() );

		TEMPSOL::Initialise();
	}

	void RANKINEN::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		vector<size_t> index_zones = _sim_hyd.PrendreZonesSimules();
		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();
		string str;
		size_t idx;

		size_t index_zone, index, index2, indexTypeSol;
		float fTAir, fM, fB, fProfondeur, fTemp, fT, fDs;

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();

		if(_nom_fichier_lecture_etat.empty() && date_courante == _sim_hyd.PrendreDateDebut())
		{
			//si le fichier d'état n'est pas utilisé, et que c'est le premier pas de temps, on initialise les profils 
			//de température une 1ere fois a partir de la temperature initiale a la base du profil et de la temperature de l'air 
			//a l'aide de l'equation lineaire y = mx + b.

			for (index = 0; index < index_zones.size(); ++index)
			{
				index_zone = index_zones[index];

				fTAir = (zones[index_zone].PrendreTMax() + zones[index_zone].PrendreTMin()) / 2.0f;	//temperature de l'air
				fProfondeur = (_mapTemperature[index_zone].size() - 1) * _fIntervalleProfil;		//profondeur du profil

				fDs = zones[index_zone].PrendreHauteurCouvertNival(); //m
				if(fDs == -999.0)
					throw ERREUR("Error TEMPSOL:Calcule: invalid snow cover value");

				if(fDs != 0.0f)
					fB = fTAir * exp(-_fParamFS * fDs);
				else
					fB = fTAir;

				fM = (_fTempIniProfondeur - fB) / fProfondeur;

				for (index2 = 0; index2 < _mapTemperature[index_zone].size(); ++index2)
				{
					fProfondeur = index2 * _fIntervalleProfil;
					fTemp = fM * fProfondeur + fB;

					_mapTemperature[index_zone].at(index2) = fTemp;
				}
			}
		}

		float fCA, fZs, fTZs, fDeltaT, fProfondeurGel, fB1, fTb, fTb1;
		int iIndexIntervalleSuperieurGel;

		fDeltaT = pas_de_temps * 60.0f * 60.0f;
		indexTypeSol = 0;
		for (index = 0; index < index_zones.size(); ++index)
		{
			//calcul du profil de temperature
			index_zone = index_zones[index];

			fTAir = (zones[index_zone].PrendreTMax() + zones[index_zone].PrendreTMin()) / 2.0f;
			fDs = zones[index_zone].PrendreHauteurCouvertNival(); //m
			iIndexIntervalleSuperieurGel = -1;

			if(fDs == -999.0)
				throw ERREUR("Error TEMPSOL:Calcule: reading snow cover");

			//pour le premier interval, a la profondeur 0, on calcule l'equation 2
			if(fDs != 0.0)
				_mapTemperature[index_zone].at(0) = fTAir * exp(-_fParamFS * fDs);
			else
				_mapTemperature[index_zone].at(0) = fTAir;

			if(_mapTemperature[index_zone].at(0) <= _fSeuilGel)
				iIndexIntervalleSuperieurGel = 0;

			//
			for (index2 = 1; index2 < _mapTemperature[index_zone].size(); ++index2)
			{
				fTZs = _mapTemperature[index_zone].at(index2);
				fZs = index2 * _fIntervalleProfil;	//m

				if(propriete_hydroliques._bDisponible)
				{
					if(fZs > zones[index_zone].PrendreZ11())
					{
						if(fZs > zones[index_zone].PrendreZ11() + zones[index_zone].PrendreZ22())
							indexTypeSol = propriete_hydroliques.PrendreIndexCouche3(index_zone);	//type de sol dominant couche 3
						else
							indexTypeSol = propriete_hydroliques.PrendreIndexCouche2(index_zone);	//type de sol dominant couche 2
					}
					else
						indexTypeSol = propriete_hydroliques.PrendreIndexCouche1(index_zone);	//type de sol dominant couche 1
				}

				//equation 1
				fCA = _vfParamCS[indexTypeSol] + _vfParamCIce[indexTypeSol];
				fT = fTZs + fDeltaT * _vfParamKT[indexTypeSol] / (fCA * pow(2.0f * fZs, 2.0f)) * (fTAir - fTZs);

				//equation2
				_mapTemperature[index_zone].at(index2) = fT * exp(-_fParamFS * fDs);

				//conserve l'index de l'intervalle superieur au gel
				if(_mapTemperature[index_zone].at(index2) <= _fSeuilGel)
					iIndexIntervalleSuperieurGel = static_cast<int>(index2);
			}

			//calcul de la profondeur du gel
			fProfondeurGel = 0.0f;

			if(iIndexIntervalleSuperieurGel != -1)
			{
				fB = iIndexIntervalleSuperieurGel * _fIntervalleProfil;

				if(iIndexIntervalleSuperieurGel == static_cast<int>(_mapTemperature[index_zone].size()) - 1)	//si cest le dernier intervalle
					fProfondeurGel = fB;
				else
				{
					fB1 = (iIndexIntervalleSuperieurGel + 1) * _fIntervalleProfil;
					fTb = _mapTemperature[index_zone].at(static_cast<size_t>(iIndexIntervalleSuperieurGel));
					fTb1 = _mapTemperature[index_zone].at(static_cast<size_t>(iIndexIntervalleSuperieurGel + 1));

					fProfondeurGel = (_fSeuilGel - fTb) * (fB1 - fB) / (fTb1 - fTb) + fB;
				}
			}

			zones[index_zone].ChangeProfondeurGel(fProfondeurGel * 100.0f);	//m -> cm
		}

		//fichier output
		OUTPUT& output = _sim_hyd.PrendreOutput();

		for(index=0; index < _vOutTempUHRH.size(); index++)
		{
			ostringstream oss;
			oss.str("");
			
			oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_dC) << setiosflags(ios::fixed);

			index_zone = zones.IdentVersIndex(static_cast<int>(_vOutTempUHRH[index]));

			for(idx=0; idx < _mapTemperature[index_zone].size(); idx++)
				oss << _mapTemperature[index_zone][idx] << output.Separator();

			str = oss.str();
			str = str.substr(0, str.length()-1);	//enleve le dernier separateur

			(*_vOutTempFile[index]) << str << endl;
		}

		//variable d'etat
		if (_sauvegarde_tous_etat || (_sauvegarde_etat && _date_sauvegarde_etat - pas_de_temps == date_courante))
			SauvegardeEtat(date_courante);

		TEMPSOL::Calcule();
	}

	void RANKINEN::Termine()
	{
		for(size_t idx=0; idx < _vOutTempFile.size(); idx++)
			(*_vOutTempFile[idx]).close();
		
		TEMPSOL::Termine();
	}

	void RANKINEN::LectureParametres()
	{
		istringstream iss;
		string cle, valeur, ligne, nom;
		size_t index;
		int iLigne;
		
		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();

		if(_sim_hyd.PrendreNomTempSol() == PrendreNomSousModele())	//si le modele RANKINEN est simulé
		{
			if(_sim_hyd._fichierParametreGlobal)
			{
				LectureParametresFichierGlobal();	//lecture du fichier de parametre global si l'option est activé
				return;
			}

			ifstream fichier( PrendreNomFichierParametres() );
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES RANKINEN");

			//version
			lire_cle_valeur(fichier, cle, valeur);
			if (cle != "PARAMETRES HYDROTEL VERSION")
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER("FICHIER PARAMETRES RANKINEN", 1);
			}

			//nom du sous modele
			getline_mod(fichier, ligne);	//ligne vide
			lire_cle_valeur(fichier, cle, valeur);
			getline_mod(fichier, ligne);	//ligne vide

			//fichiers de sortie des temperatures; liste des uhrh sauvegarde
			_vOutTempUHRH.clear();
			lire_cle_valeur(fichier, cle, valeur);
			_vOutTempUHRH = extrait_svaleur(valeur, ";");
			getline_mod(fichier, ligne);	//ligne vide

			//intervalle du profil
			lire_cle_valeur(fichier, cle, valeur);
			iss.clear();
			iss.str(valeur);
			iss >> _fIntervalleProfil;

			//temperature initiale au fond du profil
			lire_cle_valeur(fichier, cle, valeur);
			iss.clear();
			iss.str(valeur);
			iss >> _fTempIniProfondeur;
		
			//seuil du gel
			lire_cle_valeur(fichier, cle, valeur);	
			iss.clear();
			iss.str(valeur);
			iss >> _fSeuilGel;

			//parametre FS
			lire_cle_valeur(fichier, cle, valeur);	
			iss.clear();
			iss.str(valeur);
			iss >> _fParamFS;
		
			//validation intervalle du profil
			if(_fIntervalleProfil < 0.01)
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), 5, "L'intervalle pour le profil doit etre superieur ou egal a 0.01 m.");
			}
		
			//parametres pour les differents type de sol
			getline_mod(fichier, ligne);	//ligne vide
			getline_mod(fichier, ligne);	//entête

			iLigne = 13;

			if(propriete_hydroliques._bDisponible)
			{
				for (index = 0; index < propriete_hydroliques.PrendreNb(); ++index)
				{
					string nomTypeSol;
					float KT, CS, CIce;

					lire_cle_valeur(fichier, nomTypeSol, valeur);
					auto vValeur = extrait_fvaleur(valeur, ";");

					if(vValeur.size() != 3)
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), iLigne, "Nombre de colonne invalide.");

					KT = vValeur[0];
					CS = vValeur[1];
					CIce = vValeur[2];

					nom = propriete_hydroliques.Prendre(index).PrendreNom();
					if(nom != nomTypeSol)
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), iLigne, "Type de sol invalide; le type de sol doit respecter l'ordre du fichier contenant les proprietes hydroliques des sols.");
					}

					_vfParamKT[index] = KT;
					_vfParamCS[index] = CS;
					_vfParamCIce[index] = CIce;

					++iLigne;
				}
			}
			else
			{
				lire_cle_valeur(fichier, cle, valeur);
				auto vValeur = extrait_fvaleur(valeur, ";");

				if(vValeur.size() != 3)
					throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), iLigne, "Nombre de colonne invalide.");

				_vfParamKT[0] = vValeur[0];
				_vfParamCS[0] = vValeur[1];
				_vfParamCIce[0] = vValeur[2];
			}

			fichier.close();
		}
	}


	void RANKINEN::LectureParametresFichierGlobal()
	{
		ifstream fichier( _sim_hyd._nomFichierParametresGlobal );
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL;" + _sim_hyd._nomFichierParametresGlobal );

		PROPRIETE_HYDROLIQUES& propriete_hydroliques = _sim_hyd.PrendreProprieteHydrotliques();
		bool bOK = false;

		try{

		vector<float> vValeur;
		string cle, valeur, ligne;
		float fVal;
		size_t index;

		lire_cle_valeur(fichier, cle, valeur);
		if (cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
			throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, 1);

		int no_ligne = 2;

		while (!fichier.eof())
		{
			getline_mod(fichier, ligne);
			if(ligne == "RANKINEN")
			{
				++no_ligne;
				getline_mod(fichier, ligne);
				vValeur = extrait_fvaleur(ligne, ";");

				if(vValeur.size() != 4)
					throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "Nombre de colonne invalide RANKINEN.");

				_fIntervalleProfil = vValeur[0];
				_fTempIniProfondeur = vValeur[1];
				_fSeuilGel = vValeur[2];
				_fParamFS = vValeur[3];

				//validation intervalle du profil
				if(_fIntervalleProfil < 0.01)
					throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "RANKINEN; L'intervalle pour le profil doit etre superieur ou egal a 0.01 m.");

				if(propriete_hydroliques._bDisponible)
				{
					for (index = 0; index < propriete_hydroliques.PrendreNb(); ++index)
					{
						++no_ligne;
						getline_mod(fichier, ligne);
						vValeur = extrait_fvaleur(ligne, ";");

						if(vValeur.size() != 4)
							throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "RANKINEN; Nombre de colonne invalide.");

						fVal = static_cast<float>(index);
						if(fVal != vValeur[0])
							throw ERREUR_LECTURE_FICHIER( _sim_hyd._nomFichierParametresGlobal, no_ligne, "RANKINEN; ID type sol invalide. Les ID type sol doivent etre en ordre croissant.");

						_vfParamKT[index] = vValeur[1];
						_vfParamCS[index] = vValeur[2];
						_vfParamCIce[index] = vValeur[3];
					}
				}
				else
				{
					++no_ligne;
					getline_mod(fichier, ligne);
					vValeur = extrait_fvaleur(ligne, ";");

					if(vValeur.size() != 4)
						throw ERREUR_LECTURE_FICHIER( PrendreNomFichierParametres(), no_ligne, "RANKINEN; Nombre de colonne invalide.");

					_vfParamKT[0] = vValeur[1];
					_vfParamCS[0] = vValeur[2];
					_vfParamCIce[0] = vValeur[3];
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
			throw ERREUR_LECTURE_FICHIER( "FICHIER PARAMETRES GLOBAL; RANKINEN; " + _sim_hyd._nomFichierParametresGlobal );
		}

		if(!bOK)
			throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 0, "Parametres sous-modele RANKINEN");
	}


	void RANKINEN::SauvegardeParametres()
	{
		if(PrendreNomFichierParametres() == "")	//creation d'un fichier par defaut s'il n'existe pas
			ChangeNomFichierParametres(Combine(_sim_hyd.PrendreRepertoireSimulation(), "rankinen.csv"));
		
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

		fichier << "OUTPUT_TEMPERATURE_LIST_UHRH";
		for (size_t index = 0; index < _vOutTempUHRH.size(); ++index)
			fichier << ";" << _vOutTempUHRH[index];
		
		if(_vOutTempUHRH.size() == 0)
			fichier << ";";
		fichier << endl << endl;

		fichier << "INTERVALLE PROFIL (m);" << setprecision(2) << setiosflags(ios::fixed) << _fIntervalleProfil << endl;
		fichier << "TEMP INI BASE PROFIL (C);" << setprecision(1) << setiosflags(ios::fixed) << _fTempIniProfondeur << endl;
		fichier << "SEUIL GEL (C);" << setprecision(1) << setiosflags(ios::fixed) << _fSeuilGel << endl;
		fichier << "FS;" << setprecision(2) << setiosflags(ios::fixed) << _fParamFS << endl;
		fichier << endl;

		fichier << "TEXTURE SOL; Conductivite thermique du sol gele (KT) (W/m/C); Capacite thermique specifique du sol (CS) (J/m3/C); Capacite thermique specifique lie au gel/degel (CIce) (J/m3/C)" << endl;

		if(propriete_hydroliques._bDisponible)
		{
			for (size_t index = 0; index < propriete_hydroliques.PrendreNb(); ++index)
			{
				nom = propriete_hydroliques.Prendre(index).PrendreNom();

				fichier << nom << ';' << setprecision(4) << setiosflags(ios::fixed) << _vfParamKT[index] << ';' 
						<< setprecision(0) << setiosflags(ios::fixed) << _vfParamCS[index] << ';' << _vfParamCIce[index] << endl;
			}
		}
		else
		{
			fichier << "valeurs par defaut" << ';' << setprecision(4) << setiosflags(ios::fixed) << _vfParamKT[0] << ';' 
					<< setprecision(0) << setiosflags(ios::fixed) << _vfParamCS[0] << ';' << _vfParamCIce[0] << endl;
		}
		
		fichier.close();
	}

	string RANKINEN::PrendreNomFichierLectureEtat() const
	{
		return _nom_fichier_lecture_etat;
	}

	string RANKINEN::PrendreRepertoireEcritureEtat() const
	{
		return _repertoire_ecriture_etat;
	}

	bool RANKINEN::PrendreSauvegardeTousEtat() const
	{
		return _sauvegarde_tous_etat;
	}

	DATE_HEURE RANKINEN::PrendreDateHeureSauvegardeEtat() const
	{
		return _date_sauvegarde_etat;
	}

	void RANKINEN::ChangeNomFichierLectureEtat(std::string nom_fichier)
	{
		_nom_fichier_lecture_etat = nom_fichier;
	}

	void RANKINEN::ChangeRepertoireEcritureEtat(std::string repertoire)
	{
		_repertoire_ecriture_etat = repertoire;
	}

	void RANKINEN::ChangeSauvegardeTousEtat(bool sauvegarde_tous)
	{
		_sauvegarde_tous_etat = sauvegarde_tous;
	}

	void RANKINEN::ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde)
	{
		// NOTE: il n'y a pas de validation sur cette date, elle pourrait etre hors de la simulation
		_date_sauvegarde_etat = date_sauvegarde;
		_sauvegarde_etat = sauvegarde;
	}

	
	void RANKINEN::LectureEtat(DATE_HEURE date_courante)
	{
		ifstream fichier(_nom_fichier_lecture_etat);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("TEMPERATURE DU SOL; fichier etat RANKINEN; " + _nom_fichier_lecture_etat);

		vector<int> vValidation;
		string ligne;
		size_t index_zone, index2;
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
				if(valeurs.size() < 3)	//3 colonnes minimum... (id uhrh + 2 intervalles minimum)
				{
					fichier.close();
					throw ERREUR_LECTURE_FICHIER("TEMPERATURE DU SOL; fichier etat RANKINEN; " + _nom_fichier_lecture_etat + "; nombre de valeur invalide");
				}

				iIdent = static_cast<int>(valeurs[0]);
				if(find(begin(_sim_hyd.PrendreZonesSimulesIdent()), end(_sim_hyd.PrendreZonesSimulesIdent()), iIdent) != end(_sim_hyd.PrendreZonesSimulesIdent()))
				{
					index_zone = _sim_hyd.PrendreZones().IdentVersIndex(iIdent);

					//validation
					while(valeurs[valeurs.size()-1] == -999.0)	//number of columns is egal to the greatest possible number of temp. profile	//number of profile can change from one uhrh group to the other because of soil layer thickness (bv3c)
						valeurs.pop_back();

					if(valeurs.size()-1 != _mapTemperature[index_zone].size())
					{
						fichier.close();
						ostringstream oss;
						oss.str("");
						oss << "TEMPERATURE DU SOL; fichier etat RANKINEN; " + _nom_fichier_lecture_etat + "; nombre de valeur invalide; selon les parametres actuels, il devrait y avoir "  << _mapTemperature[index_zone].size() << " profils de temperature.";
						throw ERREUR_LECTURE_FICHIER(oss.str());
					}
					//

					for(index2=0; index2<_mapTemperature[index_zone].size(); index2++)
						_mapTemperature[index_zone].at(index2) = valeurs[index2+1];

					vValidation.push_back(iIdent);
				}
			}
		}

		fichier.close();

		std::sort(vValidation.begin(), vValidation.end());
		if(!equal(vValidation.begin(), vValidation.end(), _sim_hyd.PrendreZonesSimulesIdent().begin()))
			throw ERREUR_LECTURE_FICHIER("TEMPERATURE DU SOL; fichier etat RANKINEN; id mismatch; " + _nom_fichier_lecture_etat);
	}


	void RANKINEN::SauvegardeEtat(DATE_HEURE date_courante) const
	{
		BOOST_ASSERT(_repertoire_ecriture_etat.size() != 0);

		date_courante.AdditionHeure( _sim_hyd.PrendrePasDeTemps() );

		ostringstream nom_fichier, oss;
		string sSep = _sim_hyd._output._sFichiersEtatsSeparator;
		size_t index2, nbIntervalleMax;

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

		//le nb de couche de sol (intervalle) peut varier d'un uhrh a l'autre; la profondeur des couches de sol de bv3c peut varier selon le groupe du uhrh
		//obtient le nb d'intervalle maximum
		ZONES& zones = _sim_hyd.PrendreZones();

		nbIntervalleMax = 0;
		for (index2=0; index2<zones.PrendreNbZone(); index2++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index2) != end(_sim_hyd.PrendreZonesSimules()))
			{
				if(_mapTemperature.at(index2).size() > nbIntervalleMax)
					nbIntervalleMax = _mapTemperature.at(index2).size();
			}
		}

		fichier << "UHRH" << sSep;
		for (index2=0; index2<nbIntervalleMax-1; index2++)
			fichier << "Temp. profil " << index2+1 << " (dC)" << sSep;
		fichier << "Temp. profil " << nbIntervalleMax << " (dC)";
		fichier << endl;

		for (size_t index=0; index<zones.PrendreNbZone(); index++)
		{
			if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
			{
				oss.str("");
				oss << zones[index].PrendreIdent() << sSep;

				oss << setprecision(12) << setiosflags(ios::fixed);
				
				oss << _mapTemperature.at(index)[0];
				for (index2 = 1; index2 < _mapTemperature.at(index).size(); index2++)
					oss << sSep << _mapTemperature.at(index)[index2];

				while(index2<nbIntervalleMax)
				{
					oss << sSep << -999.0f;
					++index2;
				}

				fichier << oss.str() << endl;
			}
		}

		fichier.close();
	}

}
