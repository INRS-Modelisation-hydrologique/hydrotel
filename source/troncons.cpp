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

#include "troncons.hpp"

#include "barrage_historique.hpp"
#include "lac.hpp"
#include "lac_sans_laminage.hpp"
#include "riviere.hpp"
#include "erreur.hpp"
#include "sim_hyd.hpp"
#include "util.hpp"

#include <algorithm>
#include <fstream>

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	TRONCONS::TRONCONS()
		: _troncons()
		, _nom_fichier()
		, _troncons_exutoire()
		, _map()
	{
		_pSimHyd = nullptr;
		_pRasterTronconId = nullptr;
	}


	TRONCONS::~TRONCONS()
	{
		if(_pRasterTronconId)
			delete [] _pRasterTronconId;
	}


	void TRONCONS::DetruireTroncons()
	{
		_troncons.clear();
		_map.clear();
		_troncons_exutoire.clear();
	}

	string TRONCONS::PrendreNomFichier() const
	{
		return _nom_fichier;
	}

	string TRONCONS::PrendreNomFichierPixels() const
	{
		return _nom_fichier_pixels;
	}

	size_t TRONCONS::PrendreNbTroncon() const
	{
		return _troncons.size();
	}

	void TRONCONS::ChangeNomFichier(const string& nom_fichier)
	{
		//DetruireTroncons();
		_nom_fichier = nom_fichier;
	}

	void TRONCONS::ChangeNomFichierPixels(const string& nom_fichier)
	{
		// DetruireTroncons();
		_nom_fichier_pixels = nom_fichier;
	}

	TRONCON* TRONCONS::operator[] (size_t index)
	{
		BOOST_ASSERT(index < _troncons.size());
		return _troncons[index].get();
	}

	void TRONCONS::LectureTroncons(ZONES& zones, NOEUDS& noeuds, bool bUpdate)
	{
		DetruireTroncons();

		ifstream fichier(_nom_fichier);

		if (!fichier)
			throw ERREUR_LECTURE_FICHIER(_nom_fichier);

		int ligne = 2;
		try
		{
			bool bAncienneVersionTRL;
			int type;						//type fichier //1; ancien format, 2; format avec no. ordre shreve a la derniere colonne
			size_t nb_troncon;
			fichier >> type >> nb_troncon;

			if(bUpdate && type != 2)
			{
				fichier.close();
				fichier.clear();

				if(!ShreveCompute(_nom_fichier))
					throw ERREUR("Erreur lors de la mise a jour du fichier de troncon (ordre de Shreve)");

				fichier.open(_nom_fichier);
				if (!fichier)
					throw ERREUR_LECTURE_FICHIER(_nom_fichier);

				fichier >> type >> nb_troncon;
			}

			++ligne;

			vector<shared_ptr<TRONCON>> troncons(nb_troncon);

			string commentaire;
			
			fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			getline_mod(fichier, commentaire);

			++ligne;

			int id_troncon = 1;
			bAncienneVersionTRL = false;

			_tronconNoeudAval.clear();

			for (size_t index = 0; index < nb_troncon; ++index)
			{
				shared_ptr<TRONCON> troncon;
				int id, type_troncon, id_noeud_aval;

				if(index == 0)
				{
					fichier >> id;
					if(id == 0)
					{
						bAncienneVersionTRL = true;
						type_troncon = id;
						fichier >> id_noeud_aval;
					}
					else
					{
						//dans le fichier TRL exporté par PHYSITEL4, la 1ere colonne est le ID du troncon, et le type doit avoir une valeur de 1 ou 2 au lieu de 0 ou 1.
						fichier >> type_troncon >> id_noeud_aval;
						type_troncon--;
					}
				}
				else
				{
					if(bAncienneVersionTRL)
						fichier >> type_troncon >> id_noeud_aval;
					else
					{
						fichier >> id >> type_troncon >> id_noeud_aval;
						type_troncon--;
					}
				}

				_tronconNoeudAval.push_back(id_noeud_aval);

				NOEUD* noeud_aval = noeuds.Recherche(id_noeud_aval);

				switch (type_troncon)
				{
				case 0:				
					troncon = LectureRiviere(fichier, zones, noeuds);

					// calcule de la pente de la riviere
					{
						auto riviere = static_cast<RIVIERE*>(troncon.get());
						auto noeuds_amont = riviere->PrendreNoeudsAmont();

						float dz = static_cast<float>(noeuds_amont[0]->PrendreCoordonnee().PrendreZ() - noeud_aval->PrendreCoordonnee().PrendreZ());
						float pente = dz / riviere->PrendreLongueur();

						riviere->ChangePente(max(pente, 0.0025f));
					}
					break;

				case 1:
					troncon = LectureLac(fichier, zones, noeuds);
					break;

				case 3:
					troncon = LectureLacSansLaminage(fichier, zones, noeuds);
					break;

				case 4:
					troncon = LectureBarrageHistorique(fichier, zones, noeuds, id_troncon);
					break;

				default:
					throw ERREUR_LECTURE_FICHIER(_nom_fichier);
				}

				if(type == 2)
					fichier >> troncon->_iSchreve;	//no ordre de shreve

				troncon->ChangeIdent(id_troncon);

				vector<NOEUD*> vnoeuds;
				vnoeuds.push_back(noeud_aval);

				troncon->ChangeNoeudsAval(vnoeuds);

				troncons[index] = troncon;

				++id_troncon;
				++ligne;
			}

			_troncons.swap(troncons);

			// determine la connectivite
			for (auto t1 = begin(_troncons); t1 != end(_troncons); ++t1)
			{
				auto noeuds_amont = t1->get()->PrendreNoeudsAmont();

				vector<TRONCON*> troncons_amont;

				for (auto noeud = begin(noeuds_amont); noeud != end(noeuds_amont); ++noeud)
				{
					for (auto t2 = begin(_troncons); t2 != end(_troncons); ++t2)
					{
						//NOTE: a modifier pour la gestion des sorties multiples

						NOEUD* noeud_aval = t2->get()->PrendreNoeudsAval()[0];

						if (*noeud == noeud_aval)
						{
							vector<TRONCON*> troncons2;
							troncons2.push_back(t1->get());

							t2->get()->ChangeTronconsAval(troncons2);
							troncons_amont.push_back(t2->get());
						}
					}
				}

				if (troncons_amont.size() > 0)
				{
					troncons_amont.shrink_to_fit();
					t1->get()->ChangeTronconsAmont(troncons_amont);
				}
			}

			// determine les troncons exutoire
			for (auto troncon = begin(_troncons); troncon != end(_troncons); ++troncon)
			{
				if (troncon->get()->PrendreTronconsAval().empty())
					_troncons_exutoire.push_back(troncon->get());
			}

			// cree la map de recherche
			for (auto troncon = begin(_troncons); troncon != end(_troncons); ++troncon)
			{
				_map[troncon->get()->PrendreIdent()] = troncon->get();
			}

		}
		catch (const exception& e)
		{
			throw ERREUR_LECTURE_FICHIER(_nom_fichier, ligne, e.what());
		}
	}

	shared_ptr<TRONCON> TRONCONS::LectureRiviere(ifstream& fichier, ZONES& zones, NOEUDS& noeuds)
	{
		int id_noeud_amont;
		float longueur, largeur, manning;
		fichier >> id_noeud_amont >> longueur >> largeur >> manning;

		// valide les donnees
		if (longueur <= 0)
			throw ERREUR("invalid river reach length value");

		if (largeur <= 0)
			throw ERREUR("invalid river reach width value");

		if (manning < 0.02f)
			throw ERREUR("invalid river reach manning value");

		vector<NOEUD*> noeuds_amont(1);					
		noeuds_amont[0] = noeuds.Recherche(id_noeud_amont);

		shared_ptr<RIVIERE> riviere = make_shared<RIVIERE>(RIVIERE());
					
		riviere->ChangeLargeur(largeur);
		riviere->ChangeLongueur(longueur);
		riviere->ChangeManning(manning);
		riviere->ChangeNoeudsAmont(noeuds_amont);

		LectureZoneAmont(fichier, zones, riviere.get());

		return riviere;
	}

	shared_ptr<TRONCON> TRONCONS::LectureLac(ifstream& fichier, ZONES& zones, NOEUDS& noeuds)
	{
		size_t nb_noeud_amont;
		fichier >> nb_noeud_amont;

		vector<NOEUD*> noeuds_amont(nb_noeud_amont);					
		for (size_t index = 0; index < nb_noeud_amont; ++index)
		{
			int id_noeud_amont;
			fichier >> id_noeud_amont;
			noeuds_amont[index] = noeuds.Recherche(id_noeud_amont);
		}

		float longueur, surface, c, k;
		fichier >> longueur >> surface >> c >> k;

		shared_ptr<LAC> lac = make_shared<LAC>(LAC());					

		lac->ChangeLongueur(longueur);	//m
		lac->ChangeSurface(surface);	//km2
		lac->ChangeC(c);
		lac->ChangeK(k);
		lac->ChangeNoeudsAmont(noeuds_amont);

		LectureZoneAmont(fichier, zones, lac.get());

		return lac;
	}

	shared_ptr<TRONCON> TRONCONS::LectureLacSansLaminage(ifstream& fichier, ZONES& zones, NOEUDS& noeuds)
	{
		size_t nb_noeud_amont;
		fichier >> nb_noeud_amont;

		vector<NOEUD*> noeuds_amont(nb_noeud_amont);					
		for (size_t index = 0; index < nb_noeud_amont; ++index)
		{
			int id_noeud_amont;
			fichier >> id_noeud_amont;
			noeuds_amont[index] = noeuds.Recherche(id_noeud_amont);
		}

		shared_ptr<LAC_SANS_LAMINAGE> lac = make_shared<LAC_SANS_LAMINAGE>(LAC_SANS_LAMINAGE());					

		lac->ChangeNoeudsAmont(noeuds_amont);

		LectureZoneAmont(fichier, zones, lac.get());

		return lac;
	}

	shared_ptr<TRONCON> TRONCONS::LectureBarrageHistorique(ifstream& fichier, ZONES& zones, NOEUDS& noeuds, int idTroncon)
	{
		shared_ptr<BARRAGE_HISTORIQUE> barrage = make_shared<BARRAGE_HISTORIQUE>(BARRAGE_HISTORIQUE());					

		size_t nb_noeud_amont;
		string str, ident_station_hydro;
		int id_noeud_amont;

		fichier >> nb_noeud_amont;

		vector<NOEUD*> noeuds_amont(nb_noeud_amont);					
		for (size_t index = 0; index < nb_noeud_amont; ++index)
		{
			fichier >> id_noeud_amont;
			noeuds_amont[index] = noeuds.Recherche(id_noeud_amont);
		}

		fichier >> ident_station_hydro;

		barrage->ChangeIdentStationHydro(ident_station_hydro);

		str = ident_station_hydro;
		boost::algorithm::to_lower(str);
		_listHydroStationReservoirHistory.push_back(str);
		_listHydroStationReservoirHistoryIdTroncon.push_back(idTroncon);

		barrage->ChangeNoeudsAmont(noeuds_amont);

		LectureZoneAmont(fichier, zones, barrage.get());

		return barrage;
	}

	void TRONCONS::LectureZoneAmont(ifstream& fichier, ZONES& zones, TRONCON* troncon)
	{
		if (troncon == nullptr)
			throw ERREUR("TRONCONS::LectureZoneAmont; troncon == nullptr");

		size_t nb_zone;
		fichier >> nb_zone;

		vector<ZONE*> zones_amont(nb_zone);
		for (size_t index = 0; index < nb_zone; ++index)
		{
			int id_zone;
			fichier >> id_zone;

			ZONE* zone = zones.Recherche(id_zone);
			if (zone == nullptr)
				throw ERREUR("rhhu id not found");

			zone->ChangeTronconAval(troncon);
			zones_amont[index] = zone;

			if(_pSimHyd != nullptr && _pSimHyd->_versionBV3C == 1)	//_pSimHyd is null when program option -new
			{
				if(index == 0 && troncon->PrendreType() == TRONCON::LAC)
					zone->ChangeTypeZone(ZONE::LAC);	//le 1er uhrh est l'uhrh représentant le lac
			}
			else
			{
				if(index == 0 && (troncon->PrendreType() == TRONCON::LAC || troncon->PrendreType() == TRONCON::LAC_SANS_LAMINAGE))
					zone->ChangeTypeZone(ZONE::LAC);	//le 1er uhrh est l'uhrh représentant le lac
			}
		}

		troncon->ChangeZonesAmont(zones_amont);
	}

	TRONCON* TRONCONS::RechercheTroncon(int ident)
	{
		return _map.find(ident) == _map.end() ? nullptr : _map[ident];
	}

	const vector<TRONCON*>& TRONCONS::PrendreTronconsExutoire() const
	{
		return _troncons_exutoire;
	}

	void TRONCONS::LectureFichierPixels()
	{
		TRONCON* troncon = nullptr;
		int nb_ligne, nb_pixel, ligne, colonne, ident, code, n;

		ifstream fichier(_nom_fichier_pixels);

		fichier >> nb_ligne >> _nb_colonne >> nb_pixel;

		_pRasterTronconId = new size_t[_nb_colonne*nb_ligne]();		//Zero-initialized -> ()

		for (n = 0; n < nb_pixel; ++n)
		{
			fichier >> ligne >> colonne >> ident;

			code = ligne * _nb_colonne + colonne;
			
			troncon = RechercheTroncon(ident);
			if (troncon == nullptr)
				throw ERREUR("river reach id not found");

			_pixels[code] = troncon;
			_pRasterTronconId[code] = ident;
		}

		fichier.close();
	}


	void TRONCONS::LectureFichierLargeur(string sFile)
	{
		TRONCON* troncon = nullptr;
		vector<string> sList;
		istringstream iss;
		double dval;
		string str;
		int ival;

		ifstream file(sFile);
		if(!file)
			throw ERREUR("erreur lecture fichier: " + sFile);

		try{

		getline_mod(file, str);	//comment line		//IDTroncon;LargeurMoyenne(m);SuperficieLit(m2);Longueur(m)

		while(!file.eof())
		{
			getline_mod(file, str);
			if(str != "")
			{
				SplitString(sList, str, ";", true, true);
				if(sList.size() < 2)
				{
					file.close();
					throw ERREUR("erreur lecture fichier: format invalide: " + sFile);
				}

				iss.clear();
				iss.str(sList[1]);
				iss >> dval;

				if(dval != -1.0)
				{
					iss.clear();
					iss.str(sList[0]);
					iss >> ival;

					troncon = RechercheTroncon(ival);
					if(troncon == nullptr)
					{
						file.close();
						throw ERREUR("erreur lecture fichier: " + sFile + ": IDTroncon inexistant: " + sList[0]);
					}

					if(troncon->PrendreType() == TRONCON::RIVIERE)
						((RIVIERE*)troncon)->ChangeLargeur(static_cast<float>(dval));
				}
			}
		}

		file.close();

		}
		catch(const ERREUR& ex)
		{
			if(file && file.is_open())
				file.close();
			throw ex;
		}
		catch(const exception& ex)
		{
			if(file && file.is_open())
				file.close();
			string mess = ex.what();
			throw ERREUR("erreur lecture fichier: exception: " + mess + ": " + sFile);
		}
	}


	void TRONCONS::CalculeShreve()
	{
		vector<int>::iterator it;
		size_t nbTroncon, nbTraiter, i, j, k;
		int iOrdreTemp;

		nbTroncon = _troncons.size();

		//determine en premier les ordres 1 (troncons ou lac sans noeud amont), met 0 pour tous les autres
		nbTraiter = nbTroncon;

		for(i=0; i!=nbTroncon; i++)
		{
			_troncons[i]->_iSchreve = 1;	//ordre de 1 pour debuter pour tous les troncons

			for(j=0; j!=_troncons[i]->_noeuds_amont.size(); j++)
			{
				if(std::find(begin(_tronconNoeudAval), end(_tronconNoeudAval), _troncons[i]->_noeuds_amont[j]->PrendreIdent()) != _tronconNoeudAval.end())
				{
					_troncons[i]->_iSchreve = 0;	//le noeud amont courant est le noeud aval dun autre troncon
					--nbTraiter;
					break;
				}
			}
		}

		while(nbTraiter != nbTroncon)
		{
			for(i=0; i!=nbTroncon; i++)
			{
				if(_troncons[i]->_iSchreve == 0)
				{
					iOrdreTemp = 1;						

					for(j=0; j!=_troncons[i]->_noeuds_amont.size(); j++)	//pour chaque noeud amont du troncon courant
					{
						it = find(begin(_tronconNoeudAval), end(_tronconNoeudAval), _troncons[i]->_noeuds_amont[j]->PrendreIdent());

						while(it != _tronconNoeudAval.end())	//pour chaque troncon amont au troncon courant
						{
							k = it-_tronconNoeudAval.begin();	//index
							if(_troncons[k]->_iSchreve == 0)
							{
								iOrdreTemp = 0;
								break;
							}
							else
								iOrdreTemp = max(iOrdreTemp, _troncons[k]->_iSchreve);

							it = std::find(it+1, end(_tronconNoeudAval), _troncons[i]->_noeuds_amont[j]->PrendreIdent());
						}

						if(iOrdreTemp == 0)
							break;
					}

					if(iOrdreTemp != 0)
					{
						_troncons[i]->_iSchreve = iOrdreTemp+1;
						++nbTraiter;
					}
				}
			}
		}
	}


	void TRONCONS::CalculeStrahler()
	{
		vector<int>::iterator it;
		size_t nbTroncon, nbTraiter, i, j, k;
		int iOrdreMax, nbOrdreMax;

		nbOrdreMax = 0;
		nbTroncon = _troncons.size();

		//determine en premier les ordres 1 (troncons ou lac sans noeud amont), met 0 pour tous les autres
		nbTraiter = nbTroncon;

		for(i=0; i!=nbTroncon; i++)
		{
			_troncons[i]->_iSchreve = 1;	//ordre de 1 pour debuter pour tous les troncons

			for(j=0; j!=_troncons[i]->_noeuds_amont.size(); j++)
			{
				if(std::find(begin(_tronconNoeudAval), end(_tronconNoeudAval), _troncons[i]->_noeuds_amont[j]->PrendreIdent()) != _tronconNoeudAval.end())
				{
					_troncons[i]->_iSchreve = 0;	//le noeud amont courant est le noeud aval dun autre troncon
					--nbTraiter;
					break;
				}
			}
		}

		while(nbTraiter != nbTroncon)
		{
			for(i=0; i!=nbTroncon; i++)
			{
				if(_troncons[i]->_iSchreve == 0)
				{
					iOrdreMax = 0;

					for(j=0; j!=_troncons[i]->_noeuds_amont.size(); j++)	//pour chaque noeud amont du troncon courant
					{
						it = find(begin(_tronconNoeudAval), end(_tronconNoeudAval), _troncons[i]->_noeuds_amont[j]->PrendreIdent());

						while(it != _tronconNoeudAval.end())	//pour chaque troncon amont au troncon courant
						{
							k = it-_tronconNoeudAval.begin();	//index

							if(_troncons[k]->_iSchreve == 0)
							{
								iOrdreMax = -1;
								break;
							}
							else
							{
								if(_troncons[k]->_iSchreve > iOrdreMax)
								{
									nbOrdreMax = 1;
									iOrdreMax = _troncons[k]->_iSchreve;
								}
								else
								{
									if(_troncons[k]->_iSchreve == iOrdreMax)
										++nbOrdreMax;
								}
							}

							it = std::find(it+1, end(_tronconNoeudAval), _troncons[i]->_noeuds_amont[j]->PrendreIdent());
						}

						if(iOrdreMax == -1)
							break;
					}

					if(iOrdreMax != -1)
					{
						if(nbOrdreMax > 1)
							_troncons[i]->_iSchreve = iOrdreMax+1;
						else
							_troncons[i]->_iSchreve = iOrdreMax;

						++nbTraiter;
					}
				}
			}
		}
	}


	TRONCON* TRONCONS::RechercheTroncon(int ligne, int colonne)
	{
		int code = ligne * _nb_colonne + colonne;

		auto iter = _pixels.find(code);		
		return  iter == _pixels.end() ? nullptr : iter->second;
	}

	size_t TRONCONS::IdentVersIndex(int ident) const
	{
		for (size_t index = 0; index != _troncons.size(); ++index)
		{
			if (_troncons[index].get()->PrendreIdent() == ident)
				return index;
		}
		throw ERREUR("ident troncon introuvable");
	}

}
