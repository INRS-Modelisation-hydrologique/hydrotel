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

#include "occupation_sol.hpp"

#include "constantes.hpp"
#include "erreur.hpp"
#include "util.hpp"

#include <fstream>
#include <sstream>


using namespace std;


namespace HYDROTEL
{

	OCCUPATION_SOL::OCCUPATION_SOL()
	{
		_annee_courante_indices_folieres = -1;
		_annee_courante_profondeurs_racinaires = -1;
		_annee_courante_albedo = -1;
		_annee_courante_hauteur_vegetation = -1;
	}

	OCCUPATION_SOL::~OCCUPATION_SOL()
	{
	}

	string OCCUPATION_SOL::PrendreNomFichier() const
	{
		return _nom_fichier;
	}

	size_t OCCUPATION_SOL::PrendreNbClasse() const
	{
		return _classes_occupation_sol.size();
	}

	void OCCUPATION_SOL::ChangeNomFichier(const string& nom_fichier)
	{
		_nom_fichier = nom_fichier;
	}

	void OCCUPATION_SOL::ChangeNomFichierIndicesFolieres(const std::string& nom_fichier)
	{
		_nom_fichier_indices_folieres = nom_fichier;
	}

	void OCCUPATION_SOL::ChangeNomFichierProfondeursRacinaires(const std::string& nom_fichier)
	{
		_nom_fichier_profondeurs_racinaires = nom_fichier;
	}

	void OCCUPATION_SOL::ChangeNomFichierAlbedo(const std::string& nom_fichier)
	{
		_nom_fichier_albedo = nom_fichier;
	}

	void OCCUPATION_SOL::ChangeNomFichierHauteurVegetation(const std::string& nom_fichier)
	{
		_nom_fichier_hauteur_vegetation = nom_fichier;
	}

