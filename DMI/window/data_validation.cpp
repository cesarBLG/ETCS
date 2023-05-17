/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "data_validation.h"
#include "../tcp/server.h"
void validation_window::sendInformation()
{
    json j = R"({"DriverSelection":"ValidateDataEntry"})"_json;
    j["WindowTitle"] = title;
    for (auto i : validation_data)
    {
        j["DataInputResult"][i->label] = i->data;
    }
    j["DataInputResult"]["Validated"] = confirmation->data_accepted == get_text("Yes");
    write_command("json", j.dump());
}