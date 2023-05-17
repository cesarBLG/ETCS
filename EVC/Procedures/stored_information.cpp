/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../Supervision/speed_profile.h"
#include "../Position/linking.h"
#include "stored_information.h"
#include "../MA/movement_authority.h"
#include "../Packets/radio.h"
void svl_shorten(char condition)
{
    if (!MA || !MA->SvL_ma) return;
    distance d = *MA->SvL_ma;
    // MA already shortened
    delete_linking(d);
    delete_gradient(d);
    if (condition != 'a' && condition != 'b' && condition != 'f' && (level == Level::N2 || level == Level::N3))
        ma_rq_reasons[3] = true;
}
void train_shorten(char condition)
{
    distance d = d_maxsafefront(odometer_orientation, 0);
    delete_linking(d);
    delete_gradient(d);
    delete_MA(d_estfront, d);
    if (level == Level::N2 || level == Level::N3)
        ma_rq_reasons[3] = true;
}
void desk_closed_som()
{
    /*delete_linking();
    delete_TSRs();
    recalculate_MRSP();*/
}