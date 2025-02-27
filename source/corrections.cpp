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

#include "corrections.hpp"

#include "erreur.hpp"
#include "sim_hyd.hpp"
#include "util.hpp"

#include <fstream>


using namespace std;


namespace HYDROTEL
{

	CORRECTIONS::CORRECTIONS()
	{
		_bActiver = true;
	}

	CORRECTIONS::~CORRECTIONS()
	{
	}

	void CORRECTIONS::ChangeNomFichier(const string& nom_fichier)
	{
		_nom_fichier = nom_fichier;
	}

	void CORRECTIONS::LectureFichier(SIM_HYD& sim_hyd)
	{
		if (!_bActiver || _nom_fichier.empty())
			return;

		vector<CORRECTION> corrections;

		ifstream fichier(_nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER("FICHIER CORRECTIONS");

		string line, tmp;
		bool bOK = false;
		int x, no_ligne = 0;

		try{
		
		//obtient le no de ligne de la 1ere ligne de donnees
		while(!bOK && !fichier.eof())
		{
			getline_mod(fichier, line);
			++no_ligne;

			tmp = "";
			if(line.size() > 1)
				tmp = line.substr(0, 2);
			if(tmp == "0;" || tmp == "1;")
				bOK = true;
		}

		fichier.close();
		fichier.clear();

		if(bOK)
		{
			//lecture des donnees
			fichier.open(_nom_fichier);
			if (!fichier)
				throw ERREUR_LECTURE_FICHIER("FICHIER CORRECTIONS");

			fichier.exceptions(ios::failbit | ios::badbit);

			for(x=1; x<no_ligne; x++)
				getline_mod(fichier, line);	//comment

			while (!fichier.eof())
			{
                DATE_HEURE date_debut;
                DATE_HEURE date_fin;

				int actif, variable, groupe;
				float additif, multiplicatif, coeff1, coeff2, coeff3;
				string debut, fin, nom_groupe;
				char separateur;
                unsigned short annee, mois, jour, heure;

				additif = multiplicatif = coeff1 = coeff2 = coeff3 = -1.0;

                fichier >> actif;
				
				fichier >> separateur;
				fichier >> annee;
				fichier >> separateur;
                fichier >> mois;
                fichier >> separateur;
                fichier >> jour;
                fichier >> heure;                
				date_debut = DATE_HEURE(annee, mois, jour, heure);

                fichier >> separateur;
				fichier >> annee;
				fichier >> separateur;
                fichier >> mois;
                fichier >> separateur;
                fichier >> jour;
                fichier >> heure;                
				date_fin = DATE_HEURE(annee, mois, jour, heure);

                if (date_debut > date_fin)
				{
					fichier.close();
					throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);
				}

				fichier >> separateur;
				fichier >> variable;

				if(variable == 6)	//saturation reserve sol
				{
					fichier >> separateur;
					fichier >> coeff1;
					fichier >> separateur;
					fichier >> coeff2;
					fichier >> separateur;
					fichier >> coeff3;
					fichier >> separateur;
					fichier >> groupe;
					fichier >> separateur;
					fichier >> nom_groupe;
				}
				else
				{
					fichier >> separateur;
					fichier >> additif;
					fichier >> separateur;
					fichier >> multiplicatif;
					fichier >> separateur;
					fichier >> groupe;
					fichier >> separateur;
					fichier >> nom_groupe;
				}

				TYPE_GROUPE type_groupe;
				TYPE_CORRECTION type_correction;

				switch (groupe)
				{
				case TYPE_GROUPE_ALL:
					type_groupe = TYPE_GROUPE_ALL;
					break;

				case TYPE_GROUPE_HYDRO:
					if (sim_hyd.RechercheGroupeZone(nom_groupe) == nullptr)
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);
					}
					type_groupe = TYPE_GROUPE_HYDRO;
					break;

				case TYPE_GROUPE_CORRECTION:
					if (sim_hyd.RechercheGroupeCorrection(nom_groupe) == nullptr)
					{
						fichier.close();
						throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);
					}
					type_groupe = TYPE_GROUPE_CORRECTION;
					break;

				default:
					fichier.close();
					throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);
				}

