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

#include "fonte_neige.hpp"

#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"


using namespace std;


namespace HYDROTEL
{

	FONTE_NEIGE::FONTE_NEIGE(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_netCdf_apport = NULL;
	}


	FONTE_NEIGE::~FONTE_NEIGE()
	{
	}


	void FONTE_NEIGE::Initialise()
	{
		if (_sim_hyd.PrendreOutput().SauvegardeApport())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_apport = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "apport.csv") );
				_fichier_apport.open(nom_fichier);

				ZONES& zones = _sim_hyd.PrendreZones();

				_fichier_apport << "Apport fonte neige et pluie (mm)" << _sim_hyd.PrendreOutput().Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();

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
				_fichier_apport << str << endl;
			}
		}
	}


	void FONTE_NEIGE::Calcule()
	{
		string str;

		if (_sim_hyd.PrendreOutput().SauvegardeApport())
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_netCdf_apport != NULL)
			{
				size_t i, idx;

				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_apport[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreApport();
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
							oss << zones[index].PrendreApport() << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_apport << str << endl;
			}
		}
	}


	void FONTE_NEIGE::Termine()
	{
		if (_sim_hyd.PrendreOutput().SauvegardeApport())
		{
			if (_netCdf_apport != NULL)
			{
				string str1, str2;

				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "fonte_neige-apport.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "apport.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "fonte_neige-apport", _netCdf_apport, "mm", "Apport de la fonte de neige et de la pluie");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "apport", _netCdf_apport, "mm", "Apport de la fonte de neige et de la pluie");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_apport;
			}
			else
				_fichier_apport.close();
		}
	}

}
