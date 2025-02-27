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

#include "statistiques.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "station_hydro.hpp"
#include "util.hpp"

#include <fstream>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	STATISTIQUES::STATISTIQUES(SIM_HYD& sim_hyd, const string& nom_fichier_parametres)
		: _sim_hyd(&sim_hyd)
		, _nom_fichier_parametres(nom_fichier_parametres)
	{
		LectureParametres();
		LectureDonnees();
	}

	STATISTIQUES::~STATISTIQUES()
	{
	}

	void STATISTIQUES::LectureParametres()
	{
		ifstream fichier(_nom_fichier_parametres);
		fichier.exceptions(ios::failbit | ios::badbit);

		try 
		{
			while (!fichier.eof())
			{
				int troncon_id;
				string station_id, stemp;
				fichier >> troncon_id >> station_id;

				if(station_id.size() != 0)
				{
					stemp = station_id;
					boost::algorithm::to_lower(stemp);

					if(stemp.size() > 4 && stemp.substr(stemp.size()-4) == ".hyd")
						station_id = station_id.substr(0, station_id.size()-4);
				}
			
				_troncon_station[troncon_id] = station_id;
			}
		}
		catch (...)
		{
			if (!fichier.eof())
			{
				fichier.close();
				throw ERREUR("Erreur STATISTIQUES::LectureParametres.");
			}
		}

		fichier.close();
	}
	

	//creation du fichier stats.csv
	void STATISTIQUES::LectureDonnees()
	{
		string path = Combine(_sim_hyd->PrendreRepertoireResultat(), "debit_aval.csv");
		
		DATE_HEURE date_debut = _sim_hyd->PrendreDateDebut();
		DATE_HEURE date_fin = _sim_hyd->PrendreDateFin();
		unsigned short pas_de_temps = _sim_hyd->PrendrePasDeTemps();
		size_t nb_troncon;
			
		map<int, vector<pair<float, float>>> donnees;

		vector<size_t> vIDTroncon;
		string sDate;

		try
		{
			map<int, float> ecart2s;
			map<int, float> nashs;
			map<int, float> biais_relatif;
			map<int, float> biais_absolue;
			map<int, float> coefficient_correlation;
			map<int, float> mapKGE_2009;
			map<int, float> mapKGE_2012;
			map<int, float> mapCoeffPointe;
			map<int, float> mapCoeffVolume;
			map<int, float> mapNashLog;
			map<int, float> mapNashMod;
			map<int, float> mapEcartQuad;
			
			map<int, float> somme_obs;
			map<int, float> somme_sim;
			
			map<int, float> moyenne_obs;
			map<int, float> moyenne_sim;

			vector<float> vVal;

			vector<int> idents;
			for (auto iter = begin(_troncon_station); iter != end(_troncon_station); ++iter)
				idents.push_back(iter->first);

			// lecture des valeurs simulees					
			ifstream fichier(path);
			
			string ligne, sTemp, sTemp2;

			getline_mod(fichier, ligne); // commentaire
			
			//lecture de l'entete et conserve les numero de troncon present dans le fichier
			getline_mod(fichier, sTemp);
			sTemp2 = "date heure\\troncon;";
			ligne = sTemp.substr(sTemp2.size(), sTemp.size() - sTemp2.size());
			vIDTroncon = extrait_svaleur(ligne, _sim_hyd->PrendreOutput().Separator());
			nb_troncon = vIDTroncon.size();

			DATE_HEURE date_courante(date_debut);
			while (date_courante < date_fin)
			{
				getline_mod(fichier, sTemp);
				if(sTemp.size() < 18)
				{
					fichier.close();
					throw ERREUR("Computation of statistics: flows reading error: invalid debit_aval.csv file.");
				}

				sDate = sTemp.substr(0, 16);
				sTemp = sTemp.substr(17);

				vVal = extrait_fvaleur(sTemp, _sim_hyd->PrendreOutput().Separator());
				if(vVal.size() != nb_troncon)
				{
					fichier.close();
					throw ERREUR("Computation of statistics: flows reading error: invalid debit_aval.csv file.");
				}

				for (size_t n = 0; n < nb_troncon; ++n)
				{
					if (find(begin(idents), end(idents), static_cast<int>(vIDTroncon[n])) != end(idents))
						donnees[ static_cast<int>(vIDTroncon[n]) ].push_back(make_pair(vVal[n], VALEUR_MANQUANTE));
				}
				
				date_courante.AdditionHeure(pas_de_temps);
			}

			fichier.close();

			// lecture des valeurs observees

			try{

			STATIONS_HYDRO& stations_hydro = _sim_hyd->PrendreStationsHydro();

			for (auto iter = begin(_troncon_station); iter != end(_troncon_station); ++iter)
			{
				if(donnees.find(iter->first) != donnees.end())
				{
					STATION_HYDRO* station_hydro = static_cast<STATION_HYDRO*>(stations_hydro.Recherche(iter->second));

					size_t index = 0;
					DATE_HEURE date_courante2(date_debut);

					if (station_hydro)
					{
						// lecture des valeurs observees
						station_hydro->LectureDonnees(date_debut, date_fin, pas_de_temps);

						while (date_courante2 < date_fin)
						{
							donnees[iter->first][index++].second = station_hydro->PrendreDebit(date_courante2, pas_de_temps);
							date_courante2.AdditionHeure(pas_de_temps);
						}
					}
					else
					{
						string tmp;
						tmp = iter->second;
						boost::algorithm::to_lower(tmp);

						if(tmp == "absent")
						{
							while (date_courante2 < date_fin)
							{
								donnees[iter->first][index++].second = VALEUR_MANQUANTE;
								date_courante2.AdditionHeure(pas_de_temps);
							}
						}
						else
							throw ERREUR("Computation of statistics: hydrometric station not found: " + iter->second);
					}
				}
			}

			}
			catch(const ERREUR_LECTURE_FICHIER& err)
			{
				throw ERREUR(err.what());
			}
			catch(const ERREUR& err2)
			{
				throw ERREUR(err2.what());
			}

			// calcul des statistiques
			for (auto iter = begin(donnees); iter != end(donnees); ++iter)
			{
				float moy_obs = 0.0f;
				float moy_log_obs = 0.0f;
				float som_obs = 0.0f;
				size_t nb_pas_obs = 0;

				float moy_sim = 0.0f;
				float som_sim = 0.0f;
				size_t nb_pas_sim = 0;

				for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
				{
					if (debits->first > VALEUR_MANQUANTE)
					{
						moy_sim += debits->first;
						som_sim += debits->first;

						nb_pas_sim++;
					}

					if (debits->second > VALEUR_MANQUANTE)
					{
						moy_obs += debits->second;
						som_obs += debits->second;
						moy_log_obs+= log(debits->second);

						nb_pas_obs++;
					}
				}

				if (nb_pas_obs > 0)
				{
					moy_obs /= nb_pas_obs;
					moy_log_obs/= nb_pas_obs;

					somme_obs[iter->first] = som_obs;
					moyenne_obs[iter->first] = moy_obs;
				}
				else
				{
					somme_obs[iter->first] = VALEUR_MANQUANTE;
					moyenne_obs[iter->first] = VALEUR_MANQUANTE;
				}

				if (nb_pas_sim > 0)
				{
					moy_sim /= nb_pas_sim;

					somme_sim[iter->first] = som_sim;
					moyenne_sim[iter->first] = moy_sim;
				}
				else
				{
					moyenne_sim[iter->first] = VALEUR_MANQUANTE;
					somme_sim[iter->first] = VALEUR_MANQUANTE;
				}

				// nash, ecart
				float ecart2 = VALEUR_MANQUANTE;
				float nash = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					ecart2 = 0.0f;
					nash = 0.0f;

					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							ecart2 += pow(debits->second - debits->first, 2.0f);
						}
					}

					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							nash += pow(debits->second - moy_obs, 2.0f);
						}
					}

					if(nash != 0.0f)
						nash = 1.0f - ecart2 / nash;

					ecart2 = sqrt((1.0f / nb_pas_obs) * ecart2);
				}

				// biais relatif
				float biais = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float difference = 0.0f;
					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							difference += debits->first - debits->second;
						}
					}

					if(som_obs != 0.0f)
					{
						//biais = difference / som_obs * 100.0f;	//Pourcentage de biais relatif (P-Biais)
						biais = difference / som_obs;				//Biais relatif
					}
				}

				// biais absolue
				float biaisAbsolue = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float difference = 0.0f;
					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
							difference += debits->second - debits->first;
					}

					if(som_obs != 0.0f)
						biaisAbsolue = abs(difference / som_obs);
				}

				// coefficient correlation
				float coeff_corr = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float multValeurMoy = 0.0f;
					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							multValeurMoy  += (debits->second - moy_obs) * (debits->first - moy_sim);
						}
					}

					float obs2 = 0.0f;
					float sim2 = 0.0f;
					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							obs2 += pow(debits->second - moy_obs, 2.0f);	
							sim2 += pow(debits->first - moy_sim, 2.0f);
						}
					}
					
					if(obs2 != 0.0f && sim2 != 0.0f)
						coeff_corr = multValeurMoy / ((sqrt(obs2) * sqrt(sim2)));
				}

				//KGE1 - KGE original (2009) & KGE2 - KGE modifié (2012)
				float kge1 = VALEUR_MANQUANTE;
				float kge2 = VALEUR_MANQUANTE;

				if(nb_pas_obs > 0 && coeff_corr > VALEUR_MANQUANTE)
				{
					float temp1 = 0.0f;
					float temp2 = 0.0f;
					float varObs, varSim, fA, fB, fCV;

					for(auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if(debits->second > VALEUR_MANQUANTE)
							temp1 = temp1 + debits->second * debits->second;

						temp2 = temp2 + debits->first * debits->first;
					}

					varObs = temp1 / nb_pas_obs - moy_obs * moy_obs;
                    varSim = temp2 / nb_pas_sim - moy_sim * moy_sim;

					fA = varSim / varObs;
					fA = sqrt(fA);

                    fB = moy_sim / moy_obs;

					fCV = (sqrt(varSim) / moy_sim) / (sqrt(varObs) / moy_obs);

					kge1 = 1.0f - sqrt(pow(coeff_corr - 1.0f, 2.0f) + pow(fA - 1.0f, 2.0f) + pow(fB - 1.0f, 2.0f));
					kge2 = 1.0f - sqrt(pow(coeff_corr - 1.0f, 2.0f) + pow(fCV - 1.0f, 2.0f) + pow(fB - 1.0f, 2.0f));
				}

				//coefficient de pointe
				float coeffPointe = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float temp1 = 0.0f;
					float temp2 = 0.0f;

					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							temp2 = temp2 + pow(debits->second, 2.0f);
							temp1 = temp1 + debits->second * pow(debits->second - debits->first, 2.0f);
						}
					}

					if (temp2 != 0.0)
						coeffPointe = pow(temp1, 0.25f) / pow(temp2, 0.5f);
				}

				//coefficient de volume
				float coeffVolume = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float temp1 = 0.0f;

					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
							temp1 = temp1 + (debits->second - debits->first);
					}

					if (moy_obs != 0.0f)
						coeffVolume = temp1 / (nb_pas_obs * moy_obs);
				}

				//Nash-Log
				float nashLog = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float temp1 = 0.0f;
					float temp2 = 0.0f;

					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							temp1 = temp1 + pow(log(debits->second) - log(debits->first), 2.0f);
							temp2 = temp2 + pow(log(debits->second) - moy_log_obs, 2.0f);
						}
					}

					if (temp2 != 0.0f)
						nashLog = 1.0f - temp1 / temp2;
				}

				//Nash-M
				float nashMod = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float temp1 = 0.0f;
					float temp2 = 0.0f;

					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
						{
							temp2 = temp2 + pow(debits->second * (debits->second - moy_obs), 2.0f);
							temp1 = temp1 + pow(debits->second * (debits->second - debits->first), 2.0f);
						}
					}

					if (temp2 != 0.0f)
						nashMod = 1.0f - temp1 / temp2;
				}

				//EcartQdrMoyen
				float ecartQuadratique = VALEUR_MANQUANTE;
				if (nb_pas_obs > 0)
				{
					float temp1 = 0.0f;

					for (auto debits = begin(iter->second); debits != end(iter->second); ++debits)
					{
						if (debits->second > VALEUR_MANQUANTE)
							temp1 = temp1 + pow((debits->first - debits->second) / debits->second, 2.0f);
					}

					ecartQuadratique = sqrt(temp1);
				}

				//
				ecart2s[iter->first] = ecart2;
				nashs[iter->first] = nash;
				biais_relatif[iter->first] = biais;
				biais_absolue[iter->first] = biaisAbsolue;
				coefficient_correlation[iter->first] = coeff_corr;
				mapKGE_2009[iter->first] = kge1;
				mapKGE_2012[iter->first] = kge2;
				mapCoeffPointe[iter->first] = coeffPointe;
				mapCoeffVolume[iter->first] = coeffVolume;
				mapNashLog[iter->first] = nashLog;
				mapNashMod[iter->first] = nashMod;
				mapEcartQuad[iter->first] = ecartQuadratique;

			}

			// ecriture des resultats
			{
				path = Combine(_sim_hyd->PrendreRepertoireResultat(), "stats.csv");
				string sep = _sim_hyd->PrendreOutput().Separator();

				ofstream fichier3(path);
				if (!fichier3)
					throw ERREUR_ECRITURE_FICHIER(path);

				//ecriture donnees fonctions objective
				fichier3 << "troncon ident" << sep << "RCEQM" << sep << "Nash-Sutcliffe" << sep << "Biais relatif" 
					<< sep << "Biais absolue" << sep << "Coefficient de correlation" << sep << "KGE original (2009)" << sep << "KGE modifié (2012)" 
					<< sep << "Coefficient de pointe" << sep << "Coefficient de volume" << sep << "Nash-Log" 
					<< sep << "Nash-M" << sep << "Écart quadratique moyen" << sep << "Somme Obs." 
					<< sep << "Somme Sim." << sep << "Moyenne Obs." << sep << "Moyenne Sim." << endl;

				for (auto iter = begin(donnees); iter != end(donnees); ++iter)
				{
					int ident = iter->first;

					fichier3 << ident << sep 
						<< ecart2s[ident] << sep 
						<< nashs[ident] << sep
						<< biais_relatif[ident] << sep
						<< biais_absolue[ident] << sep
						<< coefficient_correlation[ident] << sep
						<< mapKGE_2009[ident] << sep
						<< mapKGE_2012[ident] << sep
						<< mapCoeffPointe[ident] << sep
						<< mapCoeffVolume[ident] << sep
						<< mapNashLog[ident] << sep
						<< mapNashMod[ident] << sep
						<< mapEcartQuad[ident] << sep
						<< somme_obs[ident] << sep
						<< somme_sim[ident] << sep
						<< moyenne_obs[ident] << sep
						<< moyenne_sim[ident] << endl;
				}
				fichier3 << endl;

				//ecriture debits simules/debits observes
				fichier3 << "date" << sep;

				string str;
				ostringstream oss;
				oss.str("");

				for (auto iter = begin(_troncon_station); iter != end(_troncon_station); ++iter)
				{
					if(donnees.find(iter->first) != donnees.end())
					{
						oss << "troncon id " << iter->first << sep;
						oss << "station id " << iter->second << sep;
					}
				}

				str = oss.str();
				if(str != "")
				{
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					fichier3 << str << endl;
				}
				else
					fichier3 << endl;

				DATE_HEURE date_courante3 = date_debut;
				int pas = 0;

				while (date_courante3 < date_fin)
				{
					fichier3 << date_courante3;

					for (auto iter = begin(donnees); iter != end(donnees); ++iter)
					{
						fichier3 << sep << iter->second[pas].first;		//debit simule
						fichier3 << sep << iter->second[pas].second;		//debit observe
					}
					fichier3 << endl;

					date_courante3.AdditionHeure(pas_de_temps);
					pas++;
				}

				fichier3.close();
			}

			//creation fichier pour Ostrich
			path = Combine(_sim_hyd->PrendreRepertoireResultat(), "obs-sim-flows.csv");
			string sep = _sim_hyd->PrendreOutput().Separator();

			ofstream fichier2(path);
			if (!fichier2)
				throw ERREUR_ECRITURE_FICHIER(path);

			//string str;
			//ostringstream oss;
			//oss.str("");

			//for (auto iter = begin(_troncon_station); iter != end(_troncon_station); ++iter)
			//{
			//	if(donnees.find(iter->first) != donnees.end())
			//	{
			//		oss << "troncon id " << iter->first << sep;
			//		oss << "station id " << iter->second << sep;
			//	}
			//}

			//str = oss.str();
			//if(str != "")
			//{
			//	str = str.substr(0, str.length()-1); //enleve le dernier separateur
			//	fichier2 << str << endl;
			//}
			//else
			//	fichier2 << endl;

			ostringstream oss;
			DATE_HEURE date_courante4 = date_debut;
			int pas = 0;

			while (date_courante4 < date_fin)
			{
				fichier2 << date_courante4;

				for (auto iter = begin(donnees); iter != end(donnees); ++iter)
				{
					//iter->second[pas].second -> debit observe
					//iter->second[pas].first -> debit simule

					oss.str("");
					oss << sep << setprecision(_sim_hyd->PrendreOutput()._nbDigit_m3s) << setiosflags(ios::fixed) << iter->second[pas].second << sep << setprecision(_sim_hyd->PrendreOutput()._nbDigit_m3s) << setiosflags(ios::fixed) << iter->second[pas].first;
					fichier2 << oss.str();
				}
				fichier2 << endl;

				date_courante4.AdditionHeure(pas_de_temps);
				pas++;
			}

			fichier2.close();

		}
		catch (const ERREUR& err)
		{
			throw ERREUR(err.what());
		}
		catch (exception&)
		{
			throw ERREUR_LECTURE_FICHIER(path);
		}
		
	}
	
}
