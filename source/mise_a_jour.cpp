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

#include "mise_a_jour.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "gdal_util.hpp"
#include "point.hpp"
#include "util.hpp"

#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	vector<int> PrendreZoneAmont(TRONCON* troncon)
	{
		vector<int> zones_amont;

		vector<TRONCON*> troncons;
		troncons.push_back(troncon);

		while (!troncons.empty())
		{
			auto tr = troncons.back();
			troncons.pop_back();

			auto zones = tr->PrendreZonesAmont();
			for (auto z = begin(zones); z != end(zones); ++z)
				zones_amont.push_back((*z)->PrendreIdent());

			auto t = tr->PrendreTronconsAmont();
			troncons.insert(end(troncons), begin(t), end(t));
		}

		return zones_amont;
	}


	void PhysitelRaster2GeoTIFF_float(const string& src, const string& dst)
	{
		RASTER<float> raster = LectureRasterPhysitel_float(src);
		WriteGeoTIFF(raster, dst);
	}


	void PhysitelRaster2GeoTIFF_int(const std::string& src, const std::string& dst, int nodata)
	{
		RASTER<int> raster = LectureRasterPhysitel_int(src);
		WriteGeoTIFF(raster, dst, nodata);
	}


	vector<int> GetIdentLacs(const string& fichier_troncons)
	{
		vector<int> lacs;

		ifstream fichier(fichier_troncons);

		string ligne;
		bool bAncienneVersionTRL = false;
			
		int type, nb_troncon;
		fichier >> type >> nb_troncon;
		
		fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		getline_mod(fichier, ligne);

		for (int n = 0; n < nb_troncon; ++n)
		{
			getline_mod(fichier, ligne);
			istringstream iss(ligne);

			int id;
			iss >> id;

			if(n == 0)
			{
				if(id == 0)
					bAncienneVersionTRL = true;
				else
				{
					//dans le fichier TRL exporté par PHYSITEL4, la 1ere colonne est le ID du troncon, le type la 2e colonne. Le type doit avoir une valeur de 1 ou 2 au lieu de 0 ou 1.
					iss >> id;	//2e colonne
					id--;
				}
			}
			else
			{
				if(!bAncienneVersionTRL)
				{
					iss >> id;	//2e colonne
					id--;
				}
			}

			if (id == TRONCON::LAC || id == TRONCON::LAC_SANS_LAMINAGE)
				lacs.push_back(n + 1);
		}

		return lacs;
	}


	void PhysitelPoint2Shapefile(const string& fichier_points, const string& fichier_troncons, const 
									string& dst_rivieres, const string& dst_lacs, const string& masque)
	{		
		RASTER<int> rmasque = ReadGeoTIFF_int(masque);
		vector<int> ident_lacs = GetIdentLacs(fichier_troncons);

		ifstream fichier(fichier_points);

		fichier.exceptions(ios::failbit | ios::badbit);

		size_t nb_ligne, nb_colonne, nb_point;
		fichier >> nb_ligne >> nb_colonne >> nb_point;

		map<int, vector<POINT>> rivieres;

		RASTER<int> points(rmasque.PrendreCoordonnee(), rmasque.PrendreProjection(), rmasque.PrendreNbLigne(), rmasque.PrendreNbColonne(), rmasque.PrendreTailleCelluleX(), rmasque.PrendreTailleCelluleY());

		for (size_t n = 0; n < nb_point; ++n)
		{
			size_t lig, col;
			int ident;
			fichier >> lig >> col >> ident;

			if (find(begin(ident_lacs), end(ident_lacs), ident) != end(ident_lacs))
				points(lig, col) = -ident;
			else
				rivieres[ident].push_back(POINT(static_cast<int>(col), static_cast<int>(lig)));
		}

		// sauvegarde les lacs en shapefile
		{
			string tmp = Combine(GetTempDirectory(), GetTempFilename()) + ".tif";
			WriteGeoTIFF(points, tmp, 0);

			Polygonize(tmp, dst_lacs);

			SupprimerFichier(tmp);
		}

		// sauvegarde les rivieres en shapefile
		{
			GDALAllRegister();
			OGRRegisterAll();

			GDALDataset* dataset = (GDALDataset*)GDALOpen(masque.c_str(), GA_ReadOnly);
			if(dataset == nullptr)
				throw ERREUR_LECTURE_FICHIER(masque.c_str());

			GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
			if(driver == nullptr)
			{
				GDALClose((GDALDatasetH)dataset);
				throw ERREUR("ESRI Shapefile driver not found");
			}

			GDALDataset* datasource = driver->Create(dst_rivieres.c_str(), 0, 0, 0, GDT_Unknown, NULL);
			if(datasource == nullptr)
			{
				GDALClose((GDALDatasetH)dataset);
				throw ERREUR_ECRITURE_FICHIER(dst_rivieres);
			}

			OGRSpatialReference sr(dataset->GetProjectionRef());

			OGRLayer* layer = datasource->CreateLayer(dst_rivieres.c_str(), &sr, wkbMultiLineString, nullptr); 
			if(layer == nullptr)
			{
				GDALClose((GDALDatasetH)dataset);
				GDALClose((GDALDatasetH)datasource);
				throw ERREUR_ECRITURE_FICHIER(dst_rivieres);
			}

			OGRFieldDefn field("ident", OFTInteger);

			if(layer->CreateField(&field) != OGRERR_NONE)
			{
				GDALClose((GDALDatasetH)dataset);
				GDALClose((GDALDatasetH)datasource);
				throw ERREUR_ECRITURE_FICHIER(dst_rivieres);
			}

			for(auto iter = begin(rivieres); iter != end(rivieres); ++iter)
			{
				OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
				feature->SetField("ident", iter->first);

				OGRErr err;
				OGRLineString line;

				for(auto point = begin(iter->second); point != end(iter->second); ++point)
				{
					COORDONNEE coord = points.LigColVersCoordonnee(point->y, point->x);

					OGRPoint p;
					p.setX(coord.PrendreX());
					p.setY(coord.PrendreY());

					line.addPoint(&p);
				}

				err = feature->SetGeometryDirectly(&line);
				if(err != OGRERR_NONE)
				{
					GDALClose((GDALDatasetH)dataset);
					GDALClose((GDALDatasetH)datasource);
					throw ERREUR_ECRITURE_FICHIER(dst_rivieres);
				}

				err = layer->CreateFeature(feature);
				if(err != OGRERR_NONE)
				{
					GDALClose((GDALDatasetH)dataset);
					GDALClose((GDALDatasetH)datasource);
					throw ERREUR_ECRITURE_FICHIER(dst_rivieres);
				}
			}

			layer->SyncToDisk();

			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)datasource);
		}
	}


	void Reseau2PointRDX(string sPathTiffReseau, string sPathRDX)
	{
		RASTER<int> src = ReadGeoTIFF_int(sPathTiffReseau);

		map<int, vector<std::pair<size_t, size_t>>> mapCell;
		size_t xsize = src.PrendreNbColonne();
		size_t ysize = src.PrendreNbLigne();
		size_t nb, i, j;
		int iNoData = src.PrendreNoData();
		int x, ival, idMax, iNbPoint;

		try{

		ofstream out(sPathRDX);

		idMax = iNbPoint = 0;
		for (j=0; j<ysize; j++)
		{
			for (i=0; i<xsize; i++)
			{
				ival = src(j, i);
				if (ival != iNoData)
				{
					mapCell[ival].push_back(make_pair(j, i));
					++iNbPoint;

					if(ival > idMax)
						idMax = ival;
				}
			}
		}

		ostringstream oss;
		
		oss.clear();
		oss.str("");
        oss << ysize << " " << xsize << " " << iNbPoint;
		out << oss.str() << endl;

		for (x=1; x<=idMax; x++)
		{
			if(mapCell.find(x) != mapCell.end())
			{
				nb = mapCell[x].size();
				for (i=0; i<nb; i++)
				{
					oss.clear();
					oss.str("");
			
					oss << mapCell[x][i].first << " " << mapCell[x][i].second << " " << x;	//y x idTroncon
					out << oss.str() << endl;
				}
			}
		}

		out.close();
		out.clear();

		}
		catch(...)
		{
			throw ERREUR("Error creating rdx file.");
		}
	}


	void PhysitelPoint2GeoTIFF(const string& fichier_points, const string& fichier_troncons, const string& dst, const string& masque)
	{
		RASTER<int> rmasque = PrendreExtension(masque) == ".uh" ? LectureRasterPhysitel_int(masque) : LectureRaster_int(masque);

		vector<int> lacs = GetIdentLacs(fichier_troncons);

		ifstream fichier(fichier_points);

		fichier.exceptions(ios::failbit | ios::badbit);

		size_t nb_ligne, nb_colonne, nb_point;
		fichier >> nb_ligne >> nb_colonne >> nb_point;

		RASTER<int> points(rmasque.PrendreCoordonnee(), rmasque.PrendreProjection(), rmasque.PrendreNbLigne(), rmasque.PrendreNbColonne(), rmasque.PrendreTailleCelluleX(), rmasque.PrendreTailleCelluleY());

		for (size_t n = 0; n < nb_point; ++n)
		{
			size_t lig, col;
			int ident;
			fichier >> lig >> col >> ident;

			if (find(begin(lacs), end(lacs), ident) != end(lacs))
				points(lig, col) = -ident;
			else
				points(lig, col) = ident;
		}

		WriteGeoTIFF(points, dst, 0);
	}


	void MiseAJour(const std::string& fichier_prj, const std::string& repertoire)
	{
		string str;

		string repertoire_src = PrendreRepertoire(fichier_prj);
		string repertoire_projet = repertoire;

		if(!boost::filesystem::is_regular_file(boost::filesystem::path(fichier_prj)))
			throw ERREUR("Error: " + fichier_prj + ": invalid file.");

		str = PrendreExtension(fichier_prj);
		boost::algorithm::to_lower(str);
		if(str != ".prj")
			throw ERREUR("Error: " + fichier_prj + ": the source project file must be a .prj file.");
		
		//erreur si le dossier de destination existe et nest pas vide
		if(boost::filesystem::exists(repertoire_projet) && !boost::filesystem::is_empty(repertoire_projet))
			throw ERREUR("Error: destination folder already exists.");

		string repertoire_simulation = Combine(repertoire_projet, "simulation");
		string repertoire_physitel = Combine(repertoire_projet, "physitel");
		string repertoire_physio = Combine(repertoire_projet, "physio");
		string repertoire_meteo = Combine(repertoire_projet, "meteo");
		string repertoire_hydro = Combine(repertoire_projet, "hydro");
		string repertoire_hgm = Combine(repertoire_projet, "hgm");

		//----------------------
		//CreeRepertoire(repertoire_projet);
		//CreeRepertoire(repertoire_simulation);
		//CreeRepertoire(repertoire_physitel);
		//CreeRepertoire(repertoire_physio);
		//CreeRepertoire(repertoire_meteo);
		//CreeRepertoire(repertoire_hydro);
		//CreeRepertoire(repertoire_hgm);
		//----------------------

		//CreeRepertoire(repertoire_projet);
		//CopieRepertoire(repertoire_src, repertoire_projet);
		//boost::filesystem::copy_directory(repertoire_src, repertoire_projet);
		vector<string> vExclusion;
		vExclusion.push_back("simulation");
		vExclusion.push_back("hgm");

		if(!RepertoireExiste(repertoire_src))
			throw ERREUR("Error: the source project filename is invalid.");

		if(!CopieRepertoireRecursive(repertoire_src, repertoire_projet, &vExclusion))
			throw ERREUR("Error: copying: " + repertoire_src + ": to: " + repertoire_projet);

		//supprime les fichiers copié du dossier de projet (ex; .prj, etc...)
		boost::filesystem::path source(repertoire_projet);
		for(boost::filesystem::directory_iterator file(source); file != boost::filesystem::directory_iterator(); ++file)
		{
			try
			{
				boost::filesystem::path current(file->path());
				if(!boost::filesystem::is_directory(current))
					boost::filesystem::remove(current);
					
			}
			catch(const boost::filesystem::filesystem_error& e)
			{
				std::cerr << e.what() << endl;
				//return false;
			}
		}

		//cree un repertoire simulation vide
		//boost::filesystem::remove_all(repertoire_simulation);
		CreeRepertoire(repertoire_simulation);		

		//cree un repertoire hgm vide
		//boost::filesystem::remove_all(repertoire_hgm);
		CreeRepertoire(repertoire_hgm);		

		// copier les fichiers physitel, physio, meteo et hydro

		//CopieRepertoire( Combine(repertoire_src, "physitel"), repertoire_physitel );
		//CopieRepertoire( Combine(repertoire_src, "physio"), repertoire_physio );
		//CopieRepertoire( Combine(repertoire_src, "meteo"), repertoire_meteo );
		//CopieRepertoire( Combine(repertoire_src, "hydro"), repertoire_hydro );

		// copier le repertoire neige s'il existe
		//string repertoire_neige = Combine(repertoire_projet, "neige");

		//if (ReperpertoireExiste(repertoire_neige))
		//{
		//	CreeRepertoire(repertoire_neige);
		//	CopieRepertoire( Combine(repertoire_src, "neige"), repertoire_neige );
		//}

		//string nom_fichier_warning = Combine(repertoire, "warnings.txt");
		//ofstream fichier_warnings( Combine(repertoire, "warnings.txt") );
		//if (!fichier_warnings)
		//	throw ERREUR_ECRITURE_FICHIER(nom_fichier_warning);

		string nom_fichier_zone = Combine(repertoire_physitel, "uhrh.uh");
		string nom_fichier_altitude = Combine(repertoire_physitel, "altitude.mna");
		string nom_fichier_pente = Combine(repertoire_physitel, "pente.pte");
		string nom_fichier_orientation = Combine(repertoire_physitel, "orientation.ori");
		string nom_fichier_point = Combine(repertoire_physitel, "point.rdx");
		string nom_fichier_troncon = Combine(repertoire_physitel, "troncon.trl");
		string nom_fichier_projection = Combine(repertoire_physitel, "CoordSys.txt");
		string nom_fichier_projection2 = Combine(repertoire_physitel, "projection.prj");
		string nom_fichier_projection3 = Combine(repertoire_physitel, "proj4.txt");

		// on s'assure que les fichiers nécessaire sont existant

		if (!FichierExiste(nom_fichier_zone))
			throw ERREUR("Error: file uhrh.uh not found.");
		if (!FichierExiste(nom_fichier_altitude))
			throw ERREUR("Error: file altitude.mna not found.");
		if (!FichierExiste(nom_fichier_pente))
			throw ERREUR("Error: file pente.pte not found.");
		if (!FichierExiste(nom_fichier_orientation))
			throw ERREUR("Error: file orientation.ori not found.");
		if (!FichierExiste(nom_fichier_troncon))
			throw ERREUR("Error: file troncon.trl not found.");
		if (!FichierExiste(nom_fichier_point))
			throw ERREUR("Error: file point.rdx not found.");
		if (!FichierExiste(nom_fichier_projection) && !FichierExiste(nom_fichier_projection2) && !FichierExiste(nom_fichier_projection3))
			throw ERREUR("Error: the projection file (coordsys.txt, proj4.txt or projection.prj) was not found in the `physitel` folder (coordsys.txt: MapInfo CoordSys, proj4.txt: PROJ4 string, projection.prj: esri).");

		//uhrh; import to GeoTIFF
		PhysitelRaster2GeoTIFF_int(nom_fichier_zone, RemplaceExtension(nom_fichier_zone, "tif"), 0);

#ifdef _TEST_
		string strtemp = nom_fichier_zone;
		strtemp.insert(strtemp.length()-1-3, "_original");	//keep original file
		boost::filesystem::copy_file(RemplaceExtension(nom_fichier_zone, "tif"), RemplaceExtension(strtemp, "tif"));
#endif

		//// extrait le numero de zone UTM de la matrice des altitudes
		//ifstream fichier;
		//int type, coord, zone;
		//fichier.open(nom_fichier_altitude);
		//fichier >> type >> coord >> zone;
		//fichier.close();

		//dem; import to GeoTIFF
		PhysitelRaster2GeoTIFF_float(nom_fichier_altitude, RemplaceExtension(nom_fichier_altitude, "tif"));
		
#ifdef _TEST_
		string strtemp2 = nom_fichier_altitude;
		strtemp2.insert(strtemp2.length()-1-3, "_original");	//keep original file
		boost::filesystem::copy_file(RemplaceExtension(nom_fichier_altitude, "tif"), RemplaceExtension(strtemp2, "tif"));
#endif

		//crop dem map with uhrh map and apply mask (exclude from dem map rows/lines having nodata in uhrh map and pixels with no uhrh value)
		CropRasterFloat(RemplaceExtension(nom_fichier_altitude, "tif"), RemplaceExtension(nom_fichier_zone, "tif"));
		
		//crop uhrh; uhrh map can have nodata lines or rows (exclude from uhrh map rows/lines having nodata in dem map)
		CropRasterInt_FloatMask(RemplaceExtension(nom_fichier_zone, "tif"), RemplaceExtension(nom_fichier_altitude, "tif"));

		RASTER<int> rZones = ReadGeoTIFF_int( RemplaceExtension(nom_fichier_zone, "tif") );
		RASTER<float> altitudes = ReadGeoTIFF_float(RemplaceExtension(nom_fichier_altitude, "tif"));		

		//ensure all uhrh pixels have dem values
		if(rZones.PrendreNbColonne() != altitudes.PrendreNbColonne() || rZones.PrendreNbLigne() != altitudes.PrendreNbLigne() || 
			rZones.PrendreCoordonnee().PrendreX() != altitudes.PrendreCoordonnee().PrendreX() || rZones.PrendreCoordonnee().PrendreY() != altitudes.PrendreCoordonnee().PrendreY())
		{
			throw ERREUR("Error: inconsistency between rhhu and elevation matrix.");
		}

		size_t szlig, szcol;
		float altNodata, somme_alt;
		long lig, col, nLine, nCol, lTemp, cTemp;
		int zoneNodata, nb_pixel;

		altNodata = altitudes.PrendreNoData();
		zoneNodata = rZones.PrendreNoData();
		nLine = static_cast<long>(rZones.PrendreNbLigne());
		nCol = static_cast<long>(rZones.PrendreNbColonne());
		
		for (lig=0; lig<nLine; lig++)
		{
			for (col=0; col<nCol; col++)
			{
				if (rZones(lig, col) != zoneNodata && altitudes(lig, col) == altNodata)
				{
					somme_alt = 0.0f;
					nb_pixel = 0;
		
					for (lTemp=lig-1; lTemp<=lig+1; lTemp++)
					{
						for (cTemp=col-1; cTemp<=col+1; cTemp++)
						{
							if(lTemp > -1 && lTemp < nLine && cTemp > -1 && cTemp < nCol)
							{
								if (altitudes(lTemp, cTemp) != altNodata)
								{
									somme_alt+= altitudes(lTemp, cTemp);
									++nb_pixel;
								}
							}
						}
					}

					if (nb_pixel != 0)
						altitudes(lig, col) = somme_alt / nb_pixel;	//neighboring cells values average
					else
						altitudes(lig, col) = 0;	//value of 0 if all neighboring cells are nodata values
				}
			}
		}

		WriteGeoTIFF(altitudes, RemplaceExtension(nom_fichier_altitude, "tif") );

		PhysitelRaster2GeoTIFF_float(nom_fichier_pente, RemplaceExtension(nom_fichier_pente, "tif"));

#ifdef _TEST_
		string strtemp3 = nom_fichier_pente;
		strtemp3.insert(strtemp3.length()-1-3, "_original");	//keep original file
		boost::filesystem::copy_file(RemplaceExtension(nom_fichier_pente, "tif"), RemplaceExtension(strtemp3, "tif"));
#endif

		//crop slope map with uhrh map and apply mask (exclude from slope map rows/lines having nodata in uhrh map and pixels with no uhrh value)
		CropRasterFloat(RemplaceExtension(nom_fichier_pente, "tif"), RemplaceExtension(nom_fichier_zone, "tif"));

		PhysitelRaster2GeoTIFF_int(nom_fichier_orientation, RemplaceExtension(nom_fichier_orientation, "tif"), 0);
		//crop aspect map with uhrh map and apply mask (exclude from aspect map rows/lines having nodata in uhrh map and pixels with no uhrh value)
		CropRaster(RemplaceExtension(nom_fichier_orientation, "tif"), RemplaceExtension(nom_fichier_zone, "tif"));

		PhysitelPoint2GeoTIFF(nom_fichier_point, nom_fichier_troncon, Combine(repertoire_physitel, "reseau.tif"), nom_fichier_zone);
		//crop network map with uhrh map and apply mask (exclude from network map rows/lines having nodata in uhrh map and pixels with no uhrh value)
		CropRaster(Combine(repertoire_physitel, "reseau.tif"), RemplaceExtension(nom_fichier_zone, "tif"));
		//re-update point.rdx since extent may have change
		Reseau2PointRDX(Combine(repertoire_physitel, "reseau.tif"), nom_fichier_point);

		SIM_HYD sim_hyd;
		sim_hyd._bUpdatingV26Project = true;
		sim_hyd.ChangeNomFichier(fichier_prj);
		
		sim_hyd._sPathProjetImport = nom_fichier_zone.substr(0, nom_fichier_zone.find_last_of('/'));	//workaround pour utiliser les nouvelles carte tif importé plutot que les anciennes (.uh, .mna, etc)
		sim_hyd.Lecture();

		sim_hyd.SauvegardeSous(repertoire_projet);

		ZONES& zones = sim_hyd.PrendreZones();
		zones.ChangeNomFichierZone( RemplaceExtension(nom_fichier_zone, "tif") );
		zones.ChangeNomFichierAltitude( RemplaceExtension(nom_fichier_altitude, "tif") );
		zones.ChangeNomFichierPente( RemplaceExtension(nom_fichier_pente, "tif") );
		zones.ChangeNomFichierOrientation( RemplaceExtension(nom_fichier_orientation, "tif") );

		if(sim_hyd._nom_fichier_milieu_humide_isole != "")
		{
			sim_hyd._nom_fichier_milieu_humide_isole = Combine(sim_hyd.PrendreRepertoireSimulation(), PrendreFilename(sim_hyd._nom_fichier_milieu_humide_isole));
			sim_hyd._bSimuleMHIsole = true;
		}

		if(sim_hyd._nom_fichier_milieu_humide_riverain != "")
		{
			sim_hyd._nom_fichier_milieu_humide_riverain = Combine(sim_hyd.PrendreRepertoireSimulation(), PrendreFilename(sim_hyd._nom_fichier_milieu_humide_riverain));
			sim_hyd._bSimuleMHRiverain = true;
		}

		if(sim_hyd._nom_fichier_milieu_humide_profondeur_troncon != "")
		{
			repertoire_hgm = sim_hyd.PrendreRepertoireProjet() + "/physio";
			sim_hyd._nom_fichier_milieu_humide_profondeur_troncon = Combine(repertoire_hgm, PrendreFilename(sim_hyd._nom_fichier_milieu_humide_profondeur_troncon));
		}
		
		sim_hyd.Sauvegarde();

		// supprime les anciennes matrices binaires
		SupprimerFichier(nom_fichier_zone);
		SupprimerFichier(nom_fichier_altitude);
		SupprimerFichier(nom_fichier_pente);
		SupprimerFichier(nom_fichier_orientation);

		// supprime le fichier de couleur de l'ancienne version
		{
			string nom_fichier_clr = RemplaceExtension(nom_fichier_zone, "clr");
			SupprimerFichier(nom_fichier_clr);
		}

		// supprime le fichier rsm pour le convertir au nouveau format
		{
			string nom_fichier_rsm = RemplaceExtension(nom_fichier_zone, "rsm");
			SupprimerFichier(nom_fichier_rsm);

			zones.LectureZones();
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//creation d'un masque pour le bassin en attendant que la fonction GDALPolygonize soit modifie
		string nom_fichier_masque = Combine(PrendreRepertoire(sim_hyd.PrendreZones().PrendreNomFichierZone()), "masque.tif");
		{
			RASTER<int> zonesras = ReadGeoTIFF_int(sim_hyd.PrendreZones().PrendreNomFichierZone());
			RASTER<int> masque(zonesras.PrendreCoordonnee(), zonesras.PrendreProjection(), zonesras.PrendreNbLigne(), zonesras.PrendreNbColonne(), zonesras.PrendreTailleCelluleX(), zonesras.PrendreTailleCelluleY());

			for (szlig = 0; szlig < zonesras.PrendreNbLigne(); ++szlig)
			{
				for (szcol = 0; szcol < zonesras.PrendreNbColonne(); ++szcol)
				{
					if (zonesras(szlig, szcol) != 0 && zonesras(szlig, szcol) != zonesras.PrendreNoData())
						masque(szlig, szcol) = 1;
				}
			}

			WriteGeoTIFF(masque, nom_fichier_masque, 0);
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		ReseauGeoTIFF2Shapefile(zones.PrendreNomFichierOrientation(), Combine(repertoire_physitel, "reseau.tif"), 
									Combine(repertoire_physitel, "rivieres.shp"), Combine(repertoire_physitel, "lacs.shp"));

		// copie le fichier des groupes d'uhrh dans la simulation
		{
			string nom_fichier_groupe_src = Combine(
				Combine( 
				Combine(repertoire_src, "simulation"), 
				sim_hyd.PrendreNomSimulation()),
				sim_hyd.PrendreNomSimulation())
				+ ".gsb";

			string nom_fichier_groupe_dst = Combine(
				sim_hyd.PrendreRepertoireSimulation(), sim_hyd.PrendreNomSimulation())
				+ ".gsb";

			Copie(nom_fichier_groupe_src, nom_fichier_groupe_dst);
		}

		// copie le fichier des groupes d'uhrh de correction dans la simulation
		{
			string nom_fichier_groupe_src = Combine(
				Combine( 
				Combine(repertoire_src, "simulation"), 
				sim_hyd.PrendreNomSimulation()),
				sim_hyd.PrendreNomSimulation())
				+ ".sbc";

			string nom_fichier_groupe_dst = Combine(
				sim_hyd.PrendreRepertoireSimulation(), sim_hyd.PrendreNomSimulation())
				+ ".sbc";

			Copie(nom_fichier_groupe_src, nom_fichier_groupe_dst);
		}

		////obtient la projection du projet hydrotel 2.6
		//ifstream fich(nom_fichier_projection);
		//string sString;

		//getline_mod(fich, sString);
		//fich.close();

		//if(sString == "")
		//	throw ERREUR("Erreur; MiseAJour;  erreur lors de la lecture du fichier CoordSys.txt; le fichier est vide.");
		//
		//OGRCoordinateTransformation *poCT;
		//OGRSpatialReference oSRSHydrotel;
		//OGRSpatialReference oSRS;

		////on assume que tous les projet 2.6 sont en UTM; theoriquement hydrotel 2.6 ne peut gérer d'autre projection

		//if(oSRSHydrotel.importFromMICoordSys(sString.c_str()) != OGRERR_NONE)
		//	throw ERREUR("Erreur lors de la lecture du systeme de coordonnee (fichier CoordSys.txt).");

		//if(oSRSHydrotel.SetUTM(zone) != OGRERR_NONE)	//set la zone UTM du projet; important si le bassin chevauche sur 2 zones utm...
		//	throw ERREUR("Erreur lors de la définition de la zone UTM lors de la lecture du systeme de coordonnee (fichier CoordSys.txt).");

		////oSRS.importFromEPSG(4326);	//lat/long (gcs) wgs84
		//oSRS.importFromEPSG(4267);	//lat/long (gcs) nad83

		////char* chProjString = NULL;
		////oSRS.exportToWkt(&chProjString);

		//poCT = OGRCreateCoordinateTransformation(&oSRS, &oSRSHydrotel);
		//if(poCT == NULL)
		//	throw ERREUR("Erreur lecture stations; OGRCreateCoordinateTransformation.");

		////-------------------------------------------------------
		//// mise a jour coordonnee station meteo lat/lon
		//{
		//	ifstream in(sim_hyd.PrendreStationsMeteo().PrendreNomFichier());

		//	int type, nb_station;
		//	in >> type >> nb_station;
		//	in.ignore();

		//	if (type == 1)
		//	{
		//		string ligne;
		//		getline_mod(in, ligne);

		//		vector<string> stations(nb_station);

		//		for (int n = 0; n < nb_station; ++n)
		//		{
		//			getline_mod(in, stations[n]);
		//		}

		//		in.close();

		//		ofstream out(sim_hyd.PrendreStationsMeteo().PrendreNomFichier());

		//		out << 2 << endl << nb_station << endl;
		//		out << ligne << endl;

		//		for (int n = 0; n < nb_station; ++n)
		//		{
		//			istringstream iss(stations[n]);

		//			string ident, c, cc;
		//			float lon, lat, est, nord, alt;
		//			int format;

		//			iss >> ident >> lon >> lat >> alt >> format >> c >> cc;

		//			double ent;
		//			lon = (float)(((int)(lon/100.0f)) + ((long)lon%100)/60.0f + modf((double)lon, &ent)/60.0f);
		//			lat = (float)(((int)(lat/100.0f)) + ((long)lat%100)/60.0f + modf((double)lat, &ent)/60.0f);

		//			ConversionLatLon_vers_Utm(lon, lat, est, nord, zone);
		//
		//			out << fixed << ident << ' ' << est << ' ' << nord << ' ' << alt << ' ' << format << ' ' << c << ' ' << cc << endl;
		//		}
		//	}
		//}

		//// mise a jour coordonnee station hydro lat/lon
		//{
		//	ifstream in(sim_hyd.PrendreStationsHydro().PrendreNomFichier());

		//	int type, nb_station;
		//	in >> type >> nb_station;
		//	in.ignore();

		//	if (type == 1)
		//	{
		//		string ligne;
		//		getline_mod(in, ligne);

		//		vector<string> stations(nb_station);

		//		for (int n = 0; n < nb_station; ++n)
		//		{
		//			getline_mod(in, stations[n]);
		//		}

		//		in.close();

		//		ofstream out(sim_hyd.PrendreStationsHydro().PrendreNomFichier());

		//		out << 2 << endl << nb_station << endl;
		//		out << ligne << endl;

		//		for (int n = 0; n < nb_station; ++n)
		//		{
		//			istringstream iss(stations[n]);

		//			string ident, c, cc;
		//			float lon, lat, est, nord, alt;
		//			int format;

		//			iss >> ident >> lon >> lat >> alt >> format >> c >> cc;

		//			double ent;
		//			lon = (float)(((int)(lon/100.0f)) + ((long)lon%100)/60.0f + modf((double)lon, &ent)/60.0f);	//sexagecimal degree -> decimal degree
		//			lat = (float)(((int)(lat/100.0f)) + ((long)lat%100)/60.0f + modf((double)lat, &ent)/60.0f);	//sexagecimal degree -> decimal degree

		//			ConversionLatLon_vers_Utm(lon, lat, est, nord, zone);

		//			//double x1, y1;
		//			//x1 = -lon;
		//			//y1 = lat;

		//			//if(!poCT->Transform(1, &x1, &y1))
		//			//	throw ERREUR("Erreur lecture stations; poCT->Transform.");
		//
		//			out << fixed << ident << ' ' << est << ' ' << nord << ' ' << alt << ' ' << format << ' ' << c << ' ' << cc << endl;
		//		}
		//	}
		//}
		////-------------------------------------------------------

		// creation d'un fichier shapefile pour les zones
		{
			string src = sim_hyd.PrendreZones().PrendreNomFichierZone();
			string dst = RemplaceExtension(src, "shp");

			Polygonize(src, dst, nom_fichier_masque);
		}

		// supprime le masque
		SupprimerFichier(nom_fichier_masque);

		// supprime le fichier reseau
		//SupprimerFichier(Combine(repertoire_physitel, "reseau.tif"));

		// supprime les fichiers de ponderations
		if (FichierExiste(Combine(repertoire_physitel, "uhrh.pth")))
		{
			SupprimerFichier(Combine(repertoire_physitel, "uhrh.pth"));
		}

		if (FichierExiste(Combine(repertoire_physitel, "uhrh.p3s")))
		{
			SupprimerFichier(Combine(repertoire_physitel, "uhrh.p3s"));
		}

		//transfert les fichiers pour les milieux humides du rep physio vers le rep de simulation, excepte troncon_width_depth.csv
		string sSource, sTarget;

		sSource = Combine(repertoire_src, "physio/milieux_humides_isoles.csv");
		if(FichierExiste(sSource))
		{
			sTarget = Combine(sim_hyd.PrendreRepertoireSimulation(), "milieux_humides_isoles.csv");
			Copie(sSource, sTarget);

			SupprimerFichier(repertoire_physio + "/milieux_humides_isoles.csv");
		}

		sSource = Combine(repertoire_src, "physio/milieux_humides_riverains.csv");
		if(FichierExiste(sSource))
		{
			sTarget = Combine(sim_hyd.PrendreRepertoireSimulation(), "milieux_humides_riverains.csv");
			Copie(sSource, sTarget);

			SupprimerFichier(repertoire_physio + "/milieux_humides_riverains.csv");
		}
	}


	void Info(const string& fichier_prj)
	{
		SIM_HYD	sim_hyd;

		sim_hyd.ChangeNomFichier(fichier_prj);
		sim_hyd.Lecture();

		string repertoire = Combine(sim_hyd.PrendreRepertoireProjet(), "project-info");
		CreeRepertoire(repertoire);

		//file rhhu, reach
		{
			string nom_fichier = Combine(repertoire, "rhhu-reach.txt");
			ofstream fichier(nom_fichier);

			try{
			fichier.exceptions(ios::failbit | ios::badbit);

			fichier << "%1. Reach id" << endl;
			fichier << "RHHU %1" << endl;

			ZONES& zones = sim_hyd.PrendreZones();

			for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
			{
				fichier << zones[index].PrendreIdent() << ' ';
				fichier << zones[index].PrendreTronconAval()->PrendreIdent() << endl;
			}

			}
			catch(...)
			{
				std::cout << "error saving file: " << nom_fichier << ": the file may be in use." << endl << endl;
			}
		}
		
		//file rhhu
		{
			string nom_fichier = Combine(repertoire, "rhhu.txt");
			ofstream fichier(nom_fichier);

			try{
			fichier.exceptions(ios::failbit | ios::badbit);

			fichier << "%1. Type (1=river; 2=lake)  %2. Elevation avg [m]  %3. Area [km2]  %4. Slope avg [ratio]  %5. Orientation avg  %6. Centroid lon [dd]  %7. Centroid lat [dd]" << endl;
			fichier << "Orientation (1=East; 2=Northeast; 3=North; 4=Northwest; 5=West; 6=Southwest; 7=South; 8=Southeast)" << endl;
			fichier << "RHHU %1 %2 %3 %4 %5 %6 %7" << endl;

			ZONES& zones = sim_hyd.PrendreZones();

			for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
			{
				ZONE& zone = zones[index];

				fichier << zone.PrendreIdent() << ' ';
				fichier << (zone.PrendreTypeZone()+1) << ' ';
				fichier << setiosflags(ios::fixed) << setprecision(2) << zone.PrendreAltitude() << ' ';
				fichier << setprecision(6) << zone.PrendreNbPixel() * zones.PrendreResolution() * zones.PrendreResolution() / 1000000.0f << ' ';
				fichier << setprecision(3) << zone.PrendrePente() << ' ';
				fichier << zone.PrendreOrientation() << ' ';

				COORDONNEE coord = zone.PrendreCentroide();
				fichier << setprecision(7) << coord.PrendreX() << ' ';
				fichier << setprecision(7) << coord.PrendreY() << endl;
			}

			}
			catch(...)
			{
				std::cout << "error saving file: " << nom_fichier << ": the file may be in use." << endl << endl;
			}
		}

		// fichier uhrh --> troncons
		{
			string nom_fichier = Combine(repertoire, "rhhu-drained.txt");
			ofstream fichier(nom_fichier);

			try{
			fichier.exceptions(ios::failbit | ios::badbit);

			fichier << "%1. 1 to N : RHHUs drained by the reach" << endl;
			fichier << "REACH %RHHU 1 %RHHU 2 %RHHU 3 %RHHU 4... %RHHU N" << endl;

			TRONCONS& troncons = sim_hyd.PrendreTroncons();

			for (size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
			{
				fichier << troncons[index]->PrendreIdent();

				vector<int> zones_amont = PrendreZoneAmont(troncons[index]);
				std::sort(begin(zones_amont), end(zones_amont));

				for (auto iter = begin(zones_amont); iter != end(zones_amont); ++iter)
					fichier << ' ' << *iter;

				fichier << endl;
			}

			}
			catch(...)
			{
				std::cout << "error saving file: " << nom_fichier << ": the file may be in use." << endl << endl;
			}
		}

		std::cout << "Information files have been saved in folder: " << repertoire << endl << endl;
	}


	void CropRaster(const string& sPathRasterSrc, const string& sPathRasterMask)
	{
		size_t xsize, ysize, x, y, x1, y1, x2, y2;
		float xcellsize, ycellsize;
		int xx, yy, ival, nbrow, nbcol;
		
		RASTER<int> mask = ReadGeoTIFF_int(sPathRasterMask);
		int iNoDataMask = mask.PrendreNoData();

		//get minRow, minCol, maxRow, maxCol of mask raster
		x1 = mask.PrendreNbColonne() - 1;
		y1 = mask.PrendreNbLigne() - 1;

		x2 = 0;
		y2 = 0;

		for (y = 0; y < mask.PrendreNbLigne(); ++y)
		{
			for (x = 0; x < mask.PrendreNbColonne(); ++x)
			{
				if (mask(y, x) != iNoDataMask)
				{
					x1 = min(x1, x);
					y1 = min(y1, y);

					x2 = max(x2, x);
					y2 = max(y2, y);
				}
			}
		}

		xsize = x2 - x1 + 1;
		ysize = y2 - y1 + 1;

		xcellsize = mask.PrendreTailleCelluleX();
		ycellsize = mask.PrendreTailleCelluleY();

		PROJECTION proj;
		proj = mask.PrendreProjection();
		//projection = mask.PrendreProjection().ExportWkt().c_str();

		COORDONNEE coord = mask.LigColVersCoordonnee(static_cast<int>(y1), static_cast<int>(x1));

		RASTER<int> src = ReadGeoTIFF_int(sPathRasterSrc);
		int iNoDataSrc = src.PrendreNoData();

		nbrow = static_cast<int>(src.PrendreNbLigne());
		nbcol = static_cast<int>(src.PrendreNbColonne());

		RASTER<int> dst(coord, proj, ysize, xsize, xcellsize, ycellsize, src.PrendreNoData());	//new cropped raster

		for (y = 0; y < ysize; ++y)
		{
			for (x = 0; x < xsize; ++x)
			{
				ival = mask(y+y1, x+x1);
				if (ival != iNoDataMask)
				{
					COORDONNEE c1 = mask.LigColVersCoordonnee(((int)y)+((int)y1), ((int)x)+((int)x1));
					src.CoordonneeVersLigCol(c1, yy, xx);	//il faut recalculer le x,y car la carte source n'a pas necessairement le meme extent que la carte mask

					if(yy < 0 || yy > nbrow-1 || xx < 0 || xx > nbcol-1)
						ival = iNoDataSrc;
					else
						ival = src(yy, xx);

					dst(y, x) = ival;
				}
			}
		}

		SupprimerFichier(sPathRasterSrc);
		WriteGeoTIFF(dst, sPathRasterSrc, src.PrendreNoData());
	}


	void CropRasterInt_FloatMask(const string& sPathRasterIntSrc, const string& sPathRasterFloatMask)
	{
		size_t xsize, ysize, x, y, x1, y1, x2, y2;
		float xcellsize, ycellsize, fNodataMask;
		int xx, yy, iVal;
		
		RASTER<float> mask = ReadGeoTIFF_float(sPathRasterFloatMask);

		fNodataMask = mask.PrendreNoData();

		//get minRow, minCol, maxRow, maxCol of mask raster
		x1 = mask.PrendreNbColonne() - 1;
		y1 = mask.PrendreNbLigne() - 1;

		x2 = 0;
		y2 = 0;

		for (y = 0; y < mask.PrendreNbLigne(); ++y)
		{
			for (x = 0; x < mask.PrendreNbColonne(); ++x)
			{
				if (mask(y, x) != fNodataMask)
				{
					x1 = min(x1, x);
					y1 = min(y1, y);

					x2 = max(x2, x);
					y2 = max(y2, y);
				}
			}
		}

		xsize = x2 - x1 + 1;
		ysize = y2 - y1 + 1;

		xcellsize = mask.PrendreTailleCelluleX();
		ycellsize = mask.PrendreTailleCelluleY();

		PROJECTION proj;
		proj = mask.PrendreProjection();
		//projection = mask.PrendreProjection().ExportWkt().c_str();

		COORDONNEE coord = mask.LigColVersCoordonnee(static_cast<int>(y1), static_cast<int>(x1));

		RASTER<int> src = ReadGeoTIFF_int(sPathRasterIntSrc);

		RASTER<int> dst(coord, proj, ysize, xsize, xcellsize, ycellsize, src.PrendreNoData());	//new cropped raster

		for (y = 0; y < ysize; ++y)
		{
			for (x = 0; x < xsize; ++x)
			{
				COORDONNEE c1 = mask.LigColVersCoordonnee(((int)y)+((int)y1), ((int)x)+((int)x1));
				src.CoordonneeVersLigCol(c1, yy, xx);	//il faut recalculer le x,y car la carte source n'a pas necessairement le meme extent que la carte mask

				iVal = src(yy, xx);
				dst(y, x) = iVal;
			}
		}

		SupprimerFichier(sPathRasterIntSrc);
		WriteGeoTIFF(dst, sPathRasterIntSrc, src.PrendreNoData());
	}


	void CropRasterFloat(const string& sPathRasterFloatSrc, const string& sPathRasterMask)
	{
		size_t xsize, ysize, x, y, x1, y1, x2, y2;
		float xcellsize, ycellsize, fVal;
		int xx, yy, ival;
		
		RASTER<int> mask = ReadGeoTIFF_int(sPathRasterMask);
		int iNoDataMask = mask.PrendreNoData();

		//get minRow, minCol, maxRow, maxCol of mask raster
		x1 = mask.PrendreNbColonne() - 1;
		y1 = mask.PrendreNbLigne() - 1;

		x2 = 0;
		y2 = 0;

		for (y = 0; y < mask.PrendreNbLigne(); ++y)
		{
			for (x = 0; x < mask.PrendreNbColonne(); ++x)
			{
				if (mask(y, x) != iNoDataMask)
				{
					x1 = min(x1, x);
					y1 = min(y1, y);

					x2 = max(x2, x);
					y2 = max(y2, y);
				}
			}
		}

		xsize = x2 - x1 + 1;
		ysize = y2 - y1 + 1;

		xcellsize = mask.PrendreTailleCelluleX();
		ycellsize = mask.PrendreTailleCelluleY();

		PROJECTION proj;
		proj = mask.PrendreProjection();
		//projection = mask.PrendreProjection().ExportWkt().c_str();

		COORDONNEE coord = mask.LigColVersCoordonnee(static_cast<int>(y1), static_cast<int>(x1));
		
		RASTER<float> src = ReadGeoTIFF_float(sPathRasterFloatSrc);

		RASTER<float> dst(coord, proj, ysize, xsize, xcellsize, ycellsize, src.PrendreNoData());

		for (y = 0; y < ysize; ++y)
		{
			for (x = 0; x < xsize; ++x)
			{
				ival = mask(y+y1, x+x1);
				if (ival != iNoDataMask)
				{
					COORDONNEE c1 = mask.LigColVersCoordonnee(((int)y)+((int)y1), ((int)x)+((int)x1));
					src.CoordonneeVersLigCol(c1, yy, xx);	//il faut recalculer le x,y car la carte source n'a pas necessairement le meme extent que la carte mask

					fVal = src(yy, xx);
					dst(y, x) = fVal;
				}
			}
		}

		SupprimerFichier(sPathRasterFloatSrc);
		WriteGeoTIFF(dst, sPathRasterFloatSrc);
	}

}
