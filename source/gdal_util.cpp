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

#include "gdal_util.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "point.hpp"
#include "util.hpp"

#include <cmath>

#include <gdal_alg.h>


using namespace std;


namespace HYDROTEL
{

	void WriteGeoTIFF(RASTER<int>& raster, const string& nom_fichier, int nodata)
	{
		int xsize = static_cast<int>(raster.PrendreNbColonne());
		int ysize = static_cast<int>(raster.PrendreNbLigne());

		GDALAllRegister();

		GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
		if (driver == nullptr)
			throw ERREUR("GTiff driver not found");

		GDALDataset* dataset = driver->Create(nom_fichier.c_str(), xsize, ysize, 1, GDT_Int32, nullptr);
		if (dataset == nullptr)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		COORDONNEE coord = raster.PrendreCoordonnee();
		double geotransform[6] = { coord.PrendreX(), raster.PrendreTailleCelluleX(), 0, coord.PrendreY(), 0, -raster.PrendreTailleCelluleY() };
		
		dataset->SetGeoTransform(geotransform);
		dataset->SetProjection(raster.PrendreProjection().ExportWkt().c_str());

		GDALRasterBand* band = dataset->GetRasterBand(1);

		band->SetNoDataValue(nodata);
		if(band->RasterIO(GF_Write, 0, 0, xsize, ysize, raster.PrendrePtr(), xsize, ysize, GDT_Int32, 0, 0) != CE_None)
		{
			GDALClose((GDALDatasetH)dataset);
			throw ERREUR_ECRITURE_FICHIER(": RasterIO: " + nom_fichier);
		}

		GDALClose((GDALDatasetH)dataset);
	}


	void WriteGeoTIFF(RASTER<float>& raster, const string& nom_fichier)
	{
		int xsize = static_cast<int>(raster.PrendreNbColonne());
		int ysize = static_cast<int>(raster.PrendreNbLigne());

		GDALAllRegister();

		GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
		if (driver == nullptr)
			throw ERREUR("GTiff driver not found");

		GDALDataset* dataset = driver->Create(nom_fichier.c_str(), xsize, ysize, 1, GDT_Float32, nullptr);
		if (dataset == nullptr)
			throw ERREUR_ECRITURE_FICHIER(nom_fichier);

		COORDONNEE coord = raster.PrendreCoordonnee();
		
		double geotransform[6] = { coord.PrendreX(), raster.PrendreTailleCelluleX(), 0, coord.PrendreY(), 0, -raster.PrendreTailleCelluleY() };
		
		dataset->SetGeoTransform(geotransform);
		dataset->SetProjection(raster.PrendreProjection().ExportWkt().c_str());

		GDALRasterBand* band = dataset->GetRasterBand(1);

		band->SetNoDataValue(VALEUR_MANQUANTE);
		if(band->RasterIO(GF_Write, 0, 0, xsize, ysize, raster.PrendrePtr(), xsize, ysize, GDT_Float32, 0, 0) != CE_None)
		{
			GDALClose((GDALDatasetH)dataset);
			throw ERREUR_ECRITURE_FICHIER(": RasterIO: " + nom_fichier);
		}

		GDALClose((GDALDatasetH)dataset);
	}


	RASTER<float> ReadGeoTIFF_float(const string& nom_fichier)
	{
		GDALAllRegister();

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(nom_fichier.c_str(), GA_ReadOnly));
		if (dataset == nullptr)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		double geotransform[6] = { 0 };
		dataset->GetGeoTransform(geotransform);

		int xsize = dataset->GetRasterXSize();
		int ysize = dataset->GetRasterYSize();

		const char* projection = dataset->GetProjectionRef();

		GDALRasterBand* band = dataset->GetRasterBand(1);
		float nodata = static_cast<float>(band->GetNoDataValue());

		RASTER<float> raster(
			COORDONNEE(geotransform[0], geotransform[3]), 
			PROJECTION(projection), 
			ysize, xsize, 
			static_cast<float>(geotransform[1]), abs(static_cast<float>(geotransform[5])), 
			nodata);

		if(band->RasterIO(GF_Read, 0, 0, xsize, ysize, raster.PrendrePtr(), xsize, ysize, GDT_Float32, 0, 0) != CE_None)
		{
			GDALClose((GDALDatasetH)dataset);
			throw ERREUR_ECRITURE_FICHIER(": RasterIO: " + nom_fichier);
		}

