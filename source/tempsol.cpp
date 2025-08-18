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

#include "tempsol.hpp"

#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"

#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	TEMPSOL::TEMPSOL(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_netCdf_profondeurgel = NULL;
		_nbZone = 0;
	}


	TEMPSOL::~TEMPSOL()
	{
	}


	void TEMPSOL::ChangeNbParams(const ZONES& zones)
	{
		_nbZone = zones.PrendreNbZone();
	}


	void TEMPSOL::Initialise()
	{
		OUTPUT& output = _sim_hyd.PrendreOutput();
		ZONES& zones = _sim_hyd.PrendreZones();

		string str;

		if (output.SauvegardeProfondeurGel())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_profondeurgel = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "profondeur_gel.csv") );
				_fichier_tempsol.open(nom_fichier);
				_fichier_tempsol << "profondeur du gel (cm)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				ostringstream oss;
				oss.str("");
				
				for (size_t index = 0; index < _nbZone; ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreIdent() << output.Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_tempsol << str << endl;
			}
		}
	}


	void TEMPSOL::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		OUTPUT& output = _sim_hyd.PrendreOutput();

		string str;
		size_t index;

		if (output.SauvegardeProfondeurGel())
		{
			if (_netCdf_profondeurgel != NULL)
			{
				size_t i, idx;

				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_profondeurgel[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreProfondeurGel();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_cm) << setiosflags(ios::fixed);

				for (index = 0; index < _nbZone; ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreProfondeurGel() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_tempsol << str << endl;
			}
		}
	}


	void TEMPSOL::Termine()
	{
		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeProfondeurGel())
		{
			if (_netCdf_profondeurgel != NULL)
			{
				string str1, str2;

				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "tempsol-profondeur_gel.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "profondeur_gel.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "tempsol-profondeur_gel", _netCdf_profondeurgel, "cm", "Profondeur du gel au sol");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "profondeur_gel", _netCdf_profondeurgel, "cm", "Profondeur du gel au sol");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_profondeurgel;
			}
			else
				_fichier_tempsol.close();
		}
	}

}
