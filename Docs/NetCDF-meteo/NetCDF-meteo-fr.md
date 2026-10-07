# Utilisation de fichiers de données météorologiques de format NetCDF

Le présent document décrit les étapes nécessaires à l’utilisation d’un fichier de données météorologiques de format NetCDF. Pour des contraintes de rapidité d’exécution, le fichier de données NetCDF ne doit pas contenir de données manquantes et le pas de temps des données disponible (intervalle de temps entre chaque mesure) doit être le même que celui de la simulation que l’on souhaite effectuer. Le format du fichier doit également respecter l’un ou l’autre des deux formats supportés par HYDROTEL, soit les formats « STATION » ou « GRID » qui sont décrits à la section 2.



## 1. Sélectionner le fichier de données NetCDF pour la lecture des données météorologique

Pour sélectionner le fichier NetCDF manuellement, sans utiliser l’interface, ouvrir le fichier de configuration de la simulation avec un éditeur texte. Le fichier de configuration d’une simulation se trouve dans le dossier de la simulation et porte le nom de cette même simulation avec l’extension « .csv » (ex : .\Dossier de projet Hydrotel\simulation\Nom de la simulation\Nom de la simulation.csv). Modifier la ligne portant l’étiquette « FICHIER STATIONS METEO » et inscrire le chemin pointant vers le fichier de données :

Ex : `FICHIER STATIONS METEO;meteo/netcdf_example_weather_9.3.1.nc`.



## 2. Configurer le fichier nc.config selon le format du fichier de données NetCDF.

Un fichier de configuration NetCDF (.nc.config) doit également être créé afin de décrire le format du fichier NetCDF qui sera utilisé. Le fichier de configuration peut être créé avec un éditeur texte. Le fichier doit se trouver dans le même dossier que le fichier de données « .nc » et porter le même nom, avec en plus l’extension « nc.config » (ex : netcdf_example_weather_9.3.1.nc.config).

Le fichier « nc.config » doit avoir le format suivant :

| Ligne | Étiquette | Valeur exemple |
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



- **Ligne 1 (TYPE (STATION/GRID))**

  Inscrire la valeur « STATION » pour utiliser le format de données stations (H2.1) contenant des séries de données temporelles par station. Inscrire la valeur « GRID » pour utiliser le format de données grille (9.3.1) contenant des séries de données temporelles pour chaque point d’une grille de coordonnées.

  

- **Ligne 2 (STATION_DIM_NAME)**

  Nom de la variable dimension du fichier NetCDF ayant une valeur égale au nombre de station. Cette valeur doit être spécifiée seulement lorsque le type choisi à la ligne 1 est « STATION ».

  Lorsque le type choisi est « GRID » la valeur est laissée en blanc. Le nombre de points de grille d’un fichier de type « GRID » est déterminé selon la taille du vecteur contenant les coordonnées pour la latitude et la taille du vecteur contenant les coordonnées pour la longitude (taille vecteur latitude X taille vecteur longitude = nombre de points de grille).

  

- **Ligne 3 (LATITUDE_NAME)**

  Nom de la variable du fichier NetCDF contenant les valeurs latitude des coordonnées (WGS 1984). Pour le type de fichier « STATION », ce vecteur doit avoir la même taille que le nombre de stations disponibles. Dans le cas du type de fichier « GRID », ce vecteur doit contenir les valeurs uniques des latitudes composant la grille, c’est-à-dire les valeurs possibles pour l’axe Y. De plus, le fichier doit contenir une variable dimension portant le même nom que la variable de données et avoir une valeur égale au nombre de valeur « latitude » contenue dans le vecteur.

  - Type STATION : `double latitude(NbStation)`

  - Type GRID : `double latitude(NbLatitude)`

    

- **Ligne 4 (LONGITUDE_NAME)**

  Nom de la variable du fichier NetCDF contenant les valeurs longitudes des coordonnées (WGS 1984). Pour le type de fichier « STATION », ce vecteur doit avoir la même taille que le vecteur contenant les valeurs latitude. Pour le type de fichier « GRID » les tailles des vecteurs latitude et longitude peuvent différer en outre dans le cas de l’utilisation d’une grille de données de forme rectangle. De plus, le fichier doit contenir une variable dimension portant le même nom que la variable de données et avoir une valeur égale au nombre de valeur « longitude » contenue dans le vecteur.

  - Type STATION : `double longitude(NbStation)`

  - Type GRID : `double longitude(NbLongitude)`

    

