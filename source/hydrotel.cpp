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

#include "gdal_util.hpp"
#include "mise_a_jour.hpp"
#include "erreur.hpp"
#include "statistiques.hpp"
#include "util.hpp"
#include "version.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/chrono.hpp>


using namespace std;
using namespace HYDROTEL;


void displayHelp()
{
	std::cout << "hydrotel [-h/i/g/mr/n/u/v] [<project filename>] [-c/d/lp/r/s] [-t <nb thread>] [-l <log filename>]" << endl;
	std::cout << endl;

	std::cout << " <project filename>          Run the simulation currently selected in the project file (*.csv)." << endl;
	std::cout << "                             USAGE: hydrotel <hydrotel project filename>" << endl;
	std::cout << endl;
	std::cout << " -c (-skipcharvalidation)    Skip validation of input files characters." << endl;
	std::cout << "                             USAGE: hydrotel <hydrotel project filename> -c" << endl;
	std::cout << endl;
	std::cout << " -d (-display)               Display simulation progress." << endl;
	std::cout << "                             USAGE: hydrotel <hydrotel project filename> -d" << endl;
	std::cout << "                             This option can slow down the execution time of simulations." << endl;
	std::cout << endl;
	std::cout << " -g (-hgm)                   Generate hgm file (geomorphological hydrograph)." << endl;
	std::cout << "                             USAGE: hydrotel -g <hydrotel project filename> <water depth (m)> <new hgm filename> [-t <nb thread>]" << endl;
	std::cout << endl;
	std::cout << " -h (-help)                  Display help." << endl;
	std::cout << "                             USAGE: hydrotel -h" << endl;
	std::cout << endl;
	std::cout << " -i (-info)                  Save connectivity and RHHUs informations in the `project-info` folder." << endl;
	std::cout << "                             USAGE: hydrotel -i <hydrotel project filename>" << endl;
	std::cout << endl;
	std::cout << " -l (-log)                   Save error messages in the specified file." << endl;
	std::cout << "                             USAGE: hydrotel <hydrotel project filename> -l <log filename>" << endl;
	std::cout << endl;
	std::cout << " -lp (-logperformance)       Save detailed program execution times." << endl;
	std::cout << "                             USAGE: hydrotel <hydrotel project filename> -lp" << endl;
	std::cout << "                             Times are expressed in seconds, except in minutes if total duration is greater than 10 minutes." << endl;
	std::cout << "                             To force output in seconds, use -lps or -logperformancesec." << endl;
	std::cout << "                             To force output in minutes, use -lpm or -logperformancemin." << endl;
	std::cout << endl;
	std::cout << " -mr (-modreach)             Modify river reach type." << endl;
	std::cout << "                             USAGE: hydrotel -mr <reach id> <new reach type> [param] <hydrotel project filename>" << endl;
	std::cout << "                             Reach types: 1 (river): [param] = <width (m)> <manning coefficient>" << endl;
	std::cout << "                                          2 (lake): [param] = <length (m)> <area (km2)> <c coefficient> <k coefficient>" << endl;
	std::cout << "                                          4 (lake without storage)" << endl;
	std::cout << "                                          5 (reservoir with history): [param] = <hydrological station id>" << endl;
	std::cout << "                             Use `d` character instead of the parameter value to use default value." << endl;
	std::cout << "                             If only one `d` character is specified for [param], all parameters will be set to default values." << endl;
	std::cout << "                             Parameters default value: <width> = Physitel default value" << endl;
	std::cout << "                                                       <manning coefficient> = 0.04" << endl;
	std::cout << "                                                       <length> = Physitel default value" << endl;
	std::cout << "                                                       <area> = Physitel default value" << endl;
	std::cout << "                                                       <c coefficient> = Physitel default value" << endl;
	std::cout << "                                                       <k coefficient> = 1.5" << endl;
	std::cout << "                             Ex: hydrotel -mr 10 1 100.25 0.06 /projectfolder/projectfile.csv" << endl;
	std::cout << "                                 hydrotel -mr 10 1 100.25 d \"/project folder/project file.csv\"" << endl;
	std::cout << "                                 hydrotel -mr 10 1 d \"/project folder/project file.csv\"" << endl;
	std::cout << endl;
	std::cout << " -n (-new)                   Creation of a new Hydrotel project from a Physitel dataset." << endl;
	std::cout << "                             USAGE: hydrotel -n <input physitel dataset folder> <output hydrotel folder>" << endl;
	std::cout << endl;
	std::cout << " -r (-autoreversetemp)       Input weather data: automatically reverse inverted minimum and" << endl;
	std::cout << "                             maximum temperature values (not modifying source dataset)." << endl;
	std::cout << "                             USAGE: hydrotel <hydrotel project filename> -r" << endl;
	std::cout << endl;
	std::cout << " -s (-skipinterpolation)     Skip interpolation of missing weather data at stations level." << endl;
	std::cout << "                             USAGE: hydrotel <hydrotel project filename> -s" << endl;
	std::cout << "                             This option can be used to speed up the initialization step of a" << endl;
	std::cout << "                             simulation when there is no missing data in source weather dataset." << endl;
	std::cout << endl;
	std::cout << " -t (-thread)                Number of threads to use for hgm computation." << endl;
	std::cout << "                             USAGE: hydrotel [-g] <hydrotel project filename> [...] -t <nb thread>" << endl;
	std::cout << "                             <nb thread> value of 0 will use the maximum number of available threads." << endl;
	std::cout << endl;
	std::cout << " -u (-update)                Update an older 2.6 (v47 ou V49) Hydrotel project to the current version." << endl;
	std::cout << "                             USAGE: hydrotel -u <prj filename> <new project folder>" << endl;
	std::cout << endl;
	std::cout << " -v (-version)               Display version informations." << endl;
	std::cout << "                             USAGE: hydrotel -v" << endl;
	std::cout << endl;
}


