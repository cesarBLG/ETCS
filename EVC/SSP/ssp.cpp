/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ssp.h"
std::vector<SSP_element> get_SSP(distance start, InternationalSSP issp)
{
    std::vector<SSP_element> ssp;
    std::vector<SSP_element_packet> elements;
    elements.push_back(issp.element);
    elements.insert(elements.end(), issp.elements.begin(), issp.elements.end());
    for (int i=0; i<elements.size(); i++)
    {
        SSP_element e;
        e.compensate_train_length = elements[i].Q_FRONT;
        double dist = elements[i].D_STATIC.get_value(issp.Q_SCALE);
        start += dist;
        e.start = start;
        if (elements[i].V_STATIC == V_STATIC_t::EndOfProfile)
        {
            e.restrictions[0][0] = -1;
            ssp.push_back(e);
            break;
        }
        e.restrictions[0][0] = elements[i].V_STATIC.get_value();
        for (int j=0; j<elements[i].diffs.size(); j++) {
            int diff = elements[i].diffs[j].Q_DIFF;
            if (diff == 0)
                e.restrictions[0][elements[i].diffs[j].NC_CDDIFF.get_value()] = elements[i].diffs[j].V_DIFF.get_value();
            else
                e.restrictions[diff][elements[i].diffs[j].NC_DIFF] = elements[i].diffs[j].V_DIFF.get_value();
        }
        ssp.push_back(e);
    }
    return ssp;
}