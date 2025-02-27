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

#ifndef DEGRE_JOUR_H_INCLUDED
#define DEGRE_JOUR_H_INCLUDED


#include "fonte_neige.hpp"
#include "stations_neige.hpp"
#include "station_neige.hpp"
#include "grille_neige.hpp"
#include "thiessen1.hpp"
#include "thiessen2.hpp"
#include "moyenne_3_stations1.hpp"
#include "moyenne_3_stations2.hpp"


namespace HYDROTEL
{

	class DEGRE_JOUR_MODIFIE : public FONTE_NEIGE
	{
	public:

		enum TYPE_INTERPOLATION
		{
			INTERPOLATION_AUCUNE,
			INTERPOLATION_THIESSEN,
			INTERPOLATION_MOYENNE_3_STATIONS,
		};

		DEGRE_JOUR_MODIFIE(SIM_HYD& sim_hyd);
		virtual ~DEGRE_JOUR_MODIFIE();

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		virtual void LectureParametres();

		void LectureParametresFichierGlobal();

		virtual void SauvegardeParametres();

		/// retourne le taux de fonte (mm/jour) 
		float PrendreTauxFonte(size_t index_zone) const;

		/// retourne la densite maximal (Kg/m3)
		float PrendreDensiteMaximal(size_t index_zone) const;

		float PrendreConstanteTassement(size_t index_zone) const;

		/// prendre le seuil de temperature feuillus (C)
		float PrendreSeuilFonteFeuillus(size_t index_zone) const;

		/// prendre le seuil de temperature conifers (C)
		float PrendreSeuilFonteConifers(size_t index_zone) const;

		/// prendre le seuil de temperature decouvers (C)
		float PrendreSeuilFonteDecouver(size_t index_zone) const;

		/// retourne le taux de fonte feuillus (mm/jour.C)
		float PrendreTauxFonteFeuillus(size_t index_zone) const;

		/// retourne le taux de fonte coniferes (mm/jour.C)
		float PrendreTauxFonteConifers(size_t index_zone) const;

		/// retourne le taux de fonte decouverts (mm/jour.C)
		float PrendreTauxFonteDecouver(size_t index_zone) const;

		/// prendre les classes d'occupation feuillus (0 based index)
		std::vector<size_t> PrendreIndexOccupationFeuillus() const;

		/// prendre les classes d'occupation coniferes (0 based index)
		std::vector<size_t> PrendreIndexOccupationConifers() const;

		std::string PrendreNomFichierLectureEtat() const;
		std::string PrendreRepertoireEcritureEtat() const;
		bool		PrendreSauvegardeTousEtat() const;
		DATE_HEURE PrendreDateHeureSauvegardeEtat() const;

		virtual void ChangeNbParams(const ZONES& zones);

		/// change le taux de fonte des feuillus (mm/jour.C)
		void ChangeTauxFonteFeuillus(size_t index_zone, float taux_fonte);

		/// change le taux de fonte des conifers (mm/jour.C)
		void ChangeTauxFonteConifers(size_t index_zone, float taux_fonte);

		/// change le taux de fonte decouver (mm/jour.C)
		void ChangeTauxFonteDecouver(size_t index_zone, float taux_fonte);

		/// change le seuil de temperature feuillus (C)
		void ChangeSeuilFonteFeuillus(size_t index_zone, float seuil_fonte);

		/// change le seuil de temperature conifers (C)
		void ChangeSeuilFonteConifers(size_t index_zone, float seuil_fonte);

		/// change le seuil de temperature decouvers (C)
		void ChangeSeuilFonteDecouver(size_t index_zone, float seuil_fonte);

		/// change le taux de fonte (mm/jour) 
		void ChangeTauxFonte(size_t index_zone, float taux_fonte);

		/// change la densite maximal (Kg/m3)
		void ChangeDesiteMaximale(size_t index_zone, float densite);

		void ChangeConstanteTassement(size_t index_zone, float constante_tassement);

		/// change les classes d'occupation feuillus (0 based index)
		void ChangeIndexOccupationFeuillus(const std::vector<size_t>& index);

		/// change les classes d'occupation coniferes (0 based index)
		void ChangeIndexOccupationConifers(const std::vector<size_t>& index);

		void ChangeNomFichierLectureEtat(std::string nom_fichier);
		void ChangeRepertoireEcritureEtat(std::string repertoire);
		void ChangeSauvegardeTousEtat(bool sauvegarde_tous);
		void ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde);

	private:

		void CalculIndiceRadiation(DATE_HEURE date_heure, unsigned short pas_de_temps, ZONE& zone, size_t index_zone);

		float ConductiviteNeige(float densite);

		void CalculeFonte(ZONE& zone, size_t index_zone, unsigned short pas_de_temps, float proportion_terrain, float coeff_fonte, 
			float temperaure_de_fonte, float& albedo, float& stock_neige, float& hauteur_neige,float& chaleur_stock, 
			float& apport, float& eau_retenue);

		float Erf(float x);

		void LectureEtat(DATE_HEURE date_courante);
		void SauvegardeEtat(DATE_HEURE date_courante) const;

