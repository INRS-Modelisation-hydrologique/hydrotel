#ifndef TEMP_EAU_H_INCLUDED
#define TEMP_EAU_H_INCLUDED

#include "sous_modele.hpp"
#include <fstream>

namespace HYDROTEL
{

	class TEMP_EAU : public SOUS_MODELE
	{
	public:
		TEMP_EAU(SIM_HYD& sim_hyd, const std::string& nom);
		virtual ~TEMP_EAU() = 0;

		virtual void ChangeNbParams(const ZONES& zones);
		virtual void Initialise();
		virtual void Calcule();
		virtual void Termine();

		std::string		_nom_fichier_temp_eau;

	protected:
		size_t			_nbTroncons;

	private:
		std::ofstream	_fichier_temp_eau;

		
		float* _netCdf_tempeau;
	};

}

#endif
