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

#ifndef RANKINEN_H_INCLUDED
#define RANKINEN_H_INCLUDED


#include "tempsol.hpp"


namespace HYDROTEL
{

	class RANKINEN : public TEMPSOL
	{
	public:

		RANKINEN(SIM_HYD& sim_hyd);
		virtual ~RANKINEN();

		virtual void	ChangeNbParams(const ZONES& zones);

		virtual void	Initialise();
		virtual void	Calcule();
		virtual void	Termine();

		virtual void	LectureParametres();
		void			LectureParametresFichierGlobal();

		virtual void	SauvegardeParametres();

		//variables d'etats

		void			ChangeNomFichierLectureEtat(std::string nom_fichier);
		void			ChangeRepertoireEcritureEtat(std::string repertoire);
		void			ChangeSauvegardeTousEtat(bool sauvegarde_tous);
		void			ChangeDateHeureSauvegardeEtat(bool sauvegarde, DATE_HEURE date_sauvegarde);

		std::string		PrendreNomFichierLectureEtat() const;
		std::string		PrendreRepertoireEcritureEtat() const;
		bool			PrendreSauvegardeTousEtat() const;
		DATE_HEURE		PrendreDateHeureSauvegardeEtat() const;

	private:

		void		LectureEtat(DATE_HEURE date_courante);
		void		SauvegardeEtat(DATE_HEURE date_courante) const;

	private:

		//parametres
		float									_fSeuilGel;						//C				//seuil de temperature du gel au sol
		float									_fIntervalleProfil;				//m				//interval du profil
		float									_fTempIniProfondeur;			//C				//temperature initiale au fond du profil

		float									_fParamFS;						//				//parametre empirique lie a l'epaisseur de neige

		std::vector<float>						_vfParamKT;						//W/m/s			//conductivite thermique du sol	gelé				//pour chaque type de sol
		std::vector<float>						_vfParamCS;						//J/m3/C		//capacite thermique specifique du sol				//pour chaque type de sol
		std::vector<float>						_vfParamCIce;					//J/m3/C		//capacite thermique specifique lie au gel/degel	//pour chaque type de sol

		//variables de simulation
		std::map<size_t, std::vector<float>>	_mapTemperature;				//C				//temperature du sol pour chaque UHRH et pour chaque intervalle

		//fichier output
		std::ofstream							_fichier_tempsol;

		std::vector<size_t>						_vOutTempUHRH;
		std::vector<std::ofstream*>				_vOutTempFile;

		std::string								_nom_fichier_lecture_etat;
		std::string								_repertoire_ecriture_etat;
		DATE_HEURE								_date_sauvegarde_etat;
		bool									_sauvegarde_etat;
		bool									_sauvegarde_tous_etat;
	};

}

#endif