- **Ligne 5 (ELEVATION_NAME)**

  Nom de la variable du fichier NetCDF contenant les valeurs d’élévation pour chaque station ou point de grille (selon le type de fichier utilisé). Les valeurs d’élévation doivent être fournies en mètres pour chacune des stations. Dans le cas du type de fichier « GRID », les valeurs d’élévation doivent être fournies pour chaque valeur de longitude correspondant à la première valeur de latitude, pour ensuite continuer avec la latitude suivante, et ainsi de suite.

  - Type STATION : `double elevation(NbStation)`

  - Type GRID : `double elevation(NbLatitude, NbLongitude)`

    

- **Ligne 6 (TIME_NAME)**

  Nom de la variable du fichier NetCDF contenant les valeurs de temps pour chaque série temporelle. La variable doit avoir un attribut «units» avec une valeur de format «days since yyyy-mm-dd hh:00:00» ou «minutes since yyyy-mm-dd hh:00:00». Ex : «days since 1970-01-01 00:00:00». Dans cet exemple, les valeurs de temps doivent être spécifiées en nombre de jours écoulé depuis le 1<sup>er</sup> janvier 1970 à minuit (0h). La valeur peut être négative. Dans le cas de l’exemple précédent ce serais pour les dates antérieures au 1<sup>er</sup> janvier 1970. Les valeurs sont considéré par HYDROTEL comme étant dans le fuseau horaire local et aucun ajustement n’est effectué. Le fichier doit également contenir une variable de type «dimensions» portant le même nom de la variable de données et avoir une valeur égale au nombre de valeurs « temps » contenue dans le vecteur.

  - Type STATION et GRID : `double time(NbPasTemps)`

    

- **Ligne 7 (TMIN_NAME)**

  Nom de la variable du fichier NetCDF contenant les valeurs de température minimum en degré Celsius.

  - Type STATION : `double tmin(NbPasTemps, NbStation)`

  - Type GRID : `double tmin(NbPasTemps, NbLatitude, NbLongitude)`

    

- **Ligne 8 (TMAX_NAME)**

  Nom de la variable du fichier NetCDF contenant les valeurs de température maximum en degré Celsius.

  - Type STATION : `double tmax(NbPasTemps, NbStation)`

  - Type GRID : `double tmax(NbPasTemps, NbLatitude, NbLongitude)`

    

- **Ligne 9 (PRECIP_NAME)**

  Nom de la variable du fichier NetCDF contenant les valeurs de précipitation (pluie + équivalent en eau des précipitations de neige) en millimètres.

  - Type STATION : `double precip(NbPasTemps, NbStation)`
  - Type GRID : `double precip(NbPasTemps, NbLatitude, NbLongitude)`



**Exemple de fichier « nc.config » de type STATION :**

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



**Exemple de fichier « nc.config » de type GRID :**

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



## 3. Délimiter l’étendu (extent) des coordonnées considéré dans la simulation (optionnel).

Il est possible de spécifier les limites nord, sud, est et ouest des coordonnées prises en compte dans la simulation. Les stations ou les points de grille doivent se situer à l’intérieur de l’étendue définie afin d’être considérées dans la simulation. Toutes stations ou tous points de grille se situant à l’extérieur seront exclus du processus d’interpolation.

Pour activer cette fonctionnalité, un fichier texte portant le nom « extent-limit.config » doit être créé dans le même dossier que le fichier de données NetCDF et être édité selon le format suivant :

| Ligne | Étiquette | Valeur exemple (WGS 84) |
| --- | --- | --- |
| 1 | North; | 45.6 |
| 2 | South; | 45.3 |
| 3 | East; | -71.0 |
| 4 | West; | -71.4 |



**Exemple de fichier « extent-limit.config » :**

```
North; 45.6
South; 45.3
East; -71.0
West; -71.4
```
