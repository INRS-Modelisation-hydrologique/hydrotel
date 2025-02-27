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

#include "propriete_hydroliques.hpp"

#include "erreur.hpp"
#include "util.hpp"
#include "sim_hyd.hpp"

#include <algorithm>
#include <fstream>


using namespace std;


namespace HYDROTEL
{

	PROPRIETE_HYDROLIQUES::PROPRIETE_HYDROLIQUES()
	{
		_bDisponible = false;
	}

	PROPRIETE_HYDROLIQUES::~PROPRIETE_HYDROLIQUES()
	{
	}


	void PROPRIETE_HYDROLIQUES::ChangeNomFichier(const string& nom_fichier)
	{
		_nom_fichier = nom_fichier;
	}

	void PROPRIETE_HYDROLIQUES::ChangeNomFichierCouche1(const string& nom_fichier)
	{
		_nom_fichier_couche1 = nom_fichier;
	}

	void PROPRIETE_HYDROLIQUES::ChangeNomFichierCouche2(const string& nom_fichier)
	{
		_nom_fichier_couche2 = nom_fichier;
	}

	void PROPRIETE_HYDROLIQUES::ChangeNomFichierCouche3(const string& nom_fichier)
	{
		_nom_fichier_couche3 = nom_fichier;
	}


	string PROPRIETE_HYDROLIQUES::PrendreNomFichier() const
	{
		return _nom_fichier;
	}

	string PROPRIETE_HYDROLIQUES::PrendreNomFichierCouche1() const
	{
		return _nom_fichier_couche1;
	}

	string PROPRIETE_HYDROLIQUES::PrendreNomFichierCouche2() const
	{
		return _nom_fichier_couche2;
	}

	string PROPRIETE_HYDROLIQUES::PrendreNomFichierCouche3() const
	{
		return _nom_fichier_couche3;
	}


	void PROPRIETE_HYDROLIQUES::Lecture(SIM_HYD& sim_hyd)
	{
		if(_nom_fichier_couche1 == "" || !LectureProprieteHydrolique())
			_bDisponible = false;
		else
		{
			LectureCouches(sim_hyd);
			_bDisponible = true;
		}
	}


	bool PROPRIETE_HYDROLIQUES::LectureProprieteHydrolique()
	{
		ifstream fichier(_nom_fichier);
		if (!fichier)
			return false;

		int type;
		size_t nb_propriete, nb_variable;

		fichier >> type >> nb_propriete >> nb_variable;
		
		fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		vector<PROPRIETE_HYDROLIQUE> propriete_hydroliques(nb_propriete);

		string tmp;

		getline_mod(fichier, tmp);
		getline_mod(fichier, tmp);

		for (size_t index = 0; index < nb_propriete; ++index)
		{
			string nom;
			float thetas , thetacc , thetapf , ks , psis , lambda , alpha;

			fichier >> nom >> thetas >> thetacc >> thetapf >> ks >> psis >> lambda >> alpha;

			if(thetas <= 0.0f || thetacc <= 0.0f || thetapf <= 0.0f || ks <= 0.0f || psis <= 0.0f || lambda <= 0.0f || alpha <= 0.0f)
			{
				fichier.close();
				std::ostringstream ss;
				ss << "erreur lecture proprietes hydrauliques; valeur(s) pour " << nom << " invalide; les valeurs doivent etre superieur a 0.";
				throw ERREUR(ss.str());
			}

			propriete_hydroliques[index] = PROPRIETE_HYDROLIQUE(nom, thetas, thetapf, thetacc, ks, lambda, psis, alpha);
		}

		fichier.close();
		_propriete_hydroliques.swap(propriete_hydroliques);
		return true;
	}


