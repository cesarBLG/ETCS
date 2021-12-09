/*
 * European Train Control System
 * Copyright (C) 2022  César Benito <cesarbema2009@hotmail.com>
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
#include "radio.h"
#include "messages.h"
#include "15.h"
std::map<int, euroradio_message*> euroradio_message::message_factory;
struct MA_message : euroradio_message
{
    std::shared_ptr<Level2_3_MA> MA;
    MA_message() {}
    MA_message(bit_read_temp &r)
    {
        r.read(&NID_MESSAGE);
        r.read(&L_MESSAGE);
        r.read(&T_TRAIN);
        r.read(&M_ACK);
        r.read(&NID_LRBG);
        MA = std::shared_ptr<Level2_3_MA>(new Level2_3_MA(r));
        packets.push_back(MA);
    }
    MA_message * create(bit_read_temp &r) override { return new MA_message(r);}
};
void euroradio_message::initialize()
{
    message_factory[3] = new MA_message();
    message_factory[24] = new euroradio_message();
}