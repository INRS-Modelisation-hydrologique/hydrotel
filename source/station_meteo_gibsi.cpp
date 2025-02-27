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

#include "station_meteo_gibsi.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"

#include <fstream>
#include <sstream>


using namespace std;


namespace HYDROTEL
{

	STATION_METEO_GIBSI::STATION_METEO_GIBSI(const string& nom_fichier, bool bAutoInverseTMinTMax)
		: STATION_METEO(nom_fichier)
		, _nb_donnee(0)
	{
		_bAutoInverseTMinTMax = bAutoInverseTMinTMax;
	}


	STATION_METEO_GIBSI::~STATION_METEO_GIBSI()
	{
	}


	void STATION_METEO_GIBSI::LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps)
	{
		//-----TESTS-----
		//unsigned short a1, a2, m1, m2, d1, d2, h1, h2;

		//a1 = debut.PrendreAnnee();
		//m1 = debut.PrendreMois();
		//d1 = debut.PrendreJour();
		//h1 = debut.PrendreHeure();

		//a2 = fin.PrendreAnnee();
		//m2 = fin.PrendreMois();
		//d2 = fin.PrendreJour();
		//h2 = fin.PrendreHeure();
		//-----TESTS-----

		ifstream fichier(_nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		fichier.exceptions(ios::failbit | ios::badbit);

		vector<std::string> sList;
		unsigned short annee, mois, jour, heure = 0;
		float fVal;
		char c;
		int type = 0, no_ligne = 1;

		map<DATE_HEURE, DONNEE_METEO> donnees_meteo;

		try
		{
			string ligne;

			// lecture de l'en-tete
			getline_mod(fichier, ligne);

			istringstream iss(ligne);
			iss.exceptions(ios::failbit | ios::badbit);

			iss >> type >> _pas_de_temps;			

			if (pas_de_temps % _pas_de_temps)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "Pas de temps invalide");

			DATE_HEURE date_a_lire = debut;

			// lecture des donnees

			bool done = false;
			bool bAdjustDate = false;

			while (!done)
			{
				++no_ligne;
				getline_mod(fichier, ligne);
				
				if(ligne.size() > 0)
				{
					//validation
					SplitString(sList, ligne, " \t", true, false);	//separateur; espace et \t
					if (_pas_de_temps == 1)
					{
						if(sList.size() != 4)
							throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "nombre de valeurs invalide; pour un pas de temps de 1 heure, le format des lignes de donnees doit etre: [JJ/MM/AAAA] [HH] [Temp.] [Precip.]");
					}
					else
					{
						if (_pas_de_temps == 24)
						{
							if(sList.size() != 4)
								throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "nombre de valeurs invalide; pour un pas de temps de 24 heures, le format des lignes de donnees doit etre: [JJ/MM/AAAA] [TMax] [TMin] [Precip.]");
						}
						else
						{
							if(sList.size() != 5)
								throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "nombre de valeurs invalide; pour un pas de temps autre que 1 ou 24 heures, le format des lignes de donnees doit etre: [JJ/MM/AAAA] [HH] [TMax] [TMin] [Precip.]");
						}
					}

					//
					istringstream iss2(ligne);
					iss2.exceptions(ios::failbit | ios::badbit);

					iss2 >> jour >> c >> mois >> c >> annee;

					if (_pas_de_temps != 24)
					{
						iss2 >> heure;

						if(heure % _pas_de_temps != 0)
							throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "L'heure lu est invalide selon le pas de temps de la simulation.");

						if(heure == 0)
						{
							heure = 24;
							bAdjustDate = true;
						}

						//L'heure lu dans les fichiers de données hydro et meteo correspond à l'heure à la fin du pas de temps.
						//Exemple: pour un pas de temps de 3h, les heures dans le fichier de données doivent être les 
						//suivantes pour une journée; 3, 6, 9, 12, 15, 18, 21, 24 (ancien format) OU 0, 3, 6, 9, 12, 15, 18, 21. 
						//L'heure lu représente la fin du pas de temps pour la données. Donc l'heure 3 représente la données pour 0h à 3h.

						//Hydrotel fonctionne à l'interne avec des début de pas de temps, on doit donc réajuster les pas de temps des données météo lu.
						//Les fichiers de résultats contiennent également des début de pas de temps. Donc l'heure 3 représente la données pour 3h à 6h.
						
						heure-= _pas_de_temps;		//heure lu en fin de pas de temps et remise en debut de pas de temps.
					}

					DATE_HEURE date_lu(annee, mois, jour, heure);
					if(bAdjustDate)
					{
						bAdjustDate = false;
						date_lu.SoustraitHeure(24);
					}						

					if (date_lu >= debut && date_lu < fin)
					{
						float tmin = VALEUR_MANQUANTE;
						float tmax = VALEUR_MANQUANTE;
					
						float pluie = VALEUR_MANQUANTE;
						float neige = VALEUR_MANQUANTE;

						// type 1 - Tmax, Tmin, Prec
						// type 2 - Tmax, Tmin, Prec, Hum, Ens
						// type 3 - Tmax, Tmin, Prec, Hum, Ens, Ray
						// type 4 - Tmax, Tmin, Prec, Hum, Ens, Vent
						// type 5 - Tmax, Tmin, Prec, Hum, Ens, Ray, Vent
					
						// NOTE: il y a une seule temperature pour un pas de temps horaire

						switch (type)
						{
						case 1:
							if (_pas_de_temps == 1)
							{
								iss2 >> tmin >> pluie;
								tmax = tmin;
							}
							else
							{
								iss2 >> tmax >> tmin >> pluie;
							}
							break;
	
						//case 2:
						//	break;

						//case 3:
						//	break;

						//case 4:
						//	break;

						//case 5:
						//	break;
	
						default:
							throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "Le type specifie est invalide (ligne 1, colonne 1)");
						}

						// valide les donnees

						if ( (tmin > VALEUR_MANQUANTE && (tmin < -100.0f || tmin > 70.0f)) || 
							(tmax > VALEUR_MANQUANTE && (tmax < -100.0f || tmax > 70.0f)) )
						{
							throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "TMin/TMax valeur invalide");
						}

						if(_bAutoInverseTMinTMax)
						{
							if (tmin > VALEUR_MANQUANTE && tmax > VALEUR_MANQUANTE && tmin > tmax)
							{
								//inverse automatiquement tmin et tmax s'ils sont inversés dans le fichier d'entrée
								fVal = tmin;
								tmin = tmax;
								tmax = fVal;
							}

							////si seulement une des valeurs est manquante (tmin ou tmax) ont met les valeurs égale (tmin = tmax)
							//if(tmin == VALEUR_MANQUANTE && tmax > VALEUR_MANQUANTE)
							//	tmin = tmax;
							//else
							//{
							//	if(tmax == VALEUR_MANQUANTE && tmin > VALEUR_MANQUANTE)
							//		tmax = tmin;
							//}
						}
						else
						{
							if (tmin > VALEUR_MANQUANTE && tmax > VALEUR_MANQUANTE && tmin > tmax)
								throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "TMin plus grand que TMax");
						}

						if (pluie > VALEUR_MANQUANTE && pluie < 0.0f)
							throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne, "Pluie invalide");

						//
						DONNEE_METEO donnee_meteo;

						if(_iVersionThiessenMoy3Station == 1)
							donnee_meteo.ChangeTemperature_v1(tmin, tmax);
						else
							donnee_meteo.ChangeTemperature(tmin, tmax);

						donnee_meteo.ChangePluie(pluie);
						donnee_meteo.ChangeNeige(neige);

						donnees_meteo[date_lu] = donnee_meteo;
					}
					else if (date_lu >= fin)
					{
						done = true;
					}
				}

			} //while (!done)
		}
		catch (const ERREUR_LECTURE_FICHIER& err)
		{
			if (!fichier.eof())
			{
				fichier.close();
				throw err;
			}
		}
		catch (...)
		{
			if (!fichier.eof())
			{
				fichier.close();
				throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);
			}
		}

		fichier.close();

		//
		DATE_HEURE date(debut);

		while (date < fin)
		{
			DONNEE_METEO donnee_meteo;

			DATE_HEURE prochaine_date(date);
			prochaine_date.AdditionHeure(pas_de_temps);

			while (date <  prochaine_date)
			{
				switch (type)
				{
				case 1:
					{
						float tmin = donnees_meteo[date].PrendreTMin();
						float tmax = donnees_meteo[date].PrendreTMax();
						float pluie = donnees_meteo[date].PrendrePluie();

						if (tmin > VALEUR_MANQUANTE && donnee_meteo.PrendreTMin() > VALEUR_MANQUANTE)
							tmin = min(tmin, donnee_meteo.PrendreTMin());
						
						if (tmax > VALEUR_MANQUANTE && donnee_meteo.PrendreTMax() > VALEUR_MANQUANTE)
							tmax = max(tmax, donnee_meteo.PrendreTMax());
						
						if (pluie > VALEUR_MANQUANTE && donnee_meteo.PrendrePluie() > VALEUR_MANQUANTE)
							pluie = pluie + donnee_meteo.PrendrePluie();

						if(_iVersionThiessenMoy3Station == 1)
							donnee_meteo.ChangeTemperature_v1(tmin, tmax);
						else
							donnee_meteo.ChangeTemperature(tmin, tmax);

						donnee_meteo.ChangePluie(pluie);
						donnee_meteo.ChangeNeige(0);
					}
					break;
				
				//case 2:
				//	break;
				//case 3:
				//	break;
				//case 4:
				//	break;
				//case 5:
				//	break;
				
				}

				date.AdditionHeure(_pas_de_temps);
			}

			_tmin.push_back(donnee_meteo.PrendreTMin());
			_tmax.push_back(donnee_meteo.PrendreTMax());
			_pluie.push_back(donnee_meteo.PrendrePluie());
			_neige.push_back(donnee_meteo.PrendreNeige());
		}

		_tmin.shrink_to_fit();
		_tmax.shrink_to_fit();
		_pluie.shrink_to_fit();
		_neige.shrink_to_fit();

 		_date_debut = debut;

		_nb_donnee = _tmin.size();

		_pas_de_temps = pas_de_temps;
	}


	void STATION_METEO_GIBSI::ChangeDonnees(const DONNEE_METEO& donnee_meteo, const DATE_HEURE& date_heure, unsigned short pas_de_temps)
	{
		int index = _date_debut.NbHeureEntre(date_heure) / pas_de_temps;
		int nb_donnee = static_cast<int>(_nb_donnee);

		if (index >= 0 && index < nb_donnee)
		{
			_tmin[index] = donnee_meteo.PrendreTMin();
			_tmax[index] = donnee_meteo.PrendreTMax();
			_pluie[index] = donnee_meteo.PrendrePluie();
			_neige[index] = donnee_meteo.PrendreNeige();
		}
	}


	DONNEE_METEO STATION_METEO_GIBSI::PrendreDonnees(const DATE_HEURE& date_heure, unsigned short pas_de_temps)
	{
		DONNEE_METEO donnee_meteo;

		int index = _date_debut.NbHeureEntre(date_heure) / pas_de_temps;
		int nb_donnee = static_cast<int>(_nb_donnee);

		if (index >= 0 && index < nb_donnee)
		{
			if(_iVersionThiessenMoy3Station == 1)
				donnee_meteo.ChangeTemperature_v1(_tmin[index], _tmax[index]);
			else
				donnee_meteo.ChangeTemperature(_tmin[index], _tmax[index]);

			donnee_meteo.ChangePluie(_pluie[index]);
			donnee_meteo.ChangeNeige(_neige[index]);
		}

		return donnee_meteo;
	}


	pair<float, float> STATION_METEO_GIBSI::PrendreTemperatureJournaliere(const DATE_HEURE& date_heure)
	{
		DATE_HEURE date(date_heure.PrendreAnnee(), date_heure.PrendreMois(), date_heure.PrendreJour(), 0);

		size_t index = _date_debut.NbHeureEntre(date) / _pas_de_temps;
		size_t nb_pas = 24 / _pas_de_temps;
		size_t n;
		float tmin, tmax;

		n = 0;
		while(_tmin[index + n] <= VALEUR_MANQUANTE && n < nb_pas)
			++n;

		if(n == nb_pas)
			tmin = tmax = VALEUR_MANQUANTE;
		else
		{
			tmin = _tmin[index + n];

			n = 0;
			while(_tmax[index + n] <= VALEUR_MANQUANTE && n < nb_pas)
				++n;

			if(n == nb_pas)
				tmin = tmax = VALEUR_MANQUANTE;
			else
			{
				tmax = _tmax[index + n];

				for (n = 1; n < nb_pas; ++n)
				{
					if(_tmin[index + n] > VALEUR_MANQUANTE)
						tmin = min(tmin, _tmin[index + n]);

					if(_tmax[index + n] > VALEUR_MANQUANTE)
						tmax = max(tmax, _tmax[index + n]);
				}
			}
		}

		return make_pair(tmin, tmax);
	}

}
