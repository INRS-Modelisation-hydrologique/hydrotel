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

#include "bilan_vertical.hpp"

#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"


using namespace std;


namespace HYDROTEL
{

	BILAN_VERTICAL::BILAN_VERTICAL(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_netCdf_prodSurf = NULL;
		_netCdf_prodHypo = NULL;
		_netCdf_prodBase = NULL;
	}

	BILAN_VERTICAL::~BILAN_VERTICAL()
	{
	}


	void BILAN_VERTICAL::Initialise()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		//string nom_fichier(_sim_hyd.PrendreRepertoireResultat() + "production_total.csv");
		//_fichier_production.open(nom_fichier.c_str());
		//_fichier_production << "production total (m)" << PrendreNomSousModele() << endl << "date heure\\uhrh;";
		//for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		//	_fichier_production << zones[index].PrendreIdent() << ';';
		//_fichier_production << endl;

		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeProductionSurf())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_prodSurf = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_surf( Combine(_sim_hyd.PrendreRepertoireResultat(), "production_surf.csv") );
				_fichier_production_surf.open(nom_fichier_surf.c_str());
				_fichier_production_surf << "Lame d'eau produite à la couche 1 (surface) (production) (mm)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
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
				_fichier_production_surf << str << endl;
			}
		}

		if (output.SauvegardeProductionHypo())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_prodHypo = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_hypo( Combine(_sim_hyd.PrendreRepertoireResultat(), "production_hypo.csv") );
				_fichier_production_hypo.open(nom_fichier_hypo.c_str());
				_fichier_production_hypo << "Lame d'eau produite à la couche 2 (hypodermique) (production) (mm)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
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
				_fichier_production_hypo << str << endl;
			}
		}

		if (output.SauvegardeProductionBase())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_prodBase = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_base( Combine(_sim_hyd.PrendreRepertoireResultat(), "production_base.csv") );
				_fichier_production_base.open(nom_fichier_base.c_str());
				_fichier_production_base << "Lame d'eau produite à la couche 3 (base) (production) (mm)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

				string str;
				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
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
				_fichier_production_base << str << endl;
			}
		}
	}


	void BILAN_VERTICAL::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		//_fichier_production << _sim_hyd.PrendreDateCourante() << ';';
		//for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
		//	_fichier_production << zones[index].PrendreProductionTotal() << ';';
		//_fichier_production << endl;

		OUTPUT& output = _sim_hyd.PrendreOutput();

		size_t i, idx;
		string str;

		if (output.SauvegardeProductionSurf())
		{
			if (_netCdf_prodSurf != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_prodSurf[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreProdSurf();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_mm) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreProdSurf() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_production_surf << str << endl;
			}
		}

		if (output.SauvegardeProductionHypo())
		{
			if (_netCdf_prodHypo != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_prodHypo[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreProdHypo();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_mm) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreProdHypo() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_production_hypo << str << endl;
			}
		}

		if (output.SauvegardeProductionBase())
		{
			if (_netCdf_prodBase != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_prodBase[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreProdBase();
			}
			else
			{
				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_mm) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
							find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							oss << zones[index].PrendreProdBase() << output.Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_production_base << str << endl;
			}
		}
	}


	void BILAN_VERTICAL::Termine()
	{
		string str1, str2;

		//_fichier_production.close();

		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeProductionSurf())
		{
			if (_netCdf_prodSurf != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-production_surf.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "production_surf.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-production_surf", _netCdf_prodSurf, "mm", "Lame d`eau produite a la surface (production)");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "production_surf", _netCdf_prodSurf, "mm", "Lame d`eau produite a la surface (production)");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_prodSurf;
			}
			else
				_fichier_production_surf.close();
		}
	
		if (output.SauvegardeProductionHypo())
		{
			if (_netCdf_prodHypo != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-production_hypo.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "production_hypo.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-production_hypo", _netCdf_prodHypo, "mm", "Lame d`eau produite par la 2e couche (production)");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "production_hypo", _netCdf_prodHypo, "mm", "Lame d`eau produite par la 2e couche (production)");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_prodHypo;
			}
			else
				_fichier_production_hypo.close();
		}

		if (output.SauvegardeProductionBase())
		{
			if (_netCdf_prodBase != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "bilanvertical-production_base.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "production_base.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "bilanvertical-production_base", _netCdf_prodBase, "mm", "Lame d`eau produite par la 3e couche (production)");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "production_base", _netCdf_prodBase, "mm", "Lame d`eau produite par la 3e couche (production)");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_prodBase;
			}
			else
				_fichier_production_base.close();
		}
	}

}
