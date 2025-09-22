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

#include "prelevements_site.hpp"

#include "util.hpp"

#include <sstream> 
#include <fstream>
#include <regex>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/math/special_functions/round.hpp>


using namespace std;


namespace HYDROTEL
{

    PRELEVEMENTS_SITE::PRELEVEMENTS_SITE(const int p_typesite, const int p_idsite, const double p_dlong, const double p_dlat, const int p_uhrh, const int p_troncon, 
                                         const int p_idoccsol, const std::string& p_soccsol, const int p_critere, const std::string& p_source, const double p_coef_consommation) 
                                         : m_typesite(p_typesite), m_idsite(p_idsite), m_dlong(p_dlong), m_dlat(p_dlat), m_uhrh(p_uhrh), m_troncon(p_troncon), 
                                           m_idoccsol(p_idoccsol), m_soccsol(p_soccsol), m_critere(p_critere), m_source(p_source), m_coef_consommation(p_coef_consommation)
    {
    }

    void PRELEVEMENTS_SITE::vector_indexfound(double d_lat, double d_long, vector<double>& v_lat, vector<double>& v_long, vector<int>& index)
    {

        for (size_t i = 0; i < v_lat.size(); i++)
        {
            if ((d_lat == v_lat[i]) && (d_long == v_long[i]))
            {
                index.push_back((int)i);
            }
        }
    }

    int PRELEVEMENTS_SITE::getTypeSite()
    {
        return m_typesite;
    }

    void PRELEVEMENTS_SITE::setGPESite(int p_siteGPE)
    {
        m_siteGPE = p_siteGPE;
    }

    double PRELEVEMENTS_SITE::getLong()
    {
        return m_dlong;
    }

    double PRELEVEMENTS_SITE::getLat()
    {
        return m_dlat;
    }

    int PRELEVEMENTS_SITE::getLongPixel()
    {
        return m_pixelLong;
    }

    int PRELEVEMENTS_SITE::getLatPixel()
    {
        return m_pixelLat;
    }

    void PRELEVEMENTS_SITE::setPixelData(int p_pixelLong, int p_pixelLat)
    {
        m_pixelLong = p_pixelLong;
        m_pixelLat = p_pixelLat;
    }

    std::string PRELEVEMENTS_SITE::printDonneeMulti()
    {
        std::ostringstream os;

        //os << m_dlat << ";" << m_dlong << ";" << m_troncon << ";" << m_uhrh << ";" << m_idoccsol << ";" << m_soccsol << ";" << m_source << ";" << m_critere << ";" << m_coef_consommation;
		//os << m_dlat << ";" << m_dlong << ";" << m_troncon << ";" << m_uhrh << ";" << m_source << ";" << m_critere << ";" << m_coef_consommation;
		os << fixed << setprecision(5) << m_dlat << ";" << m_dlong;
		os.unsetf(ios_base::floatfield);	//set default precision
		os << ";" << m_troncon << ";" << m_uhrh << ";" << m_source << ";" << m_critere << ";" << m_coef_consommation;

        return os.str();
    }

    std::string PRELEVEMENTS_SITE::printLegendMulti()
    {
         //return "LAT(d);LONG(d);ID TRONCON;ID UHRH;# OCCSOL;NOM OCCSOL;SOURCE;CRITERE;COEF_CONSO";
		return "LAT(d);LONG(d);ID TRONCON;ID UHRH;SOURCE;CRITERE;COEF_CONSO";
    }


    //------------------------------------------------------------------------- PRELEVEMENT GPE------------------------------------------------------------------///

    void PRELEVEMENTS_SITE::setIntervenantCodeScianGPE(const std::string& p_intervenantGPE, const std::string& p_nomIntervenantGPE, const std::string& p_nomLegalGPE, const std::string& p_codeScianGPE)
        {
        m_intervenantGPE = p_intervenantGPE;
        m_nomIntervenantGPE = p_nomIntervenantGPE;
        m_nomLegalGPE = p_nomLegalGPE;
        m_codescianGPE = p_codeScianGPE;
        }

    void PRELEVEMENTS_SITE::ajouteDonneeGPE(int annee, int idmois, int nbrJour, double p_volume, double p_consomation)
        {
        findAnneeGPE(annee);
        m_mapDonnePrelevementGPE[annee].asgNbrjour(idmois, nbrJour);
        m_mapDonnePrelevementGPE[annee].asgVolume(idmois, p_volume);
        m_mapDonnePrelevementGPE[annee].asgConsomation(idmois, p_consomation);

        }

    void PRELEVEMENTS_SITE::findAnneeGPE(int annee)
        {
        if (m_mapDonnePrelevementGPE.find(annee) == m_mapDonnePrelevementGPE.end()) // annee pas trouver 
            m_mapDonnePrelevementGPE[annee] = PRELEVEMENTS_DONNEES();
        }


    std::string PRELEVEMENTS_SITE::printDonneGPE(size_t numero_site)
    {
        std::ostringstream os, os2;
        os << numero_site << ";"<< printDonneeMulti() << ";" << m_intervenantGPE << ";" << m_nomIntervenantGPE << ";" << m_nomLegalGPE << ";" << m_codescianGPE;
        std::map<int, PRELEVEMENTS_DONNEES>::iterator it;

		vector<double> vals;
		vector<double> nbs;
        vector<int> valsJour;
        vector<int> nbsJour;
        double dVal;
		size_t i, idx;
        int iVal;

		vals.resize(36, 0.0);	//12 mois * 3 valeurs
		nbs.resize(36, 0.0);

        valsJour.resize(12, 0);
		nbsJour.resize(12, 0);

		for (it = m_mapDonnePrelevementGPE.begin(); it != m_mapDonnePrelevementGPE.end(); it++)
        {
			idx = 0;
			for(i=0; i!=it->second.m_vNbrjour.size(); i++)
			{
                if(it->second.m_vNbrjour[i] != 0)   //il ne faut pas prendre les mois o˘ le nombre de jour est egal ‡ 0, ceux-ci doivent Ítre exclue du calcul du nb de jour moyen (ligne MOY)
                {
                    nbs[idx]+= 1.0;     //nb d'annee
                    nbsJour[idx]+= 1;   //
                }
				++idx;
			}

			for(i=0; i!=it->second.m_vVol.size(); i++)
			{
				if(it->second.m_vVol[i] != 0.0)
					nbs[idx]+= 1.0;
				++idx;
			}

			for(i=0; i!=it->second.m_vCons.size(); i++)
			{
				if(it->second.m_vCons[i] != 0.0)
					nbs[idx]+= 1.0;
				++idx;
			}
        }

        for (it = m_mapDonnePrelevementGPE.begin(); it != m_mapDonnePrelevementGPE.end(); it++)
        {
            os2 << os.str() << ";" << it->first << ";" << it->second.printDonnee() << endl;

			idx = 0;
			for(i=0; i!=it->second.m_vNbrjour.size(); i++)
			{
				if(it->second.m_vNbrjour[i] != 0)
					valsJour[idx]+= it->second.m_vNbrjour[i];
				++idx;
			}

			for(i=0; i!=it->second.m_vVol.size(); i++)
			{
				if(it->second.m_vVol[i] != 0.0)
					vals[idx]+= (it->second.m_vVol[i] / nbs[idx]);
				++idx;
			}

			for(i=0; i!=it->second.m_vCons.size(); i++)
			{
				if(it->second.m_vCons[i] != 0.0)
					vals[idx]+= (it->second.m_vCons[i] / nbs[idx]);
				++idx;
			}
        }

		os2 << os.str() << ";" << "MOY";
        for(i=0; i!=vals.size(); i++)
        {
            if(i < 12) //colonnes nb jours
            {
                if(nbsJour[i] != 0)
                {
                    dVal = ((double)valsJour[i]) / nbsJour[i];  //pour Èviter les diffÈrence d'arrondissement pour le nb de jour moyen pour chaque mois
                    iVal = boost::math::lround(dVal);           //
                }
                else
                    iVal = 0;
                
                os2 << ";" << iVal;
            }
            else
                os2 << ";" << fixed << setprecision(0) << vals[i];
        }
		os2 << endl;

        return os2.str();
    }


    std::string PRELEVEMENTS_SITE::printLegendGPE()
    {
        //multi: "LAT(d);LONG(d);ID TRONCON;ID UHRH;# OCCSOL;NOM OCCSOL;SOURCE;CRITERE;COEF_CONSO";
        std::string str = "#SITE GPE;" + printLegendMulti() + ";# INTERVENANT; NOM INTERVENANT; Nom legal du lieu ou personne physique; Code SCIAN; ANNEE; NbJ M1; NbJ M2; NbJ M3; NbJ M4; NbJ M5; NbJ M6; NbJ M7; NbJ M8; NbJ M9; NbJ M10; NbJ M11; NbJ M12; VOL M1; VOL M2; VOL M3; VOL M4; VOL M5; VOL M6; VOL M7; VOL M8; VOL M9; VOL M10; VOL M11; VOL M12; CONS M1; CONS M2; CONS M3; CONS M4; CONS M5; CONS M6; CONS M7; CONS M8; CONS M9; CONS M10; CONS M11; CONS M12";
        return str;
    }


