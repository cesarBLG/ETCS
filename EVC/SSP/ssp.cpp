/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
        e.compensate_train_length = elements[i].Q_FRONT == elements[i].Q_FRONT.TrainLengthDelay;
        double dist = elements[i].D_STATIC.get_value(issp.Q_SCALE);
        start += dist;
        e.start = start;
        if (elements[i].V_STATIC == elements[i].V_STATIC.EndOfProfile)
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