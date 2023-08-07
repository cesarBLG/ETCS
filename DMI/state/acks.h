/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _ACKS_H
#define _ACKS_H
enum struct AckType
{
    None,
    Brake,
    Level,
    Mode,
    Message
};
extern AckType AllowedAck;
extern Component *componentAck;
extern Component ackButton;
void setAck(AckType type, int id, bool ack);
void updateAcks();
#endif