				switch (variable)
				{
				case TYPE_CORRECTION_TEMPERATURE:
					type_correction = TYPE_CORRECTION_TEMPERATURE;
					break;
				case TYPE_CORRECTION_PLUIE:
					type_correction = TYPE_CORRECTION_PLUIE;
					break;
				case TYPE_CORRECTION_NEIGE:
					type_correction = TYPE_CORRECTION_NEIGE;
					break;
				case TYPE_CORRECTION_RESERVE_SOL:
					type_correction = TYPE_CORRECTION_RESERVE_SOL;
					break;
				case TYPE_CORRECTION_NEIGE_AU_SOL:
					type_correction = TYPE_CORRECTION_NEIGE_AU_SOL;
					break;
				case TYPE_CORRECTION_SATURATION:
					type_correction = TYPE_CORRECTION_SATURATION;
					break;
				default:
					fichier.close();
					throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);
				}

				CORRECTION correction(type_correction, type_groupe, nom_groupe);
				correction.ChangePeriode(actif, date_debut, date_fin);

				if(variable == 6)	//saturation reserve sol
					correction.ChangeCoefficientSaturation(coeff1, coeff2, coeff3);
				else
					correction.ChangeCoefficient(additif, multiplicatif);

				corrections.push_back(correction);
				no_ligne++;
			}

			fichier.close();
		}

		}
		catch (...)
		{
			if(!fichier.eof())
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);
			}
			else
				fichier.close();
		}

		corrections.shrink_to_fit();
		_corrections.swap(corrections);
	}

	vector<CORRECTION*> CORRECTIONS::PrendreCorrectionsPluie()
	{
		vector<CORRECTION*> l;

		for (auto iter = begin(_corrections); iter != end(_corrections); ++iter)
		{
			if (iter->PrendreVariable() == TYPE_CORRECTION_PLUIE)
				l.push_back(&(*iter));
		}

		return l;
	}

	vector<CORRECTION*> CORRECTIONS::PrendreCorrectionsNeige()
	{
		vector<CORRECTION*> l;

		for (auto iter = begin(_corrections); iter != end(_corrections); ++iter)
		{
			if (iter->PrendreVariable() == TYPE_CORRECTION_NEIGE)
				l.push_back(&(*iter));
		}

		return l;
	}

	vector<CORRECTION*> CORRECTIONS::PrendreCorrectionsTemperature()
	{
		vector<CORRECTION*> l;

		for (auto iter = begin(_corrections); iter != end(_corrections); ++iter)
		{
			if (iter->PrendreVariable() == TYPE_CORRECTION_TEMPERATURE)
				l.push_back(&(*iter));
		}

		return l;
	}

	vector<CORRECTION*> CORRECTIONS::PrendreCorrectionsReserveSol()
	{
		vector<CORRECTION*> l;

		for (auto iter = begin(_corrections); iter != end(_corrections); ++iter)
		{
			if (iter->PrendreVariable() == TYPE_CORRECTION_RESERVE_SOL)
				l.push_back(&(*iter));
		}

		return l;
	}

	vector<CORRECTION*> CORRECTIONS::PrendreCorrectionsNeigeAuSol()
	{
		vector<CORRECTION*> l;

		for (auto iter = begin(_corrections); iter != end(_corrections); ++iter)
		{
			if (iter->PrendreVariable() == TYPE_CORRECTION_NEIGE_AU_SOL)
				l.push_back(&(*iter));
		}

		return l;
	}

	vector<CORRECTION*> CORRECTIONS::PrendreCorrectionsSaturationReserveSol()
	{
		vector<CORRECTION*> l;

		for (auto iter = begin(_corrections); iter != end(_corrections); ++iter)
		{
			if (iter->PrendreVariable() == TYPE_CORRECTION_SATURATION)
				l.push_back(&(*iter));
		}

		return l;
	}

	string CORRECTIONS::PrendreNomFichier() const
	{
		return _nom_fichier;
	}

}
