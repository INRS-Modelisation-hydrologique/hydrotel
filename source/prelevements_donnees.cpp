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

#include "prelevements_donnees.hpp"

#include <sstream> 


using namespace std;


namespace HYDROTEL
{

    PRELEVEMENTS_DONNEES::PRELEVEMENTS_DONNEES()
        {
        init();
        }

    void PRELEVEMENTS_DONNEES::init()
        {
        m_vNbrjour.assign(12, 0);
        m_vCons.assign(12, 0);
        m_vVol.assign(12, 0);
        }

    void PRELEVEMENTS_DONNEES::asgNbrjour(const int idmois, const int p_nbrjour)
        {
        m_vNbrjour[idmois - 1] = p_nbrjour;
        }

    void PRELEVEMENTS_DONNEES::asgConsomation(const int idmois, const double p_consomation)
        {
        m_vCons[idmois - 1] = p_consomation;
        }

    void PRELEVEMENTS_DONNEES::asgVolume(const int idmois, const double p_volume)
        {
        m_vVol[idmois - 1] = p_volume;
        }

    std::string PRELEVEMENTS_DONNEES::printDonnee() 
        {
        std::string str = printVector(m_vNbrjour) + ";" + printVector(m_vVol) + ";" + printVector(m_vCons);
        return str;
        }

    template<typename T>
    std::string PRELEVEMENTS_DONNEES::printVector(std::vector<T> v_print)
        {
        std::ostringstream os; 
        for (size_t i = 0; i < (v_print.size()-1); i++)
            {
            os << fixed << v_print[i] << ";";
            }
        os << fixed << v_print[v_print.size() - 1];
        return  os.str();
        }
 
}
