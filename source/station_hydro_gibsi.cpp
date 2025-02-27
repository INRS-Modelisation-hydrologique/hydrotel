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

#include "station_hydro_gibsi.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"

#include <fstream>
#include <map>


using namespace std;


namespace HYDROTEL
{

	STATION_HYDRO_GIBSI::STATION_HYDRO_GIBSI(const std::string& nom_fichier)
		: STATION_HYDRO(nom_fichier)
	{
	}

	STATION_HYDRO_GIBSI::~STATION_HYDRO_GIBSI()
	{
	}

	float STATION_HYDRO_GIBSI::PrendreDebit(DATE_HEURE date_heure, unsigned short pas_de_temps) const
	{
		int index = _date_debut.NbHeureEntre(date_heure) / pas_de_temps;
		return _debits.at(index);
	}

	void STATION_HYDRO_GIBSI::LectureDonnees(const DATE_HEURE& debut, const DATE_HEURE& fin, unsigned short pas_de_temps)
	{
		ifstream fichier(_nom_fichier);
		if (!fichier)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		fichier.exceptions(ios::failbit | ios::badbit);

		istringstream iss;
		int type = 0, no_ligne = 0;
		unsigned short pas_de_temps_lu = pas_de_temps;

		std::map<DATE_HEURE, float> debits;

		try
		{
			// lecture de l'en-tete
			string ligne;
			getline_mod(fichier, ligne);

			iss.str(ligne);
			iss.exceptions(ios::failbit | ios::badbit);

			iss >> type >> pas_de_temps_lu;		//type: 1=debit seulement, 2=debit + niveau
			++no_ligne;

			if (type != 1 && type != 2)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);

			if (pas_de_temps % pas_de_temps_lu)
				throw ERREUR_LECTURE_FICHIER(_nom_fichier, no_ligne);

			DATE_HEURE date_a_lire = debut;

			// lecture des donnees

			bool done = false;
			bool bAdjustDate = false;

			while (!done)
			{
				getline_mod(fichier, ligne);
				++no_ligne;
				
				if(ligne.size() > 0)
				{
					iss.clear();
					iss.str(ligne);
					iss.exceptions(ios::failbit | ios::badbit);

					unsigned short annee, mois, jour, heure = 0;
					char c;

					iss >> jour >> c >> mois >> c >> annee;

					if (pas_de_temps_lu != 24)
					{
						iss >> heure;

						if(heure % pas_de_temps_lu != 0)
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

						heure-= pas_de_temps_lu;	//heure lu en fin de pas de temps et remise en debut de pas de temps.
					}

					DATE_HEURE date_lu(annee, mois, jour, heure);
					if(bAdjustDate)
					{
						bAdjustDate = false;
						date_lu.SoustraitHeure(24);
					}

					if (date_lu >= debut && date_lu < fin)
					{
						float debit;
						iss >> debit;
						debits[date_lu] = debit;

						if(type == 2)
							iss >> debit;	//lecture du niveau [m]		//non utilisé
					}
					else if (date_lu >= fin)
					{
						done = true;
					}
				}

			}	//while (!done)
		}
		catch (const ERREUR_LECTURE_FICHIER& ex)
		{
			fichier.close();
			throw ex;
		}
		catch (...)
		{
			if (!fichier.eof())
			{
				fichier.close();
				throw ERREUR("Erreur STATION_HYDRO_GIBSI::LectureDonnees.");
			}
		}

		fichier.close();

		_debits.clear();

		DATE_HEURE date(debut);

		while (date < fin)
		{
			float debit = VALEUR_MANQUANTE;

			DATE_HEURE prochaine_date(date);
			prochaine_date.AdditionHeure(pas_de_temps);

			int nb_debit = 0;

			while (date <  prochaine_date)
			{
				if (debits.find(date) != end(debits))
				{
					float deb = debits[date];

					if (deb > VALEUR_MANQUANTE)
					{
						if (nb_debit == 0)
							debit = deb;
						else
							debit += deb;

						++nb_debit;
					}
				}
				date.AdditionHeure(pas_de_temps_lu);
			}

			// calcule la moyenne
			if (nb_debit > 0)
				debit = debit / nb_debit;

			_debits.push_back(debit);
		}

		_debits.shrink_to_fit();
		_date_debut = debut;
	}

}