	void OCCUPATION_SOL::Lecture(const ZONES& zones)
	{		
		if (FichierExiste(_nom_fichier) )
		{
			ifstream fichier(_nom_fichier);

			vector<string> sList;
			int format;
			size_t nb_classe;
			string str;

			fichier >> format >> nb_classe;

			MATRICE<float> pourcentages(zones.PrendreNbZone(), nb_classe, 0.0f);
			MATRICE<double> pourcentages_double(zones.PrendreNbZone(), nb_classe, 0.0);
			vector<CLASSE_OCCUPATION_SOL> classes_occupation_sol(nb_classe);

			getline_mod(fichier, str);
			if(str=="")
				getline_mod(fichier, str);


			if(str.find_first_of('\"') != string::npos)
				SplitString2(sList, str, "\"", true);	//version physitel4
			else
				SplitString2(sList, str, " \t", true);	//version physitel3


			if(sList.size()-1 != nb_classe)
				throw ERREUR("error reading `occupation_sol.cla` file: the class number is invalid");

			for (size_t n = 1; n <= nb_classe; n++)
				classes_occupation_sol[n - 1].nom = sList[n];


			for (size_t i = 0; i < zones.PrendreNbZone(); ++i)
			{
				int ident;
				fichier >> ident;

				size_t index_zone = zones.IdentVersIndex(ident);

				for (size_t j = 0; j < nb_classe; ++j)
				{
					fichier >> pourcentages_double(index_zone, j);		//nb pixel
					pourcentages(index_zone, j) = static_cast<float>(pourcentages_double(index_zone, j));
				}

				float total = 0.0f;								//calcul le pourcentage
				double total_double = 0.0;

				for (size_t j = 0; j < nb_classe; ++j)
				{
					total += pourcentages(index_zone, j);
					total_double += pourcentages_double(index_zone, j);
				}

				if (total_double != 0.0)
				{
					for (size_t j = 0; j < nb_classe; ++j)
					{
						pourcentages(index_zone, j) /= total;
						pourcentages_double(index_zone, j) /= total_double;
					}
				}
			}
			
			fichier.close();

			_classes_occupation_sol.swap(classes_occupation_sol);
			_pourcentages = pourcentages;
			_pourcentages_double = pourcentages_double;
		}
		else
		{
			//genere le fichier CLA
			map<size_t,vector<int>> mapNbCell;
			istringstream iss;
			ostringstream oss;
			COORDONNEE coord;
			string sString;
			size_t index_zone, i, stNbClasse;
			size_t nbLigUhrh, nbColUhrh, lig, col;
			int x, nb_classe, ident, iClasseSol, iVal;
			int iligOcc, icolOcc, iNoDataUhrh, iNoDataOcc;

			string nom_fichier_grille = RemplaceExtension(_nom_fichier, "tif");
			string nom_fichier_classes = RemplaceExtension(_nom_fichier, "csv");

			RASTER<int>	grilleOccSol = LectureRaster_int(nom_fichier_grille);
			iNoDataOcc = grilleOccSol.PrendreNoData();

			const RASTER<int>&	grilleZones = zones.PrendreGrille();
			nbColUhrh = grilleZones.PrendreNbColonne();
			nbLigUhrh = grilleZones.PrendreNbLigne();
			iNoDataUhrh = grilleZones.PrendreNoData();

			//lecture du nom des classes
			vector<CLASSE_OCCUPATION_SOL> classes_occupation_sol;

			try{

			ifstream fichier(nom_fichier_classes);

			if (!fichier)
				throw ERREUR("error opening: " + nom_fichier_classes);

			getline_mod(fichier, sString);
			iss.clear();
			iss.str(sString);
			iss >> nb_classe;

			if(nb_classe < 1 || nb_classe > 10000)
				throw ERREUR("error reading file " + nom_fichier_classes + ": invalid format");

			stNbClasse = static_cast<size_t>(nb_classe);
			
			classes_occupation_sol.resize(stNbClasse);

			for(x=0; x<nb_classe; x++)
			{
				getline_mod(fichier, sString);
				classes_occupation_sol[x].nom = sString;
			}

			fichier.close();

			}
			catch(const ERREUR& err)
			{
				throw err;
			}
			catch(...)
			{
				throw ERREUR("error reading file occupation_sol.csv");
			}

			try{

			//init à 0 le vecteur qui contiendra le nb de cell par classe d'occupation
			for (i = 0; i < zones.PrendreNbZone(); i++)
			{
				for (x=0; x<nb_classe; x++)
					mapNbCell[i].push_back(0);
			}

			//parcours la matrice des uhrh
			for (lig = 0; lig < nbLigUhrh; lig++)
			{
				for (col = 0; col < nbColUhrh; col++)
				{
					ident = grilleZones(lig, col);
					if(ident != iNoDataUhrh)
					{
						//transforme en coordonnee avant de revenir en coordonnee relative sur la carte occsol; l'extent de celle ci peut différé de la carte uhrh
						coord = grilleZones.LigColVersCoordonnee(static_cast<int>(lig), static_cast<int>(col));
						grilleOccSol.CoordonneeVersLigCol(coord, iligOcc, icolOcc);

						iClasseSol = grilleOccSol(iligOcc, icolOcc);
						if(iClasseSol != iNoDataOcc)
						{
							if(iClasseSol < 1)
							{
								oss.str("");
								oss << "erreur lors de la generation du fichier occupation_sol.cla: l`identifiant " << iClasseSol << " de la carte d`occupation du sol est invalide.";
								throw ERREUR(oss.str());
							}

							if(iClasseSol > nb_classe)
							{
								oss.str("");
								oss << "erreur lors de la generation du fichier occupation_sol.cla: l`identifiant " << iClasseSol << " a ete trouve dans la carte d`occupation du sol mais le fichier occupation_sol.csv contient seulement " << nb_classe << " classes.";
								throw ERREUR(oss.str());
							}

							index_zone = zones.IdentVersIndex(ident);

							iVal = mapNbCell[index_zone].at(iClasseSol-1);
							++iVal;
							mapNbCell[index_zone].at(iClasseSol-1) = iVal;
						}
					}
				}
			}

			//calcule les pourcentages & creation du fichier CLA ( nb de tuile (pixel) )
			MATRICE<float> pourcentages(zones.PrendreNbZone(), stNbClasse, 0.0f);
			MATRICE<double> pourcentages_double(zones.PrendreNbZone(), stNbClasse, 0.0);

			ofstream fichierCLA(_nom_fichier);
			
			fichierCLA << "1\n";				//format
			fichierCLA << nb_classe << "\n";	//nb de classe

			fichierCLA << "uhrh";				//nom des occupations du sol
			for(x=0; x<nb_classe; x++)
				fichierCLA << " " << "\"" << classes_occupation_sol[x].nom << "\"";
			fichierCLA << "\n";

			for (i = 0; i < zones.PrendreNbZone(); i++)
			{
				ident = zones[i].PrendreIdent();

				fichierCLA << ident;				//nom des occupations du sol
				for(x=0; x<nb_classe; x++)			//nombre de pixel par classe d'occupation
					fichierCLA << " " << mapNbCell[i].at(x);
				fichierCLA << "\n";

				for (size_t j = 0; j < stNbClasse; ++j)
				{
					pourcentages(i, j) = static_cast<float>(mapNbCell[i].at(j));
					pourcentages_double(i, j) = static_cast<double>(mapNbCell[i].at(j));
				}

				float total = 0;
				double total_double = 0.0;

				for (size_t j = 0; j < stNbClasse; ++j)
				{
					total+= pourcentages(i, j);
					total_double+= pourcentages_double(i, j);
				}

				if (total_double != 0.0)
				{
					for (size_t j = 0; j < stNbClasse; ++j)
					{
						pourcentages(i, j)/= total;
						pourcentages_double(i, j)/= total_double;
					}
				}
			}

			fichierCLA.close();
			_classes_occupation_sol.swap(classes_occupation_sol);
			_pourcentages = pourcentages;
			_pourcentages_double = pourcentages_double;

			}
			catch(const ERREUR& err)
			{
				throw err;
			}
			catch(...)
			{
				throw ERREUR("Error creating occupation_sol.cla file: " + _nom_fichier);
			}
		}
	}


