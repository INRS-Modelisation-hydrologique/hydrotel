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

#include "ruisselement_surface.hpp"

#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"


using namespace std;


namespace HYDROTEL
{

	RUISSELEMENT_SURFACE::RUISSELEMENT_SURFACE(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_netCdf_apport_lateral = NULL;
		_netCdf_apport_lateral_uhrh = NULL;

		_netCdf_ecoulement_surf = NULL;
		_netCdf_ecoulement_hypo = NULL;
		_netCdf_ecoulement_base = NULL;
	}


	RUISSELEMENT_SURFACE::~RUISSELEMENT_SURFACE()
	{
	}


	void RUISSELEMENT_SURFACE::Initialise()
	{
		string str;

		if (_sim_hyd.PrendreOutput().SauvegardeApportLateral())
		{
			if (_sim_hyd._outputCDF)
				_netCdf_apport_lateral = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._tronconOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "apport_lateral.csv") );
				_fichier_apport_lateral.open(nom_fichier.c_str());
				_fichier_apport_lateral << "Apport lateral (m3/s)" << _sim_hyd.PrendreOutput().Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\troncon" << _sim_hyd.PrendreOutput().Separator();

				TRONCONS& troncons = _sim_hyd.PrendreTroncons();

				ostringstream oss;
				oss.str("");
			
