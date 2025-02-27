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

#include "interpolation_donnees.hpp"

#include "constantes.hpp"
#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"


using namespace std;


namespace HYDROTEL
{

	INTERPOLATION_DONNEES::INTERPOLATION_DONNEES(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_netCdf_tmin = NULL;
		_netCdf_tmax = NULL;
		_netCdf_tminjour = NULL;
		_netCdf_tmaxjour = NULL;
		_netCdf_pluie = NULL;
		_netCdf_neige = NULL;
	}


	INTERPOLATION_DONNEES::~INTERPOLATION_DONNEES()
	{
	}


	void INTERPOLATION_DONNEES::Initialise()
	{
		_corrections_pluie = _sim_hyd.PrendreCorrections().PrendreCorrectionsPluie();
		_corrections_neige = _sim_hyd.PrendreCorrections().PrendreCorrectionsNeige();
		_corrections_temperature = _sim_hyd.PrendreCorrections().PrendreCorrectionsTemperature();

		ZONES& zones = _sim_hyd.PrendreZones();

		OUTPUT& output = _sim_hyd.PrendreOutput();

		if (output.SauvegardeTMin() || output.SauvegardeTMinJour() || output.SauvegardeTMax() || output.SauvegardeTMaxJour() || output.SauvegardePluie() || output.SauvegardeNeige())
			_bSauvegarde = true;
		else
			_bSauvegarde = false;

		if(_bSauvegarde)
		{
			if (output.SauvegardeTMin())
			{
				if (_sim_hyd._outputCDF)
					_netCdf_tmin = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
				else
				{
					_fichier_tmin.open( Combine(_sim_hyd.PrendreRepertoireResultat(), "tmin.csv") );
					_fichier_tmin << "Température minimum (C);" << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();
				}
			}

			if (output.SauvegardeTMinJour())
			{
				if (_sim_hyd._outputCDF)
					_netCdf_tminjour = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
				else
				{
					_fichier_tmin_jour.open( Combine(_sim_hyd.PrendreRepertoireResultat(), "tmin_jour.csv") );
					_fichier_tmin_jour << "Température minimum journalière (C);" << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();
				}
			}

			if (output.SauvegardeTMax())
			{
				if (_sim_hyd._outputCDF)
					_netCdf_tmax = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
				else
				{
					_fichier_tmax.open( Combine(_sim_hyd.PrendreRepertoireResultat(), "tmax.csv") );
					_fichier_tmax << "Température maximum (C);" << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )"<< endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();
				}
			}

			if (output.SauvegardeTMaxJour())
			{
				if (_sim_hyd._outputCDF)
					_netCdf_tmaxjour = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
				else
				{
					_fichier_tmax_jour.open( Combine(_sim_hyd.PrendreRepertoireResultat(), "tmax_jour.csv") );
					_fichier_tmax_jour << "Température maximum journalière (C);" << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )"<< endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();
				}
			}