	float OCCUPATION_SOL::PrendrePourcentage(size_t index_zone, size_t index_classe)
	{
		return _pourcentages(index_zone, index_classe);
	}


	double OCCUPATION_SOL::PrendrePourcentage_double(size_t index_zone, size_t index_classe)
	{
		return _pourcentages_double(index_zone, index_classe);
	}


	void OCCUPATION_SOL::LectureFichierInformation(string nom_fichier, int annee, vector<int>& jours, MATRICE<float>& valeurs)
	{
		istringstream iss;
		ostringstream oss;
		vector<string> sList;
		string sString;
		int nb_cot, nb_jour, i, j;

		oss << annee;
		nom_fichier = RemplaceExtension(nom_fichier, oss.str());

		if (!FichierExiste(nom_fichier))
			nom_fichier = RemplaceExtension(nom_fichier, "def");

		ifstream fichier(nom_fichier);
		if (!fichier)
			throw ERREUR("filename is invalid.");
		
		int ligne = 1;

		try
		{
			getline_mod(fichier, sString);	//type

			++ligne;
			getline_mod(fichier, sString);	//nb_occ	nb_jour
			SplitString(sList, sString, " \t", true, true);	//separateur; espace et \t
			if(sList.size() != 2)
				throw ERREUR("invalid file: the second data line (nb_occ nb_jour) is invalid.");

			iss.clear();
			iss.str(sList[0]);
			iss >> nb_cot;

			if(nb_cot != static_cast<int>(PrendreNbClasse()))
				throw ERREUR("invalid file: the land cover class number do not match the class number of the project file.");

			iss.clear();
			iss.str(sList[1]);
			iss >> nb_jour;

			if(nb_jour <= 0)
				throw ERREUR("invalid file: the number of days is invalid.");

			jours.resize(nb_jour);
			valeurs = MATRICE<float>(nb_jour, nb_cot);

			++ligne;
			getline_mod(fichier, sString);	//commentaire; description du fichier

			++ligne;
			getline_mod(fichier, sString);	//commentaire; nom des classes d'occupation du sol

			i = 0;
			++ligne;
			sString = "";
			getline_mod(fichier, sString);

			while(!fichier.eof() || sString != "")
			{
				if(i < nb_jour)
				{
					SplitString(sList, sString, " \t", true, true);
					if(sList.size() != static_cast<size_t>(nb_cot+1))
					{
						oss.str("");
						oss << "invalid file: the number of columns is invalid at line " << ligne;
						throw ERREUR(oss.str());
					}

					iss.clear();
					iss.str(sList[0]);
					iss >> jours[i];

					for(j=0; j<nb_cot; j++)
					{
						iss.clear();
						iss.str(sList[j+1]);
						iss >> valeurs(i, j);
					}

					++i;
				}
				else
				{
					if(sString != "")
						++i;
				}

				++ligne;
				sString = "";

				try{
				getline_mod(fichier, sString);
				}
				catch(...)
				{
					sString = "";
				}
			}

			fichier.close();

			if(i != nb_jour)
				throw ERREUR("invalid file: the number of days is invalid (number of lines).");

			if(jours[nb_jour-1] != 365)
				throw ERREUR("invalid file: the last day must be day 365.");

		}
		catch(const ERREUR& err)
		{
			throw err;
		}
		catch (...)
		{
			oss.str("");
			oss << "erreur de lecture a la ligne " << ligne;
			throw ERREUR(oss.str());
		}
	}


