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

#include "util.hpp"

#include "erreur.hpp"
#include "gdal_util.hpp"
#include "projections.hpp"
#include "station_meteo.hpp"
#include "station_meteo_netcdf_station.hpp"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_array.hpp>

#include <netcdf.h>


using namespace std;


namespace HYDROTEL 
{

	std::vector<std::string>		_listLog;


	//------------------------------------------------------------------------------------------------
	//Interprete la string et retourne en decimal degree.
	//
	//Formats acceptés: ddd.d						//decimal degree
	//                  ddmm.m (ou ddmm)			//degree, decimal minute
	//                  dddmm.m (ou dddmm)			//degree, decimal minute
	//                  ddmmss.s (ou ddmmss)		//degree, minute, decimal second
	//                  dddmmss.s (ou dddmmss)		//degree, minute, decimal second
	//
	//Les décimales sont optionnelles, excepté pour le format decimal degree.
	//
	//Si c'est la longitude, le signe négatif est appliqué par défaut afin de se situé 
	//dans l'hémisphère ouest (pour compatibilité). Si on veut utiliser une longitude dans 
	//l'hémisphère est, on doit spécifier le signe +.
	//

	double ParseLatLongCoord(string sString, bool bLongitude)
    {
        vector<std::string> valeurs;
		istringstream iss;
		double Deg, Min, Sec, Ret, dVal;
        bool bPos, bNeg;

		sString = TrimString(sString);

		if(sString.size() < 3)
			throw ERREUR("Error reading coordinate: invalid coordinate");

		replace(sString.begin(), sString.end(), ',', '.');

		bPos = bNeg = false;

		try{

        if (sString[0] == '+')
        {
            sString = sString.substr(1);
            bPos = true;
        }
        else
        {
            if (sString[0] == '-')
            {
                sString = sString.substr(1);
                bNeg = true;
            }
        }

		SplitString(valeurs, sString, ".", false, true);

		if (valeurs.size() != 1 && valeurs.size() != 2)
			throw ERREUR("Error reading coordinate: invalid coordinate");

		if(valeurs[0].size() == 0)
			throw ERREUR("Error reading coordinate: invalid coordinate");

		Sec = 0.0;

        if (valeurs.size() == 2)
        {
			//il y a une decimale, conserve la fraction
			if(valeurs[1].size() == 0)
				throw ERREUR("Error reading coordinate: invalid coordinate");

			iss.clear();
			iss.str((valeurs[1]));
			iss >> Sec;

            Sec/= pow(10.0, valeurs[1].size());
		}

        if (valeurs[0].size() <= 3)    //ddd.d		//degree decimal
		{
			if (valeurs.size() != 2)
				throw ERREUR("Error reading coordinate: invalid coordinate");	//les décimales doivent etres spécifiées

			iss.clear();
			iss.str((valeurs[0]));
			iss >> Deg;

            if (Deg < -180.0 || Deg > 180.0)
                throw ERREUR("Error reading coordinate: invalid coordinate");

            Ret = Deg + Sec;
		}
		else
		{
			if (valeurs[0].size() == 4)    //ddmm.m		//degree, minute decimal
			{
				iss.clear();
				iss.str((valeurs[0].substr(0, 2)));
				iss >> Deg;

				iss.clear();
				iss.str((valeurs[0].substr(2, 2)));
				iss >> Min;

				Min+= Sec;	//minute decimal
				Ret = Deg + (Min / 60.0);
			}
			else
			{
				if (valeurs[0].size() == 5)		//dddmm.m	//degree, minute decimal
				{
					iss.clear();
					iss.str((valeurs[0].substr(0, 3)));
					iss >> Deg;

					iss.clear();
					iss.str((valeurs[0].substr(3, 2)));
					iss >> Min;

					Min+= Sec;	//minute decimal
					Ret = Deg + (Min / 60.0);
				}
				else
				{
					if (valeurs[0].size() == 6)		//ddmmss.s	//degree, minute, seconde
					{
						iss.clear();
						iss.str((valeurs[0].substr(0, 2)));
						iss >> Deg;

						iss.clear();
						iss.str((valeurs[0].substr(2, 2)));
						iss >> Min;

						iss.clear();
						iss.str((valeurs[0].substr(4, 2)));
						iss >> dVal;

						Sec+= dVal;	//ajoute la valeur entiere à la fraction (décimal)

						Ret = Deg + (Min / 60.0) + (Sec / 3600.0);
					}
					else
					{
						if (valeurs[0].size() == 7)		//dddmmss.s		//degree, minute, seconde
						{
							iss.clear();
							iss.str((valeurs[0].substr(0, 3)));
							iss >> Deg;

							iss.clear();
							iss.str((valeurs[0].substr(3, 2)));
							iss >> Min;

							iss.clear();
							iss.str((valeurs[0].substr(5, 2)));
							iss >> dVal;

							Sec+= dVal;	//ajoute la valeur entiere à la fraction (décimal)

							Ret = Deg + (Min / 60.0) + (Sec / 3600.0);
						}
						else
							throw ERREUR("Error reading coordinate: invalid coordinate");
					}
				}
			}
		}

        if (bNeg || (bLongitude && !bPos))   //negatif par defaut pour longitude pour compatibilite
            Ret = -Ret;

		}
		catch(...)
		{
			throw ERREUR("Error reading coordinate: invalid coordinate");
		}

        return Ret;
    }


