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
#include "../graphics/text_button.h"
#include "platform_runtime.h"
std::string language = "en";
moFileLib::moFileReader reader;
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
    platform->clear_font_cache();
    if (lang == "en" || lang == "") {
        language = "en";
    } else {
        auto contents = platform->read_file("locales/dmi/" + lang + ".mo");
        if (!contents || reader.ParseData(*contents) != moFileLib::moFileReader::EC_SUCCESS) {
            platform->debug_print(reader.GetErrorDescription());
            language = "en";
        } else {
            language = lang;
        }
    }
    extern TextButton main_button;
    extern TextButton override_button;
    extern TextButton dataview_button;
    extern TextButton special_button;
    main_button.rename(get_text_context("Navigation bar", "Main"));
    override_button.rename(get_text_context("Navigation bar", "Over-\nride"));
    dataview_button.rename(get_text_context("Navigation bar", "Data\nview"));
    special_button.rename(get_text_context("Navigation bar", "Spec"));
}

std::string get_language()
{
    return language;
}
