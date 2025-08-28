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

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;


namespace HYDROTEL 
{

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


}