			if (output.SauvegardePluie())
			{
				if (_sim_hyd._outputCDF)
					_netCdf_pluie = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
				else
				{
					_fichier_pluie.open( Combine(_sim_hyd.PrendreRepertoireResultat(), "pluie.csv") );
					_fichier_pluie << "Précipitation pluie (mm);" << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )"<< endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();
				}
			}

			if (output.SauvegardeNeige())
			{
				if (_sim_hyd._outputCDF)
					_netCdf_neige = new float[_sim_hyd._lNbPasTempsSim*_sim_hyd.PrendreOutput()._uhrhOutputNb];
				else
				{
					_fichier_neige.open( Combine(_sim_hyd.PrendreRepertoireResultat(), "neige.csv") );
					_fichier_neige << "Précipitation neige (EEN) (mm);" << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )"<< endl << "date heure\\uhrh" << _sim_hyd.PrendreOutput().Separator();
				}
			}

			if(!_sim_hyd._outputCDF)
			{
				string str;
				ostringstream oss1;
				oss1.str("");
				ostringstream oss2;
				oss2.str("");
				ostringstream oss3;
				oss3.str("");
				ostringstream oss4;
				oss4.str("");
				ostringstream oss5;
				oss5.str("");
				ostringstream oss6;
				oss6.str("");

				for (size_t index = 0; index < zones.PrendreNbZone(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index) != end(_sim_hyd.PrendreZonesSimules()))
					{
						if (output._bSauvegardeTous || 
								find(begin(output._vIdTronconSelect), end(output._vIdTronconSelect), zones[index].PrendreTronconAval()->PrendreIdent()) != end(output._vIdTronconSelect))
						{
							if (output.SauvegardeTMin())
								oss1 << zones[index].PrendreIdent() << _sim_hyd.PrendreOutput().Separator();

							if (output.SauvegardeTMax())
								oss2 << zones[index].PrendreIdent() << _sim_hyd.PrendreOutput().Separator();

							if (output.SauvegardePluie())
								oss3 << zones[index].PrendreIdent() << _sim_hyd.PrendreOutput().Separator();

							if (output.SauvegardeNeige())
								oss4 << zones[index].PrendreIdent() << _sim_hyd.PrendreOutput().Separator();

							if (output.SauvegardeTMinJour())
								oss5 << zones[index].PrendreIdent() << _sim_hyd.PrendreOutput().Separator();

							if (output.SauvegardeTMaxJour())
								oss6 << zones[index].PrendreIdent() << _sim_hyd.PrendreOutput().Separator();
						}
					}
				}

				if (output.SauvegardeTMin())
				{
					str = oss1.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmin << str << endl;
				}

				if (output.SauvegardeTMax())
				{
					str = oss2.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmax << str << endl;
				}

				if (output.SauvegardePluie())
				{
					str = oss3.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_pluie << str << endl;
				}

				if (output.SauvegardeNeige())
				{
					str = oss4.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_neige << str << endl;
				}

				if (output.SauvegardeTMinJour())
				{
					str = oss5.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmin_jour << str << endl;
				}

				if (output.SauvegardeTMaxJour())
				{
					str = oss6.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmax_jour << str << endl;
				}
			}
		}
	}


	void INTERPOLATION_DONNEES::Calcule()
	{
		ostringstream oss;
		size_t index, i, idx;
		float densite, tmoy, neige;

		DATE_HEURE date_courante = _sim_hyd.PrendreDateCourante();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		AppliquerCorrections(date_courante);

		// sauvegarde des variables

		if(_bSauvegarde)
		{
			ZONES& zones = _sim_hyd.PrendreZones();
			OUTPUT& output = _sim_hyd.PrendreOutput();

			if(_sim_hyd._outputCDF)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _sim_hyd.PrendreOutput()._uhrhOutputNb;

				for (i=0; i<_sim_hyd.PrendreOutput()._uhrhOutputNb; i++)
				{
					if (_netCdf_tmin != NULL)
						_netCdf_tmin[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreTMin();
					
					if (_netCdf_tmax != NULL)
						_netCdf_tmax[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreTMax();

					if (_netCdf_tminjour != NULL)
						_netCdf_tminjour[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreTMinJournaliere();

					if (_netCdf_tmaxjour != NULL)
						_netCdf_tmaxjour[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreTMaxJournaliere();

					if (_netCdf_pluie != NULL)
						_netCdf_pluie[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendrePluie();

					if (_netCdf_neige != NULL)
					{
						//transforme la neige en equivalent en eau					
						if (pas_de_temps == 1)
							densite = CalculDensiteNeige(zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreTMin()) / DENSITE_EAU;
						else
						{
							tmoy = (zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreTMax() + zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreTMin()) / 2.0f;
							densite = CalculDensiteNeige(tmoy) / DENSITE_EAU;
						}

						_netCdf_neige[idx+i] = zones[_sim_hyd.PrendreOutput()._uhrhOutputIndex[i]].PrendreNeige() * densite;
					}
				}
			}
			else
			{
				if (output.SauvegardeTMin())
					_fichier_tmin << date_courante << _sim_hyd.PrendreOutput().Separator();

				if (output.SauvegardeTMax())
					_fichier_tmax << date_courante << _sim_hyd.PrendreOutput().Separator();

				if (output.SauvegardePluie())
					_fichier_pluie << date_courante << _sim_hyd.PrendreOutput().Separator();

				if (output.SauvegardeNeige())
					_fichier_neige << date_courante << _sim_hyd.PrendreOutput().Separator();

				if (output.SauvegardeTMinJour())
					_fichier_tmin_jour << date_courante << _sim_hyd.PrendreOutput().Separator();

				if (output.SauvegardeTMaxJour())
					_fichier_tmax_jour << date_courante << _sim_hyd.PrendreOutput().Separator();

				string str;
				ostringstream oss1;
				oss1.str("");
				ostringstream oss2;
				oss2.str("");
				ostringstream oss3;
				oss3.str("");
				ostringstream oss4;
				oss4.str("");
				ostringstream oss5;
				oss5.str("");
				ostringstream oss6;
				oss6.str("");
		
				for (i=0; i<_sim_hyd._outputNbZone; i++)
				{
					index = _sim_hyd._vOutputIndexZone[i];

					if (output.SauvegardeTMin())
					{
						oss.str("");
						//oss << setprecision(9) << setiosflags(ios::fixed) << zones[index].PrendreTMin();
						oss << setprecision(_sim_hyd.PrendreOutput()._nbDigit_dC) << setiosflags(ios::fixed) << zones[index].PrendreTMin();
						oss1 << oss.str() << _sim_hyd.PrendreOutput().Separator();
					}
	
					if (output.SauvegardeTMax())
					{
						oss.str("");
						oss << setprecision(_sim_hyd.PrendreOutput()._nbDigit_dC) << setiosflags(ios::fixed) << zones[index].PrendreTMax();
						oss2 << oss.str() << _sim_hyd.PrendreOutput().Separator();
					}
	
					if (output.SauvegardePluie())
					{
						oss.str("");
						oss << setprecision(_sim_hyd.PrendreOutput()._nbDigit_mm_interpolation) << setiosflags(ios::fixed) << zones[index].PrendrePluie();
						oss3 << oss.str() << _sim_hyd.PrendreOutput().Separator();
					}
			
					if (output.SauvegardeNeige())
					{
						//transforme la neige en equivalent en eau
						if (pas_de_temps == 1)
							densite = CalculDensiteNeige(zones[index].PrendreTMin()) / DENSITE_EAU;
						else
						{
							tmoy = (zones[index].PrendreTMax() + zones[index].PrendreTMin()) / 2.0f;
							densite = CalculDensiteNeige(tmoy) / DENSITE_EAU;
						}

						neige = zones[index].PrendreNeige() * densite;

						oss.str("");
						oss << setprecision(_sim_hyd.PrendreOutput()._nbDigit_mm_interpolation) << setiosflags(ios::fixed) << neige;
						oss4 << oss.str() << _sim_hyd.PrendreOutput().Separator();
					}

					if (output.SauvegardeTMinJour())
					{
						oss.str("");
						oss << setprecision(_sim_hyd.PrendreOutput()._nbDigit_dC) << setiosflags(ios::fixed) << zones[index].PrendreTMinJournaliere();
						oss5 << oss.str() << _sim_hyd.PrendreOutput().Separator();
					}

					if (output.SauvegardeTMaxJour())
					{
						oss.str("");
						oss << setprecision(_sim_hyd.PrendreOutput()._nbDigit_dC) << setiosflags(ios::fixed) << zones[index].PrendreTMaxJournaliere();
						oss6 << oss.str() << _sim_hyd.PrendreOutput().Separator();
					}
				}

				if (output.SauvegardeTMin())
				{
					str = oss1.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmin << str << endl;
				}

				if (output.SauvegardeTMax())
				{
					str = oss2.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmax << str << endl;
				}

				if (output.SauvegardePluie())
				{
					str = oss3.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_pluie << str << endl;
				}

				if (output.SauvegardeNeige())
				{
					str = oss4.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_neige << str << endl;
				}

				if (output.SauvegardeTMinJour())
				{
					str = oss5.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmin_jour << str << endl;
				}

				if (output.SauvegardeTMaxJour())
				{
					str = oss6.str();
					str = str.substr(0, str.length()-1); //enleve le dernier separateur
					_fichier_tmax_jour << str << endl;
				}
			}
		}
	}


	void INTERPOLATION_DONNEES::Termine()
	{
		if(_bSauvegarde)
		{
			string str1, str2;

			OUTPUT& output = _sim_hyd.PrendreOutput();

			if (output.SauvegardeTMin())
			{
				if (_netCdf_tmin != NULL)
				{
					//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "interpolation-tmin.nc");
					str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "tmin.nc");
					//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "interpolation-tmin", _netCdf_tmin, "C", "Temperature minimum");
					str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "tmin", _netCdf_tmin, "C", "Temperature minimum");
					if(str2 != "")
						throw ERREUR(str2);

					delete [] _netCdf_tmin;
				}
				else
					_fichier_tmin.close();
			}

			if (output.SauvegardeTMax())
			{
				if (_netCdf_tmax != NULL)
				{
					//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "interpolation-tmax.nc");
					str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "tmax.nc");
					//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "interpolation-tmax", _netCdf_tmax, "C", "Temperature maximum");
					str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "tmax", _netCdf_tmax, "C", "Temperature maximum");
					if(str2 != "")
						throw ERREUR(str2);

					delete [] _netCdf_tmax;
				}
				else
					_fichier_tmax.close();
			}

			if (output.SauvegardePluie())
			{
				if (_netCdf_pluie != NULL)
				{
					//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "interpolation-pluie.nc");
					str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "pluie.nc");
					//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "interpolation-pluie", _netCdf_pluie, "mm", "Precipitation pluie");
					str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "pluie", _netCdf_pluie, "mm", "Precipitation pluie");
					if(str2 != "")
						throw ERREUR(str2);

					delete [] _netCdf_pluie;
				}
				else
					_fichier_pluie.close();
			}

			if (output.SauvegardeNeige())
			{
				if (_netCdf_neige != NULL)
				{
					//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "interpolation-neige.nc");
					str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "neige.nc");
					//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "interpolation-neige", _netCdf_neige, "mm", "Pecipitation neige (EEN)");
					str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "neige", _netCdf_neige, "mm", "Pecipitation neige (EEN)");
					if(str2 != "")
						throw ERREUR(str2);

					delete [] _netCdf_neige;
				}
				else
					_fichier_neige.close();
			}

			if (output.SauvegardeTMinJour())
			{
				if (_netCdf_tminjour != NULL)
				{
					//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "interpolation-tminjour.nc");
					str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "tminjour.nc");
					//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "interpolation-tminjour", _netCdf_tminjour, "C", "Temperature minimum journaliere");
					str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "tminjour", _netCdf_tminjour, "C", "Temperature minimum journaliere");
					if(str2 != "")
						throw ERREUR(str2);

					delete [] _netCdf_tminjour;
				}
				else
					_fichier_tmin_jour.close();
			}

			if (output.SauvegardeTMaxJour())
			{
				if (_netCdf_tmaxjour != NULL)
				{
					//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "interpolation-tmaxjour.nc");
					str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "tmaxjour.nc");
					//str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "interpolation-tmaxjour", _netCdf_tmaxjour, "C", "Temperature maximum journaliere");
					str2 = _sim_hyd.PrendreOutput().SauvegardeOutputNetCDF(str1, true, "tmaxjour", _netCdf_tmaxjour, "C", "Temperature maximum journaliere");
					if(str2 != "")
						throw ERREUR(str2);

					delete [] _netCdf_tmaxjour;
				}
				else
					_fichier_tmax_jour.close();
			}
		}
	}


	void INTERPOLATION_DONNEES::AppliquerCorrections(DATE_HEURE date)
	{
		ZONES& zones = _sim_hyd.PrendreZones();

		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		// correction temperatures
		for(auto iter = begin(_corrections_temperature); iter != end(_corrections_temperature); ++iter)
		{
			CORRECTION* correction = *iter;

			if (correction->Applicable(date))
			{
				GROUPE_ZONE* groupe_zone = nullptr;

				switch (correction->PrendreTypeGroupe())
				{
				case TYPE_GROUPE_ALL:
					groupe_zone = _sim_hyd.PrendreToutBassin();
					break;

				case TYPE_GROUPE_HYDRO:
					groupe_zone = _sim_hyd.RechercheGroupeZone(correction->PrendreNomGroupe());
					break;

				case TYPE_GROUPE_CORRECTION:
					groupe_zone = _sim_hyd.RechercheGroupeCorrection(correction->PrendreNomGroupe());
					break;
				}

				for (size_t index = 0; index < groupe_zone->PrendreNbZone(); ++index)
				{
					int ident = groupe_zone->PrendreIdent(index);

					size_t index_zone = zones.IdentVersIndex(ident);
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
					{
						ZONE* zone = zones.Recherche(ident);

						float tmin = (zone->PrendreTMin() + correction->PrendreCoefficientAdditif()) * 
							correction->PrendreCoefficientMultiplicatif();
						float tmax = (zone->PrendreTMax() + correction->PrendreCoefficientAdditif()) * 
							correction->PrendreCoefficientMultiplicatif();

						zone->ChangeTemperature(tmin, tmax);

						float tminjr = (zone->PrendreTMinJournaliere() + correction->PrendreCoefficientAdditif()) * 
							correction->PrendreCoefficientMultiplicatif();
						float tmaxjr = (zone->PrendreTMaxJournaliere() + correction->PrendreCoefficientAdditif()) * 
							correction->PrendreCoefficientMultiplicatif();

						zone->ChangeTemperatureJournaliere(tminjr, tmaxjr);
					}
				}
			}
		}

		// correction pluie
		for(auto iter = begin(_corrections_pluie); iter != end(_corrections_pluie); ++iter)
		{
			CORRECTION* correction = *iter;

			if (correction->Applicable(date))
			{
				GROUPE_ZONE* groupe_zone = nullptr;

				switch (correction->PrendreTypeGroupe())
				{
				case TYPE_GROUPE_ALL:
					groupe_zone = _sim_hyd.PrendreToutBassin();
					break;

				case TYPE_GROUPE_HYDRO:
					groupe_zone = _sim_hyd.RechercheGroupeZone(correction->PrendreNomGroupe());
					break;

				case TYPE_GROUPE_CORRECTION:
					groupe_zone = _sim_hyd.RechercheGroupeCorrection(correction->PrendreNomGroupe());
					break;
				}

				for (size_t index = 0; index < groupe_zone->PrendreNbZone(); ++index)
				{
					int ident = groupe_zone->PrendreIdent(index);

					size_t index_zone = zones.IdentVersIndex(ident);
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
					{
						ZONE* zone = zones.Recherche(ident);

						float pluie = (zone->PrendrePluie() + correction->PrendreCoefficientAdditif()) * 
							correction->PrendreCoefficientMultiplicatif();
						zone->ChangePluie(max(0.0f, pluie));
					}
				}
			}
		}

		// correction neige
		for(auto iter = begin(_corrections_neige); iter != end(_corrections_neige); ++iter)
		{
			CORRECTION* correction = *iter;

			if (correction->Applicable(date))
			{
				GROUPE_ZONE* groupe_zone = nullptr;

				switch (correction->PrendreTypeGroupe())
				{
				case TYPE_GROUPE_ALL:
					groupe_zone = _sim_hyd.PrendreToutBassin();
					break;

				case TYPE_GROUPE_HYDRO:
					groupe_zone = _sim_hyd.RechercheGroupeZone(correction->PrendreNomGroupe());
					break;

				case TYPE_GROUPE_CORRECTION:
					groupe_zone = _sim_hyd.RechercheGroupeCorrection(correction->PrendreNomGroupe());
					break;
				}

				ZONE* zone;
				float densite_neige, tmoy, neigeAdd, neige;
				int ident;

				for (size_t index = 0; index < groupe_zone->PrendreNbZone(); ++index)
				{
					ident = groupe_zone->PrendreIdent(index);

					size_t index_zone = zones.IdentVersIndex(ident);
					if(find(begin(_sim_hyd.PrendreZonesSimules()), end(_sim_hyd.PrendreZonesSimules()), index_zone) != end(_sim_hyd.PrendreZonesSimules()))
					{
						zone = zones.Recherche(ident);

						if (pas_de_temps == 1)
							densite_neige = CalculDensiteNeige(zones[index].PrendreTMin()) / DENSITE_EAU;
						else
						{
							tmoy = (zone->PrendreTMax() + zone->PrendreTMin()) / 2.0f;
							densite_neige = CalculDensiteNeige(tmoy) / DENSITE_EAU;
						}

						neigeAdd = correction->PrendreCoefficientAdditif() / densite_neige;	//equivalent en eau de la neige [mm] -> hauteur de precipitation en neige
						neige = (zone->PrendreNeige() + neigeAdd) * correction->PrendreCoefficientMultiplicatif();

						zone->ChangeNeige(max(0.0f, neige));
					}
				}
			}
		}
	}

}
