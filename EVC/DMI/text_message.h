/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <string>
#include <functional>
#include "dmi.h"
#include "../Packets/72.h"
#include "../Packets/76.h"
#include "../Position/distance.h"
#include "../Euroradio/session.h"
#include <list>
enum struct text_message_type
{
    SystemStatus,
    PlainText,
    FixedText,
};
struct text_message
{
    unsigned int id;
    std::string text;
    int hour;
    int minute;
    bool firstGroup;
    bool ack;
    int reason;
    bool acknowledged;
    bool shown;
    text_message_type type = text_message_type::SystemStatus;
    int trackid;
    int64_t first_displayed;
    distance first_distance;
    optional<contact_info> report_rbc;
    std::function<bool(text_message&)> start_condition;
    std::function<bool(text_message&)> end_condition;
    text_message(std::string text, bool fg, bool ack, int reason, std::function<bool(text_message&)> end_condition);
};
extern std::list<text_message> messages;
text_message &add_message(text_message t);
void add_message(PlainTextMessage m, optional<distance> ref);
void add_message(FixedTextMessage m, optional<distance> ref);
void update_messages();
void message_acked(int id);