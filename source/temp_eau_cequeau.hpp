#ifndef TEMP_EAU_CEQUEAU_H_INCLUDED
#define TEMP_EAU_CEQUEAU_H_INCLUDED

#include "temp_eau.hpp"
#include "sous_modele.hpp"
#include "rayonnement_net.hpp"
#include "onde_cinematique_modifiee.hpp"

#include "transforme_coordonnee.hpp"

#include <vector>


namespace HYDROTEL
{


    class TEMP_EAU_CEQUEAU : public TEMP_EAU
    {
    public:

        TEMP_EAU_CEQUEAU(SIM_HYD& sim_hyd);
        virtual ~TEMP_EAU_CEQUEAU();

        virtual void    Initialise();
        virtual void    Calcule();
        virtual void    Termine();

        virtual void    ChangeNbParams(const ZONES& zones);

        virtual void    LectureParametres();
        virtual void    SauvegardeParametres();

        void            LectureParametresZones(); 
        void            LectureParametresTroncons();
        void            SauvegardeParametresTroncons(); 
        void            SauvegardeParametresZones();

        bool            LectureParametresFichierGlobal();
        void            SauvegardeParametresFichierGlobal();

        float		    PrendreEmissiviteAtmo(size_t index_troncon);
        float		    PrendreRayonnementNet(int iJour, size_t index_troncon);

		float           PrendreEvaporationPenman(size_t index_troncon);
        //float         PrendreEvaporationPriestlay(size_t index_troncon);


        std::string	_nom_fichier_parametres_troncons;
        std::string	_nom_fichier_parametres_zones;

    private:
          
        float               _fValSigma;                     //constante de Stefan-Boltzmann (MJ/m2K4) * 10e-6 (0.000000004903)

        std::vector<float>  _hauteurPasPrecedent;           //hauteur d'eau des troncons (m) (pas de temps précédent)
        std::vector<float>  _tempAirPasPrecedent;           //tenperature air (C) (pas de temps précédent)

        std::vector<float>  _tempbase;
		std::vector<float>  _temp_annuelle;                 //temp de l'air anuelle moyenne en °C
        std::vector<float>  _coefVarTemp1;                  //coefficient de variations sur la borne de température (pour palier aux variations intra-journalières des observations)
        std::vector<float>  _coefVarTemp2;                  //coefficient de variations sur la borne de température (pour palier aux variations intra-journalières des observations)
        std::vector<float>  _gel;                           //critère de gel en mètre
        std::vector<float>  _Csolaire;
        std::vector<float>  _Cir;
        std::vector<float>  _Cchaleur;
        std::vector<float>  _Cevapo;
        std::vector<float>  _vitvent;

        float               _C;                             //la capacité thermique de l’eau
        float               _alb;                           //l'albédo de la surface de l'eau
        float               _emissivite_eau;                //l'émissivité de la surface de l'eau 
        float               _chal_latente;                  //la chaleur latente de vaporisation de l’eau  (MJ/m3)

        //rayonnement net
        RAYONNEMENT_NET*                _rayonnementNet;

        std::vector<std::vector<float>>	_vRaTr;			    //rayonnement net à la surface [MJ/m2/Jour] pour chaque jour (0 a 364), pour chaque troncon (index troncon)

        std::vector<float>				_fCoeffATransmissiviteAtmosTr;
        std::vector<float>				_fCoeffBTransmissiviteAtmosTr;
        std::vector<float>				_fCoeffCTransmissiviteAtmosTr;

        std::vector<float>				_fCoeffAEmissiviteAtmosTr;
        std::vector<float>				_fCoeffBEmissiviteAtmosTr;
        std::vector<float>				_fCoeffCEmissiviteAtmosTr;

        TRANSFORME_COORDONNEE*          _pCoordTrans;

        float                           _rayonnement_net_IR_temp;

    };


}

#endif
