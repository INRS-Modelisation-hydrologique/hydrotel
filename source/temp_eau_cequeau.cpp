#include "temp_eau_cequeau.hpp"

#include "troncons.hpp"
#include "erreur.hpp"
#include "riviere.hpp"
#include "lac.hpp"
#include "constantes.hpp"
#include "rayonnement_net.hpp"
#include "sim_hyd.hpp"
#include "util.hpp"
#include "version.hpp"
#include "noeud.hpp"
#include "projections.hpp"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <fstream>


using namespace std;


namespace HYDROTEL
{


    TEMP_EAU_CEQUEAU::TEMP_EAU_CEQUEAU(SIM_HYD& sim_hyd)
        : TEMP_EAU(sim_hyd, "TEMP EAU CEQUEAU")
        , _C(4.187f) 
        , _alb(0.06f)
        , _emissivite_eau(0.95f)
        , _chal_latente(2480.0f)
    {
    }


    TEMP_EAU_CEQUEAU::~TEMP_EAU_CEQUEAU()
    {
    }


    void TEMP_EAU_CEQUEAU::ChangeNbParams(const ZONES& zones)
    {
		TEMP_EAU::ChangeNbParams(zones);

        _Csolaire.assign(_nbTroncons, 1.0f);
        _Cir.assign(_nbTroncons, 1.0f);
        _Cchaleur.assign(_nbTroncons, 1.0f);
        _Cevapo.assign(_nbTroncons, 1.0f);
        _gel.assign(zones.PrendreNbZone(), 0.01f);
        _tempbase.assign(zones.PrendreNbZone(), 4.0f);
        _temp_annuelle.assign(_nbTroncons, 4.0f);
        _coefVarTemp1.assign(_nbTroncons, 1.0f);
        _coefVarTemp2.assign(_nbTroncons, 1.0f);
        _vitvent.assign(_nbTroncons, 7.2f); //km/h

        //rayonnement net
        vector<float> vTemp;
        vTemp.resize(_nbTroncons, VALEUR_MANQUANTE);
        _vRaTr.resize(365, vTemp);

        _fCoeffATransmissiviteAtmosTr.assign(_nbTroncons, 0.9232f);
        _fCoeffBTransmissiviteAtmosTr.assign(_nbTroncons, 0.1121f);
        _fCoeffCTransmissiviteAtmosTr.assign(_nbTroncons, 0.8038f);

        _fCoeffAEmissiviteAtmosTr.assign(_nbTroncons, 0.7363f);
        _fCoeffBEmissiviteAtmosTr.assign(_nbTroncons, 0.0009f);
        _fCoeffCEmissiviteAtmosTr.assign(_nbTroncons, 0.9828f);

        _rayonnementNet = &_sim_hyd._rayonnementNet;
        if (_rayonnementNet == NULL)
            throw ERREUR("TEMP_EAU_CEQUEAU: error: _rayonnementNet == NULL");
    }


    void TEMP_EAU_CEQUEAU::Initialise()
    {
        if(_sim_hyd.PrendrePasDeTemps() != 24)
        {
            Log("");
            throw ERREUR("TEMP_EAU_CEQUEAU: the computation of the net radiation at the surface requires a simulation timestep of 24 hours.");
        }

        if(_sim_hyd._troncons_tries.empty())
            _sim_hyd.TrieTroncons();

        _fValSigma = SIGMA * pow(10.0f, -6.0f);

        _hauteurPasPrecedent.assign(_sim_hyd.PrendreTroncons().PrendreNbTroncon(), -999.0f);
        _tempAirPasPrecedent.assign(_sim_hyd.PrendreTroncons().PrendreNbTroncon(), -999.0f);

        _pCoordTrans = new TRANSFORME_COORDONNEE(_sim_hyd.PrendreZones().PrendreGrille().PrendreProjection(), PROJECTIONS::LONGLAT_WGS84());
        
		TEMP_EAU::Initialise();
    }


    void TEMP_EAU_CEQUEAU::LectureParametres()
    {
        if(_sim_hyd._fichierParametreGlobal)
        {
            if(!LectureParametresFichierGlobal())	    //lecture du fichier de parametre global si l'option est activé
            {
                SauvegardeParametresFichierGlobal();    //si param introuvable ajoute param par défaut dans le fichier
                Log("");
                Log("TEMP EAU CEQUEAU: model parameters was not found in the global parameter file: default parameters have been added to the file: " + _sim_hyd._nomFichierParametresGlobal);
                Log("");
            }
        }
        else
        {
            if(boost::filesystem::exists(_nom_fichier_parametres_zones))
                LectureParametresZones();
            else
            {
                SauvegardeParametresZones();
                Log("");
                Log("TEMP EAU CEQUEAU: the parameter file for RHHUs does not exist: the file has been created with default values: " + _nom_fichier_parametres_zones);
                Log("");
            }

            if(boost::filesystem::exists(_nom_fichier_parametres_troncons))
                LectureParametresTroncons();
            else
            {
                SauvegardeParametresTroncons();
                Log("");
                Log("TEMP EAU CEQUEAU: the parameter file for rivers reach does not exist: the file has been created with default values: " + _nom_fichier_parametres_troncons);
                Log("");
            }
        }
    }