	void OCCUPATION_SOL::LectureIndicesFolieres(int annee)
	{
		if (annee != _annee_courante_indices_folieres)
		{
			vector<int> jours;
			MATRICE<float> valeurs;

			try{
			LectureFichierInformation(_nom_fichier_indices_folieres, annee, jours, valeurs);
			
			}
			catch (const ERREUR& err)
			{
				throw ERREUR_LECTURE_FICHIER("FICHIER INDICE FOLIERE; (" + _nom_fichier_indices_folieres + "); " + err.what());
			}
			
			_indices_folieres.resize(jours.size());

			for (size_t index = 0; index < jours.size(); ++index)
			{
				_indices_folieres[index].jour_julien = jours[index];
				_indices_folieres[index].valeur.resize(PrendreNbClasse());

				for (size_t n = 0; n < PrendreNbClasse(); ++n)
					_indices_folieres[index].valeur[n] = valeurs(index, n);
			}

			_annee_courante_indices_folieres = annee;
		}
	}

	void OCCUPATION_SOL::LectureProfondeursRacinaires(int annee)
	{
		if (annee != _annee_courante_profondeurs_racinaires)
		{
			vector<int> jours;
			MATRICE<float> valeurs;

			try{
			LectureFichierInformation(_nom_fichier_profondeurs_racinaires, annee, jours, valeurs);
			
			}
			catch (const ERREUR& err)
			{
				throw ERREUR_LECTURE_FICHIER("FICHIER PROFONDEUR RACINAIRE; (" + _nom_fichier_profondeurs_racinaires + "); " + err.what());
			}

			_profondeurs_racinaires.resize(jours.size());

			for (size_t index = 0; index < jours.size(); ++index)
			{
				_profondeurs_racinaires[index].jour_julien = jours[index];
				_profondeurs_racinaires[index].valeur.resize(PrendreNbClasse());

				for (size_t n = 0; n < PrendreNbClasse(); ++n)
					_profondeurs_racinaires[index].valeur[n] = valeurs(index, n);
			}

			_annee_courante_profondeurs_racinaires = annee;
		}
	}

	void OCCUPATION_SOL::LectureAlbedo(int annee)
	{
		if (annee != _annee_courante_albedo)
		{
			vector<int> jours;
			MATRICE<float> valeurs;

			try{
			LectureFichierInformation(_nom_fichier_albedo, annee, jours, valeurs);
			
			}
			catch (const ERREUR& err)
			{
				throw ERREUR_LECTURE_FICHIER("FICHIER ALBEDO; (" + _nom_fichier_albedo + "); " + err.what());
			}

			_albedo.resize(jours.size());

			for (size_t index = 0; index < jours.size(); ++index)
			{
				_albedo[index].jour_julien = jours[index];
				_albedo[index].valeur.resize(PrendreNbClasse());

				for (size_t n = 0; n < PrendreNbClasse(); ++n)
					_albedo[index].valeur[n] = valeurs(index, n);
			}

			_annee_courante_albedo = annee;
		}
	}

	void OCCUPATION_SOL::LectureHauteurVegetation(int annee)
	{
		if (annee != _annee_courante_hauteur_vegetation)
		{
			vector<int> jours;
			MATRICE<float> valeurs;

			try{
			LectureFichierInformation(_nom_fichier_hauteur_vegetation, annee, jours, valeurs);
			
			}
			catch (const ERREUR& err)
			{
				throw ERREUR_LECTURE_FICHIER("FICHIER HAUTEUR VEGETATION; (" + _nom_fichier_hauteur_vegetation + "); " + err.what());
			}

			_hauteur_vegetation.resize(jours.size());

			for (size_t index = 0; index < jours.size(); ++index)
			{
				_hauteur_vegetation[index].jour_julien = jours[index];
				_hauteur_vegetation[index].valeur.resize(PrendreNbClasse());

				for (size_t n = 0; n < PrendreNbClasse(); ++n)
					_hauteur_vegetation[index].valeur[n] = valeurs(index, n);
			}

			_annee_courante_hauteur_vegetation = annee;
		}
	}

	void OCCUPATION_SOL::clear()
	{
		_indices_folieres.clear();
		_profondeurs_racinaires.clear();
		_albedo.clear();
		_hauteur_vegetation.clear();

		_annee_courante_indices_folieres = -1;
		_annee_courante_profondeurs_racinaires = -1;
		_annee_courante_albedo = -1;
		_annee_courante_hauteur_vegetation = -1;
	}

