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

#ifndef ONDE_CINEMATIQUE_H_INCLUDED
#define ONDE_CINEMATIQUE_H_INCLUDED


#include "ruisselement_surface.hpp"


namespace HYDROTEL
{

	class ONDE_CINEMATIQUE : public RUISSELEMENT_SURFACE
	{
	public:
		ONDE_CINEMATIQUE(SIM_HYD& sim_hyd);
		virtual ~ONDE_CINEMATIQUE();

		virtual void Initialise();

		virtual void Calcule();

		virtual void Termine();

		virtual void LectureParametres();

		void LectureParametresFichierGlobal();

		virtual void SauvegardeParametres();

		double PrendreLame() const;

		std::string PrendreNomFichierHgm() const;

		std::vector<size_t> PrendreIndexForets() const;

		std::vector<size_t> PrendreIndexEaux() const;

		double PrendreManningForet(size_t index_zone) const;

		double PrendreManningEaux(size_t index_zone) const;

		double PrendreManningAutres(size_t index_zone) const;

		std::string PrendreNomFichierLectureEtat() const;

		std::string PrendreRepertoireEcritureEtat() const;

		bool PrendreSauvegardeTousEtat() const;

		DATE_HEURE PrendreDateHeureSauvegardeEtat() const;

		virtual void ChangeNbParams(const ZONES& zones);

		void ChangeNomFichierHGM(const std::string& nom_fichier);

		void ChangeLame(double lame);

		void ChangeManningForet(size_t index_zone, double manning);

		void ChangeManningEau(size_t index_zone, double manning);

		void ChangeManningAutre(size_t index_zone, double manning);

		void ChangeIndexForets(std::vector<size_t> index);

		void ChangeIndexEaux(std::vector<size_t> index);

		void CalculeHgm(double lame, std::string nom_fichier);

		void ChangeNomFichierLectureEtat(std::string nom_fichier);

		void ChangeRepertoireEcritureEtat(std::string repertoire);

		void ChangeSauvegardeTousEtat(bool sauvegarde_tous);

		void ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde);

	private:

		struct oc_zone
		{
			std::vector<double> debits;     // debits a l'exutoire de la zone.
			std::vector<double> distri;     // distribution des debits dans le temps
		};

		struct rectangle
		{
			size_t min_lig;
			size_t max_lig;
			size_t min_col;
			size_t max_col;

			size_t nb_car;
		};
		
		void CalculePourcentageOccupation();
		void LectureHgm();
		void SauvegardeHgm();

		void CalculeHgm();

		//void TriCarreaux(const RASTER<int>& grille, const RASTER<int>& orientations, rectangle r, int ident, std::vector<size_t>& ind_lig, std::vector<size_t>& ind_col);
		//void TriCarreaux2(RasterInt2* uhrh, rectangle2 r, int ident, std::vector<size_t>& ind);
		void TriCarreaux3(RasterInt2* uhrh, rectangle r, int ident, std::vector<size_t>& ind);

		void Ruisselement(double arete, double pte, double man, double ra, double rb, double rc, double p, int dt, double& rd);

		void LectureEtat(DATE_HEURE date_courante);
		void SauvegardeEtat(DATE_HEURE date_courante) const;

		std::string _nom_fichier_hgm;

		double _lame;	// m

		std::vector<double> _manning_forets;
		std::vector<double> _manning_eaux;
		std::vector<double> _manning_autres;

		std::vector<size_t> _index_forets;
		std::vector<size_t> _index_eaux;
		std::vector<size_t> _index_autres;

		size_t _nb_debit;

		// variable pour lecture/ecriture de l'etat

		std::string _nom_fichier_lecture_etat;
		std::string _repertoire_ecriture_etat;

		DATE_HEURE _date_sauvegarde_etat;
		bool _sauvegarde_etat;

		bool _sauvegarde_tous_etat;

		// variables de simulation

		std::vector<double> _pourcentage_forets;
		std::vector<double> _pourcentage_eaux;
		std::vector<double> _pourcentage_autres;

		std::vector<oc_zone> _oc_zone;

		std::vector<oc_zone> _oc_surf;
		std::vector<oc_zone> _oc_hypo;
		std::vector<oc_zone> _oc_base;
	};

}

#endif