    std::vector<PRELEVEMENTS_SITE> PRELEVEMENTS_SITE::lirePrelevementGPE(const std::string& filename)
    {
        std::ifstream in(filename.c_str());
        std::ostringstream os;
        std::string str;
        int no_ligne = 0;

        string s_intervenant;
        int i_annee;
        int i_nbrjour;
        double d_volume;
        string s_source;
        string s_codescian;
        string s_nomIntervenant;
        string s_nomLegal;
        double d_lat;
        double d_long;
        double d_consomation, d_coef_consommation;
        int i_uhrh;
        int i_troncon;
        int i_idOccsol;
        string s_sOccsol;
        int i_critere;
        int k = -1;
        int nbrSiteold = 0;
        int nbrSite = 0;
        bool newSite = true;
        vector<string> v_ligne;
        string ligne;
        std::vector<PRELEVEMENTS_SITE> v_sitePrelevementGPE;

        try {

        if (!in)
            std::cout << "impossible d`ouvrir fichier GPE: " << filename << std::endl;
        else
        {
            // Read the next line from File untill it reaches the end.
            while (getline_mod(in, ligne)) //lit les lignes
            {
                try {

                newSite = false;
                // d_lat = 0;
                if (no_ligne > 0)
                {
                    //string ligne;
                    //istringstream tokenStream(str);
                    v_ligne = extrait_stringValeur(ligne);
                    nbrSite = stoi(v_ligne[0]);
                    if (nbrSite != nbrSiteold) // nouveau site
                        newSite = true;
                    if (newSite)
                    {
                        d_lat = stod(v_ligne[1]);
                        d_long = stod(v_ligne[2]);
                        i_troncon = stoi(v_ligne[3]);
                        i_uhrh = stoi(v_ligne[4]);
                        i_idOccsol = stoi(v_ligne[5]);
                        s_sOccsol = v_ligne[6];
                        s_source = v_ligne[7];
                        i_critere = stoi(v_ligne[8]);
                        d_coef_consommation = stod(v_ligne[9]);
                        s_intervenant = v_ligne[10];
                        s_nomIntervenant = v_ligne[11];
                        s_nomLegal = v_ligne[12];
                        s_codescian = v_ligne[13];
                        k++;
                        v_sitePrelevementGPE.push_back(PRELEVEMENTS_SITE(1, newSite, d_long, d_lat, i_uhrh, i_troncon, i_idOccsol, s_sOccsol, i_critere, s_source, d_coef_consommation)); // id site = 1 pour site GPE
                        v_sitePrelevementGPE[k].setIntervenantCodeScianGPE(s_intervenant, s_nomIntervenant, s_nomLegal, s_codescian);
                        i_annee = stoi(v_ligne[14]);
                        for (int i = 0; i < 12; i++)
                        {
                            i_nbrjour = stoi(v_ligne[(15 + i)]);
                            d_volume = stod(v_ligne[(15 + 12 + i)]);
                            d_consomation = stod(v_ligne[(15 + 12 + 12 + i)]);
                            v_sitePrelevementGPE[k].ajouteDonneeGPE(i_annee, i + 1, i_nbrjour, d_volume, d_consomation);
                        }
                    }
                    else
                    {
                        i_annee = stoi(v_ligne[14]);
                        for (int i = 0; i < 12; i++)
                        {
                            i_nbrjour = stoi(v_ligne[(15 + i)]);
                            d_volume = stod(v_ligne[(15 + 12 + i)]);
                            d_consomation = stod(v_ligne[(15 + 12 + 12 + i)]);
                            v_sitePrelevementGPE[k].ajouteDonneeGPE(i_annee, i + 1, i_nbrjour, d_volume, d_consomation);
                        }

                    }

                    nbrSiteold = nbrSite;
                }

                }
                catch (...)
                {
                    std::cout << "erreur lors de la lecture du fichier: " << filename << ": ligne " << no_ligne+1 << ": " << ligne << endl << endl;
                }

				no_ligne++;
            }
        }

        }
        catch (...)
        {
            std::cout << "erreur lors de la lecture du fichier: " << filename << endl;
        }

        return v_sitePrelevementGPE;
    }

