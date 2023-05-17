/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../Packets/packets.h"
#include "../Packets/radio.h"
std::shared_ptr<ETCS_packet> translate_packet(std::shared_ptr<ETCS_packet> packet, std::vector<std::shared_ptr<ETCS_packet>> packets, int version);
std::shared_ptr<euroradio_message> translate_message(std::shared_ptr<euroradio_message> message, int version);
std::shared_ptr<euroradio_message_traintotrack> translate_message(std::shared_ptr<euroradio_message_traintotrack> message, int version);