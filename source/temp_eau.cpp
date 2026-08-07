#include "temp_eau.hpp"

#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>


using namespace std;


namespace HYDROTEL
{

	TEMP_EAU::TEMP_EAU(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_netCdf_tempeau = NULL;
		_nbTroncons = 0;
	}


	TEMP_EAU::~TEMP_EAU()
	{
	}


	void TEMP_EAU::ChangeNbParams(const ZONES& zones)
	{
		zones; //warning unreferenced formal parameter

		_nbTroncons = _sim_hyd.PrendreTroncons().PrendreNbTroncon();
	}


	void TEMP_EAU::Initialise()
	{
		OUTPUT& output = _sim_hyd.PrendreOutput();
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		string str;

		if(output.SauvegardeTemperatureEau())
		{
			if(_sim_hyd._outputCDF)
				_netCdf_tempeau = new float[_sim_hyd._lNbPasTempsSim * output._tronconOutputNb];
			else
			{
				string nom_fichier(Combine(_sim_hyd.PrendreRepertoireResultat(), "temp_eau.csv"));

				_fichier_temp_eau.open(nom_fichier);
				_fichier_temp_eau << "Temperature eau (C)" << output.Separator()
					<< PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )"
					<< endl << "date heure\\troncon" << output.Separator();  

				ostringstream oss; 
				oss.str("");

				size_t index;
				for ( index = 0; index < _nbTroncons; ++index)
				{
					if (find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), troncons[index]->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << output.Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length() - 1); 
				_fichier_temp_eau << str << endl;
			}
		}
	}


	void TEMP_EAU::Calcule()
	{
		TRONCONS& troncons = _sim_hyd.PrendreTroncons();
		OUTPUT& output = _sim_hyd.PrendreOutput();

		string str;
		size_t index, tron_idx;

		if(output.SauvegardeTemperatureEau())
		{
			if (_netCdf_tempeau != NULL)
			{
				size_t i, idx;
				idx = _sim_hyd._lPasTempsCourantIndex * output._tronconOutputNb;

				for (i = 0; i < output._tronconOutputNb; i++)
				{
				    tron_idx = output._tronconOutputIndex[i];
					_netCdf_tempeau[idx + i] = troncons[tron_idx]->_tempEau;
				}
			}
			else
			{
				ostringstream oss;
				oss.str("");

				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_cm) << setiosflags(ios::fixed);

				for (index = 0; index < _nbTroncons; ++index)
				{
					if (find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if (output._bSauvegardeTous ||
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), troncons[index]->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << troncons[index]->_tempEau << output.Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length() - 1);
				_fichier_temp_eau << str << endl;
			}
		}
	}


	void TEMP_EAU::Termine()
	{
		OUTPUT& output = _sim_hyd.PrendreOutput();

		if(output.SauvegardeTemperatureEau())
		{
			if(_netCdf_tempeau != NULL)
			{
				string str1, str2;

				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "temp_eau.nc");
				str2 = output.SauvegardeOutputNetCDF(str1, false, "temp_eau", _netCdf_tempeau, "C", "Temperature eau");
				if(str2 != "")
					throw ERREUR(str2);

				delete[] _netCdf_tempeau;
			}
			else
				_fichier_temp_eau.close();
		}
	}


}