	float OCCUPATION_SOL::PrendreIndiceFoliaire(size_t index, int jour_julien)
	{
		BOOST_ASSERT(jour_julien >= 1 && jour_julien <= 366);

		size_t index2 = 0;

		while (index2 < _indices_folieres.size() && jour_julien > _indices_folieres[index2].jour_julien)
			++index2;

		if (index2 == _indices_folieres.size())
			--index2;
		
		if (jour_julien == _indices_folieres[index2].jour_julien)
			return _indices_folieres[index2].valeur[index];

		size_t index1 = index2 - 1;

		return max(0.0f, 
			InterpolationLineaire(
				static_cast<float>(_indices_folieres[index1].jour_julien), 
				_indices_folieres[index1].valeur[index], 
				static_cast<float>(_indices_folieres[index2].jour_julien), 
				_indices_folieres[index2].valeur[index], 
				static_cast<float>(jour_julien)));
	}

	float OCCUPATION_SOL::PrendreProfondeurRacinaire(size_t index, int jour_julien)
	{
		BOOST_ASSERT(jour_julien >= 1 && jour_julien <= 366);

		size_t index2 = 0;

		while (index2 < _profondeurs_racinaires.size() && jour_julien > _profondeurs_racinaires[index2].jour_julien)
			++index2;

		if (index2 == _profondeurs_racinaires.size())
			--index2;
		
		if (jour_julien == _profondeurs_racinaires[index2].jour_julien)
			return _profondeurs_racinaires[index2].valeur[index];

		size_t index1 = index2 - 1;

		return max(0.0f, 
			InterpolationLineaire(
			static_cast<float>(_profondeurs_racinaires[index1].jour_julien), 
			_profondeurs_racinaires[index1].valeur[index], 
			static_cast<float>(_profondeurs_racinaires[index2].jour_julien), 
			_profondeurs_racinaires[index2].valeur[index], 
			static_cast<float>(jour_julien)));
	}

	float OCCUPATION_SOL::PrendreAlbedo(size_t index, int jour_julien)
	{
		BOOST_ASSERT(jour_julien >= 1 && jour_julien <= 366);

		size_t index2 = 0;

		while (index2 < _albedo.size() && jour_julien > _albedo[index2].jour_julien)
			++index2;

		if (index2 == _albedo.size())
			--index2;
		
		if (jour_julien == _albedo[index2].jour_julien)
			return _albedo[index2].valeur[index];

		size_t index1 = index2 - 1;

		return max(0.0f, 
			InterpolationLineaire(
			static_cast<float>(_albedo[index1].jour_julien), 
			_albedo[index1].valeur[index], 
			static_cast<float>(_albedo[index2].jour_julien), 
			_albedo[index2].valeur[index], 
			static_cast<float>(jour_julien)));
	}

	float OCCUPATION_SOL::PrendreHauteurVegetation(size_t index, int jour_julien)
	{
		BOOST_ASSERT(jour_julien >= 1 && jour_julien <= 366);

		size_t index2 = 0;

		while (index2 < _hauteur_vegetation.size() && jour_julien > _hauteur_vegetation[index2].jour_julien)
			++index2;

		if (index2 == _hauteur_vegetation.size())
			--index2;
		
		if (jour_julien == _hauteur_vegetation[index2].jour_julien)
			return _hauteur_vegetation[index2].valeur[index];

		size_t index1 = index2 - 1;

		return max(0.0f, 
			InterpolationLineaire(
			static_cast<float>(_hauteur_vegetation[index1].jour_julien), 
			_hauteur_vegetation[index1].valeur[index], 
			static_cast<float>(_hauteur_vegetation[index2].jour_julien), 
			_hauteur_vegetation[index2].valeur[index], 
			static_cast<float>(jour_julien)));
	}

	string OCCUPATION_SOL::PrendreNomFichierIndicesFolieres() const
	{
		return _nom_fichier_indices_folieres;
	}

	string OCCUPATION_SOL::PrendreNomFichierProfondeursRacinaires() const
	{
		return _nom_fichier_profondeurs_racinaires;
	}

	string OCCUPATION_SOL::PrendreNomFichierAlbedo() const
	{
		return _nom_fichier_albedo;
	}

	string OCCUPATION_SOL::PrendreNomFichierHauteurVegetation() const
	{
		return _nom_fichier_hauteur_vegetation;
	}

}
