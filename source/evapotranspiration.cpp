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

#include "evapotranspiration.hpp"

#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"

#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	EVAPOTRANSPIRATION::EVAPOTRANSPIRATION(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_netCdf_etp = NULL;
	}

	EVAPOTRANSPIRATION::~EVAPOTRANSPIRATION()
	{
	}

	void EVAPOTRANSPIRATION::ChangeNbParams(const ZONES& zones)
	{
		_coefficients_multiplicatif.resize(zones.PrendreNbZone(), 1.0f);
	}

	void EVAPOTRANSPIRATION::ChangeCoefficientMultiplicatif(size_t index_zone, float coefficient)
	{
		BOOST_ASSERT(index_zone < _coefficients_multiplicatif.size());
		_coefficients_multiplicatif[index_zone] = coefficient;
	}

	float EVAPOTRANSPIRATION::PrendreCoefficientMultiplicatif(size_t index_zone) const
	{
		BOOST_ASSERT(index_zone < _coefficients_multiplicatif.size());
		return _coefficients_multiplicatif[index_zone];
	}

	float EVAPOTRANSPIRATION::Repartition(unsigned short pas_de_temps, unsigned short heure)
	{
		const float repartition[] = { 0.5f,0.5f,0.5f,0.5f,0.5f,1.0f,2.2f,4.0f,
			                          5.4f,8.0f,8.4f,9.6f,10.4f,10.8f,10.8f,9.6f,
									  7.8f,5.0f,2.0f,0.5f,0.5f,0.5f,0.5f,0.5f};

		if (pas_de_temps == 24)
			return 1.0f;
		else
		{
			int h = heure;
			float poids = 0.0f;

			do
			{
				poids += repartition[h];
				++h;
			} 
			while((h < 24) && (h < heure + pas_de_temps));

			poids /= 100.0f;
			return poids;
		}
	}


	void EVAPOTRANSPIRATION::Initialise()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		const size_t nb_zone = zones.PrendreNbZone();
		const size_t nb_classe = _sim_hyd.PrendreOccupationSol().PrendreNbClasse();

		for (size_t index = 0; index < nb_zone; ++index)
			zones[index].ChangeNbEtp(nb_classe);

		if (_sim_hyd.PrendreOutput().SauvegardeEtp())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_etp = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "etp.csv") );

				_fichier_etp.open(nom_fichier);

				_fichier_etp << "Évapotranspiration potentielle total (mm)" << _sim_hyd.PrendreOutput().Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();

				string str;
				ostringstream oss;
				oss.str("");

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etp << str << endl;
			}
		}
	}


	void EVAPOTRANSPIRATION::Calcule()
	{
		string str;

		if (_sim_hyd.PrendreOutput().SauvegardeEtp())
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_netCdf_etp != NULL)
			{
				size_t i, idx;

				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_etp[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreEtpTotal();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << _sim_hyd.PrendreOutput().Separator() << setprecision(_sim_hyd.PrendreOutput()._nbDigit_mm) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << zones[index].PrendreEtpTotal() << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_etp << str << endl;
			}
		}
	}


	void EVAPOTRANSPIRATION::Termine()
	{
		string str1, str2;

		if (_sim_hyd.PrendreOutput().SauvegardeEtp())
		{
			if (_netCdf_etp != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "evapotranspiration-etp.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "etp.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "evapotranspiration-etp", _netCdf_etp, "mm", "Evapotranspiration potentielle");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "etp", _netCdf_etp, "mm", "Evapotranspiration potentielle");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_etp;
			}
			else
				_fichier_etp.close();
		}
	}

}
