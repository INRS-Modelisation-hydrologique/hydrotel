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

#ifndef PRELEVEMENTS_H_INCLUDED
#define PRELEVEMENTS_H_INCLUDED


#include "sim_hyd.hpp"

#include <fstream>


namespace HYDROTEL
{

	class PRELEVEMENTS
	{
	public:

		SIM_HYD&																				_sim_hyd;

		std::string																				_FolderName;
		std::string																				_FolderNameSrc;

		bool																					_bSimulePrelevements;

		std::string																				_GpeHeader;
		std::map<std::string, size_t>															_GpeMapCol;
		std::string																				_PrHeader;
		std::map<std::string, size_t>															_PrMapCol;
		std::string																				_ElHeader;
		std::map<std::string, size_t>															_ElMapCol;
		std::string																				_EfHeader;
		std::map<std::string, size_t>															_EfMapCol;
		std::string																				_CuHeader;
		std::map<std::string, size_t>															_CuMapCol;

		std::map<std::string, int>																_donneesTYPE;			//key: TYPE, val: 0=exclure de la simulation, 1=inclure dans la simulation
		std::map<std::string, int>																_donneesTYPE_redCoeff;	//key: TYPE, val: 1 pour appliquer le coefficient de réduction au type, sinon 0

		std::map<std::string, double>															_donneesREDCOEFF;		//key: Date ('yyyymmdd'), val: coefficient de réduction (0:1) des prélèvements

		std::map<int, std::map<int, std::map<std::string, std::string>>>						_donneesGPE;			//key: idtroncon, #SITE GPE, ANNEE
		std::map<int, std::map<int, int>>														_donneesGPE_AnneeMax;	//key: idtroncon, #SITE GPE
		std::map<int, std::map<int, std::map<int, std::map<int, std::vector<int>>>>>			_donneesGPE_Jours;		//key: idtroncon, #SITE GPE, ANNEE, MOIS(1a12)
		std::map<int, std::string>																_donneesGPESite;		//key: #SITE

		std::map<int, std::map<int, std::string>>												_donneesPR;				//key: idtroncon, #SITE
		std::map<int, std::string>																_donneesPRSite;			//key: #SITE

		std::map<int, std::map<int, std::string>>												_donneesELEVAGE;		//key: idtroncon, #SITE
		std::map<int, std::string>																_donneesELEVAGESite;	//key: #SITE

		std::map<int, std::map<int, std::string>>												_donneesEFFLUENT;		//key: idtroncon, #SITE

		std::map<int, std::string>																_donneesCULTURESite;	//key: ID

		std::map<int, int>																		_donneesGPE_PR;			//key: idGpe, value: idPr		//sites Pr associés aux sites GPE
		std::map<int, int>																		_donneesELEVAGE_PR;		//key: idElevage, value: idPr	//sites Pr associés aux sites d'élevages

		std::map<int, std::vector<int>>															_donnees_PR_CULTURE;	//key: idPr, value: idCulture	//sites de cultures associés aux sites de prélèvements

		
		//pour validation
		std::map<size_t, std::vector<std::string>>												_tronconsPrelevementString;			//key: indexTroncon, value: string descriptif du prelevement
		std::map<size_t, std::vector<double>>													_tronconsPrelevementVal;			//key: indexTroncon, value: valeur du prelevement (m3/s)
		std::map<size_t, std::vector<std::string>>												_tronconsPrelevementCultureString;	//
		std::map<size_t, std::vector<double>>													_tronconsPrelevementCultureVal;		//
		std::map<size_t, std::vector<std::string>>												_tronconsRejetString;				//key: indexTroncon, value: string descriptif du prelevement
		std::map<size_t, std::vector<double>>													_tronconsRejetVal;					//key: indexTroncon, value: valeur du prelevement (m3/s)
		std::map<size_t, std::vector<std::string>>												_tronconsRejetEffluentString;		//key: indexTroncon, value: string descriptif du prelevement
		std::map<size_t, std::vector<double>>													_tronconsRejetEffluentVal;			//key: indexTroncon, value: valeur du prelevement (m3/s)

		std::ofstream																			_ofsPrelevementCalcule;
		std::ofstream																			_ofsPrelevementEffectue;


	public:

		PRELEVEMENTS(SIM_HYD& sim_hyd);
		~PRELEVEMENTS();

		bool							GenerateBdPrelevements();

		bool							AddTronconUhrhToFile(std::string sPathFileIn, std::string sPathFileOut, size_t indexCoord);

		bool							LecturePrelevements();

		bool							LecturePrelevementsTYPES();
		
		bool							LecturePrelevementsGPE();
		bool							LecturePrelevementsPR();
		bool							LecturePrelevementsELEVAGE();
		bool							LecturePrelevementsEFFLUENT();
		bool							LecturePrelevementsCULTURE();

		bool							LecturePrelevementsREDCOEFF();

		bool							CalculePrelevements();	//calcule les prelevements et rejets à effectuer pour le pas de temps courant (pour tous les troncons)

		double							GetGpeDbl(std::string sLine, std::string sColNameUpperCase);
		std::string						GetGpeStr(std::string sLine, std::string sColNameUpperCase);

		int								GetPrInt(std::string sLine, std::string sColNameUpperCase);
		double							GetPrDbl(std::string sLine, std::string sColNameUpperCase);
		std::string						GetPrStr(std::string sLine, std::string sColNameUpperCase);

		double							GetElDbl(std::string sLine, std::string sColNameUpperCase);
		std::string						GetElStr(std::string sLine, std::string sColNameUpperCase);

		double							GetEfDbl(std::string sLine, std::string sColNameUpperCase);
		std::string						GetEfStr(std::string sLine, std::string sColNameUpperCase);

		double							GetCuDbl(std::string sLine, std::string sColNameUpperCase);
		std::string						GetCuStr(std::string sLine, std::string sColNameUpperCase);

		void							GenerateAlea(size_t nbVal, int iMaxVal, unsigned int uiSeed, std::vector<int>& vAlea);



		//------------------------------------------------------------------------------------------------
		//MLB
		// Obtient l'id d'un troncon, l'id du type de sol et le nom du type de sol et l'uhrh avec coord Lat/Long
        bool FonctionObtientUhrhTroncon(std::vector<double> vLon, std::vector<double> vLat, std::vector<int>& vidUhrh, std::vector<int>& vidTroncon, std::vector<int>& vidOccsol, std::vector<std::string>& vsOccsol, std::vector<int>& vindex);
		
		// Permet d'importer des fichier GPE pour les trier et enlever les sites qui ne sont pas dans la carte
		//type 1
        void trierDonnePrelevementGPE(const std::vector<std::string>& filename, std::string sOutputFile);

		// Permet d'importer des fichier agricole pour les trier et enlever les sites qui ne sont pas dans la carte
		//type 2
		void trierDonnePrelevementagricole(const std::vector<std::string>& filename, const std::string & filenameTable, std::string sOutputFile);


	private:
		PRELEVEMENTS& operator= (const PRELEVEMENTS&); //pour eviter warning C4512 sous vc, a cause reference symbol in class data item (_sim_hyd)
	};

}

#endif
