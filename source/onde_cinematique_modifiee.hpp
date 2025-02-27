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

#ifndef ONDE_CINEMATIQUE_MODIFIEE_H_INCLUDED
#define ONDE_CINEMATIQUE_MODIFIEE_H_INCLUDED


#include "acheminement_riviere.hpp"


namespace HYDROTEL
{

	class ONDE_CINEMATIQUE_MODIFIEE : public ACHEMINEMENT_RIVIERE
	{
	public:
		ONDE_CINEMATIQUE_MODIFIEE(SIM_HYD& sim_hyd);
		virtual ~ONDE_CINEMATIQUE_MODIFIEE();

		virtual void	Initialise();

		virtual void	Calcule();	//calcule le pas de temps courant

		void			CalculeTroncon(size_t indexTroncon, int t, int dt);

		virtual void	Termine();

		virtual void LectureParametres();

		void LectureParametresFichierGlobal();

		//milieux humides riverains
		virtual void LectureMilieuHumideRiverain();
		virtual void LectureProfondeur();

		virtual void SauvegardeParametres();

		float PrendreOptimisationRugosite(size_t index_troncon) const;

		float PrendreOptimisationLargeurRiviere(size_t index_troncon) const;

		std::string PrendreNomFichierLectureEtat() const;

		std::string PrendreRepertoireEcritureEtat() const;

		bool PrendreSauvegardeTousEtat() const;

		DATE_HEURE PrendreDateHeureSauvegardeEtat() const;

		void ChangeOptimisationRugosite(size_t index_troncon, float coef);

		void ChangeOptimisationLargeurRiviere(size_t index_troncon, float coef);

		void ChangeNomFichierLectureEtat(std::string nom_fichier);

		void ChangeRepertoireEcritureEtat(std::string repertoire);

		void ChangeSauvegardeTousEtat(bool sauvegarde_tous);

		void ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde);

		virtual void ChangeNbParams(const ZONES& zones);

		double	CalculHauteurEauTrapezoidal(double dDischarge, double dLargeur, double dSideSlope, double dManningCoeff, double dLongitudinalSlope);
		double	CalculDebitTrapezoidal(double dProfondeur, double dLargeur, double dSideSlope, double dManningCoeff, double dLongitudinalSlope);

		void	LectureFichierDebitsHauteurs();
		double	ObtientHauteurGrilleQH(size_t idxTroncon, double dQ);
		double	ObtientDebitGrilleQH(size_t idxTroncon, double dHauteur);

		int									_hauteurMethodeCalcul;			//1: section rectangulaire		//2: section trapezoidal (Tiwari et al. (2012))		//3: approche HAND (fichier débits/hauteurs)

		std::vector<double>					_hauteurTrapezeSideSlope;		//'side slope H to V='		//pente des berges

		double								_hauteurHandIncrement;			//incrément des hauteurs hand (la 1ere hauteur doit etre 0...)
		std::vector<std::vector<double>>	_hauteurHandValDebits;			//valeur des débits pour chaque incrément pour chaque troncon	//[indexTroncon][indexIncrement]


		void	LectureFichierPerimetreMouilleHauteurs();
		double  ObtientPMGrillePH(size_t idxTroncon, double dHauteur);
		
		double								_hauteurPMIncrement;			//incrément des hauteurs d'eau (la 1ere hauteur doit etre 0...)
		std::vector<std::vector<double>>	_hauteurPMVal;					//valeur des périmètres mouillés pour chaque incrément pour chaque troncon	//[indexTroncon][indexIncrement]


	private:

		//std::map<int, std::vector<size_t>>		mapShreveTroncon;	//NoOrdreShreve 1-X, index des troncons
		//int										iShreveMax;			//no ordre max

		void TrieTroncons();

		void TransfertRiviere(size_t idxTroncon, int pdts, float lng, float lrg, float pte, float man, float qa, float ql,  float qb, float qc, float qm, float& hauteur, float& section, float& qd);
		void TransfertLac(int dt, float aire, float c, float k, float qa, float ql, float qb, float qc, float qm, float& haut, float& qd);

		void CalculMilieuHumideRiverain(int pdts, size_t index_troncon, TRONCON* pTroncon, MILIEUHUMIDE_RIVERAIN* wetland,float& sur_q,float& qd);

		struct OCM
		{
			float qamont;      // debit amont au pas precedent
			float qaval;       // debit aval au pas precedent
			float qapportlat;  // apport lateral au pas precedent
		};

		float Celerite(float lng, float lrg, float pte, float man, float qamont, float qaval);

		void LectureEtat(DATE_HEURE date_courante);
		void SauvegardeEtat(DATE_HEURE date_courante);

		std::vector<float> _coefficient_optimisation_rugosite;		//parametre
		std::vector<float> _coefficient_largeurs_rivieres;			//parametre

		// variable pour lecture/ecriture de l'etat

		std::string _nom_fichier_lecture_etat;
		std::string _repertoire_ecriture_etat;

		DATE_HEURE _date_sauvegarde_etat;
		bool _sauvegarde_etat;

		bool _sauvegarde_tous_etat;

		// variables de simulations

		std::vector<OCM> _ocm;
		std::vector<size_t> _troncons_tries;

        // milieux humides riverain
		std::vector<MILIEUHUMIDE_RIVERAIN*> _milieu_humide_riverain;

		std::vector<double> _hauteur;	//hauteur d'eau en aval des troncons (pas de temps interne) (m)

		std::map<int, std::vector<OCM> > _ocm_mh; // debits au pas precedant pour les milieux humides

		std::ofstream m_wetfichier;

		std::ofstream	_fichier_pm;	//output périmètre mouillé

		//prelevements
		int				_prNbJourMois;	//GPE
		std::string		_prColNbJour;	//
		std::string		_prColVol;		//

		//DEBITS AVAL MOY7J MIN
		size_t							_q7avgNbPdt;	//Nombre de pas de temps necessaire pour calculer la moyenne 7 jours (jour courant + 6 jours précédent)

		int								_q7avgYearStartY;
		int								_q7avgYearEndY;
		int								_q7avgYearCurrentY;
		std::vector<std::vector<float>> _q7avgMinY;				//m3/s		//[troncon index][simulation year]		//yearly minimum

		int								_q7avgYearStartS;
		int								_q7avgYearEndS;
		int								_q7avgYearCurrentS;
		std::vector<std::vector<float>> _q7avgMinS;				//m3/s		//[troncon index][simulation year]		//summery minimum (month 6 to 11)
	};

}

#endif