		GDALClose((GDALDatasetH)dataset);
		return raster;
	}


	RASTER<int> ReadGeoTIFF_int(const string& nom_fichier)
	{
		GDALAllRegister();

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(nom_fichier.c_str(), GA_ReadOnly));
		if (dataset == nullptr)
			throw ERREUR_LECTURE_FICHIER(nom_fichier);

		double geotransform[6] = { 0 };
		dataset->GetGeoTransform(geotransform);

		int xsize = dataset->GetRasterXSize();
		int ysize = dataset->GetRasterYSize();

		const char* projection = dataset->GetProjectionRef();

		GDALRasterBand* band = dataset->GetRasterBand(1);
		int nodata = static_cast<int>(band->GetNoDataValue());

		//int minval = static_cast<int>(band->GetMinimum());
		//int maxval = static_cast<int>(band->GetMaximum());

		RASTER<int> raster(
			COORDONNEE(geotransform[0], geotransform[3]), 
			PROJECTION(projection), 
			ysize, xsize, 
			static_cast<float>(geotransform[1]), abs(static_cast<float>(geotransform[5])), 
			nodata);

		if(band->RasterIO(GF_Read, 0, 0, xsize, ysize, raster.PrendrePtr(), xsize, ysize, GDT_Int32, 0, 0) != CE_None)
		{
			GDALClose((GDALDatasetH)dataset);
			throw ERREUR_ECRITURE_FICHIER(": RasterIO: " + nom_fichier);
		}

		GDALClose((GDALDatasetH)dataset);
		return raster;
	}


	void Polygonize(const string& src, const string& dst, const string& mask)
	{
		GDALAllRegister();
		OGRRegisterAll();

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(src.c_str(), GA_ReadOnly));
		if(dataset == nullptr)
			throw ERREUR_LECTURE_FICHIER(src.c_str());

		GDALRasterBand* band = dataset->GetRasterBand(1);

		GDALDataset* dataset_mask = (GDALDataset*)(GDALOpen(mask.c_str(), GA_ReadOnly));
		if(dataset_mask == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			throw ERREUR_LECTURE_FICHIER(mask.c_str());
		}

		GDALRasterBand* band_mask = dataset_mask->GetRasterBand(1);

		GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
		if(driver == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)dataset_mask);
			throw ERREUR("ESRI Shapefile driver not found");
		}

		GDALDataset* poDS = driver->Create(dst.c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if(poDS == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)dataset_mask);
			throw ERREUR_ECRITURE_FICHIER(dst);
		}

		OGRSpatialReference sr(dataset->GetProjectionRef());

		OGRLayer* layer = poDS->CreateLayer(dst.c_str(), &sr, wkbMultiPolygon, NULL); 
		if(layer == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)dataset_mask);
			GDALClose((GDALDatasetH)poDS);
			throw ERREUR_ECRITURE_FICHIER(dst);
		}

		OGRFieldDefn field("ident", OFTInteger);

		if(layer->CreateField(&field) != OGRERR_NONE)
		{
			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)dataset_mask);
			GDALClose((GDALDatasetH)poDS);
			throw ERREUR_ECRITURE_FICHIER(dst);
		}

		GDALPolygonize(band, band_mask, layer, 0, nullptr, nullptr, nullptr);

		GDALClose((GDALDatasetH)dataset);
		GDALClose((GDALDatasetH)dataset_mask);
		GDALClose((GDALDatasetH)poDS);
	}


	void Polygonize(const string& src, const string& dst)
	{
		GDALAllRegister();
		OGRRegisterAll();

		GDALDataset* dataset = (GDALDataset*)(GDALOpen(src.c_str(), GA_ReadOnly));
		if(dataset == nullptr)
			throw ERREUR_LECTURE_FICHIER(src.c_str());

		GDALRasterBand* band = dataset->GetRasterBand(1);

		GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
		if(driver == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			throw ERREUR("ESRI Shapefile driver not found");
		}

		GDALDataset* datasource = driver->Create(dst.c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if(datasource == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			throw ERREUR_ECRITURE_FICHIER(dst);
		}

		OGRSpatialReference sr(dataset->GetProjectionRef());

		OGRLayer* layer = datasource->CreateLayer(dst.c_str(), &sr, wkbMultiPolygon, nullptr); 
		if(layer == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)datasource);
			throw ERREUR_ECRITURE_FICHIER(dst);
		}

		OGRFieldDefn field("ident", OFTInteger);

		if(layer->CreateField(&field) != OGRERR_NONE)
		{
			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)datasource);
			throw ERREUR_ECRITURE_FICHIER(dst);
		}

		GDALPolygonize(band, nullptr, layer, 0, nullptr, nullptr, nullptr);

		// supprime le(s) polygone(s) 0 

		vector<long> fids;

		layer->ResetReading();
		OGRFeature* feature;
		while((feature = layer->GetNextFeature()) != nullptr)
		{
			int ident = feature->GetFieldAsInteger("ident");
			if (ident == 0)
			{
				long fid = static_cast<long>(feature->GetFID());
				fids.push_back(fid);
			}
		}

		for(auto iter = begin(fids); iter != end(fids); ++iter)
		{
			OGRErr err = layer->DeleteFeature(*iter);
			if(err != OGRERR_NONE)
			{
				GDALClose((GDALDatasetH)dataset);
				GDALClose((GDALDatasetH)datasource);
				throw ERREUR("error deleting feature fid 0: " + dst);
			}
		}
				
		string sql = "REPACK " + string(layer->GetName());
		datasource->ExecuteSQL(sql.c_str(), nullptr, nullptr);
		layer->SyncToDisk();

		GDALClose((GDALDatasetH)dataset);
		GDALClose((GDALDatasetH)datasource);
	}


	POINT PointAval(POINT p, int ori)
	{
		switch(ori)
		{
		case 1:
			p.x++;
			break;
		case 2:
			p.x++;
			p.y--;
			break;
		case 3:
			p.y--;
			break;
		case 4:
			p.x--;
			p.y--;
			break;
		case 5:
			p.x--;
			break;
		case 6:
			p.x--;
			p.y++;
			break;
		case 7:
			p.y++;
			break;
		case 8:
			p.x++;
			p.y++;
			break;
		}
		return p;
	}


	void ReseauGeoTIFF2Shapefile(const string& nom_fichier_orientation, const string& nom_fichier_reseau, const string& nom_fichier_riviere, const string& nom_fichier_lac)
	{
		RASTER<int> orientation = ReadGeoTIFF_int(nom_fichier_orientation);
		RASTER<int> reseau = ReadGeoTIFF_int(nom_fichier_reseau);

		const int nbLigne = static_cast<int>(reseau.PrendreNbLigne());
		const int nbColonne = static_cast<int>(reseau.PrendreNbColonne());

		//creation du fichier de lacs
		{
			RASTER<int> lac = RASTER<int>(reseau.PrendreCoordonnee(), reseau.PrendreProjection(), nbLigne, nbColonne, reseau.PrendreTailleCelluleX(), reseau.PrendreTailleCelluleY());

			for(int ligne = 0; ligne < nbLigne; ++ligne)
			{
				for(int colonne = 0; colonne < nbColonne; ++colonne)
				{
					int ident = reseau(ligne, colonne);
					if(ident < 0)
						lac(ligne, colonne) = ident;
				}
			}

			string tmp = Combine(GetTempDirectory(), GetTempFilename()) + ".tif";
			WriteGeoTIFF(lac, tmp, 0);
			Polygonize(tmp, nom_fichier_lac);
			SupprimerFichier(tmp);
		}

		//creation du fichier des rivieres
		{
			map<int, POINT> points_amont;

			for (int ligne = 0; ligne < nbLigne; ++ligne)
			{
				for (int colonne = 0; colonne < nbColonne; ++colonne)
				{
					int ident = reseau(ligne, colonne);
					if (ident > 0)
					{
						POINT point(ligne, colonne);

						bool amont = true;
						for (int y = point.y - 1; y <= point.y + 1 && amont; ++y)
						{
							for (int x = point.x - 1; x <= point.x + 1 && amont; ++x)
							{
								if (y == point.y && x == point.x)
									continue;

								if (y < 0 || y >= nbLigne || x < 0 || x >= nbColonne)
									continue;

								POINT pointAval = PointAval(POINT(y, x), orientation(y, x));
								if (pointAval == point && ident == reseau(y, x))
									amont = false;
							}
						}

						if (amont)
						{
							points_amont[ident] = point;
						}
					}
				}
			}

			OGRRegisterAll();

			GDALDataset* dataset = (GDALDataset*)GDALOpen(nom_fichier_reseau.c_str(), GA_ReadOnly);
			if(dataset == nullptr)
				throw ERREUR_LECTURE_FICHIER(nom_fichier_reseau);

			GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
			if(driver == nullptr)
			{
				GDALClose((GDALDatasetH)dataset);
				throw ERREUR("ESRI Shapefile driver not found");
			}

			GDALDataset* datasource = driver->Create(nom_fichier_riviere.c_str(), 0, 0, 0, GDT_Unknown, NULL);
			if(datasource == nullptr)
			{
				GDALClose((GDALDatasetH)dataset);
				throw ERREUR_ECRITURE_FICHIER(nom_fichier_riviere);
			}

			OGRSpatialReference sr(dataset->GetProjectionRef());

			OGRLayer* layer = datasource->CreateLayer(nom_fichier_riviere.c_str(), &sr, wkbMultiLineString, nullptr); 
			if(!layer)
			{
				GDALClose((GDALDatasetH)dataset);
				GDALClose((GDALDatasetH)datasource);
				throw ERREUR_ECRITURE_FICHIER(nom_fichier_riviere);
			}

			OGRFieldDefn field("ident", OFTInteger);

			if(layer->CreateField(&field) != OGRERR_NONE)
			{
				GDALClose((GDALDatasetH)dataset);
				GDALClose((GDALDatasetH)datasource);
				throw ERREUR_ECRITURE_FICHIER(nom_fichier_riviere);
			}

			double dx = reseau.PrendreTailleCelluleX() / 2;
			double dy = reseau.PrendreTailleCelluleY() / 2;

			OGRFeature* feature;
			COORDONNEE coord;
			POINT point;
			int ident, nbcol, nbrow;

			nbcol = static_cast<int>(orientation.PrendreNbColonne());
			nbrow = static_cast<int>(orientation.PrendreNbLigne());

			for(auto iter = begin(points_amont); iter != end(points_amont); ++iter)
			{
				feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
				feature->SetField("ident", iter->first);

				OGRLineString line;

				point = iter->second;
				ident = iter->first;

				do 
				{
					coord = reseau.LigColVersCoordonnee(point.y, point.x);
					coord = COORDONNEE(coord.PrendreX() + dx, coord.PrendreY() - dy);

					OGRPoint p;
					p.setX(coord.PrendreX());
					p.setY(coord.PrendreY());

					line.addPoint(&p);

					point = PointAval(point, orientation(point.y, point.x));
					if(point.x < 0 || point.x >= nbcol || point.y < 0 || point.y >= nbrow)
					{
						if(iter->first == 1)	//si troncon exutoire, on vient de sortir de la matrice: fin du troncon
							ident = 0;
						else
						{
							GDALClose((GDALDatasetH)dataset);
							GDALClose((GDALDatasetH)datasource);

							ostringstream oss;
							oss << "TEST ERREUR CODE 1; x=" << setiosflags(ios::fixed) << coord.PrendreX() << ", y=" << coord.PrendreY();
							throw ERREUR(oss.str());
						}
					}
					else
						ident = reseau(point.y, point.x);
				} 
				while(ident == iter->first);

				if(ident != 0 && ident != reseau.PrendreNoData())
				{
					coord = reseau.LigColVersCoordonnee(point.y, point.x);
					coord = COORDONNEE(coord.PrendreX() + dx, coord.PrendreY() - dy);

					OGRPoint p;
					p.setX(coord.PrendreX());
					p.setY(coord.PrendreY());

					line.addPoint(&p);
				}

				if(line.getNumPoints() > 1)
				{
					feature->SetGeometryDirectly(&line);
					if(layer->CreateFeature(feature) != OGRERR_NONE)
					{
						GDALClose((GDALDatasetH)dataset);
						GDALClose((GDALDatasetH)datasource);

						throw ERREUR_ECRITURE_FICHIER(": CreateFeature: " + nom_fichier_riviere);
					}
				}
			}

			layer->SyncToDisk();

			GDALClose((GDALDatasetH)dataset);
			GDALClose((GDALDatasetH)datasource);
		}
	}


}

