# Using meteorological data files in NetCDF format

This document describes the steps required to use a meteorological data file in NetCDF format. For execution-speed reasons, the NetCDF data file must not contain missing data, and the time step of the available data (the interval between each measurement) must be the same as that of the simulation to be performed. The file format must also follow one of the two formats supported by HYDROTEL, namely the « STATION » or « GRID » formats described in section 2.



## 1. Select the NetCDF data file for reading meteorological data

To select the NetCDF file manually, without using the Hydrotel UI, open the simulation configuration file with a text editor. A simulation configuration file is located in the simulation folder and has the same name as the simulation with the « .csv » extension (e.g. .\Hydrotel project folder\simulation\Simulation name\Simulation name.csv). Edit the line with the label « FICHIER STATIONS METEO » and enter the path pointing to the data file:

Ex: `FICHIER STATIONS METEO;meteo/netcdf_example_weather_9.3.1.nc`.



## 2. Configure the nc.config file according to the format of the NetCDF data file.

A NetCDF configuration file (.nc.config) must also be created to describe the format of the NetCDF file that will be used. The configuration file can be created with a text editor. The file must be in the same folder as the « .nc » data file and have the same name, with the additional « nc.config » extension (e.g. netcdf_example_weather_9.3.1.nc.config).

The « nc.config » file must have the following format:

| Line | Label | Example value |
| --- | --- | --- |
| 1 | TYPE (STATION/GRID); | STATION |
| 2 | STATION_DIM_NAME; | stations |
| 3 | LATITUDE_NAME; | lat |
| 4 | LONGITUDE_NAME; | lon |
| 5 | ELEVATION_NAME; | z |
| 6 | TIME_NAME; | time |
| 7 | TMIN_NAME; | tmin |
| 8 | TMAX_NAME; | tmax |
| 9 | PRECIP_NAME; | precip |



- **Line 1 (TYPE (STATION/GRID))**

  Enter the value « STATION » to use the station data format (H2.1) containing time series per station. Enter the value « GRID » to use the grid data format (9.3.1) containing time series for each point of a coordinate grid.

  

- **Line 2 (STATION_DIM_NAME)**

  Name of the NetCDF dimension variable whose value equals the number of stations. This value must be specified only when the type chosen on line 1 is « STATION ».

  When the chosen type is « GRID », the value is left blank. The number of grid points in a « GRID » file is determined from the size of the vector containing the latitude coordinates and the size of the vector containing the longitude coordinates (latitude vector size × longitude vector size = number of grid points).

  

- **Line 3 (LATITUDE_NAME)**

  Name of the NetCDF variable containing the latitude coordinate values (WGS 84). For the « STATION » file type, this vector must have the same size as the number of available stations. For the « GRID » file type, this vector must contain the unique latitude values that make up the grid, that is, the possible values for the Y axis. In addition, the file must contain a dimension variable with the same name as the data variable and a value equal to the number of « latitude » values contained in the vector.

  - Type STATION: `double latitude(NbStation)`

  - Type GRID: `double latitude(NbLatitude)`

    

- **Line 4 (LONGITUDE_NAME)**

  Name of the NetCDF variable containing the longitude coordinate values (WGS 84). For the « STATION » file type, this vector must have the same size as the vector containing the latitude values. For the « GRID » file type, the sizes of the latitude and longitude vectors may differ, notably when using a rectangular data grid. In addition, the file must contain a dimension variable with the same name as the data variable and a value equal to the number of « longitude » values contained in the vector.

  - Type STATION: `double longitude(NbStation)`

  - Type GRID: `double longitude(NbLongitude)`

    

- **Line 5 (ELEVATION_NAME)**

  Name of the NetCDF variable containing the elevation values for each station or grid point (depending on the file type used). Elevation values must be provided in metres for each station. For the « GRID » file type, elevation values must be provided for each longitude value corresponding to the first latitude value, then continue with the next latitude, and so on.

  - Type STATION: `double elevation(NbStation)`

  - Type GRID: `double elevation(NbLatitude, NbLongitude)`

    

- **Line 6 (TIME_NAME)**

  Name of the NetCDF variable containing the time values for each time series. The variable must have a «units» attribute with a value of the form «days since yyyy-mm-dd hh:00:00» or «minutes since yyyy-mm-dd hh:00:00». Ex: «days since 1970-01-01 00:00:00». In this example, time values must be specified as the number of days elapsed since 1 January 1970 at midnight (0h). The value may be negative. In the previous example, this would apply to dates before 1 January 1970. HYDROTEL treats the values as being in the local time zone and no adjustment is performed. The file must also contain a «dimensions» variable with the same name as the data variable and a value equal to the number of « time » values contained in the vector.

  - Type STATION and GRID: `double time(NbPasTemps)`

    

- **Line 7 (TMIN_NAME)**

  Name of the NetCDF variable containing the minimum temperature values in degrees Celsius.

  - Type STATION: `double tmin(NbPasTemps, NbStation)`

  - Type GRID: `double tmin(NbPasTemps, NbLatitude, NbLongitude)`

    

- **Line 8 (TMAX_NAME)**

  Name of the NetCDF variable containing the maximum temperature values in degrees Celsius.

  - Type STATION: `double tmax(NbPasTemps, NbStation)`

  - Type GRID: `double tmax(NbPasTemps, NbLatitude, NbLongitude)`

    

- **Line 9 (PRECIP_NAME)**

  Name of the NetCDF variable containing the precipitation values (rain + snow water equivalent) in millimeters.

  - Type STATION: `double precip(NbPasTemps, NbStation)`

  - Type GRID: `double precip(NbPasTemps, NbLatitude, NbLongitude)`

    

**Example « nc.config » file of type STATION:**

```
TYPE (STATION/GRID); STATION
STATION_DIM_NAME; nbstations
LATITUDE_NAME; lat
LONGITUDE_NAME; lon
ELEVATION_NAME; z
TIME_NAME; time
TMIN_NAME; tmin
TMAX_NAME; tmax
PRECIP_NAME; precip
```



**Example « nc.config » file of type GRID:**

```
TYPE (STATION/GRID); GRID
STATION_DIM_NAME;
LATITUDE_NAME; y
LONGITUDE_NAME; x
ELEVATION_NAME; z
TIME_NAME; time
TMIN_NAME; tmin
TMAX_NAME; tmax
PRECIP_NAME; precip
```



## 3. Delimit the extent of the coordinates considered in the simulation (optional).

It is possible to specify the north, south, east and west limits of the coordinates taken into account in the simulation. Stations or grid points must lie inside the defined extent in order to be considered in the simulation. Any stations or grid points lying outside will be excluded from the interpolation process.

To enable this feature, a text file named « extent-limit.config » must be created in the same folder as the NetCDF data file and edited according to the following format:

| Line | Label | Example value (WGS 84) |
| --- | --- | --- |
| 1 | North; | 45.6 |
| 2 | South; | 45.3 |
| 3 | East; | -71.0 |
| 4 | West; | -71.4 |



**Example « extent-limit.config » file:**

```
North; 45.6
South; 45.3
East; -71.0
West; -71.4
```
