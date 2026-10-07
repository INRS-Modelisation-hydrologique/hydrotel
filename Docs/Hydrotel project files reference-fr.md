# Dictionnaire des fichiers de projet Hydrotel

_7 octobre 2026 - Hydrotel v4.4.0_

Le présent document se veut un dictionnaire de référence décrivant les différents fichiers constituant un dossier de projet HYDROTEL.

Il y est présenté dans un premier temps un tableau démontrant et respectant la structure des dossiers ainsi que les fichiers qui y sont contenu.

Le tableau est suivi de la liste des fichiers en ordre alphabétique ainsi que la description détaillé de ces fichiers.

| Dossier | Fichier |
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

Fichier des paramètres de la simulation.

| Ligne | Description |
| --- | --- |
| SIMULATION HYDROTEL;4.3.0.0000 | Numéro de version d’HYDROTEL utilisé lors de la création de la simulation. |
| Ligne vide |  |
| FICHIER OCCUPATION SOL;[physitel/occupation_sol.cla](#occupation-sol-cla) | Nom du fichier des classes d’occupation du sol. |
| FICHIER PROPRIETE HYDROLIQUE;[physitel/proprietehydrolique.sol](#proprietehydrolique-sol) | Nom du fichier des propriétés hydraulique des sols. |
| FICHIER TYPE SOL COUCHE1;[physitel/type_sol.cla](#type-sol-cla) | Nom du fichier des types de sol dominant des UHRH pour la couche de sol 1. |
| FICHIER TYPE SOL COUCHE2;[physitel/type_sol.cla](#type-sol-cla) | Nom du fichier des types de sol dominant des UHRH pour la couche de sol 2. |
| FICHIER TYPE SOL COUCHE3;[physitel/type_sol.cla](#type-sol-cla) | Nom du fichier des types de sol dominant des UHRH pour la couche de sol 3. |
| Ligne vide |  |
| COEFFICIENT ADDITIF PROPRIETE HYDROLIQUE; | Liste des coefficients additifs des propriétés hydraulique (une valeur pour chaque groupe d’UHRH). Le coefficient sert à déplacer tous les types de sols vers les argiles s’il est positif et vers les sables, s’il est négatif. |
| Ligne vide |  |
| FICHIER INDICE FOLIERE;[physio/ind_fol.def](#ind-fol-def) | Nom du fichier des indices foliaire. |
| FICHIER PROFONDEUR RACINAIRE;[physio/pro_rac.def](#pro-rac-def) | Nom du fichier des profondeurs racinaire. |
| Ligne vide |  |
| FICHIER GRILLE METEO; | Nom du fichier des paramètres pour le sous-modèle GRILLE ([grillemeteo.csv](#grillemeteo-csv)). |
| FICHIER STATIONS METEO;[meteo/station.stm](#station-stm) | Fichier des stations météorologiques. |
| FICHIER STATIONS HYDRO;[hydro/station.sth](#station-sth) | Fichier des stations hydrométriques. |
| Ligne vide |  |
| PREVISION METEO;0 | Activation des prévisions météo (0 : désactivé, 1 : activé). |
| FICHIER GRILLE PREVISION; | Nom du fichier des paramètres pour les prévisions météorologiques ([grilleprevision.csv](#grilleprevision-csv)). |
| DATE DEBUT PREVISION;1973-06-01 00:00 | Date de début pour les prévisions météorologique (format : YYYY-MM-DD HH :00). |
| Ligne vide |  |
| DATE DEBUT;2020-01-01 00:00 | Date de début de la simulation (format : YYYY-MM-DD HH :00). |
| DATE FIN;2020-07-01 00:00 | Date de fin de la simulation (format : YYYY-MM-DD HH :00). |
| PAS DE TEMPS;24 | Pas de temps de la simulation (heures). |
| Ligne vide |  |
| TRONCON EXUTOIRE;1 | Identifiant du tronçon exutoire pour la simulation. Seuls les tronçons en amont du tronçon exutoire sont simulés. |
| Ligne vide |  |
| TRONCONS DECONNECTER;off; | Tronçons déconnectés. La première valeur «on» ou «off» indique si l’option est activée. Les valeurs suivantes sont les identifiant des tronçons déconnecté (ex : «TRONCONS DECONNECTER;on;1;72»). |
| Ligne vide |  |
| EXTERNAL DATA ROUTING; | Nom du fichier de configuration pour l'utilisation et le routage dans la simulation de données externes. Voir le document [external-data-routing](external-data-routing-fr.md) pour une description complète du fichier de configuration. |
| Ligne vide | |
| NOM FICHIER CORRECTIONS;1;[corrections.csv](#corrections-csv) | Nom du fichier de correction à utiliser précédé de 0 ou 1 pour indiquer si l’option est activée. |
| Ligne vide |  |
| LECTURE ETAT FONTE NEIGE;[etats/fonte_neige_2020010100.csv](#etats) | Nom du fichier d’état pour le sous-modèle de fonte de la neige. |
| LECTURE ETAT TEMPERATURE DU SOL; | Nom du fichier d’état pour le sous-modèle de température du sol. |
| LECTURE ETAT BILAN VERTICAL;[etats/bilan_vertical_2020010100.csv](#etats) | Nom du fichier d’état pour le sous-modèle de bilan vertical. |
| LECTURE ETAT RUISSELEMENT SURFACE;[etats/ruisselement_surface_2020010100.csv](#etats) | Nom du fichier d’état pour le sous-modèle de ruissellement de surface. |
| LECTURE ETAT ACHEMINEMENT RIVIERE;[etats/acheminement_riviere_2020010100.csv](#etats) | Nom du fichier d’état pour le sous-modèle d’acheminement en rivière. |
| Ligne vide |  |
| ECRITURE ETAT FONTE NEIGE;2020-07-01 00:00 | Date et heure du pas de temps pour la sauvegarde des variables d’état. |
| ECRITURE ETAT TEMPERATURE DU SOL; | Date et heure du pas de temps pour la sauvegarde des variables d’état. |
| ECRITURE ETAT BILAN VERTICAL;2020-07-01 00:00 | Date et heure du pas de temps pour la sauvegarde des variables d’état. |
| ECRITURE ETAT RUISSELEMENT SURFACE;2020-07-01 00:00 | Date et heure du pas de temps pour la sauvegarde des variables d’état. |
| ECRITURE ETAT ACHEMINEMENT RIVIERE;2020-07-01 00:00 | Date et heure du pas de temps pour la sauvegarde des variables d’état. |
| Ligne vide |  |
| REPERTOIRE ECRITURE ETAT FONTE NEIGE;/etats | Dossier à utiliser pour la sauvegarde des variables d’état. |
| REPERTOIRE ECRITURE ETAT TEMPERATURE DU SOL; | Dossier à utiliser pour la sauvegarde des variables d’état. |
| REPERTOIRE ECRITURE ETAT BILAN VERTICAL;/etats | Dossier à utiliser pour la sauvegarde des variables d’état. |
| REPERTOIRE ECRITURE ETAT RUISSELEMENT SURFACE;/etats | Dossier à utiliser pour la sauvegarde des variables d’état. |
| REPERTOIRE ECRITURE ETAT ACHEMINEMENT RIVIERE;/etats | Dossier à utiliser pour la sauvegarde des variables d’état. |
| Ligne vide |  |
| INTERPOLATION DONNEES;THIESSEN | Nom du sous-modèle à utiliser pour l’interpolation des données météorologiques. Valeurs possibles : THIESSEN, MOYENNE 3 STATIONS, GRILLE, LECTURE INTERPOLATION DONNEES. |
| FONTE NEIGE;DEGRE JOUR MODIFIE | Nom du sous-modèle à utiliser pour l’évolution et fonte du couvert nival. Valeurs possibles : DEGRE JOUR MODIFIE, DEGRE JOUR BANDE, LECTURE FONTE NEIGE. |
| FONTE GLACIER; | Nom du sous-modèle à utiliser pour l’évolution et la fonte de glace (glacier). Valeurs possibles : DEGRE JOUR GLACIER, LECTURE FONTE GLACIER. |
| TEMPERATURE DU SOL; | Nom du sous-modèle à utiliser pour le calcul de la température du sol. Valeurs possibles : RANKINEN, THORSEN, LECTURE TEMPERATURE DU SOL. |
| EVAPOTRANSPIRATION;HYDRO-QUEBEC | Nom du sous-modèle à utiliser pour le calcul de l’évapotranspiration potentielle. Valeurs possibles : HYDRO-QUEBEC, ETP-MC-GUINESS, LINACRE, PENMAN, PENMAN-MONTEITH, PRIESTLAY-TAYLOR, THORNTHWAITE, LECTURE EVAPOTRANSPIRATION. |
| BILAN VERTICAL;BV3C | Nom du sous-modèle à utiliser pour le calcul des bilans verticaux. Valeurs possibles : BV3C, CEQUEAU LECTURE BILAN VERTICAL. |
| RUISSELEMENT;ONDE CINEMATIQUE | Nom du sous-modèle à utiliser pour le calcul du ruissellement de surface. Valeurs possibles : ONDE CINEMATIQUE, LECTURE RUISSELEMENT SURFACE. |
| ACHEMINEMENT RIVIERE;ONDE CINEMATIQUE MODIFIEE | Nom du sous-modèle à utiliser pour le calcul de l’acheminement en rivière. Valeurs possibles : ONDE CINEMATIQUE MODIFIEE, LECTURE ACHEMINEMENT RIVIERE. |
| TEMPERATURE EAU; | Nom du sous-modèle à utiliser pour le calcul de la température de l'eau. Valeur possible : TEMP EAU CEQUEAU. |
| Ligne vide |  |
| MILIEUX HUMIDES ISOLES;1 | Active (1) ou désactive (0) la simulation des milieux humides isolés. |
| MILIEUX HUMIDES RIVERAINS;1 | Active (1) ou désactive (0) la simulation des milieux humides riverains. |
| Ligne vide |  |
| FICHIER DE PARAMETRE GLOBAL;0 | Active (1) ou désactive (0) l’utilisation d’un fichier unique pour tous les paramètres des sous-modèles ([parametres_sous_modeles.csv](#parametres-sous-modeles-csv)). |
| Ligne vide |  |
| LECTURE INTERPOLATION DONNEES;[lecture_interpolation.csv](#lecture-interpolation-csv) | Nom des fichiers des paramètres des sous-modèles. |
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
| TEMP EAU CEQUEAU;[temp_eau_cequeau_troncons.csv](#temp-eau-cequeau-troncons-csv) ; [temp_eau_cequeau_zones.csv](#temp-eau-cequeau-zones-csv) | Noms des fichiers de paramètres pour le sous-modèle TEMP EAU CEQUEAU. |
| FICHIER MILIEUX HUMIDES ISOLES;[milieux_humides_isoles.csv](#milieux-humides-isoles-csv) | Nom du fichier des paramètres pour les milieux humides isolés. |
| FICHIER MILIEUX HUMIDES RIVERAINS;[milieux_humides_riverains.csv](#milieux-humides-riverains-csv) | Nom du fichier des paramètres pour les milieux humides riverains. |

<a id="nom-simulation-gsb"></a>

## \{nom_simulation\}.gsb   (/simulation/\{nom_simulation\})

Fichier de description des groupes d’UHRH.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{NbGroupe\} |
| Ligne 2 à x | \{NomGroupe\} |
| Ligne 2 + NbGroupe et suivantes | \{IdUhrh\} \{IndexGroupe\} |

NbGroupe : Nombre de groupe d’UHRH.

NomGroupe : Nom du groupe. Il doit y avoir une ligne avec le nom du groupe pour chacun des groupes (ex : s’il y a 3 groupes il doit y avoir 3 lignes avec les noms des groupes).

IdUhrh : Identifiant de l’UHRH.

IndexGroupe : Index du groupe à lequel appartient l’UHRH. L’index du premier groupe est 0, l’index du deuxième groupe est 1, et ainsi de suite…



Ex : 

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

Fichier de données des observations météorologiques pour la station «0000040».

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type des données\} \{Pas de temps\} |
| Ligne 2 et suivantes | \{Date/Heure\} \{TMax\} \{TMin\} \{Precip\} |

Type des données : Cette valeur doit être fixée à «1».

Pas de temps : Pas de temps des données (heures).

Date/Heure : Date et heure de l’observation (JJ/MM/AAAA H). L’heure ne doit pas être spécifiée lorsque le pas de temps est égal à 24 heures.

TMax : Température maximal (°C).

TMin : Température minimal (°C).

Precip : Précipitation totale (pluie + équivalent en eau de la neige) (mm).



Ex :  

1 3

01/01/2003 3 -12.5 -14.2 1.5

01/01/2003 6 -999 –999 0.5

01/01/2003 9 -11.5 -13.0 0



Ex :  

1 24

01/01/2003 -12.5 -14.2 1.5

02/01/2003 -10.0 –13.2 -999

03/01/2003 -11.5 -13.0 5.25



L'heure lue dans les fichiers de données correspond à l'heure à la fin du pas de temps. L'heure 3 représente la donnée pour 0h à 3h. Ex : pour un pas de temps de 3h, les heures dans le fichier de données doivent être pour une journée : 3, 6, 9, 12, 15, 18, 21, 24. La valeur -999 doit être utilisée pour représenter les valeurs manquantes (No data). Dans le cas des données horaires, les champs de températures maximale et minimale sont remplacés par un champ température unique.

<a id="02mc036-hyd"></a>
## 02MC036.hyd   (/hydro)

Fichier de données des observations hydrométriques pour la station «02MC036».

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type des données\} \{Pas de temps\} |
| Ligne 2 et suivantes | \{Date/Heure\} \{Débit\} (\{Niveau\}) |

Type des données : 1 = Débits seulement, 2 = Débits et niveaux.

Pas de temps : Pas de temps des données (heures).

Date/Heure : Date et heure de l’observation (JJ/MM/AAAA H). L’heure ne doit pas être spécifiée lorsque le pas de temps est égal à 24 heures.

Débit : Débit observé (m3/s).

Niveau : Niveau observé (m).



Ex :  

1 3

01/01/2003 3 0.308

01/01/2003 6 0.291

01/01/2003 9 0.286



Ex :  

2 24

01/01/2003 0.308 0.1

02/01/2003 0.291 0.08

03/01/2003 0.286 0.06



L'heure lue dans les fichiers de données correspond à l'heure à la fin du pas de temps. L'heure 3 représente la donnée pour 0h à 3h. Ex : pour un pas de temps de 3h, les heures dans le fichier de données doivent être pour une journée : 3, 6, 9, 12, 15, 18, 21, 24. Les données de niveaux ne sont pas utilisées actuellement par HYDROTEL. La valeur -999 doit être utilisée pour représenter les valeurs manquantes (No data).

<a id="altitude-tif"></a>
## altitude.tif   (/physitel)

Carte matricielle (GeoTIFF) des élévations (MNA) (m).

<a id="bv3c-csv"></a>
## bv3c.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul BV3C (bilan d’eau vertical).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE;BV3C» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «CLASSE INTEGRE IMPERMEABLE;» \{ListOccSolImpermeable\} |
| Ligne 6 | «CLASSE INTEGRE EAU;» \{ListOccSolEau\} |
| Ligne 7 | Ligne vide |
| Ligne 8 | Commentaire |
| Ligne 9 et suivantes | \{IdUhrh\} \{EpaisseurCouche\} \{EpaisseurCouche2\} \{EpaisseurCouche3\} \{HumIniCouche1\} \{HumIniCouche2\} \{HumIniCouche3\} \{CoefExtinction\} \{CoefRecession\} \{CoefAssechement\} \{VarMaxHumidite\} \{CoefRecharge\} |

ListOccSolImpermeable : Liste des identifiants (1 à x) des classes d’occupation du sol associées à la classe intégré «Imperméable». L’identifiant 1 correspond à la première classe d’occupation du sol du fichier [occupation_sol.cla](#occupation-sol-cla), l’identifiant 2 à la deuxième classe et ainsi de suite. Les identifiants doivent être séparé par le caractère point-virgule «;».

ListOccSolEau : Liste des identifiants (1 à x) des classes d’occupation du sol associées à la classe intégré «Eau».

IdUhrh : Identifiant de l’UHRH.

EpaisseurCouche1 : Épaisseur de la 1ère couche de sol (surface) (m).

EpaisseurCouche2 : Épaisseur de la 2e couche de sol (intermédiaire) (m).

EpaisseurCouche3 : Épaisseur de la 3e couche de sol (base) (m).

HumIniCouche1 : Humidité relative initiale de la 1ère couche de sol (fraction de la saturation) (0-1).

HumIniCouche2 : Humidité relative initiale de la 2e couche de sol (fraction de la saturation) (0-1).

HumIniCouche3 : Humidité relative initiale de la 3e couche de sol (fraction de la saturation) (0-1).

CoefExtinction : Coefficient d’extinction D du rayonnement solaire dans la végétation.

CoefRecession : Coefficient de récession kr pour l’écoulement de base en provenance de la 3e couche de sol (m/h).

CoefAssechement : Coefficient multiplicatif d’optimisation de l’assèchement (appliqué au coefficient d’assèchement Cs).

VarMaxHumidite : Variation maximale de l’humidité relative dans les couches de sol pour chaque pas de temps. Sert à limiter les instabilités numériques provoquant des oscillations des valeurs d’humidité, particulièrement entre les premières et secondes couches. Plus cette valeur est faible plus les instabilités sont filtrées mais avec comme conséquence que les calculs peuvent être un peu plus longs.

CoefRecharge : Coefficient de recharge souterrain (0-1).

<a id="cequeau-csv"></a>
## cequeau.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul CEQUEAU (bilan d’eau vertical).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE;CEQUEAU» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «CLASSE INTEGRE EAU (LACS ET MARECAGES);» \{ListOccSolLacMarecage\} |
| Ligne 6 | «CLASSE INTEGRE IMPERMEABLE;» \{ListOccSolImpermeable\} |
| Ligne 7 | «CLASSE INTEGRE FORETS;» \{ListOccSolForet\} |
| Ligne 8 | Ligne vide |
| Ligne 9 | Commentaire |
| Ligne 10 et suivantes | \{IdUhrh\} \{MinRuisImpermeable\} \{NiveauEauMax\} \{SeuilVidangeSol\} \{CoefVidangeRetarde1\} \{CoefVidangeRetarde2\} \{SeuilPercolation\} \{CoefPercolation\} \{MaxPercolation\} \{NiveauEau\} \{CoefVidangeHaute\} \{CoefVidangeBasse\} \{FractionEtp\} \{HauteurVidangeHaute\} \{SeuilVidangeEau\} \{CoefVidangeEau\} \{InitSol\} \{InitNappe\} \{InitEau\} |

ListOccSolLacMarecage : Liste des identifiants (1 à x) des classes d’occupation du sol associées à la classe intégré «Lac et marécage». L’identifiant 1 correspond à la première classe d’occupation du sol du fichier [occupation_sol.cla](#occupation-sol-cla), l’identifiant 2 à la deuxième classe et ainsi de suite. Les identifiants doivent être séparé par le caractère point-virgule «;».

ListOccSolImpermeable : Liste des identifiants (1 à x) des classes d’occupation du sol associées à la classe intégré «Imperméable».

ListOccSolForet : Liste des identifiants (1 à x) des classes d’occupation du sol associées à la classe intégré «Forêt».

IdUhrh : Identifiant de l’UHRH.

MinRuisImpermeable : Seuil minimal du ruissellement de la surface imperméable (mm).

NiveauEauMax : Niveau d’eau maximal (sol) (mm).

SeuilVidangeSol : Seuil de vidange (sol) (mm).

CoefVidangeRetarde1 : Coefficient de vidange retardé 1 (sol).

CoefVidangeRetarde2 : Coefficient de vidange retardé 2 (sol).

SeuilPercolation : Seuil de percolation (sol).

CoefPercolation : Coefficient de percolation (sol).

MaxPercolation : Taux maximum de percolation (sol) (mm/j).

NiveauEau : Niveau d’eau (sol) (mm).

CoefVidangeHaute : Coefficient de vidange haute (nappe).

CoefVidangeBasse : Coefficient de vidange basse (nappe).

FractionETP : Fraction de l’ETP puisé (nappe) (0-1).

HauteurVidangeHaute : Hauteur de vidange haute (nappe) (mm).

SeuilVidangeEau : Seuil de vidange (eau) (mm).

CoefVidangeEau : Coefficient de vidange (eau).

InitSol : Niveau d’eau initial (sol) (mm).

InitNappe : Niveau d’eau initial (nappe) (mm).

InitEau : Niveau d’eau initial (eau) (mm).

<a id="corrections-csv"></a>
## corrections.csv   (/simulation/\{nom_simulation\})

Paramètres de correction des variables interne de simulation.

Le fichier de correction peut contenir une ou plusieurs lignes de correction.

Une ligne est composée des colonnes suivantes pour les variables 1 à 5 :

| \{Actif\} \{DateHeureDebut\} \{DateHeureFin\} \{Variable\} \{CoefAdditif\} \{CoefMultiplicatif\} \{TypeGroupe\} \{NomGroupe\} |
| --- |

Une ligne est composée des colonnes suivantes pour la variable 6 :

| \{Actif\} \{DateHeureDebut\} \{DateHeureFin\} \{Variable\} \{CoeffSaturationCouche1\} \{CoeffSaturationCouche2\} \{CoeffSaturationCouche3\} \{TypeGroupe\} \{NomGroupe\} |
| --- |

Actif : Indique si la ligne de correction est activée ou désactivée. Valeur : 0=Désactivée, 1=Activée.

DateHeureDebut : Date et heure de début de l’application de la correction. Format : «AAAA/MM/JJ HH». Ex : «2026/01/15 00».

DateHeureFin : Date et heure de fin de l’application de la correction. Format : «AAAA/MM/JJ HH». Ex : «2026/01/15 00».

Variable : Identifiant de la variable à corriger : 1=Température, 2=Précipitation (pluie), 3=Précipitation (neige), 4=Réserve en eau du sol, 5=Neige au sol (couvert nival), 6=Saturation de la réserve en eau du sol.

CoefAdditif : Coefficient additif.

CoefMultiplicatif : Coefficient multiplicatif.

CoeffSaturationCouche1 : Coefficient de la saturation de la couche de sol 1.

CoeffSaturationCouche2 : Coefficient de la saturation de la couche de sol 2.

CoeffSaturationCouche3 : Coefficient de la saturation de la couche de sol 3.

TypeGroupe : Indique le type de groupe (groupe d’UHRH ou groupe de correction) spécifié par la variable «NomGroupe» pour l’application de la correction. Valeur : 0=Tous les UHRH, 1=GroupeUHRH, 2=GroupeCorrection.

NomGroupe : Nom du groupe. L’application de la correction est effectuée pour chaque UHRH appartenant au groupe d’UHRH ou de correction.



Ex : 

1;2020/01/01 00;2021/01/01 00;2;0;0.5;0;aucun

1;2020/01/01 00;2021/01/01 00;3;0;0.5;0;aucun

1;2020/01/01 00;2021/01/01 00;6;0.8;0.8;0.8;1;groupe1



Les lignes du fichier ne commençant pas par «0;» ou «1;» sont traitées comme ligne de commentaire et  ignorées.

<a id="degre-jour-bande-csv"></a>
## degre-jour-bande.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul DEGRE JOUR BANDE (évolution et fonte du couvert nival).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; DEGRE JOUR BANDE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «CLASSE INTEGRE CONIFERES;» \{ListOccSolM1\} |
| Ligne 6 | «CLASSE INTEGRE FEUILLUS;» \{ListOccSolM2\} |
| Ligne 7 | Ligne vide |
| Ligne 8 | «NOM FICHIER STATION NEIGE CONIFERS;» \{NomFichierNeigeM1\} |
| Ligne 9 | «NOM FICHIER STATION NEIGE FEUILLUS;» \{NomFichierNeigeM2\} |
| Ligne 10 | «NOM FICHIER STATION NEIGE DECOUVERTS;» \{NomFichierNeigeM3\} |
| Ligne 11 | Ligne vide |
| Ligne 12 | «INTERPOLATION STATION NEIGE CONIFERS;» \{InterpolationM1\} |
| Ligne 13 | «INTERPOLATION STATION NEIGE FEUILLUS;» \{InterpolationM2\} |
| Ligne 14 | «INTERPOLATION STATION NEIGE DECOUVERTS;» \{InterpolationM3\} |
| Ligne 15 | Ligne vide |
| Ligne 16 | «HAUTEUR BANDE(m);» \{HauteurBande\} |
| Ligne 17 | Ligne vide |
| Ligne 18 | Commentaire |
| Ligne 19 et suivantes | \{IdUhrh\} \{TauxFonte\} \{DensiteMax\} \{ConstanteTassement\} \{SeuilFonteM1\} \{SeuilFonteM2\} \{SeuilFonteM3\} \{TauxFonteM1\} \{TauxFonteM2\} \{TauxFonteM3\} \{SeuilAlbedo\} |

ListOccSolM1 : Liste des identifiants (1 à x) des classes d’occupation du sol associées au milieu 1 (ex : conifères). L’identifiant 1 correspond à la première classe d’occupation du sol du fichier [occupation_sol.cla](#occupation-sol-cla), l’identifiant 2 à la deuxième classe et ainsi de suite. Les identifiants doivent être séparé par le caractère point-virgule «;».

ListOccSolM2 : Liste des identifiants (1 à x) des classes d’occupation du sol associées au milieu 2 (ex : feuillus).

NomFichierNeigeM1 : Nom du fichier des stations (.stn) pour la mise à jour de la neige pour le milieu 1 (ex : conifères). Il est possible d’utiliser un même fichier pour les 3 milieux (classes intégrés). Le chemin spécifié peut être relatif au dossier du projet (ex : «[neige/station.stn](#station-stn)»).

NomFichierNeigeM2 : Nom du fichier des stations (.stn) pour la mise à jour de la neige pour le milieu 2 (ex : feuillus). Il est possible d’utiliser un même fichier pour les 3 milieux (classes intégrés). Le chemin spécifié peut être relatif au dossier du projet (ex : «[neige/station.stn](#station-stn)»).

NomFichierNeigeM3 : Nom du fichier des stations (.stn) pour la mise à jour de la neige pour le milieu 3 (ex : découverts). Il est possible d’utiliser un même fichier pour les 3 milieux (classes intégrés). Le chemin spécifié peut être relatif au dossier du projet (ex : «[neige/station.stn](#station-stn)»).

InterpolationM1 : Méthode d’interpolation à utiliser pour la mise à jour de la neige sur le milieu 1 (ex : conifères). Valeurs possible : «MOYENNE 3 STATIONS» ou «THIESSEN».

InterpolationM2 : Méthode d’interpolation à utiliser pour la mise à jour de la neige sur le milieu 2 (ex : feuillus). Valeurs possible : «MOYENNE 3 STATIONS» ou «THIESSEN».

InterpolationM3 : Méthode d’interpolation à utiliser pour la mise à jour de la neige sur le milieu 3 (ex : découverts). Valeurs possible : «MOYENNE 3 STATIONS» ou «THIESSEN».

HauteurBande : Hauteur des bandes altitudinale (m).

IdUhrh : Identifiant de l’UHRH.

TauxFonte : Taux de fonte (neige-sol) (mm/jour).

DensiteMax : Densité maximale du couvert nival (kg/m3).

ConstanteTassement : Constante de tassement.

SeuilFonteM1 : Seuil de température au-dessus duquel il y a fonte (milieu 1, ex : conifères) (°C).

SeuilFonteM2 : Seuil de température au-dessus duquel il y a fonte (milieu 2, ex : feuillus) (°C).

SeuilFonteM3 : Seuil de température au-dessus duquel il y a fonte (milieu 3, ex : découverts) (°C).

TauxFonteM1 : Taux de fonte dans l’air (milieu 1, ex : conifères) (mm/jour/°C).

TauxFonteM2 : Taux de fonte dans l’air (milieu 2, ex : feuillus) (mm/jour/°C).

TauxFonteM3 : Taux de fonte dans l’air (milieu 3, ex : découverts) (mm/jour/°C).

SeuilAlbedo : Seuil pour algorithme d’albédo «Exponentielle avec seuil» (cm).

<a id="degre-jour-glacier-csv"></a>
## degre-jour-glacier.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul DEGRE JOUR GLACIER (évolution et fonte de glace).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; DEGRE JOUR GLACIER» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «CLASSE INTEGRE GLACIER;» \{ListOccSolGlacier\} |
| Ligne 6 | Ligne vide |
| Ligne 7 | «DENSITE GLACE(kg/m3);» \{DensiteGlace\} |
| Ligne 8 | Ligne vide |
| Ligne 9 | «CONSTANTE EMPIRIQUE 0;» \{ConstanteEmpirique0\} |
| Ligne 10 | «CONSTANTE EMPIRIQUE 1;» \{ConstanteEmpirique1\} |
| Ligne 11 | Ligne vide |
| Ligne 12 | «EPAISSEUR GLACE MIN(m);» \{EpaisseurGlaceMin\} |
| Ligne 13 | «EPAISSEUR GLACE MAX(m);» \{EpaisseurGlaceMax\} |
| Ligne 14 | Ligne vide |
| Ligne 15 | «MASSE GLACE FIXE(0/1);» \{MasseGlaceFixe\} |
| Ligne 16 | Ligne vide |
| Ligne 17 | Commentaire |
| Ligne 18 et suivantes | \{IdUhrh\} \{TauxFonte\} \{SeuilFonte\} \{Albedo\} |

ListOccSolGlacier : Liste des identifiants (1 à x) des classes d’occupation du sol associées au milieu «Glaciers» (glace). L’identifiant 1 correspond à la première classe d’occupation du sol du fichier [occupation_sol.cla](#occupation-sol-cla), l’identifiant 2 à la deuxième classe et ainsi de suite. Les identifiants doivent être séparé par le caractère point-virgule «;».

DensiteGlace : Densité de la glace (kg/m3).

ConstanteEmpirique0 : Constante empirique 0 (c0).

ConstanteEmpirique1 : Constante empirique 1 (c1).

EpaisseurGlaceMin : (équation droite épaisseur glace) (m).

EpaisseurGlaceMax : (équation droite épaisseur glace) (m).

MasseGlaceFixe : Permet d’empêcher la fonte du stock de glace (valeur : «1»). Sinon mettre la valeur «0» pour que la fonte soit permise.

IdUhrh : Identifiant de l’UHRH.

TauxFonte : Taux de fonte dans l’air (milieu 1, ex : glacier) (mm/jour/°C).

SeuilFonte : Seuil de température au-dessus duquel il y a fonte (milieu 1, ex : glacier) (°C).

Albedo : Albedo (milieu 1, ex : glacier) (0-1).

<a id="degre-jour-modifie-csv"></a>
## degre_jour_modifie.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul DEGRE JOUR MODIFIE (évolution et fonte du couvert nival).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; DEGRE JOUR MODIFIE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «CLASSE INTEGRE CONIFERES;» \{ListOccSolM1\} |
| Ligne 6 | «CLASSE INTEGRE FEUILLUS;» \{ListOccSolM2\} |
| Ligne 7 | Ligne vide |
| Ligne 8 | «NOM FICHIER STATION NEIGE CONIFERS;» \{NomFichierNeigeM1\} |
| Ligne 9 | «NOM FICHIER STATION NEIGE FEUILLUS;» \{NomFichierNeigeM2\} |
| Ligne 10 | «NOM FICHIER STATION NEIGE DECOUVERTS;» \{NomFichierNeigeM3\} |
| Ligne 11 | Ligne vide |
| Ligne 12 | «INTERPOLATION STATION NEIGE CONIFERS;» \{InterpolationM1\} |
| Ligne 13 | «INTERPOLATION STATION NEIGE FEUILLUS;» \{InterpolationM2\} |
| Ligne 14 | «INTERPOLATION STATION NEIGE DECOUVERTS;» \{InterpolationM3\} |
| Ligne 15 | Ligne vide |
| Ligne 16 | «MISE A JOUR GRILLE NEIGE;» \{MiseAJourGrilleNeige\} |
| Ligne 17 | «NOM FICHIER GRILLE NEIGE;» \{NomFichierGrilleNeige\} |
| Ligne 18 | Ligne vide |
| Ligne 19 | Commentaire |
| Ligne 20 et suivantes | \{IdUhrh\} \{TauxFonte\} \{DensiteMax\} \{ConstanteTassement\} \{SeuilFonteM1\} \{SeuilFonteM2\} \{SeuilFonteM3\} \{TauxFonteM1\} \{TauxFonteM2\} \{TauxFonteM3\} \{SeuilAlbedo\} |

ListOccSolM1 : Liste des identifiants (1 à x) des classes d’occupation du sol associées au milieu 1 (ex : conifères). L’identifiant 1 correspond à la première classe d’occupation du sol du fichier [occupation_sol.cla](#occupation-sol-cla), l’identifiant 2 à la deuxième classe et ainsi de suite. Les identifiants doivent être séparé par le caractère point-virgule «;».

ListOccSolM2 : Liste des identifiants (1 à x) des classes d’occupation du sol associées au milieu 2 (ex : feuillus).

NomFichierNeigeM1 : Nom du fichier des stations (.stn) pour la mise à jour de la neige pour le milieu 1 (ex : conifères). Il est possible d’utiliser un même fichier pour les 3 milieux (classes intégrés). Le chemin spécifié peut être relatif au dossier du projet (ex : «[neige/station.stn](#station-stn)»).

NomFichierNeigeM2 : Nom du fichier des stations (.stn) pour la mise à jour de la neige pour le milieu 2 (ex : feuillus). Il est possible d’utiliser un même fichier pour les 3 milieux (classes intégrés). Le chemin spécifié peut être relatif au dossier du projet (ex : «[neige/station.stn](#station-stn)»).

NomFichierNeigeM3 : Nom du fichier des stations (.stn) pour la mise à jour de la neige pour le milieu 3 (ex : découverts). Il est possible d’utiliser un même fichier pour les 3 milieux (classes intégrés). Le chemin spécifié peut être relatif au dossier du projet (ex : «[neige/station.stn](#station-stn)»).

InterpolationM1 : Méthode d’interpolation à utiliser pour la mise à jour de la neige sur le milieu 1 (ex : conifères). Valeurs possible : «MOYENNE 3 STATIONS» ou «THIESSEN».

InterpolationM2 : Méthode d’interpolation à utiliser pour la mise à jour de la neige sur le milieu 2 (ex : feuillus). Valeurs possible : «MOYENNE 3 STATIONS» ou «THIESSEN».

InterpolationM3 : Méthode d’interpolation à utiliser pour la mise à jour de la neige sur le milieu 3 (ex : découverts). Valeurs possible : «MOYENNE 3 STATIONS» ou «THIESSEN».

MiseAJourGrilleNeige : Permet d’activer ou de désactiver la mise à jour de la neige (couvert nival) à l’aide d’une grille de données (carte matricielle). Valeurs possible : 0=désactivé, 1=activé.

NomFichierGrilleNeige : Chemin vers le fichier des paramètres (.grn) pour la mise à jour de la neige avec grille de données. Le chemin spécifié peut être relatif au dossier du projet (ex : «[neige-grille/grilleneige.grn](#grilleneige-grn)»).

IdUhrh : Identifiant de l’UHRH.

TauxFonte : Taux de fonte (neige-sol) (mm/jour).

DensiteMax : Densité maximale du couvert nival (kg/m3).

ConstanteTassement : Constante de tassement.

SeuilFonteM1 : Seuil de température au-dessus duquel il y a fonte (milieu 1, ex : conifères) (°C).

SeuilFonteM2 : Seuil de température au-dessus duquel il y a fonte (milieu 2, ex : feuillus) (°C).

SeuilFonteM3 : Seuil de température au-dessus duquel il y a fonte (milieu 3, ex : découverts) (°C).

TauxFonteM1 : Taux de fonte dans l’air (milieu 1, ex : conifères) (mm/jour/°C).

TauxFonteM2 : Taux de fonte dans l’air (milieu 2, ex : feuillus) (mm/jour/°C).

TauxFonteM3 : Taux de fonte dans l’air (milieu 3, ex : découverts) (mm/jour/°C).

SeuilAlbedo : Seuil pour algorithme d’albédo «Exponentielle avec seuil» (cm).

<a id="etats"></a>
## etats (dossier)   (/)

Contient les fichiers avec les valeurs d’initialisation (conditions initiales) des modèles. Le nom des fichiers doit contenir en suffixe la date et l’heure du pas de temps de la simulation ([acheminement_riviere_AAAAMMJJHH.csv](#etats)). Lorsque spécifiés dans le fichier de simulation (lignes LECTURE ETAT), les variables de simulation interne des modèles sont initialisées à la date spécifiée avec les valeurs des fichiers (ex : LECTURE ETAT ACHEMINEMENT RIVIERE;[etats/acheminement_riviere_2020010100.csv](#etats)). Les fichiers peuvent être générés en inscrivant la date et l’heure du pas de temps souhaité (ex : ECRITURE ETAT ACHEMINEMENT RIVIERE;2020-07-01 00:00). Le mot clé «fin» peut être inscrit au lieu de la date et l’heure lorsque l’on souhaite avoir les états du dernier pas de temps de la simulation. Le mot clé «tous» peut être inscrit au lieu de la date et l’heure lorsque l’on souhaite avoir les états de tous les pas de temps de la simulation. Les lignes REPERTOIRE ECRITURE ETAT permettent de spécifier le dossier où seront sauvegardés les fichiers.

<a id="etp-mc-guiness-csv"></a>
## etp-mc-guiness.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul ETP-MC-GUINESS (évapotranspiration potentielle).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; ETP-MC-GUINESS» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{CoefOptimisation\} |

IdUhrh : Identifiant de l’UHRH.

CoefOptimisation : Coefficient multiplicatif d’optimisation.

<a id="grillemeteo-csv"></a>
## grillemeteo.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul GRILLE (interpolation des données météorologiques).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; GRILLE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

IdUhrh : Identifiant de l’UHRH.

GradientTemp: Gradient vertical de la température (°C/100m).

GradientPrecip: Gradient vertical des précipitations (mm/100m).

PassagePluieNeige : Température de passage de la pluie en neige (°C).

<a id="grilleneige-grn"></a>
## grilleneige.grn   (/neige-grille)

Fichier des paramètres pour la mise à jour de la neige avec grille de données.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{TypeCoord\} |
| Ligne 2 | \{UniteMesure\} |
| Ligne 3 | \{TypePasTemps\} |
| Ligne 4 | Commentaire |
| Ligne 5 | \{Frequence\} \{ChaineNbChar\} \{DossierDonnees\} |
| Ligne 6 | \{Prefixe\} |
| Ligne 7 | \{NbTypeDonnees\} |

TypeCoord : Non utilisé. Fixé à «2».

UniteMesure : Unité des mesures des fichiers de données. Valeurs possible : «1» mètres, «2» cm, «3» mm.

TypePasTemps: Non utilisé. Fixé à «1».

Frequence : Non utilisé. Doit être fixé à «32».

ChaineNbChar : Non utilisé. Fixé à «@0».

DossierDonnees : Nom du dossier contenant les données de neige (grilles de données). Le nom du dossier (chemin) spécifié peut être relatif au dossier du projet (ex : «neige-grille/donnees»).

Prefixe : Préfixe du nom des fichiers de données. Le préfixe est ajouté au nom des fichiers lors de la lecture. Ex : préfixe = «neige_», nom fichier = «[neige_2003_01_22_24h.een](#neige-2003-01-22-24h-een)». Le préfixe spécifié peut être une chaine vide dans le cas où l’on ne veut pas utiliser de préfixe.

NbTypeDonnees : Non utilisé. Doit être fixé à «3».



Les fichiers de données de l’équivalent en eau du couvert nival ainsi que la hauteur du couvert nival doivent se trouver dans le dossier de données spécifié et utiliser la nomenclature suivante : \{Prefixe\}AAAA_MM_JJ_24h.een (équivalent en eau) et \{Prefixe\}AAAA_MM_JJ_24h.hau (hauteur). Ex : pour une mise à jour le 22 janvier 2003 : [neige_2003_01_22_24h.een](#neige-2003-01-22-24h-een) et [neige_2003_01_22_24h.hau](#neige-2003-01-22-24h-hau).



Exemple fichier .grn :

2

3

1

Commentaire

32 @0 neige-grille/donnees

neige_

3

<a id="grilleprevision-csv"></a>
## grilleprevision.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul GRILLE PREVISION (prévisions météorologiques).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; GRILLE PREVISION» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

IdUhrh : Identifiant de l’UHRH.

GradientTemp: Gradient vertical de la température (°C/100m).

GradientPrecip: Gradient vertical des précipitations (mm/100m).

PassagePluieNeige : Température de passage de la pluie en neige (°C).

<a id="hydro-quebec-csv"></a>
## hydro_quebec.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul HYDRO-QUEBEC (évapotranspiration potentielle).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; HYDRO-QUEBEC» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{CoefOptimisation\} |

IdUhrh : Identifiant de l’UHRH.

CoefOptimisation : Coefficient multiplicatif d’optimisation.

<a id="hydrogramme-hgm"></a>

## hydrogramme.hgm   (/hgm)

Contient les valeurs calculées pour l’hydrogramme géomorphologique selon les paramètres spécifiés pour le modèle ONDE_CINEMATIQUE ([onde_cinematique.csv](#onde-cinematique-csv)). Ce fichier est généré automatiquement par HYDROTEL lorsqu’il est inexistant.

<a id="ind-fol-def"></a>
## ind_fol.def   (/physio)

Fichier de données portant sur les indices foliaire.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Format\} |
| Ligne 2 | \{NbOccupation\} \{NbJour\} |
| Ligne 3 | Commentaire |
| Ligne 4 | Commentaire |
| Ligne 5 et suivantes | \{Jour\} \{ValeurOccSol1\} \{ValeurOccSol2\} \{ValeurOccSol3\} etc… |

Format : Fixé à «2» (non utilisé).

NbOccupation : Nombre de classes d’occupation du sol.

NbJour : Nombres de jour (lignes avec valeurs) spécifié dans le fichier.

Jour : Jour de début de l’application des paramètres (Jour julien).

ValeurOccSol1 à ValeurOccsolX : Valeurs d’indice foliaire pour le jour en cours pour chaque classe d’occupation du sol. Les valeurs sont séparées par le caractère «espace» ou «tabulation».



Ex :   

2

10 3

Indices foliaire au jour J / Leaf area index on day D

Jour "FORETS CONIFERES" "FORETS FEUILLUS" "FORETS MIXTES" "AGRICULTURE"

(suite ligne 4)  "URBAIN" "ROUTES" "MILIEUX OUVERTS" "EAU" "SOLS NUS"

(suite ligne 4)  "MILIEUX HUMIDES"

1		5	3	3	0	0	0	1	0	0	2

210	5	5	5	2	0	0	3	0	0	4

365	5	3	3	0	0	0	1	0	0	2



Les noms des classes d’occupation doivent être identiques à ceux présent dans le fichier d’identification des classes ([occupation_sol.csv](#occupation-sol-csv)).

Les valeurs sont interpolées linéairement entre les dates fournies dans les fichiers.

Les noms des fichiers doivent être :   ind_fol.< année (aaaa)>  (ex : ind_fol.1995). Il doit y avoir un fichier pour chaque année dont on veut spécifier les valeurs. Il est également possible d’utiliser l’extension de fichier «.def» ([ind_fol.def](#ind-fol-def)) pour spécifier des valeurs par défaut à être utilisé pour toutes les années (ou les années non spécifié).

Les fichiers doivent obligatoirement terminer avec le jour 365 (dernière ligne du fichier).

<a id="lacs-shp-lacs-prj-lacs-dbf-lacs-shx"></a>
## lacs.shp, lacs.prj, lacs.dbf, lacs.shx   (/physitel)

Carte vectorielle (Shapefile) des lacs.

<a id="lecture-acheminement-csv"></a>
## lecture_acheminement.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de calcul de l’acheminement en rivière. Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE ACHEMINEMENT RIVIERE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER DEBIT AMONT;»\{FichierDebitAmont\} |
| Ligne 6 | «NOM FICHIER DEBIT AVAL;»\{FichierDebitAval\} |

FichierDebitAmont : Chemin du fichier pour le débit amont (m3/s). Ce fichier doit avoir le même format que le fichier de sortie «resultat/debit_amont.csv».

FichierDebitAval : Chemin du fichier pour le débit aval (m3/s). Ce fichier doit avoir le même format que le fichier de sortie «resultat/debit_aval.csv».



Les chemins spécifiés pour les fichiers peuvent être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="lecture-bilan-vertical-csv"></a>
## lecture_bilan_vertical.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de calcul des bilans verticaux. Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE BILAN VERTICAL» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER PRODUCTION BASE;»\{FichierProdBase\} |
| Ligne 6 | «NOM FICHIER PRODUCTION HYPO;»\{FichierProdHypo\} |
| Ligne 7 | «NOM FICHIER PRODUCTION SURF;»\{FichierProdSurf\} |

FichierProdBase : Chemin du fichier pour la lame d’eau produite par la 3ième couche (base) (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/production_base.csv».

FichierProdHypo : Chemin du fichier pour la lame d’eau produite par la 2ième couche (hypodermique) (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/production_hypo.csv».

FichierProdSurf : Chemin du fichier pour la lame d’eau produite à la surface (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/production_surf.csv».



Les chemins spécifiés pour les fichiers peuvent être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="lecture-etp-csv"></a>
## lecture_etp.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de calcul de l’évapotranspiration potentielle. Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE EVAPOTRANSPIRATION» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER ETP;»\{FichierEtp\} |

FichierEtp : Chemin du fichier pour l’évapotranspiration potentielle (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/etp.csv». Le chemin spécifié peut être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="lecture-fonte-glacier-csv"></a>
## lecture_fonte_glacier.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de calcul de la fonte de la glace. Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE FONTE GLACIER» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER APPORT GLACIER;»\{FichierApportGlace\} |

FichierApportGlace : Chemin du fichier pour l’apport de la fonte de la glace (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/glacier-apport.csv». Le chemin spécifié peut être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="lecture-fonte-neige-csv"></a>
## lecture_fonte_neige.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de calcul de la fonte de la neige. Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE FONTE NEIGE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER APPORT;»\{FichierApport\} |
| Ligne 6 | «NOM FICHIER HAUTEUR COUVERT NIVAL;»\{FichierHauteurCouvert\} |

FichierApport : Chemin du fichier pour les apports (fonte neige + pluie) (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/apport.csv».

FichierHauteurCouvert : Chemin du fichier pour la hauteur du couvert nival (m). Ce fichier doit avoir le même format que le fichier de sortie «resultat/hauteur_neige.csv».



Les chemins spécifiés pour les fichiers peuvent être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="lecture-interpolation-csv"></a>
## lecture_interpolation.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de l’interpolation des données météorologiques. Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE INTERPOLATION DONNEES» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER TMIN;»\{FichierTMin\} |
| Ligne 6 | «NOM FICHIER TMAX;»\{FichierTMax\} |
| Ligne 7 | «NOM FICHIER TMIN_JOUR;»\{FichierTMinJour\} |
| Ligne 8 | «NOM FICHIER TMAX_JOUR;»\{FichierTMaxJour\} |
| Ligne 9 | «NOM FICHIER PLUIE;»\{FichierPluie\} |
| Ligne 10 | «NOM FICHIER NEIGE;»\{FichierNeige\} |

FichierTMin : Chemin du fichier pour la température minimum (°C). Ce fichier doit avoir le même format que le fichier de sortie «resultat/tmin.csv».

FichierTMax : Chemin du fichier pour la température maximum (°C). Ce fichier doit avoir le même format que le fichier de sortie «resultat/tmax.csv».

FichierTMinJour : Chemin du fichier pour la température minimum journalière (°C). Ce fichier doit avoir le même format que le fichier de sortie «resultat/tmin_jour.csv».

FichierTMaxJour : Chemin du fichier pour la température maximum journalière (°C). Ce fichier doit avoir le même format que le fichier de sortie «resultat/tmax_jour.csv».

FichierPluie : Chemin du fichier pour les précipitations (pluie) (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/pluie.csv».

FichierNeige : Chemin du fichier pour les précipitations (neige) (EEN) (mm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/neige.csv».



Les chemins spécifiés pour les fichiers peuvent être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="lecture-ruisselement-csv"></a>
## lecture_ruisselement.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de calcul du ruissellement de surface. Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE RUISSELEMENT SURFACE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER APPORT LATERAL;»\{FichierApportLateral\} |

FichierApportLateral : Chemin du fichier pour les apports latéraux (m3/s). Ce fichier doit avoir le même format que le fichier de sortie «resultat/apport_lateral.csv». Le chemin spécifié peut être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="lecture-tempsol-csv"></a>
## lecture_tempsol.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres du mode lecture pour le sous-modèle de calcul de la température du sol (profondeur du gel). Le mode lecture permet de remplacer les données de sortie d’un sous-modèle en fournissant directement ces données à l’aide d’un fichier. Le sous-modèle n’est alors pas simulé (exécuté).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LECTURE TEMPERATURE DU SOL» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «NOM FICHIER TEMPSOL;»\{FichierTempSol\} |

FichierTempSol : Chemin du fichier pour la profondeur du gel (cm). Ce fichier doit avoir le même format que le fichier de sortie «resultat/profondeur_gel.csv». Le chemin spécifié peut être relatif au dossier du projet (ex : «modelecture/fichier_de_donnees.csv»).

<a id="linacre-csv"></a>
## linacre.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul LINACRE (évapotranspiration potentielle).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; LINACRE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{TempMoisFroid\} \{TempMoisChaud\} \{Albedo\} \{CoefOptimisation\} |

IdUhrh : Identifiant de l’UHRH.

TempMoisFroid: Température moyenne du mois le plus froid (°C).

TempMoisChaud: Température moyenne du mois le plus chaud (°C).

Albedo : Albédo (surface de référence) (0-1).

CoefOptimisation : Coefficient multiplicatif d’optimisation.

<a id="milieux-humides-isoles-csv"></a>
## milieux_humides_isoles.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres des milieux humides isolés.

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{IdUhrh\} \{SuperficieUhrh\} \{SuperficieMaxMH\} \{FractionUhrhDraineeMH\} \{FractionSuperficieMax\} \{HauteurEauNormale\} \{HauteurEauMax\} \{KsatBase\} \{CoeffETP\} \{RatioVolumesEauSortant\} \{SauvegardeEtats\} |

IdUhrh : Identifiant de l’UHRH.

SuperficieUhrh (uhrh_a) : Superficie de l’UHRH (km2).

SuperficieMaxMH (wet_a) : Superficie maximale du milieu humide équivalent (km²).

FractionUhrhDraineeMH (wet_dra_fr) : Fraction de l’UHRH drainée par le milieu humide équivalent (0-1).

FractionSuperficieMax (frac) : Fraction de la superficie maximale afin de déterminer la superficie normale (0-1).

HauteurEauNormale (wetdnor) : Hauteur d’eau normale (m).

HauteurEauMax (wetdmax) : Hauteur d’eau maximale (m).

KsatBase (ksat_bs) : Conductivité hydraulique à saturation à la base du MHE (mm/h).

CoeffETP (c_ev) : Coefficient de l’évapotranspiration potentielle (0-1).

RatioVolumesEauSortant (c_prod) : Ratio dans le calcul des volumes d’eau sortant du MHE (0-1).

SauvegardeEtats : Sauvegarde des variables d’états des milieux humides équivalents (MHE). Valeur «0» : ne pas sauvegarder. Valeur «1» : sauvegarder. La sauvegarde des données est effectuée dans le fichier de résultat [wetland_isole.csv](#wetland-isole-csv).

<a id="milieux-humides-riverains-csv"></a>
## milieux_humides_riverains.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres des milieux humides riverains.

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{IdTroncon\} \{SuperficieUhrh\} \{SuperficieMaxMH\} \{FractionUhrhDraineeTronconAmont\} \{FractionUhrhDraineeMH\} \{FractionUhrhDraineeTronconAval\} \{LongueurMH\} \{LongueurTronconAmont\} \{LongueurTronconAval\} \{HauteurEauNormale\} \{HauteurEauMax\} \{FractionSuperficieMax\} \{KsatBerge\} \{KsatBase\} \{EpaisseurAquiferePotentielle\} \{SauvegardeEtats\} |

IdTroncon : Identifiant du tronçon.

SuperficieUhrh (uhrh_a) : Superficie de l’UHRH (km2).

SuperficieMaxMH (wet_a) : Superficie maximale du milieu humide équivalent (km²).

FractionUhrhDraineeTronconAmont (wetaup_fr) : Fraction des UHRH drainée par le tronçon en amont du milieu humide riverain équivalent (0-1).

FractionUhrhDraineeMH (wetadra_fr) : Fraction des UHRH drainée par le milieu humide riverain équivalent (0-1).

FractionUhrhDraineeTronconAval (wetadown_fr) : Fraction des UHRH drainée par le tronçon en aval du milieu humide riverain équivalent (0-1).

LongueurMH (longueur) : Longueur du milieu humide riverain équivalent (m).

LongueurTronconAmont (longueur amont) : Longueur du segment de tronçon situé en amont du milieu humide riverain équivalent (m).

LongueurTronconAval (longueur aval) : Longueur du segment de tronçon situé en aval du milieu humide riverain équivalent (m).

HauteurEauNormale (wetdnor) : Hauteur d’eau normale (m).

HauteurEauMax (wetdmax) : Hauteur d’eau maximale (m).

FractionSuperficieMax (frac) : Fraction de la superficie maximale afin de déterminer la superficie normale (0-1).

KsatBerge (ksat_bk) : Conductivité hydraulique à saturation de la berge du MHE (mm/h).

KsatBase (ksat_bs) : Conductivité hydraulique à saturation à la base du MHE (mm/h).

EpaisseurAquiferePotentielle (th_aq) : Épaisseur de l’aquifère potentielle (m).

SauvegardeEtats : Sauvegarde des variables d’états des milieux humides équivalents (MHE). Valeur «0» : ne pas sauvegarder. Valeur «1» : sauvegarder. La sauvegarde des données est effectuée dans le fichier de résultat [wetland_riverain.csv](#wetland-riverain-csv)

<a id="moyennes-ponderees-troncon-idtroncon-csv"></a>
## moyennes-ponderees-troncon\{IdTroncon\}.csv   (/simulation/\{nom_simulation\}/resultat)

Fichier de résultat contenant les moyennes pondérées des températures, des précipitations, du couvert nival, de l’évapotranspiration potentielle et de l’évapotranspiration réelle pour le tronçon spécifié à la fin du nom du fichier (IdTroncon). Les moyennes sont calculées à partir des résultats et pondérées selon les superficies des UHRH en amont du tronçon. Ce fichier peut être généré en configurant la ligne «TRONCONS_MOYENNES_PONDEREES;» du fichier [output.csv](#output-csv).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «Moyennes pondérées;( VERSION 4.3.0.0000 )» |
| Ligne 2 | «Troncon;\{IdTroncon\}» |
| Ligne 3 | «UHRH amont;1;2;3;4;5;6;7;8;9;10» |
| Ligne 4 | Commentaire |
| Ligne 5 et suivantes | \{DateHeure\} \{TMin (°C)\} \{TMax (°C)\} \{TMoy °C\} \{Pluie (mm)\} \{Neige (EEN) (mm)\} \{CouvertNival (EEN) (mm)\} \{ETP (mm)\} \{ETR (mm)\} |

<a id="moyenne-3-stations-csv"></a>
## moyenne_3_stations.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul MOYENNE 3 STATIONS (interpolation des données météorologiques).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; MOYENNE 3 STATIONS» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «GRADIENT TEMPERATURE STATION(C/100m);»\{GradientStationTemp\} |
| Ligne 6 | «GRADIENT PRECIPITATION STATION(mm/100m);»\{GradientStationPrecip\} |
| Ligne 7 | Ligne vide |
| Ligne 8 | Commentaire |
| Ligne 9 et suivantes | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

GradientStationTemp : Gradient vertical de la température pour l’interpolation des données manquantes aux stations (°C/100m).

GradientStationPrecip : Gradient vertical des précipitations pour l’interpolation des données manquantes aux stations (°C/100m).

IdUhrh : Identifiant de l’UHRH.

GradientTemp: Gradient vertical de la température (°C/100m).

GradientPrecip: Gradient vertical des précipitations (mm/100m).

PassagePluieNeige : Température de passage de la pluie en neige (°C).

<a id="neige-pgn"></a>
## neige.pgn   (/meteo)

Contient les pondérations des points de grille/UHRH pour la mise à jour de la neige par grille. Les pondérations sont calculées par HYDROTEL et ce fichier est généré automatiquement lorsqu’il est inexistant.

<a id="neige-2003-01-22-24h-een"></a>
## neige_2003_01_22_24h.een   (/neige-grille/donnees)

Grille de données (carte matricielle) contenant les valeurs d’équivalent en eau du couvert nival pour la mise à jour de la neige par grille. L’unité peut être en mètre, en centimètre ou en millimètre et est spécifié dans le fichier de configuration de la mise à jour de la neige par grille ([grilleneige.grn](#grilleneige-grn)).

<a id="neige-2003-01-22-24h-hau"></a>
## neige_2003_01_22_24h.hau   (/neige-grille/donnees)

Grille de données (carte matricielle) contenant les valeurs d’hauteur du couvert nival pour la mise à jour de la neige par grille. L’unité peut être en mètre, en centimètre ou en millimètre et est spécifié dans le fichier de configuration de la mise à jour de la neige par grille ([grilleneige.grn](#grilleneige-grn)).

<a id="neige01-nei"></a>
## NEIGE01.nei   (/neige)

Observations nivométriques pour la station «NEIGE01».

| Ligne                        | Description                                                  |
| --- | --- |
| Pour chaque ligne du fichier | \{Date\} \{Hauteur neige\} \{Équivalent en eau de la neige\} \{Densité\} |

Date : (JJ/MM/AAAA).

Hauteur neige : Hauteur du couvert nival (cm).

Équivalent en eau de la neige : (mm).

Densité : Densité relative (g/cm3).



La valeur pour la hauteur du couvert nival peut être fixé à -999 lorsqu’elle est inconnue. Dans ce cas, la hauteur est calculé avec l'équivalent en eau observé multiplié par la densité simulée (hauteur avant mise à jour / stock en eau avant mise à jour). Dans le cas où le stock en eau avant mise à jour est à 0 (pas de couvert nival), la hauteur est alors calculé ainsi : hauteur = équivalent en eau observé * 2.5.

<a id="noeuds-nds"></a>
## nœuds.nds   (/physitel)

Fichier d’information sur les nœuds (jonctions) du réseau hydrologique rastérisé.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type\} |
| Ligne 2 | \{NbNoeuds\} |
| Ligne 3 | Commentaire |
| Ligne 4 et suivantes | \{IdNoeud\} \{CoordX\} \{CoordY\} \{Altitude\} \{Largeur\} |

Type : Fixé à «1» (non utilisé).

NbNoeuds : Nombre de noeuds.

IdNoeud : Identifiant du noeud.

CoordX : Coordonnée X du nœud. La coordonnée doit être dans le même système de projection que la carte des altitudes ([altitude.tif](#altitude-tif)).

CoordY : Coordonnée Y du nœud. La coordonnée doit être dans le même système de projection que la carte des altitudes ([altitude.tif](#altitude-tif)).

Altitude : Altitude (élévation) correspondant à la position du nœud (m).

Largeur : Fixé à «0» (non utilisé).

<a id="obs-sim-flows-csv"></a>
## obs-sim-flows.csv   (/simulation/\{nom_simulation\}/resultat)

Fichier de résultats contenant les débits observées et simulées pour les tronçons configurés pour le calcul des statistiques (selon le fichier [stats.txt](#stats-txt)). Ce fichier est généré seulement lorsque le calcul des statistiques est effectué. Il permet de simplifier la lecture des résultats par un programme externe afin d’effectuer une calibration automatique des paramètres de simulation. Les résultats des pairs «Tronçon/Station» présent dans le fichier [stats.txt](#stats-txt) sont sauvegardés dans le fichier [obs-sim-flows.csv](#obs-sim-flows-csv) et présentés en colonne selon l’ordre dont ils apparaissent dans le fichier [stats.txt](#stats-txt) tel qu’illustré dans le tableau suivant :

| Colonne 1 | Colonne 2 | Colonne 3 | Colonne 4 | Colonne 5 | etc… |
| --- | --- | --- | --- | --- | --- |
| Date/Heure du pas de temps de la simulation | Débit observé pour le tronçon de la 1ère ligne du fichier [stats.txt](#stats-txt) | Débit simulé pour le tronçon de la 1ère ligne du fichier [stats.txt](#stats-txt) | Débit observé pour le tronçon de la 2ème ligne du fichier [stats.txt](#stats-txt) | Débit simulé pour le tronçon de la 2ème ligne du fichier [stats.txt](#stats-txt) |  |

<a id="occupation-sol-cla"></a>
## occupation_sol.cla   (/physitel)

Contient le nombre de pixels (tuiles) pour chaque classe d’occupation du sol pour chaque UHRH.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type\} |
| Ligne 2 | \{NbClasseOccSol\} |
| Ligne 3 | uhrh "\{NomOccSol1\}" "\{NomOccSol2\}" "\{NomOccSol3\}" etc… |
| Ligne 4 et suivantes | \{IdUhrh\} \{NbPixelOccSol1\} \{NbPixelOccSol2\} \{NbPixelOccSol3\} etc… |

Type : Fixé à «1» (non utilisé).

NbClasseOccSol : Nombre de classes d’occupation du sol.

NomOccSol1 à NomOccSolX : Nom des classes d’occupation du sol. Les noms des classes doivent être séparées par le caractère «espace» et doivent être délimité avec le caractère double apostrophe ("). Les noms doivent également être identiques à ceux présent dans le fichier d’identification des classes ([occupation_sol.csv](#occupation-sol-csv)).

IdUhrh : Identifiant de l’UHRH.

NbPixelOccSol1 à NbPixelOccSolX: Nombre de pixels (tuiles) pour chaque classe d’occupation du sol pour l’UHRH en cours. Ces valeurs sont déterminées à partir de la carte matricielle des occupations du sol ([occupation_sol.tif](#occupation-sol-tif)).



Ex :   

1

10

uhrh "FORETS CONIFERES" "FORETS FEUILLUS" "FORETS MIXTES" "AGRICULTURE"

(suite ligne 3)  "URBAIN" "ROUTES" "MILIEUX OUVERTS" "EAU" "SOLS NUS"

(suite ligne 3)  "MILIEUX HUMIDES"

1 0 102 0 0 1111 280 118 119 0 24

2 0 579 0 0 883 125 52 210 0 120

3 0 0 0 0 0 0 2 1 0 0

etc…

<a id="occupation-sol-csv"></a>
## occupation_sol.csv   (/physitel)

Contient les noms des classes d’occupation du sol.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{NbOccSol\} |
| Ligne 2 et suivantes | \{NomOccsol\} |

NbOccSol : Nombre de classe d’occupation du sol.

NomOccSol : Nom de la classe d’occupation du sol.



Il doit y avoir une ligne pour chaque classe d’occupation du sol. L’ordre des classes doit respecter les identifiants de la carte matricielle d’occupation du sol ([occupation_sol.tif](#occupation-sol-tif)). Ex : le 1er nom de classe correspond à l’identifiant «1» de la carte, le 2e nom de classe correspond à l’identifiant «2» de la carte, et ainsi de suite.

<a id="occupation-sol-tif"></a>
## occupation_sol.tif   (/physitel)

Carte matricielle (GeoTIF) contenant la distribution des classes d’occupation du sol. Les identifiants doivent correspondre (en commençant par la valeur 1) à l’ordre du nom des classes identifiés dans le fichier [occupation_sol.csv](#occupation-sol-csv).

<a id="onde-cinematique-csv"></a>
## onde_cinematique.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul DEGRE JOUR MODIFIE (écoulement sur la partie terrestre du bassin) (ruissellement).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; ONDE CINEMATIQUE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «CLASSE INTEGRE FORETS;» \{ListOccSolForets\} |
| Ligne 6 | «CLASSE INTEGRE EAUX;» \{ListOccSolEaux\} |
| Ligne 7 | Ligne vide |
| Ligne 8 | «LAME;» \{Lame\} |
| Ligne 9 | Ligne vide |
| Ligne 10 | «NOM FICHIER HGM;» \{NomFichierHGM\} |
| Ligne 11 | Ligne vide |
| Ligne 12 | Commentaire |
| Ligne 13 et suivantes | \{IdUhrh\} \{ManningForets\} \{ManningEaux\} \{ManningAutres\} |

ListOccSolForets : Liste des identifiants (1 à x) des classes d’occupation du sol associées au milieu «Forêts». L’identifiant 1 correspond à la première classe d’occupation du sol du fichier [occupation_sol.cla](#occupation-sol-cla), l’identifiant 2 à la deuxième classe et ainsi de suite. Les identifiants doivent être séparé par le caractère point-virgule «;».

ListOccSolEaux : Liste des identifiants (1 à x) des classes d’occupation du sol associées au milieu «Eaux».

Lame : Lame de référence pour l’hydrogramme géomorphologique (m).

NomFichierHGM : Nom ou chemin du fichier de données de l’hydrogramme géomorphologique. Le chemin spécifié peut être relatif au dossier du projet (ex : «[hgm/hydrogramme.hgm](#hydrogramme-hgm)»).

IdUhrh : Identifiant de l’UHRH.

ManningForets : Coefficient de rugosité de Manning pour les milieux forestiers.

ManningEaux : Coefficient de rugosité de Manning pour le milieu «Eaux».

ManningAutres : Coefficient de rugosité de Manning pour les autres milieux (ne faisant pas partie du milieu Forets ni du milieu Eaux).

<a id="onde-cinematique-modifie-csv"></a>
## onde_cinematique_modifiee.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul ONDE CINEMATIQUE MODIFIEE (écoulement par le réseau hydrographique) (acheminement).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; ONDE CINEMATIQUE MODIFIEE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «METHODE_CALCUL_HAUTEUR;» \{MethodeCalculHauteur\} |
| Ligne 6 | Ligne vide |
| Ligne 7 | Commentaire |
| Ligne 8 et suivantes | \{IdTroncon\} \{CoefRugosite\} \{CoefLargeur\} |

MethodeCalculHauteur : Méthode de calcul de la hauteur d’eau des tronçons. Valeur possible de 1 à 3 : (1) Section rectangulaire, (2) Section trapézoïdal (Tiwari et al.) (2012), (3) Approche HAND (fichier débits/hauteurs).

IdTroncon : Identifiant du tronçon.

CoefRugosite : Coefficient d’optimisation de la rugosité.

CoefLargeur : Coefficient d’optimisation des largeurs des rivières.

<a id="orientation-tif"></a>
## orientation.tif   (/physitel)

Carte matricielle (GeoTIF) contenant les orientations d’écoulement. Les valeurs possibles d’écoulement sont les suivantes : 1 (Est), 2 (Nord-Est), 3 (Nord), 4 (Nord-Ouest), 5 (Ouest), 6 (Sud-Ouest), 7 (Sud), 8 (Sud-Est).

<a id="output-csv"></a>
## output.csv   (/simulation/\{nom_simulation\})

Permet d’activer/désactiver la sauvegarde des variables de simulation, la sélection des tronçons pour la sauvegarde ainsi que d’autres paramètres relatif aux variables de sortie.

Chaque ligne débute avec l’identifiant du paramètre dont on souhaite spécifier, suivi du séparateur «;» et d’une ou plusieurs valeurs (séparé également par le caractère «;»). Pour les variables de simulation, on inscrit la valeur «1» pour activer la sauvegarde et la valeur «0» pour la désactiver.

Les identifiants suivants peuvent être utilisés :

TRONCONS : Liste des tronçons dont les variables sont sauvegardées. Les identifiants des tronçons doivent être séparé par le caractère «;». Le mot-clé «tous» peut être spécifié dans la liste des tronçons afin de sauvegarder les variables pour tous les tronçons.

TRONCONS_MOYENNES_PONDEREES : Liste des tronçons pour la sauvegarde des moyennes pondérées des UHRH amont aux tronçons (TMin, TMax, TMoy, PrecipPluie, PrecipNeige, CouvertNival, ETP, ETR). Un fichier de résultat est sauvegardé pour chaque tronçon spécifié (moyennes-ponderees-troncon\{IdTroncon\}.csv).

OUTPUT_NETCDF : Fichiers de sortie au format NetCDF au lieu du format texte (.csv). 0 (Désactivé) (format texte), 1 (Activé) (format NetCDF).

SEPARATEUR : Permet de spécifier le séparateur de colonne à utiliser pour les fichiers de sortie texte (.csv). Lorsque non spécifié, le séparateur «;» est utilisé par défaut. Exemple pour utiliser le séparateur «,» : «SEPARATEUR;,».

FICHIERS_ETATS_SEPARATEUR : Permet de spécifier le séparateur de colonne à utiliser pour les fichiers d’états. Lorsque non spécifié, le séparateur «;» est utilisé par défaut. Exemple pour utiliser le séparateur «,» : «SEPARATEUR;,».

TMIN : Température minimum (°C).

TMAX : Température maximum (°C).

TMIN_JOUR : Température minimum journalière (°C).

TMAX_JOUR : Température maximum journalière (°C).

PLUIE : Précipitation pluie (mm).

NEIGE : Précipitation neige (équivalent en eau de la neige) (mm).

APPORT : Apport de la fonte de la neige et de la pluie (mm).

COUVERT_NIVAL : Équivalent en eau du couvert nival (mm).

HAUTEUR_NEIGE : Hauteur du couvert nival (m).

ALBEDO_NEIGE : Albédo de la neige (0-1).

APPORT_GLACIER : Apport de la fonte de glace (mm).

EAU_GLACIER : Équivalent en eau de la glace (m).

PROFONDEUR_GEL : Profondeur du gel au sol (cm).

ETP : Évapotranspiration potentielle (mm).

ETR1 : Évapotranspiration réelle de la couche 1 (mm).

ETR2 : Évapotranspiration réelle de la couche 2 (mm).

ETR3 : Évapotranspiration réelle de la couche 3 (mm).

ETR_TOTAL : Évapotranspiration réelle totale (mm).

PRODUCTION_BASE : Lame d’eau produite par la 3e couche (base) (production) (mm).

PRODUCTION_HYPO : Lame d’eau produite par la 2e couche (hypodermique) (production) (mm).

PRODUCTION_SURF : Lame d’eau produite par la 1e couche (surface) (production) (mm).

Q12 : Écoulement vertical de la couche 1 à 2 (mm).

Q23 : Écoulement vertical de la couche 2 à 3 (mm).

Q23_SOMME_ANNUELLE : Sommes annuelles (par UHRH) des écoulements verticaux de la couche 2 à 3 (mm).

QRECHARGE : Recharge souterraine (mm).

THETA1 : Teneur en eau de la couche 1 (0-1).

THETA2 : Teneur en eau de la couche2 (0-1).

THETA3 : Teneur en eau de la couche 3 (0-1).

APPORT_LATERAL : Apport latéraux des tronçons (m3/s).

APPORT_LATERAL_UHRH : Apport latéraux des UHRH (m3/s).

ECOULEMENT_SURF : Écoulement sur l’UHRH vers le réseau hydrographique (couche 1) (surface) (m3/s).

ECOULEMENT_HYPO : Écoulement sur l’UHRH vers le réseau hydrographique (couche 2) (hypodermique) (m3/s).

ECOULEMENT_BASE : Écoulement sur l’UHRH vers le réseau hydrographique (couche 3) (base) (m3/s).

DEBITS_AMONT : Débit en amont du tronçon (m3/s).

DEBITS_AVAL : Débit en aval du tronçon (m3/s).

HAUTEUR_AVAL : Hauteur d’eau en aval du tronçon (m).

DEBITS_AVAL_MOY7J_MIN : Débit moyen 7 jours minimum annuel et estival (m3/s).

TEMPERATURE_EAU : Température moyenne de l'eau (°C). 



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

Fichier de paramètres global. Ce fichier permet de regrouper tous les paramètres de tous les sous modèles de simulation en un seul fichier. Ce fichier contient les mêmes paramètres que les fichiers individuel à l’exception que ceux-ci sont spécifiés par groupe d’UHRH au lieu d’individuellement pour chaque UHRH.

<a id="penman-csv"></a>
## penman.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul PENMAN (évapotranspiration potentielle).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; PENMAN» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{HauteurMesureVent\} \{VitesseVent\} \{HauteurVegetation\} \{MethodeResistanceAero\} \{CoefOptimisation\} |

IdUhrh : Identifiant de l’UHRH.

HauteurMesureVent : Hauteur à laquelle la vitesse du vent est mesurée (m).

VitesseVent : Vitesse du vent à la hauteur Z (m/s).

HauteurVegetation: Hauteur de la végétation (surface de référence) (m).

MethodeResistanceAero : Relation (équation) pour le calcul de la résistance aérodynamique. Valeur «0» : relation empirique. Valeur «1» : relation à base physique.

CoefOptimisation : Coefficient multiplicatif d’optimisation.

<a id="penman-monteith-csv"></a>
## penman_monteith.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul PENMAN-MONTEITH (évapotranspiration potentielle).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; PENMAN-MONTEITH» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{HauteurMesureVent\} \{HauteurMesureHum\} \{VitesseVent\} \{HauteurVegetation\} \{ResistanceStomatale\} \{CoefOptimisation\} |

IdUhrh : Identifiant de l’UHRH.

HauteurMesureVent : Hauteur à laquelle la vitesse du vent est mesurée (m).

HauteurMesureHum : Hauteur à laquelle l’humidité est mesurée (m).

VitesseVent : Vitesse du vent à la hauteur Z (m/s).

HauteurVegetation: Hauteur de la végétation (surface de référence) (m).

ResistanceStomatale : Résistance stomatale (surface de référence) (s/m).

CoefOptimisation : Coefficient multiplicatif d’optimisation.

<a id="pente-tif"></a>
## pente.tif   (/physitel)

Carte matricielle (GeoTIF) contenant les pentes (‰) (pour mille).

<a id="physitelproject-txt"></a>
## physitelproject.txt   (/physitel)

Fichier provenant du projet PHYSITEL ayant servi à créer le projet HYDROTEL. Le fichier contient les paramètres des calculs qui ont été effectués dans le projet PHYSITEL. Ce fichier n’est pas utilisé par HYDROTEL et est présent à titre informatif seulement.

<a id="point-rdx"></a>
## point.rdx   (/physitel)

Contient les coordonnées des pixels (tuiles) pour chacun des tronçons. Ce fichier est généré à partir de la carte matricielle des tronçons.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{NbLigne\} \{NbColonne\} \{NbPixel\} |
| Ligne 2 et suivantes | \{Ligne\} \{Colonne\} \{IdTroncon\} |

NbLigne : Nombre de ligne de la matrice des tronçons.

NbColonne : Nombre de colonne de la matrice des tronçons.

NbPixel : Nombre de pixel (tuile) de la matrice des tronçons (excluant les valeurs «NoData»). Le nombre de pixel correspond également au nombre de ligne de donnée du fichier.

Ligne : Numéro de ligne du pixel (tuile) (0 à X). L’identifiant de la 1ère ligne est 0.

Colonne : Numéro de colonne du pixel (tuile) (0 à X). L’identifiant de la 1ère colonne est 0.

IdTroncon : Identifiant du tronçon correspondant au pixel (tuile).

<a id="priestlay-taylor-csv"></a>
## priestlay_taylor.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul PRIESTLAY-TAYLOR (évapotranspiration potentielle).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; PRIESTLAY-TAYLOR» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{CoefPropAlpha\} \{CoefOptimisation\} |

IdUhrh : Identifiant de l’UHRH.

CoefPropAlpha : Coefficient de proportionnalité alpha.

CoefOptimisation : Coefficient multiplicatif d’optimisation.

<a id="pro-rac-def"></a>
## pro_rac.def   (/physio)

Fichier de données portant sur les profondeurs racinaire.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Format\} |
| Ligne 2 | \{NbOccupation\} \{NbJour\} |
| Ligne 3 | Commentaire |
| Ligne 4 | Commentaire |
| Ligne 5 et suivantes | \{Jour\} \{ValeurOccSol1\} \{ValeurOccSol2\} \{ValeurOccSol3\} etc… |

Format : Fixé à «2» (non utilisé).

NbOccupation : Nombre de classes d’occupation du sol.

NbJour : Nombres de jour (lignes avec valeurs) spécifié dans le fichier.

Jour : Jour de début de l’application des paramètres (Jour julien).

ValeurOccSol1 à ValeurOccsolX : Valeurs de profondeur racinaire (m) pour le jour en cours pour chaque classe d’occupation du sol. Les valeurs sont séparées par le caractère «espace» ou «tabulation».



Ex :   

2

10 3

Profondeur racinaire (m)

Jour "FORETS CONIFERES" "FORETS FEUILLUS" "FORETS MIXTES" "AGRICULTURE"

(suite ligne 4)  "URBAIN" "ROUTES" "MILIEUX OUVERTS" "EAU" "SOLS NUS"

(suite ligne 4)  "MILIEUX HUMIDES"

1		1	1.5	1.25	0	0	0	0.5	0	0	0.75

210	1	1.5	1.25	0.8	0	0	0.5	0	0	0.75

365	1	1.5	1.25	0	0	0	0.5	0	0	0.75



Les noms des classes d’occupation doivent être identiques à ceux présent dans le fichier d’identification des classes ([occupation_sol.csv](#occupation-sol-csv)).

Les valeurs sont interpolées linéairement entre les dates fournies dans les fichiers.

Les noms des fichiers doivent être :   pro_rac.< année (aaaa)>  (ex : pro_rac.1995). Il doit y avoir un fichier pour chaque année dont on veut spécifier les valeurs. Il est également possible d’utiliser l’extension de fichier «.def» ([pro_rac.def](#pro-rac-def)) pour spécifier des valeurs par défaut à être utilisé pour toutes les années (ou les années non spécifié).

Les fichiers doivent obligatoirement terminer avec le jour 365 (dernière ligne du fichier).

<a id="proprietehydrolique-sol"></a>
## proprietehydrolique.sol   (/physitel)

Contient les propriétés hydrauliques des types de sol.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type\} |
| Ligne 2 | \{NbTypeSol\} \{NbVariable\} |
| Ligne 3 | Commentaire |
| Ligne 4 | Commentaire |
| Ligne 5 et suivantes | \{NomTypeSol\} \{thetas\} \{thetacc\} \{thetapf\} \{ks\} \{psis\} \{lambda\} \{alpha\} |

Type : Fixé à «3» (non utilisé).

NbTypeSol : Nombre de type de sol. Correspond au nombre de ligne de données du fichier.

NbVariable : Nombre de variable (fixé à 7).

NomTypeSol : Nom du type de sol.

thetas : Teneur en eau à saturation (m3m-3).

thetacc : Teneur en eau à la capacité au champ (m3m-3).

thetapf : Teneur en eau au point de flétrissement (m3m-3).

ks : Conductivité hydraulique à saturation (m/h).

psis : Potentiel matriciel à saturation (m).

lambda : Indice de distribution de la dimension des pores.

alpha : Exposant pour l’évaluation du coefficient d’assèchement (paramètre d'ajustement de la courbe de variation de kat et kas en fonction de la teneur en eau relative par rapport à la réserve utile).



Ex :   

3

2 7

Soil hydraulic properties classified by soil texture

texture thetas thetacc thetapf ks psis lambda alpha

sand 0.417000 0.091000 0.033000 0.210000 0.159800 0.694000 10.000000

loamy_sand 0.401000 0.125000 0.055000 0.061100 0.205800 0.553000 6.000000

<a id="rankinen-csv"></a>
## rankinen.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul RANKINEN (température du sol).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; RANKINEN» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «OUTPUT_TEMPERATURE_LIST_UHRH;» \{ListUhrhOutput\} |
| Ligne 6 | Ligne vide |
| Ligne 7 | «INTERVALLE PROFIL (m);» \{IntervalleProfil\} |
| Ligne 8 | «TEMP INI BASE PROFIL (C);» \{TempIniBase\} |
| Ligne 9 | «SEUIL GEL (C);» \{SeuilGel\} |
| Ligne 10 | «FS;» \{Fs\} |
| Ligne 11 | Ligne vide |
| Ligne 12 | Commentaire |
| Ligne 13 et suivantes | \{NomTypeSol\} \{Conductivite\} \{CapaciteSol\} \{CapaciteGel\} |

ListUhrhOutput : Liste des identifiants des UHRH dont on souhaite obtenir en sortie les détails des températures des couches. Un fichier de résultat est généré pour chacun des UHRH (tempsol_uhrh \{IdUhrh\}.csv). Les identifiants doivent être séparé par le caractère point-virgule «;».

IntervalleProfil : Intervalle du profil (m).

TempIniBase : Température initiale au fond du profil (°C).

SeuilGel : Seuil du gel (°C).

Fs : Paramètre FS.

NomTypeSol : Nom du type de sol. Les noms doivent être identiques aux noms présents dans le fichier des propriétés hydrauliques ([proprietehydrolique.sol](#proprietehydrolique-sol)). L’ordre dans lequel sont inscrits les types de sol doit également être respecté.

Conductivite : Conductivité thermique du sol gelé (KT) (W/m/C).

CapaciteSol : Capacité thermique spécifique du sol (CS) (J/m3/C).

CapaciteGel : Capacité thermique spécifique lie au gel/dégel (CIce) (J/m3/C).

<a id="rayonnement-net-csv"></a>
## rayonnement_net.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul RAYONNEMENT NET (rayonnement net à la surface).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; RAYONNEMENT NET» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{Albedo\} \{TransAtmoA\} \{TransAtmoB\} \{TransAtmoC\} \{EmissAtmoA\} \{EmissAtmoB\} \{EmissAtmoC\} \{EmissSurfA\} \{EmissSurfB\} |

IdUhrh : Identifiant de l’UHRH.

Albedo : Albédo (surface de référence).

TransAtmoA : Transmissivité atmosphérique (Coefficient A).

TransAtmoB : Transmissivité atmosphérique (Coefficient B).

TransAtmoC : Transmissivité atmosphérique (Coefficient C).

EmissAtmoA : Émissivité atmosphérique (Coefficient A).

EmissAtmoB : Émissivité atmosphérique (Coefficient B).

EmissAtmoC : Émissivité atmosphérique (Coefficient C).

EmissSurfA : Émissivité de la surface (Coefficient A).

EmissSurfB : Émissivité de la surface (Coefficient B).

<a id="rivieres-shp-rivieres-prj-rivieres-dbf-rivieres-shx"></a>
## rivieres.shp, rivieres.prj, rivieres.dbf, rivieres.shx   (/physitel)

Carte vectorielle (Shapefile) des rivières.

<a id="shreve-csv"></a>
## shreve.csv   (/physio)

Fichier contenant les numéros d’ordre de Shreve déterminés pour chaque tronçon. Ce fichier est généré automatiquement par HYDROTEL lorsqu’il est absent.

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{IdTroncon\} \{OrdreShreve\} |

IdTroncon : Identifiant du tronçon.

OrdreShreve : Numéro d’ordre de Shreve.

<a id="station-p3s"></a>
## station.p3s   (/meteo et /neige)

Contient les pondérations des stations (météorologiques ou nivométriques) aux UHRH pour le modèle «Moyenne 3 Stations». Les pondérations sont calculées par HYDROTEL et ce fichier est généré automatiquement lorsqu’il est inexistant.

<a id="station-pth"></a>

## station.pth   (/meteo et /neige)

Contient les pondérations des stations (météorologiques ou nivométriques) aux UHRH pour le modèle «Thiessen». Les pondérations sont calculées par HYDROTEL et ce fichier est généré automatiquement lorsqu’il est inexistant.

<a id="station-sth"></a>

## station.sth   (/hydro)

Stations hydrométriques.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type de coordonnée\} |
| Ligne 2 | \{Nb station\} |
| Ligne 3 | Commentaire |
| Ligne 4 et suivantes | \{ID Station\} \{CoordX\} \{CoordY\} \{Altitude\} \{Format des données\} \{@X\} \{Nom du dossier de données\} |

Type de coordonné : 1=Long/Lat WGS84, 2=Même système de coordonnées que le projet (selon la carte des altitudes «[altitude.tif](#altitude-tif)»).

Nb station : Nombre de station.

Commentaire : Ligne de commentaire.

ID Station : Identifiant de la station.

CoordX : Coordonnée X/Longitude de la station.

CoordY : Coordonnée Y/Latitude de la station.

Altitude : Altitude de la station (m).

Format des données : Cette valeur doit être fixée à «3».

@X : X doit être remplacé par le nombre de caractère du nom du dossier de données.

Nom du dossier de données : Nom du dossier contenant les fichiers de données (.hyd).



Ex : 

2

3

Hydrological stations

02MC036 531221 5018237 70.0 3 @5 hydro

02MC037 531230 5018215 65.0 3 @5 hydro

02MC038 531250 5018200 80.0 3 @5 hydro



Formats supportés pour les coordonnées de type long/lat (wgs84):

ddd.d				//decimal degree

ddmm.m (ou ddmm)		//degree, decimal minute

dddmm.m (ou dddmm)	//degree, decimal minute

ddmmss.s (ou ddmmss)	//degree, minute, decimal second

dddmmss.s (ou dddmmss)	//degree, minute, decimal second

Les décimales sont optionnelles, excepté pour le format `decimal degree` où le point décimal est obligatoire.

Pour la longitude, le signe négatif est appliqué par défaut afin de se situé dans l'hémisphère ouest (pour compatibilité). Si on veut utiliser une longitude dans l'hémisphère est, on doit spécifier le signe +.

<a id="station-stm"></a>
## station.stm   (/meteo)

Stations météorologiques.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type de coordonnée\} |
| Ligne 2 | \{Nb station\} |
| Ligne 3 | Commentaire |
| Ligne 4 et suivantes | \{ID Station\} \{CoordX\} \{CoordY\} \{Altitude\} \{Format des données\} \{@X\} \{Nom du dossier de données\} |

Type de coordonné : 1=Long/Lat WGS84, 2=Même système de coordonnées que le projet (selon la carte des altitudes).

Nb station : Nombre de station.

Commentaire : Ligne de commentaire.

ID Station : Identifiant de la station.

CoordX : Coordonnée X/Longitude de la station.

CoordY : Coordonnée Y/Latitude de la station.

Altitude : Altitude de la station (m).

Format des données : Cette valeur doit être fixée à «3».

@X : X doit être remplacé par le nombre de caractère du nom du dossier de données.

Nom du dossier de données : Nom du dossier contenant les fichiers de données (.met).



Ex : 

1

3

Weather stations

0000040 -74.26 45.11 50 3 @5 meteo

0000041 -74.20 45.18 30 3 @5 meteo

0000042 -74.13 45.25 80.5 3 @5 meteo



Formats supportés pour les coordonnées de type long/lat (wgs84):

ddd.d				//decimal degree

ddmm.m (ou ddmm)		//degree, decimal minute

dddmm.m (ou dddmm)	//degree, decimal minute

ddmmss.s (ou ddmmss)	//degree, minute, decimal second

dddmmss.s (ou dddmmss)	//degree, minute, decimal second

Les décimales sont optionnelles, excepté pour le format `decimal degree` où le point décimal est obligatoire.

Pour la longitude, le signe négatif est appliqué par défaut afin de se situé dans l'hémisphère ouest (pour compatibilité). Si on veut utiliser une longitude dans l'hémisphère est, on doit spécifier le signe +.

<a id="station-stn"></a>
## station.stn   (/neige)

Stations nivométriques.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type de coordonnée\} |
| Ligne 2 | \{Nb station\} |
| Ligne 3 | Commentaire |
| Ligne 4 et suivantes | \{ID Station\} \{CoordX\} \{CoordY\} \{Altitude\} \{Format des données\} \{@X\} \{Nom du dossier de données\} |

Type de coordonné : 1=Long/Lat WGS84, 2=Même système de coordonnées que le projet (selon la carte des altitudes).

Nb station : Nombre de station.

Commentaire : Ligne de commentaire.

ID Station : Identifiant de la station.

CoordX : Coordonnée X/Longitude de la station.

CoordY : Coordonnée Y/Latitude de la station.

Altitude : Altitude de la station (m).

Format des données : Cette valeur doit être fixée à «4».

@X : X doit être remplacé par le nombre de caractère du nom du dossier de données.

Nom du dossier de données : Nom du dossier contenant les fichiers de données (.nei).



Ex : 

1

3

Snow stations

NEIGE01 -74.26 45.11 50 4 @5 neige

NEIGE02 -74.20 45.18 30 4 @5 neige

NEIGE03 -74.13 45.25 80.5 4 @5 neige



Formats supportés pour les coordonnées de type long/lat (wgs84):

ddd.d				//decimal degree

ddmm.m (ou ddmm)		//degree, decimal minute

dddmm.m (ou dddmm)	//degree, decimal minute

ddmmss.s (ou ddmmss)	//degree, minute, decimal second

dddmmss.s (ou dddmmss)	//degree, minute, decimal second

Les décimales sont optionnelles, excepté pour le format `decimal degree` où le point décimal est obligatoire.

Pour la longitude, le signe négatif est appliqué par défaut afin de se situé dans l'hémisphère ouest (pour compatibilité). Si on veut utiliser une longitude dans l'hémisphère est, on doit spécifier le signe +.

<a id="station-troncon-p3s"></a>

## station-troncon.p3s   (/meteo)

Contient les pondérations des stations météorologiques aux tronçons pour le modèle «Moyenne 3 Stations». Les pondérations sont calculées par HYDROTEL et ce fichier est généré automatiquement lorsqu’il est inexistant. Ce fichier est utilisé par le modèle de calcul de la température de l'eau. 

<a id="station-troncon-pth"></a>

## station-troncon.pth   (/meteo)

Contient les pondérations des stations météorologiques aux tronçons pour le modèle «Thiessen». Les pondérations sont calculées par HYDROTEL et ce fichier est généré automatiquement lorsqu’il est inexistant. Ce fichier est utilisé par le modèle de calcul de la température de l'eau. 

<a id="stats-csv"></a>

## stats.csv   (/simulation/\{nom_simulation\}/resultat)

Fichier de résultat contenant les statistiques calculées en fin de simulation. Les indicateurs statistiques calculés pour la durée de la simulation sont suivis des débits observés et simulés pour chaque tronçon sélectionné et chaque pas de temps de la simulation. La sélection des tronçons et des stations hydrologique associées est effectué avec le fichier [stats.txt](#stats-txt).

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 à (NbTroncon+1) (une ligne pour chaque tronçon) | \{IdTroncon\} \{RCEQM\} \{Nash-Sutcliffe\} \{Biais relatif\} \{Biais absolue\} \{Coefficient de corrélation\} \{Kge original 2009\} \{Kge modifié 2012\} \{Coefficient de pointe\} \{Coefficient de volume\} \{Nash-Log\} \{Nash-M\} \{Écart quadratique moyen\} \{Somme observée\} \{Somme simulée\} \{Moyenne observée\} \{Moyenne simulée\} |
| Ligne (NbTroncon+2) | Ligne vide |
| Ligne (NbTroncon+3) | Commentaire (identifiants des tronçons et des stations hydrologiques correspondantes) |
| Ligne (NbTroncon+4) et suivantes (une ligne pour chaque pas de temps de la simulation) | \{Date/Heure\} \{Débit simulé du 1er tronçon\} \{Débit observé du 1er tronçon\} \{Débit simulé du 2ème tronçon\} \{Débit observé du 2ème tronçon\} \{etc…\} |

<a id="stats-txt"></a>
## stats.txt   (/simulation/\{nom_simulation\})

Informations sur l’association des tronçons/stations hydrologiques pour le calcul des statistiques à la fin de la simulation. Le calcul des statistiques est effectué seulement lorsque ce fichier est présent.

| Ligne                | Description                      |
| --- | --- |
| Ligne 1 et suivantes | \{IdTroncon\} \{IdStationHydro\} |

IdTroncon : Identifiant du tronçon.

IdStationHydro : Identifiant de la station hydrologique associée au tronçon. Le mot clé «absent» peut être utilisé lorsque le tronçon n’est pas associé à aucune station.



Ex : 

1 absent

91 02MC036

<a id="strahler-csv"></a>
## strahler.csv   (/physio)

Fichier contenant les numéros d’ordre de Strahler déterminés pour chaque tronçon. Ce fichier est généré automatiquement par HYDROTEL lorsqu’il est absent.

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{IdTroncon\} \{OrdreStrahler\} |

IdTroncon : Identifiant du tronçon.

OrdreStrahler : Numéro d’ordre de Strahler.

<a id="submodels-version-txt"></a>
## submodels-versions.txt   (/simulation/\{nom_simulation\})

Indique les numéros des versions à utiliser pour les sous-modèles. Lorsque le numéro de version pour un sous modèle n’est pas spécifié dans le fichier ou le fichier lui-même est inexistant, les plus récentes versions des sous-modèles sont utilisées automatiquement, excepté si le numéro de version inscrit dans le fichier de simulation est égal ou antérieur à 4.1.5. Dans ce cas, la version 1 des sous-modèles est utilisée. Ce fichier est créé automatiquement par HYDROTEL lorsqu’il est inexistant.

| Ligne         | Description                                                |
| --- | --- |
| THIESSEN;2 | Version à utiliser pour le sous-modèle THIESSEN. |
| MOY3STATION;2 | Version à utiliser pour le sous-modèle MOYENNE 3 STATIONS. |
| BV3C;2 | Version à utiliser pour le sous-modèle BV3C. |

<a id="temp-eau-cequeau-troncons-csv"></a>
## temp_eau_cequeau_troncons.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour les tronçons pour le sous-modèle de calcul TEMP EAU CEQUEAU (température de l’eau).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.4.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE;TEMP EAU CEQUEAU» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «Capacite thermique de l eau;» \{CapaciteThermiqueEau\} |
| Ligne 6 | «Albedo de la surface de l eau;» \{AlbedoEau\} |
| Ligne 7 | «Emissivite de l eau;» \{EmissiviteEau\} |
| Ligne 8 | «Chaleur latente de vaporisation de l eau;» \{ChaleurLatente\} |
| Ligne 9 | Ligne vide |
| Ligne 10 | Commentaire |
| Ligne 11 et suivantes | \{IdTroncon\} \{CoefSolaire\} \{CoefInfraRouge\} \{CoefEvapo\} \{CoefChaleur\} \{VitesseVent\} \{CoefATransmissiviteAtmo\} \{CoefBTransmissiviteAtmo\} \{CoefCTransmissiviteAtmo\} \{CoefAEmissiviteAtmo\} \{CoefBEmissiviteAtmo\} \{CoefCEmissiviteAtmo\} \{MoyenneAnnuelleTempAir\} |

CapaciteThermiqueEau : Capacité calorifique de l’eau (C) (4.187 MJ / m³ / °C).

AlbedoEau : Albédo (*a*) de la surface du plan d’eau (tronçon ou lac) (0-1) (valeur par défaut de 0.06).

EmissiviteEau : Émissivité de la surface de l’eau (*a*) (valeur par défaut de 0.95).

ChaleurLatente : Chaleur latente de vaporisation de l’eau (*H*) (2480 MJ / m³). 

IdTroncon : Identifiant du tronçon.

CoefSolaire : Coefficient de calibration lié au rayonnement net de courte longueur d’onde (solaire) (Cs). 

CoefInfraRouge : Coefficient de calibration lié au rayonnement net de grande longueur d’onde (infrarouge) (Ci).

CoefEvapo : Coefficient de calibration lié à l’évaporation de l’eau (Ce).

CoefChaleur : Coefficient de calibration lié au flux de chaleur sensible (Cc).

VitesseVent : Vitesse du vent (km/h) (U). 

CoefATransmissiviteAtmo, CoefBTransmissiviteAtmo, CoefCTransmissiviteAtmo : Coefficients de calage dans le calcul de la transmissivité atmosphérique du modèle de rayonnement d’HYDROTEL (voir documents théorie d’HYDROTEL). 

CoefAEmissiviteAtmo, CoefBEmissiviteAtmo, CoefCEmissiviteAtmo : Coefficients de calage dans le calcul de la pseudo-émissivité atmosphérique du modèle de rayonnement d’HYDROTEL (voir documents théorie d’HYDROTEL).

MoyenneAnnuelleTempAir : Température moyenne du bassin versant (°C) (valeur par défaut de 4). 

<a id="temp-eau-cequeau-zones-csv"></a>

## temp_eau_cequeau_zones.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour les UHRH pour le sous-modèle de calcul TEMP EAU CEQUEAU (température de l’eau).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.4.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE;TEMP EAU CEQUEAU» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{TemperatureBase\} \{CritereGel\} |

IdUhrh : Identifiant de l’UHRH.

TemperatureBase : Température moyenne du bassin versant (°C) (valeur par défaut de 4).

CritereGel : Critère de gel.

<a id="thiessen-csv"></a>

## thiessen.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul MOYENNE 3 STATIONS (interpolation des données météorologiques).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; THIESSEN» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «GRADIENT TEMPERATURE STATION(C/100m);»\{GradientStationTemp\} |
| Ligne 6 | «GRADIENT PRECIPITATION STATION(mm/100m);»\{GradientStationPrecip\} |
| Ligne 7 | Ligne vide |
| Ligne 8 | Commentaire |
| Ligne 9 et suivantes | \{IdUhrh\} \{GradientTemp\} \{GradientPrecip\} \{PassagePluieNeige\} |

GradientStationTemp : Gradient vertical de la température pour l’interpolation des données manquantes aux stations (°C/100m).

GradientStationPrecip : Gradient vertical des précipitations pour l’interpolation des données manquantes aux stations (°C/100m).

IdUhrh : Identifiant de l’UHRH.

GradientTemp: Gradient vertical de la température (°C/100m).

GradientPrecip: Gradient vertical des précipitations (mm/100m).

PassagePluieNeige : Température de passage de la pluie en neige (°C).

<a id="thornthwaite-csv"></a>
## thornthwaite.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul THORNTHWAITE (évapotranspiration potentielle).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; THORNTHWAITE» |
| Ligne 4 | Ligne vide |
| Ligne 5 | Commentaire |
| Ligne 6 et suivantes | \{IdUhrh\} \{IndiceThermique\} \{FacteurDephasage\} \{CoefOptimisation\} |

IdUhrh : Identifiant de l’UHRH.

IndiceThermique : Indice thermique annuel de Thornthwaite (1-100).

FacteurDephasage : Facteur de déphasage (décalage) (jours) (1-80).

CoefOptimisation : Coefficient multiplicatif d’optimisation.

<a id="thorsen-csv"></a>
## thorsen.csv   (/simulation/\{nom_simulation\})

Fichier des paramètres pour le sous-modèle de calcul THORSEN (température du sol).

| Ligne | Description |
| --- | --- |
| Ligne 1 | «PARAMETRES HYDROTEL VERSION;4.3.0.0000» |
| Ligne 2 | Ligne vide |
| Ligne 3 | «SOUS MODELE; THORSEN» |
| Ligne 4 | Ligne vide |
| Ligne 5 | «PROFONDEUR INITIALE DU GEL DANS LE SOL (m);» \{ProfondeurGelIni\} |
| Ligne 6 | «PARAMETRE EMPIRIQUE 1 (m-1);» \{ParamEmpirique\} |
| Ligne 7 | «TEMPERATURE DU GEL DE L'EAU DANS LE SOL (dC);» \{TempGelEau\} |
| Ligne 8 | «TENEUR EN EAU DISPONIBLE (INITIAL/PAR DEFAUT) (0:1);» \{TeneurEauDispo\} |
| Ligne 9 | Ligne vide |
| Ligne 10 | Commentaire |
| Ligne 11 et suivantes | \{NomTypeSol\} \{Conductivite\} |

ProfondeurGelIni : Profondeur initiale du gel dans le sol (m).

ParamEmpirique : Paramètre empirique 1 (m-1).

TempGelEau : Température du gel de l’eau dans le sol (°C).

TeneurEauDispo : Teneur en eau disponible (initial/par défaut) (0-1).

NomTypeSol : Nom du type de sol. Les noms doivent être identiques aux noms présents dans le fichier des propriétés hydrauliques ([proprietehydrolique.sol](#proprietehydrolique-sol)). L’ordre dans lequel sont inscrits les types de sol doit également être respecté.

Conductivite : Conductivité thermique du sol gelé (W/m/s).

<a id="troncon-trl"></a>
## troncon.trl   (/physitel)

Paramètres pour les tronçons.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Type\} |
| Ligne 2 | \{NbTroncon\} |
| Ligne 3 | Commentaire |
| Ligne 4 et suivantes | Si TypeTroncon est égal à 1 (rivière) :<br>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{IdNoeudAmont\} \{Longueur\} \{Largeur\} \{Manning\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\}<br/><br/>Si TypeTroncon est égal à 2 (lac) :<br/>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{NbNoeudAmont\} \{IdNoeudAmont1\} \{IdNoeudAmont2\} \{etc…\} \{Longueur\} \{Superficie\} \{CoefC\} \{CoefK\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\}<br/><br/>Si TypeTroncon est égal à 4 (lac sans laminage) :<br/>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{NbNoeudAmont\} \{IdNoeudAmont1\} \{IdNoeudAmont2\} \{etc…\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\}<br/><br/>Si TypeTroncon est égal à 5 (barrage avec historique):<br/>\{IdTroncon\} \{TypeTroncon\} \{IdNoeudAval\} \{NbNoeudAmont\} \{IdNoeudAmont1\} \{IdNoeudAmont2\} \{etc…\} \{IdStationHydro\} \{NbUhrhAmont\} \{IdUhrhAmont1\} \{IdUhrhAmont2\} \{etc…\} \{NoOrdreShreve\} |

Type : Indique si la dernière colonne des lignes de données représente le numéro d’ordre de Shreve (1 : no ordre de Shreve absent, 2 : no ordre de Shreve présent).

NbTroncon: Nombre de tronçon.

IdTroncon : Identifiant du tronçon.

TypeTroncon : Type du tronçon (1 : Rivière, 2 : Lac, 4 : Lac sans laminage, 5 : Barrage avec historique).

IdNoeudAval : Identifiant du nœud (point) aval du tronçon.

IdNoeudAmont : Identifiant du nœud (point) amont du tronçon.

Longueur : Longueur du lac ou du tronçon de rivière (m).

Largeur : Largeur du tronçon de rivière (m).

Manning : Coefficient de manning pour le tronçon de rivière. Valeur par défaut : 0.04.

NbUhrhAmont : Nombre d’UHRH se déversant dans le tronçon.

IdUhrhAmont1 à IdUhrhAmontX : Identifiant des UHRH se déversant dans le tronçon.

NoOrdreShreve : Numéro d’ordre de shreve pour le tronçon.

NbNoeudAmont : Nombre de nœud (point) amont du tronçon.

IdNoeudAmont1 à IdNoeudAmontX : Identifiant des nœuds (points) amont du tronçon.

Superficie : Superficie du lac (km2).

CoefC : Facteur multiplicatif (Coefficient C).

CoefK : Exposant (K).

IdStationHydro : Identifiant de la station hydrologique associée au tronçon.



Ex : 

2

4

TRONCONS

1 1 1 2 2027.7 19.9 0.04 2 1 2 75

2 1 2 3 20.0 19.85 0.04 1 3 74

3 1 3 4 241.42 0.22 0.04 3 4 5 6 1

4 2 151 2 152 153 72768.13 0.3088 6.4521 1.5 4 356 357 358 359 7

<a id="troncon-width-depth-csv"></a>
## troncon_width_depth.csv   (/physio)

Fichier contenant les paramètres des tronçons utilisés lors de la simulation des milieux humides riverains. Ce fichier est généré automatiquement par PHYSITEL lors de la création du projet HYDROTEL.

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{ID\} \{Superficie\} \{Width PHYSITEL\} \{Width SWAT\} \{Depth SWAT\} |

ID : Identifiant du tronçon.

Superficie : Superficie amont drainée par le tronçon (km2) (non utilisé).

Width PHYSITEL : Largeur du tronçon calculée selon la méthode PHYSITEL (m) (non utilisé).

Width SWAT : Largeur du tronçon calculée selon la méthode SWAT (m) (non utilisé).

Depth SWAT : Profondeur du tronçon calculée selon la méthode SWAT (m).

<a id="troncons-tif"></a>
## troncons.tif   (/physitel)

Carte matricielle (GeoTIFF) des tronçons. Cette carte n’est pas utilisé par HYDROTEL et est présente à titre informatif seulement.

<a id="troncons-txt"></a>
## troncons.txt   (/physitel)

Fichier d’information pour les tronçons (réseau hydrographique rastérisé). Ce fichier est généré par PHYSITEL lors de la création du projet HYDROTEL. Le fichier n’est pas utilisé par HYDROTEL et est présent à titre informatif seulement.

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{NbTroncon\} |
| Ligne 2 et suivantes (pour chaque tronçon) | Ligne vide |
| (ligne 3) | \{Type\} |
| (ligne 4) | \{NoeudAvalY\} \{NoeudAvalX\} |
| (ligne 5) | \{NbNoeudAmont\} |
| (ligne 6) | \{NoeudAmontY\} \{NoeudAmontX\} (premier nœud) |
| (ligne 7) | \{NoeudAmontY\} \{NoeudAmontX\} (deuxième nœud) |
|  | (coordonnées des autres nœuds amont) (une ligne par nœud) |
| (ligne 8) | \{NbCell\} |
| (ligne 9) | \{CellY\} \{CellX\} (première cellule) |
| (ligne 10) | \{CellY\} \{CellX\} (deuxième cellule) |
|  | (coordonnées des autres cellules) (une ligne par cellule) |
| (ligne 11) | Ligne vide |
| (ligne 12) | … |

NbTroncon : Nombre de tronçon.

Type : Type du tronçon (1 : Rivière, 2 : Lac).

NoeudAvalY : Coordonnée Y du nœud aval du tronçon. La coordonnée Y est le numéro de ligne (de 0 à x) de la cellule de la carte matricielle des tronçons ([troncons.tif](#troncons-tif)).

NoeudAvalX : Coordonnée X du nœud aval du tronçon. La coordonnée X est le numéro de colonne (de 0 à x) de la cellule de la carte matricielle des tronçons ([troncons.tif](#troncons-tif)).

NbNoeudAmont : Nombre de nœud (point de jonction) amont.

NoeudAmontY : Coordonnée Y du nœud amont au tronçon. La coordonnée Y est le numéro de ligne (de 0 à x) de la cellule de la carte matricielle des tronçons ([troncons.tif](#troncons-tif)).

NoeudAmontX : Coordonnée X du nœud amont au tronçon. La coordonnée X est le numéro de colonne (de 0 à x) de la cellule de la carte matricielle des tronçons ([troncons.tif](#troncons-tif)).

NbCell : Nombre de cellule (tuile) appartenant au tronçon (selon la carte matricielle des tronçons ([troncons.tif](#troncons-tif)).

CellY : Coordonnée Y de la cellule (tuile) appartenant au tronçon. La coordonnée Y est le numéro de ligne (de 0 à x) de la carte matricielle des tronçons ([troncons.tif](#troncons-tif)).

CellX : Coordonnée X de la cellule (tuile) appartenant au tronçon. La coordonnée X est le numéro de colonne (de 0 à x) de la carte matricielle des tronçons ([troncons.tif](#troncons-tif)).

<a id="type-sol-cla"></a>
## type_sol.cla   (/physitel)

Distribution spatiale des types de sol (type de sol attribué à chaque UHRH).

| Ligne | Description |
| --- | --- |
| Ligne 1 | \{Format\} |
| Ligne 2 | \{IdUhrh\} \{IdTypeSol\} |

Format : Fixé à «1» (non utilisé).

IdUhrh : Identifiant de l’UHRH.

IdTypeSol : Identifiant du type de sol attribué à l’UHRH. L’identifiant du type de sol doit correspondre à l’ordre des types de sol présent dans le fichier [proprietehydrolique.sol](#proprietehydrolique-sol) en commençant par l’identifiant 0. Ex : le premier type de sol du fichier [proprietehydrolique.sol](#proprietehydrolique-sol) est représenté par la valeur «0» dans le fichier [type_sol.cla](#type-sol-cla), le deuxième type de sol du fichier [proprietehydrolique.sol](#proprietehydrolique-sol) est représenté par la valeur «1» dans le fichier [type_sol.cla](#type-sol-cla), et ainsi de suite.



Ce fichier est créé automatiquement par HYDROTEL lorsqu’il est absent à partir des données de la carte [type_sol.tif](#type-sol-tif). Le type de sol retenu est le type de sol prépondérant sur l’UHRH.

<a id="type-sol-tif"></a>
## type_sol.tif   (/physitel)

Carte matricielle (GeoTIFF) des types de sol. Les valeurs de la carte doivent correspondre aux numéros (index) des types de sol définis dans le fichier [proprietehydrolique.sol](#proprietehydrolique-sol) en commençant par la valeur «1». Ex : la valeur 1 sur la carte représente le premier type de sol défini dans le fichier [proprietehydrolique.sol](#proprietehydrolique-sol), la valeur 2 sur la carte représente le deuxième type de sol défini dans le fichier [proprietehydrolique.sol](#proprietehydrolique-sol), et ainsi de suite.

<a id="uhrh-csv"></a>
## uhrh.csv   (/physitel)

Propriétés des UHRH.

| Ligne | Description |
| --- | --- |
| Ligne 1 | "RESUMER ZONES HYDROTEL VERSION;4.3.0.0000" |
| Ligne 2 | Ligne vide |
| Ligne 3 | Commentaire |
| Ligne 4 et suivantes | \{IdUhrh\} \{Type\} \{AltitudeMoyenne\} \{PenteMoyenne\} \{OrientationMoyenne\} \{NbPixel\} \{Superficie\} \{CentroidLon\} \{CentroidLat\} |

IdUhrh : Identifiant de l’UHRH.

Type : Type de l’UHRH. Indique si un UHRH est un lac (valeur «LAC») ou un sous-bassin (valeur «SOUS-BASSIN».

AltitudeMoyenne : Altitude moyenne de l’UHRH (m).

PenteMoyenne : Pente moyenne de l’UHRH (ratio).

OrientationMoyenne : Orientation moyenne de l’UHRH. Valeur possible : 1 (Est), 2 (Nord-Est), 3 (Nord), 4 (Nord-Ouest), 5 (Ouest), 6 (Sud-Ouest), 7 (Sud), 8 (Sud-Est).

NbPixel : Nombre de pixels (tuiles) appartenant à l’UHRH.

Superficie : Superficie de l’UHRH (km2).

CentroidLon : Coordonnée longitude du point centroïde de l’UHRH (WGS84) (dd).

CentroidLat : Coordonnée latitude du point centroïde de l’UHRH (WGS84) (dd).



Ce fichier est créé automatiquement par HYDROTEL lorsqu’il est absent à partir des données des cartes [uhrh.tif](#uhrh-tif), [altitude.tif](#altitude-tif), [pente.tif](#pente-tif) et [orientation.tif](#orientation-tif).

<a id="uhrh-shp-uhrh-prj-uhrh-dbf-uhrh-shx"></a>
## uhrh.shp, uhrh.prj, uhrh.dbf, uhrh.shx   (/physitel)

Carte vectorielle (Shapefile) des UHRH. Cette carte est utilisée seulement par la version interface d’HYDROTEL.

<a id="uhrh-tif"></a>
## uhrh.tif   (/physitel)

Carte matricielle (GeoTIF) des UHRH.

<a id="uhrh-txt"></a>
## uhrh.txt   (/physitel)

Fichier d’information pour les UHRH (Unités Hydrologiques Relativement Homogènes). Ce fichier est généré par PHYSITEL lors de la création du projet HYDROTEL. Le fichier n’est pas utilisé par HYDROTEL et est présent à titre informatif seulement.

<a id="wet-pixel-info-csv"></a>
## wet_pixel_info.csv   (/physio)

Fichier contenant les caractéristiques des milieux humides pour les UHRH. Ce fichier est généré automatiquement par PHYSITEL lors de la création du projet HYDROTEL. Ces données sont à titre informatif seulement (non utilisées par HYDROTEL).

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{UhrhId\} \{NbPixel\} \{NbPixelIsole\} \{NbPixelRiverain\} \{NbPixelDraineIsole\} \{NbPixelDraineRiverain\} \{NbPixelDraineCommun\} \{NbPixelIsoleDansRiverain\} |

UhrhId : Identifiant de l’UHRH.

NbPixel : Nb de pixels (tuiles) total de l’UHRH (basé sur la carte matricielle [uhrh.tif](#uhrh-tif)).

NbPixelIsole : Nb de pixel de type milieux humide isolé contenu dans l’UHRH.

NbPixelRiverain : Nb de pixel de type milieux humide riverain contenu dans l’UHRH.

NbPixelDraineIsole : Nb de pixel drainé uniquement par les milieux humide isolé contenu dans l’UHRH.

NbPixelDraineRiverain : Nb de pixel drainé uniquement par les milieux humide riverain contenu dans l’UHRH.

NbPixelDraineCommun : Nb de pixel drainé par les milieux humide isolé qui sont drainés par les milieux humide riverain contenu dans l’UHRH.

NbPixelIsoleDansRiverain : Nb de pixel de type milieux humide isolé qui sont drainés par les milieux humide riverain contenu dans l’UHRH.

<a id="wetland-isole-csv"></a>
## wetland_isole.csv   (/simulation/\{nom_simulation\}/resultat)

Fichier des résultats pour les milieux humides isolés (variables d’états). La sauvegarde de ces données et la sélection des tronçons est effectuée dans le fichier [milieux_humides_isoles.csv](#milieux-humides-isoles-csv).

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{IdUhrh\} \{Annee\} \{Mois\} \{Jour\} \{Heure\} \{Apport\} \{Evp\} \{WetSep\} \{WetVol\} \{WetFlwI\} \{WetFlwO\} \{WetProd\} |

IdUhrh : Identifiant de l’UHRH.

Annee : Pas de temps de la simulation (année).

Mois : Pas de temps de la simulation (mois).

Jour : Pas de temps de la simulation (jour).

Heure : Pas de temps de la simulation (heure).

Apport : Apport verticaux (pluie + fonte) sur le milieu humide (mm).

Evp : Évapotranspiration potentielle (mm).

WetSep : Volume d’eau s’écoulant à la base du milieu humide (m3).

WetVol : Volume d’eau dans le milieu humide (m3).

WetFlwI : Volume d’eau intercepté par le milieu humide en fonction de l’aire drainée par celui-ci (m3).

WetFlwO : Volume d’eau quittant le milieu humide à la surface (m3).

WetProd : Production totale du milieu humide (mm).

<a id="wetland-riverain-csv"></a>
## wetland_riverain.csv   (/simulation/\{nom_simulation\}/resultat)

Fichier des résultats pour les milieux humides riverains (variables d’états). La sauvegarde de ces données et la sélection des tronçons est effectuée dans le fichier [milieux_humides_riverains.csv](#milieux-humides-riverains-csv).

| Ligne | Description |
| --- | --- |
| Ligne 1 | Commentaire |
| Ligne 2 et suivantes | \{IdTroncon\} \{Annee\} \{Mois\} \{Jour\} \{Heure\} \{wet_v\} \{wet_a\} \{wet_d\} \{sur_q\} \{HauteurEau\} \{qd\} |

IdTroncon : Identifiant du tronçon.

Annee : Pas de temps de la simulation (année).

Mois : Pas de temps de la simulation (mois).

Jour : Pas de temps de la simulation (jour).

Heure : Pas de temps de la simulation (heure).

wet_v : Volume d’eau dans le milieu humide (m3).

wet_a : Superficie du milieu humide (m2).

wet_d : Hauteur d’eau dans le milieu humide (m).

sur_q : Représente le débit d’eau qui est dirigé vers (+) ou retiré (-) du tronçon de rivière avoisinant (m3/s).

HauteurEau : Hauteur d’eau dans le tronçon (m).

qd : Débit aval du tronçon (m3/s).

