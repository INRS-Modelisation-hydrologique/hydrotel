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

#include "acheminement_riviere.hpp"

#include "barrage_historique.hpp"
#include "util.hpp"
#include "version.hpp"
#include "erreur.hpp"

#include <boost/algorithm/string/case_conv.hpp>


using namespace std;


namespace HYDROTEL
{

	ACHEMINEMENT_RIVIERE::ACHEMINEMENT_RIVIERE(SIM_HYD& sim_hyd, const std::string& nom)
		: SOUS_MODELE(sim_hyd, nom)
	{
		_pOutput = &sim_hyd.PrendreOutput();
		
		_netCdf_debitaval = NULL;
		_netCdf_debitamont = NULL;

		_netCdf_prelevements_pression = NULL;
	}


	ACHEMINEMENT_RIVIERE::~ACHEMINEMENT_RIVIERE()
	{
	}


	void ACHEMINEMENT_RIVIERE::Initialise()
	{
		string str;

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();
		STATIONS_HYDRO& stations_hydro = _sim_hyd.PrendreStationsHydro();

		DATE_HEURE debut = _sim_hyd.PrendreDateDebut();
		DATE_HEURE fin = _sim_hyd.PrendreDateFin();
		unsigned short pas_de_temps = _sim_hyd.PrendrePasDeTemps();

		std::vector<size_t> vTronconSim = _sim_hyd.PrendreTronconsSimules();

		for(size_t index_troncon = 0; index_troncon < vTronconSim.size(); ++index_troncon)
		{
			//lecture des debits pour les troncons barrage historique

			if(troncons[vTronconSim[index_troncon]]->PrendreType() == TRONCON::BARRAGE_HISTORIQUE)
			{
				BARRAGE_HISTORIQUE* barrage = static_cast<BARRAGE_HISTORIQUE*>(troncons[vTronconSim[index_troncon]]);

				string ident_station_hydro = barrage->PrendreIdentStationHydro();
				str = ident_station_hydro;
				boost::algorithm::to_lower(str);

				STATION_HYDRO* station_hydro = static_cast<STATION_HYDRO*>(stations_hydro.Recherche(str));

                if(station_hydro == nullptr)
				{
                    ostringstream oss;
			        oss.clear();
					oss.str("");

					oss << "Error when reading river reach data: ";

					if(str == "aucune")
						oss << "a hydro station must be associated with the river reach (" << barrage->PrendreIdent() << ") (reservoir with history)";
					else
					{
						oss << "the hydro station (" << ident_station_hydro << ") associated with the river reach (" << barrage->PrendreIdent() 
								<< ") (reservoir with history) dont exists in the hydro stations database: " << _sim_hyd._stations_hydro.PrendreNomFichier();
					}

                    throw ERREUR(oss.str());
				}
				
				station_hydro->LectureDonnees(debut, fin, pas_de_temps);
				barrage->ChangeStationHydro(station_hydro);
			}
		}

		//initialisation sauvegarde des resultats
		
		if(_pOutput->SauvegardeDebitAval())
		{
			if(_sim_hyd._outputCDF)
				_netCdf_debitaval = new float[_sim_hyd._lNbPasTempsSim*_pOutput->_tronconOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "debit_aval.csv") );

				_fichier_debit_aval.open(nom_fichier.c_str());

				_fichier_debit_aval << "Débit en aval du tronçon (m3/s)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl;
				_fichier_debit_aval << "date heure\\troncon" << _pOutput->Separator();

				ostringstream oss;
				oss.str("");

				for(size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << _pOutput->Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_debit_aval << str << endl;
			}
		}

		if(_pOutput->SauvegardeDebitAmont())
		{
			if(_sim_hyd._outputCDF)
				_netCdf_debitamont = new float[_sim_hyd._lNbPasTempsSim*_pOutput->_tronconOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "debit_amont.csv") );

				_fichier_debit_amont.open(nom_fichier.c_str());

				_fichier_debit_amont << "Débit en amont du tronçon (m3/s)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl;
				_fichier_debit_amont << "date heure\\troncon" << _pOutput->Separator();

