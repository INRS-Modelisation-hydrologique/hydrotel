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

#include "station_neige_mef.hpp"

#include "erreur.hpp"
#include "util.hpp"

#include <fstream>


using namespace std;


namespace HYDROTEL
{

	STATION_NEIGE_MEF::STATION_NEIGE_MEF(const string& nom_fichier)
		: STATION_NEIGE(nom_fichier)
	{
	}


	STATION_NEIGE_MEF::~STATION_NEIGE_MEF()
	{
	}


	//-----------------------------------------------------------------------------------------------
	void STATION_NEIGE_MEF::LectureDonnees(const DATE_HEURE& date_debut, const DATE_HEURE& date_fin)
	{
		ifstream fichier(_nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		fichier.exceptions(ios::failbit | ios::badbit);

		vector<string> sList;
		ostringstream oss;
		string ligne;
		short annee, mois, jour;
		float epaisseur, equivalent, densite;
		bool bFormatMEF;
		int no_ligne = 1;

		bFormatMEF = true;

		try
		{
			//1ere ligne de données; détermine le format du fichier, format HYDROTEL ou format MEF
			getline_mod(fichier, ligne);
			ligne = TrimString(ligne);

			while(ligne == "" && !fichier.eof())	//ignore les lignes vides s'il y en a
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);
			}

			if( ligne.size() >= 16 && (ligne[2] == '/' || ligne[2] == '-') )
				bFormatMEF = false;		//format HYDROTEL

			if(bFormatMEF)
			{
				//FORMAT MEF
				if(ligne.size() >= 29)
				{
					string ident = ligne.substr(0, 7);

					annee = static_cast<short>( atoi(ligne.substr(7, 4).c_str()) );
					mois = static_cast<short>( atoi(ligne.substr(11, 2).c_str()) );
					jour = static_cast<short>( atoi(ligne.substr(13, 2).c_str()) );

					epaisseur = static_cast<float>( atof(ligne.substr(15, 5).c_str()) ) * 0.01f;	//cm -> m
					equivalent = static_cast<float>( atof(ligne.substr(20, 5).c_str()) ) * 0.01f;	//cm -> m
					densite = static_cast<float>( atof(ligne.substr(25, 4).c_str()) );				//% 0-100
				}
				else
				{
					oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; ligne invalide; " << no_ligne;
					throw ERREUR(oss.str());
				}
			}
			else
			{
				//FORMAT HYDROTEL
				SplitString(sList, ligne, " \t", false, true);

				if(sList.size() < 4)
				{
					oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; nb de donnees invalide; ligne " << no_ligne;
					throw ERREUR(oss.str());
				}

				//date
				if(sList[0].size() != 10)
				{
					oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; format date invalide; ligne " << no_ligne;
					throw ERREUR(oss.str());
				}

				jour = static_cast<short>( atoi(sList[0].substr(0, 2).c_str()) );
				mois = static_cast<short>( atoi(sList[0].substr(3, 2).c_str()) );
				annee = static_cast<short>( atoi(sList[0].substr(6, 4).c_str()) );

				epaisseur = static_cast<float>( atof(sList[1].c_str()) ) * 0.01f;		//cm -> m
				equivalent = static_cast<float>( atof(sList[2].c_str()) ) * 0.001f;		//mm -> m
				
				densite = static_cast<float>( atof(sList[3].c_str()) );					//g/cm³
				densite*= 100.0f;														//g/cm³ -> % 0-100

			}

			// validation des donnees
			if (epaisseur < 0.0f || equivalent < 0.0f || densite < 0.0f || densite > 100.0f)
			{
				oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; donnees invalide; ligne " << no_ligne;
				throw ERREUR(oss.str());
			}

			DATE_HEURE date_lu(annee, mois, jour, 0);

			if (date_lu >= date_debut && date_lu < date_fin)
				_donnees[date_lu] = DONNEE_NEIGE(epaisseur, equivalent, densite);
		
			//lecture du reste du fichier
			++no_ligne;
			while (!fichier.eof())
			{
				getline_mod(fichier, ligne);
				ligne = TrimString(ligne);

				if(ligne != "")
				{
					if(bFormatMEF)
					{
						//FORMAT MEF
						if(ligne.size() >= 29)
						{
							string ident = ligne.substr(0, 7);

							annee = static_cast<short>( atoi(ligne.substr(7, 4).c_str()) );
							mois = static_cast<short>( atoi(ligne.substr(11, 2).c_str()) );
							jour = static_cast<short>( atoi(ligne.substr(13, 2).c_str()) );

							epaisseur = static_cast<float>( atof(ligne.substr(15, 5).c_str()) ) * 0.01f;	//cm -> m
							equivalent = static_cast<float>( atof(ligne.substr(20, 5).c_str()) ) * 0.01f;	//cm -> m
							densite = static_cast<float>( atof(ligne.substr(25, 4).c_str()) );				//% 0-100
						}
						else
						{
							oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; ligne invalide; " << no_ligne;
							throw ERREUR(oss.str());
						}
					}
					else
					{
						//FORMAT HYDROTEL
						SplitString(sList, ligne, " \t", false, true);

						if(sList.size() < 4)
						{
							oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; nb de donnees invalide; ligne " << no_ligne;
							throw ERREUR(oss.str());
						}

						//date
						if(sList[0].size() != 10)
						{
							oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; format date invalide; ligne " << no_ligne;
							throw ERREUR(oss.str());
						}

						jour = static_cast<short>( atoi(sList[0].substr(0, 2).c_str()) );
						mois = static_cast<short>( atoi(sList[0].substr(3, 2).c_str()) );
						annee = static_cast<short>( atoi(sList[0].substr(6, 4).c_str()) );

						epaisseur = static_cast<float>( atof(sList[1].c_str()) ) * 0.01f;		//cm -> m
						equivalent = static_cast<float>( atof(sList[2].c_str()) ) * 0.001f;		//mm -> m

						densite = static_cast<float>( atof(sList[3].c_str()) );					//g/cm³
						densite*= 100.0f;														//g/cm³ -> % 0-100
					}

					// validation des donnees
					if (epaisseur < 0.0f || equivalent < 0.0f || densite < 0.0f || densite > 100.0f)
					{
						oss << "STATIONS_NEIGE; erreur lecture donnees; " << _nom_fichier << "; donnees invalide; ligne " << no_ligne;
						throw ERREUR(oss.str());
					}

					DATE_HEURE date_lu2(annee, mois, jour, 0);

					if (date_lu2 >= date_debut && date_lu2 < date_fin)
						_donnees[date_lu2] = DONNEE_NEIGE(epaisseur, equivalent, densite);
				}

				++no_ligne;
			}
		}

		catch(const ERREUR& err)
		{
			fichier.close();
			throw ERREUR(err.what());
		}

		catch(...)
		{
			if (!fichier.eof())
			{
				fichier.close();
				oss << "STATION_NEIGE; erreur lecture donnees; " << _nom_fichier << "; ligne " << no_ligne;
				throw ERREUR(oss.str());
			}

		}

		fichier.close();
	}

}