				for (size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_apport_lateral << str << endl;
			}
		}

		if(_sim_hyd.PrendreOutput()._ecoulement_surf)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_sim_hyd._outputCDF)
				_netCdf_ecoulement_surf = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_surf( Combine(_sim_hyd.PrendreRepertoireResultat(), "ecoulement_surf.csv") );
				_fichier_ecoulement_surf.open(nom_fichier_surf.c_str());
				_fichier_ecoulement_surf << "Écoulement vers le réseau hydrographique (couche 1) (surface) (m3/s)" << _sim_hyd.PrendreOutput().Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();

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
				_fichier_ecoulement_surf << str << endl;
			}
		}

		if(_sim_hyd.PrendreOutput()._ecoulement_hypo)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_sim_hyd._outputCDF)
				_netCdf_ecoulement_hypo = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_hypo( Combine(_sim_hyd.PrendreRepertoireResultat(), "ecoulement_hypo.csv") );
				_fichier_ecoulement_hypo.open(nom_fichier_hypo.c_str());
				_fichier_ecoulement_hypo << "Écoulement vers le réseau hydrographique (couche 2) (hypodermique) (m3/s)" << _sim_hyd.PrendreOutput().Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();

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
				_fichier_ecoulement_hypo << str << endl;
			}
		}

		if(_sim_hyd.PrendreOutput()._ecoulement_base)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_sim_hyd._outputCDF)
				_netCdf_ecoulement_base = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				string nom_fichier_base( Combine(_sim_hyd.PrendreRepertoireResultat(), "ecoulement_base.csv") );
				_fichier_ecoulement_base.open(nom_fichier_base.c_str());
				_fichier_ecoulement_base << "Écoulement vers le réseau hydrographique (couche 3) (base) (m3/s)" << _sim_hyd.PrendreOutput().Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();

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
				_fichier_ecoulement_base << str << endl;
			}
		}

		if(_sim_hyd.PrendreOutput()._apport_lateral_uhrh)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_sim_hyd._outputCDF)
				_netCdf_apport_lateral_uhrh = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
			else
			{
				str = Combine(_sim_hyd.PrendreRepertoireResultat(), "apport_lateral_uhrh.csv");
				_fichier_apport_lateral_uhrh.open(str.c_str());
				_fichier_apport_lateral_uhrh << "Apport lateral au troncon (m3/s)" << _sim_hyd.PrendreOutput().Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();

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
				_fichier_apport_lateral_uhrh << str << endl;
			}
		}
	}


	void RUISSELEMENT_SURFACE::Calcule()
	{
		size_t i, idx;
		string str;

		if (_sim_hyd.PrendreOutput().SauvegardeApportLateral())
		{
			if(_netCdf_apport_lateral != NULL)
			{
				TRONCONS& troncons = _sim_hyd.PrendreTroncons();

				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._tronconOutputNb;

				for(i=0; i<_sim_hyd.PrendreOutput()._tronconOutputNb; i++)
					_netCdf_apport_lateral[idx+i] = troncons[_sim_hyd.PrendreOutput()._tronconOutputIndex[i]]->PrendreApportLateral();
			}
			else
			{
				TRONCONS& troncons = _sim_hyd.PrendreTroncons();

				ostringstream oss;
				oss.str("");
			
				oss << _sim_hyd.PrendreDateCourante() << _sim_hyd.PrendreOutput().Separator() << setprecision(_sim_hyd.PrendreOutput()._nbDigit_m3s) << setiosflags(ios::fixed);

				for (size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << troncons[index]->PrendreApportLateral() << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_apport_lateral << str << endl;
			}
		}

		if (_sim_hyd.PrendreOutput()._ecoulement_surf)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_netCdf_ecoulement_surf != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_ecoulement_surf[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]]._ecoulementSurf;
			}
			else
			{
				ostringstream oss;
				oss.str("");

				oss << _sim_hyd.PrendreDateCourante() << _sim_hyd.PrendreOutput().Separator() << setprecision(_sim_hyd.PrendreOutput()._nbDigit_m3s) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << zones[index]._ecoulementSurf << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_ecoulement_surf << str << endl;
			}
		}

		if (_sim_hyd.PrendreOutput()._ecoulement_hypo)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_netCdf_ecoulement_hypo != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_ecoulement_hypo[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]]._ecoulementHypo;
			}
			else
			{
				ostringstream oss;
				oss.str("");

				oss << _sim_hyd.PrendreDateCourante() << _sim_hyd.PrendreOutput().Separator() << setprecision(_sim_hyd.PrendreOutput()._nbDigit_m3s) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << zones[index]._ecoulementHypo << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_ecoulement_hypo << str << endl;
			}
		}

		if (_sim_hyd.PrendreOutput()._ecoulement_base)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_netCdf_ecoulement_base != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_ecoulement_base[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]]._ecoulementBase;
			}
			else
			{
				ostringstream oss;
				oss.str("");

				oss << _sim_hyd.PrendreDateCourante() << _sim_hyd.PrendreOutput().Separator() << setprecision(_sim_hyd.PrendreOutput()._nbDigit_m3s) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << zones[index]._ecoulementBase << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_ecoulement_base << str << endl;
			}
		}

		if (_sim_hyd.PrendreOutput()._apport_lateral_uhrh)
		{
			ZONES& zones = _sim_hyd.PrendreZones();

			if (_netCdf_apport_lateral_uhrh != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
					_netCdf_apport_lateral_uhrh[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]]._apport_lateral_uhrh;
			}
			else
			{
				ostringstream oss;
				oss.str("");

				oss << _sim_hyd.PrendreDateCourante() << _sim_hyd.PrendreOutput().Separator() << setprecision(_sim_hyd.PrendreOutput()._nbDigit_m3s) << setiosflags(ios::fixed);

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (_sim_hyd.PrendreOutput()._bSauvegardeTous || 
							find(begin(_sim_hyd.PrendreOutput()._vIdTronconSelect), end(_sim_hyd.PrendreOutput()._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(_sim_hyd.PrendreOutput()._vIdTronconSelect))
						{
							oss << zones[index]._apport_lateral_uhrh << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_apport_lateral_uhrh << str << endl;
			}
		}
	}


	void RUISSELEMENT_SURFACE::Termine()
	{
		if (_sim_hyd.PrendreOutput().SauvegardeApportLateral())
		{
			if (_netCdf_apport_lateral != NULL)
			{
				string str1, str2;

				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "ruissellement-apport_lateral.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "apport_lateral.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, false, "ruissellement-apport_lateral", _netCdf_apport_lateral, "m3/s", "Apport lateral");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, false, "apport_lateral", _netCdf_apport_lateral, "m3/s", "Apport lateral");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_apport_lateral;
			}
			else
				_fichier_apport_lateral.close();
		}

		if (_sim_hyd.PrendreOutput()._ecoulement_surf)
		{
			if (_netCdf_ecoulement_surf != NULL)
			{
				string str1, str2;

				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "ruissellement-apport_lateral.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "ecoulement_surf.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, false, "ruissellement-apport_lateral", _netCdf_apport_lateral, "m3/s", "Apport lateral");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "ecoulement_surf", _netCdf_ecoulement_surf, "m3/s", "Ecoulement vers le reseau hydrographique (couche 1) (surface)");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_ecoulement_surf;
			}
			else
				_fichier_ecoulement_surf.close();
		}

		if (_sim_hyd.PrendreOutput()._ecoulement_hypo)
		{
			if (_netCdf_ecoulement_hypo != NULL)
			{
				string str1, str2;

				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "ruissellement-apport_lateral.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "ecoulement_hypo.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, false, "ruissellement-apport_lateral", _netCdf_apport_lateral, "m3/s", "Apport lateral");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "ecoulement_hypo", _netCdf_ecoulement_hypo, "m3/s", "Ecoulement vers le reseau hydrographique (couche 2) (hypodermique)");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_ecoulement_hypo;
			}
			else
				_fichier_ecoulement_hypo.close();
		}

		if (_sim_hyd.PrendreOutput()._ecoulement_base)
		{
			if (_netCdf_ecoulement_base != NULL)
			{
				string str1, str2;

				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "ruissellement-apport_lateral.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "ecoulement_base.nc");
				//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, false, "ruissellement-apport_lateral", _netCdf_apport_lateral, "m3/s", "Apport lateral");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "ecoulement_base", _netCdf_ecoulement_base, "m3/s", "Ecoulement vers le reseau hydrographique (couche 3) (base)");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_ecoulement_base;
			}
			else
				_fichier_ecoulement_base.close();
		}

		if (_sim_hyd.PrendreOutput()._apport_lateral_uhrh)
		{
			if (_netCdf_apport_lateral_uhrh != NULL)
			{
				string str1, str2;

				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "apport_lateral_uhrh.nc");
				str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "apport_lateral_uhrh", _netCdf_apport_lateral_uhrh, "m3/s", "Apport lateral au troncon");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_apport_lateral_uhrh;
			}
			else
				_fichier_apport_lateral_uhrh.close();
		}
	}

}