    void TEMP_EAU_CEQUEAU::LectureParametresZones()
    {
        size_t index_zone, compteur;
        int iIdent;
        string cle, valeur, ligne;
        int no_ligne = 6;

        ZONES& zones = _sim_hyd.PrendreZones();

        std::ifstream fichier(_nom_fichier_parametres_zones);
        if (!fichier.is_open()) {
            throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_zones);
        }

        getline_mod(fichier, ligne);
        getline_mod(fichier, ligne);
        getline_mod(fichier, ligne);
        getline_mod(fichier, ligne);
        getline_mod(fichier, ligne);

        for (compteur = 0; compteur < zones.PrendreNbZone(); compteur++)
        {
            getline_mod(fichier, ligne);
            auto vValeur = extrait_fvaleur(ligne, ";");

            if (vValeur.size() < 3)
            {
                fichier.close();
                throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_zones, no_ligne, "Invalid column count.");
            }

            iIdent = static_cast<int>(vValeur[0]);
            index_zone = zones.IdentVersIndex(iIdent);

            _tempbase[index_zone] = vValeur[1];
            _gel[index_zone] = vValeur[2];

            ++no_ligne;

        }
        fichier.close();
    }


    void TEMP_EAU_CEQUEAU::LectureParametresTroncons()
    {
        size_t index_troncon, compteur;
        string cle, valeur, ligne;
        int iIdent;
        int no_ligne = 5;

       
        TRONCONS& troncons = _sim_hyd.PrendreTroncons();

        std::ifstream fichier(_nom_fichier_parametres_troncons);
        if (!fichier.is_open()) {
            throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_troncons);
        }

        getline_mod(fichier, ligne);
        getline_mod(fichier, ligne);   //saut de ligne
        getline_mod(fichier, ligne);
        getline_mod(fichier, ligne);   //saut de ligne

        getline_mod(fichier, ligne);
        auto vValeur = extrait_fvaleur(ligne, ";");

        if (vValeur.size() < 2)
        {
            fichier.close();
            throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_troncons, no_ligne, "Invalid column count.");
        }

        _C = vValeur[1];
        ++no_ligne;

        getline_mod(fichier, ligne);
        vValeur = extrait_fvaleur(ligne, ";");

        if (vValeur.size() < 2)
        {
            fichier.close();
            throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_troncons, no_ligne, "Invalid column count.");
        }

        _alb = vValeur[1];
        ++no_ligne;

        getline_mod(fichier, ligne);
        vValeur = extrait_fvaleur(ligne, ";");

        if (vValeur.size() < 2)
        {
            fichier.close();
            throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_troncons, no_ligne, "Invalid column count.");
        }

        _emissivite_eau = vValeur[1];
        ++no_ligne;

        getline_mod(fichier, ligne);
        vValeur = extrait_fvaleur(ligne, ";");

        if (vValeur.size() < 2)
        {
            fichier.close();
            throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_troncons, no_ligne, "Invalid column count.");
        }

        _chal_latente = vValeur[1];



        getline_mod(fichier, ligne);   //saut de ligne
        getline_mod(fichier, ligne);   // commentaire

        ++no_ligne;
        ++no_ligne;
        ++no_ligne;

        for (compteur = 0; compteur < _nbTroncons; compteur++)
        {
            getline_mod(fichier, ligne);
            vValeur = extrait_fvaleur(ligne, ";");

            if (vValeur.size() < 13)
            {
                fichier.close();
                throw ERREUR_LECTURE_FICHIER(_nom_fichier_parametres_troncons, no_ligne, "Invalid column count.");
            }

            iIdent = static_cast<int>(vValeur[0]);
            index_troncon = troncons.IdentVersIndex(iIdent);

            _Csolaire[index_troncon] = vValeur[1];
            _Cir[index_troncon] = vValeur[2];
            _Cevapo[index_troncon] = vValeur[3];
            _Cchaleur[index_troncon] = vValeur[4];
            _vitvent[index_troncon] = vValeur[5];

            _fCoeffATransmissiviteAtmosTr[index_troncon] = vValeur[6];
            _fCoeffBTransmissiviteAtmosTr[index_troncon] = vValeur[7];
            _fCoeffCTransmissiviteAtmosTr[index_troncon] = vValeur[8];

            _fCoeffAEmissiviteAtmosTr[index_troncon] = vValeur[9];
            _fCoeffBEmissiviteAtmosTr[index_troncon] = vValeur[10];
            _fCoeffCEmissiviteAtmosTr[index_troncon] = vValeur[11];
            _temp_annuelle[index_troncon] = vValeur[12];
            _coefVarTemp1[index_troncon] = vValeur[13];
            _coefVarTemp2[index_troncon] = vValeur[14];

            ++no_ligne;

        }
        fichier.close();

    }

    //-------------------------------------------------------
    //Return true if param was found, otherwise false
    bool TEMP_EAU_CEQUEAU::LectureParametresFichierGlobal()
    {
        ZONES& zones = _sim_hyd.PrendreZones();

        ifstream fichier(_sim_hyd._nomFichierParametresGlobal);
        if(!fichier)
            throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal);

        bool bParamFound = false;

        string cle, valeur, ligne;
        lire_cle_valeur(fichier, cle, valeur);

        if(cle != "PARAMETRES GLOBAL HYDROTEL VERSION")
            throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, 1);

        size_t nbGroupe, x, y, index_zone, index_troncon;
        float fVal;
        int no_ligne = 2;
        int ident;

        nbGroupe = _sim_hyd.PrendreNbGroupe();

        while(!fichier.eof())
        {
            getline_mod(fichier, ligne);
            if(ligne == "TEMP EAU CEQUEAU")
            {
                bParamFound = true;

                ++no_ligne;
                getline_mod(fichier, ligne);
                auto vValeur = extrait_fvaleur(ligne, ";");

                if(vValeur.size() != 4)
                    throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, no_ligne, "Invalid column count. TEMP EAU CEQUEAU.");

                _C = vValeur[0];
                _alb = vValeur[1];
                _emissivite_eau = vValeur[2];
                _chal_latente = vValeur[3];

                for(x = 0; x < nbGroupe; x++)
                {
                    ++no_ligne;
                    getline_mod(fichier, ligne);
                    vValeur = extrait_fvaleur(ligne, ";");

                    if(vValeur.size() != 15)
                        throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, no_ligne, "Invalid column count. TEMP EAU CEQUEAU.");

                    fVal = static_cast<float>(x);
                    if(fVal != vValeur[0])
                        throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, no_ligne, "ID de groupe invalide. TEMP EAU CEQUEAU. Les ID de groupe doivent etre en ordre croissant.");

                    for(y = 0; y < _sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
                    {
                        ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
                        index_zone = zones.IdentVersIndex(ident);
                        index_troncon = _sim_hyd.PrendreTroncons().IdentVersIndex(zones[index_zone].PrendreTronconAval()->PrendreIdent());

                        _Csolaire[index_troncon] = vValeur[1];
                        _Cir[index_troncon] = vValeur[2];
                        _Cevapo[index_troncon] = vValeur[3];
                        _Cchaleur[index_troncon] = vValeur[4];
                        _vitvent[index_troncon] = vValeur[5];
						_fCoeffATransmissiviteAtmosTr[index_troncon] = vValeur[6];
                        _fCoeffBTransmissiviteAtmosTr[index_troncon] = vValeur[7];
                        _fCoeffCTransmissiviteAtmosTr[index_troncon] = vValeur[8];
                        _fCoeffAEmissiviteAtmosTr[index_troncon] = vValeur[9];
                        _fCoeffBEmissiviteAtmosTr[index_troncon] = vValeur[10];
                        _fCoeffCEmissiviteAtmosTr[index_troncon] = vValeur[11];
                        _temp_annuelle[index_troncon] = vValeur[12];
                        _coefVarTemp1[index_troncon] = vValeur[13];
                        _coefVarTemp2[index_troncon] = vValeur[14];
                    }
                }

                ++no_ligne;
                getline_mod(fichier, ligne);    //ligne vide

                for(x = 0; x < nbGroupe; x++)
                {
                    ++no_ligne;
                    getline_mod(fichier, ligne);
                    vValeur = extrait_fvaleur(ligne, ";");

                    if(vValeur.size() != 3)
                        throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, no_ligne, "TEMP EAU CEQUEAU: invalid column count.");

                    fVal = static_cast<float>(x);
                    if(fVal != vValeur[0])
                        throw ERREUR_LECTURE_FICHIER(_sim_hyd._nomFichierParametresGlobal, no_ligne, "TEMP EAU CEQUEAU: invalid group index (first column): groups index must be in ascending order.");

                    for(y = 0; y < _sim_hyd.PrendreGroupeZone(x).PrendreNbZone(); y++)
                    {
                        ident = _sim_hyd.PrendreGroupeZone(x).PrendreIdent(y);
                        index_zone = zones.IdentVersIndex(ident);

                        _tempbase[index_zone] = vValeur[1];
                        _gel[index_zone] = vValeur[2];
                    }
                }
                break;
            }

            ++no_ligne;
        }

        fichier.close();
        return bParamFound;
    }


    //---------------------------------------------------------
    //Append default parameters to end of file
    void TEMP_EAU_CEQUEAU::SauvegardeParametresFichierGlobal()
    {
        ostringstream oss;
        ofstream file;
        string cle, valeur, ligne;
        size_t nbGroupe, x;

        file.open(_sim_hyd._nomFichierParametresGlobal, ios_base::app);
        if(!file)
            throw ERREUR_ECRITURE_FICHIER(_sim_hyd._nomFichierParametresGlobal);

        nbGroupe = _sim_hyd.PrendreNbGroupe();

        file << endl;
        file << "TEMP EAU CEQUEAU" << endl;

        //param troncons
        oss << _C << ";" << _alb << ";" << _emissivite_eau << ";" << _chal_latente;
        file << oss.str() << endl;

        for(x=0; x!=nbGroupe; x++)
        {
            oss.clear();
            oss.str("");

            oss << x << ";";    //index groupe

            if(_sim_hyd.PrendreGroupeZone(x).PrendreNbZone() != 0)
            {
                //index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_sim_hyd.PrendreGroupeZone(x).PrendreIdent(0));
                //index_troncon = _sim_hyd.PrendreTroncons().IdentVersIndex((_sim_hyd.PrendreZones()[index_zone]).PrendreTronconAval()->PrendreIdent());

                //on prend le troncon 1: le fichier etant absent ce sont les param par défaut pour tous les troncons (groupes)
                //cette fonction est appelé lorsque les parametres n'existe pas dans les fichiers...

                oss << _Csolaire[0] << ";";
                oss << _Cir[0] << ";";
                oss << _Cevapo[0] << ";";
                oss << _Cchaleur[0] << ";";
                oss << _vitvent[0] << ";";
                oss << _fCoeffATransmissiviteAtmosTr[0] << ";";
                oss << _fCoeffBTransmissiviteAtmosTr[0] << ";";
                oss << _fCoeffCTransmissiviteAtmosTr[0] << ";";
                oss << _fCoeffAEmissiviteAtmosTr[0] << ";";
                oss << _fCoeffBEmissiviteAtmosTr[0] << ";";
                oss << _fCoeffCEmissiviteAtmosTr[0] << ";";
                oss << _temp_annuelle[0] << ";";
                oss << _coefVarTemp1[0] << ";";
                oss << _coefVarTemp2[0];
            }
            else
            {
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999" << ";";
                oss << "-999";
            }

            file << oss.str() << endl;
        }

        file << endl;

        //param uhrh
        for(x=0; x!=nbGroupe; x++)
        {
            oss.clear();
            oss.str("");

            oss << x << ";";    //index groupe

            if(_sim_hyd.PrendreGroupeZone(x).PrendreNbZone() != 0)
            {
                //index_zone = _sim_hyd.PrendreZones().IdentVersIndex(_sim_hyd.PrendreGroupeZone(x).PrendreIdent(0));

                //on prend le uhrh 1: le fichier etant absent ce sont les param par défaut pour tous les troncons (groupes)
                //cette fonction est appelé lorsque les parametres n'existe pas dans les fichiers...

                oss << _tempbase[0] << ";";
                oss << _gel[0];
            }
            else
            {
                oss << "-999" << ";";
                oss << "-999";
            }

            file << oss.str() << endl;
        }

        file << endl;
        file.close();
    }


    void TEMP_EAU_CEQUEAU::SauvegardeParametres()
    {
        SauvegardeParametresZones();
        SauvegardeParametresTroncons();
    }


    void TEMP_EAU_CEQUEAU::SauvegardeParametresTroncons()
    {
        _nbTroncons = _sim_hyd.PrendreTroncons().PrendreNbTroncon();

        if (_nom_fichier_parametres_troncons == "")	//creation d'un fichier par defaut s'il n'existe pas
            _nom_fichier_parametres_troncons = Combine(_sim_hyd.PrendreRepertoireSimulation(), "temp_eau_cequeau_troncons.csv");

        string nom_fichier = _nom_fichier_parametres_troncons;
        ofstream fichier(nom_fichier);

        if (!fichier)
            throw ERREUR_ECRITURE_FICHIER(nom_fichier);

        fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
        fichier << endl;

        fichier << "SOUS MODELE;" << "TEMP EAU CEQUEAU" << endl;
        fichier << endl;

        fichier << "Capacite thermique de l eau;" << _C << endl;
        fichier <<"Albedo de la surface de l eau;"<< _alb << endl;
        fichier <<"Emissivite de l eau;"<< _emissivite_eau << endl;
        fichier <<"Chaleur latente de vaporisation de l eau;"<< _chal_latente << endl;

        fichier << endl;

        fichier << "TRONCON ID;"
            " COEFF SOLAIRE;"
            " COEFF INFRA ROUGE;"
            " COEFF EVAPO;"
            " COEFF CHALEUR;"
            " VITESSE VENT;"
            "COEFF A TRANSMISSIVITE ATMO;"
            "COEFF B TRANSMISSIVITE ATMO;"
            "COEFF C TRANSMISSIVITE ATMO;" 
            "COEFF A EMISSIVITE ATMO;" 
            "COEFF B EMISSIVITE ATMO;" 
            "COEFF C EMISSIVITE ATMO;" 
            "MOYENNE ANNUELLE TEMP AIR"<< endl;

        for (size_t index = 0; index < _nbTroncons; index++)
        {
            fichier << index + 1 << ';';

            fichier << _Csolaire[index] << ';';
            fichier << _Cir[index] << ';';
            fichier << _Cevapo[index] << ';';
            fichier << _Cchaleur[index] << ';';
            fichier << _vitvent[index] << ';';
            fichier << _fCoeffATransmissiviteAtmosTr[index] << ';';
            fichier << _fCoeffBTransmissiviteAtmosTr[index] << ';';
            fichier << _fCoeffCTransmissiviteAtmosTr[index] << ';';
            fichier << _fCoeffAEmissiviteAtmosTr[index] << ';';
            fichier << _fCoeffBEmissiviteAtmosTr[index] << ';';
            fichier << _fCoeffCEmissiviteAtmosTr[index] << ';';
            fichier << _temp_annuelle[index] << ';';
            fichier << _coefVarTemp1[index] << ';';
            fichier << _coefVarTemp2[index] << endl;
        }

        fichier.close();
    }


    void TEMP_EAU_CEQUEAU::SauvegardeParametresZones()
    {
        ZONES& zones = _sim_hyd.PrendreZones();

        if (_nom_fichier_parametres_zones == "")	//creation d'un fichier par defaut s'il n'existe pas
            _nom_fichier_parametres_zones = Combine(_sim_hyd.PrendreRepertoireSimulation(), "temp_eau_cequeau_zones.csv");

        string nom_fichier = _nom_fichier_parametres_zones;
        ofstream fichier(nom_fichier);

        if (!fichier)
            throw ERREUR_ECRITURE_FICHIER(nom_fichier);

        fichier << "PARAMETRES HYDROTEL VERSION;" << HYDROTEL_VERSION << endl;
        fichier << endl;

        fichier << "SOUS MODELE;" << "TEMP EAU CEQUEAU" << endl;
        fichier << endl;

        fichier << "UHRH ID;"
            " TEMPERATURE BASE;"
            " CRITERE GEL" << endl;

        for (size_t index = 0; index < zones.PrendreNbZone(); index++)
        {
            fichier << zones[index].PrendreIdent() << ';';

            fichier << _tempbase[index] << ';';
            fichier << _gel[index] << endl;
        }

        fichier.close();
    }


    float TEMP_EAU_CEQUEAU::PrendreEmissiviteAtmo(size_t index_troncon)
    {
        TRONCON* troncon =  _sim_hyd.PrendreTroncons()[index_troncon];
        float fDeltaT, fTr, fCloud, fpea, fTemp;

        fDeltaT = troncon->_tmax - troncon->_tmin;

        //transmissivité atmosphérique (Tr)
        fTr = _fCoeffATransmissiviteAtmosTr[index_troncon] * (1.0f - exp(-_fCoeffBTransmissiviteAtmosTr[index_troncon] * pow(fDeltaT, _fCoeffCTransmissiviteAtmosTr[index_troncon])));

        //ennuagement (cloud)
        if (fTr > 0.75f)
            fCloud = 0.0f;
        else
        {
            if (fTr < 0.15f)
                fCloud = 1.0f;
            else
                fCloud = 1.0f - (fTr - 0.15f) / 0.6f;
        }

        //pseudo émissivité atmosphérique
        fTemp = (troncon->_tmin + troncon->_tmax) / 2.0f;	            //temperature moyenne [dC]
        fpea = (_fCoeffAEmissiviteAtmosTr[index_troncon] + _fCoeffBEmissiviteAtmosTr[index_troncon] * fTemp) * (1.0f - _fCoeffCEmissiviteAtmosTr[index_troncon] * fCloud) + _fCoeffCEmissiviteAtmosTr[index_troncon] * fCloud;
        fpea = min(fpea, 1.0f);

        return fpea;
    }


    float TEMP_EAU_CEQUEAU::PrendreRayonnementNet(int iJour, size_t index_troncon)
    {
        TRONCON* troncon = _sim_hyd.PrendreTroncons()[index_troncon];

        float fOut1, fOut2, fOut3;
        float fSlopeAzimuth = 0.0f;  // Surface horizontale
        float fPente = 0.0f;         // pente nulle (horizontale)

        ZONE zone = *(troncon->PrendreZonesAmont()[0]);

        if (troncon->_coord_noeud_aval_y == VALEUR_MANQUANTE || troncon->_coord_noeud_aval_x == VALEUR_MANQUANTE)
        {
            const COORDONNEE noeud_aval = troncon->PrendreNoeudsAval()[0]->PrendreCoordonnee();
            
            //coordonné du noeuds (qui est dans la même projection que la grille Zone (uhrh.tif)
            COORDONNEE coord = COORDONNEE(noeud_aval.PrendreX(), noeud_aval.PrendreY());

            //conversion
            COORDONNEE coordWgs84 = _pCoordTrans->TransformeXY(coord);
			troncon->_coord_noeud_aval_x = coordWgs84.PrendreX();
			troncon->_coord_noeud_aval_y = coordWgs84.PrendreY();
        }

        _rayonnementNet->Calcul_Ra(static_cast<float>(troncon->_coord_noeud_aval_x),
                                    static_cast<float>(troncon->_coord_noeud_aval_y),
                                    fSlopeAzimuth,
                                    fPente,
                                    CONSTANTE_SOLAIRE,
                                    true,
                                    iJour,
                                    0,
                                    fOut1,
                                    fOut2,
                                    fOut3);
        
        _vRaTr[iJour - 1][index_troncon] = fOut1;

        float fRs_inc, fRs_ref, fRl_atm, fRl_surf, fRn;
        float fDeltaT, fTr, fpea, fEs, fTemp;

        fDeltaT = troncon->_tmax - troncon->_tmin;
        fTemp = (troncon->_tmin + troncon->_tmax) / 2.0f;

        //transmissivité atmosphérique (Tr)
        fTr = _fCoeffATransmissiviteAtmosTr[index_troncon] * (1.0f - exp(-_fCoeffBTransmissiviteAtmosTr[index_troncon] * pow(fDeltaT, _fCoeffCTransmissiviteAtmosTr[index_troncon])));

        //rayonnement de courtes longueurs d’onde incident (Rs_inc) [MJ/m2/Jour]
        fRs_inc = fTr * _vRaTr[iJour - 1][index_troncon];
		troncon->_rayonnement_solaire = fRs_inc;

        //rayonnement de courtes longueurs d’onde (solaire) réfléchi (Rs_ref) [MJ/m2/Jour]
        fRs_ref = _alb * fRs_inc;

        //pseudo émissivité de la surface (fEs)
        fEs = min(_emissivite_eau, 1.0f);
                
        //rayonnement de grandes longueurs d’onde incident (Rl_atm) [MJ/m2/Jour]
		fpea = PrendreEmissiviteAtmo(index_troncon);

        fRl_atm = fpea * _fValSigma * pow(fTemp + 273.15f, 4.0f);

        //rayonnement de grandes longueurs d’onde émis par la surface (Rl_surf) [MJ/m2/Jour]

        fRl_surf = fEs * _fValSigma * pow(troncon->_tempEau + 273.15f, 4.0f);

        _rayonnement_net_IR_temp = fRl_atm - fRl_surf;

        //rayonnement net à la surface [MJ/m2/Jour]
        fRn = fRs_inc - fRs_ref + _rayonnement_net_IR_temp;

        return fRn;
    }


    float TEMP_EAU_CEQUEAU::PrendreEvaporationPenman(size_t index_troncon)
    {
        TRONCON* troncon = _sim_hyd.PrendreTroncons()[index_troncon];

        DATE_HEURE dtCourant;
        float fRn, fP, fGamma, fEs, fDelta, fEA, fT, fe_tmin, fe_tmax, fe_tmoy, fEvapo;
        int iJourJulien;

        dtCourant = _sim_hyd.PrendreDateCourante();
        iJourJulien = dtCourant.PrendreJourJulien();

        //si c'est une annee bissextile, on fait le jour 59 (28 fevrier) 2 fois
        if (dtCourant.Bissextile() && iJourJulien > 59)
            --iJourJulien;

        //constante psychrométrique (Gamma) [kPa/dC]
        //pression atmosphérique (P) [kPa];
        fP = 101.3f * pow(((293.0f - 0.0065f * troncon->_altitude) / 293.0f), 5.26f);

        fGamma = 0.000665f * fP;

        //tension de vapeur a saturation moyenne (Es) [kPa]
        //estimation de la tension de vapeur d'eau [kPa]
        fe_tmin = 0.6108f * exp(17.27f * troncon->_tmin / (troncon->_tmin + 237.3f));	//Ea
        fe_tmax = 0.6108f * exp(17.27f * troncon->_tmax / (troncon->_tmax + 237.3f));

        fEs = (fe_tmax + fe_tmin) / 2.0f;

        //temperature moyenne de l'air [dC]
        fT = (troncon->_tmin + troncon->_tmax) / 2.0f;

        //pente de la tension de vapeur d’eau saturante en fonction de la température (Delta) [kPa/dC]
        fe_tmoy = 0.6108f * exp(17.27f * fT / (fT + 237.3f));
        fDelta = 4098.0f * fe_tmoy / pow(fT + 237.3f, 2.0f);

        //rayonnement net à la surface (Rn) [MJ/m2/Jour]
        fRn = PrendreRayonnementNet(iJourJulien, index_troncon);

        fEA = fGamma * CHALEUR_LATENTE_VAPORISATION * 2.6f * (1.0f + 0.54f * (_vitvent[index_troncon] / 3.6f)) * (fEs - fe_tmin);   //_vitvent km/h -> m/s

        fEvapo = ((fDelta * fRn + fEA) / (fDelta + fGamma)) * 1.0f / CHALEUR_LATENTE_VAPORISATION;

		fEvapo = max(0.0f, fEvapo); //Contrôle ajouté par Stéphane Savary 02-04-2026 pour éviter des valeurs négative d'évaporation

		return fEvapo;  //mm/jour
    }


  //  float TEMP_EAU_CEQUEAU::PrendreEvaporationPriestlay(size_t index_troncon)
  //  {
  //      TRONCON* troncon = _sim_hyd.PrendreTroncons()[index_troncon];

  //      DATE_HEURE dtCourant;
  //      float fRn, fP, fGamma, fDelta, fT, fe_tmoy, fEvapo, alpha;
  //      int iJourJulien;

		//alpha = 1.26f; // coefficient Priestley-Taylor

  //      dtCourant = _sim_hyd.PrendreDateCourante();
  //      iJourJulien = dtCourant.PrendreJourJulien();

  //      //si c'est une annee bissextile, on fait le jour 59 (28 fevrier) 2 fois
  //      if (dtCourant.Bissextile() && iJourJulien > 59)
  //          --iJourJulien;

  //      //constante psychrométrique (Gamma) [kPa/dC]
  //      //pression atmosphérique (P) [kPa];
  //      fP = 101.3f * pow(((293.0f - 0.0065f * troncon->_altitude) / 293.0f), 5.26f);

  //      fGamma = 0.000665f * fP;

  //      //temperature moyenne de l'air [dC]
  //      fT = (troncon->_tmin + troncon->_tmax) / 2.0f;

  //      //pente de la tension de vapeur d’eau saturante en fonction de la température (Delta) [kPa/dC]
  //      fe_tmoy = 0.6108f * exp(17.27f * fT / (fT + 237.3f));
  //      fDelta = 4098.0f * fe_tmoy / pow(fT + 237.3f, 2.0f);

  //      //rayonnement net à la surface (Rn) [MJ/m2/Jour]
  //      fRn = PrendreRayonnementNet(iJourJulien, index_troncon);

  //      fEvapo = (1.0f / CHALEUR_LATENTE_VAPORISATION) * alpha * (fDelta/(fDelta + fGamma)) * fRn;

  //      return fEvapo;  //en mm/jour

  //  }


    void TEMP_EAU_CEQUEAU::Calcule()
    {
        const float C = _C; //capacité thermique spécifique (kJ/kg°C)
        const float alb = _alb; //albedo
		const float chal_latente = _chal_latente; //chaleur latente de vaporisation (MJ/m3)
        
        DATE_HEURE dtCourant;
        dtCourant = _sim_hyd.PrendreDateCourante();

        float longueur, largeur, surface;
        float eSolaire, eIR, eEvapo;
        float ray_solaire;
        float temp_air;
        float temp_air_precedent;
        float temp_ini;
        float emi_atm;
        float evapo;
        //float evapo_b;
        float eChaleur;
        float vitVent;
        float gel;

        float eSurf, eHypo, eBase;
        float eReseau;
        float eLocal;
        float eTotal;
        float eAmont;

        float hauteurCouvertNival;
        int compteur;

        float tempSurf, tempHypo, tempBase;
        float tempNouvelle;
        float tempUp;
        float tempAnnuelle;

        float qBase, qSurf, qHypo;
        float qUp;

        float volBase, volSurf, volHypo;
        float volReseau;
        float volAmont;
        float volTotal;

        float Cchaleur, Cevapo, Cir, Csolaire;

        TRONCONS& troncons = _sim_hyd.PrendreTroncons();
        ZONE zone;

        size_t idx_sim, idx, i, nbTroncon, idxZone;
        TRONCON* tron;
        TRONCON::TYPE_TRONCON type_troncon;

        std::vector<TRONCON*> tronconsAmont;
        TRONCON* tUp;
        size_t a;
        std::vector<ZONE*> vec;

        nbTroncon = _sim_hyd._troncons_tries.size();

        for(idx_sim=0; idx_sim<nbTroncon; idx_sim++)
        {
            idx = _sim_hyd._troncons_tries[nbTroncon-idx_sim-1]; //commence par la fin du vecteur et termine par le 1er element...
            tron = troncons[idx];

            emi_atm = PrendreEmissiviteAtmo(idx);

            Cchaleur = _Cchaleur[idx];
            Csolaire = _Csolaire[idx];
            Cevapo = _Cevapo[idx];
            Cir = _Cir[idx];
            vitVent = _vitvent[idx];

            if(tron->_tmax == VALEUR_MANQUANTE || tron->_tmin == VALEUR_MANQUANTE)
                throw ERREUR("TEMP_EAU_CEQUEAU: error: invalid tmin/tmax values");

            temp_air = (tron->_tmax + tron->_tmin) / 2.0f;

            if(tron->_tempEau == -999.0f)
            {
                tron->_tempEau = max(0.0f, temp_air);   //temperature initiale (1er pdt)

                _tempAirPasPrecedent[idx] = temp_air;
            }

            evapo = PrendreEvaporationPenman(idx) * 0.001f;   // conversion en m/jour
            ray_solaire = tron->_rayonnement_solaire;

            vec = tron->PrendreZonesAmont();
           
			qBase = 0.0f;
			qSurf = 0.0f;
			qHypo = 0.0f;
			compteur = 0;
            tempBase = 0.0f;
            gel = 0.0f;
			hauteurCouvertNival = 0.0f;

            for(i=0; i<vec.size(); i++) 
            {
                zone = *(vec[i]);
                idxZone = _sim_hyd.PrendreZones().IdentVersIndex(zone.PrendreIdent());

                qBase+= zone._ecoulementBase;
                qSurf+= zone._ecoulementSurf;
                qHypo+= zone._ecoulementHypo;

                hauteurCouvertNival+= zone.PrendreHauteurCouvertNival();

                tempBase+= _tempbase[idxZone];
                gel+= _gel[idxZone];

                ++compteur;
            }

			tempBase = tempBase / static_cast<float>(compteur);
			gel = gel / static_cast<float>(compteur);
			hauteurCouvertNival = hauteurCouvertNival / static_cast<float>(compteur);

            if (hauteurCouvertNival <= 0.0f)
            {
                tempSurf = std::max(0.0f, temp_air);
            }
            else if (hauteurCouvertNival <= gel)
            {
                tempSurf = temp_air * (1.0f - hauteurCouvertNival / (gel + 1.0f));
                tempSurf = std::max(0.0f, tempSurf);
            }
            else
            {
                tempSurf = 0.0f;
            }

            type_troncon = tron->PrendreType();

			surface = 0.0f;
            if (type_troncon == TRONCON::RIVIERE)
            {
                RIVIERE* ptr = static_cast<RIVIERE*>(tron);
                longueur = ptr->PrendreLongueur();
                largeur = ptr->PrendreLargeur();
                surface = longueur * largeur;
            }

            if (type_troncon == TRONCON::LAC)
            {
                LAC* ptr = static_cast<LAC*>(tron);
                surface = ptr->PrendreSurface();
				surface = surface * 1000000.0f;  // conversion en m2
            }

			temp_air_precedent = _tempAirPasPrecedent[idx];

            temp_ini = tron->_tempEau;

            if(_hauteurPasPrecedent[idx] == -999.0f)
                _hauteurPasPrecedent[idx] = static_cast<float>(tron->_hauteurAvalMoy);  //pour le 1er pdt on prend la hauteur actuelle... pour les autres pdt on prend la hauteur du pdt précédent

            volReseau = surface * _hauteurPasPrecedent[idx];
            eReseau = volReseau * temp_ini * C;

            eSolaire = surface * (1 - alb) * ray_solaire * Csolaire;
            eIR = surface * _rayonnement_net_IR_temp * Cir;
            eEvapo = surface * evapo * chal_latente * Cevapo;
            eChaleur = surface * 0.2f * vitVent * (temp_air - temp_ini) * Cchaleur;

            // énergie en amont

            eAmont = 0.0f;
            volAmont = 0.0f;
            tronconsAmont = tron->PrendreTronconsAmont();
            for (a = 0; a < tronconsAmont.size(); a++) 
            {
                tUp = tronconsAmont[a];
                tempUp = tUp->_tempEau;
                qUp = tUp->PrendreDebitAval();

                volAmont+= qUp * 86400.0f; //limitation au pas de temps journalier
                eAmont+= tempUp * qUp * C * 86400.0f;  //limitation au pas de temps journalier
            }

            // apport de surface, de base et hypodermique
            tempHypo = (tempSurf + tempBase) / 2.0f;

            volSurf = qSurf * 86400.0f; //limitation au pas de temps journalier
            volHypo = qHypo * 86400.0f; //limitation au pas de temps journalier
            volBase = qBase * 86400.0f; //limitation au pas de temps journalier

            eBase = volBase * C * tempBase;  
            eSurf = volSurf * C * tempSurf;  
            eHypo = volHypo * C * tempHypo;  

            // total volume
            volTotal = volSurf + volHypo+ volBase + volReseau + volAmont;

            // température finale
            eLocal = eBase + eSurf + eHypo + eSolaire + eIR - eEvapo + eChaleur;
            eTotal = eReseau + eAmont + eLocal;
            tempNouvelle = (eTotal / (volTotal * C));

            // bornage entre 0 et 35 °C
            tempNouvelle = std::max(0.0f, std::min(35.0f, tempNouvelle));

            tempAnnuelle = _temp_annuelle[idx];

            if ( (temp_air >= temp_air_precedent && tempNouvelle >= temp_ini && tempNouvelle >= tempAnnuelle && tempNouvelle >= temp_air && temp_ini < temp_air) || //cas 1 
                 (temp_air >= temp_air_precedent && tempNouvelle >= temp_ini && tempNouvelle >= tempAnnuelle && tempNouvelle >= temp_air && temp_ini >= temp_air) || //cas 2 
                 (temp_air < temp_air_precedent && tempNouvelle < temp_ini && temp_ini >= tempAnnuelle && tempNouvelle < temp_air && temp_ini >= temp_air) || //cas 3 
                 (temp_air < temp_air_precedent && tempNouvelle < temp_ini && temp_ini >= tempAnnuelle && tempNouvelle < temp_air && temp_ini < temp_air) || //cas 4 
                 (temp_air >= temp_air_precedent && tempNouvelle < temp_ini && temp_ini >= tempAnnuelle && tempNouvelle < temp_air && temp_ini >= temp_air) || //cas 5 
                 (temp_air >= temp_air_precedent && tempNouvelle < temp_ini && temp_ini >= tempAnnuelle && tempNouvelle < temp_air && temp_ini < temp_air) || //cas 6 
                 (temp_air < temp_air_precedent && tempNouvelle >= temp_ini && tempNouvelle >= tempAnnuelle && tempNouvelle >= temp_air && temp_ini < temp_air) || //cas 7
                 (temp_air < temp_air_precedent && tempNouvelle >= temp_ini && tempNouvelle >= tempAnnuelle && tempNouvelle >= temp_air && temp_ini >= temp_air) ) //cas 8
            {
                if(tempNouvelle >= temp_ini)
					tempNouvelle = std::max(0.0f, std::min(35.0f, temp_air*_coefVarTemp1[idx])); //AJOUT d'un paramètre _coefVarTemp1 par Stéphane Savary 02-04-2026 Ce paramètre d'une valeur par défaut de 1 et strictement positif doit être placé après le paramètre _temp_annuelle
				else
					tempNouvelle = std::max(0.0f, std::min(35.0f, temp_air*_coefVarTemp2[idx])); //AJOUT d'un paramètre _coefVarTemp2 par Stéphane Savary 02-04-2026 Ce paramètre d'une valeur par défaut de 1 et strictement positif doit être placé après le paramètre Cvartemp1
            }

            tron->_tempEau = tempNouvelle;
            
            _tempAirPasPrecedent[idx] = temp_air;
            _hauteurPasPrecedent[idx] = static_cast<float>(tron->_hauteurAvalMoy);
        }

		TEMP_EAU::Calcule();
    }


    void TEMP_EAU_CEQUEAU::Termine()
    {
        TEMP_EAU::Termine();
    }


}
