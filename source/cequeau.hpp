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

#ifndef CEQUEAU_H_INCLUDED
#define CEQUEAU_H_INCLUDED


#include "bilan_vertical.hpp"
#include "milieu_humide_isole.hpp"


namespace HYDROTEL
{

	class CEQUEAU : public BILAN_VERTICAL
	{
	public:
		CEQUEAU(SIM_HYD& sim_hyd);
		virtual ~CEQUEAU();

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		virtual void LectureParametres();

		void		 LectureParametresFichierGlobal();

		virtual void SauvegardeParametres();

		virtual void ChangeNbParams(const ZONES& zones);	//initialisation des vecteurs et valeurs par defaut


		//milieux humides isolés
		virtual void LectureMilieuHumideIsole();

		virtual void CalculMilieuHumideIsole(MILIEUHUMIDE_ISOLE* pMilieuHumide, int ident, float hru_ha, 
												float wet_fr, float evp, float& apport, float& prod, unsigned short pdt);

		//variables d'etat
		void		ChangeNomFichierLectureEtat(std::string nom_fichier);						//change le nom du fichier de lecture d'etat
		void		ChangeRepertoireEcritureEtat(std::string repertoire);						//change le repertoire d'ecriture d'etat
		void		ChangeSauvegardeTousEtat(bool sauvegarde_tous);								//change la sauvegarde de tous les etats
		void		ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde);	//change la date de la sauvegarde de l'etat

		std::string PrendreNomFichierLectureEtat() const;										//retourne le nom de fichier de lecture de l'etat
		std::string PrendreRepertoireEcritureEtat() const;										//retourne le repertoire d'ecriture de l'etat
		bool		PrendreSauvegardeTousEtat() const;											//retourne la sauvegarde de tous les etats
		DATE_HEURE	PrendreDateHeureSauvegardeEtat() const;										//retourne la date de sauvegarde de l'etat

		void		LectureEtat(DATE_HEURE date_courante);
		void		SauvegardeEtat(DATE_HEURE date_courante) const;

		std::string _nom_fichier_lecture_etat;
		std::string _repertoire_ecriture_etat;
		DATE_HEURE	_date_sauvegarde_etat;
		bool		_sauvegarde_etat;
		bool		_sauvegarde_tous_etat;

		//variables de simulation
		std::vector<float>		_freau;					//pourcentage d'occupation pour les classes integree
		std::vector<float>		_frimp;
		std::vector<float>		_frfor;

		std::vector<float>		_sol;
		std::vector<float>		_nappe;
		std::vector<float>		_lacma;

		//param
		std::vector<size_t>		_index_eaux;
		std::vector<size_t>		_index_impermeables;
		std::vector<size_t>		_index_forets;

		//param uhrh
		std::vector<float>		_seuil_min_rui;
		std::vector<float>		_seuil_max_sol;
		std::vector<float>		_seuil_vid_sol;
		std::vector<float>		_coef_vid_sol1;
		std::vector<float>		_coef_vid_sol2;
		std::vector<float>		_seuil_perc_sol;
		std::vector<float>		_coef_perc_sol;
		std::vector<float>		_max_perc_sol;
		std::vector<float>		_seuil_etp_etr;
		std::vector<float>		_coef_rec_nappe_haute;
		std::vector<float>		_coef_rec_nappe_basse;
		std::vector<float>		_fract_etp_nappe;
		std::vector<float>		_niv_vid_nappe;
		std::vector<float>		_seuil_vid_lacma;
		std::vector<float>		_coef_vid_lacma;
		std::vector<float>		_init_sol;
		std::vector<float>		_init_nappe;
		std::vector<float>		_init_lacma;

		//milieux humides isolés
		std::vector<MILIEUHUMIDE_ISOLE*> _milieu_humide_isole;

		struct SMilieuHumideResult
		{
			float apport;	// mm
			float evp;		// mm
			float wetsep;	// m^3
			float wetflwi;	// m^3
			float wetflwo;	// m^3
			float wetprod;	// mm
		};

		std::map<int, SMilieuHumideResult*> _milieu_humide_result;	//result for current timestep

		std::ofstream m_wetfichier;
	};

}

#endif
