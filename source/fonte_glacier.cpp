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


#include "fonte_glacier.hpp"

#include "util.hpp"
#include "version.hpp"

#include <algorithm>


using namespace std;


namespace HYDROTEL
{

	FONTE_GLACIER::FONTE_GLACIER(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_nbZone = 0;
	}


	FONTE_GLACIER::~FONTE_GLACIER()
	{
	}


	void FONTE_GLACIER::ChangeNbParams(const ZONES& zones)
	{
		_nbZone = zones.PrendreNbZone();
	}


	void FONTE_GLACIER::Initialise()
	{
		OUTPUT& output = _sim_hyd.PrendreOutput();
		ZONES& zones = _sim_hyd.PrendreZones();

		string str;

		if (output.SauvegardeApportGlacier())
		{
			string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "glacier-apport.csv") );
			_fichier_out1.open(nom_fichier);
			_fichier_out1 << "Apport fonte glacier (mm)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

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
			_fichier_out1 << str << endl;
		}

		if (output._eau_glacier)
		{
			string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "glacier-eau.csv") );
			_fichier_out2.open(nom_fichier);
			_fichier_out2 << "Equivalent en eau glacier (m)" << output.Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << output.Separator();

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
			_fichier_out2 << str << endl;
		}
	}


	void FONTE_GLACIER::Calcule()
	{
		ZONES& zones = _sim_hyd.PrendreZones();
		OUTPUT& output = _sim_hyd.PrendreOutput();

		string str;
		size_t index;

		if (output.SauvegardeApportGlacier() || output._eau_glacier)
		{
			ostringstream oss;
			oss.str("");

			ostringstream oss2;
			oss2.str("");
			
			if(output.SauvegardeApportGlacier())
				oss << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_mm) << setiosflags(ios::fixed);

			if(output._eau_glacier)
				oss2 << _sim_hyd.PrendreDateCourante() << output.Separator() << setprecision(output._nbDigit_m) << setiosflags(ios::fixed);

			for (index=0; index<_nbZone; index++)
			{
				if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
				{
					if (output._bSauvegardeTous || 
						find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
					{
						if(output.SauvegardeApportGlacier())
							oss << zones[index].PrendreApportGlacier() << output.Separator();

						if(output._eau_glacier)
							oss2 << zones[index].PrendreEauGlacier() << output.Separator();
					}
				}
			}
			
			if(output.SauvegardeApportGlacier())
			{
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_out1 << str << endl;
			}

			if(output._eau_glacier)	//fichier eau_glacier.csv	//equivalent en eau de la glace [m]
			{
				str = oss2.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_out2 << str << endl;
			}
		}
	}


	void FONTE_GLACIER::Termine()
	{
		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeApportGlacier())
			_fichier_out1.close();

		if (output._eau_glacier)
			_fichier_out2.close();
	}

}

