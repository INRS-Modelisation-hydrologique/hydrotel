
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

#ifndef PRELEVEMENTS_SITE_H_INCLUDED
#define PRELEVEMENTS_SITE_H_INCLUDED


#include "prelevements_donnees.hpp"

#include <iostream>
#include <map>


namespace HYDROTEL
{

    class PRELEVEMENTS_SITE
    {
        public:

            PRELEVEMENTS_SITE(const int p_typeside, const int p_idsite, const double p_dlong, const double p_dlat, const int p_uhrh, const int p_troncon,
                              const int p_idoccsol, const std::string& p_soccsol, const int p_critere, const std::string& p_source, const double p_coef_consommation);

            //fonction qui sert à trouver 2 coordonnées identiques et retourner l'index à la coordonnée trouvée
            static void vector_indexfound(double d_lat, double d_long, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<int>& index);

            //static void CloseToCoord(const std::vector<double>& v_longPixel, const std::vector<double>& v_latPixel, const std::vector<double>& v_long2Pixel, const std::vector<double> v_lat2Pixel, std::vector<int>& close);

            void setGPESite(int p_siteGPE);

            double getLong();

            double getLat();

            void setPixelData(int p_pixelLong, int p_pixelLat);

            int getLongPixel();

            int getLatPixel();

            // sert à print les données identiques à chacuns des prélévements, le constructeur de la classe prélèvement
            std::string printDonneeMulti();

            // sert à imprimer le nom des colonnes des éléments du constructeur
            static std::string printLegendMulti();

            int getTypeSite();

            //------------------------------------------GPE------------------------------------------------------------------//

            // Ajoute des données GPE de consommation à un site GPE
            void ajouteDonneeGPE(int annee, int idmois, int nbrJour, double p_volume, double p_consomation);

            // print les données en GPE sous format string
            std::string printDonneGPE(size_t numero_site);

            // print le nom des colonnes
            static std::string printLegendGPE();

            // Ajoute des données GPE pour un site
            void setIntervenantCodeScianGPE(const std::string& p_intervenantGPE, const std::string& p_nomIntervenantGPE, const std::string& p_nomLegalGPE, const std::string& p_codeScianGPE);

            // Lire un fichier résultat GPE pour en sortir les sites de prélévements
            static std::vector<PRELEVEMENTS_SITE> lirePrelevementGPE(const std::string& filename);

            // Enleve les données qui ne font pas partie de la carte du projet
            static void erase_indexGPE(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<std::string>& v_intervenant, std::vector<std::string>& v_nomIntervenant, std::vector<std::string>& v_nomLegal, std::vector<int>& v_annee, std::vector<int>& v_mois, std::vector<int>& v_nbrjour,
                std::vector<double>& v_volume, std::vector<std::string>& v_source, std::vector<std::string>& v_codescian, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<double>& v_consomation, std::vector<int>& v_index);
            
            // Lire un fichier de prélévements GPE
           static bool importFichierPrelevementGPE(const std::vector<std::string>& filenames, std::vector<std::string>& v_intervenant, std::vector<std::string>& v_nomIntervenant, std::vector<std::string>& v_nomLegal, std::vector<int>& v_annee, std::vector<int>& v_mois, std::vector<int>& v_nbrjour,
                std::vector<double>& v_volume, std::vector<std::string>& v_source, std::vector<std::string>& v_codescian, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<double>& v_consomation,
                double long_min, double long_max, double lat_min, double lat_max);

           std::string getnomIntervenantGPE();

           std::string getnomLegalGPE();
           
           //-----------------------------------------------agricole----------------------------------------///
           // Ajoute les données agricoles au site de prélevement 
           void ajouteDonneeagricole(const std::string& p_nomExploitant, const std::string& p_nomlieu, const std::string& p_nomMunicipalite, const std::string& p_typeCheptel, const std::string& p_nbrTete,
                const std::string& p_consomation, double p_consomationTotal, int p_siteGPE, int p_sitePrelevement);

            std::string printDonneagricole(size_t numero_site);

            static std::string printLegendAgricole();
            
