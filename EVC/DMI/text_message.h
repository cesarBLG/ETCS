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
#pragma once
#include <string>
#include <functional>
#include "dmi.h"
#include "../Packets/72.h"
#include <ctime>
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
    std::function<bool(text_message&)> start_condition;
    std::function<bool(text_message&)> end_condition;
    text_message(std::string text, bool fg, bool ack, int reason, std::function<bool(text_message&)> end_condition);
};
extern std::list<text_message> messages;
text_message &add_message(text_message t);
void add_message(PlainTextMessage m, distance ref);
void update_messages();
void message_acked(int id);