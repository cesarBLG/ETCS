/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "variables.h"
#include "types.h"
#include "packets.h"
#include "radio.h"
#include "telegram.h"
#include "../Position/linking.h"
#include "../MA/movement_authority.h"
#include "../Supervision/speed_profile.h"
#include "../Euroradio/session.h"
#include "radio.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <deque>
extern std::deque<std::pair<eurobalise_telegram, std::pair<distance,int64_t>>> pending_telegrams;
extern std::list<link_data>::iterator link_expected;
void update_track_comm();
bool handle_radio_message(std::shared_ptr<euroradio_message> msg, communication_session *session);
void set_message_filters();