    bool PRELEVEMENTS_SITE::importFichierPrelevementGPE(const std::vector<std::string>& filenames, std::vector<std::string>& v_intervenant, std::vector<std::string>& v_nomIntervenant, std::vector<std::string>& v_nomLegal, std::vector<int>& v_annee, std::vector<int>& v_mois, std::vector<int>& v_nbrjour,
            std::vector<double>& v_volume, std::vector<std::string>& v_source, std::vector<std::string>& v_codescian, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<double>& v_consomation,
           double long_min, double long_max, double lat_min, double lat_max)
    {
        bool ok = true;
        // Open the File
        for (size_t h = 0; h < filenames.size(); h++)
        {
            std::ifstream in(filenames[h].c_str());
            std::ostringstream oss;
			istringstream iss;
            string s_intervenant;
            int i_annee = -1;
            int i_mois;
            int i_nbrjour;
            double d_volume;
            string s_source;
            string s_codescian;
            string s_nomIntervenant;
            string s_nomLegal;
			string sval;
            double d_lat;
            double d_long;
            double d_consomation;
			//double dval;
			//int ival;
            int element;
            bool valid;
            std::string str;
            std::string b;
            int nbr = 0;
            ok = true;

            try {
                // Check if object is valid
                if (!in)
                {
                    std::cout << "impossible d`ouvrir fichier GPE: " << filenames[h] << std::endl;
                    ok = false;
                }
                else
                {
                    // Read the next line from File untill it reaches the end.
                     while (getline_mod(in, str))
                        {
                        try {
                            nbr++;
                            element = 0;
                            valid = true;
                            string ligne;
                            istringstream tokenStream(str);
                            while (getline(tokenStream, ligne, ';') && valid)
                            {
                                if (valid)
                                {
                                    switch (element)
                                    {

                                    case 0:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_intervenant = ligne;
                                        break;

                                    case 1:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_nomIntervenant = ligne;
                                        break;

                                    case 3:
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            i_annee = (stoi(ligne));
                                            if (i_annee < 1900)
                                            {
                                                valid = false;
                                            }
                                        }
                                        break;

                                    case 4:
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            i_mois = (stoi(ligne));
                                            if (i_mois > 12 || i_mois < 0)
                                            {
                                                valid = false;
                                            }
                                        }
                                        else { valid = false; }
                                        break;

                                    case 8:
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            i_nbrjour = (stoi(ligne));
                                            if (i_nbrjour > 31 || i_nbrjour < 0)
                                            {
                                                valid = false;
                                            }
                                        }
                                        else { valid = false; }
                                        break;

                                    case 10:
                                        ligne = findVirgule(ligne);
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            d_volume = (stod(ligne));
                                        }
                                        else { valid = false; }
                                        break;

                                    case 11:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_source = ligne;
                                        break;

                                    case 14:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_nomLegal = ligne;
                                        break;

                                    case 15:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_codescian = (ligne);
                                        break;

                                    case 18:
                                        ligne = findVirgule(ligne);
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            d_lat = (stod(ligne));
                                            if (!(d_lat >= lat_min && d_lat <= lat_max))
                                                valid = false;

											//tronque ‡ la 5e dÈcimal
											//dval = d_lat * 100000.0;
											//ival = static_cast<int>(dval);
											//d_lat = static_cast<double>(ival);
											//d_lat/= 100000.0;
											//

											//arrondie ‡ la 5e decimal
											oss.str("");
											oss << fixed << setprecision(5) << d_lat;
											iss.clear();
											iss.str(oss.str());
											iss >> d_lat;
											//d_lat = floor(d_lat * 100000.0 + 0.5) / 100000.0;
											//
                                        }
                                        else { valid = false; }
                                        break;

                                    case 19:
                                        ligne = findVirgule(ligne);
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            d_long = (stod(ligne));
                                            if (!(d_long >= long_min && d_long <= long_max))
                                                valid = false;

											//tronque ‡ la 5e dÈcimal
											//dval = d_long * 100000.0;
											//ival = static_cast<int>(dval);
											//d_long = static_cast<double>(ival);
											//d_long/= 100000.0;
											//

											//arrondie ‡ la 5e decimal
											oss.str("");
											oss << fixed << setprecision(5) << d_long;
											iss.clear();
											iss.str(oss.str());
											iss >> d_long;
											//d_long = floor(d_long * 100000.0 + 0.5) / 100000.0;
											//
                                        }
                                        else { valid = false; }
                                        break;

                                    case 20:
                                        ligne = findVirgule(ligne);
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            d_consomation = (stod(ligne));
                                        }
                                        else { d_consomation = 0; }
                                        break;

                                    default:
                                        break;
                                    }
                                }
                                element++;
                            }
                            if (element != 21)
                                valid = false;
                            if (valid)
                            {
                                v_intervenant.push_back(s_intervenant);
                                v_nomIntervenant.push_back(s_nomIntervenant);
                                v_nomLegal.push_back(s_nomLegal);
                                v_annee.push_back(i_annee);
                                v_mois.push_back(i_mois);
                                v_nbrjour.push_back(i_nbrjour);
                                v_volume.push_back(d_volume);
                                v_source.push_back(s_source);
                                v_codescian.push_back(s_codescian);
                                v_lat.push_back(d_lat);
                                v_long.push_back(d_long);
                                v_consomation.push_back(d_consomation);
                            }
                        }
                        catch (...)
                        {
                            std::cout << "erreur lors de la lecture du fichier: " << filenames[h] << ": ligne " << nbr+1 << ": " << str << endl << endl;
                        }
                    }
                }
            }
            catch (...)
            {
                std::cout << "erreur lors de la lecture du fichier: " << filenames[h] << endl;
                ok = false;
            }
        }
        return ok;
    }

    void PRELEVEMENTS_SITE::erase_indexGPE(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<std::string>& v_intervenant, std::vector<std::string>& v_nomIntervenant, std::vector<std::string>& v_nomLegal, std::vector<int>& v_annee, std::vector<int>& v_mois, std::vector<int>& v_nbrjour,
            std::vector<double>& v_volume, std::vector<std::string>& v_source, std::vector<std::string>& v_codescian, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<double>& v_consomation, std::vector<int>& v_index)
        {
            std::sort(v_index.begin(), v_index.end());
            int i = 0;
            int index;
            while (v_index.size() > 0)
            {
                index = v_index[0] - i;
                v_uhrh.erase(v_uhrh.begin() + index);
                v_troncon.erase(v_troncon.begin() + index);
                v_idOccsol.erase(v_idOccsol.begin() + index);
                v_sOccsol.erase(v_sOccsol.begin() + index);
                v_intervenant.erase(v_intervenant.begin() + index);
                v_nomIntervenant.erase(v_nomIntervenant.begin() + index);
                v_nomLegal.erase(v_nomLegal.begin() + index);
                v_annee.erase(v_annee.begin() + index);
                v_mois.erase(v_mois.begin() + index);
                v_nbrjour.erase(v_nbrjour.begin() + index);
                v_volume.erase(v_volume.begin() + index);
                v_source.erase(v_source.begin() + index);
                v_codescian.erase(v_codescian.begin() + index);
                v_lat.erase(v_lat.begin() + index);
                v_long.erase(v_long.begin() + index);
                v_consomation.erase(v_consomation.begin() + index);
                v_index.erase(v_index.begin());
                i++;
            }
        }

    std::string PRELEVEMENTS_SITE::getnomIntervenantGPE()
    {
        return m_nomIntervenantGPE;
    }

    std::string PRELEVEMENTS_SITE::getnomLegalGPE()
    {
        return m_nomLegalGPE;
    }


    // ------------------------------------------------------------------------ PRELEVEMENT agricole----------------------------------------------------------------//


    void PRELEVEMENTS_SITE::ajouteDonneeagricole(const std::string& p_nomExploitant, const std::string& p_nomlieu, const std::string& p_nomMunicipalite, const std::string& p_typeCheptel, const std::string& p_nbrTete,
        const std::string& p_consomation, double p_consomationTotal, int p_siteGPE, int p_sitePrelevement)
    {
        m_nomExploitantagricole = p_nomExploitant;
        m_nomLieuagricole = p_nomlieu;
        m_nomMunicipaliteagricole = p_nomMunicipalite;
        m_typecheptelagricole = p_typeCheptel;
        m_nbrTeteagricole = p_nbrTete;
        m_consommationagricole = p_consomation;
        m_consommationTotalagricole = p_consomationTotal;
        m_siteGPE = p_siteGPE;
        m_sitePrelevement = p_sitePrelevement;
    }

    std::string PRELEVEMENTS_SITE::printDonneagricole(size_t numero_site)
    {
        std::ostringstream os;
        os << numero_site << ";" << m_siteGPE << ";" << m_sitePrelevement << ";" << printDonneeMulti() << ";" << m_typecheptelagricole << ";"
            << m_nbrTeteagricole << ";" << m_consommationagricole << ";" << m_consommationTotalagricole << ";" << m_nomLieuagricole << ";" << m_nomExploitantagricole << ";" << m_nomMunicipaliteagricole << "\n";
        return os.str();
    }

    std::string PRELEVEMENTS_SITE::printLegendAgricole()
    {
        //multi: "LAT(d);LONG(d);ID TRONCON;ID UHRH;# OCCSOL;NOM OCCSOL;SOURCE;CRITERE;COEF_CONSO";
        std::string str = "#SITE;#SITE GPE;#SITE PRELEVEMENT;" + printLegendMulti() + ";TYPE CHEPTEL;NOMBRE TETE;CONSOMMATION m3/an;CONSOMMATION TOTAL m3/an;NOM DU LIEU;NOM DE LEXPLOITANT;MUNICIPALITE";
        return str;
    }

    int PRELEVEMENTS_SITE::FindCheptel(std::string str_tofind, const std::vector<std::string>& v_listeCheptel)
    {
        RemoveWhiteSpacePluriel(str_tofind);
        std::string cheptel;
        std::size_t found;
        int almost;
        for (size_t i = 0; i < v_listeCheptel.size(); i++)
        {
            cheptel = v_listeCheptel[i];
            RemoveWhiteSpacePluriel(cheptel);
            found = str_tofind.find(cheptel);
            if (found != std::string::npos)
                return static_cast<int>(i);
        }
        almost = AlmostEgalString(v_listeCheptel, str_tofind, (float)0.35, false);
        if (almost != -1)
        return almost;

        return -1;
    }

    void PRELEVEMENTS_SITE::erase_indexagricole(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<std::string>& v_nomExploitant, std::vector<std::string>& v_nomlieu, std::vector<std::string>& v_nomMunicipalite,
        std::vector<std::string>& v_typeCheptel, std::vector<std::string>& v_nbrTete, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<std::string>& v_consomation, std::vector<double>& v_consomationTotal, std::vector<int>& v_index)
    {
        std::sort(v_index.begin(), v_index.end());
        int i = 0;
        int index;
        while (v_index.size() > 0)
        {
            index = v_index[0] - i;
            v_uhrh.erase(v_uhrh.begin() + index);
            v_troncon.erase(v_troncon.begin() + index);
            v_idOccsol.erase(v_idOccsol.begin() + index);
            v_sOccsol.erase(v_sOccsol.begin() + index);
            v_nomExploitant.erase(v_nomExploitant.begin() + index);
            v_nomlieu.erase(v_nomlieu.begin() + index);
            v_nomMunicipalite.erase(v_nomMunicipalite.begin() + index);
            v_typeCheptel.erase(v_typeCheptel.begin() + index);
            v_nbrTete.erase(v_nbrTete.begin() + index);
            v_consomation.erase(v_consomation.begin() + index);
            v_consomationTotal.erase(v_consomationTotal.begin() + index);
            v_lat.erase(v_lat.begin() + index);
            v_long.erase(v_long.begin() + index);
            v_index.erase(v_index.begin());
            i++;
        }
    }

    bool PRELEVEMENTS_SITE::importTableConsommationCheptel(const std::string& filename, std::vector<double>& v_listeConsomation, std::vector<std::string>& v_listeCheptel)
    {
        // Open the File
        std::ifstream in(filename.c_str());
        std::ostringstream os;
        std::string str, sLigne;
        int element = 0;
        std::string s_cheptel;
        double d_consommation;
        bool valid = true;
        int no_ligne = 0;
            
        try {
            // Check if object is valid
            if (!in)
            {
                std::cout << "impossible d`ouvrir fichier consommation cheptel: " << filename << std::endl;
            }
            else
            {
                // Read the next line from File untill it reaches the end.
                while (getline_mod(in, sLigne))
                {
					str = sLigne;

                    string ligne;
                    istringstream tokenStream(str);
                    element = 0;
                    valid = true;
                    RemoveNoASCIILetter(str);
                    try {
                        while (getline(tokenStream, ligne, ';')) //separe la ligne selon les colonnes
                        {
                            if (element == 1)
                            {
                                s_cheptel = ligne;
                            }
                            if (element == 2)
                            {
                                ligne = findVirgule(ligne);
                                if (isNumber(ligne) && !(ligne.empty()))
                                {
                                    d_consommation = (stod(ligne));
                                }
                                else
                                {
                                    valid = false;
                                }
                            }
                            element++;
                        }
                        if (valid)
                        {
                            v_listeConsomation.push_back(d_consommation);
                            v_listeCheptel.push_back(s_cheptel);
                        }
                    }
                    catch (...)
                    {
                        std::cout << "erreur lors de la lecture du fichier: " << filename << ": ligne " << no_ligne+1 << ": " << sLigne << endl << endl;
                    }
                    no_ligne++;
                }
            }
        }
        catch (...)
        {
            std::cout << "erreur lors de la lecture du fichier: " << filename << endl;
            return false;
        }
        return true;
    }

    bool PRELEVEMENTS_SITE::importFichierPrelevementagricole(const std::vector<std::string>& filename, const std::string& filenameTable, std::vector<std::string>& v_nomExploitant, std::vector<std::string>& v_nomlieu, std::vector<std::string>& v_nomMunicipalite, std::vector<std::string>& v_typeCheptel, std::vector<std::string>& v_nbrTete,
        std::vector<double>& v_listeconsommation, std::vector<std::string>& v_listeCheptel, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<std::string>& v_consomation, std::vector<double>& v_consomationTotal, double long_min, double long_max, double lat_min, double lat_max)
    {
        for (size_t h = 0; h < filename.size(); h++)
        {
            std::ifstream in(filename[h].c_str());
            std::ostringstream os;
            std::string str;
            int no_ligne = 0;
            int no_col = 0;

            string s_nomExploitant, s_nomlieu, s_nomMunicipalite, s_typeCheptel, s_nbrTete, s_consommation, sLigne;
            double d_lat, d_long, d_consommation_total;
            d_lat = 0;
            d_long = 0;
            d_consommation_total = 0;
            string s_oldnomExploitant, s_oldnomlieu, s_oldnomMunicipalite, s_oldtypeCheptel, s_oldnbrTete, s_oldconsommation, s_animal;
            double d_oldlat = 0;
            double d_oldlong = 0;
            double d_nbr_tete = 0;
            double d_oldconsommation_total = 0;
            int index_cheptel = 0;

            //bool validold = true;
          //  bool valid = true;
            bool endLigne = false;

            double calcul;
            int nor = 0;
            importTableConsommationCheptel(filenameTable, v_listeconsommation, v_listeCheptel);
            int size_cheptel = (int)v_listeCheptel.size();
            try {
                // Check if object is valid
                if (!in)
                {
                    std::cout << "impossible d`ouvrir fichier prelevement agricole: " << filename[h] << std::endl;
                }
                else
                {
                    // Read the next line from File untill it reaches the end.
                    while (getline_mod(in, sLigne)) //lit les lignes
                    {
						str = sLigne;

                        RemoveNoASCIILetter(str);
                        // d_lat = 0;
                        s_animal = "";
                        try {
                            if (no_ligne > 4)
                            {
                                string ligne;
                                istringstream tokenStream(str);
                                no_col = 0;
                                endLigne = false;

                                while (getline(tokenStream, ligne, ';')) //lit les colonnes
                                {
                                    switch (no_col)
                                    {
                                    case 2:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_nomlieu = ligne;
                                        break;

                                    case 3:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_nomMunicipalite = ligne;
                                        break;

                                    case 4:
                                        ligne = findVirgule(ligne);
                                        if (!ligne.empty())
                                        {
                                            if (isNumber(ligne))
                                            {
                                                RemoveWhiteSpacePluriel(ligne);
                                                d_lat = (stod(ligne));
                                            }
                                            nor++;
                                        }
                                        else
                                        {
                                            d_lat = 0;
                                        }
                                        break;

                                    case 5:
                                        ligne = findVirgule(ligne);
                                        if (!ligne.empty())
                                        {
                                            if (isNumber(ligne))
                                            {
                                                RemoveWhiteSpacePluriel(ligne);
                                                d_long = (stod(ligne));
                                            }
                                        }
                                        else
                                        {
                                            d_long = 0;
                                        }
                                        break;

                                    case 7:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_nomExploitant = ligne;
                                        break;

                                    case 8:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_animal = ligne;
                                        index_cheptel = FindCheptel(ligne, v_listeCheptel);
                                        break;

                                    case 9:
                                        if (ligne.empty() && !(isNumber(ligne)))
                                            ligne = "0";
                                        else
                                        {
                                            RemoveWhiteSpacePluriel(ligne);
                                            d_nbr_tete = stod(ligne);
                                        }
                                        endLigne = true;
                                        break;
                                    }//fin du switch


                                    if (endLigne)
                                    {
                                        if (nor == 2)
                                        {
                                            s_oldtypeCheptel = s_typeCheptel; s_oldnbrTete = s_nbrTete; s_oldconsommation = s_consommation;
                                            d_oldconsommation_total = d_consommation_total;
                                            s_consommation.clear();
                                            s_nbrTete.clear();
                                            s_typeCheptel.clear();
                                            d_consommation_total = 0;
                                        }
                                        if (!((index_cheptel >= 0) && (index_cheptel < size_cheptel)))
                                        {
                                            //calcul = 0;// v_listeconsommation[index_cheptel] * d_nbr_tete;
                                            s_typeCheptel += s_animal + ",";
                                            s_consommation += ",";
                                            s_nbrTete += to_string(d_nbr_tete) + ",";

                                            // d_consommation_total +=0;
                                        }

                                        else
                                        {
                                            calcul = v_listeconsommation[index_cheptel] * d_nbr_tete;
                                            s_typeCheptel += v_listeCheptel[index_cheptel] + ",";
                                            s_consommation += to_string(calcul) + ",";
                                            s_nbrTete += to_string(d_nbr_tete) + ",";
                                            d_consommation_total += calcul;
                                        }
                                    }
                                    no_col++;
                                } // fin du while col

                            ///fin de la ligne 
                                if (nor == 2 && endLigne)
                                {
                                    if (((d_oldlong >= long_min && d_oldlong <= long_max)) && ((d_oldlat >= lat_min && d_oldlat <= lat_max)))
                                    {
                                        v_nomExploitant.push_back(s_oldnomExploitant);
                                        v_nomlieu.push_back(s_oldnomlieu);
                                        v_nomMunicipalite.push_back(s_oldnomMunicipalite);
                                        v_lat.push_back(d_oldlat);
                                        v_long.push_back(d_oldlong);
                                        v_typeCheptel.push_back(s_oldtypeCheptel);
                                        v_nbrTete.push_back(s_oldnbrTete);
                                        v_consomation.push_back(s_oldconsommation);
                                        v_consomationTotal.push_back(d_oldconsommation_total);
                                    }
                                    /*
                                    s_oldtypeCheptel = s_typeCheptel; s_oldnbrTete = s_nbrTete; s_oldconsommation = s_consommation;
                                    d_oldconsommation_total = d_consommation_total;
                                    s_consommation.clear();
                                    s_nbrTete.clear();
                                    s_typeCheptel.clear();
                                    d_consommation_total = 0;*/
                                    nor = 1;
                                }
                                if ((d_lat != 0)) // valeur == nouvelle entreprise  && 1er passage 
                                {
                                    s_oldnomlieu = s_nomlieu; s_oldnomExploitant = s_nomExploitant; s_oldnomMunicipalite = s_nomMunicipalite; d_oldlat = d_lat; d_oldlong = d_long;
                                    /*s_oldtypeCheptel = s_typeCheptel; s_oldnbrTete = s_nbrTete; s_oldconsommation = s_consommation;
                                    d_oldconsommation_total = d_consommation_total;
                                    s_consommation.clear();
                                    s_nbrTete.clear();
                                    s_typeCheptel.clear();
                                    d_consommation_total = 0;*/
                                }
                            }
                        }
                        catch (...)
                        {
                            std::cout << "erreur lors de la lecture du fichier: " << filename[h] << ": ligne " << no_ligne+1 << ": " << sLigne << endl << endl;
                        }
                        no_ligne++;
                    }
                }
            }
            catch (...)
            {
                std::cout << "erreur lors de la lecture du fichier: " << filename[h] << endl;
                return false;
            }
        }
        return true;
    }

    std::vector<PRELEVEMENTS_SITE> PRELEVEMENTS_SITE::lirePrelevementAGRICOLE(const std::string& filename)
    {
        std::ifstream in(filename.c_str());
        std::ostringstream os;
        std::string str;
        int no_ligne = 0;

        string s_nomExploitant, s_nomlieu, s_nomMunicipalite, s_typeCheptel, s_nbrTete, s_consommation, s_source;
        double d_lat, d_long, d_consommation_total, d_coef_consommation;
        d_lat = 0;
        d_long = 0;
        d_consommation_total = 0;
        int nbrSite = 0;
        int i_uhrh;
        int i_troncon;
        int i_idOccsol;
        string s_sOccsol;
        int i_critere = 0;
        int i_siteGPE, i_sitePrelevementEAU;
        vector<string> v_ligne;
        string ligne;
        std::vector<PRELEVEMENTS_SITE> v_sitePrelevementagricole;

        try {
            // Check if object is valid
            if (!in)
            {
                std::cout << "impossible d`ouvrir fichier prelevement agricole: " << filename << std::endl;
            }
            else
            {
                // Read the next line from File untill it reaches the end.
                while (getline_mod(in, ligne)) //lit les lignes
                {
                    try {
                        // d_lat = 0;
                        if (no_ligne > 0)
                        {
                            //string ligne;
                            //istringstream tokenStream(str);
                            v_ligne = extrait_stringValeur(ligne);
                            if (v_ligne.size() >= 19)
                            {
                                nbrSite = stoi(v_ligne[0]);
                                i_siteGPE = stoi(v_ligne[1]);
                                i_sitePrelevementEAU = stoi(v_ligne[2]);
                                d_lat = stod(v_ligne[3]);
                                d_long = stod(v_ligne[4]);
                                i_troncon = stoi(v_ligne[5]);
                                i_uhrh = stoi(v_ligne[6]);
                                i_idOccsol = stoi(v_ligne[7]);
                                s_sOccsol = v_ligne[8];
                                s_source = v_ligne[9];
                                i_critere = stoi(v_ligne[10]);
                                d_coef_consommation = stod(v_ligne[11]);
                                s_typeCheptel = v_ligne[12];
                                s_nbrTete = v_ligne[13];
                                s_consommation = v_ligne[14];
                                d_consommation_total = stod(v_ligne[15]);
                                s_nomlieu = v_ligne[16];
                                s_nomExploitant = v_ligne[17];
                                s_nomMunicipalite = v_ligne[18];
                                v_sitePrelevementagricole.push_back(PRELEVEMENTS_SITE(2, nbrSite, d_long, d_lat, i_uhrh, i_troncon, i_idOccsol, s_sOccsol, i_critere, s_source, d_coef_consommation)); // id site = 1 pour site GPE
                                v_sitePrelevementagricole[nbrSite - 1].ajouteDonneeagricole(s_nomExploitant, s_nomlieu, s_nomMunicipalite, s_typeCheptel, s_nbrTete, s_consommation, d_consommation_total, i_siteGPE, i_sitePrelevementEAU);
                            }
                        }
                    }
                    catch (...)
                    {
                        std::cout << "erreur lors de la lecture du fichier: " << filename << ": ligne " << no_ligne+1 << ": " << ligne << endl << endl;
                    }

                    no_ligne++;
                }
            }
        }
        catch (...)
        {
            std::cout << "erreur lors de la lecture du fichier: " << filename << endl;
        }
        return v_sitePrelevementagricole;
    }
    
    std::string PRELEVEMENTS_SITE::getnomLieuagricole()
    {
        return m_nomLieuagricole;
    }

    std::string PRELEVEMENTS_SITE::getnomExploitant()
    {
        return m_nomExploitantagricole; 
    }

    void PRELEVEMENTS_SITE::findCorrespondanceNom(std::vector<PRELEVEMENTS_SITE>& v_siteGPE, std::vector<PRELEVEMENTS_SITE>& v_siteAgricole)
    {
        int sizeAgricole = (int) v_siteAgricole.size();
        int sizeGPE = (int) v_siteGPE.size();
        int siteGPE;
        std::vector<std::string> v_nomIntervenantGPE, v_nomLegalGPE;
        float critere = 0.65f;

        for (int i = 0; i < sizeGPE; i++)
        {
            v_nomIntervenantGPE.push_back(v_siteGPE[i].getnomIntervenantGPE());
            v_nomLegalGPE.push_back(v_siteGPE[i].getnomLegalGPE());
        }
        // 4 test ‡ faire selon les combinaisons suivante : 
        // nomLieuagricole avec Nom intervenant / NomLieuagricole avec NomLegal / nomExploitant avec Nom intervenant / nomExploitant avec NomLegal
        
        //Test 1
        for (int i = 0; i < sizeAgricole; i++)
        {
            siteGPE = AlmostEgalString(v_nomIntervenantGPE, v_siteAgricole[i].getnomLieuagricole(), critere, true);
            if (siteGPE != -1)
            {
                v_siteAgricole[i].setGPESite(siteGPE + 1);
            }
        }

        //Test 2
        for (int i = 0; i < sizeAgricole; i++)
        {
            siteGPE = AlmostEgalString(v_nomIntervenantGPE, v_siteAgricole[i].getnomExploitant(), critere, true);
            if (siteGPE != -1)
            {
                v_siteAgricole[i].setGPESite(siteGPE + 1);
            }
        }
        //Test 3
        for (int i = 0; i < sizeAgricole; i++)
        {
            siteGPE = AlmostEgalString(v_nomLegalGPE, v_siteAgricole[i].getnomLieuagricole(), critere, true);
            if (siteGPE != -1)
            {
                v_siteAgricole[i].setGPESite(siteGPE + 1);
            }
        }
        //Test 4
        for (int i = 0; i < sizeAgricole; i++)
        {
            siteGPE = AlmostEgalString(v_nomLegalGPE, v_siteAgricole[i].getnomExploitant(), critere, true);
            if (siteGPE != -1)
            {
                v_siteAgricole[i].setGPESite(siteGPE + 1);
            }
        }

    }

    void PRELEVEMENTS_SITE::RefreashFichieragricole(std::vector<PRELEVEMENTS_SITE>& v_siteAgricole, std::string repertoireProjet)
    {
        std::string extension = "/PRELEVEMENT_agricole.csv";
        std::ofstream ofs;
        std::string filename2 = repertoireProjet + extension;
        ofs.open(filename2, std::ofstream::out | std::ofstream::trunc);
        ofs << PRELEVEMENTS_SITE::printLegendAgricole() << "\n";
        for (size_t k = 0; k < v_siteAgricole.size(); k++)
        {
            ofs << v_siteAgricole[k].printDonneagricole(k + 1);
        }

        ofs.close();
    }

    
	//----------------------------------------------------PRELEVEMENT EAU-------------------------------------------------------------------//


    void PRELEVEMENTS_SITE::lireFichierPrelevementEAU(const std::vector<std::string>& v_filename, std::vector<std::string>& v_site, std::vector<std::string>& v_siteGPE, std::vector<std::string>& v_cleCompos, std::vector<std::string>& v_DesCompo,
        std::vector<std::string>& v_nomLieu, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement, 
        std::vector<std::string>& v_codeSCIAN, std::vector<double>& v_coef_consommation, std::vector<int>& v_continue, std::vector<int>& v_moisD, std::vector<int>& v_moisF, std::vector<std::string>& v_siteGPENom, std::vector<std::string>& v_siteCulture, std::vector<std::string>& v_siteElevage,
        std::vector<double>& v_lat, std::vector<double>& v_long, double long_min, double long_max, double lat_min, double lat_max)
    {
      //Open the File
        for (size_t k = 0; k < v_filename.size(); k++)
        {
            std::ifstream in(v_filename[k].c_str());
            std::ostringstream os;
            std::string str;
            int col = 0;
            std::string s_site, s_siteGPE, s_siteGPENom, s_cleCompos, s_desCompos, s_siteElevage, s_siteCulture;
            std::string s_source, s_nomLieu, s_type, s_codeSCIAN;
            std::string s_statut;
            double d_lat, d_long, d_prelevement, d_coef_consommation;
            int i_critere, i_continue, i_moisD, i_moisF;
            int no_ligne = 0;

            bool valid = true;

            d_lat = 0.0;
            d_long = 0.0;
            d_prelevement = 0.0;
            d_coef_consommation = 0.0;
            i_critere = 0;
            i_continue = 0;
            i_moisD = 0;
            i_moisF = 0;

            try {
                // Check if object is valid
                if (!in)
                {
                    std::cout << "impossible d`ouvrir fichier prelevement eau: " << v_filename[k] << std::endl;
                }
                else
                {
                    // Read the next line from File untill it reaches the end.
                    while (getline_mod(in, str))
                    {
                        try {
                            string ligne;
                            istringstream tokenStream(str);
                            col = 0;
                            valid = true;
                            s_siteGPENom = "";
                            s_siteCulture = "";
                            s_siteElevage = "";

                            while (getline(tokenStream, ligne, ';')) //separe la ligne selon les colonnes
                            {
                                switch (col)
                                {

                                case 0:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_site = ligne;
                                    break;
                                case 1:
                                    if (ligne.empty())
                                        ligne = "-1";
                                    s_siteGPE = ligne;
                                    break;
                                case 2:
                                    ligne = findVirgule(ligne);
                                    if (isNumber(ligne) && !(ligne.empty()))
                                    {
                                        d_lat = (stod(ligne));
                                        if (!(d_lat >= lat_min && d_lat <= lat_max))
                                            valid = false;
                                    }
                                    else { valid = false; }
                                    break;

                                case 3:
                                    ligne = findVirgule(ligne);
                                    if (isNumber(ligne) && !(ligne.empty()))
                                    {
                                        d_long = (stod(ligne));
                                        if (!(d_long >= long_min && d_long <= long_max))
                                            valid = false;
                                    }
                                    else { valid = false; }
                                    break;

                                case 4:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_cleCompos = ligne;
                                    break;

                                case 5:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_desCompos = ligne;
                                    break;

                                case 6:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_nomLieu = ligne;
                                    break;

                                case 7:
                                    if (ligne.empty() || !isNumber(ligne))
                                        ligne = "-1";
                                    i_critere = stoi(ligne);
                                    break;

                                case 8:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_source = ligne;
                                    break;

                                case 9:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_type = ligne;
                                    break;

                                case 10:
                                    ligne = findVirgule(ligne);
                                    if (!ligne.empty())
                                    {
                                        if (isNumber(ligne))
                                        {
                                            d_prelevement = (stod(ligne));
                                        }
                                    }

                                case 11:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_codeSCIAN = ligne;
                                    break;

                                case 12:
                                    ligne = findVirgule(ligne);
                                    if (!ligne.empty())
                                    {
                                        if (isNumber(ligne))
                                        {
                                            d_coef_consommation = (stod(ligne));
                                        }
                                    }

                                case 13:
                                    if (ligne.empty() || !isNumber(ligne))
                                        ligne = "-1";
                                    i_continue = stoi(ligne);
                                    break;

                                case 14:
                                    if (ligne.empty() || !isNumber(ligne))
                                        ligne = "-1";
                                    i_moisD = stoi(ligne);
                                    break;

                                case 15:
                                    if (ligne.empty() || !isNumber(ligne))
                                        ligne = "-1";
                                    i_moisF = stoi(ligne);
                                    break;

                                case 16:
                                    if (ligne.empty())
                                        ligne = " ";
                                    s_siteGPENom = ligne;
                                    break;

                                case 17:
                                    if (ligne.empty())
                                        ligne = " ";
                                    s_siteElevage = ligne;
                                    break;
                                case 18:
                                    if (ligne.empty())
                                        ligne = "-";
                                    s_siteCulture = ligne;
                                    break;
                                }
                                col++;
                            }
                            if (valid)
                            {
                                v_site.push_back(s_site);
                                v_siteGPE.push_back(s_siteGPE);
                                v_lat.push_back(d_lat);
                                v_long.push_back(d_long);
                                v_cleCompos.push_back(s_cleCompos);
                                v_DesCompo.push_back(s_desCompos);
                                v_nomLieu.push_back(s_nomLieu);
                                v_critere.push_back(i_critere);
                                v_source.push_back(s_source);
                                v_type.push_back(s_type);
                                v_prelevement.push_back(d_prelevement);
                                v_codeSCIAN.push_back(s_codeSCIAN);
                                v_coef_consommation.push_back(d_coef_consommation);
                                v_continue.push_back(i_continue);
                                v_moisD.push_back(i_moisD);
                                v_moisF.push_back(i_moisF);
                                v_siteGPENom.push_back(s_siteGPENom);
                                v_siteElevage.push_back(s_siteElevage);
                                v_siteCulture.push_back(s_siteCulture);
                            }
                        }
                        catch (...)
                        {
                            std::cout << "erreur lors de la lecture du fichier: " << v_filename[k] << ": ligne " << no_ligne+1 << ": " << str << endl << endl;
                        }

                        no_ligne++;
                    }
                }
            }
            catch (...)
            {
                std::cout << "erreur lors de la lecture du fichier: " << v_filename[k] << endl;
            }
        }
    }

    void PRELEVEMENTS_SITE::ajoutDonneeEAU(const std::string& p_siteGPEEAU, const std::string& p_cleComposEAU, const std::string& p_DesCompoEAU, const std::string& p_nomLieuEAU, const std::string& p_typeEAU, 
        double p_prelevementEAU, const std::string& p_codeSCIANEAU, int p_continueEAU, int p_moisDEAU, int p_moisFEAU, const std::string& p_siteGPENomEAU, const  std::string& p_siteElevage, const std::string& p_siteCulture)
    {
       m_siteGPEEAU = p_siteGPEEAU;
       m_cleComposEAU = p_cleComposEAU;  
       m_DesCompoEAU = p_DesCompoEAU; 
       m_nomLieuEAU =  p_nomLieuEAU; 
       m_typeEAU =  p_typeEAU;
       m_prelevementEAU = p_prelevementEAU;
       m_codeSCIANEAU = p_codeSCIANEAU; 
       m_continueEAU =  p_continueEAU; 
       m_moisDEAU = p_moisDEAU; 
       m_moisFEAU =  p_moisFEAU; 
       m_siteGPENomEAU = p_siteGPENomEAU;
       m_siteElevageEAU = p_siteElevage;
       m_siteCultureEAU = p_siteCulture;
    }

    void PRELEVEMENTS_SITE::erase_indexEAU(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<std::string>& v_site, std::vector<std::string>& v_siteGPE, std::vector<std::string>& v_cleCompos, std::vector<std::string>& v_DesCompo,
        std::vector<std::string>& v_nomLieu, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement, std::vector<std::string>& v_codeSCIAN, std::vector<double>& v_coef_consommation, std::vector<int>& v_continue,
        std::vector<int>& v_moisD, std::vector<int>& v_moisF, std::vector<std::string>& v_siteGPENom, std::vector<std::string>& v_siteCulture, std::vector<std::string>& v_siteElevage, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<int>& v_index)
    {
        std::sort(v_index.begin(), v_index.end());
        int i = 0;
        int index;
        while (v_index.size() > 0)
        {
            index = v_index[0] - i;
            v_uhrh.erase(v_uhrh.begin() + index);
            v_troncon.erase(v_troncon.begin() + index);
            v_idOccsol.erase(v_idOccsol.begin() + index);
            v_sOccsol.erase(v_sOccsol.begin() + index);
            v_site.erase(v_site.begin() + index);
            v_siteGPE.erase(v_siteGPE.begin() + index);  
            v_cleCompos.erase(v_cleCompos.begin() + index); 
            v_DesCompo.erase(v_DesCompo.begin() + index);
            v_nomLieu.erase(v_nomLieu.begin() + index); 
            v_critere.erase(v_critere.begin() + index);  
            v_source.erase(v_source.begin() + index); 
            v_type.erase(v_type.begin() + index);  
            v_prelevement.erase(v_prelevement.begin() + index);
            v_codeSCIAN.erase(v_codeSCIAN.begin() + index); 
            v_coef_consommation.erase(v_coef_consommation.begin() + index); 
            v_continue.erase(v_continue.begin() + index);
            v_moisD.erase(v_moisD.begin() + index); 
            v_moisF.erase(v_moisF.begin() + index);  
            v_siteGPENom.erase(v_siteGPENom.begin() + index);
            v_siteCulture.erase(v_siteCulture.begin() + index);
            v_siteElevage.erase(v_siteElevage.begin() + index);
            v_lat.erase(v_lat.begin() + index);
            v_long.erase(v_long.begin() + index);
            v_index.erase(v_index.begin());
            i++;

        }
    }

    std::string PRELEVEMENTS_SITE::printDonneEAU(size_t numero_site)
    {
        std::ostringstream os;
        os << numero_site << ";" << m_siteGPE <<";" << m_siteElevageEAU << ";"<< m_siteCultureEAU << ";" << printDonneeMulti() << ";" << m_cleComposEAU << ";" << m_DesCompoEAU << ";" << m_nomLieuEAU << ";" << m_typeEAU << ";" << m_prelevementEAU << ";" << m_codeSCIANEAU << ";" << m_continueEAU << ";" << m_moisDEAU << ";" << m_moisFEAU << ";" << m_siteGPENomEAU << "\n";
        return os.str();

    }

    std::string PRELEVEMENTS_SITE::printLegendeEAU()
    {
        std::string str = "#SITE;#SITE GPE;# SITE_ELEVAGE;#SITE_CULTURE;" + printLegendMulti() + ";CLE_COMPOS; DESC_COMPO; nom_lieu;TYPE;PRELEV(l/j);CODE_SCIAN;CONTINUE;MOIS D;MOIS FIN;NOM_SITE GPE";
        return str;
   }


    std::string PRELEVEMENTS_SITE::getCleCompos()
    {
        return m_cleComposEAU;
    }


    //-----------------------------------------------------------------SITE SIH --------------------------------------------------------------------------//


    void PRELEVEMENTS_SITE::lireFichierPrelvementSIH(const std::vector<std::string>& v_filename, std::vector<int>& v_id, std::vector<std::string>& v_idendifiant, std::vector<std::string>& v_proprio, std::vector<std::string>& v_adresse,
        std::vector<std::string>& v_municipalite, std::vector<std::string>& v_profondeur, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement,
        std::vector<double>& v_coef_cons, std::vector<double>& v_lat, std::vector<double>& v_long, double long_min, double long_max, double lat_min, double lat_max)
    {
        //Open the File
        for (size_t k = 0; k < v_filename.size(); k++)
        {
            std::ifstream in(v_filename[k].c_str());
            std::ostringstream os;
            std::string str;
            int col = 0;
            std::string s_identifiant, s_proprio, s_adresse, s_municipalite, s_profondeur, s_source, s_type;
            double d_lat, d_long, d_prelevement, d_coef_cons;
            int i_id, i_critere;

            bool valid = true;
            int no_ligne = 0;

            try {
                // Check if object is valid
                if (!in)
                {
                    std::cout << "Cannot open the file : " << v_filename[k] << std::endl;
                }
                else
                {
                    // Read the next line from File untill it reaches the end.
                    while (getline_mod(in, str))
                    {
                        try {
                            string ligne;
                            istringstream tokenStream(str);
                            col = 1;
                            valid = true;
                            if (no_ligne > 0)
                            {
                                while (getline(tokenStream, ligne, ';')) //separe la ligne selon les colonnes
                                {

                                    switch (col)
                                    {
                                    case 1:
                                        if (ligne.empty() || !isNumber(ligne))
                                            ligne = "-1";
                                        i_id = stoi(ligne);
                                        break;

                                    case 2:
                                        ligne = findVirgule(ligne);
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            d_lat = (stod(ligne));
                                            if (!(d_lat >= lat_min && d_lat <= lat_max))
                                                valid = false;
                                        }
                                        else { valid = false; }
                                        break;

                                    case 3:
                                        ligne = findVirgule(ligne);
                                        if (isNumber(ligne) && !(ligne.empty()))
                                        {
                                            d_long = (stod(ligne));
                                            if (!(d_long >= long_min && d_long <= long_max))
                                                valid = false;
                                        }
                                        else { valid = false; }
                                        break;

                                    case 4:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_identifiant = ligne;
                                        break;

                                    case 5:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_proprio = ligne;
                                        break;
                                    case 6:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_adresse = ligne;
                                        break;
                                    case 7:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_municipalite = ligne;
                                        break;
                                    case 8:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_profondeur = ligne;
                                        break;
                                    case 9:
                                        if (ligne.empty() || !isNumber(ligne))
                                            ligne = "-1";
                                        i_critere = stoi(ligne);
                                        break;
                                    case 10:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_source = ligne;
                                        break;
                                    case 11:
                                        if (ligne.empty())
                                            ligne = "-";
                                        s_type = ligne;
                                        break;
                                    case 12:
                                        ligne = findVirgule(ligne);
                                        if (!ligne.empty())
                                        {
                                            if (isNumber(ligne))
                                            {
                                                RemoveWhiteSpacePluriel(ligne);
                                                d_prelevement = (stod(ligne));
                                            }
                                        }
                                    case 13:
                                        ligne = findVirgule(ligne);
                                        if (!ligne.empty())
                                        {
                                            if (isNumber(ligne))
                                            {
                                                RemoveWhiteSpacePluriel(ligne);
                                                d_coef_cons = (stod(ligne));
                                            }
                                        }
                                    }
                                    col++;
                                }
                                if (valid)
                                {
                                    v_idendifiant.push_back(s_identifiant);
                                    v_proprio.push_back(s_proprio);
                                    v_adresse.push_back(s_adresse);
                                    v_municipalite.push_back(s_municipalite);
                                    v_profondeur.push_back(s_profondeur);
                                    v_source.push_back(s_source);
                                    v_type.push_back(s_type);
                                    v_lat.push_back(d_lat);
                                    v_long.push_back(d_long);
                                    v_prelevement.push_back(d_prelevement);
                                    v_coef_cons.push_back(d_coef_cons);
                                    v_id.push_back(i_id);
                                    v_critere.push_back(i_critere);
                                }
                            }
                        }
                        catch (...)
                        {
                            std::cout << "No ligne :" << no_ligne << " impossible to read" << endl;
                        }
                    no_ligne++;
                    }
                }
            }
            catch (...)
            {
                std::cout << "Impossible to read the file" << endl;
            }
        }
    }
    
    void PRELEVEMENTS_SITE::erase_indexSIH(std::vector<int>& v_uhrh, std::vector<int>& v_troncon, std::vector<int>& v_idOccsol, std::vector<std::string>& v_sOccsol, std::vector<int>& v_id, std::vector<std::string>& v_identifiant, std::vector<std::string>& v_proprio, std::vector<std::string>& v_adresse,
        std::vector<std::string>& v_municipalite, std::vector<std::string>& v_profondeur, std::vector<int>& v_critere, std::vector<std::string>& v_source, std::vector<std::string>& v_type, std::vector<double>& v_prelevement,
        std::vector<double>& v_coef_cons, std::vector<double>& v_lat, std::vector<double>& v_long, std::vector<int>& v_index)
    {
        std::sort(v_index.begin(), v_index.end());
        int i = 0;
        int index;
        while (v_index.size() > 0)
        {
            index = v_index[0] - i;
            v_uhrh.erase(v_uhrh.begin() + index);
            v_troncon.erase(v_troncon.begin() + index);
            v_idOccsol.erase(v_idOccsol.begin() + index);
            v_sOccsol.erase(v_sOccsol.begin() + index);
            v_identifiant.erase(v_identifiant.begin() + index);
            v_proprio.erase(v_proprio.begin() + index);
            v_adresse.erase(v_adresse.begin() + index);
            v_municipalite.erase(v_municipalite.begin() + index);
            v_profondeur.erase(v_profondeur.begin() + index);
            v_source.erase(v_source.begin() + index);
            v_type.erase(v_type.begin() + index);
            v_lat.erase(v_lat.begin() + index);
            v_long.erase(v_long.begin() + index);
            v_prelevement.erase(v_prelevement.begin() + index);
            v_coef_cons.erase(v_coef_cons.begin() + index);
            v_id.erase(v_id.begin() + index);
            v_critere.erase(v_critere.begin() + index);
            v_index.erase(v_index.begin());
            i++;
        }
    }

    void PRELEVEMENTS_SITE::ajouteDonneeSIH(string p_idendifiant, int p_siteGPE, std::string p_proprio, std::string p_adresse, std::string p_municipalite, std::string p_profondeur, std::string p_type, double p_prelevement)
    {
         m_idendifiant = p_idendifiant;
         m_siteGPE = p_siteGPE;
         m_proprio = p_proprio;
         m_adresse = p_adresse;
         m_municipalite = p_municipalite;
         m_profondeur = p_profondeur;
         m_type =  p_type;
         m_prelevement = p_prelevement;
    }

    std::string PRELEVEMENTS_SITE::printDonneSIH(size_t numero_site)
    {
        std::ostringstream os;
        os << numero_site << ";" << m_siteGPE << ";" << printDonneeMulti() << ";" << m_idendifiant << ";" << m_proprio << ";" << m_adresse << ";" << m_municipalite << ";" << m_profondeur << ";" << m_type << ";" << m_prelevement << "\n";
        return os.str();
    }

    std::string PRELEVEMENTS_SITE::printLegendeSIH()
    {
        std::string str = "#SITE;#SITE GPE;" + printLegendMulti() + ";IDENTIFIANT;PROPRIO;ADRESSE;MUNICIPALITE;PROFONDEUR;TYPE;PRELEV(l/j)";
        return str;
    }


    //-----------------------------------------------------------UTIL--------------------------------------------------------------------------------//


    int PRELEVEMENTS_SITE::AlmostEgalString(std::vector<std::string> str1, std::string str2, float critere, bool elementDelete)
    {
        string str_tolook;
        string new_str;
        std::size_t found, i, middle;
        int size;// = str1.size();
        int lenght_newstring = 4;
        int poids = 0;
        int j = 0;
        vector<float> porportion;
        // int end_index;
        RemoveWhiteSpacePluriel(str2);

        if (elementDelete)
        {
            if ((found = str2.find("cooperativ")) != std::string::npos)
            {
                str2.erase(found, found + 10);
            }

            if ((found = str2.find("inc")) != std::string::npos)
            {
                str2.erase(found, found + 3);
            }

            if ((found = str2.find("societe")) != std::string::npos)
            {
                str2.erase(found, found + 7);
            }
        }

        for (i = 0; i < str1.size(); i++)
        {
            str_tolook = str1[i];
            RemoveWhiteSpacePluriel(str_tolook);

            if (elementDelete)
            {
                if ((found = str_tolook.find("cooperativ")) != std::string::npos)
                {
                    str_tolook.erase(found, found + 10);
                }

                if ((found = str_tolook.find("inc")) != std::string::npos)
                {
                    str_tolook.erase(found, found + 3);
                }

                if ((found = str_tolook.find("societe")) != std::string::npos)
                {
                    str_tolook.erase(found, found + 7);
                }
            }

            //std::cout << str_tolook << endl;
            poids = 0;
            j = 0;
            //   end_index = 1;

            size = (int) str_tolook.size();
            if (size > 0)
            {
                //regarde toute la string au dÈbut si pareil on arrÍte
                found = str2.find(str_tolook);
                if (found != std::string::npos)
                {
                    poids = 1;
                    j = 1;
                }

                else if (size > 4)
                {
                    middle = size / 2;

                    if (size % 2 != 0) //impaire
                        middle += 1;

                    if (str1[i].size() == 5)
                    {
                        lenght_newstring = 3;
                    }
                    while (str_tolook.size() > middle)
                    {
                        new_str = str_tolook.substr(0, lenght_newstring);
                        found = str2.find(new_str);
                        if (found != std::string::npos)
                            poids++;
                        str_tolook.erase(str_tolook.begin());
                        j++;
                    }
                    // porportion = (float)poids / size;

                    str_tolook = str1[i];
                    RemoveWhiteSpacePluriel(str_tolook);

                    while (str_tolook.size() > middle)
                    {
                        new_str = str_tolook.substr(str_tolook.size() - lenght_newstring, str_tolook.size() - 1);
                        found = str2.find(new_str);
                        if (found != std::string::npos)
                            poids++;
                        str_tolook.erase(str_tolook.size() - 1);
                        j++;
                    }
                }
            }
            if (j > 0)
                porportion.push_back((float)poids / j);
            else
                porportion.push_back(-1);
        }

        vector<float> max_element = porportion;
        while (max_element.size() != 1)
        {
            if (max_element[0] < max_element[1])
                max_element.erase(max_element.begin());
            else
                max_element.erase(max_element.begin() + 1);
        }
        if (max_element[0] < critere)
            return -1;

        for (i = 0; i < porportion.size(); i++)
            if (porportion[i] == max_element[0]) {
                return static_cast<int>(i);
            }
        return -1;
    }


    void PRELEVEMENTS_SITE::RemoveWhiteSpacePluriel(std::string& str)
    {
        //white space
        std::string::iterator end_pos = std::remove(str.begin(), str.end(), ' ');
        str.erase(end_pos, str.end());
        //s
        end_pos = std::remove(str.begin(), str.end(), 's');
        str.erase(end_pos, str.end());

        //x
        end_pos = std::remove(str.begin(), str.end(), 'x');
        str.erase(end_pos, str.end());

        //l
        end_pos = std::remove(str.begin(), str.end(), 'l');
        str.erase(end_pos, str.end());

        //'
        end_pos = std::remove(str.begin(), str.end(), '\'');
        str.erase(end_pos, str.end());

        //lowercase
		boost::algorithm::to_lower(str);
    }


    std::string PRELEVEMENTS_SITE::findVirgule(std::string data)
    {
        // Get the first occurrence
        string virgule = ",";
        string point = ".";
        size_t pos = data.find(virgule);
        // Repeat till end is reached
        while (pos != std::string::npos)
        {
            // Replace this occurrence of Sub String
            data.replace(pos, virgule.size(), point);
            // Get the next occurrence from the current position
            pos = data.find(virgule, pos + point.size());
        }
        return data;
    }

    bool PRELEVEMENTS_SITE::isNumber(std::string token)
    {
        return std::regex_match(token, std::regex(("((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?")));
    }

    std::string PRELEVEMENTS_SITE::replaceFirstOccurrence(std::string& s, const std::string& toReplace, const std::string& replaceWith)
    {
        std::size_t pos = s.find(toReplace);
        if (pos == std::string::npos) return s; // pas de trouver
        return s.replace(pos, toReplace.length(), replaceWith); // trouver
    }

    void PRELEVEMENTS_SITE::RemoveNoASCIILetter(std::string& str)
    {
        size_t i;

        std::vector<std::string> caratere_speciaux;

        caratere_speciaux.push_back("√Ä");	caratere_speciaux.push_back("√Å");	caratere_speciaux.push_back("√Ç");	caratere_speciaux.push_back("√É");	
        caratere_speciaux.push_back("√Ñ");	caratere_speciaux.push_back("√Ö");	caratere_speciaux.push_back("√Ü");	caratere_speciaux.push_back("√á");	
        caratere_speciaux.push_back("√à");	caratere_speciaux.push_back("√â");	caratere_speciaux.push_back("√ä");	caratere_speciaux.push_back("√ã");	
        caratere_speciaux.push_back("√å");	caratere_speciaux.push_back("√ç");	caratere_speciaux.push_back("√é");	caratere_speciaux.push_back("√è");	
        caratere_speciaux.push_back("√ê");	caratere_speciaux.push_back("√ë");	caratere_speciaux.push_back("√í");	caratere_speciaux.push_back("√ì");	
        caratere_speciaux.push_back("√î");	caratere_speciaux.push_back("√ï");	caratere_speciaux.push_back("√ñ");	caratere_speciaux.push_back("√ó");	
        caratere_speciaux.push_back("√ò");	caratere_speciaux.push_back("√ô");	caratere_speciaux.push_back("√ö");	caratere_speciaux.push_back("√õ");	
        caratere_speciaux.push_back("√ú");	caratere_speciaux.push_back("√ù");	caratere_speciaux.push_back("√û");	caratere_speciaux.push_back("√ü");	
        caratere_speciaux.push_back("√†");	caratere_speciaux.push_back("√°");	caratere_speciaux.push_back("√¢");	caratere_speciaux.push_back("√£");	
        caratere_speciaux.push_back("√§");	caratere_speciaux.push_back("√•");	caratere_speciaux.push_back("√¶");	caratere_speciaux.push_back("√ß");	
        caratere_speciaux.push_back("√®");	caratere_speciaux.push_back("√©");	caratere_speciaux.push_back("√™");	caratere_speciaux.push_back("√´");	
        caratere_speciaux.push_back("√¨");	caratere_speciaux.push_back("√≠");	caratere_speciaux.push_back("√Æ");	caratere_speciaux.push_back("√Ø");	
        caratere_speciaux.push_back("√∞");	caratere_speciaux.push_back("√±");	caratere_speciaux.push_back("√≤");	caratere_speciaux.push_back("√≥");	
        caratere_speciaux.push_back("√¥");	caratere_speciaux.push_back("√µ");	caratere_speciaux.push_back("√∂");	caratere_speciaux.push_back("√∑");	
        caratere_speciaux.push_back("√∏");	caratere_speciaux.push_back("√π");	caratere_speciaux.push_back("√∫");	caratere_speciaux.push_back("√ª");	
        caratere_speciaux.push_back("√º");	caratere_speciaux.push_back("√Ω");	caratere_speciaux.push_back("√æ");	caratere_speciaux.push_back("√ø");

        std::vector<std::string> caratere_speciaux2;
        caratere_speciaux2.push_back("¿");	caratere_speciaux2.push_back("¡");	caratere_speciaux2.push_back("¬");	caratere_speciaux2.push_back("√");	
        caratere_speciaux2.push_back("ƒ");	caratere_speciaux2.push_back("≈");	caratere_speciaux2.push_back("∆");	caratere_speciaux2.push_back("«");	
        caratere_speciaux2.push_back("»");	caratere_speciaux2.push_back("…");	caratere_speciaux2.push_back(" ");	caratere_speciaux2.push_back("À");	
        caratere_speciaux2.push_back("Ã");	caratere_speciaux2.push_back("Õ");	caratere_speciaux2.push_back("Œ");	caratere_speciaux2.push_back("œ");	
        caratere_speciaux2.push_back("–");	caratere_speciaux2.push_back("—");	caratere_speciaux2.push_back("“");	caratere_speciaux2.push_back("”");	
        caratere_speciaux2.push_back("‘");	caratere_speciaux2.push_back("’");	caratere_speciaux2.push_back("÷");	caratere_speciaux2.push_back("◊");	
        caratere_speciaux2.push_back("ÿ");	caratere_speciaux2.push_back("Ÿ");	caratere_speciaux2.push_back("⁄");	caratere_speciaux2.push_back("€");	
        caratere_speciaux2.push_back("‹");	caratere_speciaux2.push_back("›");	caratere_speciaux2.push_back("ﬁ");	caratere_speciaux2.push_back("ﬂ");	
        caratere_speciaux2.push_back("‡");	caratere_speciaux2.push_back("·");	caratere_speciaux2.push_back("‚");	caratere_speciaux2.push_back("„");	
        caratere_speciaux2.push_back("‰");	caratere_speciaux2.push_back("Â");	caratere_speciaux2.push_back("Ê");	caratere_speciaux2.push_back("Á");	
        caratere_speciaux2.push_back("Ë");	caratere_speciaux2.push_back("È");	caratere_speciaux2.push_back("Í");	caratere_speciaux2.push_back("Î");	
        caratere_speciaux2.push_back("Ï");	caratere_speciaux2.push_back("Ì");	caratere_speciaux2.push_back("Ó");	caratere_speciaux2.push_back("Ô");	
        caratere_speciaux2.push_back("");	caratere_speciaux2.push_back("Ò");	caratere_speciaux2.push_back("Ú");	caratere_speciaux2.push_back("Û");	
        caratere_speciaux2.push_back("Ù");	caratere_speciaux2.push_back("ı");	caratere_speciaux2.push_back("ˆ");	caratere_speciaux2.push_back("˜");	
        caratere_speciaux2.push_back("¯");	caratere_speciaux2.push_back("˘");	caratere_speciaux2.push_back("˙");	caratere_speciaux2.push_back("˚");	
        caratere_speciaux2.push_back("¸");	caratere_speciaux2.push_back("˝");	caratere_speciaux2.push_back("˛");	caratere_speciaux2.push_back("ˇ");

        std::vector<std::string> caractere_normal;
        caractere_normal.push_back("A");	caractere_normal.push_back("A");	caractere_normal.push_back("A");	caractere_normal.push_back("A");	
        caractere_normal.push_back("A");	caractere_normal.push_back("A");	caractere_normal.push_back("A");	caractere_normal.push_back("C");	
        caractere_normal.push_back("E");	caractere_normal.push_back("E");	caractere_normal.push_back("E");	caractere_normal.push_back("E");	
        caractere_normal.push_back("I");	caractere_normal.push_back("I");	caractere_normal.push_back("I");	caractere_normal.push_back("I");	
        caractere_normal.push_back("D");	caractere_normal.push_back("N");	caractere_normal.push_back("O");	caractere_normal.push_back("O");	
        caractere_normal.push_back("O");	caractere_normal.push_back("O");	caractere_normal.push_back("O");	caractere_normal.push_back("X");	
        caractere_normal.push_back("O");	caractere_normal.push_back("U");	caractere_normal.push_back("U");	caractere_normal.push_back("U");	
        caractere_normal.push_back("U");	caractere_normal.push_back("Y");	caractere_normal.push_back("p");	caractere_normal.push_back("B");	
        caractere_normal.push_back("a");	caractere_normal.push_back("a");	caractere_normal.push_back("a");	caractere_normal.push_back("a");	
        caractere_normal.push_back("A");	caractere_normal.push_back("a");	caractere_normal.push_back("ae");	caractere_normal.push_back("c");	
        caractere_normal.push_back("e");	caractere_normal.push_back("e");	caractere_normal.push_back("e");	caractere_normal.push_back("e");	
        caractere_normal.push_back("i");	caractere_normal.push_back("i");	caractere_normal.push_back("i");	caractere_normal.push_back("i");	
        caractere_normal.push_back("o");	caractere_normal.push_back("n");	caractere_normal.push_back("o");	caractere_normal.push_back("o");	
        caractere_normal.push_back("o");	caractere_normal.push_back("o");	caractere_normal.push_back("o");	caractere_normal.push_back("-");	
        caractere_normal.push_back("o");	caractere_normal.push_back("u");	caractere_normal.push_back("u");	caractere_normal.push_back("u");	
        caractere_normal.push_back("u");	caractere_normal.push_back("y");	caractere_normal.push_back("p");	caractere_normal.push_back("y");

        for (std::string::size_type k = 0; k < caratere_speciaux.size(); k++)
        {
            for (i = 0; i < str.length(); i++) {
                if (str.substr(i, caratere_speciaux[k].length()) == caratere_speciaux[k])
                {
                    str.replace(i, caratere_speciaux[k].length(), caractere_normal[k]); // remplacer
                }
            }
        }

        for (std::string::size_type k = 0; k < caratere_speciaux2.size(); k++)
        {
            for (i = 0; i < str.length(); i++) {
                if (str.substr(i, caratere_speciaux2[k].length()) == caratere_speciaux2[k])
                {
                    str.replace(i, caratere_speciaux2[k].length(), caractere_normal[k]); // remplacer
                }
            }
        }
    }

}
