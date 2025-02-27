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

#ifndef PRELEVEMENTS_DONNEES_H_INCLUDED
#define PRELEVEMENTS_DONNEES_H_INCLUDED


#include <iostream>
#include <vector>


namespace HYDROTEL
{

    class PRELEVEMENTS_DONNEES
    {
        public:
            PRELEVEMENTS_DONNEES();

            void init();
            void asgNbrjour(const int idmois, const int p_nbrjour);
            void asgConsomation(const int idmois, const double p_consomation);
            void asgVolume(const int idmois, const double p_volume);
            std::string printDonnee();

            template<typename T>
            std::string printVector(std::vector<T> v_print);
            std::vector<int> m_vNbrjour;
            std::vector<double> m_vCons;
            std::vector<double> m_vVol;
    };

}

#endif 