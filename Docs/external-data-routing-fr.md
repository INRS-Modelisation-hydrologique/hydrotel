# Routage de données externes dans Hydrotel

Le routage de données externes permet à Hydrotel d’utiliser des **fichiers NetCDF** comme source de certaines variables hydrologiques internes (production des couches, apports latéraux ou apports verticaux), puis de poursuivre la simulation à partir de ce point.



---

## 1. Activer l’option

Dans le **fichier de simulation**, ajoutez une ligne qui pointe vers le fichier de configuration:

```text
EXTERNAL DATA ROUTING;external-data/config.txt
```

- Le chemin peut être absolu ou relatif au **répertoire du projet**.

- Retirez le chemin vers le fichier de configuration (ou omettez cette ligne) pour exécuter une simulation normale, sans routage de données externes.

  

---

## 2. Format du fichier de configuration

Le fichier de configuration est un fichier texte (`clé;valeur`).

| Règle | Description |
|------|-------------|
| Séparateur | Utilisez `;` entre le mot-clé et sa valeur |
| Commentaires | Les lignes commençant par `//` sont ignorées |
| Activation | Décommentez les lignes du mode souhaité ; commentez les autres avec `//` |



---

## 3. Choisir un seul mode de variables

Vous devez activer **exactement un** groupe de variables. Le mélange de groupes n’est pas permis.

| Mode | Mots-clés à activer (`VAR`) | Unités | Signification |
|------|--------------------|-------|---------|
| Apports verticaux | `VCONT` | mm | Apport d’eau provenant de la pluie et de la fonte de neige |
| Production par couche | `PROD1`, `PROD2`, `PROD3` | mm | Hauteur d’eau produite par la couche de sol (surface, hypodermique, base) |
| Production totale | `PRODTOT` | mm | Hauteur d’eau totale produite par les trois couches de sol |
| Apport latéral par couche | `QLAT1`, `QLAT2`, `QLAT3` | m³/s | Apport latéral (UHRH) pour la couche de sol (surface, hypodermique, base) |
| Apport latéral total | `QLATTOT` | m³/s | Apport latéral total (UHRH) |

Pour chaque variable activée `VAR`, définissez :

```text
VAR_SOURCE;chemin/vers/fichier.nc
VAR_VARNAME;nom_de_la_variable_dans_le_netcdf
```

Ex. :

```
PROD1_SOURCE;external-data/production_surf.nc
PROD1_VARNAME;production_surf
```

- `*_SOURCE` peut être un chemin absolu ou relatif au **répertoire du projet**.
- Plusieurs variables peuvent partager le même fichier NetCDF (avec des `*_VARNAME` différents).

### Ce qu’Hydrotel omet pour chaque mode

| Mode | Comportement |
|------|-----------|
| `VCONT` | La fonte de neige est omise ; Hydrotel utilise l’apport d’eau fourni et poursuit avec le bilan hydrique vertical, le ruissellement de surface et le routage dans le réseau hydrographique. L’interpolation des données météorologiques s’exécute tout de même, en ignorant les précipitations (les températures de l’air TMin/TMax sont nécessaires pour le modèle d’évapotranspiration) |
| `PROD1`/`PROD2`/`PROD3` ou `PRODTOT` | La fonte de neige et le bilan hydrique vertical sont omis ; Hydrotel utilise la production fournie et poursuit avec le ruissellement de surface et le routage dans le réseau hydrographique |
| `QLAT1`/`QLAT2`/`QLAT3` ou `QLATTOT` | La fonte de neige, le bilan hydrique vertical et le ruissellement de surface sont omis ; Hydrotel injecte les apports latéraux dans les tronçons et poursuit avec le routage dans le réseau hydrographique |

Lorsque le modèle de température de l’eau est activé, l’interpolation des données météorologiques s’exécute en ignorant les précipitations ; les températures de l’air (TMin/TMax) sont nécessaires pour le modèle de température de l’eau.



---

## 4. Choisir un type de dimensions (`DIMTYPE`)

Décommentez **un** bloc `DIMTYPE` ainsi que les mots-clés de dimensions / coordonnées associés.

### GRID

Données sur une grille régulière lon/lat (ou x/y) (format NetCDF 9.3.1. Orthogonal multidimensional array representation). Hydrotel interpole les valeurs de la grille vers les UHRH.

```text
DIMTYPE;GRID
LON_DIMNAME;x
LAT_DIMNAME;y
LON_VARNAME;x
LAT_VARNAME;y
```

Compatible avec : `VCONT`, `PROD1`/`PROD2`/`PROD3`, `PRODTOT`.

### STATION

Données aux stations avec des coordonnées lon/lat (format NetCDF H.2.1. Orthogonal multidimensional array representation of time series). Hydrotel interpole les valeurs des stations vers les UHRH.

```text
DIMTYPE;STATION
STATION_DIMNAME;nbstation
LON_VARNAME;lon
LAT_VARNAME;lat
```

Compatible avec : `VCONT`, `PROD1`/`PROD2`/`PROD3`, `PRODTOT`.

### RHHU

Une valeur par UHRH. Les valeurs sont affectées directement à chaque UHRH (sans interpolation spatiale).

