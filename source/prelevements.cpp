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

#include "prelevements.hpp"

#include "util.hpp"
#include "projections.hpp"
#include "transforme_coordonnee.hpp"
#include "prelevements_site.hpp"
#include "erreur.hpp"

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	PRELEVEMENTS::PRELEVEMENTS(SIM_HYD& sim_hyd)
		: _sim_hyd(sim_hyd)
	{
		_FolderName = "prelevements";
		_FolderNameSrc = "SitesPrelevements";

		_bSimulePrelevements = false;
	}


	PRELEVEMENTS::~PRELEVEMENTS()
	{
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::GenerateBdPrelevements()
	{
		string str, str2, path;

		std::cout << endl << "creation bd prelevements..." << endl << endl;

		try{

		path = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + _FolderNameSrc;
		if(boost::filesystem::exists(path))
		{
			vector<string> v_fichier;

			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_GPE.csv";
			if(boost::filesystem::exists(str))
				boost::filesystem::remove(str);
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_ELEVAGES.csv";
			if(boost::filesystem::exists(str))
				boost::filesystem::remove(str);
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_PRELEVEMENTS.csv";
			if(boost::filesystem::exists(str))
				boost::filesystem::remove(str);
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_SIH.csv";
			if(boost::filesystem::exists(str))
				boost::filesystem::remove(str);
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_CULTURES.csv";
			if(boost::filesystem::exists(str))
				boost::filesystem::remove(str);
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_EFFLUENTS.csv";
			if(boost::filesystem::exists(str))
				boost::filesystem::remove(str);

			//fichier GPE
			boost::filesystem::directory_iterator iter;
			boost::filesystem::path p(path);
			for(iter = boost::filesystem::directory_iterator(p); iter!=boost::filesystem::directory_iterator(); iter++)
			{
				if(boost::filesystem::is_regular_file(*iter))
				{
					str = iter->path().string();
					str2 = str.substr(str.rfind("/")+1);
					boost::algorithm::to_lower(str2);

					if(str2.length() == 33 && 
						str2.substr(0, 25) == "gpe_volumes_prelevements_" && str2.substr(29, 4) == ".csv")
					{
						v_fichier.push_back(str);
					}
				}
			}

			std::sort(v_fichier.begin(), v_fichier.end());
			reverse(v_fichier.begin(), v_fichier.end());

			if(v_fichier.size() != 0)
			{
				str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_GPE.csv";
				_sim_hyd._pr->trierDonnePrelevementGPE(v_fichier, str);
				std::cout << "creation BD_GPE.csv ok" << endl;
			}
			else
				std::cout << "donnees GPE_Volumes_prelevements_YYYY.csv absent" << endl;

			//Agricole	//Elevage
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + _FolderNameSrc + "/" + "ELEVAGES_AGR_lieuxElevage.csv";
			str2 = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + _FolderNameSrc + "/" + "ELEVAGES_Consommation_cheptel.csv";
			if(boost::filesystem::exists(str) && boost::filesystem::exists(str2))
			{
				v_fichier.clear();
				v_fichier.push_back(str);

				str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_ELEVAGES.csv";
				_sim_hyd._pr->trierDonnePrelevementagricole(v_fichier, str2, str);
				std::cout << "creation BD_ELEVAGES.csv ok" << endl;
			}
			else
				std::cout << "donnees ELEVAGES_AGR_lieuxElevage.csv et ELEVAGES_Consommation_cheptel.csv absent" << endl;

			//Sites prelevements
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + _FolderNameSrc + "/" + "PRELEVEMENTS_SITES.csv";
			if(boost::filesystem::exists(str))
			{
				str2 = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_PRELEVEMENTS.csv";
				if(!_sim_hyd._pr->AddTronconUhrhToFile(str, str2, 1))
					std::cout << "erreur Prelevements_AddTronconUhrhToFile: " << str << endl;
				else
					std::cout << "creation BD_PRELEVEMENTS.csv ok" << endl;
			}
			else
				std::cout << "donnees PRELEVEMENTS_SITES.csv absent" << endl;

			//Sites SIH
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + _FolderNameSrc + "/" + "SIH_SITES.csv";
			if(boost::filesystem::exists(str))
			{
				str2 = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_SIH.csv";
				if(!_sim_hyd._pr->AddTronconUhrhToFile(str, str2, 1))
					std::cout << "erreur Prelevements_AddTronconUhrhToFile: " << str << endl;
				else
					std::cout << "creation BD_SIH.csv ok" << endl;
			}
			else
				std::cout << "donnees SIH_SITES.csv absent" << endl;

			//Sites CULTURES
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + _FolderNameSrc + "/" + "CULTURES_SITES.csv";
			if(boost::filesystem::exists(str))
			{
				str2 = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_CULTURES.csv";
				if(!_sim_hyd._pr->AddTronconUhrhToFile(str, str2, 1))
					std::cout << "erreur Prelevements_AddTronconUhrhToFile: " << str << endl;
				else
					std::cout << "creation BD_CULTURES.csv ok" << endl;
			}
			else
				std::cout << "donnees CULTURES_SITES.csv absent" << endl;

			//Sites EFFLUENTS
			str = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + _FolderNameSrc + "/" + "EFFLUENTS_SITES.csv";
			if(boost::filesystem::exists(str))
			{
				str2 = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_EFFLUENTS.csv";
				if(!_sim_hyd._pr->AddTronconUhrhToFile(str, str2, 1))
					std::cout << "erreur Prelevements_AddTronconUhrhToFile: " << str << endl;
				else
					std::cout << "creation BD_EFFLUENTS.csv ok" << endl;
			}
			else
				std::cout << "donnees EFFLUENTS_SITES.csv absent" << endl;

			std::cout << endl << "termine" << endl << endl;
		}
		else
			std::cout << endl << "dossier de donnees sources introuvable (prelevements/SitesPrelevements)" << endl << endl;

		}
		catch(const exception& ex)
		{
			std::cout << endl << "erreur exception: " << ex.what() << endl << endl;
			return false;
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevements()
	{
		_donneesREDCOEFF.clear();

		//lecture des types de prélèvement et si on doit les inclure ou non dans la simulation
		if(!LecturePrelevementsTYPES())
			return false;

		if(_donneesTYPE_redCoeff.size() != 0)	//read only if the reduction coefficient is enabled at least for one type
		{
			if(!LecturePrelevementsREDCOEFF())
				return false;
		}

		if(!LecturePrelevementsEFFLUENT())
			return false;

		if(!LecturePrelevementsGPE())
			return false;

		if(!LecturePrelevementsPR())
			return false;

		if(_donneesPR.size() != 0)	//les prelevements des sites de culture et des sites d'élevage sont effectué avec les sites de prélevement. Si pas de site Pr pas de prélevement culture/elevage...
		{
			if(!LecturePrelevementsELEVAGE())
				return false;
			
			if(!LecturePrelevementsCULTURE())
				return false;
		}

		//pre traitements

		map<int, vector<int>>::const_iterator itPrCu;
		map<int, std::string>::const_iterator itElSite;
		map<int, string>::const_iterator itPr;
		map<int, int>::const_iterator itAss;
		ostringstream oss;
		vector<string> vAssStr;
		vector<string> vErr;
		vector<int> vAssID;
		ofstream ofs;
		double dLat, dLon, dLatPr, dLonPr, dDistance;
		size_t i, j;
		string sNom, sNom2, sNomO, sNom2O, sNomPr;
		bool bFound;

		dLon = -1.0;

		_donneesGPE_PR.clear();
		_donneesELEVAGE_PR.clear();
		_donnees_PR_CULTURE.clear();

		if(_donneesGPE.size() != 0 && _donneesPR.size() != 0)
		{
			//recherche les sites de prelevements associés aux sites gpe (1:1) (un site gpe peut avoir 1 site de prelevement qui lui est associé)
			map<int, map<int, map<string, string>>>::const_iterator itGpe;
			map<int, map<string, string>>::const_iterator itGpeSite;
			map<string, string>::const_iterator itGpeSiteAnnee;

			//pour chaque troncon ayant au moins 1 site gpe
			for(itGpe=_donneesGPE.begin(); itGpe!=_donneesGPE.end(); itGpe++)
			{
				//pour chaque site gpe du troncon
				for(itGpeSite=itGpe->second.begin(); itGpeSite!=itGpe->second.end(); itGpeSite++)
				{
					itGpeSiteAnnee = itGpeSite->second.begin();

					dLat = -1.0;
									
					sNom = GetGpeStr(itGpeSiteAnnee->second, "NOM LEGAL DU LIEU OU PERSONNE PHYSIQUE");
					sNom = TrimString(sNom);
					sNomO = sNom;
					boost::algorithm::to_upper(sNom);	//uppercase

					sNom2 = GetGpeStr(itGpeSiteAnnee->second, "NOM INTERVENANT");
					sNom2 = TrimString(sNom2);
					sNom2O = sNom2;
					boost::algorithm::to_upper(sNom2);	//uppercase

					//pour chaque site PR
					for(itPr=_donneesPRSite.begin(); itPr!=_donneesPRSite.end(); itPr++)
					{
						sNomPr = GetPrStr(itPr->second, "SITE_GPE");
						sNomPr = TrimString(sNomPr);
						boost::algorithm::to_upper(sNomPr);	//uppercase

						if( sNomPr != "" && (sNomPr == sNom || sNomPr == sNom2) )
						{
							if(dLat == -1.0)
							{
								dLat = GetGpeDbl(itGpeSiteAnnee->second, "LAT(D)");
								dLon = GetGpeDbl(itGpeSiteAnnee->second, "LONG(D)");
							}

							dLatPr = GetPrDbl(itPr->second, "SITE_GPE_LAT");
							dLonPr = GetPrDbl(itPr->second, "SITE_GPE_LONG");

							dDistance = sqrt( pow(dLon - dLonPr, 2.0) + pow(dLat - dLatPr, 2.0) );

							if(dDistance <= 0.00001)
							{
								//pour verification association des site PR et GPE
								oss.str("");
								oss << itGpeSite->first << _sim_hyd.PrendreOutput().Separator() << sNomO << _sim_hyd.PrendreOutput().Separator() << sNom2O << _sim_hyd.PrendreOutput().Separator() << itPr->first << _sim_hyd.PrendreOutput().Separator() << setiosflags(ios::fixed) << setprecision(8) << dDistance;
								vAssID.push_back(itGpeSite->first);
								vAssStr.push_back(oss.str());
								//

								_donneesGPE_PR[itGpeSite->first] = itPr->first;
								break;
							}										
						}
					}
				}
			}

			//pour verification association des sites GPE & PR
			sNom = Combine(_sim_hyd.PrendreRepertoireResultat(), "prelevements_association_gpe_pr.csv");
			ofs.open(sNom);
			ofs << "Compteur" << _sim_hyd.PrendreOutput().Separator() << "#SITE GPE" << _sim_hyd.PrendreOutput().Separator() << "Nom legal du lieu ou personne physique" << _sim_hyd.PrendreOutput().Separator() <<  "NOM INTERVENANT" << _sim_hyd.PrendreOutput().Separator() << "#SITE PRELEVEMENT" << _sim_hyd.PrendreOutput().Separator() << "Distance (dd)" << endl;

			j = 1;
			for(itPr=_donneesGPESite.begin(); itPr!=_donneesGPESite.end(); itPr++)
			{
				bFound = false;
				for(i=0; i!=vAssID.size(); i++)
				{
					if(vAssID[i] == itPr->first)
					{
						ofs << j << _sim_hyd.PrendreOutput().Separator() << vAssStr[i] << endl;
						bFound = true;
						++j;
					}
				}

				if(!bFound)
				{
					sNom = GetGpeStr(itPr->second, "NOM LEGAL DU LIEU OU PERSONNE PHYSIQUE");
					sNom = TrimString(sNom);

					sNom2 = GetGpeStr(itPr->second, "NOM INTERVENANT");
					sNom2 = TrimString(sNom2);

					ofs << j << _sim_hyd.PrendreOutput().Separator() << itPr->first << _sim_hyd.PrendreOutput().Separator() << sNom << _sim_hyd.PrendreOutput().Separator() << sNom2 << endl;
					++j;
				}
			}
			ofs.close();
		}

		if(_donneesPR.size() != 0 && _donneesELEVAGE.size() != 0)
		{
			//recherche les sites de prelevements associés aux sites d'élevages (1:1) (un site d'élevage peut avoir 1 site de prelevement qui lui est associé)
			map<int, map<int, string>>::const_iterator itEl;

			vAssID.clear();
			vAssStr.clear();

			//pour chaque troncon ayant au moins 1 site d'élevage
			for(itEl=_donneesELEVAGE.begin(); itEl!=_donneesELEVAGE.end(); itEl++)
			{
				//pour chaque site d'élevage du troncon
				for(itElSite=itEl->second.begin(); itElSite!=itEl->second.end(); itElSite++)
				{
					dLat = -1.0;

					sNom = GetElStr(itElSite->second, "NOM DU LIEU");
					sNom = TrimString(sNom);
					sNomO = sNom;
					boost::algorithm::to_upper(sNom);	//uppercase

					sNom2 = GetElStr(itElSite->second, "NOM DE LEXPLOITANT");
					sNom2 = TrimString(sNom2);
					sNom2O = sNom2;
					boost::algorithm::to_upper(sNom2);	//uppercase

					//pour chaque site PR
					for(itPr=_donneesPRSite.begin(); itPr!=_donneesPRSite.end(); itPr++)
					{
						sNomPr = GetPrStr(itPr->second, "SITE_ELEVAGES");
						sNomPr = TrimString(sNomPr);
						boost::algorithm::to_upper(sNomPr);	//uppercase

						if( sNomPr != "" && (sNomPr == sNom || sNomPr == sNom2) )
						{
							if(dLat == -1.0)
							{
								dLat = GetElDbl(itElSite->second, "LAT(D)");
								dLon = GetElDbl(itElSite->second, "LONG(D)");
							}

							dLatPr = GetPrDbl(itPr->second, "SITE_ELEVAGES_LAT");
							dLonPr = GetPrDbl(itPr->second, "SITE_ELEVAGES_LONG");

							dDistance = sqrt( pow(dLon - dLonPr, 2.0) + pow(dLat - dLatPr, 2.0) );

							if(dDistance <= 0.00001)
							{
								//pour verification association des site PR et ELEVAGE
								oss.str("");
								oss << itElSite->first << _sim_hyd.PrendreOutput().Separator() << sNomO << _sim_hyd.PrendreOutput().Separator() << sNom2O << _sim_hyd.PrendreOutput().Separator() << itPr->first << _sim_hyd.PrendreOutput().Separator() << setiosflags(ios::fixed) << setprecision(8) << dDistance;
								vAssID.push_back(itElSite->first);
								vAssStr.push_back(oss.str());
								//

								_donneesELEVAGE_PR[itElSite->first] = itPr->first;
								break;
							}										
						}
					}
				}
			}
			
			//pour verification association des sites ELEVAGE & PR
			sNom = Combine(_sim_hyd.PrendreRepertoireResultat(), "prelevements_association_elevage_pr.csv");
			ofs.open(sNom);
			ofs << "#SITE ELEVAGE" << _sim_hyd.PrendreOutput().Separator() << "NOM DU LIEU" << _sim_hyd.PrendreOutput().Separator() <<  "NOM DE LEXPLOITANT" << _sim_hyd.PrendreOutput().Separator() << "#SITE PRELEVEMENT" << _sim_hyd.PrendreOutput().Separator() << "Distance (dd)" << endl;

			for(itPr=_donneesELEVAGESite.begin(); itPr!=_donneesELEVAGESite.end(); itPr++)
			{
				for(i=0; i!=vAssID.size(); i++)
				{
					if(vAssID[i] == itPr->first)
						ofs << vAssStr[i] << endl;
				}
			}
			ofs.close();
		}

		if(_donneesPR.size() != 0 && _donneesCULTURESite.size() != 0)
		{
			//recherche les sites de cultures associé à chaque site de prélevement (N:1) (un site de prélevement peut avoir plusieurs sites de culture qui lui est associé)
			for(itPr=_donneesPRSite.begin(); itPr!=_donneesPRSite.end(); itPr++)	//pour chaque site PR
			{
				vector<int> vIds;
				_donnees_PR_CULTURE[itPr->first] = vIds;

				sNomPr = GetPrStr(itPr->second, "CLE_COMPOS");
				sNomPr = TrimString(sNomPr);
				boost::algorithm::to_upper(sNomPr);	//uppercase

				for(itElSite=_donneesCULTURESite.begin(); itElSite!=_donneesCULTURESite.end(); itElSite++)	//pour chaque site de culture
				{
					sNom = GetCuStr(itElSite->second, "SITE_PREL");
					sNom = TrimString(sNom);
					boost::algorithm::to_upper(sNom);	//uppercase

					if(sNom == sNomPr)
						_donnees_PR_CULTURE[itPr->first].push_back(itElSite->first);
				}
			}			
		}

		//vérification des association GPE/PR et ELEVAGE/PR
		for(itPr=_donneesPRSite.begin(); itPr!=_donneesPRSite.end(); itPr++)	//pour chaque site PR
		{
			sNom = GetPrStr(itPr->second, "SITE_GPE");
			sNom = TrimString(sNom);
			if(sNom != "")
			{
				bFound = false;
				for(itAss=_donneesGPE_PR.begin(); itAss!=_donneesGPE_PR.end(); itAss++)
				{
					if(itAss->second == itPr->first)
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
				{
					oss.str("");
					oss << "erreur: impossible de trouver le site gpe associe au site de prelevement #" << itPr->first << " (SITE_GPE=`" << sNom << "`)";
					vErr.push_back(oss.str());
				}
			}

			sNom = GetPrStr(itPr->second, "SITE_ELEVAGES");
			sNom = TrimString(sNom);
			if(sNom != "")
			{
				bFound = false;
				for(itAss=_donneesELEVAGE_PR.begin(); itAss!=_donneesELEVAGE_PR.end(); itAss++)
				{
					if(itAss->second == itPr->first)
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
				{
					oss.str("");
					oss << "erreur: impossible de trouver le site elevage associe au site de prelevement #" << itPr->first << " (SITE_ELEVAGES=`" << sNom << "`)";
					vErr.push_back(oss.str());
				}
			}
		}

		//vérification des association CULTURE/PR
		for(itElSite=_donneesCULTURESite.begin(); itElSite!=_donneesCULTURESite.end(); itElSite++)	//pour chaque site de culture
		{
			sNom = GetCuStr(itElSite->second, "SITE_PREL");
			sNom = TrimString(sNom);
			if(sNom != "")
			{
				bFound = false;
				for(itPrCu=_donnees_PR_CULTURE.begin(); itPrCu!=_donnees_PR_CULTURE.end(); itPrCu++)
				{
					for(i=0; i!=itPrCu->second.size(); i++)
					{
						if(itPrCu->second[i] == itElSite->first)
						{
							bFound = true;
							break;
						}
					}
					if(bFound)
						break;
				}
				if(!bFound)
				{
					oss.str("");
					oss << "erreur: impossible de trouver le site de prelevement associe au site de culture #" << itElSite->first << " (SITE_PREL=`" << sNom << "`)";
					vErr.push_back(oss.str());
				}
			}
		}

		if(vErr.size() != 0)
		{
			std::cout << endl;
			for(i=0; i!=vErr.size(); i++)
				std::cout << vErr[i] << endl;

			return false;
		}

		//
		if(_donneesGPE.size() != 0 || _donneesPR.size() != 0 || _donneesEFFLUENT.size() != 0) //on ne met pas || _donneesCULTURESite.size() != 0 || _donneesELEVAGE.size() != 0 : les prelevements des sites de culture et des sites d'élevage sont effectué avec les sites de prélevement. Si pas de site Pr pas de prélevement culture/elevage...
			_bSimulePrelevements = true;

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevementsTYPES()
	{
		vector<string> sList;
		fstream file;
		string str, str2, sPathFile;
		bool bAllRedCoeffDisabled;

		//m_sError = "";
		
		bAllRedCoeffDisabled = true;

		_donneesTYPE.clear();
		_donneesTYPE_redCoeff.clear();

		sPathFile = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "TypePrelevement.csv";
		if(boost::filesystem::exists(boost::filesystem::path(sPathFile)))
		{
			file.open(sPathFile.c_str(), ios::in);
			if(file.fail())
			{
				//m_sError = "Erreur lors de l`ouverture du fichier.";
				return false;
			}

			//entete
			getline_mod(file, str);

			if(file.fail())
			{
				std::cout << "erreur LecturePrelevementsTYPES: le fichier est vide"<< endl << endl;
				return false;
			}

			getline_mod(file, str);	//1e ligne de données
			if(!file.fail())
			{
				while(!file.fail())
				{
					if(!str.empty())
					{
						try{
						SplitString(sList, str, ";", true, false);
						if(sList.size() != 2 && sList.size() != 3)
						{
							file.close();
							std::cout << "erreur LecturePrelevementsTYPES: nombre de colonne invalide. Le fichier doit avoir les colonnes suivantes: Type;Inclu dans la simulation (0 ou 1);Coefficient Reduction (0 ou 1)" << endl << endl;
							return false;
						}

						str = sList[0];
						str = TrimString(str);
						if(str == "")
						{
							file.close();
							std::cout << "erreur LecturePrelevementsTYPES: type non valide (valeur vide)" << endl << endl;
							return false;
						}
						boost::algorithm::to_upper(str);	//uppercase

						if(_donneesTYPE.count(str) != 0)
						{
							file.close();
							std::cout << "erreur LecturePrelevementsTYPES: le type " << str << " existe plus d`une fois" << endl << endl;
							return false;
						}

						str2 = sList[1];
						str2 = TrimString(str2);
						if(str2 == "0")
							_donneesTYPE[str] = 0;
						else
						{
							if(str2 == "1")
								_donneesTYPE[str] = 1;
							else
							{							
								file.close();
								std::cout << "erreur LecturePrelevementsTYPES: colonne 2 non valide (doit être 0 ou 1)" << endl << endl;
								return false;
							}
						}

						if(sList.size() == 3)
						{
							//Colonne Coefficient Reduction (0 ou 1)
							str2 = sList[2];
							str2 = TrimString(str2);
							if(str2 == "0")
								_donneesTYPE_redCoeff[str] = 0;
							else
							{
								if(str2 == "1")
								{
									_donneesTYPE_redCoeff[str] = 1;
									bAllRedCoeffDisabled = false;
								}
								else
								{							
									file.close();
									std::cout << "erreur LecturePrelevementsTYPES: colonne 3 non valide (doit être 0 ou 1)" << endl << endl;
									return false;
								}
							}
						}

						}
						catch(const ERREUR& err)
						{
							throw err;
						}
						catch(...)
						{
							file.close();
							std::cout << "erreur LecturePrelevementsTYPES: exception lors de la lecture du fichier" << endl << endl;
							return false;
						}
					}

					getline_mod(file, str);
				}
			}

			file.close();

			if(_donneesTYPE_redCoeff.size() != 0 && bAllRedCoeffDisabled)
				_donneesTYPE_redCoeff.clear();	//empty the vector if all readed type are disabled
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevementsREDCOEFF()
	{
		vector<string> sList;
		istringstream iss;
		fstream file;
		double dVal;
		string str, str2, sPathFile;
		bool bValide;
		int iVal;

		//m_sError = "";

		sPathFile = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "COEFFICIENT_REDUCTION.csv";
		if(boost::filesystem::exists(boost::filesystem::path(sPathFile)))
		{
			file.open(sPathFile.c_str(), ios::in);
			if(file.fail())
			{
				//m_sError = "Erreur lors de l`ouverture du fichier.";
				return false;
			}

			//entete
			getline_mod(file, str);

			if(file.fail())
			{
				std::cout << "erreur lecture: " << sPathFile << ": le fichier est vide" << endl << endl;
				return false;
			}

			getline_mod(file, str);	//1e ligne de données
			if(!file.fail())
			{
				while(!file.fail())
				{
					if(!str.empty())
					{
						try{
						SplitString(sList, str, ";", true, true);
						if(sList.size() != 2)
						{
							file.close();
							std::cout << "erreur lecture: " << sPathFile << ": nombre de colonne invalide. Le fichier doit avoir les colonnes suivantes: Date(JJ/MM/AAAA);Reduction (0:1)" << endl << endl;
							return false;
						}

						//validation colonne 1 Date(JJ/MM/AAAA)
						str = sList[0];
						str = TrimString(str);
						if(str == "")
						{
							file.close();
							std::cout << "erreur lecture: " << sPathFile << ": la valeur pour la colonne Date(JJ/MM/AAAA) est vide" << endl << endl;
							return false;
						}
						if(str.length() != 10)
						{
							file.close();
							std::cout << "erreur lecture: " << sPathFile << ": la valeur pour la colonne Date(JJ/MM/AAAA) est invalide: " << str << endl << endl;
							return false;
						}

						bValide = false;

						try{
						iss.clear();
						iss.str(str.substr(0, 2));	//JJ
						iss >> iVal;
						if(iVal >= 1 && iVal <= 31)
						{
							iss.clear();
							iss.str(str.substr(3, 2));	//MM
							iss >> iVal;
							if(iVal >= 1 && iVal <= 12)
							{
								iss.clear();
								iss.str(str.substr(6, 4));	//AAAA
								iss >> iVal;
								if(iVal >= 1000)
									bValide = true;
								else
									std::cout << "erreur lecture: " << sPathFile << ": la valeur pour la colonne Date(JJ/MM/AAAA) est invalide: l`annee est invalide: " << str << endl << endl;
							}
							else
								std::cout << "erreur lecture: " << sPathFile << ": la valeur pour la colonne Date(JJ/MM/AAAA) est invalide: le mois est invalide: " << str << endl << endl;
						}
						else
							std::cout << "erreur lecture: " << sPathFile << ": la valeur pour la colonne Date(JJ/MM/AAAA) est invalide: le jour est invalide: " << str << endl << endl;
						}
						catch(...)
						{
							file.close();
							std::cout << "erreur lecture (exception): " << sPathFile << ": la valeur pour la colonne Date(JJ/MM/AAAA) est invalide: " << str << endl << endl;
							return false;
						}

						if(!bValide)
						{
							file.close();
							return false;
						}

						str2 = str.substr(6, 4);	//AAAA
						str2+= str.substr(3, 2);	//MM
						str2+= str.substr(0, 2);	//JJ

						//colonne 2, coefficient de réduction (0:1)
						str = sList[1];
						str = TrimString(str);
						if(str == "")
						{
							file.close();
							std::cout << "erreur lecture: " << sPathFile << ": la valeur pour la colonne Reduction (0:1) est vide" << endl << endl;
							return false;
						}

						try{
						iss.clear();
						iss.str(str);
						iss >> dVal;
						}
						catch(...)
						{
							file.close();
							std::cout << "erreur lecture (exception): " << sPathFile << ": la valeur pour la colonne Reduction (0:1) est invalide: " << str << endl << endl;
							return false;
						}

						if(dVal < 0.0 || dVal > 1.0)
						{
							file.close();
							std::cout << "erreur lecture: " << sPathFile << ": la valeur pour la colonne Reduction (0:1) est invalide: " << str << endl << endl;
							return false;
						}

						_donneesREDCOEFF[str2] = dVal;

						}
						catch(const ERREUR& err)
						{
							throw err;
						}
						catch(...)
						{
							file.close();
							std::cout << "erreur lecture: " << sPathFile << ": exception lors de la lecture du fichier" << endl << endl;
							return false;
						}
					}

					getline_mod(file, str);
				}
			}

			file.close();
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevementsGPE()
	{
		string ColNbJour = "NBJ M";

		istringstream iss;
		ostringstream oss;
		unsigned int uiSeed;
		fstream file;
		double dval;
		string str, sPathFile, sYear, ColNbJourTemp;
		size_t NbJourTemp;
		int idTroncon, idSite, iYear, x, y, NbJourMois, iNbJourTemp;

		//m_sError = "";

		_donneesGPE.clear();
		_donneesGPE_AnneeMax.clear();
		_donneesGPE_Jours.clear();
		_donneesGPESite.clear();

		sPathFile = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_GPE.csv";
		if(boost::filesystem::exists(boost::filesystem::path(sPathFile)))
		{
			file.open(sPathFile.c_str(), ios::in);
			if(file.fail())
			{
				//m_sError = "Erreur lors de l`ouverture du fichier.";
				return false;
			}

			//entete
			getline_mod(file, str);
			_GpeHeader = str;

			if(file.fail())
			{
				//le fichier est vide
				file.close();
				return true;

				//m_sError = "Erreur lors de la lecture du fichier: le fichier est vide.";
				//return false;
			}

			getline_mod(file, str);	//1e ligne de données
			if(!file.fail())
			{
				while(!file.fail())
				{
					if(!str.empty())
					{
						try{

						dval = GetGpeDbl(str, "ID TRONCON");
						idTroncon = static_cast<int>(dval);

						dval = GetGpeDbl(str, "#SITE GPE");
						idSite = static_cast<int>(dval);

						sYear = TrimString(GetGpeStr(str, "ANNEE"));
						boost::algorithm::to_upper(sYear);	//uppercase

						if(sYear != "MOY")
						{
							iss.clear();
							iss.str(sYear);
							iss >> iYear;
						}
						else
							iYear = -1;

						if(_donneesGPESite.count(idSite) == 0)
							_donneesGPESite[idSite] = str;

						if(_donneesGPE.count(idTroncon) == 0 || _donneesGPE[idTroncon].count(idSite) == 0)
						{
							_donneesGPE[idTroncon][idSite][sYear] = str;
							_donneesGPE_AnneeMax[idTroncon][idSite] = iYear;
						}
						else
						{
							if(_donneesGPE[idTroncon][idSite].count(sYear) == 0)
							{
								_donneesGPE[idTroncon][idSite][sYear] = str;
								
								if(iYear != -1)
								{
									if(_donneesGPE_AnneeMax[idTroncon][idSite] == -1 || iYear > _donneesGPE_AnneeMax[idTroncon][idSite])
										_donneesGPE_AnneeMax[idTroncon][idSite] = iYear;
								}
							}
							else
							{
								file.close();
								std::cout << "erreur LecturePrelevementsGPE: doublon #SITE GPE " << idSite << endl << endl;
								return false;
							}
						}

						}
						catch(const ERREUR& err)
						{
							throw err;
						}
						catch(...)
						{
							file.close();
							//m_sError = "Erreur lors de la lecture du fichier: format invalide";
							return false;
						}
					}

					getline_mod(file, str);
				}
			}

			file.close();

			//determine les jours de prelevements de chaque site gpe pour chaque annees de simulation et chaque mois
			std::map<int, std::map<int, std::map<std::string, std::string>>>::const_iterator itGpe;
			std::map<int, std::map<std::string, std::string>>::const_iterator itGpeSite;
			double dVal1, dVal2;
			vector<int> vJours;
			int startYear, endYear, iVal;

			NbJourMois = 0;

			startYear = _sim_hyd.PrendreDateDebut().PrendreAnnee();
			endYear = _sim_hyd.PrendreDateFin().PrendreAnnee();

			for(itGpe=_donneesGPE.begin(); itGpe!=_donneesGPE.end(); itGpe++)	//pour chaque troncon ayant au moins 1 site gpe
			{
				//pour chaque site gpe du troncon
				for(itGpeSite=itGpe->second.begin(); itGpeSite!=itGpe->second.end(); itGpeSite++)
				{
					dVal1 = abs(GetGpeDbl(itGpeSite->second.begin()->second, "LAT(D)"));
					dVal2 = abs(GetGpeDbl(itGpeSite->second.begin()->second, "LONG(D)"));
					dVal1*= 100000.0;
					dVal2*= 100000.0;
					iVal = static_cast<int>(dVal1+dVal2);	//genere un seed pour le site gpe selon les coordonnees

					for(iYear=startYear; iYear!=endYear+1; iYear++)
					{
						oss.str("");
						oss << iYear;

						if(itGpeSite->second.count(oss.str()) != 0)	//si l'annee est presente
							sYear = oss.str();
						else
							sYear = "MOY";

						for(x=1; x!=13; x++)	//pour chaque mois
						{
							oss.str("");
							oss << x;
							ColNbJourTemp = ColNbJour + oss.str();

							if(x == 2)
							{
								DATE_HEURE dt(static_cast<unsigned short>(iYear), 1, 1, 0);
								if(dt.Bissextile())
									NbJourMois = 29;
								else
									NbJourMois = 28;
							}
							else
							{
								if(x == 4 || x == 6 || x == 9 || x == 11)
									NbJourMois = 30;
								else
									NbJourMois = 31;
							}

							vJours.clear();

							iNbJourTemp = static_cast<int>(GetGpeDbl(itGpeSite->second.at(sYear), ColNbJourTemp));

							if(iNbJourTemp < NbJourMois)
							{
								if(NbJourMois == 29 && iNbJourTemp == 28 && sYear == "MOY")	//iNbJourTemp == 28 lorsque annees == moy dans les fichiers sources...
								{
									//tous les jours du mois seront effectués
									for(y=1; y!=32; y++)
										vJours.push_back(y);
								}
								else
								{
									if(iNbJourTemp != 0)
									{
										uiSeed = iYear * 100 + x + iVal;
										NbJourTemp = static_cast<size_t>(iNbJourTemp);
										_sim_hyd._pr->GenerateAlea(NbJourTemp, NbJourMois, uiSeed, vJours);
									}
								}
							}
							else
							{
								//tous les jours du mois seront effectués
								for(y=1; y!=32; y++)
									vJours.push_back(y);
							}
							
							_donneesGPE_Jours[itGpe->first][itGpeSite->first][iYear][x] = vJours;
						}
					}
				}
			}
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevementsPR()	//BD_PRELEVEMENTS.csv
	{
		fstream file;
		string str, sPathFile;
		int idTroncon, idSite;

		//m_sError = "";

		_donneesPR.clear();
		_donneesPRSite.clear();

		sPathFile = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_PRELEVEMENTS.csv";
		if(boost::filesystem::exists(boost::filesystem::path(sPathFile)))
		{
			file.open(sPathFile.c_str(), ios::in);
			if(file.fail())
			{
				//m_sError = "Erreur lors de l`ouverture du fichier.";
				return false;
			}

			//entete
			getline_mod(file, str);
			_PrHeader = str;

			if(file.fail())
			{
				//le fichier est vide
				file.close();
				return true;

				//m_sError = "Erreur lors de la lecture du fichier: le fichier est vide.";
				//return false;
			}

			getline_mod(file, str);	//1e ligne de données
			if(!file.fail())
			{
				while(!file.fail())
				{
					if(!str.empty())
					{
						try{

						idTroncon = GetPrInt(str, "IDTRONCON");
						idSite = GetPrInt(str, "#SITE");

						//conserve les donnees PR par idTroncon/idSite
						if(_donneesPR.count(idTroncon) == 0)
							_donneesPR[idTroncon][idSite] = str;
						else
						{
							if(_donneesPR[idTroncon].count(idSite) == 0)
								_donneesPR[idTroncon][idSite] = str;
							else
							{
								file.close();
								std::cout << "erreur LecturePrelevementsPR: doublon #SITE " << idSite << endl << endl;
								return false;
							}
						}

						//conserve également les donnees PR par idSite
						if(_donneesPRSite.count(idSite) == 0)
							_donneesPRSite[idSite] = str;
						else
						{
							file.close();
							std::cout << "erreur LecturePrelevementsPR (_donneesPRSite): doublon #SITE " << idSite << endl << endl;
							return false;
						}

						}
						catch(const ERREUR& err)
						{
							throw err;
						}
						catch(...)
						{
							file.close();
							//m_sError = "Erreur lors de la lecture du fichier: format invalide";
							return false;
						}
					}

					getline_mod(file, str);
				}
			}

			file.close();
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevementsELEVAGE()
	{
		fstream file;
		double dval;
		string str, sPathFile;
		int idTroncon, idSite;

		//m_sError = "";

		_donneesELEVAGE.clear();
		_donneesELEVAGESite.clear();

		sPathFile = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_ELEVAGES.csv";
		if(boost::filesystem::exists(boost::filesystem::path(sPathFile)))
		{
			file.open(sPathFile.c_str(), ios::in);
			if(file.fail())
			{
				//m_sError = "Erreur lors de l`ouverture du fichier.";
				return false;
			}

			//entete
			getline_mod(file, str);
			_ElHeader = str;

			if(file.fail())
			{
				//le fichier est vide
				file.close();
				return true;

				//m_sError = "Erreur lors de la lecture du fichier: le fichier est vide.";
				//return false;
			}

			getline_mod(file, str);	//1e ligne de données
			if(!file.fail())
			{
				while(!file.fail())
				{
					if(!str.empty())
					{
						try{

						dval = GetElDbl(str, "ID TRONCON");
						idTroncon = static_cast<int>(dval);

						dval = GetElDbl(str, "#SITE");
						idSite = static_cast<int>(dval);

						if(_donneesELEVAGESite.count(idSite) == 0)
							_donneesELEVAGESite[idSite] = str;

						if(_donneesELEVAGE.count(idTroncon) == 0)
							_donneesELEVAGE[idTroncon][idSite] = str;
						else
						{
							if(_donneesELEVAGE[idTroncon].count(idSite) == 0)
								_donneesELEVAGE[idTroncon][idSite] = str;
							else
							{
								file.close();
								std::cout << "erreur LecturePrelevementsELEVAGE: doublon #SITE " << idSite << endl << endl;
								return false;
							}
						}

						}
						catch(const ERREUR& err)
						{
							throw err;
						}
						catch(...)
						{
							file.close();
							//m_sError = "Erreur lors de la lecture du fichier: format invalide";
							return false;
						}
					}

					getline_mod(file, str);
				}
			}

			file.close();
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevementsEFFLUENT()
	{
		fstream file;
		double dval;
		string str, sPathFile;
		int idTroncon, idSite;

		//m_sError = "";

		_donneesEFFLUENT.clear();

		sPathFile = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_EFFLUENTS.csv";
		if(boost::filesystem::exists(boost::filesystem::path(sPathFile)))
		{
			file.open(sPathFile.c_str(), ios::in);
			if(file.fail())
			{
				//m_sError = "Erreur lors de l`ouverture du fichier.";
				return false;
			}

			//entete
			getline_mod(file, str);
			_EfHeader = str;

			if(file.fail())
			{
				//le fichier est vide
				file.close();
				return true;

				//m_sError = "Erreur lors de la lecture du fichier: le fichier est vide.";
				//return false;
			}

			getline_mod(file, str);	//1e ligne de données
			if(!file.fail())
			{
				while(!file.fail())
				{
					if(!str.empty())
					{
						try{

						dval = GetEfDbl(str, "IDTRONCON");
						idTroncon = static_cast<int>(dval);

						dval = GetEfDbl(str, "#SITE");
						idSite = static_cast<int>(dval);

						if(_donneesEFFLUENT.count(idTroncon) == 0)
							_donneesEFFLUENT[idTroncon][idSite] = str;
						else
						{
							if(_donneesEFFLUENT[idTroncon].count(idSite) == 0)
								_donneesEFFLUENT[idTroncon][idSite] = str;
							else
							{
								file.close();
								std::cout << "erreur LecturePrelevementsEFFLUENTS: doublon #SITE " << idSite << endl << endl;
								return false;
							}
						}

						}
						catch(const ERREUR& err)
						{
							throw err;
						}
						catch(...)
						{
							file.close();
							//m_sError = "Erreur lors de la lecture du fichier: format invalide";
							return false;
						}
					}

					getline_mod(file, str);
				}
			}

			file.close();
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::LecturePrelevementsCULTURE()
	{
		fstream file;
		double dval;
		string str, sPathFile;
		int idSite;

		//m_sError = "";

		_donneesCULTURESite.clear();

		sPathFile = _sim_hyd.PrendreRepertoireProjet() + "/" + _FolderName + "/" + "BD_CULTURES.csv";
		if(boost::filesystem::exists(boost::filesystem::path(sPathFile)))
		{
			file.open(sPathFile.c_str(), ios::in);
			if(file.fail())
			{
				//m_sError = "Erreur lors de l`ouverture du fichier.";
				return false;
			}

			//entete
			getline_mod(file, str);
			_CuHeader = str;

			if(file.fail())
			{
				//le fichier est vide
				file.close();
				return true;

				//m_sError = "Erreur lors de la lecture du fichier: le fichier est vide.";
				//return false;
			}

			getline_mod(file, str);	//1e ligne de données
			if(!file.fail())
			{
				while(!file.fail())
				{
					if(!str.empty())
					{
						try{

						dval = GetCuDbl(str, "ID");
						idSite = static_cast<int>(dval);

						if(_donneesCULTURESite.count(idSite) == 0)
							_donneesCULTURESite[idSite] = str;
						else
						{
							file.close();
							std::cout << "erreur LecturePrelevementsCULTURE: doublon ID " << idSite << endl << endl;
							return false;
						}

						}
						catch(const ERREUR& err)
						{
							throw err;
						}
						catch(...)
						{
							file.close();
							//m_sError = "Erreur lors de la lecture du fichier: format invalide";
							return false;
						}
					}

					getline_mod(file, str);
				}
			}

			file.close();
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//calcule les prelevements et rejets à effectuer pour le pas de temps courant (pour tous les troncons)
	//
	//Si le fichier TypePrelevement.csv est présent, le type doit être à 1 pour que le prélèvement soit effectué.
	//Si le type est absent du fichier (le fichier est présent et contient d'autre type), le prélèvement est effectué.
	//
	bool PRELEVEMENTS::CalculePrelevements()
	{
		string ColNbJour = "NBJ M";
		string ColVol = "VOL M";
		string ColCons = "CONS M";

		DATE_HEURE dt;
		size_t index, i, j;
		size_t indexTroncon;
		ostringstream oss, ossAnnee, ossDateStr;
		string ColNbJourTemp, ColVolTemp, ColConsTemp, sAnnee, str1, str2, sType;
		double dNbJour, dVal1, dVal2, dPrelevementSite, dRejetSite, dCoeff, dCoeffApplicable;
		bool OnProcede, OnRejete, bTypeOk;
		int moisCourant, anneeCourant, jourCourant, idTroncon, idUhrh, iNbJourAnneeCourant, iMoisDebut, iMoisFin;

		map<int, map<int, map<string, string>>>::const_iterator itGpe;
		map<int, map<string, string>>::const_iterator itGpeSite;
		map<int, map<int, string>>::const_iterator itTroncon;
		map<int, string>::const_iterator itSite;

		vector<size_t> index_troncons = _sim_hyd.PrendreTronconsSimules();
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();
		ZONES& zones = _sim_hyd.PrendreZones();
		TRONCON* troncon;

		try{
		
		dt = _sim_hyd.PrendreDateCourante();
		jourCourant = dt.PrendreJour();
		moisCourant = dt.PrendreMois();
		anneeCourant = dt.PrendreAnnee();

		dCoeffApplicable = 1.0;
		if(_donneesTYPE_redCoeff.size() != 0)	//only if the reduction coefficient is enabled at least for one type
		{
			ossDateStr.str("");
			ossDateStr << std::setfill('0') << std::setw(4) << anneeCourant << std::setw(2) << moisCourant << std::setw(2) << jourCourant;

			if(_donneesREDCOEFF.count(ossDateStr.str()) != 0)
				dCoeffApplicable-= _donneesREDCOEFF[ossDateStr.str()];
		}

		if(dt.Bissextile())
			iNbJourAnneeCourant = 366;
		else
			iNbJourAnneeCourant = 365;

		if(_donneesGPE.size() != 0)
		{
			oss.str("");
			oss << moisCourant;

			ColNbJourTemp = ColNbJour + oss.str();	//pour prelevements GPE
			ColVolTemp = ColVol + oss.str();		//
			ColConsTemp = ColCons + oss.str();		//
		}

		ossAnnee << anneeCourant;

		_tronconsPrelevementString.clear();
		_tronconsPrelevementVal.clear();
		_tronconsPrelevementCultureString.clear();
		_tronconsPrelevementCultureVal.clear();
		_tronconsRejetString.clear();
		_tronconsRejetVal.clear();
		_tronconsRejetEffluentString.clear();
		_tronconsRejetEffluentVal.clear();

		//boucle sur les troncons simulés
		for(index=0; index!=index_troncons.size(); index++)
		{
			indexTroncon = index_troncons[index];			
			troncon = troncons[indexTroncon];

			idTroncon = troncon->PrendreIdent();

			troncon->_prPrelevementTotal = 0.0;
			troncon->_prPrelevementCulture = 0.0;
			troncon->_prRejetTotal = 0.0;
			troncon->_prRejetEffluent = 0.0;

			//Sites GPE

			itGpe = _donneesGPE.find(idTroncon);
			if(itGpe != _donneesGPE.end())
			{
				//pour chaque site gpe associé au troncon courant
				for(itGpeSite=itGpe->second.begin(); itGpeSite!=itGpe->second.end(); itGpeSite++)
				{
					OnProcede = false;
					dCoeff = 1.0;

					bTypeOk = true;
					if(_donneesTYPE.size() != 0)	//si le fichier TypePrelevement.csv est présent
					{
						if(_donneesGPE_PR.count(itGpeSite->first) != 0)		//s'il y a un site de prelevement associé au site gpe
						{
							str1 = TrimString(GetPrStr(_donneesPRSite[_donneesGPE_PR[itGpeSite->first]], "TYPE"));
							boost::algorithm::to_upper(str1);	//uppercase

							if(_donneesTYPE.count(str1) != 0 && _donneesTYPE[str1] == 0)
								bTypeOk = false;
							else
							{
								//coefficient de réduction
								if(_donneesTYPE_redCoeff.size() != 0 && _donneesTYPE_redCoeff.count(str1) != 0 && _donneesTYPE_redCoeff[str1] == 1)
									dCoeff = dCoeffApplicable;
							}
						}
					}

					if(bTypeOk)
					{
						if(itGpeSite->second.count(ossAnnee.str()) != 0)	//si l'annee courante est presente
							sAnnee = ossAnnee.str();
						else
							sAnnee = "MOY";

						str1 = TrimString(GetGpeStr(itGpeSite->second.at(sAnnee), "SOURCE"));
						boost::algorithm::to_upper(str1);	//uppercase
					
						if(str1 == "SURFACE" && TrimString(GetGpeStr(itGpeSite->second.at(sAnnee), "CRITERE")) == "1")
						{
							//Verifie si c'est un jour de prelevement
							//Les jours de prélèvements ont été préalablement déterminés pour toute les années de simulation (fonction LecturePrelevements)
							//avec aléatoire lorsque NbJ MX est < que le nb de jour du mois

							if(std::find(_donneesGPE_Jours[idTroncon][itGpeSite->first][anneeCourant][moisCourant].begin(), _donneesGPE_Jours[idTroncon][itGpeSite->first][anneeCourant][moisCourant].end(), jourCourant) != _donneesGPE_Jours[idTroncon][itGpeSite->first][anneeCourant][moisCourant].end())
							{
								OnProcede = true;

								if(sAnnee == "MOY")
								{
									if(anneeCourant > _donneesGPE_AnneeMax[idTroncon][itGpeSite->first])
									{
										//s'il y a un site de prelevement associé au site gpe
										if(_donneesGPE_PR.count(itGpeSite->first) != 0)
										{
											//si CRITERE du site PR correspondant égal 0 (désactivé)
											str1 = TrimString(GetPrStr(_donneesPRSite[_donneesGPE_PR[itGpeSite->first]], "CRITERE"));
											if(str1 == "0")
												OnProcede = false;
										}
									}
								}
							}
						}
					}

					if(OnProcede)
					{
						//prelevement
						dNbJour = GetGpeDbl(itGpeSite->second.at(sAnnee), ColNbJourTemp);
						dVal1 = GetGpeDbl(itGpeSite->second.at(sAnnee), ColVolTemp);

						dVal1*= dCoeff;		//coefficient de réduction (la valeur est à 1 si non applicable)
						
						dPrelevementSite = dVal1 / dNbJour;	//l/jour
						dPrelevementSite = dPrelevementSite / 1000.0 / 86400.0;	//m3/s

						if(dPrelevementSite > 0.0)
						{
							troncon->_prPrelevementTotal+= dPrelevementSite;

							oss.str("");
							oss << "GPE;" << itGpeSite->first << ";";

							if(_donneesGPE_PR.count(itGpeSite->first) != 0)		//s'il y a un site de prelevement associé au site gpe
								oss << _donneesGPE_PR[itGpeSite->first];

							oss << ";" << dPrelevementSite;

							_tronconsPrelevementString[indexTroncon].push_back(oss.str());
							_tronconsPrelevementVal[indexTroncon].push_back(dPrelevementSite);
						}

						//rejet
						OnRejete = true;

						//s'il y a un site de prelevement associé au site gpe
						if(_donneesGPE_PR.count(itGpeSite->first) != 0)
						{
							str1 = TrimString(GetPrStr(_donneesPRSite[_donneesGPE_PR[itGpeSite->first]], "CRITERE_CONSOMMATION"));
							if(str1 != "1")
								OnRejete = false;
						}
						
						if(OnRejete)
						{
							dVal2 = GetGpeDbl(itGpeSite->second.at(sAnnee), ColConsTemp);
							dVal2*= dCoeff;		//coefficient de réduction (la valeur est à 1 si non applicable)

							dRejetSite = (dVal1 - dVal2) / dNbJour;			//l/jour
							dRejetSite = dRejetSite / 1000.0 / 86400.0;		//m3/s

							if(dRejetSite > 0.0)
							{
								if(dPrelevementSite > 0.0 && dRejetSite > dPrelevementSite)
									dRejetSite = dPrelevementSite;

								troncon->_prRejetTotal+= dRejetSite;

								oss.str("");
								oss << "GPE;" << itGpeSite->first << ";";

								if(_donneesGPE_PR.count(itGpeSite->first) != 0)		//s'il y a un site de prelevement associé au site gpe
									oss << _donneesGPE_PR[itGpeSite->first];

								oss << ";" << dRejetSite;

								_tronconsRejetString[indexTroncon].push_back(oss.str());
								_tronconsRejetVal[indexTroncon].push_back(dRejetSite);
							}
						}
					}
				}
			}

			//Sites de prélèvements (PR)

			itTroncon = _donneesPR.find(idTroncon);
			if(itTroncon != _donneesPR.end())
			{
				//pour chaque site de prélèvement associé au troncon courant
				for(itSite=itTroncon->second.begin(); itSite!=itTroncon->second.end(); itSite++)
				{
					str1 = TrimString(GetPrStr(itSite->second, "SOURCE"));
					boost::algorithm::to_upper(str1);	//uppercase

					if(str1 == "SURFACE" && TrimString(GetPrStr(itSite->second, "CRITERE")) == "1" && 
						TrimString(GetPrStr(itSite->second, "SITE_GPE")) == "" && TrimString(GetPrStr(itSite->second, "SITE_ELEVAGES")) == "")
					{
						if(TrimString(GetPrStr(itSite->second, "SITE_CULTURES")) == "")
						{
							OnProcede = false;

							//si le TYPE du prélèvement est activé
							str1 = TrimString(GetPrStr(itSite->second, "TYPE"));
							boost::algorithm::to_upper(str1);	//uppercase
							if(_donneesTYPE.count(str1) == 0 || _donneesTYPE[str1] == 1)
							{
								//si dans l'intervalle de temps (mois début et fin)
								iMoisDebut = GetPrInt(itSite->second, "MOIS D");
								iMoisFin = GetPrInt(itSite->second, "MOIS FIN");

								if(iMoisDebut <= iMoisFin && moisCourant >= iMoisDebut && moisCourant <= iMoisFin)
									OnProcede = true;
								else
								{
									if( iMoisDebut > iMoisFin && (moisCourant >= iMoisDebut || moisCourant <= iMoisFin) )
										OnProcede = true;
								}
							}
					
							if(OnProcede)
							{
								//prelevement
								dVal1 = GetPrDbl(itSite->second, "PRELEV (L/J)");

								//coefficient de réduction
								if(_donneesTYPE_redCoeff.size() != 0 && _donneesTYPE_redCoeff.count(str1) != 0 && _donneesTYPE_redCoeff[str1] == 1)
									dVal1*= dCoeffApplicable;

								dPrelevementSite = dVal1 / 1000.0 / 86400.0;	//m3/s

								if(dPrelevementSite > 0.0)
								{
									troncon->_prPrelevementTotal+= dPrelevementSite;

									oss.str("");
									oss << "PR;" << itSite->first << ";" << ";" << dPrelevementSite;
									_tronconsPrelevementString[indexTroncon].push_back(oss.str());
									_tronconsPrelevementVal[indexTroncon].push_back(dPrelevementSite);
								}

								//rejet
								str1 = TrimString(GetPrStr(itSite->second, "CRITERE_CONSOMMATION"));
								if(str1 == "1")
								{
									dVal2 = GetPrDbl(itSite->second, "COEF_CONS");
									dRejetSite = (dVal1 - dVal1 * dVal2) / 1000.0 / 86400.0;		//m3/s
								
									if(dRejetSite > 0.0)
									{
										if(dPrelevementSite > 0.0 && dRejetSite > dPrelevementSite)
											dRejetSite = dPrelevementSite;

										troncon->_prRejetTotal+= dRejetSite;

										oss.str("");
										oss << "PR;" << itSite->first << ";" << ";" << dRejetSite;
										_tronconsRejetString[indexTroncon].push_back(oss.str());
										_tronconsRejetVal[indexTroncon].push_back(dRejetSite);
									}
								}
							}
						}
						else
						{
							if(TrimString(GetPrStr(itSite->second, "SITE_CULTURES")) == "1")
							{
								//si le TYPE du prélèvement est activé
								str1 = TrimString(GetPrStr(itSite->second, "TYPE"));
								boost::algorithm::to_upper(str1);	//uppercase
								if(_donneesTYPE.count(str1) == 0 || _donneesTYPE[str1] == 1)
								{
									for(i=0; i!=_donnees_PR_CULTURE[itSite->first].size(); i++)	//pour chaque site de culture associé au site de prélevement
									{
										//verifie si c'est un jour d'irrigation pour le uhrh du site de culture en cours
										dVal1 = GetCuDbl(_donneesCULTURESite[_donnees_PR_CULTURE[itSite->first][i]], "IDUHRH");
										idUhrh = static_cast<int>(dVal1);
										j = zones._vIdentVersIndex[static_cast<size_t>(abs(idUhrh))];

										if(zones[j]._prJourIrrigation)
										{
											dVal1 = GetCuDbl(_donneesCULTURESite[_donnees_PR_CULTURE[itSite->first][i]], "MAX_7_JOUR");
											dVal2 = GetCuDbl(_donneesCULTURESite[_donnees_PR_CULTURE[itSite->first][i]], "SUP");

											dPrelevementSite = dVal1 / 1000.0 * dVal2 / 7.0 / 86400.0;	//m3/s

											if(dPrelevementSite > 0.0)
											{
												//coefficient de réduction
												if(_donneesTYPE_redCoeff.size() != 0 && _donneesTYPE_redCoeff.count(str1) != 0 && _donneesTYPE_redCoeff[str1] == 1)
													dPrelevementSite*= dCoeffApplicable;

												troncon->_prPrelevementCulture+= dPrelevementSite;

												oss.str("");
												oss << "CULTURE;" << _donnees_PR_CULTURE[itSite->first][i] << ";" << itSite->first << ";" << dPrelevementSite;
												_tronconsPrelevementCultureString[indexTroncon].push_back(oss.str());
												_tronconsPrelevementCultureVal[indexTroncon].push_back(dPrelevementSite);
											}
										}
									}
								}
							}
						}
					}
				}
			}

			//Sites d'élevages

			itTroncon = _donneesELEVAGE.find(idTroncon);
			if(itTroncon != _donneesELEVAGE.end())
			{
				//pour chaque site d'élevage associé au troncon courant
				for(itSite=itTroncon->second.begin(); itSite!=itTroncon->second.end(); itSite++)
				{					
					if(TrimString(GetElStr(itSite->second, "CRITERE")) == "1")
					{
						if(_donneesELEVAGE_PR.count(itSite->first) != 0)		//s'il y a un site de prélèvement associé au site d'élevage
						{
							//si le TYPE du prélèvement est activé
							sType = TrimString(GetPrStr(_donneesPRSite[_donneesELEVAGE_PR[itSite->first]], "TYPE"));
							boost::algorithm::to_upper(sType);	//uppercase
							if(_donneesTYPE.count(sType) == 0 || _donneesTYPE[sType] == 1)
							{
								str1 = TrimString(GetPrStr(_donneesPRSite[_donneesELEVAGE_PR[itSite->first]], "SOURCE"));
								boost::algorithm::to_upper(str1);	//uppercase

								if(str1 == "SURFACE" && TrimString(GetPrStr(_donneesPRSite[_donneesELEVAGE_PR[itSite->first]], "SITE_GPE")) == "")
								{
									dVal1 = GetElDbl(itSite->second, "CONSOMMATION TOTAL M^3/AN");
									dPrelevementSite = dVal1 / iNbJourAnneeCourant / 86400.0;	//m3/s

									if(dPrelevementSite > 0.0)
									{
										//coefficient de réduction
										if(_donneesTYPE_redCoeff.size() != 0 && _donneesTYPE_redCoeff.count(sType) != 0 && _donneesTYPE_redCoeff[sType] == 1)
											dPrelevementSite*= dCoeffApplicable;

										troncon->_prPrelevementTotal+= dPrelevementSite;

										oss.str("");
										oss << "ELEVAGE;" << itSite->first << ";" << _donneesELEVAGE_PR[itSite->first] << ";" << dPrelevementSite;
										_tronconsPrelevementString[indexTroncon].push_back(oss.str());
										_tronconsPrelevementVal[indexTroncon].push_back(dPrelevementSite);
									}
								}
							}
						}
					}
				}
			}

			//Sites effluent
			if(_donneesTYPE.count("EFFLUENT") == 0 || _donneesTYPE["EFFLUENT"] == 1)
			{
				itTroncon = _donneesEFFLUENT.find(idTroncon);
				if(itTroncon != _donneesEFFLUENT.end())
				{
					//pour chaque site effluent associé au troncon courant
					for(itSite=itTroncon->second.begin(); itSite!=itTroncon->second.end(); itSite++)
					{
						str1 = TrimString(GetEfStr(itSite->second, "SOURCE"));
						boost::algorithm::to_upper(str1);	//uppercase
					
						if(str1 == "SURFACE" && TrimString(GetEfStr(itSite->second, "CRITERE")) == "1")
						{
							dVal1 = GetEfDbl(itSite->second, "EFFLUENT (L/J)");
							dRejetSite = dVal1 / 1000.0 / 86400.0;	//m3/s

							if(dRejetSite > 0.0)
							{
								troncon->_prRejetEffluent+= dRejetSite;

								oss.str("");
								oss << "EFFLUENT;" << itSite->first << ";" << ";" << dRejetSite;
								_tronconsRejetEffluentString[indexTroncon].push_back(oss.str());
								_tronconsRejetEffluentVal[indexTroncon].push_back(dRejetSite);
							}
						}
					}
				}
			}
		}

		}
		catch(const ERREUR& err)
		{
			throw err;
		}
		catch(...)
		{
			return false;
		}

		return true;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string PRELEVEMENTS::GetGpeStr(string sLine, string sColNameUpperCase)	//sColName doit etre dejà trimer et en upperccase... (il y a un trim sur la valeur du fichier lors de la comparaison)
	{
		string ret = "";

		map<string, size_t>::iterator it;
		vector<string> sList;
		size_t idx = string::npos;

		it = _GpeMapCol.find(sColNameUpperCase);
		if(it == _GpeMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _GpeHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_GpeMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			ret = sList[idx];
		}
		else
			throw ERREUR("Erreur: prelevements: Gpe (GetGpeStr): colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	double PRELEVEMENTS::GetGpeDbl(string sLine, string sColNameUpperCase)
	{
		double ret = -999.0;

		map<string, size_t>::iterator it;
		vector<string> sList;
		istringstream iss;
		size_t idx = string::npos;

		it = _GpeMapCol.find(sColNameUpperCase);
		if(it == _GpeMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _GpeHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_GpeMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			iss.str(sList[idx]);
			iss >> ret;
		}
		else
			throw ERREUR("Erreur: prelevements: Gpe (GetGpeDbl): colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string PRELEVEMENTS::GetPrStr(string sLine, string sColNameUpperCase)
	{
		string ret = "";

		map<string, size_t>::iterator it;
		vector<string> sList;
		size_t idx = string::npos;

		it = _PrMapCol.find(sColNameUpperCase);
		if(it == _PrMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _PrHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_PrMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			ret = sList[idx];
		}
		else
			throw ERREUR("Erreur: prelevements: Pr: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	double PRELEVEMENTS::GetPrDbl(string sLine, string sColNameUpperCase)
	{
		double ret = -999.0;

		map<string, size_t>::iterator it;
		vector<string> sList;
		istringstream iss;
		size_t idx = string::npos;

		it = _PrMapCol.find(sColNameUpperCase);
		if(it == _PrMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _PrHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_PrMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			iss.str(sList[idx]);
			iss >> ret;
		}
		else
			throw ERREUR("Erreur: prelevements: Pr: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	int PRELEVEMENTS::GetPrInt(string sLine, string sColNameUpperCase)
	{
		int ret = -999;

		map<string, size_t>::iterator it;
		vector<string> sList;
		istringstream iss;
		size_t idx = string::npos;

		it = _PrMapCol.find(sColNameUpperCase);
		if(it == _PrMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _PrHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_PrMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			iss.str(sList[idx]);
			iss >> ret;
		}
		else
			throw ERREUR("Erreur: prelevements: Pr: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string PRELEVEMENTS::GetElStr(string sLine, string sColNameUpperCase)
	{
		string ret = "";

		map<string, size_t>::iterator it;
		vector<string> sList;
		size_t idx = string::npos;

		it = _ElMapCol.find(sColNameUpperCase);
		if(it == _ElMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _ElHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_ElMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			ret = sList[idx];
		}
		else
			throw ERREUR("Erreur: prelevements: Elevage: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	double PRELEVEMENTS::GetElDbl(string sLine, string sColNameUpperCase)
	{
		double ret = -999.0;

		map<string, size_t>::iterator it;
		vector<string> sList;
		istringstream iss;
		size_t idx = string::npos;

		it = _ElMapCol.find(sColNameUpperCase);
		if(it == _ElMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _ElHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_ElMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			iss.str(sList[idx]);
			iss >> ret;
		}
		else
			throw ERREUR("Erreur: prelevements: Elevage: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string PRELEVEMENTS::GetEfStr(string sLine, string sColNameUpperCase)
	{
		string ret = "";

		map<string, size_t>::iterator it;
		vector<string> sList;
		size_t idx = string::npos;

		it = _EfMapCol.find(sColNameUpperCase);
		if(it == _EfMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _EfHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_EfMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			ret = sList[idx];
		}
		else
			throw ERREUR("Erreur: prelevements: Effluent: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	double PRELEVEMENTS::GetEfDbl(string sLine, string sColNameUpperCase)
	{
		double ret = -999.0;

		map<string, size_t>::iterator it;
		vector<string> sList;
		istringstream iss;
		size_t idx = string::npos;

		it = _EfMapCol.find(sColNameUpperCase);
		if(it == _EfMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _EfHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_EfMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			iss.str(sList[idx]);
			iss >> ret;
		}
		else
			throw ERREUR("Erreur: prelevements: Effluent: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	string PRELEVEMENTS::GetCuStr(string sLine, string sColNameUpperCase)
	{
		string ret = "";

		map<string, size_t>::iterator it;
		vector<string> sList;
		size_t idx = string::npos;

		it = _CuMapCol.find(sColNameUpperCase);
		if(it == _CuMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _CuHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_CuMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			ret = sList[idx];
		}
		else
			throw ERREUR("Erreur: prelevements: Culture: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	double PRELEVEMENTS::GetCuDbl(string sLine, string sColNameUpperCase)
	{
		double ret = -999.0;

		map<string, size_t>::iterator it;
		vector<string> sList;
		istringstream iss;
		size_t idx = string::npos;

		it = _CuMapCol.find(sColNameUpperCase);
		if(it == _CuMapCol.end())
		{
			string str;
			size_t i;
			
			SplitString(sList, _CuHeader, ";", false, false);
			for(i=0; i!=sList.size(); i++)
			{
				str = TrimString(sList[i]);
				boost::algorithm::to_upper(str);	//uppercase
				if(str == sColNameUpperCase)
					break;
			}
			if(i != sList.size())
			{
				_CuMapCol[sColNameUpperCase] = i;
				idx = i;
			}
		}
		else
			idx = it->second;

		if(idx != string::npos)
		{
			SplitString(sList, sLine, ";", false, false);
			iss.str(sList[idx]);
			iss >> ret;
		}
		else
			throw ERREUR("Erreur: prelevements: Culture: colonne " + sColNameUpperCase + " introuvable");

		return ret;
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void PRELEVEMENTS::GenerateAlea(size_t nbVal, int iMaxVal, unsigned int uiSeed, vector<int>& vAlea)
	{
		int ival;

		srand(uiSeed);
		ival = static_cast<int>(1.0 + static_cast<double>(rand()) / RAND_MAX * (iMaxVal - 1 + 1));	//skip first value (always == 1) ???

		vAlea.clear();
		while(vAlea.size() != nbVal)
		{
			ival = static_cast<int>(1.0 + static_cast<double>(rand()) / RAND_MAX * (iMaxVal - 1 + 1));
			if(find(begin(vAlea), end(vAlea), ival) == end(vAlea))
				vAlea.push_back(ival);
		}
	}


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	bool PRELEVEMENTS::AddTronconUhrhToFile(string sPathFileIn, string sPathFileOut, size_t indexCoord)
	{
		ostringstream oss;
		vector<string> sList;
		vector<string> sListO;
		vector<string> newFile;
		istringstream is;
		ofstream fileOut;
		fstream file;
		double lon, lat;
		size_t i;
		string sString;
		int iNoDataUhrh, row, col, rowMax, colMax, id_uhrh, id_troncon, x;

		//m_sError = "";

		file.open(sPathFileIn.c_str(), ios::in);
		if(file.fail())
		{
			//m_sError = "Erreur lors de l`ouverture du fichier.";
			return false;
		}

		//entete
		getline_mod(file, sString);
		SplitString(sList, sString, ";", false, false);
		if(sList[sList.size()-1].length() != 0)
		{
			if(sList[sList.size()-1][sList[sList.size()-1].length()-1] == '\r')								//supprime le dernier car. si \r
				sList[sList.size()-1].erase(sList[sList.size()-1].begin()+sList[sList.size()-1].size()-1);	//
		}

		oss.str("");
		oss << sList[0];
		for(i=1; i!=sList.size(); i++)
		{
			if(i == indexCoord+2)
				oss << ";" << "IDTRONCON" << ";" << "IDUHRH";

			oss << ";" << sList[i];
		}
		newFile.push_back(oss.str());

		//
		if(file.fail())
		{
			//le fichier est vide
			file.close();
			return true;

			//m_sError = "Erreur lors de la lecture du fichier: le fichier est vide.";
			//return false;
		}

		const RASTER<int>&	grilleZones = _sim_hyd.PrendreZones().PrendreGrille();
        iNoDataUhrh = grilleZones.PrendreNoData();
		rowMax = static_cast<int>(grilleZones.PrendreNbLigne());
		colMax = static_cast<int>(grilleZones.PrendreNbColonne());

		TRANSFORME_COORDONNEE trans(PROJECTIONS::LONGLAT_WGS84(), grilleZones.PrendreProjection());

		getline_mod(file, sString);	//1e ligne de données
		if(!file.fail())
		{
			while(!file.fail())
			{
				if(!sString.empty())
				{
					try{

					SplitString(sList, sString, ";", false, true);
					if(sList[sList.size()-1].length() != 0)
					{
						if(sList[sList.size()-1][sList[sList.size()-1].length()-1] == '\r')								//supprime le dernier car. si \r
							sList[sList.size()-1].erase(sList[sList.size()-1].begin()+sList[sList.size()-1].size()-1);	//
					}

					SplitString(sListO, sString, ";", false, false);
					if(sListO[sListO.size()-1].length() != 0)
					{
						if(sListO[sListO.size()-1][sListO[sListO.size()-1].length()-1] == '\r')									//supprime le dernier car. si \r
							sListO[sListO.size()-1].erase(sListO[sListO.size()-1].begin()+sListO[sListO.size()-1].size()-1);	//
					}

					is.clear();
					is.str(TrimString(sList[indexCoord]));
					is >> lat;

					is.clear();
					is.str(TrimString(sList[indexCoord+1]));
					is >> lon;

					//obtient troncon et uhrh
					id_uhrh = 0;
					id_troncon = 0;

					grilleZones.CoordonneeVersLigCol(trans.TransformeXY(COORDONNEE(lon, lat)), row, col); // transformer coordonnéee vers wgs84 et ensuite en pixel
					if(row >= 0 && col >= 0 && row < rowMax && col < colMax)
					{
						x = grilleZones(row, col);
						if(x != iNoDataUhrh)
						{
							id_uhrh = x;
							
							i = _sim_hyd.PrendreZones()._vIdentVersIndex[static_cast<size_t>(abs(id_uhrh))];
							id_troncon = _sim_hyd.PrendreZones()[i].PrendreTronconAval()->PrendreIdent();
						}
					}

					//
					oss.str("");
					oss << sListO[0];
					for(i=1; i!=sListO.size(); i++)
					{
						if(i == indexCoord+2)
							oss << ";" << id_troncon << ";" << id_uhrh;

						oss << ";" << sListO[i];
					}

					newFile.push_back(oss.str());

					}
					catch(...)
					{
						file.close();
						//m_sError = "Erreur lors de la lecture du fichier: format invalide";
						return false;
					}
				}

				getline_mod(file, sString);
			}
		}

		file.close();

		//
		try{
		fileOut.open(sPathFileOut, ios_base::trunc);

		if(fileOut.is_open())
		{
			for(i=0; i!=newFile.size(); i++)
				fileOut << newFile[i] << endl;

			fileOut.close();
		}
		else
		{
			//m_sError = "Error creating file: " + sFileName;
			return false;
		}
		}
		//catch(const exception& ex)
		catch(exception&)
		{
			if(fileOut && fileOut.is_open())
				fileOut.close();

			//str = ex.what();
			//m_sError = "Error writing file: exception: " + str + ": " + sFileName;
			return false;
		}

		return true;
	}


	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Code MLB

	//obtient uhrh troncon occ sol et nom d'occ sol avec coordonnée
     bool PRELEVEMENTS::FonctionObtientUhrhTroncon(std::vector<double> vLon, std::vector<double> vLat, std::vector<int>& vidUhrh, std::vector<int>& vidTroncon, std::vector<int>& vidOccsol, std::vector<std::string>& vsOccsol, std::vector<int>& vindex)
        {
        std::size_t index_uhrh;
        int id_uhrh, id_troncons;
        int iNoDataUhrh, iNoDataOcc;
        int ligne, colonne;
        int idOccsol;
        string sOccsol;

        std::size_t sizeV = vLon.size();
        if (sizeV != vLat.size())
            return false;

        const RASTER<int>&	grilleZones = _sim_hyd.PrendreZones().PrendreGrille();
        iNoDataUhrh = grilleZones.PrendreNoData();

        RASTER<int>	grilleOccSol = LectureRaster_int(RemplaceExtension(_sim_hyd.PrendreOccupationSol().PrendreNomFichier(), "tif"));
        iNoDataOcc = grilleOccSol.PrendreNoData();
		int ligneMax = (int) grilleZones.PrendreNbLigne();
		int colMax = (int) grilleZones.PrendreNbColonne();
		int nbrClasse = (int) _sim_hyd.PrendreOccupationSol().PrendreNbClasse();

        TRANSFORME_COORDONNEE trans(PROJECTIONS::LONGLAT_WGS84(), grilleZones.PrendreProjection());

        for (std::size_t i = 0; i < sizeV; i++)
            {
            grilleZones.CoordonneeVersLigCol(trans.TransformeXY(COORDONNEE(vLon[i], vLat[i])), ligne, colonne); // transformer coordonnéee vers wgs84 et ensuite en pixel
			if (ligne >= 0 && colonne >= 0 && ligne < ligneMax && colonne < colMax)
			{
				id_uhrh = grilleZones(ligne, colonne);  // trouve l'id de l'uhrh
				if (id_uhrh != iNoDataUhrh) // si appartient a la carte
				{
					index_uhrh = _sim_hyd.PrendreZones()._vIdentVersIndex[abs(id_uhrh)];  // convertit l'id de l'uhrh en son index
					id_troncons = _sim_hyd.PrendreZones()[index_uhrh].PrendreTronconAval()->PrendreIdent(); // trouve l'id du troncon
					idOccsol = grilleOccSol(ligne, colonne);  // trouve le id de occsol
					if (idOccsol != iNoDataOcc && idOccsol > 1 && idOccsol <= nbrClasse)
					{
						sOccsol = _sim_hyd.PrendreOccupationSol()._classes_occupation_sol[idOccsol - 1].nom; // trouve le nom de la classe occsol  
					}
					else
					{
						idOccsol = -1;
						sOccsol = "N/A";
					}
				}
                else
                    {
                    vindex.push_back((int)i); // la lat et long donnée n'est pas dans la carte, numero de l'index du vector lat et long
                    id_uhrh = -1;
                    id_troncons = -1;
                    idOccsol = -1;
                    sOccsol = "N/A";
                    }
                }
            else
                {
                vindex.push_back((int)i);
                id_uhrh = -1;
                id_troncons = -1;
                idOccsol = -1;
                sOccsol = "N/A";
                }
            vidUhrh.push_back(id_uhrh);
            vidTroncon.push_back(id_troncons);
            vidOccsol.push_back(idOccsol);
            vsOccsol.push_back(sOccsol);
            }
        return true;
        }
	     

     void PRELEVEMENTS::trierDonnePrelevementGPE(const vector<string>& filename, string sOutputFile)
            {
		 //données prévelements 
			vector<int> v_index, v_uhrh, v_troncon, v_idOccsol, v_critere;
			vector<string> v_source, v_sOccsol;
			vector<double> v_lat, v_long, v_coef_consommation;

		 //données spécifiques GPE
            vector<int>  v_annee, v_mois, v_nbrjour, vindex;
			vector<string> v_intervenant, v_codescian, v_nomIntervenant, v_nomLegal;
            vector<double> v_volume, v_consomation;
            

            double dPixelArea;
            double lat_max_nad83, lat_min_nad83, long_max_nad83, long_min_nad83;
            double lat_max, lat_min, long_max, long_min;
            size_t nbColZone, nbRowZone;
            const RASTER<int>&	grilleZones = _sim_hyd.PrendreZones().PrendreGrille();

            TRANSFORME_COORDONNEE trans(grilleZones.PrendreProjection(), PROJECTIONS::LONGLAT_WGS84());
            //valide que les coordonnees superieur gauche des matrices sont identique
            long_min_nad83 = grilleZones.PrendreCoordonnee().PrendreX();
            lat_min_nad83 = grilleZones.PrendreCoordonnee().PrendreY();
            dPixelArea = static_cast<double>(grilleZones.PrendreTailleCelluleX());
            nbColZone = grilleZones.PrendreNbColonne();
            nbRowZone = grilleZones.PrendreNbLigne();
            long_max_nad83 = long_min_nad83 + nbColZone * dPixelArea;
            lat_max_nad83 = lat_min_nad83 - nbRowZone * dPixelArea;

            COORDONNEE coin_sup_gauche = trans.TransformeXY(COORDONNEE(long_min_nad83, lat_min_nad83)); // transformer coordonnéee vers wgs84 et ensuite en pixel
            COORDONNEE coin_inf_droit = trans.TransformeXY(COORDONNEE(long_max_nad83, lat_max_nad83)); // transformer coordonnéee vers wgs84 et ensuite en pixel
            long_min = coin_sup_gauche.PrendreX();
            long_max = coin_inf_droit.PrendreX();
            lat_max = coin_sup_gauche.PrendreY();
            lat_min = coin_inf_droit.PrendreY();

            std::ofstream ofs;
            std::string filename2;

			filename2 = sOutputFile;          

            PRELEVEMENTS_SITE::importFichierPrelevementGPE(filename, v_intervenant, v_nomIntervenant, v_nomLegal, v_annee, v_mois, v_nbrjour, v_volume, v_source, v_codescian, v_lat, v_long, v_consomation, long_min, long_max, lat_min, lat_max);
            FonctionObtientUhrhTroncon(v_long, v_lat, v_uhrh, v_troncon, v_idOccsol, v_sOccsol, vindex);
			PRELEVEMENTS_SITE::erase_indexGPE(v_uhrh, v_troncon, v_idOccsol, v_sOccsol, v_intervenant, v_nomIntervenant, v_nomLegal, v_annee, v_mois, v_nbrjour, v_volume, v_source, v_codescian, v_lat, v_long, v_consomation, vindex);
//            size_t size = v_annee.size();
            v_critere.assign(v_uhrh.size(), 1);
			v_coef_consommation.assign(v_uhrh.size(), -1);
          
            vector<PRELEVEMENTS_SITE> v_site_prelevement;
            int nbr_site = -1;
            while (v_lat.size() > 0)
                {
				nbr_site++;
                v_site_prelevement.push_back(PRELEVEMENTS_SITE(1, nbr_site, v_long[0], v_lat[0], v_uhrh[0], v_troncon[0], v_idOccsol[0], v_sOccsol[0], v_critere[0], v_source[0], v_coef_consommation[0])); // id site = 1 pour site GPE
                v_site_prelevement[nbr_site].setIntervenantCodeScianGPE(v_intervenant[0], v_nomIntervenant[0], v_nomLegal[0], v_codescian[0]);
				PRELEVEMENTS_SITE::vector_indexfound(v_lat[0], v_long[0], v_lat, v_long, v_index); // trouve les autres sites relier au site de prelevement trouver
                for (size_t j = 0; j < v_index.size(); j++)
                    {
                    v_site_prelevement[nbr_site].ajouteDonneeGPE(v_annee[v_index[j]], v_mois[v_index[j]], v_nbrjour[v_index[j]], v_volume[v_index[j]], v_consomation[v_index[j]]);
                    }
				PRELEVEMENTS_SITE::erase_indexGPE(v_uhrh, v_troncon, v_idOccsol, v_sOccsol, v_intervenant, v_nomIntervenant, v_nomLegal, v_annee, v_mois, v_nbrjour, v_volume, v_source, v_codescian, v_lat, v_long, v_consomation, v_index);//, v_exist); 
                }

            ofs.open(filename2, std::ofstream::out | std::ofstream::trunc);
			ofs << PRELEVEMENTS_SITE::printLegendGPE() << "\n";
            for (size_t k = 0; k < v_site_prelevement.size(); k++)
                {
                ofs << v_site_prelevement[k].printDonneGPE(k + 1);
                }

            ofs.close();
            }


	 void PRELEVEMENTS::trierDonnePrelevementagricole(const std::vector<std::string>& filename, const std::string& filenameTable, string sOutputFile)
	 {
		 //données prélèvements 
		 vector<int> v_index, v_uhrh, v_troncon, v_idOccsol, v_critere;
		 vector<string> v_source, v_sOccsol;
		 vector<double> v_lat, v_long, v_coef_consommation;

		 //données spécifiques Agricole
		 vector<int> v_siteGPE, v_sitePrelevementEAU;
		 std::vector<std::string> v_nomExploitant, v_nomlieu, v_nomMunicipalite, v_typeCheptel, v_nbrTete, v_listeCheptel, v_consomation;
		 std::vector<double>  v_consomationTotal, v_listeconsommation;

		 //vector<int> vindex;
		 double dPixelArea;
		 double lat_max_nad83, lat_min_nad83, long_max_nad83, long_min_nad83;
		 double lat_max, lat_min, long_max, long_min;
		 size_t nbColZone, nbRowZone;
		 const RASTER<int>& grilleZones = _sim_hyd.PrendreZones().PrendreGrille();

		 TRANSFORME_COORDONNEE trans(grilleZones.PrendreProjection(), PROJECTIONS::LONGLAT_WGS84());
		 long_min_nad83 = grilleZones.PrendreCoordonnee().PrendreX();
		 lat_min_nad83 = grilleZones.PrendreCoordonnee().PrendreY();
		 dPixelArea = static_cast<double>(grilleZones.PrendreTailleCelluleX());
		 nbColZone = grilleZones.PrendreNbColonne();
		 nbRowZone = grilleZones.PrendreNbLigne();
		 long_max_nad83 = long_min_nad83 + nbColZone * dPixelArea;
		 lat_max_nad83 = lat_min_nad83 - nbRowZone * dPixelArea;

		 COORDONNEE coin_sup_gauche = trans.TransformeXY(COORDONNEE(long_min_nad83, lat_min_nad83)); // transformer coordonnéee vers wgs84 et ensuite en pixel
		 COORDONNEE coin_inf_droit = trans.TransformeXY(COORDONNEE(long_max_nad83, lat_max_nad83)); // transformer coordonnéee vers wgs84 et ensuite en pixel
		 long_min = coin_sup_gauche.PrendreX();
		 long_max = coin_inf_droit.PrendreX();
		 lat_max = coin_sup_gauche.PrendreY();
		 lat_min = coin_inf_droit.PrendreY();
		 		 
		 std::ofstream ofs;
		 std::string filename2;

		 filename2 = sOutputFile;

		 //importTableConsommationCheptel(filenameTable, v_listeconsommation, v_listeCheptel);
		 PRELEVEMENTS_SITE::importFichierPrelevementagricole(filename, filenameTable, v_nomExploitant, v_nomlieu, v_nomMunicipalite, v_typeCheptel, v_nbrTete, v_listeconsommation, v_listeCheptel, v_lat, v_long, v_consomation, v_consomationTotal, long_min, long_max, lat_min, lat_max);
		 FonctionObtientUhrhTroncon(v_long, v_lat, v_uhrh, v_troncon, v_idOccsol, v_sOccsol, v_index);		 
		 PRELEVEMENTS_SITE::erase_indexagricole(v_uhrh, v_troncon, v_idOccsol, v_sOccsol, v_nomExploitant, v_nomlieu, v_nomMunicipalite, v_typeCheptel, v_nbrTete, v_lat, v_long, v_consomation, v_consomationTotal, v_index);
		
		 v_critere.assign(v_uhrh.size(), 1);
		 v_source.assign(v_uhrh.size(), "souterain");
		 v_siteGPE.assign(v_uhrh.size(), -1);
		 v_sitePrelevementEAU.assign(v_uhrh.size(), -1);
		 v_coef_consommation.assign(v_uhrh.size(), 0.8); 


		 vector<PRELEVEMENTS_SITE> v_site_prelevement;

		 for (size_t i = 0; i < v_lat.size() ; i++)
		 {
			 v_site_prelevement.push_back(PRELEVEMENTS_SITE(2, static_cast<int>(i), v_long[i], v_lat[i], v_uhrh[i], v_troncon[i], v_idOccsol[i], v_sOccsol[i], v_critere[i], v_source[i], v_coef_consommation[i])); // id site = 1 pour site GPE
			 v_site_prelevement[i].ajouteDonneeagricole(v_nomExploitant[i], v_nomlieu[i], v_nomMunicipalite[i], v_typeCheptel[i], v_nbrTete[i], v_consomation[i], v_consomationTotal[i], v_siteGPE[i], v_sitePrelevementEAU[i]);
		 }

		 //sert à trouver un site GPE proche désactiver pour le moment
		// convertWGS84toPixel(v_site_prelevement);
		 //std::string filenameGPE = PrendreRepertoireProjet() + "/BD_GPE.csv";
		 //std::vector<SITE_PRELEVEMENT> v_siteGPERead = SITE_PRELEVEMENT::lirePrelevementGPE(filenameGPE);
		// SiteGPEProche(v_siteGPERead, v_site_prelevement, 60);

		 //Sert à trouver un site GPE dont le nom peut ressembler à un site Agricole, désactiver pour le moment
		 //SITE_PRELEVEMENT::findCorrespondanceNom(v_siteGPERead, v_site_prelevement);

		 ofs.open(filename2, std::ofstream::out | std::ofstream::trunc);
		 ofs << PRELEVEMENTS_SITE::printLegendAgricole() << "\n";
		 for (size_t k = 0; k < v_site_prelevement.size(); k++)
		 {
			 ofs << v_site_prelevement[k].printDonneagricole(k + 1);
		 }

		 ofs.close();
	 }

}