		// mise a jour de la neige

		struct MISE_A_JOUR_NEIGE
		{
			int nb_pas_derniere_correction;

			float pourcentage_sim_eq;
			float pourcentage_sim_ha;

			float pourcentage_corrige;

			std::vector<bool> stations_utilisees;
		};

		void MiseAJour(const DATE_HEURE& date_courante, size_t index_zone);
		void MiseAJourGrille(size_t index_zone, STATION_NEIGE::typeOccupationStation occupation);

		void UneMiseAJourOccupation(float mesure, float& valeur_actuelle, float valeur_ref, float fraction_corrigee, float ponderation, float &fraction_simulee);

		bool _mise_a_jour_neige;

		STATIONS_NEIGE _stations_neige_feuillus;
		STATIONS_NEIGE _stations_neige_conifers;
		STATIONS_NEIGE _stations_neige_decouver;

		TYPE_INTERPOLATION _interpolation_feuillus;
		TYPE_INTERPOLATION _interpolation_conifers;
		TYPE_INTERPOLATION _interpolation_decouver;

		MATRICE<double> _ponderation_feuillus;	//for thiessen2 & moy3station2
		MATRICE<double> _ponderation_conifers;
		MATRICE<double> _ponderation_decouver;

		MATRICE<float> _ponderation_feuillusF;	//for thiessen1 & moy3station1
		MATRICE<float> _ponderation_conifersF;
		MATRICE<float> _ponderation_decouverF;

		std::vector<MISE_A_JOUR_NEIGE> _maj_feuillus;
		std::vector<MISE_A_JOUR_NEIGE> _maj_conifers;
		std::vector<MISE_A_JOUR_NEIGE> _maj_decouver;

		// parametres

		std::vector<float> _seuil_fonte_feuillus;	// C
		std::vector<float> _seuil_fonte_conifers;	// C
		std::vector<float> _seuil_fonte_decouver;	// C

		std::vector<float> _taux_fonte_feuillus;	// mm/jour.C
		std::vector<float> _taux_fonte_conifers;	// mm/jour.C
		std::vector<float> _taux_fonte_decouver;	// mm/jour.C

		std::vector<float> _taux_fonte;				// mm/jour
		std::vector<float> _densite_maximale;		// kg/m3
		std::vector<float> _constante_tassement;

		std::vector<float> _seuil_albedo;

		std::vector<size_t> _index_occupation_feuillus;
		std::vector<size_t> _index_occupation_conifers;

		int _nbr_jour_delai_mise_a_jour;

		// variable pour lecture/ecriture de l'etat

		std::string _nom_fichier_lecture_etat;
		std::string _repertoire_ecriture_etat;

		DATE_HEURE _date_sauvegarde_etat;
		bool _sauvegarde_etat;

		bool _sauvegarde_tous_etat;

		//

		bool			_bMAJGrilleNeige;
		GRILLE_NEIGE	_grilleneige;

		// variables de simulation

		THIESSEN1*				_pThiessen1;		//for snow cover update
		THIESSEN2*				_pThiessen2;		//
		MOYENNE_3_STATIONS1*	_pMoy3station1;		//
		MOYENNE_3_STATIONS2*	_pMoy3station2;		//

		std::ofstream		_fichier_couvert_nival;
		std::ofstream		_fichier_hauteur_neige;
		std::ofstream		_fichier_albedo_neige;

		std::vector<float> _ce1;
		std::vector<float> _ce0;
		std::vector<float> _tsn;

		std::vector<float> _stock_feuillus;		//stock (en eau) de neige [m]
		std::vector<float> _stock_conifers;		//stock (en eau) de neige [m]
		std::vector<float> _stock_decouver;		//stock (en eau) de neige [m]

		std::vector<float> _hauteur_feuillus;	//hauteur de neige [m]
		std::vector<float> _hauteur_conifers;	//hauteur de neige [m]
		std::vector<float> _hauteur_decouver;	//hauteur de neige [m]

		std::vector<float> _chaleur_feuillus;	//chaleur du stock
		std::vector<float> _chaleur_conifers;	//chaleur du stock
		std::vector<float> _chaleur_decouver;	//chaleur du stock

		std::vector<float> _eau_retenu_feuillus;	//eau retenue dans le stock de neige
		std::vector<float> _eau_retenu_conifers;	//eau retenue dans le stock de neige
		std::vector<float> _eau_retenu_decouver;	//eau retenue dans le stock de neige

		std::vector<float> _albedo_feuillus;	//albedo du stock de neige
		std::vector<float> _albedo_conifers;	//albedo du stock de neige
		std::vector<float> _albedo_decouver;	//albedo du stock de neige

		std::vector<float> _pourcentage_feuillus;
		std::vector<float> _pourcentage_conifers;
		std::vector<float> _pourcentage_autres;

		std::vector<int> _methode_albedo;

		std::vector<CORRECTION*> _corrections_neige_au_sol;

		OUTPUT*				_pOutput;

		float*				_netCdf_couvertnival;
		float*				_netCdf_hauteurneige;
		float*				_netCdf_albedoneige;
	};

}

#endif