	//---------------------------------------------------------------------------------------------
	//Calcule l'ordre de Shreve pour chaque troncon et ajoute ds le fichier TRL (derniere colonne)
	bool ShreveCompute(const string& sTronconFile)
	{
		vector<int>				vID;		
		vector<int>				vNoeudAval;			//id noeud aval
		map<int, vector<int>>	mapNoeudAmont;		//id troncon, id noeud amont
		vector<int>				vOrdre;				//no ordre
		string sString;
		int						iTypeFichier;		//type fichier //1; ancien format, 2; format avec no. ordre shreve a la derniere colonne

		bool bAncienneVersionTRL = false;
		bool bRet = false;

		//lecture du fichier troncon trl
		ifstream doc(sTronconFile);

		if (doc.good())
		{
			size_t k;
			int iNbTroncon, iType, iVal, i, j, n;
			int iID = 0;

			doc >> iTypeFichier;	//type fichier //1; ancien format, 2; format avec no. ordre shreve a la derniere colonne
			doc >> iNbTroncon;		//nb troncon
			doc >> sString;			//commentaire

			vNoeudAval.resize(iNbTroncon, 0);

			for (i=0; i<iNbTroncon; i++)
			{
				if (i == 0)
				{
					doc >> iID;
					if(iID == 0)
					{
						bAncienneVersionTRL = true;
						iType = iID;
						iID = 1;
						doc >> iVal;
					}
					else
					{
						//dans le fichier TRL exporté par PHYSITEL4, la 1ere colonne est le ID du troncon et le type commence a 1 au lieu de 0
						doc >> iType >> iVal;
						--iType;
					}
				}
				else
				{
					if(bAncienneVersionTRL)
					{
						doc >> iType >> iVal;
						++iID;
					}
					else
					{
						doc >> iID >> iType >> iVal;
						--iType;
					}
				}

				vID.push_back(iID);
				vNoeudAval[i] = iVal;	//noeud aval

				if (iType == 0)	//troncon riviere
				{
					doc >> iVal;
					mapNoeudAmont[iID].push_back(iVal);		//noeud amont

					getline_mod(doc, sString); //lit le reste de la ligne pour acceder au debut de la prochaine ligne
				}
				else			//autres types; il y a plusieurs noeud amont
				{
					doc >> n;	//nombre de noeuds amont
					for (j=0; j<n; j++)
					{
						doc >> iVal;
						mapNoeudAmont[iID].push_back(iVal);		//noeud amont
					}

					getline_mod(doc, sString); //lit le reste de la ligne pour acceder au debut de la prochaine ligne
				}
			}

			doc.close();
			doc.clear();

			//algorithme de l'ordre de Shreve
			vector<int>::iterator it;
			size_t x, y, z;
			int iOrdreTemp, iNbTraiter;

			//determine en premier les ordre 1 (troncons ou lac sans noeud amont), met 0 pour tous les autres
			vOrdre.resize(iNbTroncon, 1);	//ordre de 1 pour debuter pour tous les troncons
			iNbTraiter = iNbTroncon;

			for(x=0; x<vID.size(); x++)
			{
				for(y=0; y<mapNoeudAmont[vID[x]].size(); y++)
				{
					it = find(begin(vNoeudAval), end(vNoeudAval), mapNoeudAmont[vID.at(x)][y]);
					if(it != vNoeudAval.end())
					{
						vOrdre[x] = 0;	//le noeud amont courant est le noeud aval dun autre troncon
						--iNbTraiter;
						break;
					}
				}
			}

			//pour chaque troncon
			while(iNbTraiter != iNbTroncon)
			{
				for(x=0; x<vID.size(); x++)
				{
					if(vOrdre[x] == 0)
					{
						iOrdreTemp = 1;						
						//pour chaque noeud amont du troncon courant
						for(y=0; y<mapNoeudAmont[vID[x]].size(); y++)
						{
							it = find(begin(vNoeudAval), end(vNoeudAval), mapNoeudAmont[vID[x]][y]);
							//pour chaque troncon amont au troncon courant
							while(it != vNoeudAval.end())
							{
								z = it-vNoeudAval.begin();	//index
								if(vOrdre[z] == 0)
								{
									iOrdreTemp = 0;
									break;
								}
								else
									iOrdreTemp = max(iOrdreTemp, vOrdre[z]);

								it = find(it+1, end(vNoeudAval), mapNoeudAmont[vID[x]][y]);
							}

							if(iOrdreTemp == 0)
								break;
						}

						if(iOrdreTemp != 0)
						{
							vOrdre[x] = iOrdreTemp+1;
							++iNbTraiter;
						}
					}
				}
			}

			//lecture et re-ecriture du fichier TRL 
			vector<string>	sList;
			vector<string>	vFichierTRL;
			istringstream	iss;
			ofstream		out;

			doc.open(sTronconFile);
			if(doc)
			{
				while(doc.good())
				{
					getline_mod(doc, sString);
					if(sString != "")
						vFichierTRL.push_back(sString);
				}

				doc.close();
				doc.clear();

				//réécris le fichier
				out.open(sTronconFile);
				out << "2" << endl;	//type fichier //1; ancien format, 2; format avec no. ordre shreve a la derniere colonne
				
				for(x=1; x<vFichierTRL.size(); x++)
				{					
					if(x>=3)
					{
						if(iTypeFichier == 2)
						{
							SplitString(sList, vFichierTRL[x], " ", true, false);
							for(k=0; k<sList.size()-1; k++)
								out << sList[k] << " ";
						}
						else
						{
							if(vFichierTRL[x][vFichierTRL[x].size()-1] == ' ')
								out << vFichierTRL[x];
							else
								out << vFichierTRL[x] << " ";
						}

						out << vOrdre[x-3] << endl;
					}
					else
						out << vFichierTRL[x] << endl;
				}

				out.close();
				bRet = true;
			}
		}

		return bRet;
	}