	void PROPRIETE_HYDROLIQUES::LectureCouche(const string& nom_fichier, vector<int>& couche, SIM_HYD& sim_hyd)
	{
		int index_max = static_cast<int>(_propriete_hydroliques.size() - 1);
		vector<int> coucheTemp(sim_hyd.PrendreZones().PrendreNbZone());

		int x;

		ifstream fichier(nom_fichier);
		if (fichier)
		{
			int type;
			fichier >> type;

			int ident, index_propriete;

			try{

			for (size_t n = 0; n < sim_hyd.PrendreZones().PrendreNbZone(); ++n)
			{
				fichier >> ident >> index_propriete;

				x = sim_hyd.RechercheZoneIndexGroupe(ident);
				if(_coefficient_additif.size() != 0)
				{
					index_propriete+= _coefficient_additif[x];
					index_propriete = max(0, min(index_propriete, index_max));
				}

				size_t index_zone = sim_hyd.PrendreZones().IdentVersIndex(ident);
				coucheTemp[index_zone] = index_propriete;
			}

			}
			catch(...)
			{
				fichier.close();
				throw ERREUR("erreur; propriete_hydroliques::LectureCouche");
			}

			fichier.close();
			couche.swap(coucheTemp);
		}
		else
		{
			//lecture de la matrice des types de sol
			map<size_t,vector<int>> mapNbCell;
			string nom_fichier_grille;
			size_t i;
			int iNb, ident, iTypeSol, iVal;

			nom_fichier_grille = RemplaceExtension(nom_fichier, "tif");
			RASTER<int>	grilleTypeSol = LectureRaster_int(nom_fichier_grille);

			const RASTER<int>&	grilleZones = sim_hyd.PrendreZones().PrendreGrille();

			iNb = static_cast<int>(_propriete_hydroliques.size());
			for (i = 0; i < sim_hyd.PrendreZones().PrendreNbZone(); i++)
			{
				for (x=0; x<iNb; x++)
					mapNbCell[i].push_back(0);
			}

			//parcours la matrice des types de sol
			for (size_t lig = 0; lig < grilleTypeSol.PrendreNbLigne(); lig++)
			{
				for (size_t col = 0; col < grilleTypeSol.PrendreNbColonne(); col++)
				{
					ident = grilleZones(lig, col);
					if(ident != 0 && ident != grilleZones.PrendreNoData())
					{
						iTypeSol = grilleTypeSol(lig, col);
						if(iTypeSol != grilleTypeSol.PrendreNoData())
						{
							i = sim_hyd.PrendreZones().IdentVersIndex(ident);

							try{
							iVal = mapNbCell[i].at(iTypeSol-1);
							}
							catch(...)
							{
								throw ERREUR("erreur propriete_hydroliques; id matrice typesol introuvable dans proprietehydrolique.sol");
							}

							++iVal;
							mapNbCell[i].at(iTypeSol-1) = iVal;
						}
					}
				}
			}

			//creation du fichier typesol.cla
			ofstream fichier2(nom_fichier);

			fichier2 << "1\n";	//format
			int iIDMax, iID;
			size_t iIndex;

			iIDMax = sim_hyd.PrendreZones()[sim_hyd.PrendreZones().PrendreNbZone()-1].PrendreIdent();
			iID = abs(sim_hyd.PrendreZones()[0].PrendreIdent());
			if(iID > iIDMax)
				iIDMax = iID;

			for (iID = 1; iID <= iIDMax; iID++)
			{
				ident = iID;
				try{
					iIndex = sim_hyd.PrendreZones().IdentVersIndex(ident);}
				catch(ERREUR&)
				{
					ident = -ident;
					try{
						iIndex = sim_hyd.PrendreZones().IdentVersIndex(ident);}					
					catch(ERREUR&)
					{
						fichier2.close();
						std::ostringstream ss;
						ss << "erreur; propriete_hydroliques::LectureCouche; identifiant uhrh " <<  abs(ident) << " invalide";
						throw ERREUR(ss.str());
					}
				}

				int nTexture = 0;
				int nTextureMax = mapNbCell[iIndex].at(0);

				for (int typeIndex = 1; typeIndex < iNb; typeIndex++)
				{
					if (mapNbCell[iIndex].at(typeIndex) > nTextureMax)
					{
						nTexture = typeIndex;
						nTextureMax = mapNbCell[iIndex].at(typeIndex);
					}
				}

				fichier2 << ident << " " << nTexture << "\n";	//type de sol dominant pour l'uhrh en cours

				//conserve l'information
				if(_coefficient_additif.size() != 0)
				{
					nTexture+= _coefficient_additif[sim_hyd.RechercheZoneIndexGroupe(ident)];
					nTexture = max(0, min(nTexture, index_max));
				}
				coucheTemp[iIndex] = nTexture;
			}

			fichier2.close();
			couche.swap(coucheTemp);
		}
	}


	void PROPRIETE_HYDROLIQUES::LectureCouches(SIM_HYD& sim_hyd)
	{
		LectureCouche(_nom_fichier_couche1, _couche1, sim_hyd);
		LectureCouche(_nom_fichier_couche2, _couche2, sim_hyd);
		LectureCouche(_nom_fichier_couche3, _couche3, sim_hyd);
	}


	size_t PROPRIETE_HYDROLIQUES::PrendreNb() const
	{
		return _propriete_hydroliques.size();
	}

	PROPRIETE_HYDROLIQUE& PROPRIETE_HYDROLIQUES::Prendre(size_t index)
	{
		BOOST_ASSERT(index < _propriete_hydroliques.size());
		return _propriete_hydroliques[index];
	}

	size_t PROPRIETE_HYDROLIQUES::PrendreIndexCouche1(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _couche1.size());
		return _couche1[index_zone];
	}

	size_t PROPRIETE_HYDROLIQUES::PrendreIndexCouche2(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _couche2.size());
		return _couche2[index_zone];
	}

	size_t PROPRIETE_HYDROLIQUES::PrendreIndexCouche3(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _couche3.size());
		return _couche3[index_zone];
	}

	PROPRIETE_HYDROLIQUE& PROPRIETE_HYDROLIQUES::PrendreProprieteHydroliqueCouche1(size_t index_zone)
	{
		BOOST_ASSERT(index_zone < _couche1.size());
		return _propriete_hydroliques[_couche1[index_zone]];
	}

	PROPRIETE_HYDROLIQUE& PROPRIETE_HYDROLIQUES::PrendreProprieteHydroliqueCouche2(size_t index_zone)
	{
		BOOST_ASSERT(index_zone < _couche2.size());
		return _propriete_hydroliques[_couche2[index_zone]];
	}

	PROPRIETE_HYDROLIQUE& PROPRIETE_HYDROLIQUES::PrendreProprieteHydroliqueCouche3(size_t index_zone)
	{
		BOOST_ASSERT(index_zone < _couche3.size());
		return _propriete_hydroliques[_couche3[index_zone]];
	}

}