```text
DIMTYPE;RHHU
ID_DIMNAME;uhrh
ID_VARNAME;iduhrh
```

Compatible avec : `VCONT`, `PROD1`/`PROD2`/`PROD3`, `PRODTOT`, `QLAT1`/`QLAT2`/`QLAT3`, `QLATTOT`.

Les identifiants d’UHRH dans le fichier NetCDF doivent correspondre aux UHRH du projet.

### REACH

Une valeur par tronçon de rivière. Les valeurs sont affectées directement à chaque tronçon (sans interpolation spatiale).

```text
DIMTYPE;REACH
ID_DIMNAME;reach
ID_VARNAME;idreach
```

Compatible avec : `QLAT1`/`QLAT2`/`QLAT3`, `QLATTOT`.

Les identifiants des tronçons dans le fichier NetCDF doivent correspondre aux tronçons du projet.



---

## 5. Dimension temporelle (toujours requise)

```text
TIME_DIMNAME;time
TIME_VARNAME;time
```

Noms de la dimension et de la variable temporelles du NetCDF.

La variable doit posséder un attribut « units » dont la valeur est au format « days since yyyy-mm-dd hh:00:00 » ou « minutes since yyyy-mm-dd hh:00:00 ». Ex. : « days since 1970-01-01 00:00:00 ».

La série temporelle NetCDF doit couvrir la période de simulation au pas de temps de la simulation.



---

## 6. Coefficients de distribution

Lorsque vous utilisez une variable **totale** (`PRODTOT` ou `QLATTOT`), Hydrotel répartit le total entre les trois couches avec :

```text
DISTRIBUTION_COEFF1;0.333
DISTRIBUTION_COEFF2;0.333
```

| Coefficient | Appliqué à |
|-------------|------------|
| `DISTRIBUTION_COEFF1` | Couche 1 (surface) |
| `DISTRIBUTION_COEFF2` | Couche 2 (hypodermique) |
| `1 − DISTRIBUTION_COEFF1 − DISTRIBUTION_COEFF2` | Couche 3 (base) |

Contraintes :

- Chaque coefficient doit être compris entre **0 et 1**
- `DISTRIBUTION_COEFF1 + DISTRIBUTION_COEFF2` doit être **≤ 1**

Ces mots-clés sont obligatoires pour `PRODTOT` / `QLATTOT`, et ne sont pas utilisés lorsque vous fournissez les trois couches séparément (`PROD1`/`PROD2`/`PROD3` ou `QLAT1`/`QLAT2`/`QLAT3`).



---

## 7. Exemples

### Exemple A — Production totale sur une grille

Objectif : alimenter la production totale (`PRODTOT`) à partir d’un fichier NetCDF en grille.

1. Dans le fichier de simulation :

   ```text
   EXTERNAL DATA ROUTING;external-data/config.txt
   ```

2. Dans `external-data/config.txt`, les lignes suivantes doivent être présentes :

   - `TIME_DIMNAME;time`

   - `TIME_VARNAME;time`

   - `DIMTYPE;GRID`

   - `LON_DIMNAME;x`

   - `LAT_DIMNAME;y`

   - `LON_VARNAME;x`

   - `LAT_VARNAME;y`

   - `PRODTOT_SOURCE;external-data/prod.nc`

   - `PRODTOT_VARNAME;prod`

   - `DISTRIBUTION_COEFF1;0.333`

   - `DISTRIBUTION_COEFF2;0.333`

     Les autres lignes du fichier (s’il y en a) doivent être commentées avec le préfixe `//`.

Hydrotel interpolera `prod` de `external-data/prod.nc` vers chaque UHRH, le répartira entre les trois couches à l’aide des coefficients et poursuivra la simulation.

### Exemple B — Production par couche et par UHRH

Objectif : alimenter les trois couches de production déjà disponibles par UHRH.

1. Dans le fichier de simulation :

   ```text
   EXTERNAL DATA ROUTING;external-data/config.txt
   ```

2. Dans `config.txt`, les lignes suivantes doivent être présentes :

   - `TIME_DIMNAME;time`

   - `TIME_VARNAME;time`

   - `DIMTYPE;RHHU`

   - `ID_DIMNAME;uhrh`

   - `ID_VARNAME;iduhrh`

   - `PROD1_SOURCE;external-data/production_surf.nc`

   - `PROD1_VARNAME;production_surf`

   - `PROD2_SOURCE;external-data/production_hypo.nc`

   - `PROD2_VARNAME;production_hypo`

   - `PROD3_SOURCE;external-data/production_base.nc`

   - `PROD3_VARNAME;production_base`

     Les autres lignes du fichier (s’il y en a) doivent être commentées avec le préfixe `//`.

Hydrotel affecte à chaque UHRH ses trois valeurs de production à partir des fichiers NetCDF et poursuit la simulation.



---

## 8. Combinaisons compatibles

| Mode de variables | GRID/STATION | RHHU | REACH |
|---------------|:----:|:----:|:-----:|
| `VCONT` | oui | oui | non |
| `PROD1`/`PROD2`/`PROD3`/`PRODTOT` | oui | oui | non |
| `QLAT1`/`QLAT2`/`QLAT3`/`QLATTOT` | non | oui | oui |
