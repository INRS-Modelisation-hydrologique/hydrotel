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

#ifndef OCCUPATION_H_INCLUDED
#define OCCUPATION_H_INCLUDED


#include "matrice.hpp"
#include "zones.hpp"

#include <string>
#include <vector>


namespace HYDROTEL
{

	class OCCUPATION_SOL
	{
	public:
		OCCUPATION_SOL();
		~OCCUPATION_SOL();

		std::string PrendreNomFichier() const;

		std::string PrendreNomFichierIndicesFolieres() const;

		std::string PrendreNomFichierProfondeursRacinaires() const;

		std::string PrendreNomFichierAlbedo() const;

		std::string PrendreNomFichierHauteurVegetation() const;

		// retourne le nombre de classe d'occupation du sol
		size_t PrendreNbClasse() const;

		// retourne le pourcentage d'occupation
		float PrendrePourcentage(size_t index_zone, size_t index_classe);
		double PrendrePourcentage_double(size_t index_zone, size_t index_classe);

		float PrendreIndiceFoliaire(size_t index, int jour_julien);

		float PrendreProfondeurRacinaire(size_t index, int jour_julien);

		float PrendreAlbedo(size_t index, int jour_julien);

		float PrendreHauteurVegetation(size_t index, int jour_julien);

		void ChangeNomFichier(const std::string& nom_fichier);

		void ChangeNomFichierIndicesFolieres(const std::string& nom_fichier);

		void ChangeNomFichierProfondeursRacinaires(const std::string& nom_fichier);

		void ChangeNomFichierAlbedo(const std::string& nom_fichier);

		void ChangeNomFichierHauteurVegetation(const std::string& nom_fichier);

		void Lecture(const ZONES& zones);

		void LectureIndicesFolieres(int annee);

		void LectureProfondeursRacinaires(int annee);

		void LectureAlbedo(int annee);

		void LectureHauteurVegetation(int annee);

		// supprime les donnees d'occupation
		void clear();

		void LectureFichierInformation(std::string nom_fichier, int annee, std::vector<int>& jours, MATRICE<float>& valeurs);

	public:
		struct CLASSE_OCCUPATION_SOL
		{
			std::string nom;
		};

		std::vector<CLASSE_OCCUPATION_SOL> _classes_occupation_sol;

	private:
		struct INFORMATION
		{
			int jour_julien;
			std::vector<float> valeur;
		};

		std::string _nom_fichier;

		std::string _nom_fichier_indices_folieres;
		std::string _nom_fichier_profondeurs_racinaires;
		std::string _nom_fichier_albedo;
		std::string _nom_fichier_hauteur_vegetation;

		MATRICE<float> _pourcentages;
		MATRICE<double> _pourcentages_double;

		std::vector<INFORMATION> _indices_folieres;
		std::vector<INFORMATION> _profondeurs_racinaires;
		std::vector<INFORMATION> _albedo;
		std::vector<INFORMATION> _hauteur_vegetation;

		int _annee_courante_indices_folieres;
		int _annee_courante_profondeurs_racinaires;
		int _annee_courante_albedo;
		int _annee_courante_hauteur_vegetation;
	};

}

#endif
