#pragma once
#include <string>
#include <functional>
#include "dmi.h"
#include "../Packets/72.h"
#include <ctime>
struct text_message
{
    int id;
    std::string text;
    int hour;
    int minute;
    bool firstGroup;
    bool ack;
    bool ackeb;
    bool acksb;
    int reason;
    bool acknowledged;
    bool shown;
    int trackid;
    int64_t first_displayed;
    distance first_distance;
    std::function<bool(text_message&)> start_condition;
    std::function<bool(text_message&)> end_condition;
    text_message(std::string text, bool fg, bool ack, int reason, std::function<bool(text_message&)> end_condition);
};
text_message &add_message(text_message t);
void add_message(PlainTextMessage m, distance ref);
void update_messages();
void message_acked(int id);