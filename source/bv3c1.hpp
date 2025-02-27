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

#ifndef BV3C1_H_INCLUDED
#define BV3C1_H_INCLUDED


#include "bilan_vertical.hpp"
#include "milieu_humide_isole.hpp"


namespace HYDROTEL
{

	class BV3C1 : public BILAN_VERTICAL
	{
	public:
		BV3C1(SIM_HYD& sim_hyd);
		virtual ~BV3C1();

		virtual void	Initialise();

		virtual void	Calcule();	//calcule le pas de temps courant

		void			CalculeUHRH(int iIndexZone);

		virtual void	CalculMilieuHumideIsole(MILIEUHUMIDE_ISOLE* pMilieuHumide, int ident, float hru_ha, 
												float wet_fr, float evp, float& apport, float& prod, unsigned short pdt);

		virtual void	Termine();

		virtual void LectureParametres();

		void LectureParametresFichierGlobal();

		virtual void LectureMilieuHumideIsole();

		virtual void SauvegardeParametres();

		virtual void ChangeNbParams(const ZONES& zones);

		/// change le % de saturation initial de la couche1
		void ChangeTheta1Initial(size_t index_zone, float theta);

		/// change le % de saturation initial de la couche2
		void ChangeTheta2Initial(size_t index_zone, float theta);

		/// change le % de saturation initial de la couche3
		void ChangeTheta3Initial(size_t index_zone, float theta);

		void ChangeKrec(size_t index_zone, float krec);

		void ChangeDes(size_t index_zone, float des);

		void ChangeCin(size_t index_zone, float fCin);

		void ChangeCoefAssechement(size_t index_zone, float coef_assech);

		/// change les classes d'occupation eaux (0 based index)
		void ChangeIndexEaux(const std::vector<size_t>& index);

		/// change les classes d'occupation impermeables (0 based index)
		void ChangeIndexImpermeables(const std::vector<size_t>& index);

		void ChangeNomFichierLectureEtat(std::string nom_fichier);

		void ChangeRepertoireEcritureEtat(std::string repertoire);

		void ChangeSauvegardeTousEtat(bool sauvegarde_tous);

		void ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde);

		/// retourne le % de saturation initial de la couche1
		float PrendreTheta1Initial(size_t index_zone) const;

		// retourne le % de saturation initial de la couche2
		float PrendreTheta2Initial(size_t index_zone) const;

		/// retourne le % de saturation initial de la couche3
		float PrendreTheta3Initial(size_t index_zone) const;

		float PrendreDes(size_t index_zone) const;

		float PrendreKrec(size_t index_zone) const;

		float PrendreCin(size_t index_zone) const;

		float PrendreCoeffAssechement(size_t index_zone) const;

		/// retourne les classes d'occupation eaux (0 based index)
		std::vector<size_t> PrendreIndexEaux() const;

		/// retourne les classes d'occupation impermeables (0 based index)
		std::vector<size_t> PrendreIndexImpermeables() const;

		std::string PrendreNomFichierLectureEtat() const;

		std::string PrendreRepertoireEcritureEtat() const;

		bool PrendreSauvegardeTousEtat() const;

		DATE_HEURE PrendreDateHeureSauvegardeEtat() const;

		//NetCDF	//results are kept in memory while running and are saved at the end of the simulation
		float*				_netCdf_theta1;	//[time*stations]
		float*				_netCdf_theta2;
		float*				_netCdf_theta3;
		float*				_netCdf_etr1;
		float*				_netCdf_etr2;
		float*				_netCdf_etr3;
		float*				_netCdf_etr_total;
		float*				_netCdf_q12;
		float*				_netCdf_q23;
		//

	private:
		float ConductiviteHydrolique(float theta, PROPRIETE_HYDROLIQUE& typesol, size_t index_sol);
		float CalculePsi(float theta, PROPRIETE_HYDROLIQUE& typesol, size_t index_sol);

		void CalculeEtr();
		void CalculeRuisselement(ZONE& zone, size_t index_zone, float& pinf, float& ruis);		
		void TriCoucheOct97(ZONE& zone, size_t index_zone, float pinf, float& ruis, float& q2, float& q3, float& dtc);

		void LectureEtat(DATE_HEURE date_courante);
		void SauvegardeEtat(DATE_HEURE date_courante) const;

		std::vector<size_t> _index_eaux;
		std::vector<size_t> _index_impermeables;
		std::vector<size_t> _index_autres;

		std::vector<float> _theta1_initial;
		std::vector<float> _theta2_initial;
		std::vector<float> _theta3_initial;

		std::vector<float> _krec;	// m/h
		std::vector<float> _des;
		std::vector<float> _cin;
		std::vector<float> _coef_assech;

		// variable pour lecture/ecriture de l'etat

		std::string _nom_fichier_lecture_etat;
		std::string _repertoire_ecriture_etat;

		DATE_HEURE _date_sauvegarde_etat;
		bool _sauvegarde_etat;

		bool _sauvegarde_tous_etat;

		// variables de simulation

		std::ofstream _fichier_theta1;
		std::ofstream _fichier_theta2;
		std::ofstream _fichier_theta3;

		std::ofstream _fichier_etr1;
		std::ofstream _fichier_etr2;
		std::ofstream _fichier_etr3;
		
		std::ofstream _fichier_etr_total;

		std::ofstream _fichier_q12;
		std::ofstream _fichier_q23;

		std::vector<float> _pourcentage_eau;
		std::vector<float> _pourcentage_impermeable;
		std::vector<float> _pourcentage_autre;

		std::vector<float> _q12;	// mm/pdt
		std::vector<float> _q23;	// mm/pdt

		std::vector<float> _b;
		std::vector<float> _omegpi;
		std::vector<float> _mm;
		std::vector<float> _nn;

		// milieux humides isolés
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

        // 
        std::vector<CORRECTION*> _corrections_reserve_sol;
		std::vector<CORRECTION*> _corrections_saturation_sol;

		float _fDTCMin;		//pas de temps interne minimum possible		
	};

}

#endif
