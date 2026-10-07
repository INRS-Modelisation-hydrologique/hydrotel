# Hydrotel project files reference

_7 October 2026 - Hydrotel v4.4.0_

This document is a reference dictionary describing the files that make up a HYDROTEL project folder.

It first presents a table that follows the folder structure and lists the files contained in each folder.

The table is followed by an alphabetical list of files and a detailed description of each file.

| Folder | File |
| --- | --- |
| /etats | [acheminement_riviere_2020010100.csv](#etats) |
|  | [acheminement_riviere_MH_2020010100.csv](#etats) |
|  | [bilan_vertical_2020010100.csv](#etats) |
|  | [fonte_neige_2020010100.csv](#etats) |
|  | [ruisselement_surface_2020010100.csv](#etats) |
|  | [ruisselement_surface_base_2020010100.csv](#etats) |
|  | [ruisselement_surface_hypo_2020010100.csv](#etats) |
|  | [ruisselement_surface_surf_2020010100.csv](#etats) |
| |  |
| /hgm | [hydrogramme.hgm](#hydrogramme-hgm) |
|  |  |
| /hydro | [02MC036.hyd](#02mc036-hyd) |
|  | [station.sth](#station-sth) |
| |  |
| /meteo | [0000040.met](#0000040-met) |
|  | [neige.pgn](#neige-pgn) |
|  | [station.p3s](#station-p3s) |
|  | [station.pth](#station-pth) |
|  | [station.stm](#station-stm) |
| | [station-troncon.p3s](#station-troncon-p3s) |
| | [station-troncon.pth](#station-troncon-pth) |
| |  |
| /neige | [NEIGE01.nei](#neige01-nei) |
|  | [station.p3s](#station-p3s) |
|  | [station.pth](#station-pth) |
|  | [station.stn](#station-stn) |
| |  |
| /neige-grille | [grilleneige.grn](#grilleneige-grn) |
|  |  |
| /neige-grille/donnees | [neige_2003_01_22_24h.een](#neige-2003-01-22-24h-een) |
|  | [neige_2003_01_22_24h.hau](#neige-2003-01-22-24h-hau) |
| |  |
| /physio | [ind_fol.def](#ind-fol-def) |
|  | [pro_rac.def](#pro-rac-def) |
|  | [shreve.csv](#shreve-csv) |
|  | [strahler.csv](#strahler-csv) |
|  | [troncon_width_depth.csv](#troncon-width-depth-csv) |
|  | [wet_pixel_info.csv](#wet-pixel-info-csv) |
| |  |
| /physitel | [altitude.tif](#altitude-tif) |
|  | [lacs.shp, lacs.prj, lacs.dbf, lacs.shx](#lacs-shp-lacs-prj-lacs-dbf-lacs-shx) |
|  | [noeuds.nds](#noeuds-nds) |
|  | [occupation_sol.cla](#occupation-sol-cla) |
|  | [occupation_sol.csv](#occupation-sol-csv) |
|  | [occupation_sol.tif](#occupation-sol-tif) |
|  | [orientation.tif](#orientation-tif) |
|  | [pente.tif](#pente-tif) |
|  | [physitelproject.txt](#physitelproject-txt) |
|  | [point.rdx](#point-rdx) |
|  | [proprietehydrolique.sol](#proprietehydrolique-sol) |
|  | [rivieres.shp, rivieres.prj, rivieres.dbf, rivieres.shx](#rivieres-shp-rivieres-prj-rivieres-dbf-rivieres-shx) |
|  | [troncon.trl](#troncon-trl) |
|  | [troncons.tif](#troncons-tif) |
|  | [troncons.txt](#troncons-txt) |
|  | [type_sol.cla](#type-sol-cla) |
|  | [type_sol.tif](#type-sol-tif) |
|  | [uhrh.csv](#uhrh-csv) |
|  | [uhrh.shp, uhrh.prj, uhrh.dbf, uhrh.shx](#uhrh-shp-uhrh-prj-uhrh-dbf-uhrh-shx) |
|  | [uhrh.tif](#uhrh-tif) |
|  | [uhrh.txt](#uhrh-txt) |
| |  |
| /simulation/\{nom_simulation\} | [\{nom_simulation\}.csv](#nom-simulation-csv) |
|  | [\{nom_simulation\.}gsb](#nom-simulation-gsb)                |
|  | [bv3c.csv](#bv3c-csv) |
|  | [cequeau.csv](#cequeau-csv) |
|  | [corrections.csv](#corrections-csv) |
|  | [degre_jour_modifie.csv](#degre-jour-modifie-csv) |
|  | [degre-jour-bande.csv](#degre-jour-bande-csv) |
|  | [degre-jour-glacier.csv](#degre-jour-glacier-csv) |
|  | [etp-mc-guiness.csv](#etp-mc-guiness-csv) |
|  | [grillemeteo.csv](#grillemeteo-csv) |
|  | [grilleprevision.csv](#grilleprevision-csv) |
|  | [hydro_quebec.csv](#hydro-quebec-csv) |
|  | [lecture_acheminement.csv](#lecture-acheminement-csv) |
|  | [lecture_bilan_vertical.csv](#lecture-bilan-vertical-csv) |
|  | [lecture_etp.csv](#lecture-etp-csv) |
|  | [lecture_fonte_glacier.csv](#lecture-fonte-glacier-csv) |
|  | [lecture_fonte_neige.csv](#lecture-fonte-neige-csv) |
|  | [lecture_interpolation.csv](#lecture-interpolation-csv) |
|  | [lecture_ruisselement.csv](#lecture-ruisselement-csv) |
|  | [lecture_tempsol.csv](#lecture-tempsol-csv) |
|  | [linacre.csv](#linacre-csv) |
|  | [milieux_humides_isoles.csv](#milieux-humides-isoles-csv) |
|  | [milieux_humides_riverains.csv](#milieux-humides-riverains-csv) |
|  | [moyenne_3_stations.csv](#moyenne-3-stations-csv) |
|  | [onde_cinematique.csv](#onde-cinematique-csv) |
|  | [onde_cinematique_modifiee.csv](#onde-cinematique-modifie-csv) |
|  | [output.csv](#output-csv) |
|  | [parametres_sous_modeles.csv](#parametres-sous-modeles-csv) |
|  | [penman.csv](#penman-csv) |
|  | [penman_monteith.csv](#penman-monteith-csv) |
|  | [priestlay_taylor.csv](#priestlay-taylor-csv) |
|  | [rankinen.csv](#rankinen-csv) |
|  | [rayonnement_net.csv](#rayonnement-net-csv) |
|  | [stats.txt](#stats-txt) |
|  | [submodels-versions.txt](#submodels-version-txt) |
|  | [temp_eau_cequeau_troncons.csv](#temp-eau-cequeau-troncons-csv) |
|  | [temp_eau_cequeau_zones.csv](#temp-eau-cequeau-zones-csv) |
|  | [thiessen.csv](#thiessen-csv) |
|  | [thornthwaite.csv](#thornthwaite-csv) |
|  | [thorsen.csv](#thorsen-csv) |
| |  |
| /simulation/\{nom_simulation\}/resultat | [moyennes-ponderees-troncon\{IdTroncon\}.csv](#moyennes-ponderees-troncon-idtroncon-csv) |
|  | [obs-sim-flows.csv](#obs-sim-flows-csv) |
|  | [stats.csv](#stats-csv) |
|  | [wetland_isole.csv](#wetland-isole-csv) |
|  | [wetland_riverain.csv](#wetland-riverain-csv) |

<a id="nom-simulation-csv"></a>

## \{nom_simulation\}.csv   (/simulation/\{nom_simulation\})

Simulation parameter file.

| Line | Description |
| --- | --- |
| SIMULATION HYDROTEL;4.3.0.0000 | HYDROTEL version number used when the simulation was created. |
| Blank line |  |
| FICHIER OCCUPATION SOL;[physitel/occupation_sol.cla](#occupation-sol-cla) | Name of the land-cover class file. |
| FICHIER PROPRIETE HYDROLIQUE;[physitel/proprietehydrolique.sol](#proprietehydrolique-sol) | Name of the soil hydraulic properties file. |
| FICHIER TYPE SOL COUCHE1;[physitel/type_sol.cla](#type-sol-cla) | Name of the dominant soil-type file for RHHUs for soil layer 1. |
| FICHIER TYPE SOL COUCHE2;[physitel/type_sol.cla](#type-sol-cla) | Name of the dominant soil-type file for RHHUs for soil layer 2. |
| FICHIER TYPE SOL COUCHE3;[physitel/type_sol.cla](#type-sol-cla) | Name of the dominant soil-type file for RHHUs for soil layer 3. |
| Blank line |  |
| COEFFICIENT ADDITIF PROPRIETE HYDROLIQUE; | List of additive coefficients for hydraulic properties (one value for each RHHU group). A positive coefficient shifts all soil types toward clays; a negative coefficient shifts them toward sands. |
| Blank line |  |
| FICHIER INDICE FOLIERE;[physio/ind_fol.def](#ind-fol-def) | Name of the leaf-area-index file. |
| FICHIER PROFONDEUR RACINAIRE;[physio/pro_rac.def](#pro-rac-def) | Name of the rooting-depth file. |
| Blank line |  |
| FICHIER GRILLE METEO; | Name of the parameter file for the GRILLE sub-model ([grillemeteo.csv](#grillemeteo-csv)). |
| FICHIER STATIONS METEO;[meteo/station.stm](#station-stm) | Meteorological station file. |
| FICHIER STATIONS HYDRO;[hydro/station.sth](#station-sth) | Hydrometric station file. |
| Blank line |  |
| PREVISION METEO;0 | Activation of weather forecasts (0: disabled, 1: enabled). |
| FICHIER GRILLE PREVISION; | Name of the parameter file for meteorological forecasts ([grilleprevision.csv](#grilleprevision-csv)). |
| DATE DEBUT PREVISION;1973-06-01 00:00 | Start date for meteorological forecasts (format: YYYY-MM-DD HH:00). |
| Blank line |  |
| DATE DEBUT;2020-01-01 00:00 | Simulation start date (format: YYYY-MM-DD HH:00). |
| DATE FIN;2020-07-01 00:00 | Simulation end date (format: YYYY-MM-DD HH:00). |
| PAS DE TEMPS;24 | Simulation time step (hours). |
| Blank line |  |
| TRONCON EXUTOIRE;1 | Identifier of the outlet reach for the simulation. Only reaches upstream of the outlet reach are simulated. |
| Blank line |  |
| TRONCONS DECONNECTER;off; | Disconnected reaches. The first value, «on» or «off», indicates whether the option is enabled. Subsequent values are the identifiers of the disconnected reaches (e.g. «TRONCONS DECONNECTER;on;1;72»). |
| Blank line |  |
| EXTERNAL DATA ROUTING; | Name of the configuration file for using and routing external data in the simulation. See the [external-data-routing](external-data-routing-en.md) document for a full description of the configuration file. |
| Blank line | |
| NOM FICHIER CORRECTIONS;1;[corrections.csv](#corrections-csv) | Name of the correction file to use, preceded by 0 or 1 to indicate whether the option is enabled. |
| Blank line |  |
| LECTURE ETAT FONTE NEIGE;[etats/fonte_neige_2020010100.csv](#etats) | Name of the state file for the snowmelt sub-model. |
| LECTURE ETAT TEMPERATURE DU SOL; | Name of the state file for the soil-temperature sub-model. |
| LECTURE ETAT BILAN VERTICAL;[etats/bilan_vertical_2020010100.csv](#etats) | Name of the state file for the vertical water-budget sub-model. |
| LECTURE ETAT RUISSELEMENT SURFACE;[etats/ruisselement_surface_2020010100.csv](#etats) | Name of the state file for the overland-flow sub-model. |
| LECTURE ETAT ACHEMINEMENT RIVIERE;[etats/acheminement_riviere_2020010100.csv](#etats) | Name of the state file for the river-routing sub-model. |
| Blank line |  |
| ECRITURE ETAT FONTE NEIGE;2020-07-01 00:00 | Date and time of the time step for saving state variables. |
| ECRITURE ETAT TEMPERATURE DU SOL; | Date and time of the time step for saving state variables. |
| ECRITURE ETAT BILAN VERTICAL;2020-07-01 00:00 | Date and time of the time step for saving state variables. |
| ECRITURE ETAT RUISSELEMENT SURFACE;2020-07-01 00:00 | Date and time of the time step for saving state variables. |
| ECRITURE ETAT ACHEMINEMENT RIVIERE;2020-07-01 00:00 | Date and time of the time step for saving state variables. |
| Blank line |  |
| REPERTOIRE ECRITURE ETAT FONTE NEIGE;/etats | Folder to use for saving state variables. |
| REPERTOIRE ECRITURE ETAT TEMPERATURE DU SOL; | Folder to use for saving state variables. |
| REPERTOIRE ECRITURE ETAT BILAN VERTICAL;/etats | Folder to use for saving state variables. |
| REPERTOIRE ECRITURE ETAT RUISSELEMENT SURFACE;/etats | Folder to use for saving state variables. |
| REPERTOIRE ECRITURE ETAT ACHEMINEMENT RIVIERE;/etats | Folder to use for saving state variables. |
| Blank line |  |
| INTERPOLATION DONNEES;THIESSEN | Name of the sub-model to use for interpolating meteorological data. Possible values: THIESSEN, MOYENNE 3 STATIONS, GRILLE, LECTURE INTERPOLATION DONNEES. |
| FONTE NEIGE;DEGRE JOUR MODIFIE | Name of the sub-model to use for snowpack evolution and melt. Possible values: DEGRE JOUR MODIFIE, DEGRE JOUR BANDE, LECTURE FONTE NEIGE. |
| FONTE GLACIER; | Name of the sub-model to use for ice (glacier) evolution and melt. Possible values: DEGRE JOUR GLACIER, LECTURE FONTE GLACIER. |
| TEMPERATURE DU SOL; | Name of the sub-model to use for soil-temperature calculation. Possible values: RANKINEN, THORSEN, LECTURE TEMPERATURE DU SOL. |
| EVAPOTRANSPIRATION;HYDRO-QUEBEC | Name of the sub-model to use for potential evapotranspiration. Possible values: HYDRO-QUEBEC, ETP-MC-GUINESS, LINACRE, PENMAN, PENMAN-MONTEITH, PRIESTLAY-TAYLOR, THORNTHWAITE, LECTURE EVAPOTRANSPIRATION. |
| BILAN VERTICAL;BV3C | Name of the sub-model to use for the vertical water budget. Possible values: BV3C, CEQUEAU, LECTURE BILAN VERTICAL. |
| RUISSELEMENT;ONDE CINEMATIQUE | Name of the sub-model to use for overland flow. Possible values: ONDE CINEMATIQUE, LECTURE RUISSELEMENT SURFACE. |
| ACHEMINEMENT RIVIERE;ONDE CINEMATIQUE MODIFIEE | Name of the sub-model to use for river routing. Possible values: ONDE CINEMATIQUE MODIFIEE, LECTURE ACHEMINEMENT RIVIERE. |
| TEMPERATURE EAU; | Name of the sub-model to use for water-temperature calculation. Possible value: TEMP EAU CEQUEAU. |
| Blank line |  |
| MILIEUX HUMIDES ISOLES;1 | Enables (1) or disables (0) simulation of isolated wetlands. |
| MILIEUX HUMIDES RIVERAINS;1 | Enables (1) or disables (0) simulation of riparian wetlands. |
| Blank line |  |
| FICHIER DE PARAMETRE GLOBAL;0 | Enables (1) or disables (0) use of a single file for all sub-model parameters ([parametres_sous_modeles.csv](#parametres-sous-modeles-csv)). |
| Blank line |  |
| LECTURE INTERPOLATION DONNEES;[lecture_interpolation.csv](#lecture-interpolation-csv) | Names of the sub-model parameter files. |
| THIESSEN;[thiessen.csv](#thiessen-csv) |  |
| MOYENNE 3 STATIONS;[moyenne_3_stations.csv](#moyenne-3-stations-csv) |  |
| LECTURE FONTE NEIGE;[lecture_fonte_neige.csv](#lecture-fonte-neige-csv) |  |
| DEGRE JOUR MODIFIE;[degre_jour_modifie.csv](#degre-jour-modifie-csv) |  |
| LECTURE TEMPERATURE DU SOL;[lecture_tempsol.csv](#lecture-tempsol-csv) |  |
| RANKINEN;[rankinen.csv](#rankinen-csv) |  |
| THORSEN;[thorsen.csv](#thorsen-csv) |  |
| LECTURE EVAPOTRANSPIRATION;[lecture_etp.csv](#lecture-etp-csv) |  |
| HYDRO-QUEBEC;[hydro_quebec.csv](#hydro-quebec-csv) |  |
| THORNTHWAITE;[thornthwaite.csv](#thornthwaite-csv) |  |
| LINACRE;[linacre.csv](#linacre-csv) |  |
| PENMAN;[penman.csv](#penman-csv) |  |
| PRIESTLAY-TAYLOR;[priestlay_taylor.csv](#priestlay-taylor-csv) |  |
| PENMAN-MONTEITH;[penman_monteith.csv](#penman-monteith-csv) |  |
| LECTURE BILAN VERTICAL;[lecture_bilan_vertical.csv](#lecture-bilan-vertical-csv) |  |
| BV3C;[bv3c.csv](#bv3c-csv) |  |
| CEQUEAU;[cequeau.csv](#cequeau-csv) |  |
| LECTURE RUISSELEMENT SURFACE;[lecture_ruisselement.csv](#lecture-ruisselement-csv) |  |
| ONDE CINEMATIQUE;[onde_cinematique.csv](#onde-cinematique-csv) |  |
| LECTURE ACHEMINEMENT RIVIERE;[lecture_acheminement.csv](#lecture-acheminement-csv) |  |
| ONDE CINEMATIQUE MODIFIEE;[onde_cinematique_modifiee.csv](#onde-cinematique-modifie-csv) |  |
| TEMP EAU CEQUEAU;[temp_eau_cequeau_troncons.csv](#temp-eau-cequeau-troncons-csv) ; [temp_eau_cequeau_zones.csv](#temp-eau-cequeau-zones-csv) | Names of the parameter files for the TEMP EAU CEQUEAU sub-model. |
| FICHIER MILIEUX HUMIDES ISOLES;[milieux_humides_isoles.csv](#milieux-humides-isoles-csv) | Name of the parameter file for isolated wetlands. |
| FICHIER MILIEUX HUMIDES RIVERAINS;[milieux_humides_riverains.csv](#milieux-humides-riverains-csv) | Name of the parameter file for riparian wetlands. |

<a id="nom-simulation-gsb"></a>

## \{nom_simulation\}.gsb   (/simulation/\{nom_simulation\})

Description file for RHHU groups.

| Line | Description |
| --- | --- |
| Line 1 | \{NbGroupe\} |
| Line 2 to x | \{NomGroupe\} |
| Line 2 + NbGroupe and following | \{IdUhrh\} \{IndexGroupe\} |

NbGroupe: Number of RHHU groups.

NomGroupe: Group name. There must be one line with the group name for each group (e.g. if there are 3 groups there must be 3 lines with the group names).

IdUhrh: RHHU identifier.

IndexGroupe: Index of the group to which the RHHU belongs. The index of the first group is 0, the index of the second group is 1, and so on.



Ex:

2

Amont

Aval

1 1

2 1

3 1

4 0

5 0

6 1

…

<a id="0000040-met"></a>
## 0000040.met   (/meteo)

Meteorological observation data file for station «0000040».

| Line | Description |
| --- | --- |
| Line 1 | \{Data type\} \{Time step\} |
| Line 2 and following | \{Date/Time\} \{TMax\} \{TMin\} \{Precip\} |

Data type: This value must be set to «1».

Time step: Data time step (hours).

Date/Time: Date and time of the observation (DD/MM/YYYY H). The hour must not be specified when the time step is 24 hours.

TMax: Maximum temperature (°C).

TMin: Minimum temperature (°C).

Precip: Total precipitation (rain + snow water equivalent) (mm).



Ex:

1 3

01/01/2003 3 -12.5 -14.2 1.5

01/01/2003 6 -999 –999 0.5

01/01/2003 9 -11.5 -13.0 0



Ex:

1 24

01/01/2003 -12.5 -14.2 1.5

02/01/2003 -10.0 –13.2 -999

03/01/2003 -11.5 -13.0 5.25



The time read in the data files corresponds to the time at the end of the time step. Hour 3 represents the data for 0h to 3h. Ex: for a 3-hour time step, the hours in the data file for one day must be: 3, 6, 9, 12, 15, 18, 21, 24. The value -999 must be used for missing values (No data). For hourly data, the maximum and minimum temperature fields are replaced by a single temperature field.

<a id="02mc036-hyd"></a>
## 02MC036.hyd   (/hydro)

Hydrometric observation data file for station «02MC036».

| Line | Description |
| --- | --- |
| Line 1 | \{Data type\} \{Time step\} |
| Line 2 and following | \{Date/Time\} \{Discharge\} (\{Level\}) |

Data type: 1 = Discharge only, 2 = Discharge and water levels.

Time step: Data time step (hours).

Date/Time: Date and time of the observation (DD/MM/YYYY H). The hour must not be specified when the time step is 24 hours.

Discharge: Observed discharge (m3/s).

Level: Observed water level (m).



Ex:

1 3

01/01/2003 3 0.308

01/01/2003 6 0.291

01/01/2003 9 0.286



Ex:

2 24

01/01/2003 0.308 0.1

02/01/2003 0.291 0.08

03/01/2003 0.286 0.06



The time read in the data files corresponds to the time at the end of the time step. Hour 3 represents the data for 0h to 3h. Ex: for a 3-hour time step, the hours in the data file for one day must be: 3, 6, 9, 12, 15, 18, 21, 24. Water-level data are not currently used by HYDROTEL. The value -999 must be used for missing values (No data).

<a id="altitude-tif"></a>
## altitude.tif   (/physitel)

Raster map (GeoTIFF) of elevations (DEM) (m).

<a id="bv3c-csv"></a>
## bv3c.csv   (/simulation/\{nom_simulation\})

Parameter file for the BV3C calculation sub-model (vertical water budget).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE;BV3C» |
| Line 4 | Blank line |
| Line 5 | «CLASSE INTEGRE IMPERMEABLE;» \{ListOccSolImpermeable\} |
| Line 6 | «CLASSE INTEGRE EAU;» \{ListOccSolEau\} |
| Line 7 | Blank line |
| Line 8 | Comment |
| Line 9 and following | \{IdUhrh\} \{EpaisseurCouche\} \{EpaisseurCouche2\} \{EpaisseurCouche3\} \{HumIniCouche1\} \{HumIniCouche2\} \{HumIniCouche3\} \{CoefExtinction\} \{CoefRecession\} \{CoefAssechement\} \{VarMaxHumidite\} \{CoefRecharge\} |

ListOccSolImpermeable: List of identifiers (1 to x) of the land-cover classes associated with the aggregated class «Imperméable». Identifier 1 corresponds to the first land-cover class in [occupation_sol.cla](#occupation-sol-cla), identifier 2 to the second class, and so on. Identifiers must be separated by a semicolon «;».

ListOccSolEau: List of identifiers (1 to x) of the land-cover classes associated with the aggregated class «Eau».

IdUhrh: RHHU identifier.

EpaisseurCouche1: Thickness of the 1st soil layer (surface) (m).

EpaisseurCouche2: Thickness of the 2nd soil layer (intermediate) (m).

EpaisseurCouche3: Thickness of the 3rd soil layer (base) (m).

HumIniCouche1: Initial relative moisture of the 1st soil layer (fraction of saturation) (0-1).

HumIniCouche2: Initial relative moisture of the 2nd soil layer (fraction of saturation) (0-1).

HumIniCouche3: Initial relative moisture of the 3rd soil layer (fraction of saturation) (0-1).

CoefExtinction: Extinction coefficient D of solar radiation in the vegetation.

CoefRecession: Recession coefficient kr for baseflow from the 3rd soil layer (m/h).

CoefAssechement: Multiplicative optimization coefficient for drying (applied to the drying coefficient Cs).

VarMaxHumidite: Maximum variation of relative moisture in the soil layers for each time step. Used to limit numerical instabilities that cause moisture oscillations, particularly between the first and second layers. The lower this value, the more instabilities are filtered, but calculations may take slightly longer.

CoefRecharge: Groundwater recharge coefficient (0-1).

<a id="cequeau-csv"></a>
## cequeau.csv   (/simulation/\{nom_simulation\})

Parameter file for the CEQUEAU calculation sub-model (vertical water budget).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE;CEQUEAU» |
| Line 4 | Blank line |
| Line 5 | «CLASSE INTEGRE EAU (LACS ET MARECAGES);» \{ListOccSolLacMarecage\} |
| Line 6 | «CLASSE INTEGRE IMPERMEABLE;» \{ListOccSolImpermeable\} |
| Line 7 | «CLASSE INTEGRE FORETS;» \{ListOccSolForet\} |
| Line 8 | Blank line |
| Line 9 | Comment |
| Line 10 and following | \{IdUhrh\} \{MinRuisImpermeable\} \{NiveauEauMax\} \{SeuilVidangeSol\} \{CoefVidangeRetarde1\} \{CoefVidangeRetarde2\} \{SeuilPercolation\} \{CoefPercolation\} \{MaxPercolation\} \{NiveauEau\} \{CoefVidangeHaute\} \{CoefVidangeBasse\} \{FractionEtp\} \{HauteurVidangeHaute\} \{SeuilVidangeEau\} \{CoefVidangeEau\} \{InitSol\} \{InitNappe\} \{InitEau\} |

ListOccSolLacMarecage : List of identifiers (1 to x) of the land-cover classes associated with the aggregated class «Lac et marécage». Identifier 1 corresponds to the first land-cover class in [occupation_sol.cla](#occupation-sol-cla), identifier 2 to the second class, and so on. Identifiers must be separated by a semicolon «;».

ListOccSolImpermeable : List of identifiers (1 to x) of the land-cover classes associated with the aggregated class «Imperméable».

ListOccSolForet : List of identifiers (1 to x) of the land-cover classes associated with the aggregated class «Forêt».

IdUhrh: RHHU identifier.

MinRuisImpermeable: Minimum threshold for runoff from the impermeable surface (mm).

NiveauEauMax: Maximum water level (soil) (mm).

SeuilVidangeSol: Drainage threshold (soil) (mm).

CoefVidangeRetarde1: Delayed drainage coefficient 1 (soil).

CoefVidangeRetarde2: Delayed drainage coefficient 2 (soil).

SeuilPercolation: Percolation threshold (soil).

CoefPercolation: Percolation coefficient (soil).

MaxPercolation: Maximum percolation rate (soil) (mm/d).

NiveauEau: Water level (soil) (mm).

CoefVidangeHaute: High drainage coefficient (groundwater).

CoefVidangeBasse: Low drainage coefficient (groundwater).

FractionETP: Fraction of PET withdrawn (groundwater) (0-1).

HauteurVidangeHaute: High drainage height (groundwater) (mm).

SeuilVidangeEau: Drainage threshold (water) (mm).

CoefVidangeEau: Drainage coefficient (water).

InitSol: Initial water level (soil) (mm).

InitNappe: Initial water level (groundwater) (mm).

InitEau: Initial water level (water) (mm).

<a id="corrections-csv"></a>
## corrections.csv   (/simulation/\{nom_simulation\})

Correction parameters for internal simulation variables.

The correction file may contain one or more correction lines.

A line is composed of the following columns for variables 1 to 5:

| \{Actif\} \{DateHeureDebut\} \{DateHeureFin\} \{Variable\} \{CoefAdditif\} \{CoefMultiplicatif\} \{TypeGroupe\} \{NomGroupe\} |
| --- |

A line is composed of the following columns for variable 6:

| \{Actif\} \{DateHeureDebut\} \{DateHeureFin\} \{Variable\} \{CoeffSaturationCouche1\} \{CoeffSaturationCouche2\} \{CoeffSaturationCouche3\} \{TypeGroupe\} \{NomGroupe\} |
| --- |

Actif: Indicates whether the correction line is enabled or disabled. Value: 0=Disabled, 1=Enabled.

DateHeureDebut: Start date and time of the correction. Format: «YYYY/MM/DD HH». Ex: «2026/01/15 00».

DateHeureFin: End date and time of the correction. Format: «YYYY/MM/DD HH». Ex: «2026/01/15 00».

Variable: Identifier of the variable to correct: 1=Temperature, 2=Precipitation (rain), 3=Precipitation (snow), 4=Soil water reserve, 5=Snow on the ground (snowpack), 6=Saturation of the soil water reserve.

CoefAdditif: Additive coefficient.

CoefMultiplicatif: Multiplicative coefficient.

CoeffSaturationCouche1: Saturation coefficient of soil layer 1.

CoeffSaturationCouche2: Saturation coefficient of soil layer 2.

CoeffSaturationCouche3: Saturation coefficient of soil layer 3.

TypeGroupe: Indicates the type of group (RHHU group or correction group) specified by «NomGroupe» for applying the correction. Value: 0=All RHHUs, 1=GroupeUHRH, 2=GroupeCorrection.

NomGroupe: Group name. The correction is applied to each RHHU belonging to the RHHU group or correction group.



Ex:

1;2020/01/01 00;2021/01/01 00;2;0;0.5;0;aucun

1;2020/01/01 00;2021/01/01 00;3;0;0.5;0;aucun

1;2020/01/01 00;2021/01/01 00;6;0.8;0.8;0.8;1;groupe1



Lines in the file that do not start with «0;» or «1;» are treated as comments and ignored.

<a id="degre-jour-bande-csv"></a>
## degre-jour-bande.csv   (/simulation/\{nom_simulation\})

Parameter file for the DEGRE JOUR BANDE calculation sub-model (snowpack evolution and melt).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; DEGRE JOUR BANDE» |
| Line 4 | Blank line |
| Line 5 | «CLASSE INTEGRE CONIFERES;» \{ListOccSolM1\} |
| Line 6 | «CLASSE INTEGRE FEUILLUS;» \{ListOccSolM2\} |
| Line 7 | Blank line |
| Line 8 | «NOM FICHIER STATION NEIGE CONIFERS;» \{NomFichierNeigeM1\} |
| Line 9 | «NOM FICHIER STATION NEIGE FEUILLUS;» \{NomFichierNeigeM2\} |
| Line 10 | «NOM FICHIER STATION NEIGE DECOUVERTS;» \{NomFichierNeigeM3\} |
| Line 11 | Blank line |
| Line 12 | «INTERPOLATION STATION NEIGE CONIFERS;» \{InterpolationM1\} |
| Line 13 | «INTERPOLATION STATION NEIGE FEUILLUS;» \{InterpolationM2\} |
| Line 14 | «INTERPOLATION STATION NEIGE DECOUVERTS;» \{InterpolationM3\} |
| Line 15 | Blank line |
| Line 16 | «HAUTEUR BANDE(m);» \{HauteurBande\} |
| Line 17 | Blank line |
| Line 18 | Comment |
| Line 19 and following | \{IdUhrh\} \{TauxFonte\} \{DensiteMax\} \{ConstanteTassement\} \{SeuilFonteM1\} \{SeuilFonteM2\} \{SeuilFonteM3\} \{TauxFonteM1\} \{TauxFonteM2\} \{TauxFonteM3\} \{SeuilAlbedo\} |

ListOccSolM1: List of identifiers (1 to x) of the land-cover classes associated with environment 1 (e.g. conifers). Identifier 1 corresponds to the first land-cover class in [occupation_sol.cla](#occupation-sol-cla), identifier 2 to the second class, and so on. Identifiers must be separated by a semicolon «;».

ListOccSolM2: List of identifiers (1 to x) of the land-cover classes associated with environment 2 (e.g. deciduous).

NomFichierNeigeM1: Name of the station file (.stn) for snow updating for environment 1 (e.g. conifers). The same file may be used for all 3 environments (aggregated classes). The specified path may be relative to the project folder (e.g. «[neige/station.stn](#station-stn)»).

NomFichierNeigeM2: Name of the station file (.stn) for snow updating for environment 2 (e.g. deciduous). The same file may be used for all 3 environments (aggregated classes). The specified path may be relative to the project folder (e.g. «[neige/station.stn](#station-stn)»).

NomFichierNeigeM3: Name of the station file (.stn) for snow updating for environment 3 (e.g. open areas). The same file may be used for all 3 environments (aggregated classes). The specified path may be relative to the project folder (e.g. «[neige/station.stn](#station-stn)»).

InterpolationM1: Interpolation method to use for snow updating over environment 1 (e.g. conifers). Possible values: «MOYENNE 3 STATIONS» or «THIESSEN».

InterpolationM2: Interpolation method to use for snow updating over environment 2 (e.g. deciduous). Possible values: «MOYENNE 3 STATIONS» or «THIESSEN».

InterpolationM3: Interpolation method to use for snow updating over environment 3 (e.g. open areas). Possible values: «MOYENNE 3 STATIONS» or «THIESSEN».

HauteurBande: Height of the elevation bands (m).

IdUhrh: RHHU identifier.

TauxFonte: Melt rate (snow-ground) (mm/day).

DensiteMax: Maximum snowpack density (kg/m3).

ConstanteTassement: Compaction constant.

SeuilFonteM1: Temperature threshold above which melt occurs (environment 1, e.g. conifers) (°C).

SeuilFonteM2: Temperature threshold above which melt occurs (environment 2, e.g. deciduous) (°C).

SeuilFonteM3: Temperature threshold above which melt occurs (environment 3, e.g. open areas) (°C).

TauxFonteM1: Melt rate in air (environment 1, e.g. conifers) (mm/day/°C).

TauxFonteM2: Melt rate in air (environment 2, e.g. deciduous) (mm/day/°C).

TauxFonteM3: Melt rate in air (environment 3, e.g. open areas) (mm/day/°C).

SeuilAlbedo: Threshold for the «Exponential with threshold» albedo algorithm (cm).

<a id="degre-jour-glacier-csv"></a>
## degre-jour-glacier.csv   (/simulation/\{nom_simulation\})

Parameter file for the DEGRE JOUR GLACIER calculation sub-model (ice evolution and melt).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; DEGRE JOUR GLACIER» |
| Line 4 | Blank line |
| Line 5 | «CLASSE INTEGRE GLACIER;» \{ListOccSolGlacier\} |
| Line 6 | Blank line |
| Line 7 | «DENSITE GLACE(kg/m3);» \{DensiteGlace\} |
| Line 8 | Blank line |
| Line 9 | «CONSTANTE EMPIRIQUE 0;» \{ConstanteEmpirique0\} |
| Line 10 | «CONSTANTE EMPIRIQUE 1;» \{ConstanteEmpirique1\} |
| Line 11 | Blank line |
| Line 12 | «EPAISSEUR GLACE MIN(m);» \{EpaisseurGlaceMin\} |
| Line 13 | «EPAISSEUR GLACE MAX(m);» \{EpaisseurGlaceMax\} |
| Line 14 | Blank line |
| Line 15 | «MASSE GLACE FIXE(0/1);» \{MasseGlaceFixe\} |
| Line 16 | Blank line |
| Line 17 | Comment |
| Line 18 and following | \{IdUhrh\} \{TauxFonte\} \{SeuilFonte\} \{Albedo\} |

ListOccSolGlacier: List of identifiers (1 to x) of the land-cover classes associated with the «Glaciers» environment (ice). Identifier 1 corresponds to the first land-cover class in [occupation_sol.cla](#occupation-sol-cla), identifier 2 to the second class, and so on. Identifiers must be separated by a semicolon «;».

DensiteGlace: Ice density (kg/m3).

ConstanteEmpirique0 : Empirical constant 0 (c0).

ConstanteEmpirique1 : Empirical constant 1 (c1).

EpaisseurGlaceMin: (linear ice-thickness equation) (m).

EpaisseurGlaceMax: (linear ice-thickness equation) (m).

MasseGlaceFixe: Prevents melt of the ice store (value: «1»). Otherwise set to «0» to allow melt.

IdUhrh: RHHU identifier.

TauxFonte: Melt rate in air (environment 1, e.g. glacier) (mm/day/°C).

SeuilFonte: Temperature threshold above which melt occurs (environment 1, e.g. glacier) (°C).

Albedo: Albedo (environment 1, e.g. glacier) (0-1).

<a id="degre-jour-modifie-csv"></a>
## degre_jour_modifie.csv   (/simulation/\{nom_simulation\})

Parameter file for the DEGRE JOUR MODIFIE calculation sub-model (snowpack evolution and melt).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; DEGRE JOUR MODIFIE» |
| Line 4 | Blank line |
| Line 5 | «CLASSE INTEGRE CONIFERES;» \{ListOccSolM1\} |
| Line 6 | «CLASSE INTEGRE FEUILLUS;» \{ListOccSolM2\} |
| Line 7 | Blank line |
| Line 8 | «NOM FICHIER STATION NEIGE CONIFERS;» \{NomFichierNeigeM1\} |
| Line 9 | «NOM FICHIER STATION NEIGE FEUILLUS;» \{NomFichierNeigeM2\} |
| Line 10 | «NOM FICHIER STATION NEIGE DECOUVERTS;» \{NomFichierNeigeM3\} |
| Line 11 | Blank line |
| Line 12 | «INTERPOLATION STATION NEIGE CONIFERS;» \{InterpolationM1\} |
| Line 13 | «INTERPOLATION STATION NEIGE FEUILLUS;» \{InterpolationM2\} |
| Line 14 | «INTERPOLATION STATION NEIGE DECOUVERTS;» \{InterpolationM3\} |
| Line 15 | Blank line |
| Line 16 | «MISE A JOUR GRILLE NEIGE;» \{MiseAJourGrilleNeige\} |
| Line 17 | «NOM FICHIER GRILLE NEIGE;» \{NomFichierGrilleNeige\} |
| Line 18 | Blank line |
| Line 19 | Comment |
| Line 20 and following | \{IdUhrh\} \{TauxFonte\} \{DensiteMax\} \{ConstanteTassement\} \{SeuilFonteM1\} \{SeuilFonteM2\} \{SeuilFonteM3\} \{TauxFonteM1\} \{TauxFonteM2\} \{TauxFonteM3\} \{SeuilAlbedo\} |

ListOccSolM1: List of identifiers (1 to x) of the land-cover classes associated with environment 1 (e.g. conifers). Identifier 1 corresponds to the first land-cover class in [occupation_sol.cla](#occupation-sol-cla), identifier 2 to the second class, and so on. Identifiers must be separated by a semicolon «;».

ListOccSolM2: List of identifiers (1 to x) of the land-cover classes associated with environment 2 (e.g. deciduous).

NomFichierNeigeM1: Name of the station file (.stn) for snow updating for environment 1 (e.g. conifers). The same file may be used for all 3 environments (aggregated classes). The specified path may be relative to the project folder (e.g. «[neige/station.stn](#station-stn)»).

NomFichierNeigeM2: Name of the station file (.stn) for snow updating for environment 2 (e.g. deciduous). The same file may be used for all 3 environments (aggregated classes). The specified path may be relative to the project folder (e.g. «[neige/station.stn](#station-stn)»).

NomFichierNeigeM3: Name of the station file (.stn) for snow updating for environment 3 (e.g. open areas). The same file may be used for all 3 environments (aggregated classes). The specified path may be relative to the project folder (e.g. «[neige/station.stn](#station-stn)»).

InterpolationM1: Interpolation method to use for snow updating over environment 1 (e.g. conifers). Possible values: «MOYENNE 3 STATIONS» or «THIESSEN».

InterpolationM2: Interpolation method to use for snow updating over environment 2 (e.g. deciduous). Possible values: «MOYENNE 3 STATIONS» or «THIESSEN».

InterpolationM3: Interpolation method to use for snow updating over environment 3 (e.g. open areas). Possible values: «MOYENNE 3 STATIONS» or «THIESSEN».

MiseAJourGrilleNeige: Enables or disables snowpack updating using a data grid (raster map). Possible values: 0=disabled, 1=enabled.

NomFichierGrilleNeige: Path to the parameter file (.grn) for snow updating with a data grid. The specified path may be relative to the project folder (e.g. «[neige-grille/grilleneige.grn](#grilleneige-grn)»).

IdUhrh: RHHU identifier.

TauxFonte: Melt rate (snow-ground) (mm/day).

DensiteMax: Maximum snowpack density (kg/m3).

ConstanteTassement: Compaction constant.

SeuilFonteM1: Temperature threshold above which melt occurs (environment 1, e.g. conifers) (°C).

SeuilFonteM2: Temperature threshold above which melt occurs (environment 2, e.g. deciduous) (°C).

SeuilFonteM3: Temperature threshold above which melt occurs (environment 3, e.g. open areas) (°C).

TauxFonteM1: Melt rate in air (environment 1, e.g. conifers) (mm/day/°C).

TauxFonteM2: Melt rate in air (environment 2, e.g. deciduous) (mm/day/°C).

TauxFonteM3: Melt rate in air (environment 3, e.g. open areas) (mm/day/°C).

SeuilAlbedo: Threshold for the «Exponential with threshold» albedo algorithm (cm).

<a id="etats"></a>
## etats (folder)   (/)

Contains files with initialization values (initial conditions) for the models. File names must include as a suffix the date and time of the simulation time step ([acheminement_riviere_AAAAMMJJHH.csv](#etats)). When specified in the simulation file (LECTURE ETAT lines), the models’ internal simulation variables are initialized at the specified date with the values from the files (e.g. LECTURE ETAT ACHEMINEMENT RIVIERE;[etats/acheminement_riviere_2020010100.csv](#etats)). Files can be generated by entering the date and time of the desired time step (e.g. ECRITURE ETAT ACHEMINEMENT RIVIERE;2020-07-01 00:00). The keyword «fin» may be entered instead of the date and time to obtain the states of the last time step of the simulation. The keyword «tous» may be entered instead of the date and time to obtain the states of all simulation time steps. The REPERTOIRE ECRITURE ETAT lines specify the folder where the files will be saved.

<a id="etp-mc-guiness-csv"></a>
## etp-mc-guiness.csv   (/simulation/\{nom_simulation\})

Parameter file for the ETP-MC-GUINESS calculation sub-model (potential evapotranspiration).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; ETP-MC-GUINESS» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{CoefOptimisation\} |

IdUhrh: RHHU identifier.

CoefOptimisation: Multiplicative optimization coefficient.

<a id="grillemeteo-csv"></a>
## grillemeteo.csv   (/simulation/\{nom_simulation\})

Parameter file for the GRILLE calculation sub-model (interpolation of meteorological data).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; GRILLE» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

IdUhrh: RHHU identifier.

GradientTemp: Vertical temperature gradient (°C/100m).

GradientPrecip: Vertical precipitation gradient (mm/100m).

PassagePluieNeige: Rain-to-snow transition temperature (°C).

<a id="grilleneige-grn"></a>
## grilleneige.grn   (/neige-grille)

Parameter file for snow updating with a data grid.

| Line | Description |
| --- | --- |
| Line 1 | \{TypeCoord\} |
| Line 2 | \{UniteMesure\} |
| Line 3 | \{TypePasTemps\} |
| Line 4 | Comment |
| Line 5 | \{Frequence\} \{ChaineNbChar\} \{DossierDonnees\} |
| Line 6 | \{Prefixe\} |
| Line 7 | \{NbTypeDonnees\} |

TypeCoord: Unused. Set to «2».

UniteMesure : Unit of measurement of the data files. Possible values: «1» metres, «2» cm, «3» mm.

TypePasTemps: Unused. Set to «1».

Frequence: Unused. Must be set to «32».

ChaineNbChar: Unused. Set to «@0».

DossierDonnees: Name of the folder containing the snow data (data grids). The specified folder name (path) may be relative to the project folder (e.g. «neige-grille/donnees»).

Prefixe: Prefix of the data file names. The prefix is prepended to the file names when reading. Ex: prefix = «neige_», file name = «[neige_2003_01_22_24h.een](#neige-2003-01-22-24h-een)». The specified prefix may be an empty string if no prefix is wanted.

NbTypeDonnees: Unused. Must be set to «3».



The snow water equivalent and snowpack depth data files must be located in the specified data folder and use the following naming: \{Prefixe\}YYYY_MM_DD_24h.een (water equivalent) and \{Prefixe\}YYYY_MM_DD_24h.hau (depth). Ex: for an update on 22 January 2003: [neige_2003_01_22_24h.een](#neige-2003-01-22-24h-een) and [neige_2003_01_22_24h.hau](#neige-2003-01-22-24h-hau).



Example .grn file:

2

3

1

Comment

32 @0 neige-grille/donnees

neige_

3

<a id="grilleprevision-csv"></a>
## grilleprevision.csv   (/simulation/\{nom_simulation\})

Parameter file for the GRILLE PREVISION calculation sub-model (meteorological forecasts).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; GRILLE PREVISION» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

IdUhrh: RHHU identifier.

GradientTemp: Vertical temperature gradient (°C/100m).

GradientPrecip: Vertical precipitation gradient (mm/100m).

PassagePluieNeige: Rain-to-snow transition temperature (°C).

<a id="hydro-quebec-csv"></a>
## hydro_quebec.csv   (/simulation/\{nom_simulation\})

Parameter file for the HYDRO-QUEBEC calculation sub-model (potential evapotranspiration).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; HYDRO-QUEBEC» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{CoefOptimisation\} |

IdUhrh: RHHU identifier.

CoefOptimisation: Multiplicative optimization coefficient.

<a id="hydrogramme-hgm"></a>

## hydrogramme.hgm   (/hgm)

Contains the values computed for the geomorphological hydrograph according to the parameters specified for the ONDE_CINEMATIQUE model ([onde_cinematique.csv](#onde-cinematique-csv)). This file is generated automatically by HYDROTEL when it does not exist.

<a id="ind-fol-def"></a>
## ind_fol.def   (/physio)

Leaf-area-index data file.

| Line | Description |
| --- | --- |
| Line 1 | \{Format\} |
| Line 2 | \{NbOccupation\} \{NbJour\} |
| Line 3 | Comment |
| Line 4 | Comment |
| Line 5 and following | \{Jour\} \{ValeurOccSol1\} \{ValeurOccSol2\} \{ValeurOccSol3\} etc… |

Format: Set to «2» (unused).

NbOccupation: Number of land-cover classes.

NbJour: Number of days (lines with values) specified in the file.

Jour: Start day of application of the parameters (Julian day).

ValeurOccSol1 to ValeurOccsolX: Leaf-area-index values for the current day for each land-cover class. Values are separated by a space or a tab.

Ex:

2

10 3

Indices foliaire au jour J / Leaf area index on day D

Jour "FORETS CONIFERES" "FORETS FEUILLUS" "FORETS MIXTES" "AGRICULTURE"

(continuation of line 4)  "URBAIN" "ROUTES" "MILIEUX OUVERTS" "EAU" "SOLS NUS"

(continuation of line 4)  "MILIEUX HUMIDES"

1		5	3	3	0	0	0	1	0	0	2

210	5	5	5	2	0	0	3	0	0	4

365	5	3	3	0	0	0	1	0	0	2



Land-cover class names must be identical to those in the class identification file ([occupation_sol.csv](#occupation-sol-csv)).

Values are linearly interpolated between the dates provided in the files.

File names must be:   ind_fol.< year (yyyy)>  (e.g. ind_fol.1995). There must be one file for each year for which values are specified. The «.def» file extension ([ind_fol.def](#ind-fol-def)) may also be used to specify default values to be used for all years (or years that are not specified).

Files must end with day 365 (last line of the file).

<a id="lacs-shp-lacs-prj-lacs-dbf-lacs-shx"></a>
## lacs.shp, lacs.prj, lacs.dbf, lacs.shx   (/physitel)

Vector map (shapefile) of lakes.

<a id="lecture-acheminement-csv"></a>
## lecture_acheminement.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the river-routing calculation sub-model. Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE ACHEMINEMENT RIVIERE» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER DEBIT AMONT;»\{FichierDebitAmont\} |
| Line 6 | «NOM FICHIER DEBIT AVAL;»\{FichierDebitAval\} |

FichierDebitAmont: Path of the upstream discharge file (m3/s). This file must have the same format as the output file «resultat/debit_amont.csv».

FichierDebitAval: Path of the downstream discharge file (m3/s). This file must have the same format as the output file «resultat/debit_aval.csv».



Specified file paths may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="lecture-bilan-vertical-csv"></a>
## lecture_bilan_vertical.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the vertical water-budget calculation sub-model. Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE BILAN VERTICAL» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER PRODUCTION BASE;»\{FichierProdBase\} |
| Line 6 | «NOM FICHIER PRODUCTION HYPO;»\{FichierProdHypo\} |
| Line 7 | «NOM FICHIER PRODUCTION SURF;»\{FichierProdSurf\} |

FichierProdBase: Path of the file for the water depth produced by the 3rd layer (base) (mm). This file must have the same format as the output file «resultat/production_base.csv».

FichierProdHypo : Path of the file for the water depth produced by the 2nd layer (hypodermic) (mm). This file must have the same format as the output file «resultat/production_hypo.csv».

FichierProdSurf : Path of the file for the water depth produced at the surface (mm). This file must have the same format as the output file «resultat/production_surf.csv».



Specified file paths may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="lecture-etp-csv"></a>
## lecture_etp.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the potential-evapotranspiration calculation sub-model. Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE EVAPOTRANSPIRATION» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER ETP;»\{FichierEtp\} |

FichierEtp: Path of the potential evapotranspiration file (mm). This file must have the same format as the output file «resultat/etp.csv». The specified path may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="lecture-fonte-glacier-csv"></a>
## lecture_fonte_glacier.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the ice-melt calculation sub-model. Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE FONTE GLACIER» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER APPORT GLACIER;»\{FichierApportGlace\} |

FichierApportGlace : Path of the file for ice-melt contribution (mm). This file must have the same format as the output file «resultat/glacier-apport.csv». The specified path may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="lecture-fonte-neige-csv"></a>
## lecture_fonte_neige.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the snowmelt calculation sub-model. Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE FONTE NEIGE» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER APPORT;»\{FichierApport\} |
| Line 6 | «NOM FICHIER HAUTEUR COUVERT NIVAL;»\{FichierHauteurCouvert\} |

FichierApport : Path of the file for contributions (snowmelt + rain) (mm). This file must have the same format as the output file «resultat/apport.csv».

FichierHauteurCouvert: Path of the snowpack-depth file (m). This file must have the same format as the output file «resultat/hauteur_neige.csv».



Specified file paths may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="lecture-interpolation-csv"></a>
## lecture_interpolation.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the meteorological-data interpolation sub-model. Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE INTERPOLATION DONNEES» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER TMIN;»\{FichierTMin\} |
| Line 6 | «NOM FICHIER TMAX;»\{FichierTMax\} |
| Line 7 | «NOM FICHIER TMIN_JOUR;»\{FichierTMinJour\} |
| Line 8 | «NOM FICHIER TMAX_JOUR;»\{FichierTMaxJour\} |
| Line 9 | «NOM FICHIER PLUIE;»\{FichierPluie\} |
| Line 10 | «NOM FICHIER NEIGE;»\{FichierNeige\} |

FichierTMin : Path of the minimum-temperature file (°C). This file must have the same format as the output file «resultat/tmin.csv».

FichierTMax : Path of the maximum-temperature file (°C). This file must have the same format as the output file «resultat/tmax.csv».

FichierTMinJour: Path of the daily minimum-temperature file (°C). This file must have the same format as the output file «resultat/tmin_jour.csv».

FichierTMaxJour: Path of the daily maximum-temperature file (°C). This file must have the same format as the output file «resultat/tmax_jour.csv».

FichierPluie: Path of the precipitation (rain) file (mm). This file must have the same format as the output file «resultat/pluie.csv».

FichierNeige: Path of the precipitation (snow) file (SWE) (mm). This file must have the same format as the output file «resultat/neige.csv».



Specified file paths may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="lecture-ruisselement-csv"></a>
## lecture_ruisselement.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the overland-flow calculation sub-model. Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE RUISSELEMENT SURFACE» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER APPORT LATERAL;»\{FichierApportLateral\} |

FichierApportLateral : Path of the lateral-inflow file (m3/s). This file must have the same format as the output file «resultat/apport_lateral.csv». The specified path may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="lecture-tempsol-csv"></a>
## lecture_tempsol.csv   (/simulation/\{nom_simulation\})

Parameter file for read mode for the soil-temperature calculation sub-model (frost depth). Read mode replaces a sub-model’s output by supplying those data directly from a file. The sub-model is then not simulated (executed).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LECTURE TEMPERATURE DU SOL» |
| Line 4 | Blank line |
| Line 5 | «NOM FICHIER TEMPSOL;»\{FichierTempSol\} |

FichierTempSol : Path of the frost-depth file (cm). This file must have the same format as the output file «resultat/profondeur_gel.csv». The specified path may be relative to the project folder (e.g. «modelecture/fichier_de_donnees.csv»).

<a id="linacre-csv"></a>
## linacre.csv   (/simulation/\{nom_simulation\})

Parameter file for the LINACRE calculation sub-model (potential evapotranspiration).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; LINACRE» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{TempMoisFroid\} \{TempMoisChaud\} \{Albedo\} \{CoefOptimisation\} |

IdUhrh: RHHU identifier.

TempMoisFroid: Mean temperature of the coldest month (°C).

TempMoisChaud: Mean temperature of the warmest month (°C).

Albedo: Albedo (reference surface) (0-1).

CoefOptimisation: Multiplicative optimization coefficient.

<a id="milieux-humides-isoles-csv"></a>
## milieux_humides_isoles.csv   (/simulation/\{nom_simulation\})

Parameter file for isolated wetlands.

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{IdUhrh\} \{SuperficieUhrh\} \{SuperficieMaxMH\} \{FractionUhrhDraineeMH\} \{FractionSuperficieMax\} \{HauteurEauNormale\} \{HauteurEauMax\} \{KsatBase\} \{CoeffETP\} \{RatioVolumesEauSortant\} \{SauvegardeEtats\} |

IdUhrh: RHHU identifier.

SuperficieUhrh (uhrh_a): RHHU area (km2).

SuperficieMaxMH (wet_a) : Maximum area of the equivalent wetland (km²).

FractionUhrhDraineeMH (wet_dra_fr) : Fraction of the RHHU drained by the equivalent wetland (0-1).

FractionSuperficieMax (frac) : Fraction of the maximum area used to determine the normal area (0-1).

HauteurEauNormale (wetdnor) : Normal water depth (m).

HauteurEauMax (wetdmax) : Maximum water depth (m).

KsatBase (ksat_bs) : Saturated hydraulic conductivity at the base of the equivalent wetland (EW) (mm/h).

CoeffETP (c_ev) : Potential evapotranspiration coefficient (0-1).

RatioVolumesEauSortant (c_prod): Ratio used in calculating volumes of water leaving the EW (0-1).

SauvegardeEtats: Saving of equivalent-wetland (EW) state variables. Value «0»: do not save. Value «1»: save. Data are saved in the result file [wetland_isole.csv](#wetland-isole-csv).

<a id="milieux-humides-riverains-csv"></a>
## milieux_humides_riverains.csv   (/simulation/\{nom_simulation\})

Parameter file for riparian wetlands.

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{IdTroncon\} \{SuperficieUhrh\} \{SuperficieMaxMH\} \{FractionUhrhDraineeTronconAmont\} \{FractionUhrhDraineeMH\} \{FractionUhrhDraineeTronconAval\} \{LongueurMH\} \{LongueurTronconAmont\} \{LongueurTronconAval\} \{HauteurEauNormale\} \{HauteurEauMax\} \{FractionSuperficieMax\} \{KsatBerge\} \{KsatBase\} \{EpaisseurAquiferePotentielle\} \{SauvegardeEtats\} |

IdTroncon: Reach identifier.

SuperficieUhrh (uhrh_a): RHHU area (km2).

SuperficieMaxMH (wet_a) : Maximum area of the equivalent wetland (km²).

FractionUhrhDraineeTronconAmont (wetaup_fr) : Fraction of RHHUs drained by the reach upstream of the equivalent riparian wetland (0-1).

FractionUhrhDraineeMH (wetadra_fr) : Fraction of RHHUs drained by the equivalent riparian wetland (0-1).

FractionUhrhDraineeTronconAval (wetadown_fr) : Fraction of RHHUs drained by the reach downstream of the equivalent riparian wetland (0-1).

LongueurMH (longueur) : Length of the equivalent riparian wetland (m).

LongueurTronconAmont (longueur amont) : Length of the reach segment located upstream of the equivalent riparian wetland (m).

LongueurTronconAval (longueur aval) : Length of the reach segment located downstream of the equivalent riparian wetland (m).

HauteurEauNormale (wetdnor) : Normal water depth (m).

HauteurEauMax (wetdmax) : Maximum water depth (m).

FractionSuperficieMax (frac) : Fraction of the maximum area used to determine the normal area (0-1).

KsatBerge (ksat_bk): Saturated hydraulic conductivity of the EW bank (mm/h).

KsatBase (ksat_bs): Saturated hydraulic conductivity at the base of the EW (mm/h).

EpaisseurAquiferePotentielle (th_aq): Potential aquifer thickness (m).

SauvegardeEtats: Saving of equivalent-wetland (EW) state variables. Value «0»: do not save. Value «1»: save. Data are saved in the result file [wetland_riverain.csv](#wetland-riverain-csv)

<a id="moyennes-ponderees-troncon-idtroncon-csv"></a>
## moyennes-ponderees-troncon\{IdTroncon\}.csv   (/simulation/\{nom_simulation\}/resultat)

Result file containing area-weighted means of temperatures, precipitation, snowpack, potential evapotranspiration and actual evapotranspiration for the reach specified at the end of the file name (IdTroncon). Means are computed from the results and weighted by the areas of the RHHUs upstream of the reach. This file can be generated by configuring the «TRONCONS_MOYENNES_PONDEREES;» line in [output.csv](#output-csv).

| Line | Description |
| --- | --- |
| Line 1 | «Moyennes pondérées;( VERSION 4.3.0.0000 )» |
| Line 2 | «Troncon;\{IdTroncon\}» |
| Line 3 | «UHRH amont;1;2;3;4;5;6;7;8;9;10» |
| Line 4 | Comment |
| Line 5 and following | \{DateHeure\} \{TMin (°C)\} \{TMax (°C)\} \{TMoy °C\} \{Pluie (mm)\} \{Neige (EEN) (mm)\} \{CouvertNival (EEN) (mm)\} \{ETP (mm)\} \{ETR (mm)\} |

<a id="moyenne-3-stations-csv"></a>
## moyenne_3_stations.csv   (/simulation/\{nom_simulation\})

Parameter file for the MOYENNE 3 STATIONS calculation sub-model (interpolation of meteorological data).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; MOYENNE 3 STATIONS» |
| Line 4 | Blank line |
| Line 5 | «GRADIENT TEMPERATURE STATION(C/100m);»\{GradientStationTemp\} |
| Line 6 | «GRADIENT PRECIPITATION STATION(mm/100m);»\{GradientStationPrecip\} |
| Line 7 | Blank line |
| Line 8 | Comment |
| Line 9 and following | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

GradientStationTemp: Vertical temperature gradient for interpolating missing data at stations (°C/100m).

GradientStationPrecip: Vertical precipitation gradient for interpolating missing data at stations (°C/100m).

IdUhrh: RHHU identifier.

GradientTemp: Vertical temperature gradient (°C/100m).

GradientPrecip: Vertical precipitation gradient (mm/100m).

PassagePluieNeige: Rain-to-snow transition temperature (°C).

<a id="neige-pgn"></a>
## neige.pgn   (/meteo)

Contains the weights of grid points/RHHUs for snow updating by grid. Weights are computed by HYDROTEL and this file is generated automatically when it does not exist.

<a id="neige-2003-01-22-24h-een"></a>
## neige_2003_01_22_24h.een   (/neige-grille/donnees)

Data grid (raster map) containing snow water equivalent values for snow updating by grid. The unit may be metres, centimetres or millimetres and is specified in the snow-update-by-grid configuration file ([grilleneige.grn](#grilleneige-grn)).

<a id="neige-2003-01-22-24h-hau"></a>
## neige_2003_01_22_24h.hau   (/neige-grille/donnees)

Data grid (raster map) containing snowpack-depth values for snow updating by grid. The unit may be metres, centimetres or millimetres and is specified in the snow-update-by-grid configuration file ([grilleneige.grn](#grilleneige-grn)).

<a id="neige01-nei"></a>
## NEIGE01.nei   (/neige)

Snow observations for station «NEIGE01».

| Line                        | Description                                                  |
| --- | --- |
| For each line of the file | \{Date\} \{Snow depth\} \{Snow water equivalent\} \{Density\} |

Date: (DD/MM/YYYY).

Snow depth: Snowpack depth (cm).

Snow water equivalent: (mm).

Density: Relative density (g/cm3).



The snowpack-depth value may be set to -999 when it is unknown. In that case, depth is computed from the observed water equivalent multiplied by the simulated density (depth before update / water store before update). If the water store before update is 0 (no snowpack), depth is then computed as: depth = observed water equivalent * 2.5.

<a id="noeuds-nds"></a>
## nœuds.nds   (/physitel)

Information file on the nodes (junctions) of the rasterized hydrological network.

| Line | Description |
| --- | --- |
| Line 1 | \{Type\} |
| Line 2 | \{NbNoeuds\} |
| Line 3 | Comment |
| Line 4 and following | \{IdNoeud\} \{CoordX\} \{CoordY\} \{Altitude\} \{Largeur\} |

Type: Set to «1» (unused).

NbNoeuds: Number of nodes.

IdNoeud : Node identifier.

CoordX : X coordinate of the node. The coordinate must be in the same projection system as the elevation map ([altitude.tif](#altitude-tif)).

CoordY : Y coordinate of the node. The coordinate must be in the same projection system as the elevation map ([altitude.tif](#altitude-tif)).

Altitude: Altitude (elevation) corresponding to the node location (m).

Largeur: Set to «0» (unused).

<a id="obs-sim-flows-csv"></a>
## obs-sim-flows.csv   (/simulation/\{nom_simulation\}/resultat)

Result file containing observed and simulated discharges for the reaches configured for statistics calculation (according to [stats.txt](#stats-txt)). This file is generated only when statistics are computed. It simplifies reading of results by an external program in order to perform automatic calibration of simulation parameters. Results for the «Reach/Station» pairs present in [stats.txt](#stats-txt) are saved in [obs-sim-flows.csv](#obs-sim-flows-csv) and presented in columns in the order in which they appear in [stats.txt](#stats-txt), as shown in the following table:

| Column 1 | Column 2 | Column 3 | Column 4 | Column 5 | etc… |
| --- | --- | --- | --- | --- | --- |
| Date/Time of the simulation time step | Observed discharge for the reach on the 1st line of [stats.txt](#stats-txt) | Simulated discharge for the reach on the 1st line of [stats.txt](#stats-txt) | Observed discharge for the reach on the 2nd line of [stats.txt](#stats-txt) | Simulated discharge for the reach on the 2nd line of [stats.txt](#stats-txt) |  |

<a id="occupation-sol-cla"></a>
## occupation_sol.cla   (/physitel)

Contains the number of pixels (tiles) for each land-cover class for each RHHU.

| Line | Description |
| --- | --- |
| Line 1 | \{Type\} |
| Line 2 | \{NbClasseOccSol\} |
| Line 3 | uhrh "\{NomOccSol1\}" "\{NomOccSol2\}" "\{NomOccSol3\}" etc… |
| Line 4 and following | \{IdUhrh\} \{NbPixelOccSol1\} \{NbPixelOccSol2\} \{NbPixelOccSol3\} etc… |

Type: Set to «1» (unused).

NbClasseOccSol: Number of land-cover classes.

NomOccSol1 to NomOccSolX : Names of the land-cover classes. Class names must be separated by a space and delimited with double quotes ("). Names must also be identical to those in the class identification file ([occupation_sol.csv](#occupation-sol-csv)).

IdUhrh : RHHU identifier.

NbPixelOccSol1 to NbPixelOccSolX: Number of pixels (tiles) for each land-cover class for the current RHHU. These values are determined from the land-cover raster map ([occupation_sol.tif](#occupation-sol-tif)).



Ex:

1

10

uhrh "FORETS CONIFERES" "FORETS FEUILLUS" "FORETS MIXTES" "AGRICULTURE"

(continuation of line 3)  "URBAIN" "ROUTES" "MILIEUX OUVERTS" "EAU" "SOLS NUS"

(continuation of line 3)  "MILIEUX HUMIDES"

1 0 102 0 0 1111 280 118 119 0 24

2 0 579 0 0 883 125 52 210 0 120

3 0 0 0 0 0 0 2 1 0 0

etc…

<a id="occupation-sol-csv"></a>
## occupation_sol.csv   (/physitel)

Contains the names of the land-cover classes.

| Line | Description |
| --- | --- |
| Line 1 | \{NbOccSol\} |
| Line 2 and following | \{NomOccsol\} |

NbOccSol: Number of land-cover classes.

NomOccSol: Name of the land-cover class.



There must be one line for each land-cover class. The order of the classes must match the identifiers of the land-cover raster map ([occupation_sol.tif](#occupation-sol-tif)). Ex: the 1st class name corresponds to identifier «1» on the map, the 2nd class name corresponds to identifier «2» on the map, and so on.

<a id="occupation-sol-tif"></a>
## occupation_sol.tif   (/physitel)

Raster map (GeoTIFF) containing the distribution of land-cover classes. Identifiers must match (starting at value 1) the order of the class names identified in [occupation_sol.csv](#occupation-sol-csv).

<a id="onde-cinematique-csv"></a>
## onde_cinematique.csv   (/simulation/\{nom_simulation\})

Parameter file for the ONDE CINEMATIQUE calculation sub-model (flow over the land portion of the basin) (overland flow).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; ONDE CINEMATIQUE» |
| Line 4 | Blank line |
| Line 5 | «CLASSE INTEGRE FORETS;» \{ListOccSolForets\} |
| Line 6 | «CLASSE INTEGRE EAUX;» \{ListOccSolEaux\} |
| Line 7 | Blank line |
| Line 8 | «LAME;» \{Lame\} |
| Line 9 | Blank line |
| Line 10 | «NOM FICHIER HGM;» \{NomFichierHGM\} |
| Line 11 | Blank line |
| Line 12 | Comment |
| Line 13 and following | \{IdUhrh\} \{ManningForets\} \{ManningEaux\} \{ManningAutres\} |

ListOccSolForets: List of identifiers (1 to x) of the land-cover classes associated with the «Forêts» environment. Identifier 1 corresponds to the first land-cover class in [occupation_sol.cla](#occupation-sol-cla), identifier 2 to the second class, and so on. Identifiers must be separated by a semicolon «;».

ListOccSolEaux: List of identifiers (1 to x) of the land-cover classes associated with the «Eaux» environment.

Lame: Reference depth for the geomorphological hydrograph (m).

NomFichierHGM: Name or path of the geomorphological hydrograph data file. The specified path may be relative to the project folder (e.g. «[hgm/hydrogramme.hgm](#hydrogramme-hgm)»).

IdUhrh : RHHU identifier.

ManningForets: Manning roughness coefficient for forested environments.

ManningEaux: Manning roughness coefficient for the «Eaux» environment.

ManningAutres: Manning roughness coefficient for other environments (not part of the Forests or Waters environments).

<a id="onde-cinematique-modifie-csv"></a>
## onde_cinematique_modifiee.csv   (/simulation/\{nom_simulation\})

Parameter file for the ONDE CINEMATIQUE MODIFIEE calculation sub-model (flow through the hydrographic network) (routing).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; ONDE CINEMATIQUE MODIFIEE» |
| Line 4 | Blank line |
| Line 5 | «METHODE_CALCUL_HAUTEUR;» \{MethodeCalculHauteur\} |
| Line 6 | Blank line |
| Line 7 | Comment |
| Line 8 and following | \{IdTroncon\} \{CoefRugosite\} \{CoefLargeur\} |

MethodeCalculHauteur: Method for calculating water depth in the reaches. Possible value from 1 to 3: (1) Rectangular section, (2) Trapezoidal section (Tiwari et al.) (2012), (3) HAND approach (discharge/depth file).

IdTroncon: Reach identifier.

CoefRugosite: Roughness optimization coefficient.

CoefLargeur: River-width optimization coefficient.

<a id="orientation-tif"></a>
## orientation.tif   (/physitel)

Raster map (GeoTIFF) containing flow directions. Possible flow values are: 1 (East), 2 (Northeast), 3 (North), 4 (Northwest), 5 (West), 6 (Southwest), 7 (South), 8 (Southeast).

<a id="output-csv"></a>
## output.csv   (/simulation/\{nom_simulation\})

Used to enable/disable saving of simulation variables, select reaches for saving, and set other parameters related to output variables.

Each line starts with the identifier of the parameter to specify, followed by the separator «;» and one or more values (also separated by «;»). For simulation variables, enter «1» to enable saving and «0» to disable it.

The following identifiers may be used:

TRONCONS: List of reaches whose variables are saved. Reach identifiers must be separated by «;». The keyword «tous» may be specified in the reach list to save variables for all reaches.

TRONCONS_MOYENNES_PONDEREES: List of reaches for saving area-weighted means of upstream RHHUs at the reaches (TMin, TMax, TMoy, PrecipPluie, PrecipNeige, CouvertNival, ETP, ETR). A result file is saved for each specified reach (moyennes-ponderees-troncon\{IdTroncon\}.csv).

OUTPUT_NETCDF: Output files in NetCDF format instead of text format (.csv). 0 (Disabled) (text format), 1 (Enabled) (NetCDF format).

SEPARATEUR: Specifies the column separator to use for text output files (.csv). When not specified, the separator «;» is used by default. Example to use the separator «,»: «SEPARATEUR;,».

FICHIERS_ETATS_SEPARATEUR: Specifies the column separator to use for state files. When not specified, the separator «;» is used by default. Example to use the separator «,»: «SEPARATEUR;,».

TMIN: Minimum temperature (°C).

TMAX: Maximum temperature (°C).

TMIN_JOUR: Daily minimum temperature (°C).

TMAX_JOUR: Daily maximum temperature (°C).

PLUIE: Rain precipitation (mm).

NEIGE: Snow precipitation (snow water equivalent) (mm).

APPORT: Contribution from snowmelt and rain (mm).

COUVERT_NIVAL: Snow water equivalent of the snowpack (mm).

HAUTEUR_NEIGE: Snowpack depth (m).

ALBEDO_NEIGE: Snow albedo (0-1).

APPORT_GLACIER: Ice-melt contribution (mm).

EAU_GLACIER: Ice water equivalent (m).

PROFONDEUR_GEL: Ground frost depth (cm).

ETP: Potential evapotranspiration (mm).

ETR1: Actual evapotranspiration of layer 1 (mm).

ETR2: Actual evapotranspiration of layer 2 (mm).

ETR3: Actual evapotranspiration of layer 3 (mm).

ETR_TOTAL: Total actual evapotranspiration (mm).

PRODUCTION_BASE: Water depth produced by the 3rd layer (base) (production) (mm).

PRODUCTION_HYPO: Water depth produced by the 2nd layer (hypodermic) (production) (mm).

PRODUCTION_SURF: Water depth produced by the 1st layer (surface) (production) (mm).

Q12: Vertical flow from layer 1 to 2 (mm).

Q23: Vertical flow from layer 2 to 3 (mm).

Q23_SOMME_ANNUELLE: Annual sums (per RHHU) of vertical flows from layer 2 to 3 (mm).

QRECHARGE: Groundwater recharge (mm).

THETA1: Water content of layer 1 (0-1).

THETA2: Water content of layer 2 (0-1).

THETA3: Water content of layer 3 (0-1).

APPORT_LATERAL: Lateral inflows of the reaches (m3/s).

APPORT_LATERAL_UHRH: Lateral inflows of the RHHUs (m3/s).

ECOULEMENT_SURF: Flow on the RHHU toward the hydrographic network (layer 1) (surface) (m3/s).

ECOULEMENT_HYPO: Flow on the RHHU toward the hydrographic network (layer 2) (hypodermic) (m3/s).

ECOULEMENT_BASE: Flow on the RHHU toward the hydrographic network (layer 3) (base) (m3/s).

DEBITS_AMONT: Discharge upstream of the reach (m3/s).

DEBITS_AVAL: Discharge downstream of the reach (m3/s).

HAUTEUR_AVAL : Water depth downstream of the reach (m).

DEBITS_AVAL_MOY7J_MIN: Annual and summer 7-day minimum mean discharge (m3/s).

TEMPERATURE_EAU : Mean water temperature (°C). 



Ex :

TRONCONS;tous

TRONCONS_MOYENNES_PONDEREES;1;91;152

OUTPUT_NETCDF;0

DEBITS_AMONT;0

DEBITS_AVAL;1

TMIN;1

TMAX;1



Ex :

TRONCONS;1;91

TRONCONS_MOYENNES_PONDEREES;

OUTPUT_NETCDF;1

APPORT;1

DEBITS_AVAL;1

<a id="parametres-sous-modeles-csv"></a>
## parametres_sous_modeles.csv   (/simulation/\{nom_simulation\})

Global parameter file. This file groups all parameters of all simulation sub-models into a single file. It contains the same parameters as the individual files, except that they are specified by RHHU group instead of individually for each RHHU.

<a id="penman-csv"></a>
## penman.csv   (/simulation/\{nom_simulation\})

Parameter file for the PENMAN calculation sub-model (potential evapotranspiration).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; PENMAN» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{HauteurMesureVent\} \{VitesseVent\} \{HauteurVegetation\} \{MethodeResistanceAero\} \{CoefOptimisation\} |

IdUhrh: RHHU identifier.

HauteurMesureVent : Height at which wind speed is measured (m).

VitesseVent : Wind speed at height Z (m/s).

HauteurVegetation: Vegetation height (reference surface) (m).

MethodeResistanceAero : Relation (equation) for calculating aerodynamic resistance. Value «0»: empirical relation. Value «1»: physically based relation.

CoefOptimisation: Multiplicative optimization coefficient.

<a id="penman-monteith-csv"></a>
## penman_monteith.csv   (/simulation/\{nom_simulation\})

Parameter file for the PENMAN-MONTEITH calculation sub-model (potential evapotranspiration).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; PENMAN-MONTEITH» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{HauteurMesureVent\} \{HauteurMesureHum\} \{VitesseVent\} \{HauteurVegetation\} \{ResistanceStomatale\} \{CoefOptimisation\} |

IdUhrh: RHHU identifier.

HauteurMesureVent : Height at which wind speed is measured (m).

HauteurMesureHum : Height at which humidity is measured (m).

VitesseVent : Wind speed at height Z (m/s).

HauteurVegetation: Vegetation height (reference surface) (m).

ResistanceStomatale : Stomatal resistance (reference surface) (s/m).

CoefOptimisation: Multiplicative optimization coefficient.

<a id="pente-tif"></a>
## pente.tif   (/physitel)

Raster map (GeoTIFF) containing slopes (‰) (per thousand).

<a id="physitelproject-txt"></a>
## physitelproject.txt   (/physitel)

File from the PHYSITEL project used to create the HYDROTEL project. The file contains the parameters of the calculations performed in the PHYSITEL project. This file is not used by HYDROTEL and is present for information only.

<a id="point-rdx"></a>
## point.rdx   (/physitel)

Contains the coordinates of the pixels (tiles) for each reach. This file is generated from the reach raster map.

| Line | Description |
| --- | --- |
| Line 1 | \{NbLigne\} \{NbColonne\} \{NbPixel\} |
| Line 2 and following | \{Ligne\} \{Colonne\} \{IdTroncon\} |

NbLigne: Number of rows of the reach matrix.

NbColonne: Number of columns of the reach matrix.

NbPixel: Number of pixels (tiles) of the reach matrix (excluding «NoData» values). The number of pixels also corresponds to the number of data lines in the file.

Ligne: Pixel (tile) row number (0 to X). The identifier of the 1st row is 0.

Colonne: Pixel (tile) column number (0 to X). The identifier of the 1st column is 0.

IdTroncon: Identifier of the reach corresponding to the pixel (tile).

<a id="priestlay-taylor-csv"></a>
## priestlay_taylor.csv   (/simulation/\{nom_simulation\})

Parameter file for the PRIESTLAY-TAYLOR calculation sub-model (potential evapotranspiration).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; PRIESTLAY-TAYLOR» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{CoefPropAlpha\} \{CoefOptimisation\} |

IdUhrh: RHHU identifier.

CoefPropAlpha : Alpha proportionality coefficient.

CoefOptimisation: Multiplicative optimization coefficient.

<a id="pro-rac-def"></a>
## pro_rac.def   (/physio)

Rooting-depth data file.

| Line | Description |
| --- | --- |
| Line 1 | \{Format\} |
| Line 2 | \{NbOccupation\} \{NbJour\} |
| Line 3 | Comment |
| Line 4 | Comment |
| Line 5 and following | \{Jour\} \{ValeurOccSol1\} \{ValeurOccSol2\} \{ValeurOccSol3\} etc… |

Format: Set to «2» (unused).

NbOccupation: Number of land-cover classes.

NbJour: Number of days (lines with values) specified in the file.

Jour: Start day of application of the parameters (Julian day).

ValeurOccSol1 to ValeurOccsolX: Rooting-depth values (m) for the current day for each land-cover class. Values are separated by a space or a tab.



Ex:

2

10 3

Profondeur racinaire (m)

Jour "FORETS CONIFERES" "FORETS FEUILLUS" "FORETS MIXTES" "AGRICULTURE"

(continuation of line 4)  "URBAIN" "ROUTES" "MILIEUX OUVERTS" "EAU" "SOLS NUS"

(continuation of line 4)  "MILIEUX HUMIDES"

1		1	1.5	1.25	0	0	0	0.5	0	0	0.75

210	1	1.5	1.25	0.8	0	0	0.5	0	0	0.75

365	1	1.5	1.25	0	0	0	0.5	0	0	0.75



Land-cover class names must be identical to those in the class identification file ([occupation_sol.csv](#occupation-sol-csv)).

Values are linearly interpolated between the dates provided in the files.

File names must be:   pro_rac.< year (yyyy)>  (e.g. pro_rac.1995). There must be one file for each year for which values are specified. The «.def» file extension ([pro_rac.def](#pro-rac-def)) may also be used to specify default values to be used for all years (or years that are not specified).

Files must end with day 365 (last line of the file).

<a id="proprietehydrolique-sol"></a>
## proprietehydrolique.sol   (/physitel)

Contains the hydraulic properties of the soil types.

| Line | Description |
| --- | --- |
| Line 1 | \{Type\} |
| Line 2 | \{NbTypeSol\} \{NbVariable\} |
| Line 3 | Comment |
| Line 4 | Comment |
| Line 5 and following | \{NomTypeSol\} \{thetas\} \{thetacc\} \{thetapf\} \{ks\} \{psis\} \{lambda\} \{alpha\} |

Type: Set to «3» (unused).

NbTypeSol: Number of soil types. Corresponds to the number of data lines in the file.

NbVariable: Number of variables (set to 7).

NomTypeSol: Soil-type name.

thetas: Water content at saturation (m3m-3).

thetacc: Water content at field capacity (m3m-3).

thetapf: Water content at wilting point (m3m-3).

ks: Saturated hydraulic conductivity (m/h).

psis: Matric potential at saturation (m).

lambda: Pore-size distribution index.

alpha: Exponent for evaluating the drying coefficient (fitting parameter of the kat and kas variation curve as a function of relative water content with respect to available water).



Ex:

3

2 7

Soil hydraulic properties classified by soil texture

texture thetas thetacc thetapf ks psis lambda alpha

sand 0.417000 0.091000 0.033000 0.210000 0.159800 0.694000 10.000000

loamy_sand 0.401000 0.125000 0.055000 0.061100 0.205800 0.553000 6.000000

<a id="rankinen-csv"></a>
## rankinen.csv   (/simulation/\{nom_simulation\})

Parameter file for the RANKINEN calculation sub-model (soil temperature).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; RANKINEN» |
| Line 4 | Blank line |
| Line 5 | «OUTPUT_TEMPERATURE_LIST_UHRH;» \{ListUhrhOutput\} |
| Line 6 | Blank line |
| Line 7 | «INTERVALLE PROFIL (m);» \{IntervalleProfil\} |
| Line 8 | «TEMP INI BASE PROFIL (C);» \{TempIniBase\} |
| Line 9 | «SEUIL GEL (C);» \{SeuilGel\} |
| Line 10 | «FS;» \{Fs\} |
| Line 11 | Blank line |
| Line 12 | Comment |
| Line 13 and following | \{NomTypeSol\} \{Conductivite\} \{CapaciteSol\} \{CapaciteGel\} |

ListUhrhOutput: List of identifiers of the RHHUs for which detailed layer temperatures are requested as output. A result file is generated for each RHHU (tempsol_uhrh \{IdUhrh\}.csv). Identifiers must be separated by a semicolon «;».

IntervalleProfil: Profile interval (m).

TempIniBase: Initial temperature at the bottom of the profile (°C).

SeuilGel: Freezing threshold (°C).

Fs: FS parameter.

NomTypeSol: Soil-type name. Names must be identical to those in the hydraulic properties file ([proprietehydrolique.sol](#proprietehydrolique-sol)). The order in which the soil types are listed must also be respected.

Conductivite: Thermal conductivity of frozen soil (KT) (W/m/C).

CapaciteSol: Specific heat capacity of the soil (CS) (J/m3/C).

CapaciteGel: Specific heat capacity related to freeze/thaw (CIce) (J/m3/C).

<a id="rayonnement-net-csv"></a>
## rayonnement_net.csv   (/simulation/\{nom_simulation\})

Parameter file for the RAYONNEMENT NET calculation sub-model (net radiation at the surface).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; RAYONNEMENT NET» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{Albedo\} \{TransAtmoA\} \{TransAtmoB\} \{TransAtmoC\} \{EmissAtmoA\} \{EmissAtmoB\} \{EmissAtmoC\} \{EmissSurfA\} \{EmissSurfB\} |

IdUhrh: RHHU identifier.

Albedo : Albedo (reference surface).

TransAtmoA : Atmospheric transmissivity (Coefficient A).

TransAtmoB : Atmospheric transmissivity (Coefficient B).

TransAtmoC : Atmospheric transmissivity (Coefficient C).

EmissAtmoA: Atmospheric emissivity (Coefficient A).

EmissAtmoB: Atmospheric emissivity (Coefficient B).

EmissAtmoC: Atmospheric emissivity (Coefficient C).

EmissSurfA: Surface emissivity (Coefficient A).

EmissSurfB: Surface emissivity (Coefficient B).

<a id="rivieres-shp-rivieres-prj-rivieres-dbf-rivieres-shx"></a>
## rivieres.shp, rivieres.prj, rivieres.dbf, rivieres.shx   (/physitel)

Vector map (shapefile) of rivers.

<a id="shreve-csv"></a>
## shreve.csv   (/physio)

File containing the Shreve order numbers determined for each reach. This file is generated automatically by HYDROTEL when it is absent.

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{IdTroncon\} \{OrdreShreve\} |

IdTroncon: Reach identifier.

OrdreShreve: Shreve order number.

<a id="station-p3s"></a>
## station.p3s   (/meteo and /neige)

Contains the weights of stations (meteorological or snow) at RHHUs for the «Moyenne 3 Stations» model. Weights are computed by HYDROTEL and this file is generated automatically when it does not exist.

<a id="station-pth"></a>

## station.pth   (/meteo and /neige)

Contains the weights of stations (meteorological or snow) at RHHUs for the «Thiessen» model. Weights are computed by HYDROTEL and this file is generated automatically when it does not exist.

<a id="station-sth"></a>

## station.sth   (/hydro)

Hydrometric stations.

| Line | Description |
| --- | --- |
| Line 1 | \{Coordinate type\} |
| Line 2 | \{Nb station\} |
| Line 3 | Comment |
| Line 4 and following | \{ID Station\} \{CoordX\} \{CoordY\} \{Altitude\} \{Data format\} \{@X\} \{Data folder name\} |

Coordinate type: 1=Long/Lat WGS84, 2=Same coordinate system as the project (according to the elevation map «[altitude.tif](#altitude-tif)»).

Nb station: Number of stations.

Comment: Comment line.

ID Station: Station identifier.

CoordX: Station X coordinate/longitude.

CoordY: Station Y coordinate/latitude.

Altitude: Station altitude (m).

Data format: This value must be set to «3».

@X: X must be replaced by the number of characters in the data-folder name.

Data folder name: Name of the folder containing the data files (.hyd).



Ex:

2

3

Hydrological stations

02MC036 531221 5018237 70.0 3 @5 hydro

02MC037 531230 5018215 65.0 3 @5 hydro

02MC038 531250 5018200 80.0 3 @5 hydro



Supported formats for long/lat coordinates (wgs84):

ddd.d				//decimal degree

ddmm.m (or ddmm)		//degree, decimal minute

dddmm.m (or dddmm)	//degree, decimal minute

ddmmss.s (or ddmmss)	//degree, minute, decimal second

dddmmss.s (or dddmmss)	//degree, minute, decimal second

Decimals are optional, except for the `decimal degree` format where the decimal point is mandatory.

For longitude, the negative sign is applied by default so as to be located in the western hemisphere (for compatibility). To use a longitude in the eastern hemisphere, the + sign must be specified.

<a id="station-stm"></a>
## station.stm   (/meteo)

Meteorological stations.

| Line | Description |
| --- | --- |
| Line 1 | \{Coordinate type\} |
| Line 2 | \{Nb station\} |
| Line 3 | Comment |
| Line 4 and following | \{ID Station\} \{CoordX\} \{CoordY\} \{Altitude\} \{Data format\} \{@X\} \{Data folder name\} |

Coordinate type: 1=Long/Lat WGS84, 2=Same coordinate system as the project (according to the elevation map).

Nb station: Number of stations.

Comment: Comment line.

ID Station: Station identifier.

CoordX: Station X coordinate/longitude.

CoordY: Station Y coordinate/latitude.

Altitude: Station altitude (m).

Data format: This value must be set to «3».

@X: X must be replaced by the number of characters in the data-folder name.

Data folder name: Name of the folder containing the data files (.met).



Ex:

1

3

Weather stations

0000040 -74.26 45.11 50 3 @5 meteo

0000041 -74.20 45.18 30 3 @5 meteo

0000042 -74.13 45.25 80.5 3 @5 meteo



Supported formats for long/lat coordinates (wgs84):

ddd.d				//decimal degree

ddmm.m (or ddmm)		//degree, decimal minute

dddmm.m (or dddmm)	//degree, decimal minute

ddmmss.s (or ddmmss)	//degree, minute, decimal second

dddmmss.s (or dddmmss)	//degree, minute, decimal second

Decimals are optional, except for the `decimal degree` format where the decimal point is mandatory.

For longitude, the negative sign is applied by default so as to be located in the western hemisphere (for compatibility). To use a longitude in the eastern hemisphere, the + sign must be specified.

<a id="station-stn"></a>
## station.stn   (/neige)

Snow stations.

| Line | Description |
| --- | --- |
| Line 1 | \{Coordinate type\} |
| Line 2 | \{Nb station\} |
| Line 3 | Comment |
| Line 4 and following | \{ID Station\} \{CoordX\} \{CoordY\} \{Altitude\} \{Data format\} \{@X\} \{Data folder name\} |

Coordinate type: 1=Long/Lat WGS84, 2=Same coordinate system as the project (according to the elevation map).

Nb station: Number of stations.

Comment: Comment line.

ID Station: Station identifier.

CoordX: Station X coordinate/longitude.

CoordY: Station Y coordinate/latitude.

Altitude: Station altitude (m).

Data format: This value must be set to «4».

@X: X must be replaced by the number of characters in the data-folder name.

Data folder name: Name of the folder containing the data files (.nei).



Ex:

1

3

Snow stations

NEIGE01 -74.26 45.11 50 4 @5 neige

NEIGE02 -74.20 45.18 30 4 @5 neige

NEIGE03 -74.13 45.25 80.5 4 @5 neige



Supported formats for long/lat coordinates (wgs84):

ddd.d				//decimal degree

ddmm.m (or ddmm)		//degree, decimal minute

dddmm.m (or dddmm)	//degree, decimal minute

ddmmss.s (or ddmmss)	//degree, minute, decimal second

dddmmss.s (or dddmmss)	//degree, minute, decimal second

Decimals are optional, except for the `decimal degree` format where the decimal point is mandatory.

For longitude, the negative sign is applied by default so as to be located in the western hemisphere (for compatibility). To use a longitude in the eastern hemisphere, the + sign must be specified.

<a id="station-troncon-p3s"></a>

## station-troncon.p3s   (/meteo)

Contains the weights of meteorological stations at reaches for the «Moyenne 3 Stations» model. Weights are computed by HYDROTEL and this file is generated automatically when it does not exist. This file is used by the water-temperature calculation model.

<a id="station-troncon-pth"></a>

## station-troncon.pth   (/meteo)

Contains the weights of meteorological stations at reaches for the «Thiessen» model. Weights are computed by HYDROTEL and this file is generated automatically when it does not exist. This file is used by the water-temperature calculation model.

<a id="stats-csv"></a>

## stats.csv   (/simulation/\{nom_simulation\}/resultat)

Result file containing the statistics computed at the end of the simulation. The statistical indicators computed for the simulation period are followed by the observed and simulated discharges for each selected reach and each simulation time step. Selection of reaches and associated hydrological stations is done with [stats.txt](#stats-txt).

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 to (NbTroncon+1) (one line for each reach) | \{IdTroncon\} \{RMSE\} \{Nash-Sutcliffe\} \{Relative bias\} \{Absolute bias\} \{Correlation coefficient\} \{Kge original 2009\} \{Kge modified 2012\} \{Peak coefficient\} \{Volume coefficient\} \{Nash-Log\} \{Nash-M\} \{Root mean square error\} \{Observed sum\} \{Simulated sum\} \{Observed mean\} \{Simulated mean\} |
| Line (NbTroncon+2) | Blank line |
| Line (NbTroncon+3) | Comment (identifiers of the reaches and corresponding hydrological stations) |
| Line (NbTroncon+4) and following (one line for each simulation time step) | \{Date/Time\} \{Simulated discharge of the 1st reach\} \{Observed discharge of the 1st reach\} \{Simulated discharge of the 2nd reach\} \{Observed discharge of the 2nd reach\} \{etc…\} |

<a id="stats-txt"></a>
## stats.txt   (/simulation/\{nom_simulation\})

Information on the association of reaches/hydrological stations for statistics calculation at the end of the simulation. Statistics are computed only when this file is present.

| Line                | Description                      |
| --- | --- |
| Line 1 and following | \{IdTroncon\} \{IdStationHydro\} |

IdTroncon: Reach identifier.

IdStationHydro: Identifier of the hydrological station associated with the reach. The keyword «absent» may be used when the reach is not associated with any station.



Ex:

1 absent

91 02MC036

<a id="strahler-csv"></a>
## strahler.csv   (/physio)

File containing the Strahler order numbers determined for each reach. This file is generated automatically by HYDROTEL when it is absent.

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{IdTroncon\} \{OrdreStrahler\} |

IdTroncon: Reach identifier.

OrdreStrahler: Strahler order number.

<a id="submodels-version-txt"></a>
## submodels-versions.txt   (/simulation/\{nom_simulation\})

Indicates the version numbers to use for the sub-models. When the version number for a sub-model is not specified in the file or the file itself does not exist, the most recent versions of the sub-models are used automatically, except if the version number written in the simulation file is equal to or earlier than 4.1.5. In that case, version 1 of the sub-models is used. This file is created automatically by HYDROTEL when it does not exist.

| Line         | Description                                                |
| --- | --- |
| THIESSEN;2 | Version to use for the THIESSEN sub-model. |
| MOY3STATION;2 | Version to use for the MOYENNE 3 STATIONS sub-model. |
| BV3C;2 | Version to use for the BV3C sub-model. |

<a id="temp-eau-cequeau-troncons-csv"></a>
## temp_eau_cequeau_troncons.csv   (/simulation/\{nom_simulation\})

Parameter file for reaches for the TEMP EAU CEQUEAU calculation sub-model (water temperature).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.4.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE;TEMP EAU CEQUEAU» |
| Line 4 | Blank line |
| Line 5 | «Capacite thermique de l eau;» \{CapaciteThermiqueEau\} |
| Line 6 | «Albedo de la surface de l eau;» \{AlbedoEau\} |
| Line 7 | «Emissivite de l eau;» \{EmissiviteEau\} |
| Line 8 | «Chaleur latente de vaporisation de l eau;» \{ChaleurLatente\} |
| Line 9 | Blank line |
| Line 10 | Comment |
| Line 11 and following | \{IdTroncon\} \{CoefSolaire\} \{CoefInfraRouge\} \{CoefEvapo\} \{CoefChaleur\} \{VitesseVent\} \{CoefATransmissiviteAtmo\} \{CoefBTransmissiviteAtmo\} \{CoefCTransmissiviteAtmo\} \{CoefAEmissiviteAtmo\} \{CoefBEmissiviteAtmo\} \{CoefCEmissiviteAtmo\} \{MoyenneAnnuelleTempAir\} |

CapaciteThermiqueEau: Specific heat capacity of water (C) (4.187 MJ / m³ / °C).

AlbedoEau: Albedo (*a*) of the water surface of the water body (reach or lake) (0-1) (default value 0.06).

EmissiviteEau: Emissivity of the water surface (*a*) (default value 0.95).

ChaleurLatente: Latent heat of vaporization of water (*H*) (2480 MJ / m³).

IdTroncon: Reach identifier.

CoefSolaire: Calibration coefficient related to net shortwave (solar) radiation (Cs).

CoefInfraRouge: Calibration coefficient related to net longwave (infrared) radiation (Ci).

CoefEvapo: Calibration coefficient related to water evaporation (Ce).

CoefChaleur: Calibration coefficient related to the sensible heat flux (Cc).

VitesseVent: Wind speed (km/h) (U).

CoefATransmissiviteAtmo, CoefBTransmissiviteAtmo, CoefCTransmissiviteAtmo: Calibration coefficients in the calculation of atmospheric transmissivity of the HYDROTEL radiation model (see HYDROTEL theory documents).

CoefAEmissiviteAtmo, CoefBEmissiviteAtmo, CoefCEmissiviteAtmo: Calibration coefficients in the calculation of atmospheric pseudo-emissivity of the HYDROTEL radiation model (see HYDROTEL theory documents).

MoyenneAnnuelleTempAir: Mean watershed temperature (°C) (default value 4).

<a id="temp-eau-cequeau-zones-csv"></a>

## temp_eau_cequeau_zones.csv   (/simulation/\{nom_simulation\})

Parameter file for RHHUs for the TEMP EAU CEQUEAU calculation sub-model (water temperature).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.4.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE;TEMP EAU CEQUEAU» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{TemperatureBase\} \{CritereGel\} |

IdUhrh: RHHU identifier.

TemperatureBase: Mean watershed temperature (°C) (default value 4).

CritereGel: Freeze criterion.

<a id="thiessen-csv"></a>

## thiessen.csv   (/simulation/\{nom_simulation\})

Parameter file for the THIESSEN calculation sub-model (interpolation of meteorological data).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; THIESSEN» |
| Line 4 | Blank line |
| Line 5 | «GRADIENT TEMPERATURE STATION(C/100m);»\{GradientStationTemp\} |
| Line 6 | «GRADIENT PRECIPITATION STATION(mm/100m);»\{GradientStationPrecip\} |
| Line 7 | Blank line |
| Line 8 | Comment |
| Line 9 and following | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

GradientStationTemp: Vertical temperature gradient for interpolating missing data at stations (°C/100m).

GradientStationPrecip: Vertical precipitation gradient for interpolating missing data at stations (°C/100m).

IdUhrh: RHHU identifier.

GradientTemp: Vertical temperature gradient (°C/100m).

GradientPrecip: Vertical precipitation gradient (mm/100m).

PassagePluieNeige: Rain-to-snow transition temperature (°C).

<a id="thornthwaite-csv"></a>
## thornthwaite.csv   (/simulation/\{nom_simulation\})

Parameter file for the THORNTHWAITE calculation sub-model (potential evapotranspiration).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; THORNTHWAITE» |
| Line 4 | Blank line |
| Line 5 | Comment |
| Line 6 and following | \{IdUhrh\} \{IndiceThermique\} \{FacteurDephasage\} \{CoefOptimisation\} |

IdUhrh: RHHU identifier.

IndiceThermique: Thornthwaite annual heat index (1-100).

FacteurDephasage: Phase-shift (lag) factor (days) (1-80).

CoefOptimisation: Multiplicative optimization coefficient.

<a id="thorsen-csv"></a>
## thorsen.csv   (/simulation/\{nom_simulation\})

Parameter file for the THORSEN calculation sub-model (soil temperature).

| Line | Description |
| --- | --- |
| Line 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Line 2 | Blank line |
| Line 3 | «SOUS MODELE; THORSEN» |
| Line 4 | Blank line |
| Line 5 | «PROFONDEUR INITIALE DU GEL DANS LE SOL (m);» \{ProfondeurGelIni\} |
| Line 6 | «PARAMETRE EMPIRIQUE 1 (m-1);» \{ParamEmpirique\} |
| Line 7 | «TEMPERATURE DU GEL DE L'EAU DANS LE SOL (dC);» \{TempGelEau\} |
| Line 8 | «TENEUR EN EAU DISPONIBLE (INITIAL/PAR DEFAUT) (0:1);» \{TeneurEauDispo\} |
| Line 9 | Blank line |
| Line 10 | Comment |
| Line 11 and following | \{NomTypeSol\} \{Conductivite\} |

ProfondeurGelIni: Initial frost depth in the soil (m).

ParamEmpirique: Empirical parameter 1 (m-1).

TempGelEau : Freezing temperature of water in the soil (°C).

TeneurEauDispo: Available water content (initial/default) (0-1).

NomTypeSol: Soil-type name. Names must be identical to those in the hydraulic properties file ([proprietehydrolique.sol](#proprietehydrolique-sol)). The order in which the soil types are listed must also be respected.

Conductivite: Thermal conductivity of frozen soil (W/m/s).

<a id="troncon-trl"></a>
## troncon.trl   (/physitel)

Parameters for the reaches.

| Line | Description |
| --- | --- |
| Line 1 | \{Type\} |
| Line 2 | \{NbTroncon\} |
| Line 3 | Comment |
| Line 4 and following | If TypeTroncon is equal to 1 (river) :<br>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{IdNoeudAmont\} \{Longueur\} \{Largeur\} \{Manning\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\}<br/><br/>If TypeTroncon is equal to 2 (lake) :<br/>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{NbNoeudAmont\} \{IdNoeudAmont1\} \{IdNoeudAmont2\} \{etc…\} \{Longueur\} \{Superficie\} \{CoefC\} \{CoefK\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\}<br/><br/>If TypeTroncon is equal to 4 (lake without routing) :<br/>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{NbNoeudAmont\} \{IdNoeudAmont1\} \{IdNoeudAmont2\} \{etc…\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\}<br/><br/>If TypeTroncon is equal to 5 (dam with historical record):<br/>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{NbNoeudAmont\} \{IdNoeudAmont1\} \{IdNoeudAmont2\} \{etc…\} \{IdStationHydro\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\} |

Type: Indicates whether the last column of the data lines represents the Shreve order number (1: Shreve order number absent, 2: Shreve order number present).

NbTroncon: Number of reaches.

IdTroncon : Reach identifier.

TypeTroncon : Reach type (1: River, 2: Lake, 4: Lake without routing, 5: Dam with historical record).

IdNoeudAval: Identifier of the downstream node (point) of the reach.

IdNoeudAmont: Identifier of the upstream node (point) of the reach.

Longueur: Length of the lake or river reach (m).

Largeur: Width of the river reach (m).

Manning: Manning coefficient for the river reach. Default value: 0.04.

NbUhrhAmont: Number of RHHUs discharging into the reach.

IdUhrhAmont1 to IdUhrhAmontX: Identifiers of the RHHUs discharging into the reach.

NoOrdreShreve: Shreve order number for the reach.

NbNoeudAmont: Number of upstream nodes (points) of the reach.

IdNoeudAmont1 to IdNoeudAmontX: Identifiers of the upstream nodes (points) of the reach.

Superficie: Lake area (km2).

CoefC: Multiplicative factor (Coefficient C).

CoefK: Exponent (K).

IdStationHydro: Identifier of the hydrological station associated with the reach.



Ex:

2

4

TRONCONS

1 1 1 2 2027.7 19.9 0.04 2 1 2 75

2 1 2 3 20.0 19.85 0.04 1 3 74

3 1 3 4 241.42 0.22 0.04 3 4 5 6 1

4 2 151 2 152 153 72768.13 0.3088 6.4521 1.5 4 356 357 358 359 7

<a id="troncon-width-depth-csv"></a>
## troncon_width_depth.csv   (/physio)

File containing reach parameters used during simulation of riparian wetlands. This file is generated automatically by PHYSITEL when the HYDROTEL project is created.

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{ID\} \{Superficie\} \{Width PHYSITEL\} \{Width SWAT\} \{Depth SWAT\} |

ID: Reach identifier.

Superficie: Upstream area drained by the reach (km2) (unused).

Width PHYSITEL: Reach width computed according to the PHYSITEL method (m) (unused).

Width SWAT: Reach width computed according to the SWAT method (m) (unused).

Depth SWAT: Reach depth computed according to the SWAT method (m).

<a id="troncons-tif"></a>
## troncons.tif   (/physitel)

Raster map (GeoTIFF) of reaches. This map is not used by HYDROTEL and is present for information only.

<a id="troncons-txt"></a>
## troncons.txt   (/physitel)

Information file for reaches (rasterized hydrographic network). This file is generated by PHYSITEL when the HYDROTEL project is created. The file is not used by HYDROTEL and is present for information only.

| Line | Description |
| --- | --- |
| Line 1 | \{NbTroncon\} |
| Line 2 and following (for each reach) | Blank line |
| (line 3) | \{Type\} |
| (line 4) | \{NoeudAvalY\} \{NoeudAvalX\} |
| (line 5) | \{NbNoeudAmont\} |
| (line 6) | \{NoeudAmontY\} \{NoeudAmontX\} (first node) |
| (line 7) | \{NoeudAmontY\} \{NoeudAmontX\} (second node) |
|  | (coordinates of the other upstream nodes) (one line per node) |
| (line 8) | \{NbCell\} |
| (line 9) | \{CellY\} \{CellX\} (first cell) |
| (line 10) | \{CellY\} \{CellX\} (second cell) |
|  | (coordinates of the other cells) (one line per cell) |
| (line 11) | Blank line |
| (line 12) | … |

NbTroncon: Number of reaches.

Type: Reach type (1: River, 2: Lake).

NoeudAvalY: Y coordinate of the downstream node of the reach. The Y coordinate is the row number (from 0 to x) of the cell of the reach raster map ([troncons.tif](#troncons-tif)).

NoeudAvalX: X coordinate of the downstream node of the reach. The X coordinate is the column number (from 0 to x) of the cell of the reach raster map ([troncons.tif](#troncons-tif)).

NbNoeudAmont: Number of upstream nodes (junction points).

NoeudAmontY : Y coordinate of the upstream node of the reach. The Y coordinate is the row number (from 0 to x) of the cell of the reach raster map ([troncons.tif](#troncons-tif)).

NoeudAmontX : X coordinate of the upstream node of the reach. The X coordinate is the column number (from 0 to x) of the cell of the reach raster map ([troncons.tif](#troncons-tif)).

NbCell: Number of cells (tiles) belonging to the reach (according to the reach raster map ([troncons.tif](#troncons-tif)).

CellY: Y coordinate of the cell (tile) belonging to the reach. The Y coordinate is the row number (from 0 to x) of the reach raster map ([troncons.tif](#troncons-tif)).

CellX: X coordinate of the cell (tile) belonging to the reach. The X coordinate is the column number (from 0 to x) of the reach raster map ([troncons.tif](#troncons-tif)).

<a id="type-sol-cla"></a>
## type_sol.cla   (/physitel)

Spatial distribution of soil types (soil type assigned to each RHHU).

| Line | Description |
| --- | --- |
| Line 1 | \{Format\} |
| Line 2 | \{IdUhrh\} \{IdTypeSol\} |

Format : Set to «1» (unused).

IdUhrh: RHHU identifier.

IdTypeSol: Identifier of the soil type assigned to the RHHU. The soil-type identifier must correspond to the order of the soil types in [proprietehydrolique.sol](#proprietehydrolique-sol) starting at identifier 0. Ex: the first soil type in [proprietehydrolique.sol](#proprietehydrolique-sol) is represented by the value «0» in [type_sol.cla](#type-sol-cla), the second soil type in [proprietehydrolique.sol](#proprietehydrolique-sol) is represented by the value «1» in [type_sol.cla](#type-sol-cla), and so on.



This file is created automatically by HYDROTEL when it is absent from the data of the [type_sol.tif](#type-sol-tif) map. The retained soil type is the predominant soil type on the RHHU.

<a id="type-sol-tif"></a>
## type_sol.tif   (/physitel)

Raster map (GeoTIFF) of soil types. Map values must correspond to the numbers (indices) of the soil types defined in [proprietehydrolique.sol](#proprietehydrolique-sol) starting at value «1». Ex: the value 1 on the map represents the first soil type defined in [proprietehydrolique.sol](#proprietehydrolique-sol), the value 2 on the map represents the second soil type defined in [proprietehydrolique.sol](#proprietehydrolique-sol), and so on.

<a id="uhrh-csv"></a>
## uhrh.csv   (/physitel)

RHHU properties.

| Line | Description |
| --- | --- |
| Line 1 | "RESUMER ZONES HYDROTEL VERSION;4.3.0.0000" |
| Line 2 | Blank line |
| Line 3 | Comment |
| Line 4 and following | \{IdUhrh\} \{Type\} \{AltitudeMoyenne\} \{PenteMoyenne\} \{OrientationMoyenne\} \{NbPixel\} \{Superficie\} \{CentroidLon\} \{CentroidLat\} |

IdUhrh: RHHU identifier.

Type : Type of the RHHU. Indicates whether an RHHU is a lake (value «LAC») or a sub-basin (value «SOUS-BASSIN».

AltitudeMoyenne: Mean altitude of the RHHU (m).

PenteMoyenne: Mean slope of the RHHU (ratio).

OrientationMoyenne: Mean orientation of the RHHU. Possible value: 1 (East), 2 (Northeast), 3 (North), 4 (Northwest), 5 (West), 6 (Southwest), 7 (South), 8 (Southeast).

NbPixel: Number of pixels (tiles) belonging to the RHHU.

Superficie: Area of the RHHU (km2).

CentroidLon: Longitude coordinate of the RHHU centroid (WGS84) (dd).

CentroidLat: Latitude coordinate of the RHHU centroid (WGS84) (dd).



This file is created automatically by HYDROTEL when it is absent from the data of the maps [uhrh.tif](#uhrh-tif), [altitude.tif](#altitude-tif), [pente.tif](#pente-tif) and [orientation.tif](#orientation-tif).

<a id="uhrh-shp-uhrh-prj-uhrh-dbf-uhrh-shx"></a>
## uhrh.shp, uhrh.prj, uhrh.dbf, uhrh.shx   (/physitel)

Vector map (shapefile) of RHHUs. This map is used only by the HYDROTEL interface version.

<a id="uhrh-tif"></a>
## uhrh.tif   (/physitel)

Raster map (GeoTIFF) of RHHUs.

<a id="uhrh-txt"></a>
## uhrh.txt   (/physitel)

Information file for RHHUs (Relatively Homogeneous Hydrological Units). This file is generated by PHYSITEL when the HYDROTEL project is created. The file is not used by HYDROTEL and is present for information only.

<a id="wet-pixel-info-csv"></a>
## wet_pixel_info.csv   (/physio)

File containing wetland characteristics for RHHUs. This file is generated automatically by PHYSITEL when the HYDROTEL project is created. These data are for information only (not used by HYDROTEL).

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{UhrhId\} \{NbPixel\} \{NbPixelIsole\} \{NbPixelRiverain\} \{NbPixelDraineIsole\} \{NbPixelDraineRiverain\} \{NbPixelDraineCommun\} \{NbPixelIsoleDansRiverain\} |

UhrhId : RHHU identifier.

NbPixel: Total number of pixels (tiles) of the RHHU (based on the raster map [uhrh.tif](#uhrh-tif)).

NbPixelIsole: Number of isolated-wetland pixels contained in the RHHU.

NbPixelRiverain: Number of riparian-wetland pixels contained in the RHHU.

NbPixelDraineIsole : Number of pixels drained only by isolated wetlands contained in the RHHU.

NbPixelDraineRiverain : Number of pixels drained only by riparian wetlands contained in the RHHU.

NbPixelDraineCommun : Number of pixels drained by isolated wetlands that are drained by riparian wetlands contained in the RHHU.

NbPixelIsoleDansRiverain : Number of isolated-wetland pixels that are drained by riparian wetlands contained in the RHHU.

<a id="wetland-isole-csv"></a>
## wetland_isole.csv   (/simulation/\{nom_simulation\}/resultat)

Result file for isolated wetlands (state variables). Saving of these data and selection of reaches is done in [milieux_humides_isoles.csv](#milieux-humides-isoles-csv).

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{IdUhrh\} \{Annee\} \{Mois\} \{Jour\} \{Heure\} \{Apport\} \{Evp\} \{WetSep\} \{WetVol\} \{WetFlwI\} \{WetFlwO\} \{WetProd\} |

IdUhrh : RHHU identifier.

Annee: Simulation time step (year).

Mois: Simulation time step (month).

Jour: Simulation time step (day).

Heure: Simulation time step (hour).

Apport: Vertical contributions (rain + snowmelt) on the wetland (mm).

Evp: Potential evapotranspiration (mm).

WetSep: Volume of water flowing at the base of the wetland (m3).

WetVol: Volume of water in the wetland (m3).

WetFlwI: Volume of water intercepted by the wetland as a function of the area drained by it (m3).

WetFlwO : Volume of water leaving the wetland at the surface (m3).

WetProd: Total production of the wetland (mm).

<a id="wetland-riverain-csv"></a>
## wetland_riverain.csv   (/simulation/\{nom_simulation\}/resultat)

Result file for riparian wetlands (state variables). Saving of these data and selection of reaches is done in [milieux_humides_riverains.csv](#milieux-humides-riverains-csv).

| Line | Description |
| --- | --- |
| Line 1 | Comment |
| Line 2 and following | \{IdTroncon\} \{Annee\} \{Mois\} \{Jour\} \{Heure\} \{wet_v\} \{wet_a\} \{wet_d\} \{sur_q\} \{HauteurEau\} \{qd\} |

IdTroncon : Reach identifier.

Annee: Simulation time step (year).

Mois: Simulation time step (month).

Jour: Simulation time step (day).

Heure: Simulation time step (hour).

wet_v: Volume of water in the wetland (m3).

wet_a: Wetland area (m2).

wet_d: Water depth in the wetland (m).

sur_q: Discharge of water directed toward (+) or withdrawn from (−) the neighbouring river reach (m3/s).

HauteurEau: Water depth in the reach (m).

qd : Downstream discharge of the reach (m3/s).



