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

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED


#include "date_heure.hpp"
#include "projection.hpp"
#include "raster.hpp"

#include <map>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/assert.hpp>


namespace HYDROTEL 
{

	//Retourne en decimal degree (sString; coordonnee provenant des fichiers de donnees)
	double	ParseLatLongCoord(std::string sString, bool bLongitude);

	//compute Shreve method of stream ordering and add to troncon.trl file
	bool	ShreveCompute(const std::string& sTronconFile);

	// lecture d'une matrice de reel physitel avec multiplication des valeurs
	RASTER<float> LectureRasterPhysitel_float(const std::string& nom_fichier, float mult = 1);

	// lecture d'une matrice d'entier physitel
	RASTER<int> LectureRasterPhysitel_int(const std::string& nom_fichier);

	// lecture d'une matrice reel georeference
	RASTER<float> LectureRaster_float(const std::string& nom_fichier, float mult = 1);

	// lecture d'une matrice d'entier georeference
	RASTER<int> LectureRaster_int(const std::string& nom_fichier);

	// extrait le nom du fichier sans extension
	std::string ExtraitNomFichier(const std::string& nom_fichier);

	// retourne le repertoire du nom de fichier
	std::string PrendreRepertoire(const std::string& nom_fichier);

	// retourne l'extension du nom de fichier
	std::string PrendreExtension(const std::string& nom_fichier);

	std::string PrendreFilename(const std::string& nom_fichier);

	// retourne le nom de fichier avec la nouvelle extension
	std::string RemplaceExtension(const std::string& nom_fichier, const std::string& ext);

	// retourne vrai si le fichier existe
	bool FichierExiste(const std::string& nom_fichier);

	// retourne vrai si le repertoire existe
	bool RepertoireExiste(const std::string& repertoire);

	// supprime le fichier s'il existe
	void SupprimerFichier(const std::string& nom_fichier);

	// lire une chaine (@n ...) dans une stream
	std::string LireChaine(std::istream& stream, size_t nb_flag = 0);

	// lire un nom de fichier et y ajouter un repertoire
	std::string LireNomFichier(const std::string& repertoire, std::istream& stream, size_t nb_flag = 0);

	size_t GetIndexNearestCoord(const std::vector<COORDONNEE>& coordonnees, const COORDONNEE& coordonnee);

	// retourne les index des coordonnees les plus proche de la coordonnee
	std::vector<size_t> CalculDistance(const std::vector<COORDONNEE>& coordonnees, const COORDONNEE& coordonnee);

	// retourne les distances des stations par rapport à une coordonnee (pixel)
	void CalculDistanceEx(const std::vector<COORDONNEE>& coordonnees, const COORDONNEE& coordonnee, std::vector<double>* vDistances);

	// calcul la densite de la neige selon la temperature
	float CalculDensiteNeige(float temperature);
	double CalculDensiteNeige(double temperature);

	// retourne le path relatif
	std::string PrendreRepertoireRelatif(const std::string& repertoire, const std::string& nom_fichier);

	float InterpolationLineaire(float x1, float y1, float x2, float y2, float x);

	// cree un repertoire (et ses sous repertoires)
	void CreeRepertoire(const std::string& repertoire);

	// retourne la position du carreau aval selon l'orientation
	void CarreauAval(int ligne, int colonne, int orientation, int& ligne_aval, int& colonne_aval);
	void CarreauAval2(size_t ligne, size_t colonne, int orientation, size_t& ligne_aval, size_t& colonne_aval);

	// copie tous les fichiers de src vers dst
	void CopieRepertoire(const std::string& src, const std::string& dst);

	bool DeleteFolderContent(std::string sDirectory);

	bool CopieRepertoireRecursive(boost::filesystem::path const & source, boost::filesystem::path const & destination, std::vector<std::string>* pvExcludedFolderName = NULL);

	// copie un fichier
	void Copie(const std::string& src, const std::string& dst);

	// remplace le repertoire
	std::string RemplaceRepertoire(const std::string& nom_fichier, const std::string& repertoire);

	//
	std::string ValidateInputFilesCharacters(std::vector<std::string> &listInputFiles, std::vector<std::string> &listErrMessCharValidation);

	//
	std::istream& getline_mod(std::istream& stream, std::string& line);

	//
	void lire_cle_valeur(std::istream& stream, std::string& cle, std::string& valeur);
	bool lire_cle_valeur_try(std::istream& stream, std::string& cle, std::string& valeur);	//return true/false instead of raising exception

	void lire_cle_valeur(std::string& ligne, std::string& cle, std::string& valeur);

	//
	std::vector<size_t>			extrait_valeur(const std::string& csv);

	//
	std::vector<size_t>			extrait_svaleur(const std::string& csv, const std::string& separator);
	std::vector<float>			extrait_fvaleur(const std::string& csv, const std::string& separator);
	std::vector<double>			extrait_dvaleur(const std::string& csv, const std::string& separator);
	
	std::vector<std::string>	extrait_stringValeur(const std::string& csv);
	std::vector<std::string>	extrait_stringValeur(const std::string& csv, const std::string& separator);

	//
	void	SplitString(std::vector<std::string>& sList, const std::string& input, const std::string& separators, bool remove_empty, bool bReplaceVirgule = true);

	//fait un trim sur les strings lues
	void	SplitString2(std::vector<std::string>& sList, const std::string& input, const std::string& separators, bool remove_empty);

	std::string TrimString(std::string str);

	double string_to_double(const std::string& s);

	int string_to_int(const std::string& s);

	unsigned short string_to_ushort(const std::string& s);

	bool AlmostEqual(double a, double b, double epsilon);

	// determine si un nom de fichier contient une racine
	bool Racine(const std::string& nom_fichier);
	
	std::string Combine(const std::string& racine, const std::string& chemin);

	std::string GetTempDirectory();

	std::string GetTempFilename();

}

#endif