bool _log = false;
string _nom_fichier_log;


int main(int argc, char* argv[])
{
	string sLogExecution = GetCurrentTimeStr() + "   Hydrotel " + HYDROTEL_VERSION + " program execution start";

	ostringstream oss;
	vector<string> unrecognizedParam;
	vector<string> unrecognizedParamException;
	istringstream iss;
	size_t idx;
	string option, str, str2, str3;
	time_t begin;
	time_t end;
	float lame;
	bool bAutoInverseTMinTmax, bDisplay, bStationInterpolation, bSkipCharacterValidation, bGenereBdPrelev;
	bool bLogPerformance, bLogPerfForceUnit;
	int nbThread, iLogPerfUnit, ret, n;

	std::unique_ptr<SIM_HYD> sim_hyd;

	idx = (size_t)-1;

	nbThread = 1;	//default: use only 1 thread

	ret = 0;

	str = argv[0];
	for(n=1; n!=argc; n++)
	{
		str2 = argv[n];
		str+= " " + str2;
	}
	HYDROTEL::_listLog.push_back(str);
	HYDROTEL::_listLog.push_back("");

	HYDROTEL::Log("");
	oss << "HYDROTEL " << HYDROTEL_VERSION;
	HYDROTEL::Log(oss.str());
	HYDROTEL::Log("INRS - Eau Terre Environnement Research Center");
	HYDROTEL::Log("");

	bAutoInverseTMinTmax = false;
	bDisplay = false;
	bStationInterpolation = true;
	bSkipCharacterValidation = false;
	bGenereBdPrelev = false;
	bLogPerformance = false;
	iLogPerfUnit = 1; //seconds
	bLogPerfForceUnit = false;

	if (argc == 1)
		displayHelp();
	else
	{
		for (n = 1; n != argc; n++)
		{
			option = argv[n];
			boost::algorithm::to_lower(option);

			if (option.compare("-l") == 0 || option.compare("-log") == 0)
			{
				if (n + 1 == argc)
				{
					std::cout << "Missing parameter <log filename>" << endl << endl;
					displayHelp();
					ret = 1;
				}
				else
				{
					_log = true;
					_nom_fichier_log = argv[n + 1];
					unrecognizedParamException.push_back(_nom_fichier_log);

					std::replace(_nom_fichier_log.begin(), _nom_fichier_log.end(), '\\', '/');		//replace \ to / for compatibility between windows and unix, / are ok for windows but \ are not ok for unix...

					if (FichierExiste(_nom_fichier_log))
						SupprimerFichier(_nom_fichier_log);
				}
			}
			else
			{
				if (option.compare("-lp") == 0 || option.compare("-logperformance") == 0 || 
					option.compare("-lps") == 0 || option.compare("-logperformancesec") == 0 || 
					option.compare("-lpm") == 0 || option.compare("-logperformancemin") == 0)
				{
					bLogPerformance = true;

					if(option.compare("-lpm") == 0 || option.compare("-logperformancemin") == 0)
						iLogPerfUnit = 2;	//minutes
					
					if(option.compare("-lpm") == 0 || option.compare("-logperformancemin") == 0 || option.compare("-lps") == 0 || option.compare("-logperformancesec") == 0)
						bLogPerfForceUnit = true;
				}
				else
				{
					if (option.compare("-r") == 0 || option.compare("-autoreversetemp") == 0)
						bAutoInverseTMinTmax = true;
					else
					{
						if (option.compare("-c") == 0 || option.compare("-skipcharvalidation") == 0)
							bSkipCharacterValidation = true;
						else
						{
							if (option.compare("-d") == 0 || option.compare("-display") == 0)
								bDisplay = true;
							else
							{
								if (option.compare("-s") == 0 || option.compare("-skipinterpolation") == 0)
									bStationInterpolation = false;
								else
								{
									if (option.compare("-t") == 0 || option.compare("-thread") == 0)
									{
										if (n + 1 == argc)
										{
											std::cout << "Missing parameter <nb thread>" << endl << endl;
											displayHelp();
											ret = 1;
										}
										else
										{
											str = argv[n + 1];
											unrecognizedParamException.push_back(str);

											iss.clear();
											iss.str(str);
											iss >> nbThread;

											if (nbThread < 0)
											{
												std::cout << "Parameter <nb thread> is invalid: \"" << argv[n + 1] << "\": must be greater or equal 0" << endl;
												ret = 1;
											}
										}
									}
									else
									{
										if (option.compare("-generebdprelevements") == 0)
											bGenereBdPrelev = true;
										else
										{
											if(n != 1)
											{
												str = argv[n];
												if(std::find(std::begin(unrecognizedParamException), std::end(unrecognizedParamException), str) == std::end(unrecognizedParamException))
													unrecognizedParam.push_back(option);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (ret == 0)
		{
			try
			{
				// verifie les options
				option = argv[1];

				if (option.compare("-u") == 0 || option.compare("-update") == 0)
				{
					// -u [nom fichier projet v26] [nouveau nom repertoire]								
					if (argc < 4)
					{
						std::cout << "Missing parameters..." << endl << endl;
						displayHelp();
						ret = 1;
					}
					else
					{
						GDALAllRegister();
						OGRRegisterAll();

						str2 = argv[2];
						std::replace(str2.begin(), str2.end(), '\\', '/');
						str3 = argv[3];
						std::replace(str3.begin(), str3.end(), '\\', '/');

						std::cout << "Updating project..." << endl;
						std::cout << str2 << endl;

						HYDROTEL::MiseAJourProjet(str2, str3);
						std::cout << endl << "Update completed: " << str3 << endl << endl;
					}
				}
				else if (option.compare("-i") == 0 || option.compare("-info") == 0)
				{
					// -i [nom fichier projet]
					if (argc < 3)
					{
						std::cout << "Missing parameter <hydrotel project filename>" << endl << endl;
						displayHelp();
						ret = 1;
					}
					else
					{
						GDALAllRegister();

						str2 = argv[2];
						std::replace(str2.begin(), str2.end(), '\\', '/');

						std::cout << "Info " << str2 << endl << endl;
						Info(str2);
					}
				}
				else if (option.compare("-g") == 0 || option.compare("-hgm") == 0)
				{
					// -g [nom fichier projet] [lame (m)] [nom fichier hgm]
					if (argc < 5)
					{
						std::cout << "Missing parameters..." << endl << endl;
						displayHelp();
						ret = 1;
					}
					else
					{
						str = argv[3];
						std::replace(str.begin(), str.end(), ',', '.');
						iss.clear();
						iss.str(str);
						iss >> lame;

						if (lame <= 0.0f || lame > 1000.0f)
						{
							std::cout << "Parameter <water depth (m)> is invalid: \"" << argv[3] << "\": must be greater than 0 and less than 1000" << endl;
							ret = 1;
						}
						else
						{
							GDALAllRegister();

							sim_hyd = std::make_unique<SIM_HYD>();
							sim_hyd->_nbThread = nbThread;
							sim_hyd->_bSkipCharacterValidation = bSkipCharacterValidation;

							str = argv[2];
							std::replace(str.begin(), str.end(), '\\', '/');
							sim_hyd->ChangeNomFichier(str);
							sim_hyd->Lecture();

							std::time(&begin);

							str = argv[4];
							std::replace(str.begin(), str.end(), '\\', '/');
							sim_hyd->CalculeHgm(lame, str);

							std::time(&end);

							oss.str("");
							if ((end - begin) / 60.0 / 60.0 < 1.0)
							{
								if ((end - begin) / 60.0 < 1.0)
									oss << "   completed in " << setprecision(0) << setiosflags(ios::fixed) << (end - begin) << " sec";
								else
									oss << "   completed in " << setprecision(2) << setiosflags(ios::fixed) << (end - begin) / 60.0 << " min";
							}
							else
								oss << "   completed in " << setprecision(2) << setiosflags(ios::fixed) << (end - begin) / 60.0 / 60.0 << " h";

							std::cout << oss.str() << endl << endl;
							std::cout << "File saved: " << str << endl;
						}
					}
				}
				else if (option.compare("-n") == 0 || option.compare("-new") == 0)
				{
					if (argc < 4)
					{
						std::cout << "Missing parameters..." << endl << endl;
						displayHelp();
						ret = 1;
					}
					else
					{
						GDALAllRegister();
						OGRRegisterAll();

						string path_in(argv[2]);
						string path_out(argv[3]);

						std::replace(path_in.begin(), path_in.end(), '\\', '/');
						std::replace(path_out.begin(), path_out.end(), '\\', '/');

						if (!boost::filesystem::is_directory(boost::filesystem::path(path_in)))
							throw ERREUR("Error: invalid input folder path.");

						//erreur si le dossier de destination existe et nest pas vide
						if (boost::filesystem::exists(path_out) && !boost::filesystem::is_empty(path_out))
							throw ERREUR("Error: output folder already exists.");

						string nom_bassin = ExtraitNomFichier(path_out);
						string nom_fichier = Combine(path_out, nom_bassin + ".csv");

						string repertoire_physitel = Combine(path_out, "physitel");
						HYDROTEL::CreeRepertoire(repertoire_physitel);

						HYDROTEL::CopieRepertoire(path_in, repertoire_physitel);

						std::cout << "Input: " << path_in << endl << endl;

						std::cout << "Creating hydrotel project... " << endl;

						sim_hyd = std::make_unique<SIM_HYD>();
						sim_hyd->ChangeNomFichier(nom_fichier);
						sim_hyd->CreerNouveauProjet(path_out);

						std::cout << endl << "Hydrotel project created: " << path_out << endl << endl;
					}
				}
				else if (option.compare("-mr") == 0 || option.compare("-modreach") == 0)
				{
					if(argc < 5)
					{
						std::cout << "Missing parameters..." << endl << endl;
						displayHelp();
						ret = 1;
					}
					else
					{
						GDALAllRegister();

						sim_hyd = std::make_unique<SIM_HYD>();
						sim_hyd->_bSkipCharacterValidation = true;

						str = argv[argc-1];
						std::replace(str.begin(), str.end(), '\\', '/');
						sim_hyd->ChangeNomFichier(str);
						sim_hyd->Lecture(false);

						str = "";
						for(n=2; n!=argc-1; n++)
						{
							option = argv[n];
							str+= " " + option;
						}
						str = str.substr(1);

						str = sim_hyd->Command_ChangeReachType(str);
						if(str != "")
						{
							std::cout << "Error modifying reach type: " << str << endl << endl;
							ret = 1;
						}
						else
							std::cout << "Reach id " << argv[2] << " modified successfully." << endl << endl;
					}
				}
				else if (option.compare("-v") == 0 || option.compare("-version") == 0)
				{
					std::cout << "GDAL/OGR  " << GDAL_RELEASE_NAME << endl;
					std::cout << "boost     " << BOOST_LIB_VERSION << endl;
					std::cout << endl;
					std::cout << "https://inrs.ca/en/inrs/research-centres/eau-terre-environnement-research-centre" << endl << endl;
					std::cout << "https://github.com/INRS-Modelisation-hydrologique/hydrotel" << endl;
					std::cout << endl;
				}
				else if (option.compare("-h") == 0 || option.compare("-help") == 0)
				{
					displayHelp();
				}
				else
				{
					GDALAllRegister();

					if (bGenereBdPrelev)
					{
						//genere fichiers bd prelevements (/simulation/[nom_simulation]/prelevements/BD_*.csv) à partir des fichiers sources (/simulation/[nom_simulation]/prelevements/SitesPrelevements/*.csv)

						sim_hyd = std::make_unique<SIM_HYD>();

						sim_hyd->_bSkipCharacterValidation = bSkipCharacterValidation;
						sim_hyd->_bGenereBdPrelevements = true;

						str = argv[1];
						std::replace(str.begin(), str.end(), '\\', '/');
						sim_hyd->ChangeNomFichier(str);	// [nom fichier projet]
						sim_hyd->Lecture();

						sim_hyd->_pr->GenerateBdPrelevements();
					}
					else
					{
						//demarre la simulation
						sim_hyd = std::make_unique<SIM_HYD>();

						for(idx=0; idx!=unrecognizedParam.size(); idx++)
						{
							HYDROTEL::Log("Unknown parameter: " + unrecognizedParam[idx]);
							HYDROTEL::Log("");
						}

						str = argv[1];	//[nom fichier projet]
						std::replace(str.begin(), str.end(), '\\', '/');

						sim_hyd->_bLogPerf = bLogPerformance;

						if(sim_hyd->_bLogPerf)
						{
							sim_hyd->_logPerformance._iUnit = iLogPerfUnit;
							sim_hyd->_logPerformance._bForceUnit = bLogPerfForceUnit;

							sim_hyd->_logPerformance.AddStep(sLogExecution, false);	//log start of execution
							sim_hyd->_logPerformance.AddStep("Project: " + str);

							sim_hyd->_logPerformance._tpInitAndSimBegin =  boost::chrono::high_resolution_clock::now();
						}

						std::time(&begin);

						sim_hyd->_bSimul = true;
						sim_hyd->_nbThread = nbThread;
						sim_hyd->_bAutoInverseTMinTMax = bAutoInverseTMinTmax;
						sim_hyd->_bStationInterpolation = bStationInterpolation;
						sim_hyd->_bSkipCharacterValidation = bSkipCharacterValidation;

						HYDROTEL::Log("Reading simulation data...   " + GetCurrentTimeStr());

						sim_hyd->ChangeNomFichier(str);
						sim_hyd->Lecture();

						if(_log && _nom_fichier_log.find('/') == string::npos)
							_nom_fichier_log = sim_hyd->PrendreRepertoireSimulation() + "/" + _nom_fichier_log; //save log file to simulation folder if only filename without path is specified

						if(sim_hyd->_bLogPerf)
						{
							sim_hyd->_logPerformance.EndStep(sim_hyd->_tempVal, boost::chrono::high_resolution_clock::now());	//Lecture données

							sim_hyd->_logPerformance._tpLectureEnd = boost::chrono::high_resolution_clock::now();
							sim_hyd->_logPerformance._tpInitBegin = boost::chrono::high_resolution_clock::now();
						}

						std::cout << endl << "Initialization...   " << GetCurrentTimeStr() << flush;
						HYDROTEL::_listLog.push_back("");
						HYDROTEL::_listLog.push_back("Initialization...   " + GetCurrentTimeStr());

						if (!bStationInterpolation)
						{
							std::cout << endl << "Interpolation of missing weather data at stations level will be skipped     " << flush;
							HYDROTEL::_listLog.push_back("Interpolation of missing weather data at stations level will be skipped");
						}

						sim_hyd->Initialise();

						DATE_HEURE date_fin, date_courante;

						date_fin = sim_hyd->PrendreDateFin();

						std::cout << endl << "Simulation in progress...   " << GetCurrentTimeStr() << flush;
						HYDROTEL::_listLog.push_back("Simulation in progress...   " + GetCurrentTimeStr());

						if(bDisplay)
							std::cout << endl;

						if(sim_hyd->_bLogPerf)
						{
							sim_hyd->_logPerformance._tpInitEnd = boost::chrono::high_resolution_clock::now();
							sim_hyd->_logPerformance._tpSimBegin = boost::chrono::high_resolution_clock::now();
							sim_hyd->_logPerformance.AddStep("Simulation start");
						}

						do
						{
							if(bDisplay)
							{
								date_courante = sim_hyd->PrendreDateCourante();
								std::cout << date_courante << '\r' << flush;
							}

							sim_hyd->Calcule();
							date_courante = sim_hyd->PrendreDateCourante();
						} 
						while(date_courante < date_fin);

						if(sim_hyd->_bLogPerf)
						{
							sim_hyd->_logPerformance._tpSimEnd = boost::chrono::high_resolution_clock::now();
							sim_hyd->_logPerformance.AddStep("Simulation end");

							idx = sim_hyd->_logPerformance.AddStep("Post-processing", boost::chrono::high_resolution_clock::now());

							sim_hyd->_logPerformance._tpPostBegin = boost::chrono::high_resolution_clock::now();
						}

						if(bDisplay)
							std::cout << date_courante << flush;

						sim_hyd->Termine();

						if(sim_hyd->_bLogPerf)
							sim_hyd->_logPerformance.EndStep(idx, boost::chrono::high_resolution_clock::now());

						if(!sim_hyd->_outputCDF)	//le calcul des statistiques n'est pas adapté aux netCDF
						{
							string nom_fichier_stats = Combine(sim_hyd->PrendreRepertoireSimulation(), "stats.txt");
							if(FichierExiste(nom_fichier_stats))
							{
								if(sim_hyd->_acheminement_riviere)
								{
									std::cout << endl << "Computing statistics...   " << GetCurrentTimeStr() << flush;
									HYDROTEL::_listLog.push_back("Computing statistics...   " + GetCurrentTimeStr());

									STATISTIQUES stats(*sim_hyd, nom_fichier_stats);
								}
							}
						}

						std::cout << endl;

						std::time(&end);

						oss.str("");
						if((end - begin) / 60.0 / 60.0 < 1.0)
						{
							if ((end - begin) / 60.0 < 1.0)
								oss << "Simulation completed in " << setprecision(0) << setiosflags(ios::fixed) << (end - begin) << " sec   " << GetCurrentTimeStr();
							else
								oss << "Simulation completed in " << setprecision(2) << setiosflags(ios::fixed) << (end - begin) / 60.0 << " min   " << GetCurrentTimeStr();
						}
						else
							oss << "Simulation completed in " << setprecision(2) << setiosflags(ios::fixed) << (end - begin) / 60.0 / 60.0 << " h   " << GetCurrentTimeStr();

						HYDROTEL::Log("");
						HYDROTEL::Log(oss.str());
						HYDROTEL::Log("");

						if(sim_hyd->_bLogPerf)
						{
							sim_hyd->_logPerformance._tpPostEnd = boost::chrono::high_resolution_clock::now();
							sim_hyd->_logPerformance._tpInitAndSimEnd = boost::chrono::high_resolution_clock::now();

							sim_hyd->_logPerformance.AddStep("Program execution end");

							str = sim_hyd->PrendreRepertoireSimulation() + "/log-performance-" + GetCurrentTimeStrForFile() + ".txt";
							if(!sim_hyd->_logPerformance.SaveFile(str))
								HYDROTEL::Log(sim_hyd->_logPerformance._sErr);
							else
							{
								HYDROTEL::Log("Performance log file saved: " + str);
								HYDROTEL::Log("");
							}
						}
					}
				}
			}
			catch(const exception& e)
			{
				ret = 1;

				std::cout << endl << endl << e.what() << endl << endl;

				if(_log)
				{
					ofstream fichier_log(_nom_fichier_log);
					if(fichier_log)
					{
						for(idx=0; idx!=HYDROTEL::_listLog.size(); idx++)
						{
							if(HYDROTEL::_listLog[idx] == "")
								fichier_log << endl;
							else
								fichier_log << HYDROTEL::_listLog[idx] << endl;
						}

						fichier_log << endl << endl << e.what() << endl << endl;
						fichier_log.close();
					}
					else
						std::cout << "Error saving log file: " << _nom_fichier_log << endl << endl;
				}
			}
		}
	}

	if(ret != 1 && _log && HYDROTEL::_listLog.size() != 0)
	{
		ofstream fichier_log(_nom_fichier_log);
		if(fichier_log)
		{
			for(idx=0; idx!=HYDROTEL::_listLog.size(); idx++)
			{
				if(HYDROTEL::_listLog[idx] == "")
					fichier_log << endl;
				else
					fichier_log << HYDROTEL::_listLog[idx] << endl;
			}

			fichier_log.close();
		}
		else
			std::cout << "Error saving log file: " << _nom_fichier_log << endl << endl;
	}

	return ret;
}


namespace boost
{
	//catch BOOST_ASSERT in release mode	//BOOST_ENABLE_ASSERT_HANDLER must be defined

	void assertion_failed(char const* expr, char const* function, char const* file, long line)
	{
		size_t i;
		string str;
		
		str = function;

		ostringstream oss;
		oss.str("");
		oss << line;

		str = "Assertion failed: ";
		str += expr;
		str += ", file ";
		str += file;
		str += ", line" + oss.str();

		std::cout << endl << endl << str << endl;

		if(_log)
		{
			ofstream fichier_log(_nom_fichier_log);
			if(fichier_log)
			{
				for(i=0; i!=HYDROTEL::_listLog.size(); i++)
				{
					if(HYDROTEL::_listLog[i] == "")
						fichier_log << endl;
					else
						fichier_log << HYDROTEL::_listLog[i] << endl;
				}

				fichier_log << endl << endl << str << endl;
				fichier_log.close();
			}
			else
				std::cout << "Error saving log file: " << _nom_fichier_log << endl << endl;
		}

		exit(2);
	}
}

