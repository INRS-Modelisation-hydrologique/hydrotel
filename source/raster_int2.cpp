//
// HYDROTEL a spatially distributed hydrological model
// Copyright (C) 2013 INRS Centre Eau Terre Environnement
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

#include "raster_int2.hpp"

#include "gdal_util.hpp"


using namespace std;


namespace HYDROTEL
{

	RasterInt2::RasterInt2()
    {
		_values = nullptr;
    }

    
    bool RasterInt2::Open(string path)
    {
		int iSizeX, iSizeY;

		_sError = "";
		
		GDALDataset* dataset = nullptr;
		dataset = (GDALDataset*)GDALOpen(path.c_str(), GA_ReadOnly);

		if(!dataset)
		{
			_sError = "error opening file: GDALOpen: " + path;
			return false;
		}

		//double geotransform[6] = { 0 };
		if(dataset->GetGeoTransform(_geotransform) != CE_None)
		{
			GDALClose((GDALDatasetH)dataset);
			_sError = "error reading file: GetGeoTransform: " + path;
			return false;
		}

		iSizeX = dataset->GetRasterXSize();
		iSizeY = dataset->GetRasterYSize();

		_xSize = static_cast<size_t>(iSizeX);
		_ySize = static_cast<size_t>(iSizeY);


		_projectionStr = dataset->GetProjectionRef();
		if(_projectionStr == "")
		{
			GDALClose((GDALDatasetH)dataset);
			_sError = "error reading file: coordinate system undefined: " + path;
			return false;
		}

		GDALRasterBand* band = dataset->GetRasterBand(1);
		if(band == NULL)
		{
			GDALClose((GDALDatasetH)dataset);
			_sError = "error reading file: GetRasterBand: " + path;
			return false;
		}

		_noData = static_cast<int>(band->GetNoDataValue());
	
		_values = (int*)VSIMalloc(sizeof(int) * _xSize * _ySize);
		if(_values == nullptr)
		{
			GDALClose((GDALDatasetH)dataset);
			_sError = "error reading file data: VSIMalloc: " + path;
			return false;
		}

		if(band->RasterIO(GF_Read, 0, 0, iSizeX, iSizeY, _values, iSizeX, iSizeY, GDT_Int32, 0, 0) != CE_None)
		{
			GDALClose((GDALDatasetH)dataset);
			VSIFree(_values);
			_values = nullptr;
			_sError = "error reading file data: RasterIO: " + path;
			return false;
		}

		GDALClose((GDALDatasetH)dataset);
		return true;
    }


	void RasterInt2::Close()
	{
		if(_values != nullptr)
		{
			VSIFree(_values);
			_values = nullptr;
		}
	}

}