            // Lire les fichiers agricoles initiaux
            static bool importFichierPrelevementagricole(const std::vector<std::string>& filename, const std::string& filenameTable, std::vector<std::string>& v_nomExploitant, std::vector<std::string>& v_nomlieu, std::vector<std::string>& v_nomMunicipalite, std::vector<std::string>& v_typeCheptel, std::vector<std::string>& v_nbrTete,
                    std::vector<double>& v_listeconsommation, std::vector<std::string>& v_listeCheptel, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<std::string>& v_consomation, std::vector<double>& v_consomationTotal, double long_min, double long_max, double lat_min, double lat_max);

            // Lire la table de consommation en fonction du type de cheptel
            static bool importTableConsommationCheptel(const std::string& filename, std::vector<double>& v_listeConsomation, std::vector<std::string>& v_listeCheptel);

            // Supprime les données pour lesquelles les prélévements ne sont pas dans la carte
            static void erase_indexagricole(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<std::string>& v_nomExploitant, std::vector<std::string>& v_nomlieu, std::vector<std::string>& v_nomMunicipalite,
                std::vector<std::string>& v_typeCheptel, std::vector<std::string>& v_nbrTete, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<std::string>& v_consomation, std::vector<double>& v_consomationTotal, std::vector<int>& v_index);
       
            // Trouve le type de cheptel dans la phrase et l'associe à la table de consommation
            static int FindCheptel(std::string str_tofind, const std::vector<std::string>& v_listeCheptel);

            std::string getnomLieuagricole();

            std::string getnomExploitant();

            // Regarde s'il y a des sites de prélévement gpe qui s'apparante à des sites de prélevement agricole par le nom des données
            static void findCorrespondanceNom(std::vector<PRELEVEMENTS_SITE>& v_siteGPE, std::vector<PRELEVEMENTS_SITE>& v_siteAgricole);

            // Lire le fichier résultat des prélévement agricole
            static std::vector<PRELEVEMENTS_SITE> lirePrelevementAGRICOLE(const std::string& filename);

            // Réécrit le fichier résultat des prélévements agricoles
            static void RefreashFichieragricole(std::vector<PRELEVEMENTS_SITE>& v_siteAgricole, std::string repertoireProjet);


            //---------------------------------------------PRELEVEMENT EAU--------------------------------------------------//

            // Lire fichier prelevement eau
            static void lireFichierPrelevementEAU(const std::vector<std::string>& v_filename, std::vector<std::string>& v_site, std::vector<std::string>& v_siteGPE, std::vector<std::string>& v_cleCompos, std::vector<std::string>& v_DesCompo,
                std::vector<std::string>& v_nomLieu, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement,
                std::vector<std::string>& v_codeSCIAN, std::vector<double>& v_coef_consommation, std::vector<int>& v_continue, std::vector<int>& v_moisD, std::vector<int>& v_moisF, std::vector<std::string>& v_siteGPENom, std::vector<std::string>& v_siteCulture, std::vector<std::string>& v_siteElevage,
                std::vector<double>& v_lat, std::vector<double>& v_long, double long_min, double long_max, double lat_min, double lat_max);

            // Ajoute les données spécifiques au prélévement eau
            void ajoutDonneeEAU(const std::string& p_siteGPEEAU, const std::string& p_cleComposEAU, const std::string& p_DesCompoEAU, const std::string& p_nomLieuEAU, const std::string& p_typeEAU,
                double p_prelevementEAU, const std::string& p_codeSCIANEAU, int p_continueEAU, int p_moisDEAU, int p_moisFEAU, const std::string& p_siteGPENomEAU, const  std::string& p_siteElevage, const std::string& p_siteCulture);
           
            // Enlève les sites de prélévements qui ne sont pas contenue dans la carte
            static void erase_indexEAU(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<std::string>& v_site, std::vector<std::string>& v_siteGPE, std::vector<std::string>& v_cleCompos, std::vector<std::string>& v_DesCompo,
                std::vector<std::string>& v_nomLieu, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement, std::vector<std::string>& v_codeSCIAN, std::vector<double>& v_coef_consommation, std::vector<int>& v_continue,
                std::vector<int>& v_moisD, std::vector<int>& v_moisF, std::vector<std::string>& v_siteGPENom, std::vector<std::string>& v_siteCulture, std::vector<std::string>& v_siteElevage, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<int>& v_index);
           
            std::string getCleCompos();

            std::string printDonneEAU(size_t numero_site);

           static std::string printLegendeEAU();