				ostringstream oss;
				oss.str("");
			
				for(size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << _pOutput->Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_debit_amont << str << endl;
			}
		}

		if(_pOutput->_hauteur_aval)
		{
			//if (_sim_hyd._outputCDF)
			//	_netCdf_hauteuraval = new float[_sim_hyd._lNbPasTempsSim*_pOutput->_tronconOutputNb];
			//else
			//{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "hauteur_aval.csv") );

				_fichier_hauteur_aval.open(nom_fichier.c_str());

				_fichier_hauteur_aval << "Hauteur eau en aval du tronçon (m)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl;
				_fichier_hauteur_aval << "date heure\\troncon" << _pOutput->Separator();

				ostringstream oss;
				oss.str("");

				for(size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << _pOutput->Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_hauteur_aval << str << endl;
			//}
		}

		if(_sim_hyd._pr->_bSimulePrelevements)	//if (_pOutput->SauvegardePrelevementPression())
		{
			if(_sim_hyd._outputCDF)
				_netCdf_prelevements_pression = new float[_sim_hyd._lNbPasTempsSim*_pOutput->_tronconOutputNb];
			else
			{
				string nom_fichier( Combine(_sim_hyd.PrendreRepertoireResultat(), "prelevements_pression.csv") );

				_fichier_prelevements_pression.open(nom_fichier.c_str());

				_fichier_prelevements_pression << "Indice de pression (0-1)" << _pOutput->Separator() << PrendreNomSousModele() << " ( VERSION " << HYDROTEL_VERSION << " )" << endl;
				_fichier_prelevements_pression << "Date heure\\troncon" << _pOutput->Separator();

				ostringstream oss;
				oss.str("");

				for(size_t index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreIdent() << _pOutput->Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_prelevements_pression << str << endl;
			}
		}
	}


	void ACHEMINEMENT_RIVIERE::Calcule()
	{
		ostringstream oss;
		size_t i, idx, index;
		string str;

		TRONCONS& troncons = _sim_hyd.PrendreTroncons();

		if(_pOutput->SauvegardeDebitAval())
		{
			if(_netCdf_debitaval != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _pOutput->_tronconOutputNb;

				for(i=0; i<_pOutput->_tronconOutputNb; i++)
					_netCdf_debitaval[idx+i] = troncons[_pOutput->_tronconOutputIndex[i]]->PrendreDebitAvalMoyen();
			}
			else
			{
				oss.str("");
				oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed);

				for(index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreDebitAvalMoyen() << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_debit_aval << str << endl;
			}
		}

		if(_pOutput->SauvegardeDebitAmont())
		{
			if(_netCdf_debitamont != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _pOutput->_tronconOutputNb;

				for(i=0; i<_pOutput->_tronconOutputNb; i++)
					_netCdf_debitamont[idx+i] = troncons[_pOutput->_tronconOutputIndex[i]]->PrendreDebitAmontMoyen();
			}
			else
			{
				oss.str("");
				oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed);

				for(index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->PrendreDebitAmontMoyen() << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_debit_amont << str << endl;
			}
		}

		if(_pOutput->_hauteur_aval)
		{
			//if(_netCdf_hauteuraval != NULL)
			//{
			//	idx = _sim_hyd._lPasTempsCourantIndex * _pOutput->_tronconOutputNb;

			//	for(i=0; i<_pOutput->_tronconOutputNb; i++)
			//		_netCdf_hauteuraval[idx+i] = troncons[_pOutput->_tronconOutputIndex[i]]->_hauteurAvalMoy;
			//}
			//else
			//{
				oss.str("");
				oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_m3s) << setiosflags(ios::fixed);

				for(index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->_hauteurAvalMoy << _pOutput->Separator();
						}
					}
				}

				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_hauteur_aval << str << endl;
			//}
		}

		if(_sim_hyd._pr->_bSimulePrelevements)	//if (_pOutput->SauvegardePrelevementPression())
		{
			if(_netCdf_prelevements_pression != NULL)
			{
				idx = _sim_hyd._lPasTempsCourantIndex * _pOutput->_tronconOutputNb;

				for(i=0; i<_pOutput->_tronconOutputNb; i++)
					_netCdf_prelevements_pression[idx+i] = static_cast<float>(troncons[_pOutput->_tronconOutputIndex[i]]->_prIndicePression);
			}
			else
			{
				oss.str("");
				oss << _sim_hyd.PrendreDateCourante() << _pOutput->Separator() << setprecision(_pOutput->_nbDigit_ratio) << setiosflags(ios::fixed);

				for(index = 0; index < troncons.PrendreNbTroncon(); ++index)
				{
					if(find(begin(_sim_hyd.PrendreTronconsSimules()), end(_sim_hyd.PrendreTronconsSimules()), index) != end(_sim_hyd.PrendreTronconsSimules()))
					{
						if(_pOutput->_bSauvegardeTous ||
							find(begin(_pOutput->_vIdTronconSelect), end(_pOutput->_vIdTronconSelect), troncons[index]->PrendreIdent()) != end(_pOutput->_vIdTronconSelect))
						{
							oss << troncons[index]->_prIndicePression << _pOutput->Separator();
						}
					}
				}
			
				str = oss.str();
				str = str.substr(0, str.length()-1); //enleve le dernier separateur
				_fichier_prelevements_pression << str << endl;
			}
		}
	}


	void ACHEMINEMENT_RIVIERE::Termine()
	{
		string str1, str2;

		if(_pOutput->SauvegardeDebitAval())
		{
			if(_netCdf_debitaval != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "acheminement-debit_aval.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "debit_aval.nc");
				//str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "acheminement-debit_aval", _netCdf_debitaval, "m3/s", "Debit en aval du troncon");
				str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "debit_aval", _netCdf_debitaval, "m3/s", "Debit en aval du troncon");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_debitaval;
			}
			else
				_fichier_debit_aval.close();
		}

		if(_pOutput->SauvegardeDebitAmont())
		{
			if(_netCdf_debitamont != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "acheminement-debit_amont.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "debit_amont.nc");
				//str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "acheminement-debit_amont", _netCdf_debitamont, "m3/s", "Debit en amont du troncon");
				str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "debit_amont", _netCdf_debitamont, "m3/s", "Debit en amont du troncon");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_debitamont;
			}
			else
				_fichier_debit_amont.close();
		}

		if(_pOutput->_hauteur_aval)
		{
			//if(_netCdf_hauteuraval != NULL)
			//{
			//	//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "acheminement-hauteur_aval.nc");
			//	str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "hauteur_aval.nc");
			//	//str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "acheminement-hauteur_aval", _netCdf_hauteuraval, "m3/s", "Hauteur eau en aval du troncon");
			//	str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "hauteur_aval", _netCdf_hauteuraval, "m3/s", "Hauteur eau en aval du troncon");
			//	if(str2 != "")
			//		throw ERREUR(str2);

			//	delete [] _netCdf_hauteuraval;
			//}
			//else
				_fichier_hauteur_aval.close();
		}

		if(_sim_hyd._pr->_bSimulePrelevements)	//if (_pOutput->SauvegardePrelevementPression())
		{
			if(_netCdf_prelevements_pression != NULL)
			{
				//str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "acheminement-debit_aval.nc");
				str1 = Combine(_sim_hyd.PrendreRepertoireResultat(), "prelevements_pression.nc");
				//str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "acheminement-debit_aval", _netCdf_debitaval, "m3/s", "Debit en aval du troncon");
				str2 = _pOutput->SauvegardeOutputNetCDF(str1, false, "prelevements_pression", _netCdf_prelevements_pression, "0-1", "Indice de pression des prélèvements");
				if(str2 != "")
					throw ERREUR(str2);

				delete [] _netCdf_prelevements_pression;
			}
			else
				_fichier_prelevements_pression.close();
		}
	}

}
