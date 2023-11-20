/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "language.h"
#include <moFileReader.hpp>
#include "../DMI/dmi.h"
#include "../STM/stm.h"
#include "../Packets/STM/30.h"
#include "../TrainSubsystems/cold_movement.h"
#include "platform_runtime.h"
moFileLib::moFileReader reader;
std::string language = "en";
std::string get_text(std::string id)
{
    if (language == "en") return id;
    return reader.Lookup(id.c_str());
}
std::string get_text_context(std::string context, std::string id)
{
    if (language == "en") return id;
    return reader.LookupWithContext(context.c_str(), id.c_str());
}
void set_language(std::string lang)
{
    if (lang == "en" || lang == "") {
        language = "en";
    } else {
        auto contents = platform->read_file("locales/evc/" + lang + ".mo");
        reader.ClearTable();
        if (!contents || reader.ParseData(*contents) != moFileLib::moFileReader::EC_SUCCESS) {
            platform->debug_print(reader.GetErrorDescription());
            language = "en";
        } else {
            language = lang;
        }
    }
    for (auto it : installed_stms) {
        stm_message msg;
        auto *pack = new STMLanguage();
        pack->NID_DRV_LANGUAGE.rawdata = ((language[0]&0xFF)<<8)|(language[1]&0xFF);
        msg.packets.push_back(std::shared_ptr<ETCS_packet>(pack));
        it.second->send_message(&msg);
    }
    set_persistent_command("language", lang);
    json j = json::parse("\""+lang+"\"");
    save_cold_data("Language", j);
}
void load_language()
{
    json j = load_cold_data("Language");
    set_language(j.is_null() ? "" : j);
}