            //----------------------------------------------------SITE SIH--------------------------------------------------------//
           // Lire fichier SIH initial
           static void lireFichierPrelvementSIH(const std::vector<std::string>& v_filename, std::vector<int>& v_id, std::vector<std::string>& v_idendifiant, std::vector<std::string>& v_proprio, std::vector<std::string>& v_adresse,
                std::vector<std::string>& v_municipalite, std::vector<std::string>& v_profondeur, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement,
                std::vector<double>& v_coef_cons, std::vector<double>& v_lat, std::vector<double>& v_long, double long_min, double long_max, double lat_min, double lat_max);

           // Enlève les sites SIH qui ne sont pas dans la carte
            static void erase_indexSIH(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<int>& v_id, std::vector<std::string>& v_identifiant, std::vector<std::string>& v_proprio, std::vector<std::string>& v_adresse,
                std::vector<std::string>& v_municipalite, std::vector<std::string>& v_profondeur, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement,
                std::vector<double>& v_coef_cons, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<int>& v_index);

            //Ajoute les données SIH spécifiques
            void ajouteDonneeSIH(std::string p_idendifiant, int p_siteGPE, std::string p_proprio, std::string p_adresse, std::string p_municipalite, std::string p_profondeur, std::string p_type, double p_prelevement);

            std::string printDonneSIH(size_t numero_site);
           
            static std::string printLegendeSIH();


           // ----------------------------------------------------------UTIL----------------------------------------------------------------------------//

            //Regarde si 2 string sont presque égales. Avec un critére de pondération. Permet de trouver entre autre le cheptel dans une table même si ce n'est pas écrit exactement pareil
            static int AlmostEgalString(std::vector<std::string> str1, std::string str2, float critere, bool elementDelete);

            // Trouve une virgule et remplace avec un point
            static std::string findVirgule(std::string data);

            // Enlève les espaces et les pluriels.  Permet de trouver entre autre le cheptel dans une table même si ce n'est pas écrit exactement pareil
            static void RemoveWhiteSpacePluriel(std::string& str);

            //Regarde si la string est un nombre
            static bool isNumber(std::string token);

            //Remplace la première occurance dans s si toReplace est trouvé.
            static std::string replaceFirstOccurrence(std::string& s, const std::string& toReplace, const std::string& replaceWith);

            //Enlève les caractères spéciaux (accent et autres)
            static void RemoveNoASCIILetter(std::string& str);

        private:

            // donnee commun
            int m_typesite;
            int m_idsite;
            double m_dlong;
            double m_dlat;
            int m_uhrh;
            int m_troncon;
            int m_idoccsol;
            std::string m_soccsol;
            int m_critere;
            std::string m_source;
            double m_coef_consommation;

            int m_pixelLong;
            int m_pixelLat;

            //donnee GPE type : 1
            void findAnneeGPE(int annee);
            std::string m_intervenantGPE;
            std::string m_codescianGPE;
            std::string m_nomIntervenantGPE;
            std::string m_nomLegalGPE;
            std::map<int, PRELEVEMENTS_DONNEES> m_mapDonnePrelevementGPE;

            // donne argicolte type : 2
            std::string m_typecheptelagricole;
            std::string m_nbrTeteagricole;
            std::string m_consommationagricole;
            double m_consommationTotalagricole;
            std::string m_nomExploitantagricole;
            std::string m_nomLieuagricole;
            std::string m_nomMunicipaliteagricole;
            int m_siteGPE;
            int m_sitePrelevement;
            
            // donne prelevement en eau type : 3  et eau potable type 4
            std::string m_siteGPEEAU;
            std::string m_cleComposEAU; 
            std::string m_DesCompoEAU; 
            std::string m_nomLieuEAU; 
            std::string m_typeEAU; 
            double m_prelevementEAU; 
            std::string m_codeSCIANEAU; 
            int m_continueEAU; 
            int m_moisDEAU; 
            int m_moisFEAU; 
            std::string m_siteGPENomEAU;
            std::string m_siteElevageEAU;
            std::string m_siteCultureEAU;

            // donnee prelevement SIH type : 5
            std::string m_idendifiant;
            std::string m_proprio;
            std::string m_adresse;
            std::string m_municipalite;
            std::string m_profondeur;
            std::string m_type;
            double m_prelevement;
    };

}

#endif 