	RASTER<float> LectureRasterPhysitel_float(const string& nom_fichier, float mult)
	{
		try
		{
			ifstream fichier(nom_fichier, ios_base::binary);

			char entete[513] = { "" };
		
			fichier.read(entete, sizeof(char) * 512);
		
			istringstream iss(entete);
			int type, proj, zone;
			size_t ncol, nlig, nbCell, x;		
			float estG, nordG, res_x, res_y;
		
			iss >> type >> proj >> zone >> nlig >> ncol >> estG >> nordG >> res_x >> res_y;
		
			nbCell = nlig * ncol;
			vector<int> donnee(nbCell, -999);
			fichier.read(reinterpret_cast<char*>(&donnee[0]), sizeof(int) * nlig * ncol);

			for(x=0; x<nbCell; x++)
			{
				if(donnee[x] < -999)	//les valeurs nodata provenant du fichier mna sont egal à -2.14748e+009
					donnee[x] = -999;
			}

			//obtient la projection du projet hydrotel 2.6
			PROJECTION projectionV26;
			string sString;

			string nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "CoordSys.txt");
			if(FichierExiste(nom_fichier_projection))
			{
				ifstream fich(nom_fichier_projection);

				getline_mod(fich, sString);
				fich.close();

				if(sString == "")
					throw ERREUR("Erreur; LectureRasterPhysitel_float;  erreur lors de la lecture du fichier CoordSys.txt; le fichier est vide.");
		
				projectionV26 = PROJECTION::ImportFromCoordSys(sString);
			}
			else
			{
				nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "proj4.txt");
				if(FichierExiste(nom_fichier_projection))
					projectionV26 = PROJECTION::ImportFromProj4File(nom_fichier_projection);
				else
				{
					nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "projection.prj");
					if(!FichierExiste(nom_fichier_projection))
						throw ERREUR("Erreur; LectureRasterPhysitel_float;  erreur lors de la lecture du fichier de projection (/physitel/ CoordSys.txt ou projection.prj).");
					else
						projectionV26 = PROJECTION::ImportFromPRJ(nom_fichier_projection);
				}					
			}

			//si projection UTM; set la zone utm du projet, important si le bassin chevauche sur 2 zones utm
			sString = projectionV26.ExportWkt();
			if(sString.find("PROJECTION[\"Transverse_Mercator\"]") != string::npos)
			{
				if(projectionV26._spatial_reference.SetUTM(zone) != OGRERR_NONE)
					throw ERREUR("Erreur; LectureRasterPhysitel_float;  erreur lors de la lecture du fichier CoordSys.txt; zone utm.");
			}

			//		
			RASTER<float> raster(COORDONNEE(estG, nordG), projectionV26, nlig, ncol, res_x, res_y);

			for (size_t ligne = 0; ligne < nlig; ++ligne)
			{
				for (size_t colonne = 0; colonne < ncol; ++colonne)
					raster(ligne, colonne) = mult * donnee[ligne * ncol + colonne];
			}

			return raster;
		}
		catch (...)
		{
			throw ERREUR_LECTURE_FICHIER(nom_fichier);
		}	
	}

	RASTER<int> LectureRasterPhysitel_int(const string& nom_fichier)
	{
		try
		{
			ifstream fichier(nom_fichier, ios_base::binary);

			char entete[513] = { "" };
		
			fichier.read(entete, sizeof(char) * 512);
		
			istringstream iss(entete);
			int type, proj, zone;
			size_t ncol, nlig;		
			float estG, nordG, res_x, res_y;
		
			iss >> type >> proj >> zone >> nlig >> ncol >> estG >> nordG >> res_x >> res_y;
		
			//obtient la projection du projet hydrotel 2.6
			PROJECTION projectionV26;
			string sString;

			string nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "CoordSys.txt");
			if(FichierExiste(nom_fichier_projection))
			{
				ifstream fich(nom_fichier_projection);
				
				getline_mod(fich, sString);
				fich.close();

				if(sString == "")
					throw ERREUR("Erreur; LectureRasterPhysitel_int;  erreur lors de la lecture du fichier CoordSys.txt; le fichier est vide.");
		
				projectionV26 = PROJECTION::ImportFromCoordSys(sString);
			}
			else
			{
				nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "proj4.txt");
				if(FichierExiste(nom_fichier_projection))
					projectionV26 = PROJECTION::ImportFromProj4File(nom_fichier_projection);
				else
				{
					nom_fichier_projection = Combine(nom_fichier.substr(0, nom_fichier.rfind("/")), "projection.prj");
					if(!FichierExiste(nom_fichier_projection))
						throw ERREUR("Erreur; LectureResumerRsm;  erreur lors de la lecture du fichier de projection (/physitel/ CoordSys.txt ou projection.prj).");
					else
						projectionV26 = PROJECTION::ImportFromPRJ(nom_fichier_projection);
				}
			}

			//si projection UTM; set la zone utm du projet, important si le bassin chevauche sur 2 zones utm
			sString = projectionV26.ExportWkt();
			if(sString.find("PROJECTION[\"Transverse_Mercator\"]") != string::npos)
			{
				if(projectionV26._spatial_reference.SetUTM(zone) != OGRERR_NONE)
					throw ERREUR("Erreur; LectureRasterPhysitel_int;  erreur lors de la lecture du fichier CoordSys.txt; zone utm.");
			}

			//
			RASTER<int> raster(COORDONNEE(estG, nordG), projectionV26, nlig, ncol, res_x, res_y);

			fichier.read(reinterpret_cast<char*>(raster.PrendrePtr()), sizeof(int) * nlig * ncol);

			return raster;
		}
		catch (...)
		{
			throw ERREUR_LECTURE_FICHIER(nom_fichier);
		}
	}


	string ExtraitNomFichier(const string& nom_fichier)
	{
		boost::filesystem::path p(nom_fichier);
		return p.stem().string();
	}


	//IMPORTANT: la string retourné par PrendreRepertoire() peut etre vide lorsque _nom_fichier est un chemin relatif et qu'il n'y a pas de dossier parent.
	string PrendreRepertoire(const string& nom_fichier)
	{
		boost::filesystem::path p(nom_fichier);
		return p.parent_path().string();
	}


	string PrendreExtension(const string& nom_fichier)
	{
		boost::filesystem::path p(nom_fichier);
		return p.extension().string();
	}

	string PrendreFilename(const string& nom_fichier)
	{
		boost::filesystem::path p(nom_fichier);
		return p.filename().string();
	}

	string RemplaceExtension(const string& nom_fichier, const string& ext)
	{
		size_t pos = nom_fichier.rfind('.');
		return pos != string::npos ? nom_fichier.substr(0, pos) + '.' + ext : nom_fichier + '.' + ext;
	}

	bool FichierExiste(const string& nom_fichier)
	{
		return boost::filesystem::exists(nom_fichier);
	}

	void SupprimerFichier(const string& nom_fichier)
	{
		boost::filesystem::remove(nom_fichier);
	}

	bool RepertoireExiste(const string& repertoire)
	{
		return boost::filesystem::exists(repertoire);
	}

	string LireChaine(istream& stream, size_t nb_flag)
	{
		char c;
		size_t nb_carac;

		stream >> c >> nb_carac;

		vector<char> chaine(nb_carac + 2, '\0');
		stream.read(&chaine[0], nb_carac + 1);

		for (size_t n = 0; n < nb_flag; ++n)
		{
			int tmp;
			stream >> tmp;
		}

		return string(&chaine[1]);
	}

	string LireNomFichier(const string& repertoire, istream& stream, size_t nb_flag)
	{
		string nom_fichier = LireChaine(stream, nb_flag);
		replace(nom_fichier.begin(), nom_fichier.end(), '\\', '/');

		return Combine(repertoire, nom_fichier);
	}


	//-----------------------------------------------------------------------------------------------
	size_t GetIndexNearestCoord(const vector<COORDONNEE>& coordonnees, const COORDONNEE& coordonnee)
	{
		double distance, minDistance;
		size_t index, minDistanceIndex, nbCoord;

		nbCoord = coordonnees.size();
		minDistance = 1000000000.0;
		minDistanceIndex = 0;

		//calcule les distances
		for(index=0; index<nbCoord; index++)
		{
			distance = sqrt(pow(coordonnee.PrendreX() - coordonnees[index].PrendreX(), 2.0) + pow(coordonnee.PrendreY() - coordonnees[index].PrendreY(), 2.0));
			if(distance < minDistance)
			{
				minDistanceIndex = index;
				minDistance = distance;
			}
		}

		return minDistanceIndex;
	}


	//----------------------------------------------------------------------------------------------------
	//For thiessen v1 and moyenne_3_stations v1
	vector<size_t> CalculDistance_v1(const vector<COORDONNEE>& coordonnees, const COORDONNEE& coordonnee)
	{
		map<double, size_t> distances;

		for (size_t index = 0; index < coordonnees.size(); ++index)
		{
			double distance = sqrt(pow(coordonnee.PrendreX() - coordonnees[index].PrendreX(), 2) + pow(coordonnee.PrendreY() - coordonnees[index].PrendreY(), 2));
			distances[distance] = index;
		}

		vector<size_t> index;
		for (auto iter = begin(distances); iter != end(distances); ++iter)
			index.push_back(iter->second);

		return index;
	}


	//-------------------------------------------------------------------------------------------------
	//retourne les index des coordonnees les plus proche de la coordonnee
	vector<size_t> CalculDistance(const vector<COORDONNEE>& coordonnees, const COORDONNEE& coordonnee)
	{
		vector<double> vDistance;
		vector<size_t> vIndex;
		double distance;
		size_t index, nbCoord, i, j, indexTemp;

		indexTemp = 0;

		nbCoord = coordonnees.size();

		//calcule les distances
		for(index=0; index!=nbCoord; index++)
		{
			distance = sqrt(pow(coordonnee.PrendreX() - coordonnees[index].PrendreX(), 2.0) + pow(coordonnee.PrendreY() - coordonnees[index].PrendreY(), 2.0));
			vDistance.push_back(distance);
		}

		//trie en ordre croissant du plus pres au plus loin
		while(vIndex.size() != nbCoord)
		{
			distance = -1.0;

			for(i=0; i!=nbCoord; i++)
			{
				//verifie si l'index a deja ete ajoute
				for(j=0; j!=vIndex.size(); j++)
				{
					if(i == vIndex[j])
						break;
				}

				if(j == vIndex.size())	//l'index n'a pas ete trouve dans vIndex
				{
					if(distance == -1.0 || distance > vDistance[i])
					{
						distance = vDistance[i];
						indexTemp = i;
					}
				}				
			}

			vIndex.push_back(indexTemp);
		}

		return vIndex;
	}

	
	//---------------------------------------------------------------------------------------------------------------------
	void CalculDistanceEx(const vector<COORDONNEE>& coordonnees, const COORDONNEE& coordonnee, vector<double>* vDistances)
	{
		double distance;
		size_t index;

		vDistances->clear();

		for(index=0; index<coordonnees.size(); index++)
		{
			distance = sqrt(pow(coordonnee.PrendreX() - coordonnees[index].PrendreX(), 2) + pow(coordonnee.PrendreY() - coordonnees[index].PrendreY(), 2));
			vDistances->push_back(distance);
		}
	}


	float CalculDensiteNeige(float temperature)
	{
		if (temperature < -17.0f)
			return 50.0f;
		else if (temperature > 0.0f)
			return 150.0f;

		return 151.0f + 10.63f * temperature + 0.2767f * temperature * temperature;
	}

	double CalculDensiteNeige(double temperature)
	{
		if (temperature < -17.0)
			return 50.0;
		else if (temperature > 0.0)
			return 150.0;

		return 151.0 + 10.63 * temperature + 0.2767 * temperature * temperature;
	}


	string RepertoireRelatif(const string& c1, const string& c2)
	{
		boost::filesystem::path p1(c1);
		boost::filesystem::path p2(c2);

		boost::filesystem::path::iterator i1 = begin(p1);
		boost::filesystem::path::iterator i2 = begin(p2);

		boost::filesystem::path rel;

		while (i1 != end(p1) && i2 != end(p2) && *i1 == *i2)
		{
			rel /= *i1;
			++i1;
			++i2;
		}

		return rel.generic_string();
	}

	string PrendreRepertoireRelatif(const string& repertoire, const string& nom_fichier)
	{
		boost::filesystem::path p1( RepertoireRelatif(repertoire, nom_fichier) );
		boost::filesystem::path p2(nom_fichier);

		boost::filesystem::path::iterator i1 = begin(p1);
		boost::filesystem::path::iterator i2 = begin(p2);

		while (i1 != end(p1) && i2 != end(p2) && *i1 == *i2)
		{
			++i1;
			++i2;
		}

		boost::filesystem::path p3;

		while (i2 != end(p2))
		{
			p3 /= *i2;
			++i2;
		}

		return p3.generic_string();
	}

	float InterpolationLineaire(float x1, float y1, float x2, float y2, float x)
	{
		BOOST_ASSERT(x2 > x1);

		float a = (y2 - y1) / (x2 - x1);

		if (x <= x1) 
			return y1 - a * (x1 - x);
		else if (x >= x2) 
			return y2 + a * (x - x2);
		else 
			return a * (x - x1) + y1;
	}

	void CreeRepertoire(const string& repertoire)
	{
		if (!boost::filesystem::exists(repertoire))
		{
			try{
			boost::filesystem::create_directories(repertoire);
			}
			catch (...)
			{
				throw ERREUR_CREE_REPERTOIRES(repertoire);
			}
		}
	}

	void CarreauAval(int ligne, int colonne, int orientation, int& ligne_aval, int& colonne_aval)
	{
		switch(orientation)
		{
		// Est.
		case 1: ligne_aval = ligne;
			colonne_aval = colonne+1;
			break;

		// Nord-Est.
		case 2: ligne_aval = ligne-1;
			colonne_aval = colonne+1;
			break;

		// Nord.
		case 3: ligne_aval = ligne-1;
			colonne_aval = colonne;
			break;

		// Nord-Ouest.
		case 4: ligne_aval = ligne-1;
			colonne_aval = colonne-1;
			break;

		// Ouest.
		case 5: ligne_aval = ligne;
			colonne_aval = colonne-1;
			break;

		// Sud-Ouest.
		case 6: ligne_aval = ligne+1;
			colonne_aval = colonne-1;
			break;

		// Sud.
		case 7: ligne_aval = ligne+1;
			colonne_aval = colonne;
			break;

		// Sud-Est.
		case 8: ligne_aval = ligne+1;
			colonne_aval = colonne+1;
			break;

		default: 
			throw ERREUR("CarreauAval() orientation invalide");
		}
	}
	
	void CarreauAval2(size_t ligne, size_t colonne, int orientation, size_t& ligne_aval, size_t& colonne_aval)
	{
		switch(orientation)
		{
			// Est.
		case 1: ligne_aval = ligne;
			colonne_aval = colonne+1;
			break;

			// Nord-Est.
		case 2: ligne_aval = ligne-1;
			colonne_aval = colonne+1;
			break;

			// Nord.
		case 3: ligne_aval = ligne-1;
			colonne_aval = colonne;
			break;

			// Nord-Ouest.
		case 4: ligne_aval = ligne-1;
			colonne_aval = colonne-1;
			break;

			// Ouest.
		case 5: ligne_aval = ligne;
			colonne_aval = colonne-1;
			break;

			// Sud-Ouest.
		case 6: ligne_aval = ligne+1;
			colonne_aval = colonne-1;
			break;

			// Sud.
		case 7: ligne_aval = ligne+1;
			colonne_aval = colonne;
			break;

			// Sud-Est.
		case 8: ligne_aval = ligne+1;
			colonne_aval = colonne+1;
			break;

		default: 
			throw ERREUR("CarreauAval() orientation invalide");
		}
	}


	void CopieRepertoire(const string& src, const string& dst)
	{
		boost::filesystem::path p(src);
		for (auto iter = boost::filesystem::directory_iterator(p); iter != boost::filesystem::directory_iterator(); ++iter)
		{
			if (boost::filesystem::is_regular_file(*iter))
			{
				boost::filesystem::path to( Combine(dst, iter->path().filename().string()) );
				boost::filesystem::copy_file(*iter, to, boost::filesystem::copy_options::overwrite_existing);
			}
		}
	}


	//------------------------------------------------------------------------------------------------
	bool DeleteFolderContent(string sDirectory)
	{
		try{

		boost::filesystem::directory_iterator iter;

		boost::filesystem::path p(sDirectory);
		for (iter = boost::filesystem::directory_iterator(p); iter!=boost::filesystem::directory_iterator(); iter++)
		{
			if (boost::filesystem::is_regular_file(*iter))
				boost::filesystem::remove(iter->path().string());
		}


		}
		catch(...)
		{
			return false;
		}

		return true;
	}


	//----------------------------------------------------------------------------------------------------------------
	//pvExcludedFolderName; nom des dossier a exclure (doit etre en lower case)
	bool CopieRepertoireRecursive(boost::filesystem::path const & source, 
									boost::filesystem::path const & destination, vector<string>* pvExcludedFolderName)
	{
		namespace fs = boost::filesystem;
		string tmp;

		try
		{
			if(!fs::exists(source) || !fs::is_directory(source))
			{
				//std::cerr << "Source directory " << source.string() << " does not exist or is not a directory." << '\n';
				return false;
			}
			if(fs::exists(destination))
			{
				if(!fs::is_empty(destination))
				{
					//std::cerr << "Destination directory " << destination.string() << " already exists." << '\n';
					return false;
				}
			}
			else
			{
				if(!fs::create_directory(destination))
				{
					//std::cerr << "Unable to create destination directory" << destination.string() << '\n';
					return false;
				}
			}
		}

		//catch(const fs::filesystem_error& e)
		catch(...)
		{
			//std::cerr << e.what() << '\n';
			return false;
		}

		// Iterate through the source directory
		for(fs::directory_iterator file(source); file != fs::directory_iterator(); ++file)
		{
			try
			{
				fs::path current(file->path());
				if(fs::is_directory(current))
				{
					if(pvExcludedFolderName != NULL)
					{
						tmp = current.filename().string();
						boost::algorithm::to_lower(tmp);

						if(find(begin(*pvExcludedFolderName), end(*pvExcludedFolderName), tmp) == end(*pvExcludedFolderName))
						{
							if(!CopieRepertoireRecursive(current, destination / current.filename()))
								return false;
						}
					}
					else
					{
						if(!CopieRepertoireRecursive(current, destination / current.filename()))
							return false;
					}
				}
				else
					fs::copy_file(current, destination / current.filename());
			}

			//catch(const fs::filesystem_error& e)
			catch(...)
			{
				//std:: cerr << e.what() << '\n';
				return false;
			}
		}

		return true;
	}

	void Copie(const string& src, const string& dst)
	{
		boost::filesystem::copy_file(src, dst);
	}

	string RemplaceRepertoire(const string& nom_fichier, const string& repertoire)
	{
		string s = ExtraitNomFichier(nom_fichier) + PrendreExtension(nom_fichier);
		return Combine(repertoire, s);
	}


	istream& getline_mod(istream& stream, string& line)
	{
		string str;
		getline(stream, str);

		if(str.size() != 0 && str[str.size()-1] == '\r')				//remove any trailing \r (for unix to correctly read windows file) (LF vs CRLF end of line)
			str.resize(str.size()-1);

		if(str.size() != 0)
		{
			replace(str.begin(), str.end(), '\\', '/');					//replace \ to / for compatibility between windows and unix, / are ok for windows but \ are not ok for unix...
			line = str;
		}
		else
			line = "";

		return stream;
	}


	void lire_cle_valeur(istream& stream, string& cle, string& valeur)
	{
		string str, str2;

		//cle
		getline(stream, str, ';');

		while(str.size() != 0 && (str[0] == '\r' || str[0] == '\n'))	//remove any preceding \r & \n
			str.erase(0, 1);											//happen when reading empty line in the file. windows dont care of the preceding endline char when comparing strings but unix do care
																		//this happen because of the way the function read values: getline(stream, str, ';')
		if(str.size() != 0)
			cle = str;
		else
			cle = "";

		//valeur
		getline(stream, str2, '\n');

		if(str2.size() != 0 && str2[str2.size()-1] == '\r')				//remove any trailing \r (for unix to correctly read windows file) (LF vs CRLF end of line)
			str2.resize(str2.size()-1);

		if(str2.size() != 0)
		{
			replace(str2.begin(), str2.end(), '\\', '/');
			valeur = str2;
		}
		else
			valeur = "";
	}


	bool lire_cle_valeur_try(istream& stream, string& cle, string& valeur)
	{
		bool ret = true;

		try{
		getline(stream, cle, ';');
		while(cle.size() != 0 && (cle[0] == '\r' || cle[0] == '\n'))	//remove any preceding \r & \n
			cle.erase(0, 1);											//happen when reading empty line in the file. windows dont care of the preceding endline char when comparing strings but unix do care
																		//this happen because of the way the function read values: getline(stream, str, ';')
		getline(stream, valeur, '\n');
		if(valeur.size() != 0 && valeur[valeur.size()-1] == '\r')		//remove any trailing \r (for unix to correctly read windows file) (LF vs CRLF end of line)
			valeur.resize(valeur.size()-1);

		replace(valeur.begin(), valeur.end(), '\\', '/');
		}
		catch(...)
		{
			ret = false;
		}
		return ret;
	}


	void lire_cle_valeur(string& ligne, string& cle, string& valeur)
	{
		vector<string> sList;
		size_t i;

		cle = "";
		valeur = "";

		SplitString(sList, ligne, ";", true, true);
		if(sList.size() > 0)
		{
			cle = sList[0];
			if(sList.size() > 1)
			{
				for(i=1; i<sList.size(); i++)
					valeur+= sList[i] + ";";
				valeur = valeur.substr(0, valeur.size()-1);	//supprime le dernier ';'

				replace(valeur.begin(), valeur.end(), '\\', '/');
			}
		}
	}


	//------------------------------------------------------------------------------------------------
	//retourne les valeurs (size_t) en les diminuant de 1 (zero-based index)
	vector<size_t> extrait_valeur(const string& csv)
	{
		istringstream iss;
		vector<size_t> valeurs;
		vector<string> sVal;
		size_t x, stVal;

		valeurs.clear();

		SplitString2(sVal, csv, ";", true);
		for(x=0; x<sVal.size(); x++)
		{
			iss.clear();
			iss.str(sVal[x]);
			iss >> stVal;

			valeurs.push_back( (stVal - 1) ); // zero base index
		}

		return valeurs;
	}

	
	//-------------------------------------------------------------------------------
	vector<size_t> extrait_svaleur(const string& csv,  const std::string& separator)
	{
		vector<size_t> valeurs;
		vector<string> sVal;
		istringstream iss;
		size_t x, val;

		valeurs.clear();
		SplitString2(sVal, csv, separator, true);

		for(x=0; x<sVal.size(); x++)
		{
			iss.clear();
			iss.str(sVal[x]);
			iss >> val;

			valeurs.push_back(val);
		}

		return valeurs;
	}


	//------------------------------------------------------------------------------
	vector<float> extrait_fvaleur(const string& csv,  const std::string& separator)
	{
		vector<float> valeurs;
		vector<string> sVal;
		istringstream iss;
		size_t x;
		string str;
		float val;

		valeurs.clear();
		SplitString(sVal, csv, separator, true, true);

		for(x=0; x<sVal.size(); x++)
		{
			str = TrimString(sVal[x]);

			iss.clear();
			iss.str(str);
			iss >> val;

			valeurs.push_back(val);
		}

		return valeurs;
	}


	//------------------------------------------------------------------------------
	vector<double> extrait_dvaleur(const string& csv, const std::string& separator)
	{
		vector<double> valeurs;
		vector<string> sVal;
		istringstream iss;
		size_t x;
		string str;
		double val;

		valeurs.clear();
		SplitString(sVal, csv, separator, true, true);

		for(x=0; x<sVal.size(); x++)
		{
			str = TrimString(sVal[x]);

			iss.clear();
			iss.str(str);
			iss >> val;

			valeurs.push_back(val);
		}

		return valeurs;
	}


	//-----------------------------------------------------
	vector<string> extrait_stringValeur(const string& csv)
	{
		vector<string> sVal;

		SplitString2(sVal, csv, ";", true);
		return sVal;
	}


	vector<string> extrait_stringValeur(const string& csv, const std::string& separator)
	{
		vector<string> sVal;

		SplitString2(sVal, csv, separator, true);
		return sVal;
	}


	double string_to_double(const string& s)
	{
		istringstream iss(s);
		double valeur;
		iss >> valeur;
		return valeur;
	}

	int string_to_int(const string& s)
	{
		istringstream iss(s);
		int valeur;
		iss >> valeur;
		return valeur;
	}

	unsigned short string_to_ushort(const string& s)
	{
		istringstream iss(s);
		unsigned short valeur;
		iss >> valeur;
		return valeur;
	}


	bool AlmostEqual(double a, double b, double epsilon)
	{
		return abs(a - b) < epsilon;
	}

	bool Racine(const string& nom_fichier)
	{
		boost::filesystem::path p(nom_fichier);
		return p.is_absolute();
	}
	
	string Combine(const string& racine, const string& chemin)
	{
		string str;

		boost::filesystem::path p(racine);
		p /= chemin;

		str = p.string();
		replace(str.begin(), str.end(), '\\', '/');

		return str; 
	}


	RASTER<float> LectureRaster_float(const string& nom_fichier, float mult)
	{
		string extension = PrendreExtension(nom_fichier);

		if (extension == ".mna" || extension == ".pte")
		{
			return LectureRasterPhysitel_float(nom_fichier, mult);
		}
		else if (extension == ".tif")
		{
			RASTER<float> raster = ReadGeoTIFF_float(nom_fichier);

			if (mult != 1)
			{
				for (size_t lig = 0; lig < raster.PrendreNbLigne(); ++lig)
				{
					for (size_t col = 0; col < raster.PrendreNbColonne(); ++col)
					{
						raster(lig, col) *= mult;
					}
				}
			}

			return raster;
		}
		else
		{
			throw ERREUR_LECTURE_FICHIER(nom_fichier);
		}
	}

	RASTER<int> LectureRaster_int(const string& nom_fichier)
	{
		string extension = PrendreExtension(nom_fichier);

		if (extension == ".ori" || extension == ".uh")
		{
			return LectureRasterPhysitel_int(nom_fichier);
		}
		else if (extension == ".tif")
		{
			return ReadGeoTIFF_int(nom_fichier);
		}
		else
		{
			throw ERREUR_LECTURE_FICHIER(nom_fichier);
		}
	}


	string GetTempFilename()
	{
		return boost::filesystem::unique_path().string();
	}


	//------------------------------------------------------------------------------------------
	void SplitString(std::vector<std::string>& sList, const std::string& input, 
						const std::string& separators, bool remove_empty, bool bReplaceVirgule)
	{
		ostringstream sString;
		string sTemp;

		sList.clear();
		for(size_t x = 0; x < input.size(); ++x)
		{
			if(string::npos == separators.find(input[x]))
				sString << input[x];
			else
			{
				if(!sString.str().empty() || !remove_empty)
				{
					sTemp = sString.str();
					if(bReplaceVirgule)
						replace(sTemp.begin(), sTemp.end(), ',', '.');
					sList.push_back(sTemp);
				}
				sString.str("");
			}
		}

		if(!sString.str().empty() || !remove_empty)
		{
			sTemp = sString.str();
			if(bReplaceVirgule)
				replace(sTemp.begin(), sTemp.end(), ',', '.');
			sList.push_back(sTemp); 
		}
	}

	//----------------------------------------------------------------------------------------------
	//Fait un trim sur les strings lues
	void SplitString2(std::vector<std::string>& sList, 
						const std::string& input, const std::string& separators, bool remove_empty)
	{
		ostringstream sString;
		string sTemp;

		sList.clear();
		for(size_t x = 0; x < input.size(); ++x)
		{
			if(string::npos == separators.find(input[x]))
				sString << input[x];
			else
			{
				sTemp = TrimString(sString.str());
				if(!sTemp.empty() || !remove_empty)
					sList.push_back(sTemp);

				sString.str("");
			}
		}

		sTemp = TrimString(sString.str());
		if(!sTemp.empty() || !remove_empty)
			sList.push_back(sTemp); 
	}

	//------------------------------------------------------------------------------------------------
	//Trim string leading and trailing whitespaces
	string TrimString(string str)
	{
		if(str.length() > 0)
		{
			size_t first, last;

			first = str.find_first_not_of(' ');
			if(first == string::npos)
				str = "";
			else
			{
				last = str.find_last_not_of(' ');
				str = str.substr(first, (last-first+1));
			}
		}

		return str;
	}


	string ValidateInputFilesCharacters(vector<string> &listInputFiles, vector<string> &listErrMessCharValidation)
	{
		vector<string> listChar;
		ostringstream oss;
		ifstream file;
		string ret, str, str2;
		size_t i, j, k, line;

		listErrMessCharValidation.clear();
		ret = "";

		try{

		for(i=0; i!=listInputFiles.size(); i++)
		{
			if(boost::filesystem::exists(listInputFiles[i]))
			{
				file.open(listInputFiles[i], ios_base::in);

				line = 0;
				while(!file.eof())
				{
					++line;

					getline_mod(file, str);

					for(j=0; j!=str.size(); j++)
					{
						if( (str[j] < 32 || str[j] > 126) && str[j] != 9)	//exclude code 9 -> TABULATION
						{																									//exclude comments in some files
							if( !(listInputFiles[i].size() > 4 && listInputFiles[i].substr(listInputFiles[i].size()-4) == ".stm" && line == 3) &&							//.stm (line 3)
								!(listInputFiles[i].size() > 4 && listInputFiles[i].substr(listInputFiles[i].size()-4) == ".sth" && line == 3) &&							//.sth (line 3)
								!(listInputFiles[i].size() > 10 && listInputFiles[i].substr(listInputFiles[i].size()-11) == "troncon.trl" && line == 3) &&					//troncon.trl (line 3)
								!(listInputFiles[i].size() > 17 && listInputFiles[i].substr(listInputFiles[i].size()-18) == "occupation_sol.cla" && line == 3) && 			//occupation_sol.cla
								!(listInputFiles[i].size() > 4 && listInputFiles[i].substr(listInputFiles[i].size()-4) == ".sol" && line == 3) && 							//.sol (line 3)
								!(listInputFiles[i].size() > 10 && listInputFiles[i].substr(listInputFiles[i].size()-11) == "ind_fol.def" && (line == 3 || line == 4)) && 	//ind_fol.def (line 3 & 4)
								!(listInputFiles[i].size() > 10 && listInputFiles[i].substr(listInputFiles[i].size()-11) == "pro_rac.def" && (line == 3 || line == 4)) )	//ind_fol.def (line 3 & 4)
							{
								//invalid character
								str2 = str[j];
								k = std::find(listChar.begin(), listChar.end(), str2) - listChar.begin();	//get index

								if(k >= listChar.size()) //not found
								{
									listChar.push_back(str2);

									oss.str("");
								
									oss << "Invalid character in file: " << listInputFiles[i] << " (line " << line << ", col " << (j+1) << ")";								
									//int code = str[j];
									//oss << "Invalid character: " << str2 << ": code " << code << ": in file: " << listInputFiles[i] << " (line " << line << ", col " << (j+1) << ")";

									listErrMessCharValidation.push_back(oss.str());
								}
							}
						}
					}
				}

				file.close();
				file.clear();
			}
		}

		}
		catch(const exception& ex)
		{
			if(file && file.is_open())
				file.close();

			ret = "error validating input files: exception: ";
			ret+= ex.what();
		}

		return ret;
	}


	string GetCurrentTimeStr()
	{
		string ret;
		char buf[20];

		try{
		chrono::system_clock::time_point tp = chrono::system_clock::now();
		time_t tt = chrono::system_clock::to_time_t(tp);
		tm* ptm = std::localtime(&tt);
		std::strftime(buf, 20, "%Y-%m-%d %H:%M:%S", ptm);
		ret = buf;

		}
		catch(...)
		{
			ret = "";
		}

		return ret;
	}


	string GetCurrentTimeStrForFile()
	{
		string ret;
		char buf[16];

		try{
			chrono::system_clock::time_point tp = chrono::system_clock::now();
			time_t tt = chrono::system_clock::to_time_t(tp);
			tm* ptm = std::localtime(&tt);
			std::strftime(buf, 16, "%Y%m%d-%H%M%S", ptm);
			ret = buf;

		}
		catch(...)
		{
			ret = "";
		}

		return ret;
	}


	void Log(string sLog)
	{
		_listLog.push_back(sLog);

		if(sLog == "")
			std::cout << endl;
		else
			std::cout << sLog << endl;
	}


	string ReadParameterFile(string sPathFile, std::map<string, string>& mapParam)
	{
		vector<string> sList;
		ifstream file;
		string ret, str;

		ret = "";
		mapParam.clear();

		file.open(sPathFile, ios_base::in);
		if(file)
		{
			try{
			while(!file.eof())
			{
				getline_mod(file, str);
				boost::trim(str);

				if(str.size() > 2 && !(str[0] == '/' && str[1] == '/'))
				{
					SplitString(sList, str, ";", false, true);

					boost::trim(sList[0]);
					if(sList[0] != "")
					{
						boost::algorithm::to_upper(sList[0]);

						if(sList.size() == 1)
							mapParam[sList[0]] = "";
						else
						{
							boost::trim(sList[1]);
							mapParam[sList[0]] = sList[1];
						}
					}
				}
			}

			file.close();
			}
			catch(const exception& ex)
			{
				if(file && file.is_open())
					file.close();
				str = ex.what();
				ret = "error reading file: exception: " + str + ": " + sPathFile;
			}
		}
		else
			ret = "error opening file: " + sPathFile;

		return ret;
	}


	//--------------------------------------------------------------------------------------------------------------
	//NetCDF


	//------------------------------------------------------------------------------------------------
	//Pour type == GRID
	//Format 9.3.1

	string LectureFormatNetCDFTypeGrid(DATE_HEURE* pDateDebutSim, DATE_HEURE* pDateFinSim, size_t simulationTimestep, 
										string sPathFile, string sTimeVar, string sLonVar, string sLatVar, 
										vector<string> vValVar, vector<double*> pVal, vector<shared_ptr<STATION>>& stationsInterpol, 
										double dExtentLimitNorth, double dExtentLimitSouth, double dExtentLimitEast, double dExtentLimitWest)
	{
		unsigned short yy, mm, dd, hh, min, ss;
		istringstream iss;
		ostringstream oss;
		vector<int> vVarId;
		DATE_HEURE dtDebutFichier;
		DATE_HEURE dtFinFichier;
		DATE_HEURE dtTimeUnit;
		DATE_HEURE dateDebutVecteur;
		DATE_HEURE dt;
		PROJECTION projection;
		size_t lNbPasTempsFichier, i, j, k, indexDebut, indexFin;
		size_t lNbLat, lNbLong, lPasTemps, lNbPasTemps;
		size_t lNbCoord;	//(lNbLat*lNbLong)
		double dVal;
		string ret, str1, str2, str3;
		bool bMinutesUnit;
		int iNcid, iRet, iVal;
		int latid, lonid, timedimid, timeid, iLatDimID, iLonDimID;

		ret = "";

		stationsInterpol.clear();

		oss.str("");

		iRet = nc_open(sPathFile.c_str(), NC_NOWRITE, &iNcid);
		if(iRet != NC_NOERR)
		{
			oss << iRet;
			ret = "error opening NetCDF file: " + sPathFile + ": nc_open return code " + oss.str() + ".";
			return ret;
		}

		//valide l'unité des pas de temps
		//l'unité doit etre "days since yyyy-mm-dd hh:00:00" ou "minutes since yyyy-mm-dd hh:00:00"
		iRet = nc_inq_varid(iNcid, sTimeVar.c_str(), &timeid);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": time variable `" + sTimeVar.c_str() + "` not found.";
			return ret;
		}

		iRet = nc_inq_attlen(iNcid, timeid, "units", &i);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile	 + ": time variable `" + sTimeVar.c_str() + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`.";
			return ret;
		}

		boost::shared_array<char> str_att(new char[i+1]);
		iRet = nc_get_att(iNcid, timeid, "units", reinterpret_cast<void*>(&str_att[0]));
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": time variable `" + sTimeVar.c_str() + "` must have a `units` attribute equal to `minutes since 1970-01-01 00:00:00` or `days since 1970-01-01 00:00:00`.";
			return ret;
		}

		str1 = str_att.get();
		str2 = str1.substr(0, i);
		boost::algorithm::to_lower(str2);

		if(str2.substr(0, 14) == "minutes since " && str2.length() == 33)
		{
			bMinutesUnit = true;	//"minutes since 1970-01-01 00:00:00"

			str3 = str2.substr(14);

			iss.str(str3.substr(0, 4));
			iss >> yy;
			iss.clear();
			iss.str(str3.substr(5, 2));
			iss >> mm;
			iss.clear();
			iss.str(str3.substr(8, 2));
			iss >> dd;
			iss.clear();
			iss.str(str3.substr(11, 2));
			iss >> hh;
			iss.clear();
			iss.str(str3.substr(14, 2));
			iss >> min;
			iss.clear();
			iss.str(str3.substr(17, 2));
			iss >> ss;

			if(min != 0 || ss != 0)
			{
				nc_close(iNcid);
				ret = "error reading NetCDF file: " + sPathFile + ": invalid time units: minutes and seconds must be 0.";
				return ret;
			}

			dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
		}
		else
		{
			if(str2.substr(0, 11) == "days since " && str2.length() == 30)
			{
				bMinutesUnit = false;	//"days since 1970-01-01 00:00:00"

				str3 = str2.substr(11);

				iss.str(str3.substr(0, 4));
				iss >> yy;
				iss.clear();
				iss.str(str3.substr(5, 2));
				iss >> mm;
				iss.clear();
				iss.str(str3.substr(8, 2));
				iss >> dd;
				iss.clear();
				iss.str(str3.substr(11, 2));
				iss >> hh;
				iss.clear();
				iss.str(str3.substr(14, 2));
				iss >> min;
				iss.clear();
				iss.str(str3.substr(17, 2));
				iss >> ss;

				if(min != 0 || ss != 0)
				{
					nc_close(iNcid);
					ret = "error reading NetCDF file: " + sPathFile + ": invalid time units: minutes and seconds must be 0.";
					return ret;
				}

				dtTimeUnit = DATE_HEURE(yy, mm, dd, hh);
			}
			else
			{
				nc_close(iNcid);
				ret = "error reading NetCDF file: " + sPathFile + ": invalid time units: must be `minutes since yyyy-mm-dd hh:00:00` or `days since yyyy-mm-dd hh:00:00`" + ".";
				return ret;
			}
		}

		//lecture longitude et latitude dimensions
		iRet = nc_inq_dimid(iNcid, sLatVar.c_str(), &iLatDimID);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": latitude dimension `" + sLatVar.c_str() + "` not found.";
			return ret;
		}

		iRet = nc_inq_dimlen(iNcid, iLatDimID, &lNbLat);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			oss << iRet;
			ret = "error reading NetCDF file: " + sPathFile + ": error reading latitude dimension length: error code " + oss.str() + ".";
			return ret;
		}

		iRet = nc_inq_dimid(iNcid, sLonVar.c_str(), &iLonDimID);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": longitude dimension `" + sLonVar.c_str() + "` not found.";
			return ret;
		}

		iRet = nc_inq_dimlen(iNcid, iLonDimID, &lNbLong);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			oss << iRet;
			ret = "error reading NetCDF file: " + sPathFile + ": error reading longitude dimension length: error code " + oss.str() + ".";
			return ret;
		}

		lNbCoord = lNbLat * lNbLong;
		
		//lecture variables ids
		iRet = nc_inq_varid(iNcid, sLatVar.c_str(), &latid);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": latitude variable `" + sLatVar.c_str() + "` not found.";
			return ret;
		}

		iRet = nc_inq_varid(iNcid, sLonVar.c_str(), &lonid);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": longitude variable `" + sLonVar.c_str() + "` not found.";
			return ret;
		}

		for(i=0; i!=vValVar.size(); i++) //pour chaque variables à lire
		{
			iRet = nc_inq_varid(iNcid, vValVar[i].c_str(), &iVal);
			vVarId.push_back(iVal);
			if(iRet != NC_NOERR)
			{
				nc_close(iNcid);
				ret = "error reading NetCDF file: " + sPathFile + ": tmax variable `" + vValVar[i].c_str() + "` not found.";
				return ret;
			}
		}

		//lecture des coordonnees et elevations
		projection = PROJECTIONS::LONGLAT_WGS84();

		vector<double> latitudes(lNbLat);
		iRet = nc_get_var_double(iNcid, latid, &latitudes[0]);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			oss << iRet;
			ret = "error reading NetCDF file: " + sPathFile + ": error reading latitude data: error code " + oss.str() + ".";
			return ret;
		}

		vector<double> longitudes(lNbLong);
		iRet = nc_get_var_double(iNcid, lonid, &longitudes[0]);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			oss << iRet;
			ret = "error reading NetCDF file: " + sPathFile + ": error reading longitude data: error code " + oss.str() + ".";
			return ret;
		}

		double* elevations = NULL;
		size_t start2[] = { 0, 0 };	//row, col	//y, x
		size_t count2[] = { lNbLat, lNbLong };

		//lecture des pas de temps
		iRet = nc_inq_dimid(iNcid, sTimeVar.c_str(), &timedimid);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": time dimension `" + sTimeVar.c_str() + "` not found.";
			return ret;
		}

		iRet = nc_inq_dimlen(iNcid, timedimid, &lNbPasTempsFichier);
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			oss << iRet;
			ret = "error reading NetCDF file: " + sPathFile + ": error reading time dimension length: error code " + oss.str() + ".";
			return ret;
		}

		if(lNbPasTempsFichier < 2)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": invalid timestep count.";
			return ret;
		}

		vector<double> dTimes;
		vector<int> iTimes;

		if(bMinutesUnit)
		{
			iTimes.resize(lNbPasTempsFichier);
			iRet = nc_get_var_int(iNcid, timeid, &iTimes[0]);
		}
		else
		{
			dTimes.resize(lNbPasTempsFichier);
			iRet = nc_get_var_double(iNcid, timeid, &dTimes[0]);
		}
		
		if(iRet != NC_NOERR)
		{
			nc_close(iNcid);
			oss << iRet;
			ret = "error reading NetCDF file: " + sPathFile + ": error reading time data: error code " + oss.str() + ".";
			return ret;
		}

		//determine dates debut et fin du fichier
		if(bMinutesUnit)
			lPasTemps = static_cast<size_t>((iTimes[1] - iTimes[0]) / 60);		//time step [hrs]
		else
			lPasTemps = static_cast<size_t>((dTimes[1] - dTimes[0]) * 24.0);	//

		if(!bMinutesUnit && (lPasTemps != simulationTimestep))	//only for days unit because of double precision
		{
			//on passe d'une valeur en jours (epoch time) vers une valeur en heures
			//essaie d'ajouter ou d'enlever 1 sec pour corriger les problemes de précision des réels lors de la conversion
			dVal = (dTimes[1] - dTimes[0]) * 24.0 + 0.000011574074074074074074074074074074;	//ajoute 1 sec
			lPasTemps = static_cast<size_t>(dVal);

			if(lPasTemps != simulationTimestep)
			{
				dVal = (dTimes[1] - dTimes[0]) * 24.0 - 0.000011574074074074074074074074074074;	//enleve 1 sec
				lPasTemps = static_cast<size_t>(dVal);
			}
		}

		if(lPasTemps != simulationTimestep)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": timestep of data must be equal to simulation timestep.";
			return ret;
		}

		//
		dtDebutFichier = DATE_HEURE(dtTimeUnit.PrendreAnnee(), dtTimeUnit.PrendreMois(), dtTimeUnit.PrendreJour(), dtTimeUnit.PrendreHeure());

		if(bMinutesUnit)
			iVal = iTimes[0] / 60;	//hrs
		else
			iVal = static_cast<int>(dTimes[0] * 24.0);	//hrs

		if(iVal > 0)
			dtDebutFichier.AdditionHeure(iVal);
		else
		{
			if (iVal < 0)
				dtDebutFichier.SoustraitHeure(abs(iVal));
		}

		//
		dtFinFichier = DATE_HEURE(dtTimeUnit.PrendreAnnee(), dtTimeUnit.PrendreMois(), dtTimeUnit.PrendreJour(), dtTimeUnit.PrendreHeure());

		if(bMinutesUnit)
			iVal = iTimes[lNbPasTempsFichier-1] / 60;	//hrs
		else
			iVal = static_cast<int>(dTimes[lNbPasTempsFichier-1] * 24.0);	//hrs

		if (iVal > 0)
			dtFinFichier.AdditionHeure(iVal);
		else
		{
			if (iVal < 0)
				dtFinFichier.SoustraitHeure(abs(iVal));
		}

		//determine la plage de données a lire selon les date de debut et fin de la simulation

		//date debut
		//met heure debut à 0; //si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		
		dateDebutVecteur = DATE_HEURE(pDateDebutSim->PrendreAnnee(), pDateDebutSim->PrendreMois(), pDateDebutSim->PrendreJour(), 0);

		if(dateDebutVecteur < dtDebutFichier)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": data missing for simulation begin date.";
			return ret;
		}

		indexDebut = dtDebutFichier.NbHeureEntre(dateDebutVecteur) / lPasTemps;

		//date fin
		dt = DATE_HEURE(pDateFinSim->PrendreAnnee(), pDateFinSim->PrendreMois(), pDateFinSim->PrendreJour(), 0); 		
		if(simulationTimestep != 24)
			dt.AdditionHeure(24);	//si pdt < 24 tous les pdt de la derniere journee doivent etre lus pour fonction PrendreTemperatureJournaliere		

		if(dt > dtFinFichier)
		{
			nc_close(iNcid);
			ret = "error reading NetCDF file: " + sPathFile + ": data missing for simulation end date.";
			return ret;
		}

		indexFin = dtDebutFichier.NbHeureEntre(dt) / lPasTemps;

		//heure lu en fin de pas de temps et remise en debut de pas de temps.
		if(simulationTimestep != 24)
			indexDebut = indexDebut + 1;

		lNbPasTemps = indexFin - indexDebut + 1;

		//lit et conserve les donnees en ram
		size_t start[] = { indexDebut, 0, 0 };	//depth, row, col	//time, y, x
		size_t count[] = { lNbPasTemps, lNbLat, lNbLong };

		for(i=0; i!=vValVar.size(); i++) //pour chaque variables à lire
		{
			pVal[i] = new double[lNbPasTemps*lNbCoord];

			iRet = nc_get_vara_double(iNcid, vVarId[i], start, count, &pVal[i][0]);
			if(iRet != NC_NOERR)
			{
				nc_close(iNcid);
				oss << iRet;
				ret = "error reading NetCDF file: " + sPathFile + ": error reading data (" + vValVar[i] + "): error code " + oss.str() + ".";
				return ret;
			}
		}

		//initialisation des objets station
		stationsInterpol.clear();
		k = 0;
		for(i=0; i!=lNbLat; i++)
		{
			for(j=0; j!=lNbLong; j++)
			{
				//pour cadrant nord/west
				if( dExtentLimitNorth == -1.0 || 
						(latitudes[i] <= dExtentLimitNorth && latitudes[i] >= dExtentLimitSouth &&
						 longitudes[j] <= dExtentLimitEast && longitudes[j] >= dExtentLimitWest) )
				{
					shared_ptr<STATION> st = make_shared<STATION_METEO_NETCDF_STATION>(sPathFile, nullptr, i, j);
					//if(_pSimHyd->PrendreNomInterpolationDonnees() == "THIESSEN1" || _pSimHyd->PrendreNomInterpolationDonnees() == "MOYENNE 3 STATIONS1")
					//	st.get()->_iVersionThiessenMoy3Station = 1;
					st.get()->_iVersionThiessenMoy3Station = 2;

					oss.str("");
					oss << "station" << k + 1;

					st->ChangeNom(oss.str());
					st->ChangeIdent(oss.str());
					st->ChangeCoordonnee(COORDONNEE(longitudes[j], latitudes[i], elevations[i * lNbLong + j]));

					stationsInterpol.push_back(st);
					++k;
				}
			}
		}

		iRet = nc_close(iNcid);
		if(iRet != NC_NOERR)
		{
			oss << iRet;
			ret = "error closing NetCDF file: " + sPathFile + ": nc_close error code " + oss.str() + ".";
		}

		return ret;
	}

}

